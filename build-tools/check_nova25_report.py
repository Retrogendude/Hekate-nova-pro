"""Exercise the real manual SD report writer with file/card I/O substituted."""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v25';d=w/'nova25-tests'
f=p/'nyx/nyx_gui/frontend/gui_bluetooth.c';s=f.read_text();a=s.index('static bool save_report(');b=s.index('\nstatic lv_res_t stop(',a);body=s[a:b]
test=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <utils/types.h>
#include <utils/sprintf.h>
#include "nova_bt.h"
#define NYX_NOVA_VERSION "25"
#define FR_EXIST 8
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 8
typedef struct {int unused;} FIL;
static int mounted,mount_error,mkdir_error,open_error,write_error,short_write,close_error,no_memory;
static unsigned mounts,unmounts,opens,writes,closes,now;
static char captured[8192];static nova_bt_status state;
uint32_t nova_bt_now(void){return now;}
const nova_bt_status *nova_bt_get_status(void){return &state;}
static int sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){mounts++;if(mount_error)return mount_error;mounted=1;return 0;}
static void sd_unmount(void){unmounts++;mounted=0;}
static int f_mkdir(const char *path){assert(!strcmp(path,"bootloader/bluetooth"));return mkdir_error;}
static int f_open(FIL *fp,const char *path,int flags){(void)fp;(void)flags;assert(!strcmp(path,"bootloader/bluetooth/last-session.txt"));opens++;return open_error;}
static int f_write(FIL *fp,const void *data,u32 n,u32 *written){(void)fp;assert(n<sizeof captured);memcpy(captured,data,n);captured[n]=0;writes++;*written=short_write?n-1:n;return write_error;}
static int f_close(FIL *fp){(void)fp;closes++;return close_error;}
static void *report_alloc(unsigned n){return no_memory?NULL:calloc(1,n);}
#define malloc(n) report_alloc(n)
'''+body+r'''
static void reset(void){mounted=mount_error=mkdir_error=open_error=write_error=short_write=close_error=no_memory=0;mounts=unmounts=opens=writes=closes=0;captured[0]=0;}
int main(void){
 nova_bt_trace_reset();nova_bt_trace_add(5,0x40,0x0201,0x11);state.pad[0].last_error=0x69;state.pad[0].last_psm=0x11;state.pad[0].auth_error=5;state.host_identity=1;state.pad[0].auth_seen=1;state.pad[0].encryption_seen=1;state.pad[0].encryption_error=8;state.pad[0].disconnect_reason=8;
 reset();assert(save_report());assert(mounts==1&&unmounts==1&&opens==1&&writes==1&&closes==1);assert(strstr(captured,"ERR=69 PSM=0011"));assert(strstr(captured,"AUTH=05 SEEN=1 ENC=08 SEEN=1 ENABLED=0 DISC=08")&&strstr(captured,"HOST=1"));assert(strstr(captured,"00000000 05 0040 0201 0011"));
 reset();mounted=1;assert(save_report()&&mounted&&!unmounts);
 reset();mount_error=1;assert(!save_report()&&!opens&&!unmounts);
 reset();mkdir_error=3;assert(!save_report()&&!opens&&unmounts==1);
 reset();mkdir_error=FR_EXIST;assert(save_report());
 reset();open_error=3;assert(!save_report()&&!writes&&!closes&&unmounts==1);
 reset();write_error=2;assert(!save_report()&&closes==1&&unmounts==1);
 reset();short_write=1;assert(!save_report()&&closes==1);
 reset();close_error=3;assert(!save_report()&&unmounts==1);
 reset();no_memory=1;assert(!save_report()&&!mounts);
 nova_bt_trace_reset();for(unsigned n=0;n<160;n++)nova_bt_trace_add(5,n,0x0201,17);
 reset();assert(save_report()&&strlen(captured)<8192);
 puts("PASS: 11 real report writer paths: mount success=0, restore mount state, existing directory, short writes, errors, bounded trace");
}
'''
(d/'report.c').write_text(test)
cc=w/'toolchains/w64devkit/bin/gcc.exe'
r=subprocess.run([str(cc),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth'),str(d/'report.c'),str(p/'bdk/utils/sprintf.c'),str(p/'nyx/nyx_gui/bluetooth/nova_bt_trace.c'),'-o',str(d/'report.exe')],capture_output=True,text=True)
assert r.returncode==0,r.stderr
r=subprocess.run([str(d/'report.exe')],capture_output=True,text=True,timeout=10);print(r.stdout,r.stderr);assert r.returncode==0,r.returncode
(w/'nova25-report-check.json').write_text(json.dumps({'passed':True,'cases':11,'output':r.stdout,'source_sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'card_io':'substituted'},indent=2))
