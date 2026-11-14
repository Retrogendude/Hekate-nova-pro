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
typedef struct { int16_t x1,y1,x2,y2; } lv_area_t;
typedef int lv_res_t;
typedef struct { int prio; } lv_task_t;
#define LV_RES_OK 0
#define LV_RES_INV 1
#define LV_TASK_PRIO_OFF 0
#define LV_TASK_PRIO_LOWEST 1
#define LV_HOR_RES 1280
#define LV_VER_RES 720
#define LV_TRIGO_SIN_MAX 32767
#define FX_LINES 8
#define FX_POINTS 17
#define FX_BAND_HEIGHT 64
#define FX_IDLE_MS 900
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
    lv_obj_t *holder;
    lv_point_t points[FX_LINES][FX_POINTS];
    lv_task_t *task;
    u32 phase,effect,speed,opacity,pause_depth,interaction_tick;
    bool frame_ready,interaction_pending;
} background;
static bool mounted,invalid_font;
static int mount_error,stat_error,write_error;
static int mount_calls,unmount_calls,write_calls,reload_calls,notice_calls,options_calls;
static bool options_active,options_saved;
static char notice[256];
static int invalidations;
static u32 fake_tick;
static u32 lv_tick_get(void) { return fake_tick; }
static u32 lv_tick_elaps(u32 previous) { return fake_tick-previous; }
static lv_obj_t objects[FX_LINES];


static u32 nyx_font_count(void) { return 3; }
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
static bool nyx_options_commit_boot(void) { return true; }
static void appearance_notice(const char *s) { ++notice_calls;strcpy(notice,s); }
static void nyx_options_appearance_active(bool active,bool saved) {
    ++options_calls;options_active=active;options_saved=saved;
}
static void reload_nyx(lv_obj_t *o,bool force) { assert(!o&&!force);++reload_calls; }
static lv_res_t nyx_win_close_action(lv_obj_t *o) { (void)o;close_btn=NULL;return LV_RES_INV; }
static void lv_task_set_prio(lv_task_t *task,int priority) { task->prio=priority; }
static void lv_obj_get_coords(lv_obj_t *o,lv_area_t *area) {
    assert(o==background.holder);*area=(lv_area_t){0,LV_VER_RES-FX_BAND_HEIGHT,LV_HOR_RES-1,LV_VER_RES-1};
}
static void lv_inv_area(const lv_area_t *area) {
    assert(area->x1>=-3&&area->x2<LV_HOR_RES+3);
    assert(area->y1>=LV_VER_RES-FX_BAND_HEIGHT&&area->y2<LV_VER_RES);
    assert(area->x2-area->x1<14&&area->y2-area->y1<9);
    ++invalidations;
}
static void lv_obj_invalidate(lv_obj_t *o) { assert(o==background.holder);++invalidations; }
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
    for(int effect=1;effect<=3;effect++)for(int speed=1;speed<=3;speed++)for(int phase=0;phase<360;phase+=15) {
        background.effect=effect;background.speed=speed;background.phase=phase;
        background.frame_ready=false;invalidations=0;
        background_frame(NULL);CHECK(invalidations==(effect==2?8:1));
        for(u32 line=0;line<background_line_count();line++)for(int point=0;point<(effect==2?2:FX_POINTS);point++) {
            assert(background.points[line][point].x>=0&&background.points[line][point].x<LV_HOR_RES);
            assert(background.points[line][point].y>=0&&background.points[line][point].y<FX_BAND_HEIGHT);
        }
        invalidations=0;background_frame(NULL);CHECK(invalidations==(effect==2?16:1));
    }
    u32 phase=background.phase;nyx_background_pause(true);nyx_background_pause(true);
    CHECK(background.pause_depth==2&&task.prio==LV_TASK_PRIO_OFF);
    invalidations=0;background_frame(NULL);CHECK(background.phase==phase&&invalidations==0);
    nyx_background_pause(false);CHECK(task.prio==LV_TASK_PRIO_OFF&&background.pause_depth==1);
    nyx_background_pause(false);nyx_background_pause(false);CHECK(task.prio==LV_TASK_PRIO_LOWEST&&!background.pause_depth);
    fake_tick=1000;nyx_background_interaction();CHECK(background.interaction_pending&&background.interaction_tick==1000);
    fake_tick=1899;phase=background.phase;invalidations=0;background_frame(NULL);
    CHECK(background.phase==phase&&invalidations==0);
    fake_tick=1900;background_frame(NULL);CHECK(background.phase!=phase&&!background.interaction_pending&&invalidations==1);
    fake_tick=UINT32_MAX-99;nyx_background_interaction();fake_tick=799;phase=background.phase;invalidations=0;background_frame(NULL);
    CHECK(background.phase==phase&&invalidations==0);
    fake_tick=800;background_frame(NULL);CHECK(background.phase!=phase&&!background.interaction_pending&&invalidations==1);
    background.effect=0;invalidations=0;background_frame(NULL);CHECK(invalidations==0);
    printf("PASS: %d actual-C appearance cases; pending/cancel, font/SD preflight, rollback/save, and three bounded background geometries, nested pause and wrap-safe idle scheduling.\n",cases);
    return 0;
}
'''

functions='\n'.join(function(name) for name in (
    'bounded','background_line_count','background_particle_invalidate','background_frame','nyx_background_pause','nyx_background_interaction','appearance_close',
    'appearance_set_selection','appearance_apply','appearance_save'))
directory=WORK/'appearance-actions-check'
directory.mkdir(exist_ok=True)
harness=directory/'check.c'
harness.write_text(preamble+'\n'+functions+'\nstatic lv_res_t appearance_select(lv_obj_t *o) { return appearance_set_selection(o->id, o->selected); }\n'+tests,encoding='utf-8')
compiler=WORK/'toolchains/w64devkit/bin/gcc.exe'
executable=directory/'check.exe'
subprocess.run([str(compiler),'-std=c11','-Wall','-Wextra','-Werror','-O2',
    '-I'+str(PROJECT/'bdk'),str(harness),str(PROJECT/'bdk/libs/lvgl/lv_misc/lv_math.c'),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
