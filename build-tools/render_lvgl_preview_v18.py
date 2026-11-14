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
directory=work/'lvgl-preview-v18';directory.mkdir(exist_ok=True)
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
static lv_style_t hint_small_style, hint_small_style_white,mbox_darken,monospace_text;
typedef struct _emmc_tool_gui_t
{
	lv_obj_t *label_log;
	lv_obj_t *label_info;
	lv_obj_t *label_pct;
	lv_obj_t *label_finish;
	lv_obj_t *bar;
	lv_style_t *bar_teal_bg;
	lv_style_t *bar_teal_ind;
	lv_style_t *bar_white_bg;
	lv_style_t *bar_white_ind;
	lv_style_t *bar_orange_bg;
	lv_style_t *bar_orange_ind;
	char *txt_buf;
	char *base_path;
	bool raw_emummc;
} emmc_tool_gui_t;
static void nyx_window_toggle_buttons(lv_obj_t*w,bool disable){(void)w;(void)disable;}
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
static int f_open(FIL*f,const char*path,int mode){(void)mode;char full[1200];snprintf(full,sizeof(full),strstr(path,"icone/")==path?"%s/../../../../icone/%s":strstr(path,"wallpapers/")==path?"%s/../../../../wallpapers/%s":"%s/%s",font_root,strrchr(path,'/')?strrchr(path,'/')+1:path);f->fp=fopen(full,"rb");if(!f->fp)return 1;fseek(f->fp,0,SEEK_END);f->size=ftell(f->fp);rewind(f->fp);return 0;}
static int f_close(FIL*f){return fclose(f->fp);}
static int f_lseek(FIL*f,u32 off){return fseek(f->fp,off,SEEK_SET);}
static int f_read(FIL*f,void*p,u32 length,UINT*actual){*actual=fread(p,1,length,f->fp);return ferror(f->fp)?1:0;}
static int f_stat(const char*p,void*x){(void)p;(void)x;return 0;}
static int f_opendir(DIR*d,const char*p){d->index=0;d->wallpaper=!strcmp(p,"icone")?2:!strcmp(p,"wallpapers");return 0;}
static int f_readdir(DIR*d,FILINFO*f){static const char*names[]={"Lecture.nyxf","Lecture-renforcee.nyxf","Lecture-fixe.nyxf",""};strcpy(f->fname,d->wallpaper?(d->index++?"":d->wallpaper==2?"launch.png":"Nova-Carbon.bmp"):names[d->index<3?d->index++:3]);f->fattrib=0;return 0;}
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
headers=stripped(nyx/'nova_version.h')+'\n'+stripped(nyx/'frontend/gui_partition_plan.h')+'\n'+'\n'.join(stripped(p)for p in [nyx/'fonts/font_manager.h',nyx/'localization.h',nyx/'fonts/unicode_fonts.h',nyx/'frontend/gui_nova.h',nyx/'frontend/gui_color_picker.h',nyx/'frontend/nova_image.h',nyx/'frontend/gui_options.h',nyx/'frontend/gui_readability.h',nyx/'frontend/gui_layout.h',nyx/'frontend/gui_choices.h',nyx/'frontend/gui_appearance.h'])
body=common+headers+stripped(nyx/'fonts/font_manager.c')+stripped(nyx/'localization.c',True)
body+='static bool legacy_scale_font(const lv_font_t *source, u32 height, managed_font *target, bool icons, bool fixed)\n{\n    u32 codepoints[MAX_GLYPHS], count = make_codepoints(codepoints, icons), total = 0;\n    u32 widths[MAX_GLYPHS], source_height = source->h_px;\n    u32 fixed_width = (10u * height + source_height / 2) / source_height;\n    for (u32 i = 0; i < count; ++i) {\n        u32 source_width = lv_font_get_width(source, codepoints[i]);\n        widths[i] = fixed ? fixed_width : (source_width * height + source_height / 2) / source_height;\n        if (!widths[i]) widths[i] = height / 2;\n        if (widths[i] > 128) return false;\n        total += ((widths[i] + 1) / 2) * height;\n    }\n    if (!allocate_font(target, count, total, height)) return false;\n    memset(target->bitmap, 0, total);\n    u32 offset = 0;\n    for (u32 i = 0; i < count; ++i) {\n        u32 cp = codepoints[i], width = widths[i];\n        const u8 *source_bitmap = lv_font_get_bitmap(source, cp);\n        u32 source_width = lv_font_get_width(source, cp), source_bpp = lv_font_get_bpp(source, cp);\n        u32 source_stride = source_bpp == 8 ? source_width : (source_width + 1) / 2;\n        target->codepoints[i] = cp; target->glyphs[i].w_px = width; target->glyphs[i].glyph_index = offset;\n        if (source_bitmap && source_width && (source_bpp == 4 || source_bpp == 8)) {\n            for (u32 y = 0; y < height; ++y) {\n                u32 source_y = y * source_height / height;\n                for (u32 x = 0; x < width; ++x) {\n                    u32 source_x = x * source_width / width;\n                    u8 byte = source_bitmap[source_y * source_stride + (source_bpp == 8 ? source_x : source_x / 2)];\n                    u8 pixel = source_bpp == 8 ? (byte + 8) / 17 : (source_x & 1 ? byte & 15 : byte >> 4);\n                    target->bitmap[offset + y * ((width + 1) / 2) + x / 2] |= pixel << (x & 1 ? 0 : 4);\n                }\n            }\n        }\n        offset += ((width + 1) / 2) * height;\n    }\n    target->font.unicode_first = target->codepoints[0]; target->font.unicode_last = target->codepoints[count - 1];\n    if (fixed) target->font.monospace = fixed_width;\n    return true;\n}\n'
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
static void display_backlight_brightness(int value,int delay){(void)delay;assert(value>=10&&value<=200);}

