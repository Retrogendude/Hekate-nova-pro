"""Exercise actual native adapter capture/export code with mocked FATfs only."""
from pathlib import Path
import re, subprocess
workspace=Path(__file__).resolve().parent.parent
src=workspace/'work/hekate-6.5.4/nyx/nyx_gui/diagnostics/nyx_diagnostics.c'
s=src.read_text(encoding='utf-8')
def function(signature):
    start=s.index(signature);brace=s.index('{',start);depth=0
    for i in range(brace,len(s)):
        if s[i]=='{':depth+=1
        elif s[i]=='}':
            depth-=1
            if depth==0:return s[start:i+1]
    raise ValueError(signature)
header=(src.parent/'nyx_diagnostics.h').read_text(encoding='utf-8')
enum=re.search(r'typedef enum \{.*?\} nyx_diag_state_t;',header,re.S).group()
probe=re.search(r'typedef struct \{\n    const char \*name;.*?\} nyx_diag_probe_t;',header,re.S).group()
test=r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;
#define CAPACITY (256u*1024u)
static char *nyx_translate_text(const char *text) { (void)text; return NULL; }
#define NYX_DIAG_PROBE_COUNT 42
#define COL_ERR 3
#define COL_WARN 2
__ENUM__
__PROBE__
static char _log_buf[4096];
static char *report;
static u32 report_used,active_index,worst_finding;
static bool running,muted,truncated,has_finding;
static nyx_diag_probe_t metadata[NYX_DIAG_PROBE_COUNT];
static bool mounted;
static int mount_error,open_error,write_error,close_error,partial,unmounts,closes;
typedef int FRESULT;
typedef unsigned UINT;
typedef struct { int dummy; } FIL;
#define FR_OK 0
#define FA_WRITE 1
#define FA_CREATE_NEW 2
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){mounted=false;unmounts++;}
static int f_mkdir(const char*p){(void)p;return 0;}
static FRESULT f_open(FIL*f,const char*p,int flags){(void)f;assert(strstr(p,"bootloader/diagnostics/hwtest_")==p);assert(flags==(FA_WRITE|FA_CREATE_NEW));return open_error;}
static FRESULT f_write(FIL*f,const void*p,UINT n,UINT*w){(void)f;assert(p==report);*w=partial?n-1:n;return write_error;}
static FRESULT f_close(FIL*f){(void)f;closes++;return close_error;}
static u32 get_tmr_ms(void){return 7;}
static void s_printf(char *p,const char *fmt,...){va_list ap;va_start(ap,fmt);vsprintf(p,fmt,ap);va_end(ap);}
__FUNCTIONS__
static void reset(void){report_used=0;report[0]=0;running=muted=truncated=false;active_index=0;has_finding=false;worst_finding=0;memset(metadata,0,sizeof(metadata));mounted=false;mount_error=open_error=write_error=close_error=partial=unmounts=closes=0;}
int main(void){
 report=malloc(CAPACITY+16);assert(report);memset(report,0xA5,CAPACITY+16);reset();
 append("hello",5);assert(report_used==5&&!strcmp(report,"hello"));
 muted=true;append("ignored",7);assert(report_used==5);muted=false;
 running=true;metadata[0].state=NYX_DIAG_OBSERVATION;strcpy(_log_buf,"warn\n");_log_emit(COL_WARN);assert(metadata[0].state==NYX_DIAG_WARN);
 _log_emit(COL_ERR);assert(metadata[0].state==NYX_DIAG_FAIL);
 _log_emit(COL_WARN);assert(metadata[0].state==NYX_DIAG_FAIL);
 nyx_diag_record_severity(0);assert(has_finding&&worst_finding==0);nyx_diag_record_severity(2);assert(worst_finding==2);nyx_diag_record_severity(1);assert(worst_finding==2);
 muted=true;metadata[1].state=NYX_DIAG_OBSERVATION;active_index=1;_log_emit(COL_ERR);assert(metadata[1].state==NYX_DIAG_OBSERVATION);muted=false;running=false;
 reset();char *large=malloc(CAPACITY);memset(large,'x',CAPACITY);append(large,CAPACITY-22);append("oversized",9);assert(truncated);assert(strstr(report,"[REPORT TRUNCATED]"));assert(report[report_used]==0);for(int i=0;i<16;i++)assert((unsigned char)report[CAPACITY+i]==0xA5);u32 len=report_used;append(large,100);assert(report_used==len);free(large);
 for(int own=0;own<2;own++)for(int failure=0;failure<5;failure++){
   reset();append("diagnostic\n",11);mounted=own;char path[128]="unchanged";
   if(failure==1)open_error=1;if(failure==2)write_error=1;if(failure==3)partial=1;if(failure==4)close_error=1;
   int result=nyx_diag_export(path);assert((result==0)==(failure==0));assert(mounted==(bool)own);assert(unmounts==!own);assert(closes==(failure==1?0:1));
   if(!failure)assert(!strcmp(path,"bootloader/diagnostics/hwtest_00000007.txt"));else assert(!strcmp(path,"unchanged"));
 }
 reset();mount_error=1;append("x",1);assert(nyx_diag_export(NULL)==2);assert(!mounted);
 reset();assert(nyx_diag_export(NULL)==1);append("x",1);running=true;assert(nyx_diag_export(NULL)==1);
 free(report);puts("PASS: real adapter bounded capture, mute/severity rules, and 10 export success/fault cases plus preconditions.");
}
'''.replace('__ENUM__',enum).replace('__PROBE__',probe)
functions=[function('static void append('),function('void _log_emit('),function('void nyx_diag_record_severity('),function('int nyx_diag_export(')]
test=test.replace('__FUNCTIONS__','\n'.join(functions))
dest=workspace/'work/native_backend_host_test.c';dest.write_text(test,encoding='utf-8')
exe=workspace/'work/native_backend_host_test.exe'
gcc=workspace/'work/toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(gcc),'-std=c11','-O2','-Wall',str(dest),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
