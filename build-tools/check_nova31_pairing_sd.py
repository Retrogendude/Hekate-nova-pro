"""Actual explicit SD save/load and rollback with an in-memory FatFS model."""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v31';d=w/'nova31-tests';d.mkdir(exist_ok=True)
src=p/'nyx/nyx_gui/frontend/gui_bluetooth.c';s=src.read_text();a=s.index('static int pairing_file(');b=s.index('static lv_res_t signal(',a)
decl=next(x for x in s.splitlines() if x.startswith('static struct {u8 attempted'))
c=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/types.h>
#include "nova_bt.h"
#undef assert
#define assert(x) do{if(!(x)){fprintf(stderr,"FAIL line%d %s\n",__LINE__,#x);exit(1);}}while(0)
#define FR_OK 0
#define FR_NO_FILE 4
#define FR_EXIST 8
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 8
#define LV_RES_OK 0
typedef int lv_res_t;typedef void lv_obj_t;
static nova_bt_peer paired[2];static struct{char note[512];}ui;
'''+decl+r'''
static nova_bt_status state;const nova_bt_status *nova_bt_get_status(void){return &state;}
static bool gui_bluetooth_active(void){return state.state==NOVA_BT_READY;}
static bool store_write;static int lv_obj_get_free_num(lv_obj_t*b){(void)b;return store_write;}
static const char*nyx_tr(const char*s){return s;}static void refresh(void){}
#define s_printf sprintf
typedef struct{u8 data[100];u32 size;bool present;}file;
static file files[3];typedef struct{unsigned slot;}FIL;
static unsigned mounts,unmounts;static bool mounted,mount_error,mkdir_error,write_error,short_write,close_error,read_error,short_read,rename_error,stat_error,unlink_error,open_error;
static int path_index(const char*p){if(strstr(p,".bin"))return 0;if(strstr(p,".bak"))return 1;assert(strstr(p,".tmp"));return 2;}
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){mounts++;if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){unmounts++;mounted=false;}
static int f_mkdir(const char*p){assert(!strcmp(p,"bootloader/bluetooth"));return mkdir_error?3:FR_EXIST;}
static int f_open(FIL*f,const char*p,int mode){f->slot=path_index(p);if(open_error)return 3;if(mode&FA_WRITE){files[f->slot].present=true;files[f->slot].size=0;return 0;}return files[f->slot].present?0:FR_NO_FILE;}
static u32 f_size(FIL*f){return files[f->slot].size;}
static int f_write(FIL*f,const void*p,u32 n,u32*written){assert(f->slot==2&&n==72);*written=short_write?n-1:n;memcpy(files[2].data,p,*written);files[2].size=*written;return write_error?3:0;}
static int f_read(FIL*f,void*p,u32 n,u32*read){assert(n==72);*read=short_read?n-1:n;memcpy(p,files[f->slot].data,*read);return read_error?3:0;}
static int f_close(FIL*f){(void)f;return close_error?3:0;}
static int f_stat(const char*p,void*info){(void)info;if(stat_error)return 3;return files[path_index(p)].present?0:FR_NO_FILE;}
static int f_unlink(const char*p){unsigned i=path_index(p);if(i==1&&unlink_error)return 3;bool present=files[i].present;memset(&files[i],0,sizeof(file));return present?0:FR_NO_FILE;}
static int f_rename(const char*a,const char*b){unsigned from=path_index(a),to=path_index(b);if(rename_error&&from==2&&to==0)return 3;if(files[to].present)return FR_EXIST;if(!files[from].present)return FR_NO_FILE;files[to]=files[from];memset(&files[from],0,sizeof(file));return 0;}
'''+s[a:b]+r'''
static void reset(void){memset(files,0,sizeof files);memset(paired,0,sizeof paired);memset(&state,0,sizeof state);memset(&import_info,0,sizeof import_info);mounted=mount_error=mkdir_error=write_error=short_write=close_error=read_error=short_read=rename_error=stat_error=unlink_error=open_error=false;mounts=unmounts=0;store_write=false;}
static void fixture(void){for(unsigned i=0;i<2;i++){paired[i].valid=1;paired[i].mac[0]=0x40;paired[i].mac[5]=i+1;paired[i].host[0]=0x60;for(unsigned k=0;k<16;k++)paired[i].key[k]=k+i*32+1;}}
int main(void){
 reset();assert(!pairing_file(true)&&!mounts);
 reset();fixture();assert(pairing_file(true)==2&&files[0].size==72&&!files[2].present&&unmounts==1);
 nova_bt_peer golden[2];memcpy(golden,paired,sizeof golden);memset(paired,0,sizeof paired);assert(pairing_file(false)==2&&!memcmp(paired,golden,sizeof paired));assert(import_info.source==2&&import_info.recognized[0]&&import_info.recognized[1]);
 paired[0].key[0]^=1;assert(pairing_file(true)==2&&files[1].present);files[0].data[30]^=1;memset(paired,0,sizeof paired);assert(pairing_file(false)==2&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();mounted=true;assert(pairing_file(true)==2&&mounted&&!unmounts);
 reset();fixture();mount_error=true;assert(!pairing_file(true)&&!unmounts&&!files[0].present);
 reset();fixture();mkdir_error=true;assert(!pairing_file(true)&&unmounts==1);
 reset();fixture();open_error=true;assert(!pairing_file(true)&&unmounts==1);
 for(unsigned fault=0;fault<6;fault++){
  reset();fixture();assert(pairing_file(true)==2);file old=files[0];paired[0].key[0]^=1;
  switch(fault){case 0:short_write=true;break;case 1:write_error=true;break;case 2:close_error=true;break;case 3:rename_error=true;break;case 4:stat_error=true;break;case 5:unlink_error=true;break;}
  assert(!pairing_file(true)&&!memcmp(&files[0],&old,sizeof old)&&!files[2].present);
 }
 reset();fixture();assert(pairing_file(true)==2);files[0].size--;assert(!pairing_file(false)&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();assert(pairing_file(true)==2);files[0].data[24]^=1;assert(!pairing_file(false)&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();assert(pairing_file(true)==2);read_error=true;assert(!pairing_file(false)&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();assert(pairing_file(true)==2);short_read=true;assert(!pairing_file(false)&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();assert(pairing_file(true)==2);close_error=true;assert(!pairing_file(false)&&!memcmp(paired,golden,sizeof paired));
 reset();fixture();store_write=true;state.state=NOVA_BT_READY;store_pairing(NULL);assert(!mounts&&strstr(ui.note,"Stop Bluetooth"));
 state.state=NOVA_BT_STARTING;store_pairing(NULL);assert(!mounts);
 state.state=NOVA_BT_OFF;store_pairing(NULL);assert(files[0].present&&strstr(ui.note,"saved to SD"));
 memset(paired,0,sizeof paired);store_write=false;store_pairing(NULL);assert(!memcmp(paired,golden,sizeof paired)&&strstr(ui.note,"loaded from SD"));
 puts("PASS: real SD pairing save/load, backup fallback, rename rollback, six write faults, five read faults, exact sizes, mount restoration, RAM preservation and active-radio guard");
}
'''
f=d/'pairing-sd.c';f.write_text(c);cc=w/'toolchains/w64devkit/bin/gcc.exe'
r=subprocess.run([str(cc),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth'),str(f),str(p/'nyx/nyx_gui/bluetooth/nova_bt_protocol.c'),'-o',str(f.with_suffix('.exe'))],capture_output=True,text=True);assert not r.returncode,r.stderr
r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True,timeout=10);assert not r.returncode,r.stderr;print(r.stdout)
(w/'nova31-pairing-sd-check.json').write_text(json.dumps({'passed':True,'physical_card_io':False,'rollback_and_backup':True,'logs':r.stdout,'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest()},indent=2))
