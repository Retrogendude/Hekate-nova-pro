"""Headless rendering of the actual LVGL 5 code, card builders and Appearance UI.

Only platform I/O and hardware actions are substituted. Fonts come from the
real SD NYXF loader; icons come from the unmodified official res.pak.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from zipfile import ZipFile
import json
import os
import re
import subprocess
import sys
from PIL import Image

work=Path(__file__).resolve().parent
project=work/'hekate-6.5.4'
directory=work/'lvgl-preview-v13';directory.mkdir(exist_ok=True)
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
#include <libs/lvgl/lv_draw/lv_draw_vbasic.h>
#include <libs/lvgl/lv_misc/lv_math.h>
#define HOME_LAYOUT_TILES 0
#define HOME_LAYOUT_CAROUSEL 1
'''+cfg+r'''
static nyx_config n_cfg;
static struct{bool rcm_patched;u32 autoboot,autonogc,autohosoff,updater2p,bootwait,backlight;}h_cfg;
static struct {lv_obj_t *bar_bg,*mid,*time_temp,*temp_symbol,*temp_degrees,*battery,*battery_more;} status_bar;
static bool ini_changes_made,nyx_changes_made,autoboot_first_time;
static lv_obj_t *autoboot_btn;
static char *text_color="#FF0000";
#define s_printf sprintf
static lv_obj_t *close_btn,*autorcm_btn;
static lv_style_t hint_small_style, hint_small_style_white,mbox_darken;
static lv_res_t _create_mbox_clock_edit(lv_obj_t *button);
static lv_res_t _action_win_nyx_options_close(lv_obj_t *button);
static lv_img_dsc_t *hekate_bg;
static char font_root[1024];
typedef unsigned int UINT;
typedef struct{FILE*fp;u32 size;}FIL;
typedef struct{u32 index,wallpaper;}DIR;
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
static int f_open(FIL*f,const char*path,int mode){(void)mode;char full[1200];snprintf(full,sizeof(full),strstr(path,"wallpapers/")==path?"%s/../../../../wallpapers/%s":"%s/%s",font_root,strrchr(path,'/')?strrchr(path,'/')+1:path);f->fp=fopen(full,"rb");if(!f->fp)return 1;fseek(f->fp,0,SEEK_END);f->size=ftell(f->fp);rewind(f->fp);return 0;}
static int f_close(FIL*f){return fclose(f->fp);}
static int f_lseek(FIL*f,u32 off){return fseek(f->fp,off,SEEK_SET);}
static int f_read(FIL*f,void*p,u32 length,UINT*actual){*actual=fread(p,1,length,f->fp);return ferror(f->fp)?1:0;}
static int f_stat(const char*p,void*x){(void)p;(void)x;return 0;}
static int f_opendir(DIR*d,const char*p){d->index=0;d->wallpaper=!strcmp(p,"wallpapers");return 0;}
static int f_readdir(DIR*d,FILINFO*f){static const char*names[]={"Lecture.nyxf","Lecture-renforcee.nyxf","Lecture-fixe.nyxf",""};strcpy(f->fname,d->wallpaper?(d->index++?"":"Nova-Carbon.bmp"):names[d->index<3?d->index++:3]);f->fattrib=0;return 0;}
static int f_closedir(DIR*d){(void)d;return 0;}
static int create_nyx_config_entry(bool b){(void)b;return 0;}
static int create_config_entry(void){return 0;}
static void reload_nyx(lv_obj_t*b,bool x){(void)b;(void)x;}
void nyx_options_appearance_active(bool a,bool b);
static lv_res_t nyx_win_close_action(lv_obj_t*b){close_btn=NULL;return lv_win_close_action(b);}
static lv_obj_t*nyx_create_standard_window(const char*,lv_action_t);
static lv_action_t last_action;
static unsigned action_invocations;
#define ACTION(name) static lv_res_t name(lv_obj_t*b){(void)b;last_action=name;++action_invocations;return LV_RES_OK;}
ACTION(create_window_backup_restore_tool) ACTION(create_window_sd_partition_manager) ACTION(create_window_emmc_partition_manager)
ACTION(_create_window_usb_tools) ACTION(_create_window_unset_abit_tool) ACTION(_create_mbox_fix_touchscreen) ACTION(_create_mbox_autorcm_status) ACTION(_create_window_dump_pk12_tool)
ACTION(_create_window_bootrom_info_status) ACTION(_create_mbox_lockpick) ACTION(_create_window_hw_info_status) ACTION(_kfuse_dump_window_action) ACTION(_create_window_emmc_info_status) ACTION(_create_window_sdcard_info_status) ACTION(_create_window_battery_status)
static bool get_set_autorcm_status(bool b){(void)b;return false;}
static lv_res_t nyx_generic_onoff_toggle(lv_obj_t*b);
static bool _lockpick_exists_check(void){return getenv("NYX_PREVIEW_LOCKPICK")!=NULL;}
'''
headers='\n'.join(stripped(p)for p in [nyx/'fonts/font_manager.h',nyx/'localization.h',nyx/'fonts/unicode_fonts.h',nyx/'frontend/gui_nova.h',nyx/'frontend/gui_options.h',nyx/'frontend/gui_readability.h',nyx/'frontend/gui_layout.h',nyx/'frontend/gui_choices.h',nyx/'frontend/gui_appearance.h'])
body=common+headers+stripped(nyx/'fonts/font_manager.c')+stripped(nyx/'localization.c',True)
guih=(nyx/'frontend/gui.h').read_text();macros='\n'.join(re.findall(r'^#define lv_.*$',guih,re.M))
body+='\n'+macros+'\n'
nova=(nyx/'frontend/gui_nova.c').read_text(encoding='utf-8')
nova=nova.replace('lv_obj_t *label = lv_label_create(parent, NULL);','if(getenv("NYX_TRACE"))fprintf(stderr,"NOVA TEXT: %s\\n",text);\n    lv_obj_t *label = lv_label_create(parent, NULL);')
for asset in ['nova_reference.inc','nova_icons.inc']:
    nova=nova.replace('#include "nova_assets/'+asset+'"',(nyx/'frontend/nova_assets'/asset).read_text(encoding='utf-8'))
body+=stripped(nyx/'frontend/gui_nova_reflection.c')+re.sub(r'^#include[^\n]*\n','',nova,flags=re.M)+stripped(nyx/'frontend/gui_readability.c')+stripped(nyx/'frontend/gui_choices.c')+stripped(nyx/'frontend/gui_layout.c')+stripped(nyx/'frontend/gui_appearance.c')
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
body+=r'''
typedef struct{u8 weekday,sec,min,hour,day,month;u16 year;}rtc_time_t;
enum{BQ24193_ChargeStatus,MAX17050_RepSOC,MAX17050_VCELL,MAX17050_Current};
static u32 preview_battery_percent=82;
static u32 preview_temperature=28;
static void max77620_rtc_get_time_adjusted(rtc_time_t*t){*t=(rtc_time_t){.year=2026,.month=11,.day=6,.hour=18,.min=42};}
static void max77620_rtc_get_time(rtc_time_t*t){max77620_rtc_get_time_adjusted(t);}
static void max77620_rtc_set_epoch_offset(int offset){(void)offset;}
static void max77620_rtc_set_auto_dst(bool enabled){(void)enabled;}

static u16 tmp451_get_soc_temp(bool b){(void)b;return preview_temperature<<8;}
static void bq24193_get_property(int p,int*v){(void)p;*v=0;}
static void max17050_get_property(int p,int*v){*v=p==MAX17050_RepSOC?preview_battery_percent<<8:p==MAX17050_VCELL?4000:0;}
static void fan_set_from_temp(u32 t){assert(t==preview_temperature);}
'''
body+=function(gui,'_update_status_bar')
opts=(nyx/'frontend/gui_options.c').read_text(encoding='utf-8')
body+=r"""
ACTION(_joycon_info_dump_action)
ACTION(_autoboot_hide_delay_action)
ACTION(_slider_brightness_action) ACTION(_save_nyx_options_action)
"""
for name in ['_autoboot_delay_action','_data_verification_action','_home_screen_action','auto_nogc_toggle','auto_hos_poweroff_toggle','_update_r2p_action','_entries_columns_action']:body+=function(opts,name)
body+='\nstatic void _check_nyx_changes(void){assert(!nyx_changes_made); }\n'+function(opts,'_action_win_nyx_options_close')
body+=function(opts,'nyx_options_appearance_active')
body+=opts[opts.index('static const u16 theme_colors[18]'):opts.index('typedef struct _time_edit_obj_t')]
body+=opts[opts.index('/* Compact pages keep controls'):]
body+=stripped(nyx/'frontend/gui_nova_theme.c')
rtc=(project/'bdk/rtc/max77620-rtc.c').read_text()
at=rtc.index('u32 max77620_rtc_date_to_epoch(');end=rtc.index('\n}',at)+2
body+=rtc[at:end]+'\n'
body+='\n'.join(re.findall(r'^#define CLOCK_[^\n]*',opts,re.M))+'\n'
body+=function(gui,'nyx_create_onoff_button')+function(gui,'nyx_mbox_action')
body+=opts[opts.index('typedef struct _time_edit_obj_t'):opts.index('static lv_res_t _joycon_info_dump_action(')]

emmc=(nyx/'frontend/gui_emmc_tools.c').read_text(encoding='utf-8')
body+=emmc[emmc.index('typedef struct _emmc_backup_buttons_t'):emmc.index('static void _create_window_backup_restore')]
body+='\nACTION(_emmc_backup_buttons_decider)\n'
body+=function(emmc,'_emmc_backup_buttons_raw_toggle')
body+=function(emmc,'create_window_backup_restore_tool').replace('create_window_backup_restore_tool(', 'preview_backup_restore(')
tools=(nyx/'frontend/gui_tools.c').read_text(encoding='utf-8')
body+='\nstatic int usb_msc_emmc_read_only;\nACTION(action_ums_sd) ACTION(_action_ums_emmc_gpp) ACTION(_action_ums_emmc_boot0) ACTION(_action_ums_emmc_boot1) ACTION(_action_ums_emuemmc_gpp) ACTION(_action_ums_emuemmc_boot0) ACTION(_action_ums_emuemmc_boot1) ACTION(_action_hid_jc)\n'
body+=function(tools,'_emmc_read_only_toggle')
body+=function(tools,'_create_window_usb_tools').replace('_create_window_usb_tools(', 'preview_usb_tools(')
body+=r"""
typedef struct{u32 enabled,sector;char*path,*nintendo_path;}emummc_cfg_t;
static lv_obj_t*emummc_manage_window;static lv_res_t(*emummc_tools)(lv_obj_t*);
static void load_emummc_cfg(emummc_cfg_t*c){c->enabled=1;c->sector=0x100000;c->path=strdup("emuMMC/RAW1");c->nintendo_path=strdup("emuMMC/RAW1/Nintendo");}
ACTION(_create_change_emummc_window) ACTION(_create_mbox_emummc_create) ACTION(_create_mbox_emummc_migrate)
static lv_res_t create_win_emummc_tools(lv_obj_t*);
"""
body+=function((nyx/'frontend/gui_emummc_tools.c').read_text(encoding='utf-8'),'create_win_emummc_tools').replace('create_win_emummc_tools(', 'preview_emummc_tools(')

diag=(nyx/'frontend/gui_diagnostics.c').read_text(encoding='utf-8')
groups=diag[diag.index('static const struct {'):diag.index('static lv_obj_t *text(')]
body+=stripped(nyx/'diagnostics/nyx_diagnostics.h')+stripped(nyx/'diagnostics/ram_handoff.h')+'\n'+groups
body+=function(diag,'text')+function(diag,'button')+'\nACTION(show_group)\n'

# Real report/group card layouts with captured fixture probe metadata.
registry=(nyx/'diagnostics/hwtest_pages.c').read_text()
entries=re.findall(r'\{\s*(probe_\w+)\s*,\s*"([^"]+)"\s*\}',registry)
group_values=re.findall(r'\b(?:PROBE_G_\w+|0)\b',registry.split('const u32 g_page_groups[] = {')[1].split('};')[0])
assert len(entries)==len(group_values)==42
body+='\nstatic nyx_diag_probe_t preview_probes[]={\n'
for i,((fn,name),group) in enumerate(zip(entries,group_values)):
    body+='{'+f'"{name}","{fn}",{group.replace("PROBE_G_","NYX_DIAG_G_")},{2+i%4},0,0'+'},\n'
body+='};\nconst nyx_diag_probe_t*nyx_diag_probe(u32 i){return i<42?&preview_probes[i]:NULL;}\n'
body+='\n#define ONE_PROBE (1u<<30)\nstatic lv_obj_t*results_window,*diagnostic_last_holder;static nyx_ram_handoff_t last_ram;static bool last_ram_valid;\nACTION(run_selection) ACTION(show_report) ACTION(export_report) ACTION(launch_ram)\nbool nyx_diag_truncated(void){return false;}static bool gui_diagnostics_inputs_ready(void){return true;}\n'
body+=function(diag,'notice')
state=diag[diag.index('static const char *state_name'):diag.index('static void render_report_page')]
body+=state+function(diag,'close_results')
tiles=diag[diag.index('/* Stable pages of native Nova cards'):diag.index('static void show_results')]
tiles=tiles.replace('lv_obj_t *holder=lv_cont_create(win,NULL);','lv_obj_t *holder=lv_cont_create(win,NULL);diagnostic_last_holder=holder;')
body+=tiles+function(diag,'show_results')
ram=diag[diag.index('static const char *ram_state'):diag.index('static lv_res_t launch_ram')]
body+=ram+function(diag,'show_ram_service')
body+=function(diag,'show_group').replace('show_group(', 'preview_show_group(')

constructor=diag[diag.index('void create_tab_diagnostics('):diag.index('    nyx_ram_handoff_t *saved')]
body+=constructor+'}\n'
body+='\nACTION(_create_window_home_launch) ACTION(_create_mbox_payloads) ACTION(create_win_emummc_tools) ACTION(_create_mbox_reload) ACTION(_create_mbox_reboot) ACTION(_create_mbox_poweroff)\n'
body+=gui[gui.index('/* Both native home layouts'):gui.index('static lv_res_t _save_options_action(')]
indev_source=(lvgl/'lv_core/lv_indev.c').read_text()
at=indev_source.index('static lv_obj_t * indev_search_obj(const lv_indev_proc_t * proc, lv_obj_t * obj)\n{');end=indev_source.index('\n}',at)+2
body+=indev_source[at:end].replace('indev_search_obj','preview_hit_search')
body+=r'''
static lv_color_t screen_pixels[LV_HOR_RES*LV_VER_RES];
static unsigned long rendered_pixels;
static void flush(int32_t x1,int32_t y1,int32_t x2,int32_t y2,const lv_color_t *pixels){
rendered_pixels+=(x2-x1+1)*(y2-y1+1);
for(int y=y1;y<=y2;y++)for(int x=x1;x<=x2;x++){if(x>=0&&x<LV_HOR_RES&&y>=0&&y<LV_VER_RES)screen_pixels[y*LV_HOR_RES+x]=*pixels;pixels++;}lv_flush_ready();}
static void dump(const char*path){FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",LV_HOR_RES,LV_VER_RES);for(u32 i=0;i<LV_HOR_RES*LV_VER_RES;i++){u8 rgb[]={screen_pixels[i].red,screen_pixels[i].green,screen_pixels[i].blue};fwrite(rgb,1,3,f);}fclose(f);}


static unsigned tree_checks;
static void verify_tree(lv_obj_t *parent){
 lv_obj_t *previous=NULL,*head=lv_ll_get_head(&parent->child_ll),*tail=lv_ll_get_tail(&parent->child_ll);unsigned forward=0,backward=0;
 assert(!head||!lv_ll_get_prev(&parent->child_ll,head));assert(!tail||!lv_ll_get_next(&parent->child_ll,tail));
 for(lv_obj_t *child=head;child;child=lv_ll_get_next(&parent->child_ll,child)){
  assert(++forward<4096);assert(lv_obj_get_parent(child)==parent);assert(lv_ll_get_prev(&parent->child_ll,child)==previous);previous=child;verify_tree(child);++tree_checks;
 }assert(previous==tail);previous=NULL;
 for(lv_obj_t *child=tail;child;child=lv_ll_get_prev(&parent->child_ll,child)){assert(++backward<4096);assert(lv_ll_get_next(&parent->child_ll,child)==previous);previous=child;}
 assert(previous==head&&forward==backward);
}
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

static lv_indev_state_t preview_pointer_state;
static lv_point_t preview_pointer_point;
static unsigned touch_checks;
static bool preview_pointer_read(lv_indev_data_t *data){data->point=preview_pointer_point;data->state=preview_pointer_state;return false;}
static void preview_click_point(int x,int y){
 preview_pointer_point=(lv_point_t){x,y};nyx_nova_reflection_interaction();
 preview_pointer_state=LV_INDEV_STATE_PR;for(u32 i=0;i<4;++i){lv_tick_inc(20);lv_task_handler();}
 preview_pointer_state=LV_INDEV_STATE_REL;for(u32 i=0;i<4;++i){lv_tick_inc(20);lv_task_handler();}
 verify_tree(lv_scr_act());++touch_checks;
}
static lv_obj_t *preview_hit(int x,int y){lv_indev_proc_t proc;memset(&proc,0,sizeof proc);proc.act_point=(lv_point_t){x,y};lv_obj_t *hit=preview_hit_search(&proc,lv_layer_top());return hit?hit:preview_hit_search(&proc,lv_scr_act());}
static void preview_click(lv_obj_t *button){
 assert(button&&lv_obj_get_click(button));lv_area_t coords;lv_obj_get_coords(button,&coords);int x=(coords.x1+coords.x2)/2,y=(coords.y1+coords.y2)/2;
 assert(preview_hit(x,y)==button);preview_click_point(x,y);
}
static lv_obj_t *find_action(lv_obj_t *object,lv_action_t action){
 if(type_is(object,"lv_btn")&&lv_btn_get_action(object,LV_BTN_ACTION_CLICK)==action)return object;
 for(lv_obj_t *child=lv_obj_get_child(object,NULL);child;child=lv_obj_get_child(object,child)){lv_obj_t *found=find_action(child,action);if(found)return found;}
 return NULL;
}
static void verify_reachable(lv_obj_t *object,lv_area_t clip){
 if(lv_obj_get_hidden(object))return;lv_area_t coords,visible;lv_obj_get_coords(object,&coords);if(!lv_area_intersect(&visible,&coords,&clip))return;
 if(type_is(object,"lv_btn")&&lv_obj_get_click(object)&&coords.x1>=clip.x1&&coords.y1>=clip.y1&&coords.x2<=clip.x2&&coords.y2<=clip.y2){
  int x=(coords.x1+coords.x2)/2,y=(coords.y1+coords.y2)/2;lv_obj_t *hit=preview_hit(x,y),*ancestor=hit;
  while(ancestor&&ancestor!=object)ancestor=lv_obj_get_parent(ancestor);
  if(!ancestor){fprintf(stderr,"Unreachable button at %d,%d; actual hit=%p expected=%p\n",x,y,(void*)hit,(void*)object);abort();}++touch_checks;
 }
 for(lv_obj_t *child=lv_obj_get_child(object,NULL);child;child=lv_obj_get_child(object,child))verify_reachable(child,visible);
}
static void verify_wallpaper_navigation(void){
 assert(nova_wallpaper);lv_indev_drv_t driver;lv_indev_drv_init(&driver);driver.type=LV_INDEV_TYPE_POINTER;driver.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&driver);
 for(u32 cycle=0;cycle<3;++cycle){
  preview_click(nova_quick_buttons[0]);assert(nova_navigation_locks==1&&close_btn);lv_obj_t *parent_close=close_btn,*window=lv_win_get_from_btn(close_btn);
  lv_obj_t *colors=find_action(window,_create_window_nyx_colors);assert(colors);preview_click(colors);assert(nova_theme.open&&nova_navigation_locks==2);
  lv_obj_t *choice=find_choice(nova_theme.window,THEME_TARGET,3);assert(choice);preview_click(choice);assert(nova_theme.target==NOVA_COLOR_LOGO);
  preview_click(close_btn);assert(!nova_theme.open&&close_btn==parent_close&&nova_navigation_locks==1);
  lv_obj_t *appearance=find_action(window,nyx_create_appearance_window);assert(appearance);preview_click(appearance);assert(appearance_open&&nova_navigation_locks==2);
  preview_click(close_btn);assert(!appearance_open&&close_btn==parent_close&&nova_navigation_locks==1);
  preview_click(close_btn);assert(!close_btn&&!nova_navigation_locks);
 }
 for(u32 tab=1;tab<6;++tab){preview_click(nova_nav_buttons[tab]);assert(lv_tabview_get_tab_act(home_tabview)==tab);}
 preview_click(nova_nav_buttons[1]);assert(lv_tabview_get_tab_act(home_tabview)==1);
 printf("PASS: wallpaper navigation through actual LVGL press/release events; repeated Nyx/colors/appearance open and close, color selection and all main tabs; %u checks\n",touch_checks);
}
static unsigned glyph_pixel_checks;
static void verify_glyph_pixels(lv_obj_t*object,lv_area_t clip){
 if(lv_obj_get_hidden(object))return;
 lv_area_t observed;if(!reflection_visible(object,&observed))return;
 lv_area_t area,visible;lv_obj_get_coords(object,&area);if(!lv_area_intersect(&visible,&area,&clip))return;
 if(type_is(object,"lv_img")&&area.x1>=clip.x1&&area.y1>=clip.y1&&area.x2<=clip.x2&&area.y2<=clip.y2){
  const lv_img_dsc_t*asset=lv_img_get_src(object);
  if(asset&&lv_img_src_get_type(asset)==LV_IMG_SRC_VARIABLE&&asset->header.cf==LV_IMG_CF_ALPHA_8BIT){
   const lv_style_t*style=lv_img_get_style(object);u32 strongest=0;
   for(u32 i=1;i<asset->data_size;i++)if(asset->data[i]>asset->data[strongest])strongest=i;
   assert(asset->data[strongest]>64&&asset->data[0]==0);
   lv_color_t background=screen_pixels[area.y1*LV_HOR_RES+area.x1];
   lv_color_t actual=screen_pixels[(area.y1+strongest/asset->header.w)*LV_HOR_RES+area.x1+strongest%asset->header.w];
   lv_color_t expected=lv_color_mix(style->image.color,background,asset->data[strongest]);
   int tolerance=nova_wallpaper?20:4;
   if(abs((int)actual.red-expected.red)>tolerance||abs((int)actual.green-expected.green)>tolerance||abs((int)actual.blue-expected.blue)>tolerance){fprintf(stderr,"Icon pixel mismatch at %d,%d alpha=%d actual=%06x expected=%06x\n",area.x1,area.y1,asset->data[strongest],actual.full&0xFFFFFF,expected.full&0xFFFFFF);abort();}
   ++glyph_pixel_checks;
  }
 }
 for(lv_obj_t*c=lv_obj_get_child(object,NULL);c;c=lv_obj_get_child(object,c))verify_glyph_pixels(c,visible);
}
static void verify_nova_bindings(lv_obj_t*page){
 nova_grid_ext*grid=lv_obj_get_ext_attr(page);unsigned count=grid->kind==NOVA_TOOLS?8:grid->kind==NOVA_INFO?7:12;assert(grid->count==count);
 lv_action_t tools[]={create_window_backup_restore_tool,create_window_backup_restore_tool,create_window_sd_partition_manager,_create_window_usb_tools,_create_window_unset_abit_tool,_create_mbox_fix_touchscreen,_create_mbox_autorcm_status,_create_window_dump_pk12_tool};
 lv_action_t info[]={_create_window_hw_info_status,_create_window_emmc_info_status,_create_window_sdcard_info_status,_create_window_battery_status,_create_window_bootrom_info_status,_kfuse_dump_window_action,_create_mbox_lockpick};
 for(unsigned i=0;i<count;i++){
  lv_obj_t*b=grid->buttons[i];lv_action_t expected=grid->kind==NOVA_TOOLS?tools[i]:grid->kind==NOVA_INFO?info[i]:show_group;assert(grid->actions[i]==expected);
  if(grid->kind==NOVA_TOOLS)assert(lv_obj_get_free_num(b)==(i==1?1:0));
  if(grid->kind==NOVA_DIAGNOSTICS)assert(lv_obj_get_free_num(b)==i);
  grid->armed=false;unsigned before=action_invocations;
  lv_btn_get_action(b,LV_BTN_ACTION_CLICK)(b);assert(action_invocations==before);
  if(lv_obj_get_click(b)){
   assert(grid->selected==i&&grid->armed);lv_btn_get_action(b,LV_BTN_ACTION_CLICK)(b);
   assert(action_invocations==before+1&&last_action==expected&&!grid->armed);
  } else {lv_btn_get_action(b,LV_BTN_ACTION_CLICK)(b);assert(action_invocations==before);}
  assert(!grid->open);

 }
 if(grid->kind==NOVA_TOOLS){assert(grid->holds[2]==create_window_emmc_partition_manager);nova_card_hold(grid->buttons[2]);assert(last_action==create_window_emmc_partition_manager);}
 grid->diagnostics_opened=false;nova_grid_select(page,0);action_invocations=0;last_action=NULL;
}
int main(int argc,char**argv){
SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
_set_error_mode(_OUT_TO_STDERR);
assert(argc==7);strcpy(font_root,argv[1]);
FILE*f=fopen(argv[2],"rb");assert(f);fseek(f,0,SEEK_END);u32 bytes=ftell(f);rewind(f);void*res=VirtualAlloc((void*)0xEE000000,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(res==(void*)0xEE000000);assert(fread(res,1,bytes,f)==bytes);fclose(f);
n_cfg.timedst=1;n_cfg.nova_logo_color=n_cfg.nova_icon_color=0x55C9F1;n_cfg.nova_selection_color=0x1C2937;n_cfg.theme_bg=0x141A19;n_cfg.nova_text_color=0xF2F5FA;n_cfg.nova_border_color=0x334050;n_cfg.nova_reflection_color=0x55C9F1;n_cfg.nova_menu_opacity=100;n_cfg.nova_icon_opacity=100;n_cfg.font_size=atoi(argv[5]);strcpy(n_cfg.font_file,getenv("NYX_PREVIEW_FONT")?getenv("NYX_PREVIEW_FONT"):"Lecture.nyxf");n_cfg.language=atoi(argv[4]);n_cfg.background_effect=0;n_cfg.background_speed=2;n_cfg.background_opacity=20;n_cfg.nova_halo=1;n_cfg.verification=1;h_cfg.autonogc=1;h_cfg.backlight=130;
h_cfg.rcm_patched=getenv("NYX_PREVIEW_PATCHED")!=NULL;
if(getenv("NYX_RGB_LOGO"))n_cfg.nova_logo_color=strtoul(getenv("NYX_RGB_LOGO"),NULL,16);
if(getenv("NYX_RGB_ICONS"))n_cfg.nova_icon_color=strtoul(getenv("NYX_RGB_ICONS"),NULL,16);
if(getenv("NYX_RGB_SELECTION"))n_cfg.nova_selection_color=strtoul(getenv("NYX_RGB_SELECTION"),NULL,16);
if(getenv("NYX_RGB_BACKGROUND"))n_cfg.theme_bg=strtoul(getenv("NYX_RGB_BACKGROUND"),NULL,16);
if(getenv("NYX_RGB_TEXT"))n_cfg.nova_text_color=strtoul(getenv("NYX_RGB_TEXT"),NULL,16);
if(getenv("NYX_RGB_BORDER"))n_cfg.nova_border_color=strtoul(getenv("NYX_RGB_BORDER"),NULL,16);
if(getenv("NYX_RGB_REFLECTION"))n_cfg.nova_reflection_color=strtoul(getenv("NYX_RGB_REFLECTION"),NULL,16);

if(getenv("NYX_PREVIEW_WALLPAPER")){strcpy(n_cfg.wallpaper_file,getenv("NYX_PREVIEW_WALLPAPER"));nyx_nova_wallpaper_load();assert(nova_wallpaper);}
if(getenv("NYX_PREVIEW_MENU_OPACITY"))n_cfg.nova_menu_opacity=atoi(getenv("NYX_PREVIEW_MENU_OPACITY"));
if(getenv("NYX_PREVIEW_ICON_OPACITY"))n_cfg.nova_icon_opacity=atoi(getenv("NYX_PREVIEW_ICON_OPACITY"));
lv_init();nyx_localization_init();nyx_font_manager_init();lv_disp_drv_t driver;lv_disp_drv_init(&driver);driver.disp_flush=flush;lv_disp_drv_register(&driver);
lv_theme_hekate_set_accents(n_cfg.nova_icon_color,n_cfg.nova_selection_color);lv_theme_hekate_set_colors(n_cfg.nova_text_color,n_cfg.nova_border_color,n_cfg.nova_reflection_color,n_cfg.nova_menu_opacity);lv_theme_t*theme=lv_theme_hekate_init(n_cfg.theme_bg,n_cfg.theme_color,(lv_font_t*)nyx_font_ui(20));lv_theme_set_current(theme);lv_style_copy(&hint_small_style,theme->label.hint);hint_small_style.text.font=nyx_font_small();hint_small_style.text.letter_space=1;lv_style_copy(&hint_small_style_white,&hint_small_style);nyx_nova_init(theme);
lv_obj_t *screen=lv_cont_create(NULL,NULL);lv_scr_load(screen);nyx_nova_screen(screen);
nyx_nova_footer(lv_layer_top(),nyx_options_save_all);lv_label_set_text(status_bar.time_temp,"18:42 · 28 °C");lv_label_set_text(status_bar.battery,"82 %");
lv_obj_t*root=NULL;
if(strcmp(argv[3],"about")&&strcmp(argv[3],"home")&&strcmp(argv[3],"carousel")&&strcmp(argv[3],"tools")&&strcmp(argv[3],"info")&&strcmp(argv[3],"options")&&strcmp(argv[3],"diagnostics")){
 lv_obj_t*tabs=nyx_nova_tabview(lv_scr_act());lv_obj_t*home=NULL;for(u32 i=0;i<6;i++){lv_obj_t*p=lv_tabview_add_tab(tabs,"");if(i==1)home=p;}
 home_tabview=tabs;_create_tab_home(theme,home);lv_tabview_set_tab_act(tabs,1,false);
 nyx_nova_navigation(lv_scr_act(),tabs,"hekate 6.5.4",_create_nova_quick_actions);lv_action_t quick[]={create_win_nyx_options,_create_mbox_reload,_create_mbox_reboot,_create_mbox_poweroff};nyx_nova_quick_actions(quick);
}
if(!strcmp(argv[3],"backup")||!strcmp(argv[3],"restore")){lv_obj_t b;memset(&b,0,sizeof b);lv_obj_set_free_num(&b,!strcmp(argv[3],"restore"));preview_backup_restore(&b);}
else if(!strcmp(argv[3],"usb"))preview_usb_tools(NULL);
else if(!strcmp(argv[3],"emummc"))preview_emummc_tools(NULL);
else if(!strcmp(argv[3],"diagnostic-group")){lv_obj_t b;memset(&b,0,sizeof b);lv_obj_set_free_num(&b,atoi(argv[6]));preview_show_group(&b);}
else if(!strcmp(argv[3],"diagnostic-results"))show_results(0);
else if(!strcmp(argv[3],"ram-service"))show_ram_service(NULL);
else if(!strcmp(argv[3],"setup")){first_time_clock_edit(NULL);root=lv_obj_get_child(lv_obj_get_child(lv_scr_act(),NULL),NULL);}
else if(!strcmp(argv[3],"appearance"))nyx_create_appearance_window(NULL);
else if(!strcmp(argv[3],"nyx"))create_win_nyx_options(NULL);
else if(!strcmp(argv[3],"colors"))_create_window_nyx_colors(NULL);
else if(!strcmp(argv[3],"boot-options"))_nova_boot_options(NULL);
else if(!strcmp(argv[3],"quick-actions"))_create_nova_quick_actions(NULL);
else {
 lv_obj_t*tabs=nyx_nova_tabview(lv_scr_act());
 if(nyx_font_size()>30){static lv_style_t st[4];int kinds[]={LV_TABVIEW_STYLE_BTN_REL,LV_TABVIEW_STYLE_BTN_PR,LV_TABVIEW_STYLE_BTN_TGL_REL,LV_TABVIEW_STYLE_BTN_TGL_PR};for(int j=0;j<4;j++){lv_style_copy(&st[j],lv_tabview_get_style(tabs,kinds[j]));st[j].text.font=nyx_font_small();lv_tabview_set_style(tabs,kinds[j],&st[j]);}}
 const char*names[]={"hekate 6.5.4",SYMBOL_HOME" Home",SYMBOL_TOOLS" Tools",SYMBOL_INFO" Console Info",SYMBOL_SETTINGS" Options",SYMBOL_CHIP" Diagnostic"};lv_obj_t*pages[6];
 for(int i=0;i<6;i++)pages[i]=lv_tabview_add_tab(tabs,names[i]);
 nyx_nova_navigation(lv_scr_act(),tabs,"hekate 6.5.4",_create_nova_quick_actions);lv_action_t quick[]={create_win_nyx_options,_create_mbox_reload,_create_mbox_reboot,_create_mbox_poweroff};nyx_nova_quick_actions(quick);
 int active=!strcmp(argv[3],"about")?0:(!strcmp(argv[3],"home")||!strcmp(argv[3],"carousel"))?1:!strcmp(argv[3],"tools")?2:!strcmp(argv[3],"info")?3:!strcmp(argv[3],"options")?4:5;
 root=pages[active];home_tabview=tabs;n_cfg.home_layout=!strcmp(argv[3],"carousel");
 nyx_nova_about(pages[0]);_create_tab_home(theme,pages[1]);create_tab_tools(theme,pages[2]);create_tab_info(theme,pages[3]);create_tab_options(theme,pages[4]);create_tab_diagnostics(theme,pages[5]);
 lv_tabview_set_tab_act(tabs,active,false);
 for(u32 i=1;i<6;i++){nova_navigate(nova_nav_buttons[i]);assert(lv_tabview_get_tab_act(tabs)==i);}
 lv_tabview_set_tab_act(tabs,active,false);nyx_nova_navigation_select(active);
 assert(lv_tabview_get_btns_hidden(tabs));assert(lv_obj_get_width(root)==1280);assert(lv_obj_get_height(root)==580);
 lv_area_t navigation_bounds={0,0,1279,75};for(int i=1;i<6;i++)verify_visible(nova_nav_buttons[i],navigation_bounds);
 if(active==2||active==3||active==5)verify_nova_bindings(root);

}
if(!strcmp(argv[3],"colors"))lv_tabview_set_tab_act(nova_theme.tabs,atoi(argv[6]),false);
if(!root){assert(nova_navigation_locks);for(u32 i=1;i<6;i++)assert(!lv_obj_get_click(nova_nav_buttons[i]));lv_obj_t*window=lv_win_get_from_btn(close_btn);lv_win_ext_t*ext=lv_obj_get_ext_attr(window);root=ext->page;
 if(!strcmp(argv[3],"appearance")||!strcmp(argv[3],"nyx")){lv_obj_t*tabs=lv_obj_get_child(lv_page_get_scrl(root),NULL);lv_tabview_set_tab_act(tabs,atoi(argv[6]),false);}}
verify_tree(lv_scr_act());verify_tree(lv_layer_top());nyx_readability_refresh(lv_scr_act());for(int i=0;i<4;i++){lv_tick_inc(40);lv_task_handler();}lv_refr_now();
for(u32 percent=0;percent<=100;percent++){preview_battery_percent=percent;_update_status_bar(NULL);assert(lv_obj_get_x(status_bar.battery)==1174&&lv_obj_get_y(status_bar.battery)==24);}
preview_battery_percent=82;
u32 temperatures[]={0,44,45,54,55,64,65,99};
for(u32 i=0;i<8;++i){preview_temperature=temperatures[i];_update_status_bar(NULL);char rgb[16];sprintf(rgb,"#%06X ",nyx_nova_temperature_rgb(preview_temperature));assert(strstr(lv_label_get_text(status_bar.time_temp),rgb));assert(strstr(lv_label_get_text(status_bar.time_temp),"18:42"));}
preview_temperature=getenv("NYX_PREVIEW_TEMPERATURE")?atoi(getenv("NYX_PREVIEW_TEMPERATURE")):28;_update_status_bar(NULL);lv_refr_now();
lv_area_t bounds;lv_obj_get_coords(root,&bounds);assert(bounds.y1>=0&&bounds.y2<656);
verify_visible(root,bounds);verify_reachable(root,bounds);
nyx_nova_reflection_pause(true);lv_refr_now();lv_area_t screen_bounds={0,0,1279,719};verify_glyph_pixels(lv_scr_act(),screen_bounds);verify_glyph_pixels(lv_layer_top(),screen_bounds);
nyx_nova_reflection_pause(false);
if(nova_brand_button&&(!strcmp(argv[3],"home")||!strcmp(argv[3],"tools"))){lv_action_t action=lv_btn_get_action(nova_brand_button,LV_BTN_ACTION_CLICK);assert(action==nova_navigate);u32 active=lv_tabview_get_tab_act(home_tabview);action(nova_brand_button);assert(lv_tabview_get_tab_act(home_tabview)==0);lv_tabview_set_tab_act(home_tabview,active,false);nyx_nova_navigation_select(active);}
if(!strcmp(argv[3],"home")||!strcmp(argv[3],"tools")||!strcmp(argv[3],"info")||!strcmp(argv[3],"diagnostics")||!strcmp(argv[3],"options"))assert(glyph_pixel_checks>=10);
if(!strcmp(argv[3],"options")){
 lv_obj_t*page=lv_page_get_scrl(root);unsigned switches=0;
 for(lv_obj_t*c=lv_obj_get_child(page,NULL);c;c=lv_obj_get_child(page,c))for(lv_obj_t*b=lv_obj_get_child(c,NULL);b;b=lv_obj_get_child(c,b))if(lv_obj_get_signal_func(b)==nova_toggle_signal){
  nova_toggle_ext*e=lv_obj_get_ext_attr(b);assert(e->knob&&e->track&&e->state);lv_action_t action=lv_btn_get_action(b,LV_BTN_ACTION_CLICK);
  for(lv_obj_t*l=lv_obj_get_child(b,NULL);l;l=lv_obj_get_child(b,l))if(type_is(l,"lv_label")&&l!=e->state)assert(lv_obj_get_x(l)+lv_obj_get_width(l)<lv_obj_get_x(e->track));
  bool before=(lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0;action(b);assert(((lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0)!=before);assert(lv_obj_get_x(e->knob)==(before?5:35));action(b);assert(((lv_btn_get_state(b)&LV_BTN_STATE_TGL_REL)!=0)==before);++switches;
 }assert(switches==4);ini_changes_made=nyx_changes_made=false;lv_obj_set_opa_scale(status_bar.mid,LV_OPA_TRANSP);lv_obj_set_click(status_bar.mid,false);
 u32 original_language=n_cfg.language,original_size=n_cfg.font_size;
 nyx_appearance_open_language((original_language+1)%7);assert(appearance_pending.language==(original_language+1)%7&&n_cfg.language==original_language);assert(lv_tabview_get_tab_act(appearance_tabs)==0);appearance_close(close_btn);assert(!appearance_open&&!close_btn);
 nyx_appearance_open_size(32);assert(appearance_pending.font_size==32&&n_cfg.font_size==original_size);assert(lv_tabview_get_tab_act(appearance_tabs)==1);appearance_close(close_btn);assert(!appearance_open&&!close_btn);
 nyx_appearance_open_font();assert(lv_tabview_get_tab_act(appearance_tabs)==1);appearance_close(close_btn);assert(!appearance_open&&!close_btn);
}
if(!strcmp(argv[3],"appearance")){
 u32 language=n_cfg.language;lv_obj_t*choice=find_choice(root,AP_LANGUAGE,2);assert(choice);lv_btn_get_action(choice,LV_BTN_ACTION_CLICK)(choice);assert(appearance_pending.language==2&&n_cfg.language==language);
 choice=find_choice(root,AP_SIZE,8);assert(choice);lv_btn_get_action(choice,LV_BTN_ACTION_CLICK)(choice);assert(appearance_pending.font_size==36&&n_cfg.font_size==(u32)atoi(argv[5]));
 char before[64];strcpy(before,appearance_pending.font_file);lv_obj_t step;memset(&step,0,sizeof step);lv_obj_set_free_num(&step,1);appearance_font_step(&step);lv_obj_set_free_num(&step,0);appearance_font_step(&step);assert(!strcmp(before,appearance_pending.font_file));
}
if(!strcmp(argv[3],"colors")){
 nyx_config before=n_cfg;u32 roles[]={NOVA_COLOR_BACKGROUND,NOVA_COLOR_TEXT,NOVA_COLOR_BORDER,NOVA_COLOR_LOGO,NOVA_COLOR_ICONS,NOVA_COLOR_SELECTION};
 u32 *fields[]={&nova_theme.pending.theme_bg,&nova_theme.pending.nova_text_color,&nova_theme.pending.nova_border_color,&nova_theme.pending.nova_logo_color,&nova_theme.pending.nova_icon_color,&nova_theme.pending.nova_selection_color};
 u32 original[6];for(u32 i=0;i<6;++i)original[i]=*fields[i];
 for(u32 target=0;target<6;++target){nova_theme.target=roles[target];assert(theme_target_color()==fields[target]);*theme_target_color()=0xAABBCC;theme_controls_refresh();
  for(u32 i=0;i<6;++i)assert(*fields[i]==(i==target?0xAABBCC:original[i]));*fields[target]=original[target];}

 assert(!memcmp(&before,&n_cfg,sizeof n_cfg));nova_theme.target=0;theme_controls_refresh();
 theme_reset(NULL);assert(nova_theme.pending.nova_menu_opacity==100&&nova_theme.pending.nova_icon_opacity==100&&!nova_theme.pending.wallpaper_file[0]);assert(!memcmp(&before,&n_cfg,sizeof n_cfg));
}

if(!strcmp(argv[3],"setup")){assert(clock_language==n_cfg.language&&clock_ctxt.year&&clock_ctxt.month&&clock_ctxt.day&&clock_ctxt.hour&&clock_ctxt.min);assert(type_is(root,"lv_mbox"));}
if(diagnostic_last_holder){diag_tiles_ext*ext=lv_obj_get_ext_attr(diagnostic_last_holder);u32 total=ext->count;unsigned visits=0;
 for(u32 page=0;page<(total+8)/9;++page){ext->page=page;diagnostic_tiles_fill(diagnostic_last_holder);lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);for(lv_obj_t*c=lv_obj_get_child(ext->tiles,NULL);c;c=lv_obj_get_child(ext->tiles,c))for(lv_obj_t*b=lv_obj_get_child(c,NULL);b;b=lv_obj_get_child(c,b))if(type_is(b,"lv_btn")){u32 id=lv_obj_get_free_num(b);assert((id&~ONE_PROBE)<42);lv_action_t action=lv_btn_get_action(b,LV_BTN_ACTION_CLICK);assert(action==(ext->results?show_report:run_selection));++visits;}}
 assert(visits==total);ext->page=0;diagnostic_tiles_fill(diagnostic_last_holder);lv_refr_now();
}

if(nova_wallpaper){lv_obj_t *back=lv_obj_get_child_back(lv_scr_act(),NULL);assert(type_is(back,"lv_img")&&!lv_obj_get_click(back)&&lv_obj_is_protected(back,LV_PROTECT_PARENT));assert(lv_img_get_src(back)==nova_wallpaper);}
if(getenv("NYX_TEST_NAVIGATION"))verify_wallpaper_navigation();
if(!getenv("NYX_PREVIEW_NO_DUMP"))dump("preview.ppm");
if(getenv("NYX_CAPTURE_REFLECTION")){
 FILE*frames=fopen("reflection.bgrx","wb");assert(frames&&sizeof(lv_color_t)==4);
 for(u32 frame=0;frame<72;++frame){lv_tick_inc(50);lv_task_handler();lv_refr_now();assert(fwrite(screen_pixels,sizeof(lv_color_t),LV_HOR_RES*LV_VER_RES,frames)==LV_HOR_RES*LV_VER_RES);}
 fclose(frames);
}
if((!strcmp(argv[3],"tools")||!strcmp(argv[3],"home")||!strcmp(argv[3],"carousel"))&&!strcmp(argv[4],"0")&&!strcmp(argv[5],"30")){
 nyx_background_pause(true);for(int i=0;i<20;i++){lv_tick_inc(50);lv_task_handler();}lv_refr_now();
 rendered_pixels=0;for(int i=0;i<60;i++){lv_tick_inc(50);lv_task_handler();}lv_refr_now();assert(rendered_pixels==0);
 nyx_background_pause(false);rendered_pixels=0;unsigned long maximum=0,total=0;
 for(int i=0;i<72;i++){unsigned long before=rendered_pixels;lv_tick_inc(50);lv_task_handler();lv_refr_now();unsigned long pixels=rendered_pixels-before;total+=pixels;if(pixels>maximum)maximum=pixels;}
 assert(total>0&&maximum<100000);
 printf("PASS: Nova reflection max=%lu total=%lu pixels over 3600ms; pause=0\n",maximum,total);
 nyx_nova_reflection_reset();rendered_pixels=0;lv_refr_now();rendered_pixels=0;for(int i=0;i<60;i++){lv_tick_inc(50);lv_task_handler();}lv_refr_now();assert(rendered_pixels==0);

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
jobs=[('tools',1,20,0),('tools',4,36,0),('info',1,20,0),('options',1,20,0),('options',4,36,0),('diagnostics',4,36,0),('appearance',1,30,0),('appearance',1,30,1),('appearance',1,30,2),('nyx',4,36,0),('nyx',4,36,1),('colors',1,30,0)]
jobs.extend([('home',1,30,0),('carousel',4,36,0),('tools',1,30,0),('info',1,30,0),('options',1,30,0),('diagnostics',1,30,0),('boot-options',1,30,0),('quick-actions',1,30,0)])
jobs.extend([('about',1,30,0),('backup',1,30,0),('restore',1,30,0),('usb',1,30,0),('emummc',1,30,0),('diagnostic-group',1,30,2),('diagnostic-results',1,30,0),('ram-service',1,30,0),('colors',1,30,1)])
if '--check-all' in sys.argv:
    modes=[('about',0),('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('appearance',0),('appearance',1),('appearance',2),('nyx',0),('nyx',1),('colors',0),('colors',1),('boot-options',0),('quick-actions',0),('setup',0)]
    modes.extend([('backup',0),('restore',0),('usb',0),('emummc',0),('diagnostic-results',0),('ram-service',0)]+[('diagnostic-group',i)for i in range(1,11)])
    jobs=[(screen,language,size,tab) for screen,tab in modes for language in range(7) for size in range(20,38,2)]
if '--case' in sys.argv:
    at=sys.argv.index('--case');jobs=[(sys.argv[at+1],int(sys.argv[at+2]),int(sys.argv[at+3]),int(sys.argv[at+4]))]
environment=os.environ.copy()
if '--trace' in sys.argv:environment['NYX_TRACE']='1'
if '--check-all' in sys.argv:environment['NYX_PREVIEW_NO_DUMP']='1'
for screen,language,size,tab in jobs:
    try:
        run=subprocess.run([str(exe),str(nyx/'fonts/sd'),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],cwd=directory,capture_output=True,text=True,timeout=20,env=environment)
    except subprocess.TimeoutExpired as error:
        partial=error.stderr or b''
        if isinstance(partial,bytes):partial=partial.decode('utf-8',errors='replace')
        raise RuntimeError(f'{screen} {language} {size} {tab} timed out. Last trace:\n'+partial[-6000:]) from error
    if run.returncode:raise RuntimeError(f'{screen} {language} {size} {tab}: '+run.stdout+run.stderr)
    performance=re.search(r'Nova reflection max=(\d+) total=(\d+)',run.stdout)
    if performance:
        result={'reflection_period_ms':50,'max_redraw_pixels_per_frame':int(performance[1]),'total_redraw_pixels_over_3600ms':int(performance[2]),'paused_redraw_pixels':0,'platform':'host LVGL, not console FPS'}
        (directory/('nova-performance-'+screen+'.json')).write_text(json.dumps(result,indent=2));(directory/'nova-performance.json').write_text(json.dumps(result,indent=2))
        print('PASS: white diagonal full-tile sheen; '+performance[0],flush=True)
    if '--check-all' not in sys.argv:
        target=work/f'preview-v13-{screen}-{language}-{size}-{tab}.png'
        Image.open(directory/'preview.ppm').save(target)
        print(target.name,run.stdout.strip(),flush=True)
print(f'PASS: real-LVGL UI geometry and selector checks across {len(jobs)} screens; visible controls stay above the status bar; two-tap actions, independent colors and bounded reflections checked.',flush=True)
