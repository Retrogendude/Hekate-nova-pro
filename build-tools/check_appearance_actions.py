"""Exercise real appearance callbacks and background geometry with a fake SD/UI."""
from pathlib import Path
import re
import subprocess

WORK=Path(__file__).resolve().parent
PROJECT=WORK/'hekate-6.5.4'
SOURCE=(PROJECT/'nyx/nyx_gui/frontend/gui_appearance.c').read_text(encoding='utf-8')
CONFIG=(PROJECT/'nyx/nyx_gui/config.h').read_text(encoding='utf-8')

def function(name):
    found=re.search(r'^(?:static )?(?:u32|void|lv_res_t) '+name+r'\([^\n]*\)\s*\n\{',SOURCE,re.M)
    assert found,name
    end=re.search(r'^\}',SOURCE[found.end():],re.M)
    assert end,name
    return SOURCE[found.start():found.end()+end.end()]

config=re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;',CONFIG,re.S).group()
preamble=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef struct { int selected,id; } lv_obj_t;
typedef struct { int16_t x,y; } lv_point_t;
typedef int lv_res_t;
typedef struct { int prio; } lv_task_t;
#define LV_RES_OK 0
#define LV_RES_INV 1
#define LV_TASK_PRIO_OFF 0
#define LV_TASK_PRIO_LOWEST 1
#define LV_HOR_RES 1280
#define LV_VER_RES 720
#define LV_TRIGO_SIN_MAX 32767
#define FX_LINES 20
#define FX_POINTS 49
#define I18N_EN 0
#define I18N_LANGUAGE_COUNT 7
#define HOME_LAYOUT_TILES 0
#define HOME_LAYOUT_CAROUSEL 1
#define FR_OK 0
int16_t lv_trigo_sin(int16_t angle);
'''+config+r'''
static nyx_config n_cfg,appearance_pending,saved_config;
static lv_obj_t *appearance_parent_close,*close_btn;
static bool appearance_open;
enum { AP_LANGUAGE, AP_FONT, AP_SIZE, AP_LAYOUT, AP_EFFECT, AP_SPEED, AP_OPACITY };
static struct {
    lv_obj_t *holder,*lines[FX_LINES];
    lv_point_t points[FX_LINES][FX_POINTS];
    lv_task_t *task;
    u32 phase,effect,speed,opacity,pause_depth;
} background;
static bool mounted,invalid_font;
static int mount_error,stat_error,write_error;
static int mount_calls,unmount_calls,write_calls,reload_calls,notice_calls,options_calls;
static bool options_active,options_saved;
static char notice[256];
static int line_calls,point_counts[FX_LINES];
static int line_x[FX_LINES],line_y[FX_LINES],line_w[FX_LINES],line_h[FX_LINES];
static lv_obj_t objects[FX_LINES];
static int lv_ddlist_get_selected(lv_obj_t *o) { return o->selected; }
static int lv_obj_get_free_num(lv_obj_t *o) { return o->id; }
static const char *nyx_font_filename(u32 i) {
    static const char *names[]={"","NotoSans.nyxf","Readable.nyxf"};
    return i<3?names[i]:NULL;
}
static bool sd_get_card_mounted(void) { return mounted; }
static int sd_mount(void) { ++mount_calls; if(!mount_error)mounted=true;return mount_error; }
static void sd_unmount(void) { ++unmount_calls;mounted=false; }
static int f_stat(const char *p,void *unused) {
    (void)unused; assert(!strcmp(p,"bootloader/sys/nyx.bin"));return stat_error;
}
static bool nyx_font_available_at_size(const char *p, u32 size) { assert(p[0]);assert(size >= 20 && size <= 36);return !invalid_font; }
static int create_nyx_config_entry(bool force) {
    assert(force);++write_calls;saved_config=n_cfg;sd_unmount();return write_error;
}
static void appearance_notice(const char *s) { ++notice_calls;strcpy(notice,s); }
static void nyx_options_appearance_active(bool active,bool saved) {
    ++options_calls;options_active=active;options_saved=saved;
}
static void reload_nyx(lv_obj_t *o,bool force) { assert(!o&&!force);++reload_calls; }
static lv_res_t nyx_win_close_action(lv_obj_t *o) { (void)o;close_btn=NULL;return LV_RES_INV; }
static void lv_task_set_prio(lv_task_t *task,int priority) { task->prio=priority; }
static void lv_obj_set_pos(lv_obj_t *o,int x,int y) { line_x[o->id]=x;line_y[o->id]=y; }
static void lv_obj_set_size(lv_obj_t *o,int w,int h) { assert(w>0&&h>0);line_w[o->id]=w;line_h[o->id]=h; }
static void lv_line_set_points(lv_obj_t *line,const lv_point_t *p,int count) {
    int id=line->id; assert(id>=0&&id<FX_LINES);assert(p==background.points[id]);
    ++line_calls;point_counts[id]=count;
    for(int i=0;i<count;i++) {
        assert(p[i].x>=0&&p[i].x<line_w[id]);
        assert(p[i].y>=0&&p[i].y<line_h[id]);
        assert(p[i].x+line_x[id]>=0&&p[i].x+line_x[id]<=LV_HOR_RES);
        assert(p[i].y+line_y[id]>=0&&p[i].y+line_y[id]<=LV_VER_RES);
    }
    if(background.effect==2)assert(line_w[id]*line_h[id]<300);
}
static void reset(void) {
    memset(&n_cfg,0,sizeof(n_cfg));n_cfg.language=1;n_cfg.theme_bg=0x123456;
    n_cfg.font_size=30;n_cfg.background_speed=2;n_cfg.background_opacity=20;
    appearance_pending=n_cfg;strcpy(appearance_pending.font_file,"NotoSans.nyxf");
    appearance_pending.language=4;appearance_pending.font_size=36;
    appearance_pending.background_effect=3;appearance_pending.background_speed=1;
    appearance_pending.background_opacity=50;appearance_pending.home_layout=1;
    mounted=false;invalid_font=false;mount_error=stat_error=write_error=0;
    mount_calls=unmount_calls=write_calls=reload_calls=notice_calls=options_calls=0;
    options_active=options_saved=false;notice[0]=0;
}
'''
tests=r'''
int main(void) {
    int cases=0;
    #define CHECK(x) do { assert(x);++cases; } while(0)
    for(int i=0;i<7;i++) {
        reset();lv_obj_t list={i,AP_LANGUAGE};appearance_select(&list);
        CHECK(appearance_pending.language==(u32)i&&n_cfg.language==1);
    }
    reset();lv_obj_t list={99,AP_LANGUAGE};appearance_select(&list);
    CHECK(appearance_pending.language==0&&n_cfg.language==1);
    for(int i=0;i<9;i++) {
        list=(lv_obj_t){i,AP_SIZE};appearance_select(&list);
        CHECK(appearance_pending.font_size==20+(u32)i*2&&n_cfg.font_size==30);
    }
    list=(lv_obj_t){99,AP_SIZE};appearance_select(&list);CHECK(appearance_pending.font_size==30);
    list=(lv_obj_t){2,AP_FONT};appearance_select(&list);CHECK(!strcmp(appearance_pending.font_file,"Readable.nyxf"));
    list=(lv_obj_t){99,AP_FONT};appearance_select(&list);CHECK(!strcmp(appearance_pending.font_file,"Readable.nyxf"));
    for(int i=0;i<4;i++) { list=(lv_obj_t){i,AP_EFFECT};appearance_select(&list);CHECK(appearance_pending.background_effect==(u32)i); }
    list=(lv_obj_t){99,AP_EFFECT};appearance_select(&list);CHECK(appearance_pending.background_effect==0);
    for(int i=0;i<3;i++) { list=(lv_obj_t){i,AP_SPEED};appearance_select(&list);CHECK(appearance_pending.background_speed==(u32)i+1); }
    list=(lv_obj_t){99,AP_SPEED};appearance_select(&list);CHECK(appearance_pending.background_speed==2);
    for(int i=0;i<5;i++) { list=(lv_obj_t){i,AP_OPACITY};appearance_select(&list);CHECK(appearance_pending.background_opacity==((u32)i+1)*10); }
    list=(lv_obj_t){99,AP_OPACITY};appearance_select(&list);CHECK(appearance_pending.background_opacity==20);
    list=(lv_obj_t){1,AP_LAYOUT};appearance_select(&list);CHECK(appearance_pending.home_layout==1);
    list=(lv_obj_t){99,AP_LAYOUT};appearance_select(&list);CHECK(appearance_pending.home_layout==0);
    reset();appearance_open=true;appearance_parent_close=&objects[1];close_btn=&objects[2];
    nyx_config before=n_cfg;CHECK(appearance_close(NULL)==LV_RES_INV);
    CHECK(!appearance_open&&close_btn==&objects[1]&&!appearance_parent_close);
    CHECK(!memcmp(&before,&n_cfg,sizeof(before))&&write_calls==0&&!options_active&&!options_saved);
    reset();mount_error=1;appearance_save(NULL);
    CHECK(write_calls==0&&reload_calls==0&&notice_calls==1&&n_cfg.language==1);
    for(int was_mounted=0;was_mounted<2;was_mounted++) {
        reset();mounted=was_mounted;stat_error=1;appearance_save(NULL);
        CHECK(write_calls==0&&reload_calls==0&&notice_calls==1&&n_cfg.language==1);
        CHECK(mounted==!!was_mounted&&unmount_calls==!was_mounted);
        reset();mounted=was_mounted;invalid_font=true;appearance_save(NULL);
        CHECK(write_calls==0&&reload_calls==0&&notice_calls==1&&n_cfg.language==1);
        CHECK(mounted==!!was_mounted&&unmount_calls==!was_mounted);
    }
    reset();before=n_cfg;write_error=1;appearance_save(NULL);
    CHECK(write_calls==1&&reload_calls==0&&notice_calls==1&&options_calls==0);
    CHECK(!memcmp(&before,&n_cfg,sizeof(before)));
    reset();appearance_save(NULL);
    CHECK(write_calls==1&&reload_calls==1&&notice_calls==1&&options_active&&options_saved);
    CHECK(n_cfg.language==4&&n_cfg.font_size==36&&!strcmp(n_cfg.font_file,"NotoSans.nyxf"));
    CHECK(n_cfg.background_effect==3&&n_cfg.background_speed==1&&n_cfg.background_opacity==50);
    CHECK(n_cfg.theme_bg==0x123456&&saved_config.language==4&&saved_config.theme_bg==0x123456);
    reset();invalid_font=true;appearance_pending.font_file[0]=0;appearance_save(NULL);
    CHECK(write_calls==1&&reload_calls==1&&!n_cfg.font_file[0]);
    CHECK(bounded(9,10,50,20)==20&&bounded(51,10,50,20)==20&&bounded(50,10,50,20)==50);
    memset(&background,0,sizeof(background));lv_task_t task={1};
    background.holder=&objects[0];background.task=&task;
    for(int i=0;i<FX_LINES;i++){objects[i].id=i;background.lines[i]=&objects[i];}
    for(int effect=1;effect<=3;effect++)for(int speed=1;speed<=3;speed++)for(int phase=0;phase<360;phase+=15) {
        background.effect=effect;background.speed=speed;background.phase=phase;line_calls=0;
        background_frame(NULL);CHECK(line_calls==(effect==2?20:effect==3?7:4));
        CHECK(point_counts[0]==(effect==2?2:FX_POINTS));
    }
    u32 phase=background.phase;nyx_background_pause(true);nyx_background_pause(true);
    CHECK(background.pause_depth==2&&task.prio==LV_TASK_PRIO_OFF);
    line_calls=0;background_frame(NULL);CHECK(background.phase==phase&&line_calls==0);
    nyx_background_pause(false);CHECK(task.prio==LV_TASK_PRIO_OFF&&background.pause_depth==1);
    nyx_background_pause(false);nyx_background_pause(false);CHECK(task.prio==LV_TASK_PRIO_LOWEST&&!background.pause_depth);
    background.effect=0;line_calls=0;background_frame(NULL);CHECK(line_calls==0);
    printf("PASS: %d actual-C appearance cases; pending/cancel, font/SD preflight, rollback/save, and three bounded background geometries with nested pause.\n",cases);
    return 0;
}
'''

functions='\n'.join(function(name) for name in (
    'bounded','background_frame','nyx_background_pause','appearance_close',
    'appearance_select','appearance_apply','appearance_save'))
directory=WORK/'appearance-actions-check'
directory.mkdir(exist_ok=True)
harness=directory/'check.c'
harness.write_text(preamble+'\n'+functions+'\n'+tests,encoding='utf-8')
compiler=WORK/'toolchains/w64devkit/bin/gcc.exe'
executable=directory/'check.exe'
subprocess.run([str(compiler),'-std=c11','-Wall','-Wextra','-Werror','-O2',
    '-I'+str(PROJECT/'bdk'),str(harness),str(PROJECT/'bdk/libs/lvgl/lv_misc/lv_math.c'),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