"""
body+=function(opts,'_slider_brightness_action')+function(opts,'_save_nyx_options_action')
for name in ['_autoboot_delay_action','_data_verification_action','_home_screen_action','auto_nogc_toggle','auto_hos_poweroff_toggle','_update_r2p_action','_entries_columns_action']:body+=function(opts,name)
body+='\nstatic void _check_nyx_changes(void){assert(!nyx_changes_made); }\n'+function(opts,'_action_win_nyx_options_close')
body+=function(opts,'nyx_options_appearance_active')
body+=opts[opts.index('static const u16 theme_colors[18]'):opts.index('typedef struct _time_edit_obj_t')]
body+=opts[opts.index('/* Compact pages keep controls'):]
body+=stripped(nyx/'frontend/gui_color_picker.c')+stripped(nyx/'frontend/gui_nova_theme.c')+stripped(nyx/'frontend/gui_nova_report.c')
rtc=(project/'bdk/rtc/max77620-rtc.c').read_text()
at=rtc.index('u32 max77620_rtc_date_to_epoch(');end=rtc.index('\n}',at)+2
body+=rtc[at:end]+'\n'
body+='\n'.join(re.findall(r'^#define CLOCK_[^\n]*',opts,re.M))+'\n'
body+=function(gui,'nyx_create_onoff_button')+function(gui,'nyx_mbox_action')
body+=opts[opts.index('typedef struct _time_edit_obj_t'):opts.index('static lv_res_t _joycon_info_dump_action(')]

emmc=(nyx/'frontend/gui_emmc_tools.c').read_text(encoding='utf-8')
body+=emmc[emmc.index('typedef struct _emmc_backup_buttons_t'):emmc.index('static void _create_window_backup_restore')]
body+='\ntypedef int emmcPartType_t;\n#define PART_BOOT 0\nstatic emmc_tool_gui_t preview_transfer;\nstatic void dump_emmc_selected(emmcPartType_t type,emmc_tool_gui_t *context){\n (void)type;preview_transfer=*context;char text[2048];\n s_printf(text,"01: BOOT1\\n%s: 0x00000000 - 0x00001FFF\\n\\n",nyx_tr("Range"));lv_label_set_text(context->label_info,text);\n s_printf(text,nyx_tr("#96FF00 SD Card free space:# %d MiB\\n#96FF00 Total backup size:# %d MiB\\n\\n"),803971,4);lv_label_ins_text(context->label_info,LV_LABEL_POS_LAST,text);\n s_printf(text,nyx_tr("#96FF00 Filepath:#\\n%s\\n#96FF00 Filename:# #FF8000 %s#"),"backup/1034b3fe/","BOOT1");lv_label_ins_text(context->label_info,LV_LABEL_POS_LAST,text);\n lv_bar_set_value(context->bar,100);lv_label_set_text(context->label_pct,"100%");lv_label_set_text(context->label_finish,"Finished and verified!");\n lv_label_set_text(context->label_log,"BOOT0: OK\\nBOOT1: OK\\nSHA-256: 0123456789ABCDEF\\nVérification · Проверка");\n}\nstatic void restore_emmc_selected(emmcPartType_t type,emmc_tool_gui_t *context){dump_emmc_selected(type,context);}\n'
body+=function(emmc,'_create_window_backup_restore')
body+='\nACTION(_emmc_backup_buttons_decider)\n'
body+=function(emmc,'_emmc_backup_buttons_raw_toggle')
body+=function(emmc,'create_window_backup_restore_tool').replace('create_window_backup_restore_tool(', 'preview_backup_restore(')
tools=(nyx/'frontend/gui_tools.c').read_text(encoding='utf-8')
body+='\nstatic int usb_msc_emmc_read_only;\nACTION(action_ums_sd) ACTION(_action_ums_emmc_gpp) ACTION(_action_ums_emmc_boot0) ACTION(_action_ums_emmc_boot1) ACTION(_action_ums_emuemmc_gpp) ACTION(_action_ums_emuemmc_boot0) ACTION(_action_ums_emuemmc_boot1) ACTION(_action_hid_jc)\n'
body+=function(tools,'_emmc_read_only_toggle')
body+=function(tools,'_create_window_usb_tools').replace('_create_window_usb_tools(', 'preview_usb_tools(')

partition=(nyx/'frontend/gui_tools_partition_manager.c').read_text(encoding='utf-8')
body+='\n#define AU_ALIGN_SECTORS 0x8000\n#define ALIGN_DOWN(x,a) ((x)&~((a)-1))\n#define HOS_USER_SECTOR 0x53C000\n'
body+='\n'.join(re.findall(r'^#define (?:EMU_|AND_SYS_)[^\n]*',partition,re.M))+'\n'
body+='\ntypedef struct{u32 sec_cnt;}sdmmc_storage_t;\ntypedef struct{u32 unused;}mbr_t;\nstatic sdmmc_storage_t sd_storage,emmc_storage;\n'
start=partition.index('typedef struct _partition_ctxt_t');end=partition.index('typedef struct _l4t_flasher_ctxt_t',start)
body+=partition[start:end]
body+='\nstatic partition_ctxt_t part_info;static lv_obj_t *btn_flash_l4t,*btn_flash_android;\nstatic u32 fixture_model;\nenum{FUSE_NX_HW_TYPE_IOWA=1,FUSE_NX_HW_TYPE_HOAG=2,FUSE_NX_HW_TYPE_AULA=3,EMMC_GPP=0};\nstatic u32 fuse_read_hw_type(void){return fixture_model;}\nstatic int emmc_initialize(bool b){(void)b;return 0;}\nstatic void emmc_set_partition(u32 p){(void)p;}\nstatic void emmc_end(void){}\nACTION(_action_part_manager_ums_sd) ACTION(_action_check_flash_linux) ACTION(_action_flash_android) ACTION(_action_fix_mbr_gpt) ACTION(_create_mbox_partitioning_next_legacy)\nstatic void nova_partition_refresh(void);\n'
body+=function(partition,'_update_partition_bar')+function(partition,'_action_slider_emu')+function(partition,'_action_slider_l4t')+function(partition,'_action_slider_and')
body+=stripped(nyx/'frontend/gui_partition_plan.c')+(nyx/'frontend/gui_partition_nova.inc').read_text(encoding='utf-8')
body+=r"""
static lv_res_t preview_partition(const char *mode,u32 options){
 fixture_model=getenv("NYX_TEST_MODEL")?atoi(getenv("NYX_TEST_MODEL")):0;
 memset(&part_info,0,sizeof part_info);part_info.emmc=!strcmp(mode,"partition-emmc");
 part_info.total_sct=(getenv("NYX_TEST_CAPACITY")?atoi(getenv("NYX_TEST_CAPACITY")):128)*2097152u;
 part_info.storage=part_info.emmc?&emmc_storage:&sd_storage;part_info.storage->sec_cnt=part_info.total_sct+(part_info.emmc?HOS_USER_SECTOR:0);
 part_info.emmc_size_mb=fixture_model==3?59640:29820;part_info.emmc_is_64gb=fixture_model==3;
 part_info.hos_min_size=part_info.emmc?1024:2048;part_info.hos_size=(part_info.total_sct>>11)-16;
 lv_obj_t *window=nyx_create_standard_window(part_info.emmc?"eMMC Partition Manager":"SD Partition Manager",NULL);
 if(!part_info.emmc)lv_win_add_btn(window,NULL,"Fix Hybrid MBR/GPT",_action_fix_mbr_gpt);
 nova_partition_create_ui(window);
 partition_nova.option[0]=options%3;partition_nova.option[1]=options/3%3;
 partition_nova.preset[0]=partition_nova.option[0]>0?partition_nova.option[0]-1:0;
 partition_nova.preset[1]=partition_nova.option[1]>0?partition_nova.option[1]-1:0;
 if(!strcmp(mode,"partition-expert")){
  partition_nova.mode=1;part_info.l4t_size=16384;part_info.and_size=16384;
  part_info.emu_size=part_info.emmc_is_64gb?EMU_64GB_FULL:EMU_32GB_FULL;
 }
 nova_partition_build_controls();return LV_RES_OK;
}

