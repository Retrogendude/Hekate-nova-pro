"""Compile actual Nyx I/O functions with fake FatFS and host memory, never HW."""
from pathlib import Path
import hashlib
import json
import subprocess

work = Path(__file__).resolve().parent
project = work / "hekate-nova-pro-v30"
output = work / "nova30-core-tests"
output.mkdir(exist_ok=True)
compiler = work / "toolchains/w64devkit/bin/gcc.exe"


def extract(text, start, end):
    begin = text.index(start)
    return text[begin:text.index(end, begin)]


prefix = r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
typedef uint32_t u32; typedef unsigned int UINT; typedef int FRESULT;
typedef int lv_res_t; typedef void lv_obj_t; typedef struct { int open; } FIL;
enum {FR_OK=0,FR_INT_ERR=2,FR_INVALID_OBJECT=9,FR_NOT_ENOUGH_CORE=17,FA_READ=1,LV_RES_OK=0};
enum {RESOURCE_MIN=0x36E00+4*103*120,NYX_RES_SZ=16*1024*1024};
static struct { unsigned char before[32], data[NYX_RES_SZ], after[32]; } resource;
static unsigned char payload[0x30000];
#define NYX_RES_ADDR resource.data
#define RCM_PAYLOAD_ADDR payload
#define PATCHED_RELOC_ENTRY 0x40010000
#define ALIGN(n,a) (((n)+(a)-1)&~((a)-1))
static void fake_entry(void);
#define EXT_PAYLOAD_ADDR ((uintptr_t)fake_entry)
static u32 file_size, read_short; static int open_error,read_error,close_error,mount_error;
static int opens,reads,closes,mounts,unmounts,ends,launches,relocs,deinits,notices,alloc_fail;
static size_t live, allocs, frees; static const char *filename="test.bin";
#define assert(x) do {if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void *track_alloc(size_t n) { if(alloc_fail)return NULL; size_t *p=malloc(n+sizeof(size_t));assert(p);*p=n;live+=n;allocs++;return p+1; }
static void track_free(void *p) {if(p){size_t *q=(size_t*)p-1;live-=*q;frees++;free(q);}}
static int f_open(FIL *f,const char *p,int mode){(void)p;(void)mode;opens++;f->open=!open_error;return open_error;}
static u32 f_size(FIL *f){assert(f->open);return file_size;}
static int f_close(FIL *f){assert(f->open);f->open=0;closes++;return close_error;}
static int f_read(FIL *f,void *p,UINT n,UINT *received){
 assert(f->open);reads++;u32 actual=read_short?n-1:n;if(received)*received=actual;
 /* Instrument the old unsafe request rather than allowing host corruption. */
 if(p==resource.data&&n>NYX_RES_SZ)return FR_INT_ERR;
 if(!read_error)memset(p,0x6B,actual);return read_error;
}
static const char *lv_list_get_btn_text(lv_obj_t *p){(void)p;return filename;}
static int sd_mount(void){mounts++;return mount_error;}
static void sd_unmount(void){unmounts++;}
static void sd_end(void){ends++;}
static void hw_deinit(int b){(void)b;deinits++;}
static void _reloc_append(uintptr_t a,uintptr_t b,u32 c){(void)a;(void)b;(void)c;relocs++;}
static void fake_entry(void){launches++;}
#define EPRINTF(...) (notices++)
#define EPRINTFARGS(...) (notices++)
#define false 0
#define malloc track_alloc
#define free track_free
static void *sd_file_read(const char *p,u32 *size){FIL f;if(f_open(&f,p,FA_READ))return NULL;u32 n=f_size(&f);if(size)*size=n;void *b=malloc(n);if(f_read(&f,b,n,NULL)){free(b);f_close(&f);return NULL;}f_close(&f);return b;}
'''

suffix = r'''
static void reset(void){
 file_size=0x100;read_short=open_error=read_error=close_error=mount_error=alloc_fail=0;
 opens=reads=closes=mounts=unmounts=ends=launches=relocs=deinits=notices=allocs=frees=0;
 assert(!live);filename="test.bin";memset(&resource,0xA5,sizeof(resource));memset(payload,0,sizeof(payload));
}
static void guards(void){for(unsigned i=0;i<32;i++){assert(resource.before[i]==0xA5);assert(resource.after[i]==0xA5);}}
int main(int argc,char **argv){
 assert(argc==2);reset();int kind=atoi(argv[1]);
 switch(kind){
 case 0: /* Repeated rejection never allocates and never leaks. */
   file_size=0x30001;for(int i=0;i<100;i++){launch_payload(NULL);assert(live==0);}
   assert(allocs==0&&reads==0&&closes==100&&unmounts==100&&!launches);break;
 case 1: filename=NULL;launch_payload(NULL);assert(!opens&&!allocs&&unmounts==1);break;
 case 2: mount_error=1;launch_payload(NULL);assert(mounts==1&&!opens&&!allocs&&unmounts==1);break;
 case 3: open_error=4;launch_payload(NULL);assert(!closes&&!allocs&&!launches&&unmounts==1);break;
 case 4: file_size=0;launch_payload(NULL);assert(!allocs&&!reads&&closes==1&&!launches&&unmounts==1);break;
 case 5: read_short=1;launch_payload(NULL);assert(!live&&allocs==1&&frees==1&&closes==1&&!launches&&unmounts==1);break;
 case 6: read_error=1;launch_payload(NULL);assert(!live&&allocs==1&&frees==1&&closes==1&&!launches&&unmounts==1);break;
 case 7: close_error=1;launch_payload(NULL);assert(!live&&allocs==1&&frees==1&&closes==1&&!launches&&unmounts==1);break;
 case 8: file_size=0x30000;launch_payload(NULL);assert(!live&&allocs==1&&frees==1&&closes==1&&launches==1&&ends==1&&deinits==1&&relocs==1&&unmounts==1);assert(payload[0]==0x6B&&payload[file_size-1]==0x6B);break;
 case 9: alloc_fail=1;launch_payload(NULL);assert(!reads&&closes==1&&!launches&&!live&&unmounts==1);break;
 case 10: file_size=NYX_RES_SZ+1;assert(nyx_load_resources()!=FR_OK);assert(reads==0&&closes==1);guards();break;
 case 11: file_size=RESOURCE_MIN-1;assert(nyx_load_resources()!=FR_OK);assert(!reads&&closes==1);guards();break;
 case 12: file_size=0;assert(nyx_load_resources()!=FR_OK);assert(!reads&&closes==1);guards();break;
 case 13: file_size=RESOURCE_MIN;assert(nyx_load_resources()==FR_OK);assert(reads==1&&closes==1);guards();assert(resource.data[file_size-1]==0x6B&&resource.data[file_size]==0xA5);break;
 case 14: file_size=NYX_RES_SZ;assert(nyx_load_resources()==FR_OK);assert(reads==1&&closes==1);guards();assert(resource.data[file_size-1]==0x6B);break;
 case 15: file_size=405504;assert(nyx_load_resources()==FR_OK);assert(reads==1&&closes==1);guards();break;
 case 16: file_size=RESOURCE_MIN;read_short=1;assert(nyx_load_resources()!=FR_OK);assert(reads==1&&closes==1);guards();break;
 case 17: file_size=RESOURCE_MIN;read_error=1;assert(nyx_load_resources()!=FR_OK);assert(reads==1&&closes==1);guards();break;
 case 18: file_size=RESOURCE_MIN;close_error=1;assert(nyx_load_resources()!=FR_OK);assert(reads==1&&closes==1);guards();break;
 case 19: open_error=4;assert(nyx_load_resources()==4);assert(!reads&&!closes);guards();break;
 default: assert(0);
 }
 assert(!live);printf("PASS case %d\n",kind);return 0;
}
'''

results = {}
for version in (29, 30):
    source_path = work / f"hekate-nova-pro-v{version}/nyx/nyx_gui/nyx.c"
    source = source_path.read_text()
    payload = extract(source, "lv_res_t launch_payload(", "/* Validate size before allocating")
    resources = extract(source, "static int nyx_load_resources()", "static void nyx_load_bg_icons()")
    harness = output / f"core-io-v{version}.c"
    harness.write_text(prefix + payload + resources + suffix)
    executable = harness.with_suffix(".exe")
    build = subprocess.run([str(compiler), "-std=gnu11", "-O1", str(harness), "-o", str(executable)], capture_output=True, text=True)
    assert build.returncode == 0, build.stderr
    cases = range(20) if version == 30 else (0, 5, 7, 10, 11, 12, 16, 18)
    runs = []
    for case in cases:
        run = subprocess.run([str(executable), str(case)], capture_output=True, text=True, timeout=10)
        if version == 30:
            assert run.returncode == 0, f"v30 case {case}: {run.stderr}"
        else:
            assert run.returncode != 0, f"Old v29 unexpectedly passed regression {case}"
        runs.append({"case": case, "exit": run.returncode, "stdout": run.stdout.strip(), "stderr": run.stderr.strip()})
    results[f"v{version}"] = {"source_sha256": hashlib.sha256(source_path.read_bytes()).hexdigest(), "runs": runs}
results["physical_hardware"] = False
results["scope"] = "Extracted actual launch_payload and nyx_load_resources; fake FatFS, heap, registers, entry and buffers"
(output / "results.json").write_text(json.dumps(results, indent=2))
(work/'nova30-core-io-check.json').write_text(json.dumps({'passed':True,'results':results,'source_sha256':results['v30']['source_sha256']},indent=2))
print("PASS: 20 v30 I/O cases; 8 independent v29 regressions reproduced. No hardware benchmark.")
