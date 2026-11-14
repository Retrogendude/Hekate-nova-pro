"""Exercise Nyx's real display batching against counted hardware operations."""
from pathlib import Path
import subprocess

WORK = Path(__file__).resolve().parent
source = (WORK/'hekate-6.5.4/nyx/nyx_gui/frontend/gui.c').read_text(encoding='utf-8')
assert 'lv_refr_set_monitor_cb(_disp_fb_present);' in source
driver = source[source.index('static bool display_frame_dirty;'):source.index('static touch_event_t touchpad;')]
preamble = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef uint32_t u32;
typedef struct {u32 full;} lv_color_t;
#define NYX_FB2_ADDRESS 0x12340000
static bool disp_init_done;
static u32 writes,composes,waits,ready,initializations,order;
static int vic_wait_idle(void){assert(!order);order=1;++waits;return 0;}
static int vic_compose(void){++composes;return 0;}
static void _nyx_disp_init(void){++initializations;++composes;}
static void lv_flush_ready(void){++ready;}
static void gfx_set_rect_pitch(u32*target,u32*source,u32 pitch,int x1,int y1,int x2,int y2){
 (void)source;assert(target==(u32*)NYX_FB2_ADDRESS&&pitch==1280);
 assert(x1>=0&&y1>=0&&x2<1280&&y2<720&&x2>=x1&&y2>=y1);++writes;order=2;
}
'''
tests = r'''
int main(void){unsigned checks=0;
#define CHECK(c)do{++checks;assert(c);}while(0)
 _disp_fb_present(0,0);CHECK(!composes&&!waits&&!initializations);
 _disp_fb_flush(0,0,10,10,NULL);CHECK(writes==1&&ready==1&&!disp_init_done);
 _disp_fb_present(0,121);CHECK(!composes&&!initializations&&!display_frame_dirty);
 order=0;_disp_fb_flush(0,0,1279,20,NULL);_disp_fb_flush(2,30,20,40,NULL);
 CHECK(!disp_init_done&&!composes&&writes==3);
 _disp_fb_present(0,30000);CHECK(disp_init_done&&initializations==1&&composes==1&&!waits);
 for(unsigned regions=1;regions<=32;regions++){
  u32 old_writes=writes,old_ready=ready,old_waits=waits,old_composes=composes;order=0;
  for(unsigned region=0;region<regions;region++){
   _disp_fb_flush(region,100,region+3,103,NULL);
   CHECK(waits==old_waits+1&&composes==old_composes);
   CHECK(writes==old_writes+region+1&&ready==old_ready+region+1);
  }
  _disp_fb_present(0,regions*16);CHECK(composes==old_composes+1&&!display_frame_dirty);
  _disp_fb_present(0,0);CHECK(composes==old_composes+1&&waits==old_waits+1);
 }
 CHECK(initializations==1);
 printf("PASS: %u display checks; 1-32 dirty regions copied completely, one presentation per refresh, prior frame waited before writes, initial display transition retained.\n",checks);
 return 0;
}
'''
directory=WORK/'display-present-check';directory.mkdir(exist_ok=True)
path=directory/'check.c';path.write_text(preamble+driver+tests,encoding='utf-8')
executable=directory/'check.exe'
subprocess.run([str(WORK/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-Wall','-Wextra','-Werror',str(path),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
