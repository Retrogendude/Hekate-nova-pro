"""Headless rendering of the actual LVGL 5 code, card builders and Appearance UI.

Only platform I/O and hardware actions are substituted. Fonts come from the
real SD NYXF loader; icons come from the unmodified official res.pak.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from zipfile import ZipFile
import json
import re
import subprocess
import sys
from PIL import Image

work=Path(__file__).resolve().parent
project=work/'hekate-6.5.4'
directory=work/'lvgl-preview-v9';directory.mkdir(exist_ok=True)
lvgl=project/'bdk/libs/lvgl'
config=(project/'bdk/libs/lv_conf.h').read_text()
config=config.replace('#include <soc/timer.h>','#include <utils/types.h>').replace('#include <memory_map.h>','')
config=config.replace('#define LV_MEM_CUSTOM         0','#define LV_MEM_CUSTOM         1').replace('#  define LV_MEM_CUSTOM_INCLUDE <mem/heap.h>','#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>')
config=config.replace('#define LV_VDB_ADR          NYX_LV_VDB_ADR','#define LV_VDB_ADR          0').replace('#define LV_TICK_CUSTOM               1','#define LV_TICK_CUSTOM               0')
(directory/'lv_conf.h').write_text(config)
with ZipFile(work/'hekate_ctcaer_6.5.4_Nyx_1.9.4.zip')as z:(directory/'res.pak').write_bytes(z.read('bootloader/sys/res.pak'))

def stripped(path,keep_catalog=False):
    s=path.read_text(encoding='utf-8')
    return re.sub(r'^#include(?! "localization_catalog.inc")[^\n]*\n' if keep_catalog else r'^#include[^\n]*\n','',s,flags=re.M)

nyx=project/'nyx/nyx_gui'
cfg=(nyx/'config.h').read_text();cfg=cfg[cfg.index('typedef struct _nyx_config'):cfg.index('} nyx_config;')+len('} nyx_config;')]
common=r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <libs/lvgl/lvgl.h>
#include <libs/lvgl/lv_misc/lv_math.h>
#define HOME_LAYOUT_TILES 0
#define HOME_LAYOUT_CAROUSEL 1
'''+cfg+r'''
static nyx_config n_cfg;
static struct{bool rcm_patched;u32 autoboot,autonogc,autohosoff,updater2p,bootwait,backlight;}h_cfg;
static struct {lv_obj_t *mid;} status_bar;
static bool ini_changes_made,nyx_changes_made,autoboot_first_time;
static lv_obj_t *autoboot_btn;
static char *text_color="#FF0000";
#define s_printf sprintf
static lv_obj_t *close_btn,*autorcm_btn;
static lv_style_t hint_small_style, hint_small_style_white;
static lv_img_dsc_t *hekate_bg;
static char font_root[1024];
typedef unsigned int UINT;
typedef struct{FILE*fp;u32 size;}FIL;
typedef struct{u32 index;}DIR;
typedef struct{char fname[256];u8 fattrib;}FILINFO;
#define FR_OK 0
#define FA_READ 1
#define AM_DIR 0x10
#define AM_HID 0x02
#define f_size(f) ((f)->size)
static bool mounted;
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){mounted=true;return 0;}
static void sd_unmount(void){mounted=false;}
static int f_open(FIL*f,const char*path,int mode){(void)mode;char full[1200];snprintf(full,sizeof(full),"%s/%s",font_root,strrchr(path,'/')?strrchr(path,'/')+1:path);f->fp=fopen(full,"rb");if(!f->fp)return 1;fseek(f->fp,0,SEEK_END);f->size=ftell(f->fp);rewind(f->fp);return 0;}
static int f_close(FIL*f){return fclose(f->fp);}
static int f_lseek(FIL*f,u32 off){return fseek(f->fp,off,SEEK_SET);}
static int f_read(FIL*f,void*p,u32 length,UINT*actual){*actual=fread(p,1,length,f->fp);return ferror(f->fp)?1:0;}
static int f_stat(const char*p,void*x){(void)p;(void)x;return 0;}
static int f_opendir(DIR*d,const char*p){(void)p;d->index=0;return 0;}
static int f_readdir(DIR*d,FILINFO*f){static const char*names[]={"Lecture.nyxf","Lecture-renforcee.nyxf","Lecture-fixe.nyxf",""};strcpy(f->fname,names[d->index<3?d->index++:3]);f->fattrib=0;return 0;}
static int f_closedir(DIR*d){(void)d;return 0;}
static int create_nyx_config_entry(bool b){(void)b;return 0;}
static void reload_nyx(lv_obj_t*b,bool x){(void)b;(void)x;}
static void nyx_options_appearance_active(bool a,bool b){(void)a;(void)b;}
static lv_res_t nyx_win_close_action(lv_obj_t*b){return lv_win_close_action(b);}
static lv_obj_t*nyx_create_standard_window(const char*,lv_action_t);
#define ACTION(name) static lv_res_t name(lv_obj_t*b){(void)b;return LV_RES_OK;}
ACTION(create_window_backup_restore_tool) ACTION(create_window_sd_partition_manager) ACTION(create_window_emmc_partition_manager)
ACTION(_create_window_usb_tools) ACTION(_create_window_unset_abit_tool) ACTION(_create_mbox_fix_touchscreen) ACTION(_create_mbox_autorcm_status) ACTION(_create_window_dump_pk12_tool)
ACTION(_create_window_bootrom_info_status) ACTION(_create_mbox_lockpick) ACTION(_create_window_hw_info_status) ACTION(_kfuse_dump_window_action) ACTION(_create_window_emmc_info_status) ACTION(_create_window_sdcard_info_status) ACTION(_create_window_battery_status)
static bool get_set_autorcm_status(bool b){(void)b;return false;}
static lv_res_t nyx_generic_onoff_toggle(lv_obj_t*b);
static bool _lockpick_exists_check(void){return false;}
'''
headers='\n'.join(stripped(p)for p in [nyx/'fonts/font_manager.h',nyx/'localization.h',nyx/'fonts/unicode_fonts.h',nyx/'frontend/gui_studio.h',nyx/'frontend/gui_readability.h',nyx/'frontend/gui_layout.h',nyx/'frontend/gui_choices.h',nyx/'frontend/gui_appearance.h'])
body=common+headers+stripped(nyx/'fonts/font_manager.c')+stripped(nyx/'localization.c',True)
guih=(nyx/'frontend/gui.h').read_text();macros='\n'.join(re.findall(r'^#define lv_.*$',guih,re.M))
body+='\n'+macros+'\n'
body+=stripped(nyx/'frontend/gui_studio.c')+stripped(nyx/'frontend/gui_readability.c')+stripped(nyx/'frontend/gui_choices.c')+stripped(nyx/'frontend/gui_layout.c')+stripped(nyx/'frontend/gui_appearance.c')
for filename,name in [('gui_tools.c','create_tab_tools'),('gui_info.c','create_tab_info')]:
    s=(nyx/'frontend'/filename).read_text();body+=s[s.index('void '+name+'('):]
gui=(nyx/'frontend/gui.c').read_text();start=gui.index('lv_obj_t *nyx_create_standard_window(');end=gui.index('\nstatic bool launch_logs_enable',start)
body+=gui[start:end]

def function(source,name):
    at=re.search(r'^(?:static )?(?:lv_res_t|void|lv_obj_t \*)\s*'+name+r'\([^;]*?\)\s*\n\{',source,re.M)
    assert at,name
    end=re.search(r'^\}',source[at.end():],re.M)
    return source[at.start():at.end()+end.end()]+'\n'
body+=function(gui,'nyx_generic_onoff_toggle')
opts=(nyx/'frontend/gui_options.c').read_text(encoding='utf-8')
body+=r"""
ACTION(_action_win_nyx_options_close) ACTION(_joycon_info_dump_action) ACTION(_create_mbox_clock_edit)
ACTION(_autoboot_hide_delay_action)
ACTION(_slider_brightness_action) ACTION(_save_nyx_options_action)
"""
for name in ['_autoboot_delay_action','_data_verification_action','_home_screen_action','auto_nogc_toggle','auto_hos_poweroff_toggle','_update_r2p_action','_entries_columns_action']:body+=function(opts,name)
body+=opts[opts.index('static const u16 theme_colors[18]'):opts.index('typedef struct _time_edit_obj_t')]
body+=opts[opts.index('/* Compact pages keep controls'):]
diag=(nyx/'frontend/gui_diagnostics.c').read_text(encoding='utf-8')
groups=diag[diag.index('static const struct {'):diag.index('static lv_obj_t *text(')]
body+=re.sub(r'NYX_DIAG_G_\w+','0',groups)
body+=function(diag,'text')+function(diag,'button')+'\nACTION(show_group)\n'
constructor=diag[diag.index('void create_tab_diagnostics('):diag.index('    nyx_ram_handoff_t *saved')]
body+=constructor+'}\n'
body+='\nACTION(_create_window_home_launch) ACTION(_create_mbox_payloads) ACTION(create_win_emummc_tools) ACTION(_create_mbox_reload) ACTION(_create_mbox_reboot) ACTION(_create_mbox_poweroff)\n'
body+=gui[gui.index('/* Both native home layouts'):gui.index('static lv_res_t _save_options_action(')]
body+=r'''
static lv_color_t screen_pixels[LV_HOR_RES*LV_VER_RES];
static unsigned long rendered_pixels;
static void flush(int32_t x1,int32_t y1,int32_t x2,int32_t y2,const lv_color_t *pixels){
rendered_pixels+=(x2-x1+1)*(y2-y1+1);
for(int y=y1;y<=y2;y++)for(int x=x1;x<=x2;x++){if(x>=0&&x<LV_HOR_RES&&y>=0&&y<LV_VER_RES)screen_pixels[y*LV_HOR_RES+x]=*pixels;pixels++;}lv_flush_ready();}
static void dump(const char*path){FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",LV_HOR_RES,LV_VER_RES);for(u32 i=0;i<LV_HOR_RES*LV_VER_RES;i++){u8 rgb[]={screen_pixels[i].red,screen_pixels[i].green,screen_pixels[i].blue};fwrite(rgb,1,3,f);}fclose(f);}

static unsigned geometry_checks;
static bool type_is(lv_obj_t*o,const char*name){lv_obj_type_t t;lv_obj_get_type(o,&t);return t.type[0]&&!strcmp(t.type[0],name);}
static void verify_visible(lv_obj_t*object,lv_area_t clip){
 if(lv_obj_get_hidden(object))return;
 lv_area_t area,visible;lv_obj_get_coords(object,&area);if(!lv_area_intersect(&visible,&area,&clip))return;
 if(type_is(object,"lv_ddlist")){fprintf(stderr,"Unexpected dropdown\n");abort();}
 if(type_is(object,"lv_btn")){
  if(lv_obj_get_parent(object)!=home_carousel.track&&(area.x1<clip.x1||area.y1<clip.y1||area.x2>clip.x2||area.y2>clip.y2)){fprintf(stderr,"Button outside viewport: %d %d %d %d clip %d %d %d %d\n",area.x1,area.y1,area.x2,area.y2,clip.x1,clip.y1,clip.x2,clip.y2);abort();}
  ++geometry_checks;
  for(lv_obj_t*l=lv_obj_get_child(object,NULL);l;l=lv_obj_get_child(object,l))if(type_is(l,"lv_label")&&!lv_obj_get_hidden(l)){
   if(lv_label_get_long_mode(l)==LV_LABEL_LONG_BREAK&&lv_obj_get_height(l)>lv_obj_get_height(object)-4){fprintf(stderr,"Label taller than button: %s %d>%d\n",lv_label_get_text(l),lv_obj_get_height(l),lv_obj_get_height(object));abort();}
   ++geometry_checks;
  }
 }
 for(lv_obj_t*child=lv_obj_get_child(object,NULL);child;child=lv_obj_get_child(object,child))verify_visible(child,visible);
}
static lv_obj_t*find_choice(lv_obj_t*object,u32 group,u32 value){
 if(type_is(object,"lv_btn")&&lv_btn_get_action(object,LV_BTN_ACTION_CLICK)==choice_pressed&&lv_obj_get_free_num(object)==value&&lv_obj_get_free_num(lv_obj_get_parent(object))==group)return object;
 for(lv_obj_t*child=lv_obj_get_child(object,NULL);child;child=lv_obj_get_child(object,child)){lv_obj_t*found=find_choice(child,group,value);if(found)return found;}
 return NULL;
}
int main(int argc,char**argv){
assert(argc==7);strcpy(font_root,argv[1]);
FILE*f=fopen(argv[2],"rb");assert(f);fseek(f,0,SEEK_END);u32 bytes=ftell(f);rewind(f);void*res=VirtualAlloc((void*)0xEE000000,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(res==(void*)0xEE000000);assert(fread(res,1,bytes,f)==bytes);fclose(f);
n_cfg.font_size=atoi(argv[5]);strcpy(n_cfg.font_file,getenv("NYX_PREVIEW_FONT")?getenv("NYX_PREVIEW_FONT"):"Lecture.nyxf");n_cfg.language=atoi(argv[4]);n_cfg.background_effect=1;n_cfg.background_speed=2;n_cfg.background_opacity=20;h_cfg.backlight=130;
lv_init();nyx_localization_init();nyx_font_manager_init();lv_disp_drv_t driver;lv_disp_drv_init(&driver);driver.disp_flush=flush;lv_disp_drv_register(&driver);
lv_theme_t*theme=lv_theme_hekate_init(0x141A19,0,(lv_font_t*)nyx_font_regular());lv_theme_set_current(theme);lv_style_copy(&hint_small_style,theme->label.hint);hint_small_style.text.font=nyx_font_small();hint_small_style.text.letter_space=1;lv_style_copy(&hint_small_style_white,&hint_small_style);nyx_studio_init(theme);
static lv_style_t screen_style;lv_style_copy(&screen_style,&lv_style_plain);screen_style.body.main_color=screen_style.body.grad_color=LV_COLOR_HEX(0x111219);lv_obj_set_style(lv_scr_act(),&screen_style);nyx_background_create(lv_scr_act());
lv_obj_t *status=lv_obj_create(lv_layer_top(),NULL);lv_obj_set_style(status,&screen_style);lv_obj_set_size(status,1280,64);lv_obj_set_pos(status,0,656);lv_obj_set_click(status,false);
lv_obj_t *clock=lv_label_create(status,NULL);lv_label_set_text(clock,"16:58   18.3 C");lv_obj_set_pos(clock,32,18);
lv_obj_t *battery=lv_label_create(status,NULL);lv_label_set_text(battery,"48.8%");lv_obj_align(battery,status,LV_ALIGN_IN_RIGHT_MID,-32,0);
status_bar.mid=lv_btn_create(status,NULL);lv_obj_set_size(status_bar.mid,200,48);lv_obj_align(status_bar.mid,status,LV_ALIGN_CENTER,0,0);
lv_obj_t*root=NULL;
if(!strcmp(argv[3],"appearance"))nyx_create_appearance_window(NULL);
else if(!strcmp(argv[3],"nyx"))create_win_nyx_options(NULL);
else if(!strcmp(argv[3],"colors"))_create_window_nyx_colors(NULL);
else {
 lv_obj_t*tabs=nyx_studio_tabview(lv_scr_act());
 if(nyx_font_size()>30){static lv_style_t st[4];int kinds[]={LV_TABVIEW_STYLE_BTN_REL,LV_TABVIEW_STYLE_BTN_PR,LV_TABVIEW_STYLE_BTN_TGL_REL,LV_TABVIEW_STYLE_BTN_TGL_PR};for(int j=0;j<4;j++){lv_style_copy(&st[j],lv_tabview_get_style(tabs,kinds[j]));st[j].text.font=nyx_font_small();lv_tabview_set_style(tabs,kinds[j],&st[j]);}}
 const char*names[]={"hekate 6.5.4",SYMBOL_HOME" Home",SYMBOL_TOOLS" Tools",SYMBOL_INFO" Console Info",SYMBOL_SETTINGS" Options",SYMBOL_CHIP" Diagnostic"};lv_obj_t*pages[6];
 for(int i=0;i<6;i++)pages[i]=lv_tabview_add_tab(tabs,names[i]);
 nyx_studio_navigation(lv_scr_act(),tabs,"hekate 6.5.4",create_win_nyx_options);
 int active=(!strcmp(argv[3],"home")||!strcmp(argv[3],"carousel"))?1:!strcmp(argv[3],"tools")?2:!strcmp(argv[3],"info")?3:!strcmp(argv[3],"options")?4:5;
 root=pages[active];if(active==1){home_tabview=tabs;n_cfg.home_layout=!strcmp(argv[3],"carousel");_create_tab_home(theme,root);}else if(active==2)create_tab_tools(theme,root);else if(active==3)create_tab_info(theme,root);else if(active==4)create_tab_options(theme,root);else create_tab_diagnostics(theme,root);
 lv_tabview_set_tab_act(tabs,active,false);
 for(u32 i=1;i<6;i++){studio_navigate(studio_buttons[i]);assert(lv_tabview_get_tab_act(tabs)==i);assert(lv_cont_get_style(studio_icons[i])->body.shadow.width==28);}
 lv_tabview_set_tab_act(tabs,active,false);nyx_studio_navigation_select(active);
 assert(lv_tabview_get_btns_hidden(tabs));assert(lv_obj_get_width(root)==1056);assert(lv_obj_get_height(root)==592);
 lv_area_t sidebar_bounds={0,0,223,655};for(int i=1;i<6;i++)verify_visible(studio_buttons[i],sidebar_bounds);
}
if(!root){lv_obj_t*window=lv_win_get_from_btn(close_btn);lv_win_ext_t*ext=lv_obj_get_ext_attr(window);root=ext->page;
 if(strcmp(argv[3],"colors")){lv_obj_t*tabs=lv_obj_get_child(lv_page_get_scrl(root),NULL);lv_tabview_set_tab_act(tabs,atoi(argv[6]),false);}}
if(strcmp(argv[3],"options")&&strcmp(argv[3],"nyx"))lv_obj_set_hidden(status_bar.mid,true);else{lv_obj_t*save=lv_label_create(status_bar.mid,NULL);lv_label_set_text(save,"Save");}
nyx_readability_refresh(lv_scr_act());for(int i=0;i<4;i++){lv_tick_inc(40);lv_task_handler();}lv_refr_now();
lv_area_t bounds;lv_obj_get_coords(root,&bounds);assert(bounds.y1>=0&&bounds.y2<656);
verify_visible(root,bounds);
if(!strcmp(argv[3],"options")){
 lv_obj_t*page=lv_page_get_scrl(root);unsigned switches=0;
 for(lv_obj_t*c=lv_obj_get_child(page,NULL);c;c=lv_obj_get_child(page,c))for(lv_obj_t*b=lv_obj_get_child(c,NULL);b;b=lv_obj_get_child(c,b))if(lv_obj_get_signal_func(b)==studio_toggle_signal){
  studio_toggle_ext*e=lv_obj_get_ext_attr(b);assert(e->knob&&e->track&&e->state);lv_action_t action=lv_btn_get_action(b,LV_BTN_ACTION_CLICK);
  for(lv_obj_t*l=lv_obj_get_child(b,NULL);l;l=lv_obj_get_child(b,l))if(type_is(l,"lv_label")&&l!=e->state)assert(lv_obj_get_y(l)+lv_obj_get_height(l)<=lv_obj_get_y(e->track)-6);
  if(action!=_autoboot_hide_delay_action){bool before=(lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0;action(b);assert(((lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0)!=before);assert(lv_obj_get_x(e->knob)==(before?4:34));action(b);assert(((lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0)==before);}
  ++switches;
 }assert(switches==4);
}
if(!strcmp(argv[3],"appearance")){
 u32 language=n_cfg.language;lv_obj_t*choice=find_choice(root,AP_LANGUAGE,2);assert(choice);lv_btn_get_action(choice,LV_BTN_ACTION_CLICK)(choice);assert(appearance_pending.language==2&&n_cfg.language==language);
 choice=find_choice(root,AP_SIZE,8);assert(choice);lv_btn_get_action(choice,LV_BTN_ACTION_CLICK)(choice);assert(appearance_pending.font_size==36&&n_cfg.font_size==(u32)atoi(argv[5]));
 char before[64];strcpy(before,appearance_pending.font_file);lv_obj_t step;memset(&step,0,sizeof step);lv_obj_set_free_num(&step,1);appearance_font_step(&step);lv_obj_set_free_num(&step,0);appearance_font_step(&step);assert(!strcmp(before,appearance_pending.font_file));
}
if(!strcmp(argv[3],"colors")){
 unsigned count=0;for(lv_obj_t*swatch=lv_obj_get_child(color_test.header1,NULL);swatch;swatch=lv_obj_get_child(color_test.header1,swatch))if(type_is(swatch,"lv_btn")){
  lv_area_t coords;lv_obj_get_coords(swatch,&coords);u32 pixel=screen_pixels[((coords.y1+coords.y2)/2)*1280+(coords.x1+coords.x2)/2].full&0xFFFFFF;
  u32 expected=lv_btn_get_style(swatch,LV_BTN_STYLE_REL)->body.main_color.full&0xFFFFFF;assert(pixel==expected);++count;
 }assert(count==18);
}
dump("preview.ppm");
if(!strcmp(argv[3],"tools")&&!strcmp(argv[4],"0")&&!strcmp(argv[5],"30")){
 nyx_background_pause(true);for(int i=0;i<20;i++){lv_tick_inc(50);lv_task_handler();}lv_refr_now();
 rendered_pixels=0;nyx_studio_icon_select(studio_icons[2],false);lv_refr_now();unsigned long off_pixels=rendered_pixels;assert(off_pixels>0&&off_pixels<=10000);
 rendered_pixels=0;nyx_studio_icon_select(studio_icons[2],true);lv_refr_now();unsigned long on_pixels=rendered_pixels;assert(on_pixels>0&&on_pixels<=10000);
 rendered_pixels=0;for(int i=0;i<100;i++){lv_tick_inc(50);lv_task_handler();}lv_refr_now();assert(rendered_pixels==0);
 printf("PASS: Studio halo redraw off=%lu on=%lu idle=%lu pixels over 5000ms\n",off_pixels,on_pixels,rendered_pixels);
}
printf("PASS UI %s language=%s size=%s tab=%s controls=%u\n",argv[3],argv[4],argv[5],argv[6],geometry_checks);return 0;}
'''
(directory/'preview.c').write_text(body,encoding='utf-8')
cc=work/'toolchains/w64devkit/bin/gcc.exe'
flags=['-std=gnu11','-O1','-include','stdint.h','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(directory),'-I'+str(project/'bdk'),'-I'+str(nyx)]
sources=list(lvgl.rglob('*.c'))+[nyx/'fonts/unicode_fonts.c',directory/'preview.c']
def compile_one(p):
    obj=directory/(p.stem+'.o')
    result=subprocess.run([str(cc),*flags,'-c',str(p),'-o',str(obj)],capture_output=True,text=True)
    if result.returncode:raise RuntimeError(str(p)+'\n'+result.stderr)
    return obj
with ThreadPoolExecutor(max_workers=6)as pool:objects=list(pool.map(compile_one,sources))
exe=directory/'preview.exe';subprocess.run([str(cc),*[str(o)for o in objects],'-o',str(exe)],check=True)
jobs=[('tools',1,20,0),('tools',4,36,0),('info',1,20,0),('options',1,20,0),('options',4,36,0),('diagnostics',4,36,0),('appearance',1,30,0),('appearance',1,30,1),('appearance',1,30,3),('nyx',4,36,0),('nyx',4,36,1),('colors',1,30,0)]
jobs.extend([('home',1,30,0),('carousel',4,36,0)])
if '--check-all' in sys.argv:
    modes=[('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('appearance',0),('appearance',1),('appearance',2),('appearance',3),('nyx',0),('nyx',1),('colors',0)]
    jobs=[(screen,language,size,tab) for screen,tab in modes for language in range(7) for size in range(20,38,2)]
for screen,language,size,tab in jobs:
    run=subprocess.run([str(exe),str(nyx/'fonts/sd'),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],cwd=directory,capture_output=True,text=True,timeout=20)
    if run.returncode:raise RuntimeError(f'{screen} {language} {size} {tab}: '+run.stdout+run.stderr)
    performance=re.search(r'Studio halo redraw off=(\d+) on=(\d+) idle=(\d+)',run.stdout)
    if performance:
        result={'halo_radius_px':28,'off_redraw_pixels':int(performance[1]),'on_redraw_pixels':int(performance[2]),'idle_redraw_pixels_over_5000ms':int(performance[3]),'platform':'host LVGL, not console FPS'}
        (directory/'studio-performance.json').write_text(json.dumps(result,indent=2))
        print('PASS: Studio halo is input-driven; '+performance[0],flush=True)
    if '--check-all' not in sys.argv:
        target=work/f'preview-v9-{screen}-{language}-{size}-{tab}.png'
        Image.open(directory/'preview.ppm').save(target)
        print(target.name,run.stdout.strip(),flush=True)
print(f'PASS: real-LVGL UI geometry and selector checks across {len(jobs)} screens; visible controls stay above the status bar; all 18 color swatches have their actual fill.',flush=True)
