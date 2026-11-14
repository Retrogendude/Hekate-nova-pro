from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32';d=w/'nova32-display-tests';d.mkdir(exist_ok=True)
def block(path,start,end):
 s=(p/path).read_text();return s[s.index(start):s.index(end,s.index(start))]
body=block('bdk/display/vic.c','int vic_wait_idle_poll(', '\nint vic_compose()')
body+=block('nyx/nyx_gui/frontend/gui.c','static void _disp_fb_flush(', '\nstatic touch_event_t touchpad;')
test=r"""
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef uint32_t u32;
typedef struct{u32 full;}lv_color_t;
static u32 now,until,start,max_gap,last_service,services,composes,fast,ready,tv_calls,init_calls;
static bool forever;
static u32 get_tmr_us(void){return now;}
static void usleep(u32 n){now+=n;}
#define PVIC_FALCON_IDLESTATE 1
#define VIC_FC_COMPOSE 2
#define COMPOSE_START 3
static int busy(void){return forever||(u32)(now-start)<until;}
#define VIC(x) busy()
static void _vic_write_priv(u32 address,u32 value){assert(address==2&&value==3);composes++;}
static void gui_bluetooth_draw_poll(void){u32 gap=now-last_service;if(gap>max_gap)max_gap=gap;last_service=now;services++;now+=5;}
static void gui_bluetooth_poll_fast(void){fast++;}
static u32 framebuffer[1280*720];
#define NYX_FB2_ADDRESS framebuffer
static bool display_frame_dirty,disp_init_done,display_frame_ready;
static void gfx_set_rect_pitch(u32 *fb,const u32 *src,u32 pitch,int x1,int y1,int x2,int y2){for(int y=y1;y<=y2;y++)memcpy(fb+y*pitch+x1,src+(y-y1)*(x2-x1+1),(x2-x1+1)*4);}
static void gui_tv_dirty(int first,int last){assert(first<=last);tv_calls++;}
static void lv_flush_ready(void){ready++;}
static void _nyx_disp_init(void){init_calls++;}
"""+body+r"""
int main(void){
 start=now=last_service=0;until=50000;assert(!vic_wait_idle_poll(gui_bluetooth_draw_poll));assert(services>70&&max_gap<=650&&now>=50000&&now<51000);
 start=now=last_service=0xffff0000u;until=0;forever=true;max_gap=services=0;assert(vic_wait_idle_poll(gui_bluetooth_draw_poll)==1);assert((u32)(now-start)>=150000&&(u32)(now-start)<151000&&max_gap<=650);
 forever=false;until=0;assert(!vic_compose_poll(gui_bluetooth_draw_poll)&&composes==1);
 static u32 src[1280*720];for(unsigned i=0;i<1280*720;i++)src[i]=i*65537u+17;
 disp_init_done=true;services=0;_disp_fb_flush(0,0,1279,719,(lv_color_t*)src);assert(services==45&&fast==1&&ready==1&&tv_calls==1);assert(!memcmp(src,framebuffer,sizeof src));assert(display_frame_dirty&&display_frame_ready);
 _disp_fb_present(0,0);assert(composes==2&&!display_frame_dirty&&!display_frame_ready);_disp_fb_present(0,0);assert(composes==2);
 memset(framebuffer,0,sizeof framebuffer);_disp_fb_flush(13,27,117,77,(lv_color_t*)src);unsigned n=0;
 for(unsigned y=0;y<720;y++)for(unsigned x=0;x<1280;x++)assert(framebuffer[y*1280+x]==(x>=13&&x<=117&&y>=27&&y<=77?src[n++]:0));
 assert(!display_frame_ready);_disp_fb_present(0,0);assert(composes==3);
 disp_init_done=false;_disp_fb_flush(0,0,10,0,(lv_color_t*)src);_disp_fb_present(0,0);assert(!disp_init_done&&!init_calls);_disp_fb_flush(0,0,1279,0,(lv_color_t*)src);_disp_fb_present(0,0);assert(disp_init_done&&init_calls==1);
 puts("PASS actual VIC: idle/busy/150-ms timeout and wrap with <=650-us service gap; exact full/partial 16-row framebuffer copies, one composition per frame, TV dirty and startup preserved");
}
"""
(d/'display.c').write_text(test)
r=subprocess.run([str(w/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-O1','-Wall','-Werror',str(d/'display.c'),'-o',str(d/'display.exe')],capture_output=True,text=True);assert r.returncode==0,r.stderr
r=subprocess.run([str(d/'display.exe')],capture_output=True,text=True,timeout=10);print(r.stdout,r.stderr);assert r.returncode==0,r.returncode
(w/'nova32-display-check.json').write_text(json.dumps({'passed':True,'output':r.stdout,'clock_and_video_hardware':'substituted','source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['bdk/display/vic.c','bdk/display/vic.h','nyx/nyx_gui/frontend/gui.c']}},indent=2))
