"""Actual boot hook, SD decoder/loader and startup policy; no console I/O."""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v30';d=w/'nova30-tests';d.mkdir(exist_ok=True)
gui=(p/'nyx/nyx_gui/frontend/gui.c').read_text();bt=(p/'nyx/nyx_gui/frontend/gui_bluetooth.c').read_text();bdk=(p/'bdk/input/joycon.c').read_text()
def fun(s,start):
 a=s.index(start);return s[a:s.index('\n}',a)+2]
prefix=(d/'pairing-sd.c').read_text();prefix=prefix[:prefix.index('int main(void)')]
prefix=prefix.replace('static struct{char note[512];}ui;', 'static struct{char note[512];void *task;}ui;')
c=prefix+r'''
static bool boot_checked;static u8 boot_status;
static struct{int jc_disable;}n_cfg;static int hw_type,rail[2],rail_calls,hw_ready,init_calls,start_calls,task_creates,task_deletes,alloc_error,start_error;
#define FUSE_NX_HW_TYPE_HOAG 4
#define LV_TASK_PRIO_HIGH 3
static int fuse_read_hw_type(void){return hw_type;}
int jc_get_attached_mask(void){assert(hw_ready);int at=rail_calls++?1:0;return rail[at];}
void jc_init_hw(void){hw_ready=true;init_calls++;}
static void tick(void*p){(void)p;}
typedef struct{u32 last_run,period;void(*task)(void*);void*param;}lv_task_t;
typedef struct{lv_task_t*tasks[2];}system_maintenance_tasks_t;
static system_maintenance_tasks_t system_tasks;
static bool diagnostics_suspended;static unsigned fast_polls,reframes,maintained;static u32 test_tick=100;
static u32 lv_tick_get(void){return test_tick;}
static u32 lv_tick_elaps(u32 last){return test_tick-last;}
static void lv_refr_now(void){reframes++;}
static void gui_bluetooth_poll_fast(void){fast_polls++;}
static void maintenance_task(void*p){(void)p;maintained++;}
static void *lv_task_create(void(*fn)(void*),unsigned period,unsigned prio,void*p){(void)p;assert(fn==tick&&period==2&&prio==LV_TASK_PRIO_HIGH);task_creates++;return alloc_error?NULL:(void*)1;}
static void lv_task_del(void*p){assert(p);task_deletes++;}
int nova_bt_start(const nova_bt_peer p[2]){assert(nova_bt_validate_peers(p));start_calls++;state.state=start_error?NOVA_BT_ERROR:NOVA_BT_STARTING;return !start_error;}
void nova_bt_stop(void){state.state=NOVA_BT_OFF;}
'''+fun(bt,'void gui_bluetooth_shutdown(')+'\n'+fun(bt,'static lv_res_t start(')+'\n'+fun(bt,'void gui_bluetooth_autostart(')+'\n'+fun(gui,'static void _joycon_boot_init(')+'\n'+fun(gui,'void manual_system_maintenance(')+r'''
static void setup(void){
 reset();fixture();assert(pairing_file(true)==2);memset(paired,0,sizeof paired);memset(&import_info,0,sizeof import_info);memset(&ui,0,sizeof ui);
 boot_checked=false;boot_status=0;hw_type=n_cfg.jc_disable=0;rail[0]=rail[1]=0;rail_calls=0;hw_ready=1;init_calls=start_calls=task_creates=task_deletes=alloc_error=start_error=0;mounts=unmounts=0;
}
int main(void){
 setup();hw_ready=0;file before[3];memcpy(before,files,sizeof files);_joycon_boot_init(NULL);
 assert(init_calls==1&&hw_ready&&start_calls==1&&task_creates==1&&state.state==NOVA_BT_STARTING&&boot_status==2&&import_info.source==2);
 assert(paired[0].valid&&paired[1].valid&&!memcmp(before,files,sizeof files));
 gui_bluetooth_autostart();assert(start_calls==1);gui_bluetooth_shutdown();gui_bluetooth_autostart();assert(state.state==NOVA_BT_OFF&&start_calls==1);
 for(unsigned mask=1;mask<=3;mask++){
  setup();rail[0]=rail[1]=mask;gui_bluetooth_autostart();assert(boot_status==1&&!mounts&&!start_calls&&!task_creates&&!paired[0].valid&&!paired[1].valid);
  rail[0]=rail[1]=0;gui_bluetooth_autostart();assert(!start_calls&&!mounts);
 }
 setup();rail[0]=-1;gui_bluetooth_autostart();assert(boot_status==5&&!mounts&&!start_calls);
 setup();n_cfg.jc_disable=1;gui_bluetooth_autostart();assert(boot_status==4&&!rail_calls&&!mounts&&!start_calls);
 setup();hw_type=FUSE_NX_HW_TYPE_HOAG;gui_bluetooth_autostart();assert(boot_status==4&&!rail_calls&&!mounts&&!start_calls);
 setup();memset(files,0,sizeof files);gui_bluetooth_autostart();assert(boot_status==3&&!start_calls&&!state.state&&!state.error);
 setup();files[0].data[40]^=1;gui_bluetooth_autostart();assert(boot_status==3&&!start_calls&&!paired[0].valid);
 setup();files[1]=files[0];files[0].data[40]^=1;gui_bluetooth_autostart();assert(boot_status==2&&start_calls==1);
 setup();mount_error=1;gui_bluetooth_autostart();assert(boot_status==3&&!start_calls&&!unmounts);
 setup();rail[1]=2;gui_bluetooth_autostart();assert(boot_status==1&&!start_calls&&!paired[0].valid&&!import_info.attempted);
 setup();rail[1]=-1;gui_bluetooth_autostart();assert(boot_status==5&&!start_calls&&!paired[1].valid);
 setup();start_error=1;gui_bluetooth_autostart();assert(boot_status==7&&start_calls==1&&task_deletes==1&&!ui.task);
 setup();alloc_error=1;gui_bluetooth_autostart();assert(boot_status==7&&!start_calls&&task_creates==1&&!ui.task);
 setup();state.state=NOVA_BT_READY;gui_bluetooth_autostart();assert(boot_status==6&&!start_calls&&!mounts);
 setup();gui_bluetooth_shutdown();gui_bluetooth_autostart();assert(!start_calls&&!mounts);
 lv_task_t upkeep={0,50,maintenance_task,NULL};system_tasks.tasks[0]=&upkeep;
 manual_system_maintenance(false);assert(fast_polls==1&&maintained==1&&!reframes);
 manual_system_maintenance(true);assert(fast_polls==2&&maintained==1&&reframes==1);
 diagnostics_suspended=true;manual_system_maintenance(true);assert(fast_polls==2&&maintained==1&&reframes==2);
 puts("PASS: boot background start, no window, physical rails 0/1/2/3, init order, once-only Stop, Lite/disabled/unknown, missing/corrupt/backup SD, attach race, allocation and radio failures");
}
'''
cc=w/'toolchains/w64devkit/bin/gcc.exe';args=[str(cc),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-Wno-unused-function','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth')]
f=d/'autostart.c';f.write_text(c)
r=subprocess.run([*args,str(f),str(p/'nyx/nyx_gui/bluetooth/nova_bt_protocol.c'),'-o',str(f.with_suffix('.exe'))],capture_output=True,text=True);assert not r.returncode,r.stderr
r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True,timeout=10);assert not r.returncode,r.stderr;print(r.stdout)
rail=r'''
#include <stdio.h>
#include <stdlib.h>
#define assert(x) do{if(!(x)){fprintf(stderr,"FAIL %s\n",#x);exit(1);}}while(0)
static int jc_init_done,detects,l,r;static struct{int sio_mode;}jc_gamepad;static struct{int detected;}jc_l,jc_r;
static void _jc_rail_detect(void){detects++;jc_l.detected=l;jc_r.detected=r;}
'''+fun(bdk,'int jc_get_attached_mask(')+r'''
int main(void){assert(jc_get_attached_mask()==-1&&!detects);jc_init_done=1;for(int mask=0;mask<4;mask++){l=mask&1;r=mask&2;assert(jc_get_attached_mask()==mask);}assert(detects==4);jc_gamepad.sio_mode=1;assert(jc_get_attached_mask()==-1&&detects==4);puts("PASS: actual BDK rail presence helper independent of UART connection handshake");}
'''
f=d/'rail-presence.c';f.write_text(rail)
subprocess.run([str(cc),'-std=gnu11','-O1',str(f),'-o',str(f.with_suffix('.exe'))],check=True)
r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True);assert not r.returncode,r.stderr;print(r.stdout)
(w/'nova30-autostart-check.json').write_text(json.dumps({'passed':True,'physical_radio_or_card':False,'boot_without_window':True,'once_only':True,'rail_masks_tested':[0,1,2,3],'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['bdk/input/joycon.c','bdk/input/joycon.h','nyx/nyx_gui/frontend/gui.c','nyx/nyx_gui/frontend/gui_bluetooth.c','nyx/nyx_gui/frontend/gui_bluetooth.h']}},indent=2))
