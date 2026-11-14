from pathlib import Path
import subprocess
workspace=Path(__file__).resolve().parent.parent
s=(workspace/'work/hekate-6.5.4/nyx/nyx_gui/diagnostics/vendor/verdict.c').read_text(encoding='utf-8')
function=s[s.index('void probe_verdict(void)'):].strip()
test=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
typedef uint32_t u32;
#define PROBE_G_POWER 2
#define PROBE_G_STORAGE 8
#define COL_ERR 3
#define COL_WARN 2
#define COL_OK 1
typedef enum {DX_PASS,DX_WARN,DX_FAIL} dx_sev_t;
typedef struct{const char*key;dx_sev_t sev;char detail[48];}dx_finding_t;
struct dx_macro{const char*name;const char*const*keys;u32 group_mask;bool require_all_groups;};
static const char*const keys[]={"a","b",NULL};
static const struct dx_macro g_dx_macros[]={{"Category",keys,PROBE_G_POWER,false}};
static dx_finding_t a={"a",DX_PASS,""},b={"b",DX_PASS,""};
static int present;
static u32 g_run_mask=PROBE_G_POWER;
static char output[4096];
static const dx_finding_t*dx_get(const char*k){return !strcmp(k,"a")?(present&1?&a:NULL):(present&2?&b:NULL);}
static void _emit_xchecks(void){}
static void emit(u32 color,const char*fmt,...){(void)color;va_list ap;va_start(ap,fmt);vsnprintf(output+strlen(output),sizeof(output)-strlen(output),fmt,ap);va_end(ap);}
#define log_color(...) emit(__VA_ARGS__)
#define HEADER(s) emit(0,"%s\n",s)
#define LOG(...) emit(0,__VA_ARGS__)
__FUNCTION__
static void run(int n){output[0]=0;present=n;probe_verdict();}
int main(void){
 run(0);assert(strstr(output,"Category : not tested"));assert(strstr(output,"Not reported : 2"));
 run(1);assert(strstr(output,"Category : partial"));assert(strstr(output,"Not reported : 1"));
 run(3);assert(strstr(output,"Category : pass"));assert(!strstr(output,"Not reported"));
 a.sev=DX_FAIL;strcpy(a.detail,"broken");run(1);assert(strstr(output,"Category : FAIL (broken)"));
 a.sev=DX_WARN;run(1);assert(strstr(output,"Category : warn (broken)"));
 b.sev=DX_FAIL;strcpy(b.detail,"worst");run(3);assert(strstr(output,"Category : FAIL (worst)"));
 puts("PASS: actual verdict distinguishes missing, partial, complete, warning and failed checks (6 cases).");
}
'''.replace('__FUNCTION__',function)
p=workspace/'work/native_verdict_host.c';p.write_text(test,encoding='utf-8')
exe=p.with_suffix('.exe')
subprocess.run([str(workspace/'work/toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(p),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
