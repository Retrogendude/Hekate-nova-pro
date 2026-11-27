from pathlib import Path
import subprocess,json,re
w=Path('work').resolve();p=w/'hekate-nova-pro-v34-alpha6';d=w/'nova34a6-tv-tests'
def fn(s,name):
 m=re.search(r'^(?:static )?(?:void|u8) '+name+r'\([^;]*?\)\s*\n\{',s,re.M);assert m,name
 a=m.start();b=s.index('\n}',m.end())+2;return s[a:b]
gui=(p/'nyx/nyx_gui/frontend/gui.c').read_text();btn=(p/'bdk/utils/btn.c').read_text()
c=r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef unsigned u32;typedef unsigned char u8;typedef int lv_task_prio_t;
typedef struct{u32 last_run,period;void(*task)(void*);void*param;}lv_task_t;
typedef struct{lv_task_t*tasks[2];}system_maintenance_tasks_t;
static system_maintenance_tasks_t system_tasks;static bool diagnostics_suspended,display_frame_dirty,display_frame_ready,disp_init_done;
static int tv,bt,refresh,compose,init,order;static u32 now;
static u32 lv_tick_get(void){return now;}static u32 lv_tick_elaps(u32 t){return now-t;}
static void gui_bluetooth_poll_fast(void){bt++;}static void gui_bluetooth_draw_poll(void){}
static void lv_refr_now(void){refresh++;order=1;}static void gui_tv_service(void){tv++;assert(order==0||order==1);}
static void gui_tv_present(void){tv++;assert(order==2);}static void vic_compose_poll(void(*f)(void)){compose++;order=2;}
static void _nyx_disp_init(void){init++;order=2;}
'''+fn(gui,'manual_system_maintenance')+'\n'+fn(gui,'_disp_fb_present')+r'''
static int waits,reads;static u8 buttons[8];
#define BTN_POWER 1
#define BTN_SINGLE 128
static void btn_wait_service(void){waits++;}
static u8 btn_read(void){return buttons[reads++%8];}
static u32 get_tmr_ms(void){return now++;}
'''+fn(btn,'btn_wait')+'\n'+fn(btn,'btn_wait_timeout')+'\n'+fn(btn,'btn_wait_timeout_single')+r'''
int main(void){
 manual_system_maintenance(false);assert(tv==1&&bt==1&&!refresh);
 manual_system_maintenance(true);assert(tv==2&&bt==2&&refresh==1);diagnostics_suspended=true;
 manual_system_maintenance(true);assert(tv==2&&bt==2&&refresh==2);diagnostics_suspended=false;
 display_frame_dirty=true;disp_init_done=true;_disp_fb_present(0,0);assert(tv==3&&compose==1&&!display_frame_dirty);
 display_frame_dirty=true;display_frame_ready=true;disp_init_done=false;_disp_fb_present(0,0);assert(tv==4&&init==1&&disp_init_done);
 order=0;buttons[0]=buttons[1]=0;buttons[2]=1;assert(btn_wait()==1&&waits==2);
 reads=waits=0;now=0;buttons[2]=2;assert(btn_wait_timeout(100,2)==2&&waits>0);
 reads=waits=0;now=0;assert(btn_wait_timeout_single(100,2)==2&&waits>0);
 puts("PASS: actual maintenance/frame-present/button-wait functions pump TV; diagnostics guard preserved; no LVGL scheduler reentry.");
}
'''
(d/'integration.c').write_text(c)
r=subprocess.run([str(w/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2',str(d/'integration.c'),'-o',str(d/'integration.exe')],capture_output=True,text=True);assert not r.returncode,r.stderr
r=subprocess.run([str(d/'integration.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
for name in ('frontend/fe_emummc_tools.c','frontend/gui_tools_partition_manager.c'):
 assert 'manual_system_maintenance(true)'in(p/'nyx/nyx_gui'/name).read_text()
(w/'nova34a6-tv-integration.json').write_text(json.dumps({'passed':True,'actual_c':True,'physical_hardware':False,'result':r.stdout.strip()},indent=2))
