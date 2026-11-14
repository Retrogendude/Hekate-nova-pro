from pathlib import Path
import re,subprocess,json
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui/diagnostics';folder=work/'components-test-v17';folder.mkdir(exist_ok=True)
def strip(p):return re.sub(r'^#include[^\n]*\n','',p.read_text(encoding='utf-8'),flags=re.M)
headers=strip(root/'nyx_diagnostics.h')+strip(root/'component_summary.h')
pre=r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef uint8_t u8;
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
'''
stubs=r'''
static nyx_diag_probe_t probes[42];static nyx_diag_finding_t findings[64];static u32 count;
static bool sd_ready,emmc_ready;
const nyx_diag_probe_t *nyx_diag_probe(u32 index){return index<42?&probes[index]:NULL;}
u32 nyx_diag_finding_count(void){return count;}
bool nyx_diag_finding(u32 i,nyx_diag_finding_t *out){if(i>=count)return false;*out=findings[i];return true;}
bool nyx_diag_storage_ready(bool emmc){return emmc?emmc_ready:sd_ready;}
bool nyx_diag_truncated(void){return false;}
static const char *nyx_tr(const char *text){return text;}
static void nyx_diag_snprintf(char *out,u32 capacity,const char *format,...){va_list ap;va_start(ap,format);vsnprintf(out,capacity,format,ap);va_end(ap);}
static void reset(void){memset(probes,0,sizeof probes);memset(findings,0,sizeof findings);count=0;sd_ready=emmc_ready=true;}
static void fact(const char *key,u32 severity,const char *detail){nyx_diag_finding_t *f=&findings[count++];f->key=key;f->severity=severity;snprintf(f->detail,sizeof f->detail,"%s",detail);}
static unsigned checks;
#define CHECK(condition)do{++checks;if(!(condition)){printf("FAIL %d: %s\n",__LINE__,#condition);return 1;}}while(0)
'''
tests=r'''
int main(void){
 for(u32 index=0;index<nyx_component_count();++index){
  reset();nyx_component_result r;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_NOT_TESTED);
  const nyx_component_definition *d=r.definition;
  for(u32 p=0;p<d->probe_count;++p)probes[d->probes[p]].state=NYX_DIAG_OBSERVATION;
  CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_PARTIAL);
  if(!d->required_count)continue;
  for(u32 k=0;k<d->required_count;++k)fact(d->keys[k],0,"");
  CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_PASS&&!r.partial);
  for(u32 k=0;k<d->required_count;++k){
   findings[k].severity=2;snprintf(findings[k].detail,48,"measured %u",k);
   CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_ANOMALY);CHECK(strstr(r.measurement,"measured"));
   findings[k].severity=1;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_WARN&&r.partial);
   findings[k].severity=0;
  }
  probes[d->probes[0]].state=NYX_DIAG_ERROR;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_TEST_ERROR);
  probes[d->probes[0]].state=NYX_DIAG_OBSERVATION;
  if(d->storage){sd_ready=emmc_ready=false;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_PARTIAL);sd_ready=emmc_ready=true;}
  findings[0].severity=999;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_TEST_ERROR);findings[0].severity=0;
  --count;CHECK(nyx_component_get(index,&r));CHECK(r.state==NYX_COMPONENT_PARTIAL);
 }
 reset();for(u32 i=0;i<42;++i)probes[i].state=NYX_DIAG_OBSERVATION;
 fact("cpu_complex",2,"CPU0 wait failed");fact("wifi_pcie",2,"no service");fact("charger_id",2,"no reply");fact("usb_pd",2,"no reply");
 char *summary=nyx_component_summary_text();CHECK(summary&&strlen(summary)<32768);
 CHECK(strstr(summary,"CPU0 wait failed")&&strstr(summary,"no reply"));
 CHECK(strstr(summary,"dépend du service CPU0")&&strstr(summary,"liaisons de contrôle communes"));
 CHECK(strstr(summary,"stress")||strstr(summary,"RAM isolé"));CHECK(strstr(summary,"NON TESTÉ")||strstr(summary,"PARTIEL"));free(summary);
 for(u32 i=0;i<nyx_component_count();++i){char *detail=nyx_component_detail_text(i);CHECK(detail&&strlen(detail)<32768);free(detail);}
 CHECK(!nyx_component_detail_text(999));CHECK(!nyx_component_get(999,NULL));
 reset();nyx_component_result manual;
 nyx_component_observation(NYX_MANUAL_COMMANDS,"buttons=9/18, rest unknown",false);
 CHECK(nyx_component_get(13,&manual)&&manual.state==NYX_COMPONENT_PARTIAL&&manual.partial);
 char *captured=nyx_component_detail_text(13);CHECK(captured&&strstr(captured,"buttons=9/18")&&strstr(captured,"Mesures interactives"));free(captured);
 nyx_component_observation(NYX_MANUAL_DISPLAY,"white pattern: user reported a spot",true);
 CHECK(nyx_component_get(11,&manual)&&manual.state==NYX_COMPONENT_WARN&&manual.partial);
 captured=nyx_component_summary_text();CHECK(captured&&strstr(captured,"white pattern: user reported a spot")&&strstr(captured,"buttons=9/18"));free(captured);
 nyx_component_observation(NYX_MANUAL_DISPLAY,NULL,false);CHECK(nyx_component_get(11,&manual)&&manual.state==NYX_COMPONENT_NOT_TESTED);
 char large[4096];memset(large,'x',4095);large[4095]=0;nyx_component_observation(NYX_MANUAL_TOUCH,large,false);
 captured=nyx_component_summary_text();CHECK(captured&&strlen(captured)<32768);free(captured);
 puts("PASS: scoped component evidence, skipped/absent/partial states, no false storage PASS, invalid records, measured causes, cross-checks and bounded summary/detail.");
 printf("PASS: %u component assertions.\n",checks);return 0;
}
'''
program=folder/'check.c';program.write_text(pre+headers+stubs+strip(root/'component_summary.c')+tests,encoding='utf-8')
exe=folder/'check.exe';subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(program),'-o',str(exe)],check=True)
run=subprocess.run([str(exe)],capture_output=True,text=True);print(run.stdout,end='');assert not run.returncode,run.stderr
(work/'components-check-v17.json').write_text(json.dumps({'summary':run.stdout,'component_count':15,'hardware_io':'not linked or executed'},indent=2),encoding='utf-8')