static void verify_partition_state(void){
 assert(lv_obj_get_y(lv_page_get_scrl(partition_nova.page))==0);
 nova_partition_plan p;nova_plan_result result=nova_partition_current(&p);
 assert((result==NOVA_PLAN_OK)==lv_obj_get_click(part_info.partition_button));
 lv_obj_t *message=partition_nova.message,*summary=partition_nova.summary;
 assert(lv_obj_get_y(summary)+lv_obj_get_height(summary)<=lv_obj_get_y(message));
 assert(lv_obj_get_y(message)+lv_obj_get_height(message)<=lv_obj_get_height(lv_obj_get_parent(message))-12);
 for(u32 i=0;i<3;++i)if(partition_nova.sliders[i]){
  lv_area_t slider;lv_obj_get_coords(partition_nova.sliders[i],&slider);
  assert(slider.y1>=NYX_NOVA_HEADER&&slider.y2<NYX_NOVA_BOTTOM);
 }
 if(result==NOVA_PLAN_OK){
  u32 sum=0;for(u32 i=0;i<4;++i)if(!lv_obj_get_hidden(partition_nova.segments[i]))sum+=lv_obj_get_width(partition_nova.segments[i]);assert(sum==1164);
  assert(part_info.hos_size==p.fat_mib);
 }
}
static void exercise_partition_controls(void){
 u32 original_mode=partition_nova.mode,model=partition_nova.model,options[]={partition_nova.option[0],partition_nova.option[1]};
 u32 emu=part_info.emu_size,linux=part_info.l4t_size,android=part_info.and_size;bool double_emu=part_info.emu_double;
 lv_obj_t button;memset(&button,0,sizeof button);
 if(!part_info.emmc&&!original_mode){
  /* Use the actual choice callback, including disabled preset controls. */
  lv_obj_t *no=NULL,*yes=NULL;
  for(lv_obj_t *child=lv_obj_get_child(partition_nova.enabled[0],NULL);child;child=lv_obj_get_child(partition_nova.enabled[0],child))if(lv_obj_get_free_num(child))yes=child;else no=child;
  if(!original_mode&&no&&yes){
   lv_btn_get_action(no,LV_BTN_ACTION_CLICK)(no);assert(!part_info.l4t_size);
   for(lv_obj_t *child=lv_obj_get_child(partition_nova.presets[0],NULL);child;child=lv_obj_get_child(partition_nova.presets[0],child))assert(!lv_obj_get_click(child));
   lv_btn_get_action(yes,LV_BTN_ACTION_CLICK)(yes);assert(part_info.l4t_size==(partition_nova.preset[0]+1)*16384);
   partition_nova.model=model<2?2:0;nova_partition_refresh();assert(partition_nova.result==NOVA_PLAN_MODEL&&!lv_obj_get_click(part_info.partition_button));
   partition_nova.model=model;
  }
 }
 partition_nova.mode=1;part_info.emu_size=part_info.l4t_size=part_info.and_size=0;part_info.emu_double=false;nova_partition_build_controls();
 for(u32 row=part_info.emmc?1:0;row<3;++row){
  lv_obj_t *slider=partition_nova.sliders[row];assert(slider);
  for(u32 value=0;value<=(row?44:44);++value){
   lv_slider_set_value(slider,value);lv_slider_get_action(slider)(slider);verify_partition_state();
  }
  part_info.emu_size=part_info.l4t_size=part_info.and_size=0;part_info.emu_double=false;nova_partition_refresh();
 }
 if(!part_info.emmc&&(part_info.total_sct>>21)>=128){
  lv_obj_t *slider=partition_nova.sliders[0];lv_slider_set_value(slider,22);lv_slider_get_action(slider)(slider);
  assert(part_info.emu_size==(part_info.emmc_is_64gb?59680:29856)&&!part_info.emu_double);
  lv_slider_set_value(slider,44);lv_slider_get_action(slider)(slider);
  assert(part_info.emu_size==(part_info.emmc_is_64gb?119360:59712)&&part_info.emu_double);
 }
 part_info.emu_size=part_info.l4t_size=part_info.and_size=0;part_info.emu_double=false;
 lv_obj_t *slider=partition_nova.sliders[2];lv_slider_set_value(slider,16);lv_slider_get_action(slider)(slider);
 if((part_info.total_sct>>21)>=32)assert(part_info.and_size==16384&&lv_slider_get_value(slider)==16);
 partition_nova.mode=original_mode;partition_nova.model=model;partition_nova.option[0]=options[0];partition_nova.option[1]=options[1];
 part_info.emu_size=emu;part_info.l4t_size=linux;part_info.and_size=android;part_info.emu_double=double_emu;nova_partition_build_controls();verify_partition_state();
}
static lv_res_t preview_archive(void){
 lv_obj_t *win=nyx_create_standard_window("Fix Archive Bit (All folders)",NULL),*labels[3];nyx_nova_archive_view(win,labels);
 lv_label_set_text(labels[0],"#00DDFF Traversing all SD card files!#\nThis may take some time...");
 lv_label_set_text(labels[1],"/Nintendo/Contents/registered/00000000000000000000000000000000.nca");
 lv_label_set_text(labels[2],"#96FF00 Done! Archive bits fixed:# #FF8000 124 unset and 35 set!#");return LV_RES_OK;
}
static lv_obj_t *fixture_log_tabs;
static lv_res_t preview_package(void){
 lv_obj_t *win=nyx_create_standard_window("Dump package1/2",NULL),*logs[2];nyx_nova_package_logs(win,logs);
 fixture_log_tabs=lv_obj_get_parent(lv_obj_get_parent(lv_obj_get_parent(lv_obj_get_parent(logs[0]))));
 char text[16000];for(u32 i=0;i<2;++i){
  s_printf(text,"#00DDFF Found %s pkg1 ('20260424090133')#\n\nPackage1: OK\nWarmboot: OK\nSecure monitor: OK\nLoader: OK\nPackage2: OK\nINI1: OK\n\n",i?"Safe":"Main");
  for(u32 row=0;row<60;++row){char line[160];s_printf(line,"- Extracted module_%02d.kip1  %s\n",row,row==59?"COMPLETE END OF LOG":"OK");strcat(text,line);}
  lv_label_set_text(logs[i],text);
 }return LV_RES_OK;
}
"""
body+=r"""
typedef struct{u32 enabled,sector;char*path,*nintendo_path;}emummc_cfg_t;
static lv_obj_t*emummc_manage_window;static lv_res_t(*emummc_tools)(lv_obj_t*);
static void load_emummc_cfg(emummc_cfg_t*c){c->enabled=1;c->sector=0x100000;c->path=strdup("emuMMC/RAW1");c->nintendo_path=strdup("emuMMC/RAW1/Nintendo");}
ACTION(_create_change_emummc_window) ACTION(_create_mbox_emummc_create) ACTION(_create_mbox_emummc_migrate)
static lv_res_t create_win_emummc_tools(lv_obj_t*);
"""
body+=function((nyx/'frontend/gui_emummc_tools.c').read_text(encoding='utf-8'),'create_win_emummc_tools').replace('create_win_emummc_tools(', 'preview_emummc_tools(')
body+='\ntypedef struct{char *name[64];char data[64*256];}dirlist_t;\ntypedef struct _emummc_images_t\n{\n\tdirlist_t *dirlist;\n\tu32 part_sector[3];\n\tu32 part_type[3];\n\tu32 part_end[3];\n\tchar part_path[3 * 128];\n\tlv_obj_t *win;\n    lv_obj_t *file_items,*file_counter;\n    u32 file_count,file_page;\n} emummc_images_t;\nstatic emummc_images_t *emummc_img;\nstatic u32 preview_saved_emu_index,preview_saved_emu_sector;\nstatic char preview_saved_emu_path[512];\nstatic void save_emummc_cfg(u32 index,u32 sector,const char *path){preview_saved_emu_index=index;preview_saved_emu_sector=sector;strcpy(preview_saved_emu_path,path);}\nstatic void _create_emummc_saved_mbox(void){}\n'
change=(nyx/'frontend/gui_emummc_tools.c').read_text()
for name in ['_save_raw_emummc_cfg_action','_save_file_emummc_cfg_action','_fill_emummc_files','_step_emummc_files','_create_change_emummc_ui']:body+=function(change,name)



joy=(project/'bdk/input/joycon.h').read_text();body+=joy[joy.index('typedef struct _jc_bt_conn_t'):joy.index('typedef struct _jc_calib_t')]
touch=(project/'bdk/input/touch.h').read_text();body+='\n#define FTS4_EVENT_SIZE 8\n'+touch[touch.index('typedef struct _touch_event_t'):touch.index('typedef struct _touch_panel_info_t')]
body+='\n#define BTN_POWER 1\n#define BTN_VOL_UP 2\n#define BTN_VOL_DOWN 4\nstatic u32 fixture_power;static u32 btn_read(void){return fixture_power;}\nstatic u32 get_tmr_ms(void){return lv_tick_get();}\nstatic bool gui_diagnostics_inputs_ready(void);\n'
body+=stripped(nyx/'frontend/gui_stick.h')+stripped(nyx/'frontend/gui_stick.c')
body+=stripped(nyx/'diagnostics/nyx_diagnostics.h')+stripped(nyx/'diagnostics/component_summary.h')+stripped(nyx/'diagnostics/diag_format.h')+re.sub(r'\btext\(', 'diagnostic_format_text(',stripped(nyx/'diagnostics/diag_format.c'))
body+=stripped(nyx/'frontend/gui_interactive.h')+stripped(nyx/'frontend/gui_interactive.c')
diag=(nyx/'frontend/gui_diagnostics.c').read_text(encoding='utf-8')
groups='\nvoid nyx_components_window(lv_action_t,lv_action_t);\n'+diag[diag.index('static const struct {'):diag.index('static lv_obj_t *text(')]
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

body+=stripped(nyx/'diagnostics/component_summary.h')
body+=r"""
static const nyx_diag_finding_t component_fixture[]={
 {"charger_id",2,"I2C no reply"},{"usb_pd",2,"I2C no reply"},
 {"dram_sym",0,"2 channels"},{"plls",0,"locked"},{"fuel_devname",0,"0x00AC"}
};
u32 nyx_diag_finding_count(void){return sizeof(component_fixture)/sizeof(*component_fixture);}
bool nyx_diag_finding(u32 i,nyx_diag_finding_t *out){if(i>=nyx_diag_finding_count())return false;*out=component_fixture[i];return true;}
bool nyx_diag_storage_ready(bool emmc){(void)emmc;return true;}
"""
body+=stripped(nyx/'diagnostics/component_summary.c')+stripped(nyx/'frontend/gui_component_summary.c')
body+=r"""
static void preview_component_results(void){
 for(u32 i=0;i<42;++i)preview_probes[i].state=NYX_DIAG_OBSERVATION;
 preview_probes[10].state=preview_probes[11].state=NYX_DIAG_FAIL;
 preview_probes[14].state=preview_probes[15].state=NYX_DIAG_PASS;
 nyx_components_window(show_report,export_report);
}
"""

ram=diag[diag.index('static const char *ram_state'):diag.index('static lv_res_t launch_ram')]
body+=ram+function(diag,'ram_controls_refresh')+function(diag,'ram_mode_choice')+function(diag,'ram_pattern_choice')+function(diag,'show_ram_service')
body+=function(diag,'show_group').replace('show_group(', 'preview_show_group(')

constructor=diag[diag.index('void create_tab_diagnostics('):diag.index('    nyx_ram_handoff_t *saved')]
body+=constructor+'}\n'
body+='\nACTION(_create_window_home_launch) ACTION(_create_mbox_payloads) ACTION(create_win_emummc_tools) ACTION(_create_mbox_reload) ACTION(_create_mbox_reboot) ACTION(_create_mbox_poweroff)\n'
body+=gui[gui.index('/* Both native home layouts'):gui.index('static lv_res_t _save_options_action(')]
body+='\nstatic lv_obj_t *preview_emmc_tabs,*preview_report_panel;\nstatic lv_obj_t *ancestor_type(lv_obj_t *object,const char *name){for(;object;object=lv_obj_get_parent(object)){lv_obj_type_t type;lv_obj_get_type(object,&type);if(type.type[0]&&!strcmp(type.type[0],name))return object;}return NULL;}\nstatic lv_res_t preview_emmc_info(void){\n lv_obj_t *win=nyx_create_standard_window("Internal eMMC Info",NULL);\n const char *values="\\nSK Hynix (90)\\nhC8aP> (4A)\\n1.0\\n1034B3FE\\n04/2021\\n\\n#00DDFF V1.2 (rev 1.8)#\\n8F5\\nHS400\\n400 MB/s (200 MHz)\\n29816 MiB\\n2 MiB\\n\\nOff: OK\\nA: -, B: 100%\\nNormal (< 80%)";\n lv_obj_t *partitions=nyx_nova_emmc_summary(win,values);preview_emmc_tabs=ancestor_type(partitions,"lv_tabview");assert(preview_emmc_tabs);\n char table[10000];strcpy(table,"#00DDFF eMMC Physical Partitions:#\\n1: #96FF00 BOOT0#  Size: 4096 KiB\\n2: #96FF00 BOOT1#  Size: 4096 KiB\\n3: #96FF00 RPMB#   Size: 4096 KiB\\n\\n#00DDFF GPP (eMMC USER) Partition Table:#\\n");\n for(u32 i=0;i<48;++i){char row[160];s_printf(row,"%02d: #96FF00 Partition-%02d#   %6d MiB   %08Xh   %08Xh\\n",i,i,1024+i,i*0x10000,0x10000);strcat(table,row);}\n preview_report_panel=nyx_nova_report(partitions,table,34,18,1212,lv_obj_get_height(partitions)-36);return LV_RES_OK;\n}\nstatic lv_res_t preview_change_close(lv_obj_t *button){free(emummc_img->dirlist);free(emummc_img);emummc_img=NULL;return nyx_win_close_action(button);}\nstatic lv_res_t preview_change_emu(u32 count){\n lv_obj_t *win=nyx_create_standard_window("Change emuMMC",preview_change_close);emummc_img=calloc(1,sizeof(*emummc_img));assert(emummc_img);emummc_img->win=win;\n emummc_img->dirlist=calloc(1,sizeof(dirlist_t));assert(emummc_img->dirlist);if(count>63)count=63;\n for(u32 i=0;i<count;++i){emummc_img->dirlist->name[i]=emummc_img->dirlist->data+i*256;s_printf(emummc_img->dirlist->name[i],"Image %02d - Español Русский",i);}\n for(u32 i=0;i<3;++i){emummc_img->part_sector[i]=0x100000*(i+1);emummc_img->part_type[i]=0xE0;s_printf(&emummc_img->part_path[128*i],"emuMMC/RAW%d",i+1);}\n if(getenv("NYX_RAW_DISABLED"))emummc_img->part_type[1]=0x83;\n _create_change_emummc_ui(win,count);return LV_RES_OK;\n}\nstatic lv_obj_t *preview_mbr_info(void){\n lv_obj_t *background=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_size(background,1280,720);lv_obj_set_style(background,&mbox_darken);\n lv_obj_t *box=lv_mbox_create(background,NULL);lv_obj_set_width(box,852);lv_mbox_set_recolor_text(box,true);\n lv_mbox_set_text(box,"#FFDD00 The SD Card cannot be backed up automatically!#\\n#FFDD00 Any other partition will be also wiped!#\\n\\nYou will be asked to back up your files later via UMS.");\n lv_obj_t *holder=lv_cont_create(box,NULL);lv_cont_set_style(holder,&lv_style_transp_tight);lv_cont_set_fit(holder,false,true);lv_obj_set_width(holder,552);\n lv_obj_t *label=nyx_nova_text(holder,"Current MBR partition layout:",0,0,552,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);\n lv_obj_t *bar=lv_bar_create(holder,NULL);lv_obj_set_size(bar,552,33);lv_obj_set_pos(bar,0,40);lv_bar_set_value(bar,100);\n label=lv_label_create(holder,NULL);lv_label_set_style(label,&monospace_text);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,552);lv_obj_set_pos(label,0,100);\n lv_label_set_text(label,"Partition 0 - Type: 0c, Start: 00008000, Size: 74700000\\nPartition 1 - Type: e0, Start: 74708000, Size: 02000000\\nPartition 2 - Type: 83, Start: 76708000, Size: 00010000\\nPartition 3 - Type: ee, Start: 76718000, Size: 00010000");\n static const char *buttons[]={"\\251","\\222OK","\\251",""};lv_mbox_add_btns(box,buttons,nyx_mbox_action);lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);return box;\n}\nstatic managed_font legacy_mono;\nstatic void verify_mono_pixels(void){\n managed_font normalized={0};assert(scale_font(&ubuntu_mono,20,&normalized,false,true));\n for(u32 cp=33;cp<127;++cp){u32 width=lv_font_get_real_width(&ubuntu_mono,cp),bpp=lv_font_get_bpp(&ubuntu_mono,cp),stride=bpp==8?width:(width+1)/2;const u8 *original=lv_font_get_bitmap(&ubuntu_mono,cp),*actual=get_bitmap(&normalized.font,cp);assert(width<=10&&actual&&original);\n  for(u32 y=0;y<20;++y)for(u32 x=0;x<10;++x){u32 expected=0;if(x>=(10-width)/2&&x<(10-width)/2+width){u32 sx=x-(10-width)/2;u8 value=original[y*stride+(bpp==8?sx:sx/2)];expected=bpp==8?(value+8)/17:(sx&1?value&15:value>>4);}\n   u8 packed=actual[y*5+x/2];u32 decoded=x&1?packed&15:packed>>4;assert(decoded==expected);}\n }\n for(u32 cp=0x410;cp<=0x44F;++cp){const u8 *bitmap=get_bitmap(&normalized.font,cp);assert(bitmap);bool nonzero=false;for(u32 i=0;i<100;++i)if(bitmap[i])nonzero=true;assert(nonzero);}\n release_font(&normalized);\n}\nstatic lv_res_t preview_font_comparison(void){\n verify_mono_pixels();lv_obj_t *win=nyx_create_standard_window("Police technique",NULL);lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;\n assert(legacy_scale_font(&ubuntu_mono,nyx_font_mono()->h_px,&legacy_mono,false,true));\n lv_obj_t *left=nyx_nova_panel(page,34,18,594,height),*right=nyx_nova_panel(page,652,18,594,height);\n nyx_nova_text(left,"Avant",24,24,546,NOVA_TITLE);nyx_nova_text(right,"Après",24,24,546,NOVA_TITLE);\n static lv_style_t legacy;lv_style_copy(&legacy,&monospace_text);legacy.text.font=&legacy_mono.font;\n const char *sample="eMMC Physical Partitions:\\nBOOT0  4096 KiB  0x00000000\\nBOOT1  4096 KiB  0x00002000\\nGPP Partition Table:\\n00: PRODINFO    0x00008000\\n01: PRODINFOF   0x00010000\\nSD backup: 0123456789ABCDEF\\nÉcriture vérifiée · Русский";\n lv_obj_t *a=lv_label_create(left,NULL),*b=lv_label_create(right,NULL);lv_label_set_style(a,&legacy);lv_label_set_style(b,&monospace_text);lv_label_set_long_mode(a,LV_LABEL_LONG_BREAK);lv_label_set_long_mode(b,LV_LABEL_LONG_BREAK);lv_obj_set_width(a,546);lv_obj_set_width(b,546);lv_obj_set_pos(a,24,94);lv_obj_set_pos(b,24,94);lv_label_set_text(a,sample);lv_label_set_text(b,sample);return LV_RES_OK;\n}\n'
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
static lv_obj_t *find_type(lv_obj_t *object,const char *name){
 if(type_is(object,name))return object;
 for(lv_obj_t *c=lv_obj_get_child(object,NULL);c;c=lv_obj_get_child(object,c)){lv_obj_t *found=find_type(c,name);if(found)return found;}
 return NULL;
}
static void verify_credit_labels(lv_obj_t *object){
 if(lv_obj_get_hidden(object))return;
 if(type_is(object,"lv_label")){assert(lv_label_get_long_mode(object)!=LV_LABEL_LONG_DOT);assert(lv_obj_get_y(object)+lv_obj_get_height(object)<=lv_obj_get_height(lv_obj_get_parent(object)));}
 for(lv_obj_t *c=lv_obj_get_child(object,NULL);c;c=lv_obj_get_child(object,c))verify_credit_labels(c);
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
  if(asset&&lv_obj_get_signal_func(object)==launch_image_signal&&asset->header.cf==LV_IMG_CF_TRUE_COLOR_ALPHA&&lv_obj_get_width(lv_obj_get_parent(object))<=100){
   const u32 *pixel=(const u32 *)asset->data;
   for(u32 i=0;i<asset->header.w*asset->header.h;++i)if((pixel[i]>>24)==255){
    u32 actual=screen_pixels[(area.y1+i/asset->header.w)*LV_HOR_RES+area.x1+i%asset->header.w].full&0xFFFFFF;
    assert(actual==(pixel[i]&0xFFFFFF));++glyph_pixel_checks;break;
   }
  }
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
n_cfg.boot_logo_color=n_cfg.boot_bar_color=0x55C9F1;n_cfg.timedst=1;n_cfg.nova_logo_color=n_cfg.nova_icon_color=0x55C9F1;n_cfg.nova_selection_color=0x1C2937;n_cfg.theme_bg=0x141A19;n_cfg.nova_text_color=0xF2F5FA;n_cfg.nova_border_color=0x334050;n_cfg.nova_reflection_color=0x55C9F1;n_cfg.nova_menu_opacity=100;n_cfg.nova_icon_opacity=100;n_cfg.font_size=atoi(argv[5]);strcpy(n_cfg.font_file,getenv("NYX_PREVIEW_FONT")?getenv("NYX_PREVIEW_FONT"):"Lecture.nyxf");n_cfg.language=atoi(argv[4]);n_cfg.background_effect=0;n_cfg.background_speed=2;n_cfg.background_opacity=20;n_cfg.nova_halo=1;n_cfg.verification=1;h_cfg.autonogc=1;h_cfg.backlight=130;
h_cfg.rcm_patched=getenv("NYX_PREVIEW_PATCHED")!=NULL;
if(getenv("NYX_RGB_LOGO"))n_cfg.nova_logo_color=strtoul(getenv("NYX_RGB_LOGO"),NULL,16);
if(getenv("NYX_RGB_ICONS"))n_cfg.nova_icon_color=strtoul(getenv("NYX_RGB_ICONS"),NULL,16);
if(getenv("NYX_RGB_SELECTION"))n_cfg.nova_selection_color=strtoul(getenv("NYX_RGB_SELECTION"),NULL,16);
if(getenv("NYX_RGB_BACKGROUND"))n_cfg.theme_bg=strtoul(getenv("NYX_RGB_BACKGROUND"),NULL,16);
if(getenv("NYX_RGB_TEXT"))n_cfg.nova_text_color=strtoul(getenv("NYX_RGB_TEXT"),NULL,16);
if(getenv("NYX_RGB_BORDER"))n_cfg.nova_border_color=strtoul(getenv("NYX_RGB_BORDER"),NULL,16);
if(getenv("NYX_RGB_REFLECTION"))n_cfg.nova_reflection_color=strtoul(getenv("NYX_RGB_REFLECTION"),NULL,16);

if(getenv("NYX_PREVIEW_LAUNCH"))strcpy(n_cfg.launch_image,getenv("NYX_PREVIEW_LAUNCH"));
if(getenv("NYX_PREVIEW_WALLPAPER")){strcpy(n_cfg.wallpaper_file,getenv("NYX_PREVIEW_WALLPAPER"));nyx_nova_wallpaper_load();assert(nova_wallpaper);}
if(getenv("NYX_PREVIEW_MENU_OPACITY"))n_cfg.nova_menu_opacity=atoi(getenv("NYX_PREVIEW_MENU_OPACITY"));
if(getenv("NYX_PREVIEW_ICON_OPACITY"))n_cfg.nova_icon_opacity=atoi(getenv("NYX_PREVIEW_ICON_OPACITY"));
lv_init();nyx_localization_init();nyx_font_manager_init();lv_disp_drv_t driver;lv_disp_drv_init(&driver);driver.disp_flush=flush;lv_disp_drv_register(&driver);
lv_theme_hekate_set_accents(n_cfg.nova_icon_color,n_cfg.nova_selection_color);lv_theme_hekate_set_colors(n_cfg.nova_text_color,n_cfg.nova_border_color,n_cfg.nova_reflection_color,n_cfg.nova_menu_opacity);lv_theme_t*theme=lv_theme_hekate_init(n_cfg.theme_bg,n_cfg.theme_color,(lv_font_t*)nyx_font_ui(20));lv_theme_set_current(theme);lv_style_copy(&hint_small_style,theme->label.hint);hint_small_style.text.font=nyx_font_small();hint_small_style.text.letter_space=1;lv_style_copy(&hint_small_style_white,&hint_small_style);nyx_nova_init(theme);lv_style_copy(&monospace_text,&lv_style_plain);monospace_text.body.empty=1;monospace_text.body.opa=0;monospace_text.text.font=nyx_font_mono();monospace_text.text.color=nyx_nova_color(NOVA_COLOR_TEXT);monospace_text.text.letter_space=monospace_text.text.line_space=0;
lv_obj_t *screen=lv_cont_create(NULL,NULL);lv_scr_load(screen);nyx_nova_screen(screen);
nyx_nova_footer(lv_layer_top(),nyx_options_save_all);lv_label_set_text(status_bar.time_temp,"18:42 · 28 °C");lv_label_set_text(status_bar.battery,"82 %");
lv_obj_t*root=NULL;
if(strcmp(argv[3],"about")&&strcmp(argv[3],"home")&&strcmp(argv[3],"carousel")&&strcmp(argv[3],"tools")&&strcmp(argv[3],"info")&&strcmp(argv[3],"options")&&strcmp(argv[3],"diagnostics")){
 lv_obj_t*tabs=nyx_nova_tabview(lv_scr_act());lv_obj_t*home=NULL;for(u32 i=0;i<6;i++){lv_obj_t*p=lv_tabview_add_tab(tabs,"");if(i==1)home=p;}
 home_tabview=tabs;_create_tab_home(theme,home);lv_tabview_set_tab_act(tabs,1,false);
 nyx_nova_navigation(lv_scr_act(),tabs,"hekate 6.5.4",_create_nova_quick_actions);lv_action_t quick[]={create_win_nyx_options,_create_mbox_reload,_create_mbox_reboot,_create_mbox_poweroff};nyx_nova_quick_actions(quick);
}
if(!strcmp(argv[3],"backup")||!strcmp(argv[3],"restore")){lv_obj_t b;memset(&b,0,sizeof b);lv_obj_set_free_num(&b,!strcmp(argv[3],"restore"));preview_backup_restore(&b);}
else if(!strncmp(argv[3],"partition-",10))preview_partition(argv[3],atoi(argv[6]));
else if(!strcmp(argv[3],"archive"))preview_archive();
else if(!strcmp(argv[3],"package"))preview_package();
else if(!strcmp(argv[3],"component-results")){preview_component_results();component_page=atoi(argv[6]);component_fill();}
else if(!strcmp(argv[3],"component-leads")){preview_component_results();component_leads(NULL);}
else if(!strcmp(argv[3],"component-detail")){preview_component_results();lv_obj_t temporary;memset(&temporary,0,sizeof temporary);lv_obj_set_free_num(&temporary,atoi(argv[6]));component_detail(&temporary);}
else if(!strcmp(argv[3],"joycon-live"))nyx_joycon_test(NULL);
else if(!strcmp(argv[3],"touch-grid"))nyx_touch_test(NULL);
else if(!strcmp(argv[3],"screen-patterns"))nyx_screen_patterns(NULL);
else if(!strcmp(argv[3],"usb"))preview_usb_tools(NULL);
else if(!strcmp(argv[3],"emummc"))preview_emummc_tools(NULL);
else if(!strcmp(argv[3],"diagnostic-group")){lv_obj_t b;memset(&b,0,sizeof b);lv_obj_set_free_num(&b,atoi(argv[6]));preview_show_group(&b);}
else if(!strcmp(argv[3],"diagnostic-results"))show_results(0);
else if(!strcmp(argv[3],"ram-service"))show_ram_service(NULL);
else if(!strcmp(argv[3],"setup")){first_time_clock_edit(NULL);root=lv_obj_get_child(lv_obj_get_child(lv_scr_act(),NULL),NULL);}
else if(!strcmp(argv[3],"appearance"))nyx_create_appearance_window(NULL);
else if(!strcmp(argv[3],"nyx"))create_win_nyx_options(NULL);
else if(!strcmp(argv[3],"emmc-info"))preview_emmc_info();
else if(!strcmp(argv[3],"emummc-change"))preview_change_emu(atoi(argv[6]));
else if(!strcmp(argv[3],"backup-progress")){emmc_btn_ctxt.restore=false;_create_window_backup_restore(PART_BOOT,"eMMC BOOT0 & BOOT1");}
else if(!strcmp(argv[3],"mbr-info"))root=preview_mbr_info();
else if(!strcmp(argv[3],"font-comparison"))preview_font_comparison();
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
if(!strcmp(argv[3],"package"))lv_tabview_set_tab_act(fixture_log_tabs,atoi(argv[6]),false);
if(!strcmp(argv[3],"emmc-info"))lv_tabview_set_tab_act(preview_emmc_tabs,atoi(argv[6]),false);
if(!strcmp(argv[3],"colors"))lv_tabview_set_tab_act(nova_theme.tabs,atoi(argv[6]),false);
if(!strcmp(argv[3],"about")){lv_obj_t *tabs=find_type(root,"lv_tabview");assert(tabs);lv_tabview_set_tab_act(tabs,atoi(argv[6]),false);lv_tabview_ext_t *ext=lv_obj_get_ext_attr(tabs);verify_credit_labels(ext->content);}
if(!root){assert(nova_navigation_locks);for(u32 i=1;i<6;i++)assert(!lv_obj_get_click(nova_nav_buttons[i]));lv_obj_t*window=lv_win_get_from_btn(close_btn);lv_win_ext_t*ext=lv_obj_get_ext_attr(window);root=ext->page;
 if(!strcmp(argv[3],"appearance")||!strcmp(argv[3],"nyx")){lv_obj_t*tabs=lv_obj_get_child(lv_page_get_scrl(root),NULL);lv_tabview_set_tab_act(tabs,atoi(argv[6]),false);}}
if(!strncmp(argv[3],"partition-",10)){verify_partition_state();if(getenv("NYX_TEST_PARTITION_ACTIONS"))exercise_partition_controls();}
verify_tree(lv_scr_act());verify_tree(lv_layer_top());nyx_readability_refresh(lv_scr_act());for(int i=0;i<4;i++){lv_tick_inc(40);lv_task_handler();}lv_refr_now();
for(u32 percent=0;percent<=100;percent++){preview_battery_percent=percent;_update_status_bar(NULL);assert(lv_obj_get_x(status_bar.battery)==1174&&lv_obj_get_y(status_bar.battery)==24);}
preview_battery_percent=82;
u32 temperatures[]={0,44,45,54,55,64,65,99};
for(u32 i=0;i<8;++i){preview_temperature=temperatures[i];_update_status_bar(NULL);char rgb[16];sprintf(rgb,"#%06X ",nyx_nova_temperature_rgb(preview_temperature));assert(strstr(lv_label_get_text(status_bar.time_temp),rgb));assert(strstr(lv_label_get_text(status_bar.time_temp),"18:42"));}
preview_temperature=getenv("NYX_PREVIEW_TEMPERATURE")?atoi(getenv("NYX_PREVIEW_TEMPERATURE")):28;_update_status_bar(NULL);lv_refr_now();
lv_area_t bounds;lv_obj_get_coords(root,&bounds);assert(bounds.y1>=76&&bounds.y2<656);
verify_visible(root,bounds);verify_reachable(root,bounds);
nyx_nova_reflection_pause(true);lv_refr_now();lv_area_t screen_bounds={0,0,1279,719};if(strcmp(argv[3],"touch-grid")&&strcmp(argv[3],"screen-patterns")){verify_glyph_pixels(lv_scr_act(),screen_bounds);verify_glyph_pixels(lv_layer_top(),screen_bounds);}
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
 theme_reset(NULL);assert(nova_theme.pending.nova_menu_opacity==25&&nova_theme.pending.nova_icon_opacity==100&&!strcmp(nova_theme.pending.wallpaper_file,"Nova-Carbon.bmp"));assert(!memcmp(&before,&n_cfg,sizeof n_cfg));
}


if(!strcmp(argv[3],"colors")){
 nyx_config saved=n_cfg,draft=nova_theme.pending;
 nova_theme.pending.theme_bg=0x080C18;nova_theme.pending.nova_text_color=0xF0D0A0;nova_theme.pending.nova_border_color=0x8899AA;
 nova_theme.pending.nova_logo_color=0xC080FF;nova_theme.pending.nova_icon_color=0x80E0C0;nova_theme.pending.nova_selection_color=0x334455;
 nyx_config expected=nova_theme.pending;
 for(u32 i=0;i<6;++i){lv_obj_t *choice=find_choice(nova_theme.window,THEME_TARGET,i);assert(choice);lv_btn_get_action(choice,LV_BTN_ACTION_CLICK)(choice);
  assert(!memcmp(&nova_theme.pending,&expected,sizeof expected));assert(!memcmp(&n_cfg,&saved,sizeof saved));
  assert((lv_img_get_style(nova_theme.sample_glyph)->image.color.full&0xFFFFFF)==0xC080FF);
  assert((lv_img_get_style(nova_theme.sample_option_glyph)->image.color.full&0xFFFFFF)==0x80E0C0);
  assert((lv_label_get_style(nova_theme.sample_text)->text.color.full&0xFFFFFF)==0xF0D0A0);
  assert((lv_cont_get_style(nova_theme.sample)->body.main_color.full&0xFFFFFF)==0x334455);
  assert((lv_cont_get_style(nova_theme.preview)->body.main_color.full&0xFFFFFF)==0x080C18);
  lv_tabview_set_tab_act(nova_theme.tabs,1,false);lv_tabview_set_tab_act(nova_theme.tabs,0,false);assert(!memcmp(&nova_theme.pending,&expected,sizeof expected));
 }
 nova_theme.pending=draft;lv_obj_t *first_choice=find_choice(nova_theme.window,THEME_TARGET,0);assert(first_choice);lv_btn_get_action(first_choice,LV_BTN_ACTION_CLICK)(first_choice);lv_tabview_set_tab_act(nova_theme.tabs,atoi(argv[6]),false);lv_refr_now();
}

if(!strcmp(argv[3],"colors")&&getenv("NYX_TEST_BOOT_LAUNCH")){
 nyx_config actual=n_cfg;
 lv_obj_t *choice=find_choice(nova_theme.window,THEME_BOOT_TARGET,1);assert(choice);theme_select(choice);
 boot_picker_changed(nova_theme.boot_picker,0xCC2299);assert(nova_theme.pending.boot_bar_color==0xCC2299&&nova_theme.pending.boot_logo_color==actual.boot_logo_color);
 choice=find_choice(nova_theme.window,THEME_BOOT_TARGET,0);assert(choice);theme_select(choice);
 boot_picker_changed(nova_theme.boot_picker,0x11DD66);assert(nova_theme.pending.boot_logo_color==0x11DD66&&nova_theme.pending.boot_bar_color==0xCC2299);
 nyx_config expected=nova_theme.pending;
 for(u32 i=0;i<4;++i)lv_tabview_set_tab_act(nova_theme.tabs,i,false);
 assert(!memcmp(&expected,&nova_theme.pending,sizeof expected)&&!memcmp(&actual,&n_cfg,sizeof actual));
 lv_obj_t fake;memset(&fake,0,sizeof fake);lv_obj_set_free_num(&fake,1);launch_step(&fake);
 assert(!strcmp(nova_theme.pending.launch_image,"Nova-Carbon.bmp")&&!strcmp(nova_theme.pending.wallpaper_file,expected.wallpaper_file));
 bool found=false;for(lv_obj_t*c=lv_obj_get_child(nova_theme.launch_card,NULL);c;c=lv_obj_get_child(nova_theme.launch_card,c))if(lv_obj_get_signal_func(c)==launch_image_signal){launch_image_ext *ext=lv_obj_get_ext_attr(c);assert(ext->pixels&&ext->pixels->header.w==472&&ext->pixels->header.h==288);found=true;}
 assert(found);launch_step(&fake);assert(!nova_theme.pending.launch_image[0]);
 assert(!memcmp(&actual,&n_cfg,sizeof actual));
 lv_tabview_set_tab_act(nova_theme.tabs,atoi(argv[6]),false);lv_refr_now();
}
if(!strcmp(argv[3],"colors")&&getenv("NYX_TEST_V18")){
 nyx_config before=n_cfg;lv_obj_t next;memset(&next,0,sizeof next);lv_obj_set_free_num(&next,1);
 bool png=false,bmp=false,empty=false;
 for(u32 i=0;i<=launch_count;++i){
  launch_step(&next);const char *file=nova_theme.pending.launch_image;
  if(!*file)empty=true;
  else if(image_png_name(file)){
   png=true;bool found=false;
   for(lv_obj_t*c=lv_obj_get_child(nova_theme.launch_card,NULL);c;c=lv_obj_get_child(nova_theme.launch_card,c))if(lv_obj_get_signal_func(c)==launch_image_signal){const lv_img_dsc_t *img=lv_img_get_src(c);assert(img->header.cf==LV_IMG_CF_TRUE_COLOR_ALPHA);found=true;}
   assert(found);
  }else bmp=true;
 }
 assert(png&&bmp&&empty&&!memcmp(&before,&n_cfg,sizeof before));
 lv_tabview_set_tab_act(nova_theme.tabs,atoi(argv[6]),false);lv_refr_now();
}
if(!strcmp(argv[3],"colors")&&!atoi(argv[6])){
 color_picker_ext *picker=lv_obj_get_ext_attr(nova_theme.picker);
 assert(picker->pixels&&picker->hit&&picker->value);
 const u32 samples[]={0,0xFFFFFF,0x59F7FF,0xAABBCC,0xFF0000,0x00FF00,0x0000FF,0x10151D};
 for(u32 i=0;i<8;++i){nyx_color_picker_set(nova_theme.picker,samples[i]);assert(nyx_color_picker_rgb(nova_theme.picker)==samples[i]);assert(picker->hue<360&&picker->saturation<=100&&picker->brightness<=100);}
 nyx_color_picker_set(nova_theme.picker,*theme_target_color());
 if(getenv("NYX_TEST_PICKER")){
  lv_indev_drv_t driver;lv_indev_drv_init(&driver);driver.type=LV_INDEV_TYPE_POINTER;driver.read=preview_pointer_read;lv_indev_drv_register(&driver);
  lv_area_t area;lv_obj_get_coords(picker->hit,&area);
  nyx_color_picker_set(nova_theme.picker,0xFFFFFF);
  preview_click_point(area.x1+picker->width/2,area.y1+4);
  assert(nyx_color_picker_rgb(nova_theme.picker)!=0xFFFFFF&&*theme_target_color()==nyx_color_picker_rgb(nova_theme.picker));
  lv_slider_set_value(picker->value,0);lv_slider_get_action(picker->value)(picker->value);
  assert(nyx_color_picker_rgb(nova_theme.picker)==0&&*theme_target_color()==0);
  lv_slider_set_value(picker->value,100);lv_slider_get_action(picker->value)(picker->value);
  assert(nyx_color_picker_rgb(nova_theme.picker)!=0);
 }
}
if(!strcmp(argv[3],"emmc-info")){
 lv_tabview_set_tab_act(preview_emmc_tabs,1,false);report_ext *report=lv_obj_get_ext_attr(preview_report_panel);u32 pages=0,end=0;
 do{assert(report->starts[report->page]==end);assert(report->next>end);assert(lv_obj_get_height(report->label)<=report->text_height);end=report->next;++pages;lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);
  if(end==strlen(report->text))break;report_step(report->following);
 }while(pages<128);
 assert(end==strlen(report->text)&&pages>=3);while(report->page)report_step(report->previous);assert(report->starts[0]==0);
 lv_tabview_set_tab_act(preview_emmc_tabs,atoi(argv[6]),false);lv_refr_now();
}
if(!strcmp(argv[3],"emummc-change")){
 u32 count=emummc_img->file_count,pages=(count+3)/4;if(!pages)pages=1;u32 visited=0;
 for(u32 page=0;page<pages;++page){emummc_img->file_page=page;_fill_emummc_files();lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);
  for(lv_obj_t *button=lv_obj_get_child(emummc_img->file_items,NULL);button;button=lv_obj_get_child(emummc_img->file_items,button))if(type_is(button,"lv_btn")){
   u32 index=lv_obj_get_free_num(button);assert(index<count&&index/4==page);lv_btn_get_action(button,LV_BTN_ACTION_CLICK)(button);char path[512];sprintf(path,"emuMMC/%s",emummc_img->dirlist->name[index]);assert(!strcmp(preview_saved_emu_path,path)&&!preview_saved_emu_index&&!preview_saved_emu_sector);++visited;
  }
 }assert(visited==count);emummc_img->file_page=0;_fill_emummc_files();lv_refr_now();
}

if(!strcmp(argv[3],"setup")){assert(clock_language==n_cfg.language&&clock_ctxt.year&&clock_ctxt.month&&clock_ctxt.day&&clock_ctxt.hour&&clock_ctxt.min);assert(type_is(root,"lv_mbox"));}
if(diagnostic_last_holder){diag_tiles_ext*ext=lv_obj_get_ext_attr(diagnostic_last_holder);u32 total=ext->count;unsigned visits=0;
 for(u32 page=0;page<(total+8)/9;++page){ext->page=page;diagnostic_tiles_fill(diagnostic_last_holder);lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);for(lv_obj_t*c=lv_obj_get_child(ext->tiles,NULL);c;c=lv_obj_get_child(ext->tiles,c))for(lv_obj_t*b=lv_obj_get_child(c,NULL);b;b=lv_obj_get_child(c,b))if(type_is(b,"lv_btn")){u32 id=lv_obj_get_free_num(b);assert((id&~ONE_PROBE)<42);lv_action_t action=lv_btn_get_action(b,LV_BTN_ACTION_CLICK);assert(action==(ext->results?show_report:run_selection));++visits;}}
 assert(visits==total);ext->page=0;diagnostic_tiles_fill(diagnostic_last_holder);lv_refr_now();
}

if(nova_wallpaper){lv_obj_t *back=lv_obj_get_child_back(lv_scr_act(),NULL);assert(type_is(back,"lv_img")&&!lv_obj_get_click(back)&&lv_obj_is_protected(back,LV_PROTECT_PARENT));assert(lv_img_get_src(back)==nova_wallpaper);}
if(getenv("NYX_TEST_NAVIGATION"))verify_wallpaper_navigation();

if(getenv("NYX_TEST_BORDER")){
 assert(!strcmp(argv[3],"home"));
 for(u32 pass=0;pass<2;++pass){
  for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].target){
   reflection_targets[i].phase=pass?16:0;reflection_targets[i].painted=false;lv_obj_invalidate(reflection_targets[i].overlay);
  }
  lv_refr_now();dump(pass?"border-16.ppm":"border-0.ppm");
 }
}
if(getenv("NYX_TEST_SHEEN_PROFILE")){
 assert(!strcmp(argv[3],"home"));nyx_nova_reflection_pause(true);lv_refr_now();dump("sheen-baseline.ppm");nyx_nova_reflection_pause(false);
 reflection_target *target=NULL;for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].target==nova_home_cards[0])target=&reflection_targets[i];assert(target);
 target->phase=(REFLECTION_SWEEP-1)/2;target->painted=true;reflection_bounds(target);lv_obj_invalidate(target->overlay);lv_refr_now();dump("sheen-profile.ppm");
 lv_area_t tile;lv_obj_get_coords(target->target,&tile);int row=lv_obj_get_height(target->target)/2;
 printf("PROFILE center=%d row=%d half=%d sweep=%d cycle=%d period=%d\n",tile.x1+reflection_center(target)-row/2,tile.y1+row,REFLECTION_HALF,REFLECTION_SWEEP,REFLECTION_CYCLE,REFLECTION_PERIOD);
}
if(!getenv("NYX_PREVIEW_NO_DUMP"))dump("preview.ppm");
if(getenv("NYX_CAPTURE_REFLECTION")){
 FILE*frames=fopen("reflection.bgrx","wb");assert(frames&&sizeof(lv_color_t)==4);
 for(u32 frame=0;frame<REFLECTION_CYCLE;++frame){lv_tick_inc(REFLECTION_PERIOD);lv_task_handler();lv_refr_now();assert(fwrite(screen_pixels,sizeof(lv_color_t),LV_HOR_RES*LV_VER_RES,frames)==LV_HOR_RES*LV_VER_RES);}
 fclose(frames);
}
if((!strcmp(argv[3],"tools")||!strcmp(argv[3],"home")||!strcmp(argv[3],"carousel"))&&!strcmp(argv[4],"0")&&!strcmp(argv[5],"30")){
 nyx_background_pause(true);for(int i=0;i<20;i++){lv_tick_inc(REFLECTION_PERIOD);lv_task_handler();}lv_refr_now();
 rendered_pixels=0;for(int i=0;i<60;i++){lv_tick_inc(REFLECTION_PERIOD);lv_task_handler();}lv_refr_now();assert(rendered_pixels==0);
 nyx_background_pause(false);rendered_pixels=0;unsigned long maximum=0,total=0;
 for(int i=0;i<REFLECTION_CYCLE;i++){unsigned long before=rendered_pixels;lv_tick_inc(REFLECTION_PERIOD);lv_task_handler();lv_refr_now();unsigned long pixels=rendered_pixels-before;total+=pixels;if(pixels>maximum)maximum=pixels;}
 /* The added color border may repaint its perimeter, but the combined
  * frame must stay below one eighth of the display, never a full redraw. */
 fprintf(stderr,"PERFORMANCE max=%lu total=%lu\n",maximum,total);assert(total>0&&maximum<LV_HOR_RES*LV_VER_RES/8);
 printf("PASS: Nova reflection max=%lu total=%lu pixels over %dms; pause=0\n",maximum,total,REFLECTION_CYCLE*REFLECTION_PERIOD);
 nyx_nova_reflection_reset();rendered_pixels=0;lv_refr_now();rendered_pixels=0;for(int i=0;i<60;i++){lv_tick_inc(REFLECTION_PERIOD);lv_task_handler();}lv_refr_now();assert(rendered_pixels==0);

}
if(getenv("NYX_TEST_HANDS")){
 if(!strcmp(argv[3],"joycon-live")){
  jc_gamepad_rpt_t pad;memset(&pad,0,sizeof pad);pad.conn_l=pad.conn_r=true;pad.lstick_x=pad.lstick_y=pad.rstick_x=pad.rstick_y=2048;
  for(u32 i=0;i<60;++i){nyx_interactive_joycon(&pad);_jc_tick(NULL);lv_tick_inc(20);}
  for(u32 i=0;i<9;++i){pad.buttons=(1u<<s_btn_l[i].bit)|(1u<<s_btn_r[i].bit);nyx_interactive_joycon(&pad);_jc_tick(NULL);pad.buttons=0;nyx_interactive_joycon(&pad);_jc_tick(NULL);}
  assert(s_jc[0].done==511&&s_jc[1].done==511);
  const int positions[][2]={{896,2048},{3200,2048},{2048,896},{2048,3200}};
  for(u32 i=0;i<4;++i){pad.lstick_x=pad.rstick_x=positions[i][0];pad.lstick_y=pad.rstick_y=positions[i][1];nyx_interactive_joycon(&pad);_jc_tick(NULL);lv_tick_inc(20);}
  pad.lstick_x=pad.lstick_y=pad.rstick_x=pad.rstick_y=2048;
  for(u32 i=0;i<60;++i){nyx_interactive_joycon(&pad);_jc_tick(NULL);lv_tick_inc(20);}
  u32 reach[4],pct[4];assert(gui_stick_range(&s_jc[0].stick,reach,pct)==4&&gui_stick_drift(&s_jc[0].stick)<=1);
  pad.conn_l=false;nyx_interactive_joycon(&pad);_jc_tick(NULL);pad.conn_l=true;nyx_interactive_joycon(&pad);_jc_tick(NULL);assert(s_jc[0].drops==1);
  pad.sio_mode=true;pad.conn_r=false;nyx_interactive_joycon(&pad);_jc_tick(NULL);assert(s_jc[0].seen&&s_jc[1].seen);
  for(u32 side=0;side<2;++side){assert(lv_obj_get_height(s_jc[side].status)<=32);assert(lv_obj_get_width(s_jc[side].status)<=JC_PANEL_W-2*JC_IN);}
 }else if(!strcmp(argv[3],"touch-grid")){
  touch_event_t event;memset(&event,0,sizeof event);event.touch=true;
  /* Begin outside the Finish panel, then cover its hidden area too. */
  event.x=1;event.y=1;assert(nyx_interactive_touch(&event));assert(lv_obj_get_hidden(s_tt_info));
  for(u32 r=0;r<TT_ROWS;++r)for(u32 c=0;c<TT_COLS;++c){event.x=c*TT_CELL+40;event.y=r*TT_CELL+40;nyx_interactive_touch(&event);}
  event.touch=false;nyx_interactive_touch(&event);assert(s_hits==TT_ROWS*TT_COLS);_tt_tick(NULL);assert(s_tt_over);
 }else if(!strcmp(argv[3],"screen-patterns")){
  lv_obj_set_hidden(mire_controls,true);
  for(u32 color=0;color<5;++color){lv_refr_now();for(u32 y=0;y<720;y+=19)for(u32 x=0;x<1280;x+=23)assert((screen_pixels[y*1280+x].full&0xFFFFFF)==mire_colors[color]);mire_change(mire_next);}
  assert(mire_visited==31&&!mire_observed&&!mire_anomalies);lv_obj_t observation;memset(&observation,0,sizeof observation);lv_obj_set_free_num(&observation,1);mire_observe(&observation);assert(mire_observed==1&&mire_anomalies==1);
 }
 if(interactive_window){fixture_power=BTN_POWER;interactive_power_tick(NULL);fixture_power=0;assert(!interactive_window&&!interactive_overlay&&!interactive_jc&&!interactive_sink&&!interactive_power);}
 if(!strcmp(argv[3],"joycon-live")){assert(manual_observations[NYX_MANUAL_COMMANDS][0]);char *saved=nyx_component_detail_text(13);assert(saved&&strstr(saved,"9/9"));free(saved);}
 if(!strcmp(argv[3],"touch-grid")){assert(manual_observations[NYX_MANUAL_TOUCH][0]);char *saved=nyx_component_detail_text(12);assert(saved&&strstr(saved,"144/144"));free(saved);}
 if(!strcmp(argv[3],"screen-patterns")){nyx_component_result result;assert(nyx_component_get(11,&result)&&result.state>=NYX_COMPONENT_WARN);char *saved=nyx_component_detail_text(11);assert(saved&&strstr(saved,"0x1F"));free(saved);}
 verify_tree(lv_scr_act());verify_tree(lv_layer_top());printf("PASS: hands-on input, coverage, full-screen colors and POWER cleanup.\n");
}
printf("PASS UI %s language=%s size=%s tab=%s controls=%u\n",argv[3],argv[4],argv[5],argv[6],geometry_checks);return 0;}
'''
(directory/'preview.c').write_text(body,encoding='utf-8')
cc=work/'toolchains/w64devkit/bin/gcc.exe'
flags=['-std=gnu11','-O1','-include','stdint.h','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(directory),'-I'+str(project/'bdk'),'-I'+str(nyx)]
sources=list(lvgl.rglob('*.c'))+[nyx/'fonts/unicode_fonts.c',nyx/'frontend/nova_image.c',nyx/'libs/png/lodepng_nyx.c',directory/'preview.c']
def compile_one(p):
    obj=directory/(p.stem+'.o')
    result=subprocess.run([str(cc),*flags,'-c',str(p),'-o',str(obj)],capture_output=True,text=True)
    if result.returncode:raise RuntimeError(str(p)+'\n'+result.stderr)
    return obj
with ThreadPoolExecutor(max_workers=6)as pool:objects=list(pool.map(compile_one,sources))
exe=directory/'preview.exe';subprocess.run([str(cc),*[str(o)for o in objects],'-o',str(exe)],check=True)
jobs=[('partition-simple',1,32,4),('partition-expert',1,32,0),('partition-emmc',1,32,0),('archive',1,32,0),('package',1,32,0),('package',1,32,1),('tools',1,20,0),('tools',4,36,0),('info',1,20,0),('options',1,20,0),('options',4,36,0),('diagnostics',4,36,0),('appearance',1,30,0),('appearance',1,30,1),('appearance',1,30,2),('nyx',4,36,0),('nyx',4,36,1),('colors',1,30,0)]
jobs.extend([('home',1,30,0),('carousel',4,36,0),('tools',1,30,0),('info',1,30,0),('options',1,30,0),('diagnostics',1,30,0),('boot-options',1,30,0),('quick-actions',1,30,0)])
jobs.extend([('about',1,30,0),('backup',1,30,0),('restore',1,30,0),('usb',1,30,0),('emummc',1,30,0),('diagnostic-group',1,30,2),('diagnostic-results',1,30,0),('ram-service',1,30,0),('colors',1,30,1)])
if '--check-all' in sys.argv:
    modes=[('partition-simple',0),('partition-simple',4),('partition-simple',8),('partition-expert',0),('partition-emmc',0),('archive',0),('package',0),('package',1),('about',0),('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('appearance',0),('appearance',1),('appearance',2),('nyx',0),('nyx',1),('colors',0),('colors',1),('boot-options',0),('quick-actions',0),('setup',0),('emmc-info',0),('emmc-info',1),('backup-progress',0),('mbr-info',0),('emummc-change',0),('emummc-change',9),('font-comparison',0)]
    modes.extend([('backup',0),('restore',0),('usb',0),('emummc',0),('diagnostic-results',0),('ram-service',0)]+[('diagnostic-group',i)for i in range(1,11)])
    jobs=[(screen,language,size,tab) for screen,tab in modes for language in range(7) for size in range(20,38,2)]
if '--case' in sys.argv:
    at=sys.argv.index('--case');jobs=[(sys.argv[at+1],int(sys.argv[at+2]),int(sys.argv[at+3]),int(sys.argv[at+4]))]
environment=os.environ.copy()
if '--trace' in sys.argv:environment['NYX_TRACE']='1'
if '--check-all' in sys.argv:environment['NYX_PREVIEW_NO_DUMP']='1'
if '--build-only' in sys.argv: jobs=[]
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
        result={'reflection_period_ms':20,'max_redraw_pixels_per_frame':int(performance[1]),'cycle_ms':8480,'total_redraw_pixels_over_cycle':int(performance[2]),'paused_redraw_pixels':0,'platform':'host LVGL, not console FPS'}
        (directory/('nova-performance-'+screen+'.json')).write_text(json.dumps(result,indent=2));(directory/'nova-performance.json').write_text(json.dumps(result,indent=2))
        print('PASS: white diagonal full-tile sheen; '+performance[0],flush=True)
    if '--check-all' not in sys.argv:
        target=work/f'preview-v18-{screen}-{language}-{size}-{tab}.png'
        Image.open(directory/'preview.ppm').save(target)
        print(target.name,run.stdout.strip(),flush=True)
print(f'PASS: real-LVGL UI geometry and selector checks across {len(jobs)} screens; visible controls stay above the status bar; two-tap actions, independent colors and bounded reflections checked.',flush=True)
