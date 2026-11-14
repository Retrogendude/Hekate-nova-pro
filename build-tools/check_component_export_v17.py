from pathlib import Path
import re,subprocess
work=Path(__file__).resolve().parent;source=(work/'hekate-6.5.4/nyx/nyx_gui/diagnostics/nyx_diagnostics.c').read_text(encoding='utf-8')
a=source.index('int nyx_diag_export(char *path)');b=source.index('\n}',a)+2
folder=work/'component-export-v17';folder.mkdir(exist_ok=True)
code=r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef unsigned UINT;typedef int FRESULT;typedef int FIL;
#define FR_OK 0
#define FA_WRITE 1
#define FA_CREATE_NEW 2
#define s_printf sprintf
static bool running,mounted,allocation_failure;static char original[]="ORIGINAL HWTEST REPORT\n";
static char *report=original;static u32 report_used=sizeof(original)-1;
static char captured[1024];static unsigned used,writes,closes,unmounts,summary_calls;
static int mount_error,open_error,close_error,short_at,error_at;
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){mounted=false;unmounts++;}
static void f_mkdir(const char *path){(void)path;}
static u32 get_tmr_ms(void){return 123;}
static int f_open(FIL *f,const char *path,int mode){(void)f;(void)path;assert(mode==(FA_WRITE|FA_CREATE_NEW));return open_error;}
static int f_write(FIL *f,const void *data,UINT length,UINT *written){(void)f;writes++;*written=length-(short_at==(int)writes);memcpy(captured+used,data,*written);used+=*written;captured[used]=0;return error_at==(int)writes?1:0;}
static int f_close(FIL *f){(void)f;closes++;return close_error;}
static char *nyx_component_summary_text(void){summary_calls++;if(allocation_failure)return NULL;char *value=malloc(25);strcpy(value,"\nCOMPONENT SUMMARY\n");return value;}
static void reset(void){running=mounted=allocation_failure=false;report=original;used=writes=closes=unmounts=summary_calls=0;mount_error=open_error=close_error=short_at=error_at=0;captured[0]=0;}
'''
code=code.replace('#include <stdbool.h>','#include <assert.h>\n#include <stdbool.h>')+source[a:b]+r'''
int main(void){
 unsigned checks=0;char path[128]={0};
 for(unsigned initial=0;initial<2;++initial){
  reset();mounted=initial;assert(nyx_diag_export(path)==0);checks++;
  assert(!strcmp(captured,"ORIGINAL HWTEST REPORT\n\nCOMPONENT SUMMARY\n"));assert(writes==2&&closes==1&&summary_calls==1);checks+=2;
  assert(mounted==initial&&unmounts==!initial);assert(strstr(path,"bootloader/diagnostics/hwtest_"));checks+=2;
  for(int step=1;step<=2;++step){
   reset();mounted=initial;short_at=step;assert(nyx_diag_export(NULL)==4&&closes==1&&mounted==initial);checks++;
   reset();mounted=initial;error_at=step;assert(nyx_diag_export(NULL)==4&&closes==1&&mounted==initial);checks++;
  }
  reset();mounted=initial;allocation_failure=true;assert(nyx_diag_export(NULL)==4&&closes==1&&writes==1&&mounted==initial);checks++;
  reset();mounted=initial;close_error=1;assert(nyx_diag_export(NULL)==4&&mounted==initial);checks++;
  reset();mounted=initial;open_error=1;assert(nyx_diag_export(NULL)==3&&!closes&&!writes&&mounted==initial);checks++;
 }
 reset();running=true;assert(nyx_diag_export(NULL)==1&&!writes);checks++;
 reset();report=NULL;assert(nyx_diag_export(NULL)==1&&!writes);checks++;
 reset();mount_error=1;assert(nyx_diag_export(NULL)==2&&!writes);checks++;
 assert(!strcmp(original,"ORIGINAL HWTEST REPORT\n"));checks++;
 printf("PASS: %u actual-C export assertions; original report + component summary, failures, short writes, close and mount ownership.\n",checks);return 0;
}
'''
(folder/'check.c').write_text(code,encoding='utf-8');exe=folder/'check.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(folder/'check.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
