
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
typedef struct _nyx_config
{
	u32 theme_bg; // COLOR_BG_BASE_MIN - COLOR_BG_BASE_MAX.
	u32 theme_color;
	u32 entries_5_col;
	u32 timeoffset;
	u32 timedst;
	u32 home_screen;
	u32 verification;
	u32 ums_emmc_rw;
	u32 jc_disable;
	u32 jc_force_right;
	u32 bpmp_clock;
	u32 home_layout; // Appended: keep all previous field offsets unchanged.
	u32 language; // Appended: UI locale, persisted independently of home layout.
	char font_file[64]; // Empty selects the embedded font; basename under /fonts.
	u32 font_size; // 20..36 px, even values.
	u32 background_effect; // 0 none, 1 waves, 2 particles, 3 aurora.
	u32 background_speed; // 1 slow, 2 normal, 3 fast.
	u32 background_opacity; // 10..50 percent.
	u32 nova_halo; // Nova selected-tile white sheen, 0 off / 1 on.
	u32 nova_text_color; // Independent 24-bit RGB colors, appended for compatibility.
	u32 nova_border_color;
	u32 nova_reflection_color;
	u32 nova_menu_opacity; // 0..100, panel background only; text stays opaque.
	u32 nova_icon_opacity; // 0..100, icon background only; glyph stays opaque.
	char wallpaper_file[64]; // Safe BMP basename under /wallpapers; empty = none.
	u32 nova_logo_color; // Independent alpha-mask tint, RGB24.
	u32 nova_icon_color;
	u32 nova_selection_color; // Selected card background, RGB24.
	char launch_image[64]; // Optional BMP for the Launch tile only, under /wallpapers.
	u32 boot_logo_color; // Independent splash emblem tint, RGB24.
	u32 boot_bar_color; // Independent splash progress tint, RGB24.
	u32 tv_auto; // Appended: automatic official dock detection, off by default.
	u32 tv_keep_screen; // 1 keeps LCD visible; 0 blanks it after TV initialization.
} nyx_config;
static nyx_config n_cfg;
static struct{bool rcm_patched;u32 autoboot,autonogc,autohosoff,updater2p,bootwait,backlight;}h_cfg;
static struct {lv_obj_t *bar_bg,*mid,*time_temp,*temp_symbol,*temp_degrees,*battery,*battery_more;} status_bar;
static bool ini_changes_made,nyx_changes_made,autoboot_first_time;
static lv_obj_t *autoboot_btn;
static lv_res_t gui_tv_open(lv_obj_t *btn);
static lv_res_t gui_bluetooth_open(lv_obj_t *btn);
static void gui_tv_auto_configure(void){}
static void gui_tv_screen_configure(void){}
static bool gui_tv_console_dark(void){return false;}
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
/* Development version for this source tree; shared by UI and packaging. */
#ifndef NYX_NOVA_VERSION_H
#define NYX_NOVA_VERSION_H
#define NYX_NOVA_VERSION "29"
#endif

/* Validated partition plans. SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef GUI_PARTITION_PLAN_H
#define GUI_PARTITION_PLAN_H

/* These are the existing Hekate FULL allocations, including boot/alignment
 * reserves. 29/58 GiB are rounded descriptions, not truncated allocations. */
#define NOVA_EMU_32_MIB 29856u
#define NOVA_EMU_64_MIB 59680u
#define NOVA_EMU_RESERVE_MIB 32u
#define NOVA_ANDROID_SYSTEM_MIB 6144u
#define NOVA_LINUX_MIN_MIB 8192u
#define NOVA_ANDROID_MIN_MIB 10240u
#define NOVA_DISK_ALIGN_SECTORS 0x8000u

typedef enum {
    NOVA_PLAN_OK, NOVA_PLAN_CAPACITY, NOVA_PLAN_MODEL,
    NOVA_PLAN_EMMC_UNAVAILABLE, NOVA_PLAN_EMMC_UPGRADED,
    NOVA_PLAN_LINUX_SIZE, NOVA_PLAN_ANDROID_SIZE,
    NOVA_PLAN_EMUMMC_SIZE, NOVA_PLAN_INVALID
} nova_plan_result;
typedef struct {
    u32 total_sectors;
    u32 fat_min_mib;
    u32 emu_mib,linux_mib,android_mib;
    u32 fat_mib,alignment_sectors;
    bool emu_double;
} nova_partition_plan;

nova_plan_result nova_partition_validate(nova_partition_plan *plan);
nova_plan_result nova_partition_simplified(nova_partition_plan *plan,
    u32 actual_model,u32 chosen_model,u32 emmc_data_mib,u32 linux_choice,u32 android_choice);
void nova_partition_widths(const nova_partition_plan *plan,u32 width,u32 out[4]);
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_FONT_MANAGER_H
#define NYX_FONT_MANAGER_H

/* Initialize after resource loading, lv_init and nyx_unicode_fonts_init. */
void nyx_font_manager_init(void);
const lv_font_t *nyx_font_regular(void);
const lv_font_t *nyx_font_small(void);
const lv_font_t *nyx_font_mono(void);
/* Cached typographic roles for Nova. Requested pixels are based on 30px UI
 * preference, scaled once at screen creation, never during a paint. */
const lv_font_t *nyx_font_ui(u32 pixels);
const char *nyx_font_options(void);
const char *nyx_font_filename(u32 index);
u32 nyx_font_count(void);
u32 nyx_font_selected_index(const char *filename);
bool nyx_font_available(const char *filename);
bool nyx_font_available_at_size(const char *filename, u32 size);
const char *nyx_font_status(void);
u32 nyx_font_size(void);
/* Standalone validator, also used by the actual-C host regression checks. */
bool nyx_font_validate_header(const u8 header[32], u32 file_size);
bool nyx_font_filename_valid(const char *filename);
#endif

/* Native Nyx language support; UI text only, never paths or boot names. */
#ifndef NYX_LOCALIZATION_H
#define NYX_LOCALIZATION_H
#define I18N_EN 0
#define I18N_FR 1
#define I18N_ES 2
#define I18N_IT 3
#define I18N_RU 4
#define I18N_DE 5
#define I18N_PT 6
#define I18N_LANGUAGE_COUNT 7
const char *nyx_tr(const char *text);
/* Returns an owned translation, or NULL if unchanged/allocation failed. */
char *nyx_translate_text(const char *text);
void nyx_localization_init(void);
void nyx_label_set_text(lv_obj_t *label, const char *text);
void nyx_label_set_static_text(lv_obj_t *label, const char *text);
void nyx_mbox_set_text(lv_obj_t *mbox, const char *text);
void nyx_ddlist_set_options(lv_obj_t *list, const char *options);
void nyx_roller_set_options(lv_obj_t *roller, const char *options);
void nyx_mbox_add_btns(lv_obj_t *mbox, const char **map, lv_btnm_action_t action);
void nyx_btnm_set_map(lv_obj_t *buttons, const char **map);
void nyx_win_set_title(lv_obj_t *win, const char *title);
lv_obj_t *nyx_win_add_btn(lv_obj_t *win, const void *image, const char *label, lv_action_t action);
lv_obj_t *nyx_tabview_add_tab(lv_obj_t *tabs, const char *title);
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_UNICODE_FONTS_H
#define NYX_UNICODE_FONTS_H

/* Call after lv_init(), once the existing text/symbol font chains are linked. */
void nyx_unicode_fonts_init(void);

#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_NOVA_H
#define NYX_GUI_NOVA_H

#define NYX_NOVA_HEADER 76
#define NYX_NOVA_BOTTOM 656
#define NYX_NOVA_HALO 18
enum { NOVA_KICKER, NOVA_SMALL, NOVA_BODY, NOVA_TITLE, NOVA_HEADING, NOVA_MUTED };
enum { NOVA_TOOLS, NOVA_INFO, NOVA_DIAGNOSTICS };
void nyx_nova_init(lv_theme_t *theme);
lv_obj_t *nyx_nova_tabview(lv_obj_t *screen);
void nyx_nova_navigation(lv_obj_t *screen, lv_obj_t *tabs, const char *version, lv_action_t brand_action);
void nyx_nova_navigation_select(u32 tab);
void nyx_nova_footer(lv_obj_t *layer, lv_action_t save);
void nyx_nova_battery(u32 percent, bool charging);
void nyx_nova_page(lv_obj_t *page, u32 tab);
const lv_style_t *nyx_nova_text_style(u32 role);
lv_obj_t *nyx_nova_text(lv_obj_t *parent, const char *text, int x, int y, int width, u32 role);
void nyx_nova_fit_text(lv_obj_t *label, int height);
lv_obj_t *nyx_nova_panel(lv_obj_t *parent, int x, int y, int width, int height);
lv_obj_t *nyx_nova_glyph(lv_obj_t *parent, const char *name, int x, int y, u32 pixels);
lv_obj_t *nyx_nova_icon_box(lv_obj_t *parent, const char *name, int x, int y, int size, bool selected);
void nyx_nova_icon_select(lv_obj_t *icon, bool selected);
void nyx_nova_grid_begin(lv_obj_t *page, u32 kind);
lv_obj_t *nyx_nova_grid_add(lv_obj_t *page, const char *help, lv_action_t action, lv_action_t hold, u32 id);
void nyx_nova_grid_finish(lv_obj_t *page);
void nyx_nova_diagnostic_result(void);
lv_res_t nyx_nova_grid_open(lv_obj_t *button);
void nyx_nova_home(lv_obj_t *page, lv_action_t activate);
void nyx_nova_carousel(lv_obj_t *page, lv_action_t activate);
void nyx_nova_about(lv_obj_t *page);
lv_obj_t *nyx_nova_toggle(lv_obj_t *parent, const char *title, lv_action_t action, bool enabled, lv_coord_t width, lv_coord_t height);
lv_obj_t *nyx_nova_switch_row(lv_obj_t *parent, const char *title, const char *help, const char *icon, lv_action_t action, bool enabled, int x, int y, int width, int height);
bool nyx_nova_toggle_refresh(lv_obj_t *button);
void nyx_nova_refresh_halos(void);
enum { NOVA_COLOR_BACKGROUND, NOVA_COLOR_TEXT, NOVA_COLOR_BORDER, NOVA_COLOR_REFLECTION, NOVA_COLOR_LOGO, NOVA_COLOR_ICONS, NOVA_COLOR_SELECTION };
lv_color_t nyx_nova_color(u32 role);
void nyx_nova_quick_actions(const lv_action_t actions[4]);
void nyx_nova_navigation_lock(bool lock);
void nyx_nova_reflect(lv_obj_t *target, bool selected);
void nyx_nova_reflection_pause(bool pause);
void nyx_nova_reflection_interaction(void);
void nyx_nova_reflection_reset(void);
void nyx_nova_window(lv_obj_t *win);
void nyx_nova_wallpaper_load(void);
void nyx_nova_launch_backdrop(lv_obj_t *card);
void nyx_nova_custom_icon(lv_obj_t *box,u32 index);
void nyx_nova_screen(lv_obj_t *screen);
u32 nyx_nova_temperature_rgb(u32 degrees);
void nyx_nova_backdrop(lv_obj_t *parent, int y);
lv_res_t nyx_nova_theme_window(lv_obj_t *button);
const char *nyx_nova_caption(const char *text);
lv_obj_t *nyx_nova_action(lv_obj_t *parent, const char *title, const char *description, const char *icon,
    lv_action_t action, u32 id, int x, int y, int width, int height);
struct _emmc_tool_gui_t;
void nyx_nova_fixed_page(lv_obj_t *page);
lv_obj_t *nyx_nova_report(lv_obj_t *parent,const char *text,int x,int y,int width,int height);
void nyx_nova_report_style(lv_obj_t *panel,const lv_style_t *style);
lv_obj_t *nyx_nova_emmc_summary(lv_obj_t *win,const char *values);
void nyx_nova_transfer(lv_obj_t *win,struct _emmc_tool_gui_t *context);
void nyx_nova_align(lv_obj_t *object,const lv_obj_t *base,lv_align_t alignment,lv_coord_t x,lv_coord_t y);
void nyx_nova_package_logs(lv_obj_t *win,lv_obj_t **logs);
void nyx_nova_archive_view(lv_obj_t *win,lv_obj_t **labels);
#endif

/* Rectangular hue/tint picker, matching the supplied reference. GPL-2.0-or-later. */
#ifndef NYX_COLOR_PICKER_H
#define NYX_COLOR_PICKER_H
typedef void (*nyx_color_changed_t)(lv_obj_t *picker,u32 rgb);
lv_obj_t *nyx_color_picker_create(lv_obj_t *parent,int x,int y,int width,int height,nyx_color_changed_t changed);
void nyx_color_picker_set(lv_obj_t *picker,u32 rgb);
u32 nyx_color_picker_rgb(lv_obj_t *picker);
#endif

/* Bounded PNG decoding and alpha-preserving scaling. GPL-2.0-or-later. */
#ifndef NOVA_IMAGE_H
#define NOVA_IMAGE_H
/* Returns an owned BGRA32 buffer, suitable for LV_IMG_CF_TRUE_COLOR_ALPHA.
 * cover crops to fill; otherwise letterbox to fit. No hardware or SD I/O. */
bool nova_image_png(const u8 *data,u32 bytes,u32 width,u32 height,bool cover,u32 **pixels);
#endif

/*
 * Copyright (c) 2018-2019 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GUI_OPTIONS_H_
#define _GUI_OPTIONS_H_


void nyx_options_clear_ini_changes_made();
bool nyx_options_get_ini_changes_made();
void first_time_clock_edit(void *param);
lv_res_t create_win_nyx_options(lv_obj_t *parrent_btn);
void create_tab_options(lv_theme_t *th, lv_obj_t *parent);
void create_tab_boot_options(lv_theme_t *th, lv_obj_t *parent);
bool nyx_options_has_changes(void);
bool nyx_options_commit_boot(void);
lv_res_t nyx_options_save_all(lv_obj_t *button);

#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_READABILITY_H
#define NYX_GUI_READABILITY_H

/* Adapt after positions/sizes are final. Does not alter application text,
 * button IDs, font choice, card heights or absolute layout positions. */
void nyx_label_adapt(lv_obj_t *label);
void nyx_readability_refresh(lv_obj_t *root);
/* UI setters can request a coalesced pass after the creating callback ends. */
void nyx_readability_request(void);
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_LAYOUT_H
#define NYX_GUI_LAYOUT_H

typedef struct {
    lv_theme_t *theme;
    lv_obj_t *page;
    lv_obj_t *previous_card;
    lv_coord_t y;
    lv_coord_t row_height;
    u32 count;
} nyx_action_grid;

void nyx_action_grid_init(nyx_action_grid *grid, lv_theme_t *theme, lv_obj_t *page, u32 kind);
/* Each button has exactly one label. Descriptions are non-clickable siblings,
 * preserving callbacks that use the button's first child as its title. */
lv_obj_t *nyx_action_grid_add(nyx_action_grid *grid, const char *title,
    const char *description, lv_action_t action, lv_action_t long_action, u32 id);
void nyx_action_grid_finish(nyx_action_grid *grid);
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_CHOICES_H
#define NYX_GUI_CHOICES_H

/* Inline choices own their labels. Actions receive the clicked button, whose
 * free number is the stable option index; the parent's number is the group. */
lv_obj_t *nyx_choices_create(lv_obj_t *parent, const char *options, u32 selected,
    u32 columns, lv_coord_t width, lv_coord_t row_height, lv_action_t action,
    u32 group, bool raw);
void nyx_choices_set_selected(lv_obj_t *choices, u32 selected);
void nyx_show_help(const char *message);
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_APPEARANCE_H
#define NYX_GUI_APPEARANCE_H

lv_res_t nyx_create_appearance_window(lv_obj_t *button);
void nyx_appearance_open_language(u32 language);
void nyx_appearance_open_font(void);
void nyx_appearance_open_size(u32 pixels);
void nyx_background_create(lv_obj_t *screen);
void nyx_background_pause(bool pause);
/* Freeze decoration while input is active; resumes after a short idle delay. */
void nyx_background_interaction(void);
/* Called by the settings window so its generic Save action cannot commit a
 * stale pending choice. Appearance owns its own explicit Save & reload. */
void nyx_options_appearance_active(bool active, bool saved);
#endif
/* SD font loading and bounded bitmap scaling. SPDX-License-Identifier: GPL-2.0-or-later */

#define FONT_LIMIT 16
#define SECTION_LIMIT 24
#define MAX_FILE_SIZE (16u * 1024u * 1024u)
#define MAX_SECTION_SIZE (2u * 1024u * 1024u)
#define MAX_GLYPHS 1024u

typedef struct {
    lv_font_t font;
    lv_font_glyph_dsc_t *glyphs;
    uint32_t *codepoints;
    u8 *bitmap;
} managed_font;
static managed_font regular, small, mono, regular_icons, small_icons;
static managed_font ui_faces[12];
static const lv_font_t *regular_font = &interui_30, *small_font = &interui_20, *mono_font = &ubuntu_mono;
static char font_files[FONT_LIMIT + 1][64];
static char font_options[(FONT_LIMIT + 1) * 65];
static u32 font_count, selected_size = 30;
static const char *status = "Default font";

static u16 read16(const u8 *p) { return p[0] | (u16)p[1] << 8; }
static u32 read32(const u8 *p) { return p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24; }
static u32 hash_bytes(const u8 *p, u32 length)
{
    u32 hash = 2166136261u;
    while (length--) { hash ^= *p++; hash *= 16777619u; }
    return hash;
}
bool nyx_font_filename_valid(const char *filename)
{
    if (!filename || !filename[0]) return false;
    u32 length = 0;
    while (length < 64 && filename[length]) ++length;
    if (length < 6 || length >= 64) return false;
    static const char extension[] = ".nyxf";
    for (u32 i = 0; i < 5; ++i) if ((filename[length - 5 + i] | 0x20) != extension[i]) return false;
    if (filename[0] == '.' || strstr(filename, "..")) return false;
    for (u32 i = 0; i < length; ++i) {
        u8 c = filename[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == ' '))
            return false;
    }
    return true;
}
bool nyx_font_validate_header(const u8 header[32], u32 file_size)
{
    return header && file_size >= 64 && file_size <= MAX_FILE_SIZE &&
        !memcmp(header, "NYXF", 4) && read16(header + 4) == 1 && read16(header + 6) == 32 &&
        read32(header + 8) == file_size && read32(header + 12) > 0 && read32(header + 12) <= SECTION_LIMIT &&
        read32(header + 16) == 32 && read32(header + 20) == 0 &&
        32 + 32 * read32(header + 12) <= file_size && read32(header + 28) == hash_bytes(header, 28);
}
static bool read_exact(FIL *file, u32 offset, void *buffer, u32 length)
{
    UINT read = 0;
    return f_lseek(file, offset) == FR_OK && f_read(file, buffer, length, &read) == FR_OK && read == length;
}
static bool open_font(const char *filename, FIL *file, u8 header[32])
{
    if (!nyx_font_filename_valid(filename)) return false;
    char path[72]; strcpy(path, "fonts/"); strcat(path, filename);
    if (f_open(file, path, FA_READ) != FR_OK) return false;
    if (f_size(file) > MAX_FILE_SIZE || !read_exact(file, 0, header, 32) || !nyx_font_validate_header(header, f_size(file))) {
        f_close(file); return false;
    }
    return true;
}
bool nyx_font_available(const char *filename)
{
    u32 size = n_cfg.font_size >= 20 && n_cfg.font_size <= 36 && !(n_cfg.font_size & 1) ? n_cfg.font_size : 30;
    return nyx_font_available_at_size(filename, size);
}

static int glyph_index(const lv_font_t *font, u32 codepoint)
{
    if (codepoint < font->unicode_first || codepoint > font->unicode_last) return -1;
    /* Every accepted text face includes the complete leading ASCII block.
     * Confirm its index for compatibility with icon/custom faces, then avoid
     * a binary search for each of the repeated width/bitmap/bpp queries. */
    if (codepoint >= 32 && codepoint <= 126) {
        u32 index = codepoint - 32;
        if (index < font->glyph_cnt && font->unicode_list[index] == codepoint) return index;
    }
    u32 low = 0, high = font->glyph_cnt;
    while (low < high) {
        u32 mid = low + (high - low) / 2;
        u32 current = font->unicode_list[mid];
        if (current == codepoint) return mid;
        if (current < codepoint) low = mid + 1; else high = mid;
    }
    return -1;
}
static const u8 *get_bitmap(const lv_font_t *font, uint32_t cp)
{
    int index = glyph_index(font, cp);
    return index < 0 ? NULL : font->glyph_bitmap + font->glyph_dsc[index].glyph_index;
}
static int16_t get_width(const lv_font_t *font, uint32_t cp)
{
    int index = glyph_index(font, cp);
    return index < 0 ? -1 : font->glyph_dsc[index].w_px;
}
static void release_font(managed_font *font)
{
    free(font->glyphs); free(font->codepoints); free(font->bitmap); memset(font, 0, sizeof(*font));
}
static bool allocate_font(managed_font *font, u32 count, u32 bytes, u32 height)
{
    memset(font, 0, sizeof(*font));
    if (!count || count > MAX_GLYPHS || !bytes || bytes > MAX_SECTION_SIZE || height < 8 || height > 48) return false;
    font->glyphs = calloc(count, sizeof(*font->glyphs));
    font->codepoints = calloc(count + 1, sizeof(*font->codepoints));
    font->bitmap = malloc(bytes);
    if (!font->glyphs || !font->codepoints || !font->bitmap) { release_font(font); return false; }
    font->font.glyph_bitmap = font->bitmap; font->font.glyph_dsc = font->glyphs;
    font->font.unicode_list = font->codepoints; font->font.glyph_cnt = count;
    font->font.h_px = height; font->font.bpp = 4;
    font->font.get_bitmap = get_bitmap; font->font.get_width = get_width;
    return true;
}
static bool load_face(FIL *file, const u8 *entry, managed_font *font)
{
    u32 height = read32(entry), count = read32(entry + 4), start = read32(entry + 8);
    u32 bitmap_start = read32(entry + 12), bytes = read32(entry + 16), end = read32(entry + 20);
    if (!count || count > MAX_GLYPHS || height < 8 || height > 48 || read32(entry + 28) != 0 ||
        count * 12 > MAX_SECTION_SIZE || bitmap_start < start || bitmap_start - start != count * 12 ||
        bytes > MAX_SECTION_SIZE || bytes > end || bitmap_start > end || bytes != end - bitmap_start || end > f_size(file)) return false;
    u32 section_size = end - start;
    if (section_size > MAX_SECTION_SIZE || !allocate_font(font, count, bytes, height)) return false;
    u8 *section = malloc(section_size);
    if (!section) { release_font(font); return false; }
    bool valid = read_exact(file, start, section, section_size) && hash_bytes(section, section_size) == read32(entry + 24);
    u32 previous_cp = 0, previous_end = 0;
    for (u32 i = 0; valid && i < count; ++i) {
        const u8 *glyph = section + i * 12;
        u32 cp = read32(glyph), width = read32(glyph + 4), offset = read32(glyph + 8);
        u32 glyph_bytes = ((width + 1) / 2) * height;
        if (cp < 32 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF) || cp <= previous_cp ||
            !width || width > 128 || offset != previous_end || offset > bytes || glyph_bytes > bytes - offset) valid = false;
        else {
            font->codepoints[i] = cp; font->glyphs[i].w_px = width; font->glyphs[i].glyph_index = offset;
            previous_cp = cp; previous_end = offset + glyph_bytes;
        }
    }
    if (valid && previous_end != bytes) valid = false;
    if (valid) {
        font->font.unicode_first = font->codepoints[0]; font->font.unicode_last = previous_cp;
        /* Missing ASCII or Cyrillic must never render a language unreadable. */
        for (u32 cp = 32; cp <= 126 && valid; ++cp) valid = glyph_index(&font->font, cp) >= 0;
        for (u32 cp = 0x400; cp <= 0x45F && valid; ++cp) valid = glyph_index(&font->font, cp) >= 0;
        if (valid) memcpy(font->bitmap, section + count * 12, bytes);
    }
    free(section);
    if (!valid) release_font(font);
    return valid;
}
static bool load_selected_into(const char *filename, u32 normal_height, u32 small_height,
    managed_font *normal_font, managed_font *hint_font)
{
    FIL file; u8 header[32], table[SECTION_LIMIT * 32];
    if (!open_font(filename, &file, header)) return false;
    u32 count = read32(header + 12), previous_end = 32 + count * 32;
    bool valid = read_exact(&file, 32, table, count * 32) && hash_bytes(table, count * 32) == read32(header + 24);
    const u8 *normal = NULL, *hint = NULL;
    u32 seen_heights = 0;
    for (u32 i = 0; valid && i < count; ++i) {
        const u8 *entry = table + i * 32;
        u32 height = read32(entry), start = read32(entry + 8), end = read32(entry + 20);
        if (height < 8 || height > 39 || (seen_heights & (1u << (height - 8))) || start != previous_end || end < start || end > f_size(&file)) valid = false;
        else {
            seen_heights |= 1u << (height - 8); previous_end = end;
            if (height == normal_height) normal = entry;
            if (height == small_height) hint = entry;
        }
    }
    valid = valid && previous_end == f_size(&file) && normal && hint;
    if (valid) valid = load_face(&file, normal, normal_font) && load_face(&file, hint, hint_font);
    if (f_close(&file) != FR_OK) valid = false;
    if (!valid) { release_font(normal_font); release_font(hint_font); }
    return valid;
}
static bool load_selected(const char *filename, u32 normal_height, u32 small_height)
{
    return load_selected_into(filename, normal_height, small_height, &regular, &small);
}
bool nyx_font_available_at_size(const char *filename, u32 size)
{
    if (size < 20 || size > 36 || size & 1) return false;
    if (!filename || !filename[0]) return true;
    if (!nyx_font_filename_valid(filename)) return false;
    bool was_mounted = sd_get_card_mounted();
    if (sd_mount()) return false;
    u32 small_size = ((size * 2 / 3) + 1) & ~1u;
    if (small_size < 14) small_size = 14;
    managed_font temporary_normal = {0}, temporary_small = {0};
    bool valid = load_selected_into(filename, size, small_size, &temporary_normal, &temporary_small);
    release_font(&temporary_normal); release_font(&temporary_small);
    if (!was_mounted) sd_unmount();
    return valid;
}

/* Scale existing, trusted font pixels for the built-in family and icon pages.
 * No TrueType parser or executable font code is run on the console. */
static u32 make_codepoints(u32 *list, bool icons)
{
    u32 count = 0;
    if (icons) { for (u32 cp = LV_SYMBOL_GLYPH_FIRST; cp <= LV_SYMBOL_GLYPH_LAST; ++cp) list[count++] = cp; }
    else {
        for (u32 cp = 32; cp <= 126; ++cp) list[count++] = cp;
        for (u32 cp = 0xA0; cp < 0x250; ++cp) list[count++] = cp;
        for (u32 cp = 0x400; cp < 0x500; ++cp) list[count++] = cp;
        static const u32 extra[] = {0x2010,0x2011,0x2013,0x2014,0x2018,0x2019,0x201A,0x201C,0x201D,0x201E,0x2022,0x2026,0x2030,0x2039,0x203A,0x20AC,0x2116,0x2122,0xFFFD};
        for (u32 i = 0; i < ARRAY_SIZE(extra); ++i) list[count++] = extra[i];
    }
    return count;
}
static bool scale_font(const lv_font_t *source, u32 height, managed_font *target, bool icons, bool fixed)
{
    u32 codepoints[MAX_GLYPHS], count = make_codepoints(codepoints, icons), total = 0;
    u32 widths[MAX_GLYPHS], source_height = source->h_px;
    u32 fixed_width = (10u * height + source_height / 2) / source_height;
    for (u32 i = 0; i < count; ++i) {
        u32 source_width = lv_font_get_real_width(source, codepoints[i]);
        widths[i] = fixed ? fixed_width : (source_width * height + source_height / 2) / source_height;
        if (!widths[i]) widths[i] = height / 2;
        if (widths[i] > 128) return false;
        total += ((widths[i] + 1) / 2) * height;
    }
    if (!allocate_font(target, count, total, height)) return false;
    memset(target->bitmap, 0, total);
    u32 offset = 0;
    for (u32 i = 0; i < count; ++i) {
        u32 cp = codepoints[i], width = widths[i];
        const u8 *source_bitmap = lv_font_get_bitmap(source, cp);
        u32 source_width = lv_font_get_real_width(source, cp), source_bpp = lv_font_get_bpp(source, cp);
        u32 source_stride = source_bpp == 8 ? source_width : (source_width + 1) / 2;
        target->codepoints[i] = cp; target->glyphs[i].w_px = width; target->glyphs[i].glyph_index = offset;
        if (source_bitmap && source_width && (source_bpp == 4 || source_bpp == 8)) {
            u32 drawn_width = fixed ? (source_width * height + source_height / 2) / source_height : width;
            if (!drawn_width) drawn_width = 1;
            if (drawn_width > width) drawn_width = width;
            u32 padding = (width - drawn_width) / 2;
            for (u32 y = 0; y < height; ++y) {
                u32 source_y = y * source_height / height;
                for (u32 x = 0; x < drawn_width; ++x) {
                    u32 source_x = x * source_width / drawn_width;
                    u8 byte = source_bitmap[source_y * source_stride + (source_bpp == 8 ? source_x : source_x / 2)];
                    u8 pixel = source_bpp == 8 ? (byte + 8) / 17 : (source_x & 1 ? byte & 15 : byte >> 4);
                    u32 destination_x = padding + x;
                    target->bitmap[offset + y * ((width + 1) / 2) + destination_x / 2] |= pixel << (destination_x & 1 ? 0 : 4);
                }
            }
        }
        offset += ((width + 1) / 2) * height;
    }
    target->font.unicode_first = target->codepoints[0]; target->font.unicode_last = target->codepoints[count - 1];
    if (fixed) target->font.monospace = fixed_width;
    return true;
}
static void scan_fonts(void)
{
    font_count = 1; font_files[0][0] = 0;
    strcpy(font_options, nyx_tr("Default"));
    DIR directory; FILINFO entry;
    if (f_opendir(&directory, "fonts") != FR_OK) return;
    while (font_count <= FONT_LIMIT && f_readdir(&directory, &entry) == FR_OK && entry.fname[0]) {
        if ((entry.fattrib & (AM_DIR | AM_HID)) || !nyx_font_filename_valid(entry.fname) || !nyx_font_available(entry.fname)) continue;
        strcpy(font_files[font_count++], entry.fname);
    }
    f_closedir(&directory);
    for (u32 i = 1; i < font_count; ++i)
        for (u32 j = i + 1; j < font_count; ++j)
            if (strcmp(font_files[i], font_files[j]) > 0) {
                char temp[64]; strcpy(temp, font_files[i]); strcpy(font_files[i], font_files[j]); strcpy(font_files[j], temp);
            }
    for (u32 i = 1; i < font_count; ++i) { strcat(font_options, "\n"); strcat(font_options, font_files[i]); }
}
void nyx_font_manager_init(void)
{
    for (u32 i = 0; i < ARRAY_SIZE(ui_faces); ++i) release_font(&ui_faces[i]);
    /* Only call while building a fresh screen; existing objects retain pointers. */
    release_font(&regular); release_font(&small); release_font(&mono); release_font(&regular_icons); release_font(&small_icons);
    regular_font = &interui_30; small_font = &interui_20; mono_font = &ubuntu_mono;
    selected_size = n_cfg.font_size >= 20 && n_cfg.font_size <= 36 && !(n_cfg.font_size & 1) ? n_cfg.font_size : 30;
    u32 small_size = ((selected_size * 2 / 3) + 1) & ~1u;
    if (small_size < 14) small_size = 14;
    bool was_mounted = sd_get_card_mounted();
    bool mounted = !sd_mount();
    if (mounted) scan_fonts();
    else { font_count = 1; font_files[0][0] = 0; strcpy(font_options, nyx_tr("Default")); }
    bool custom = mounted && n_cfg.font_file[0] && load_selected(n_cfg.font_file, selected_size, small_size);
    if (mounted && !was_mounted) sd_unmount();
    if (n_cfg.font_file[0]) status = custom ? "SD font loaded" : "Font unavailable or invalid. Default font restored.";
    else status = "Default font";
    if (custom) { regular_font = &regular.font; small_font = &small.font; }
    else if (selected_size != 30) {
        if (scale_font(&interui_30, selected_size, &regular, false, false) && scale_font(&interui_20, small_size, &small, false, false)) {
            regular_font = &regular.font; small_font = &small.font;
        } else { release_font(&regular); release_font(&small); selected_size = 30; small_size = 20; status = "Not enough memory. Default font restored."; }
    }
    bool icons_ok = true;
    if (regular_font == &regular.font) {
        icons_ok = scale_font(&hekate_symbol_30, selected_size, &regular_icons, true, false);
        if (icons_ok) regular.font.next_page = &regular_icons.font;
    }
    if (icons_ok && small_font == &small.font) {
        icons_ok = scale_font(&hekate_symbol_20, small_size, &small_icons, true, false);
        if (icons_ok) small.font.next_page = &small_icons.font;
    }
    if (!icons_ok) {
        release_font(&regular); release_font(&small); release_font(&regular_icons); release_font(&small_icons);
        regular_font = &interui_30; small_font = &interui_20; selected_size = 30; small_size = 20;
        status = "Not enough memory. Default font restored.";
    }
    if (scale_font(&ubuntu_mono, small_size, &mono, false, true)) mono_font = &mono.font;
}
const lv_font_t *nyx_font_regular(void) { return regular_font; }
const lv_font_t *nyx_font_small(void) { return small_font; }
const lv_font_t *nyx_font_ui(u32 pixels)
{
    if (pixels < 14) pixels = 14;
    if (pixels > 36) pixels = 36;
    u32 height = ((pixels * selected_size / 30) + 1) & ~1u;
    if (height < 14) height = 14;
    if (height > 36) height = 36;
    if (height == regular_font->h_px) return regular_font;
    if (height == small_font->h_px) return small_font;
    managed_font *face = &ui_faces[(height - 14) / 2];
    if (face->bitmap) return &face->font;
    bool was_mounted = sd_get_card_mounted();
    managed_font temporary = {0};
    bool loaded = false;
    if (n_cfg.font_file[0] && !sd_mount())
        loaded = load_selected_into(n_cfg.font_file, height, height, face, &temporary);
    release_font(&temporary);
    if (!was_mounted && sd_get_card_mounted()) sd_unmount();
    if (!loaded) loaded = scale_font(regular_font, height, face, false, false);
    return loaded ? &face->font : small_font;
}
const lv_font_t *nyx_font_mono(void) { return mono_font; }
u32 nyx_font_size(void) { return selected_size; }
const char *nyx_font_status(void) { return status; }
const char *nyx_font_options(void) { return font_options; }
const char *nyx_font_filename(u32 index) { return index < font_count ? font_files[index] : ""; }
u32 nyx_font_count(void) { return font_count; }
u32 nyx_font_selected_index(const char *filename)
{
    if (filename) for (u32 i = 0; i < font_count; ++i) if (!strcmp(font_files[i], filename)) return i;
    return 0;
}
/* Nyx UI translation boundary. GPL-2.0. */

typedef struct { const char *source; const char *text[I18N_LANGUAGE_COUNT]; } locale_entry;
#include "localization_catalog.inc"

const char *nyx_tr(const char *text)
{
    if (!text) return NULL;
    u32 lang = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
    u32 low = 0, high = ARRAY_SIZE(locale_catalog);
    while (low < high) {
        u32 middle = low + (high - low) / 2;
        int comparison = strcmp(text, locale_catalog[middle].source);
        if (!comparison) return locale_catalog[middle].text[lang];
        if (comparison < 0) high = middle; else low = middle + 1;
    }
    return text;
}

static const locale_entry *lookup_span(const char *text, u32 length)
{
    u32 low = 0, high = ARRAY_SIZE(locale_catalog);
    while (low < high) {
        u32 middle = low + (high - low) / 2;
        const char *source = locale_catalog[middle].source;
        int comparison = strncmp(text, source, length);
        if (!comparison && source[length]) comparison = -1;
        if (!comparison) return &locale_catalog[middle];
        if (comparison < 0) high = middle; else low = middle + 1;
    }
    return NULL;
}

void nyx_localization_init(void) { nyx_unicode_fonts_init(); }

/* Translate whole phrases first; otherwise preserve icons, matrix control
 * bytes, whitespace and LVGL color commands around translated fragments.
 * Every returned buffer is owned by the caller. No UI buffer is overwritten. */
typedef struct { char *data; u32 size, length; bool changed, failed; } text_builder;
static void append(text_builder *out, const char *text, u32 count)
{
    if (out->failed) return;
    if (out->length + count + 1 > out->size) {
        u32 capacity = (out->length + count + 1) * 2;
        char *next = malloc(capacity);
        if (!next) { out->failed = true; return; }
        if (out->length) memcpy(next, out->data, out->length);
        free(out->data);
        out->data = next; out->size = capacity;
    }
    memcpy(out->data + out->length, text, count);
    out->length += count; out->data[out->length] = 0;
}

/* Match formatted display text without ever writing a larger translation into
 * the caller's original (often small) printf buffer. Values and paths are
 * captured verbatim, and the catalog retains exactly the same argument order. */
static const char *format_end(const char *format, char *type)
{
    const char *cursor = format + 1;
    while (*cursor && ((*cursor >= '0' && *cursor <= '9') || *cursor == ' ' ||
        *cursor == '.' || *cursor == '-' || *cursor == '+' || *cursor == '#')) ++cursor;
    if (!strchr("dsuoxXpc", *cursor) || !*cursor) return NULL;
    *type = *cursor; return cursor + 1;
}
static void translate_piece(text_builder *out, const char *text, u32 length, u32 depth);
static bool translate_pattern(text_builder *out, const char *text, u32 length, u32 depth)
{
    u32 lang = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
    u32 first[2] = { '%', (u8)text[0] };
    if (first[1] < first[0]) { u32 swap = first[0]; first[0] = first[1]; first[1] = swap; }
    u32 buckets = first[0] == first[1] ? 1 : 2;
    for (u32 bucket = 0; bucket < buckets; ++bucket)
    for (u32 index = locale_pattern_start[first[bucket]]; index < locale_pattern_start[first[bucket] + 1]; ++index) {
        u32 row = locale_pattern_rows[index];
        const char *source = locale_catalog[row].source;
        const char *replacement = locale_catalog[row].text[lang];
        if (!strcmp(source, replacement)) continue;
        const char *starts[12]; u32 sizes[12], count = 0, at = 0;
        const char *format = source;
        bool match = true;
        while (*format && match) {
            if (*format != '%') {
                if (at >= length || text[at++] != *format++) match = false;
                continue;
            }
            if (format[1] == '%') {
                if (at >= length || text[at++] != '%') match = false;
                format += 2; continue;
            }
            char type;
            const char *end = format_end(format, &type);
            if (!end || count == ARRAY_SIZE(starts)) { match = false; break; }
            u32 group = 1;
            u32 start = at;
            if (type == 's') {
                /* Adjacent text fields retain their original concatenation.
                 * Their internal boundary is immaterial to the translated
                 * surrounding phrase, so preserve them as one captured span. */
                while (*end == '%') {
                    char next_type; const char *next = format_end(end, &next_type);
                    if (!next || next_type != 's') break;
                    ++group; end = next;
                }
                if (count + group > ARRAY_SIZE(starts)) { match = false; break; }
                if (!*end) at = length;
                else {
                    if (*end == '%') { match = false; break; }
                    const char *next = strchr(end, '%');
                    u32 literal = next ? (u32)(next - end) : strlen(end);
                    while (at <= length && (literal > length - at || memcmp(text + at, end, literal))) ++at;
                    if (at > length) { match = false; break; }
                }
            } else if (type == 'c') {
                if (at == length) { match = false; break; }
                ++at;
            } else {
                while (at < length && text[at] == ' ') ++at;
                if (at < length && (text[at] == '-' || text[at] == '+')) ++at;
                u32 digits = at;
                while (at < length && ((text[at] >= '0' && text[at] <= '9') ||
                    ((type == 'x' || type == 'X' || type == 'p') &&
                    ((text[at] >= 'A' && text[at] <= 'F') || (text[at] >= 'a' && text[at] <= 'f'))))) ++at;
                if (at == digits) { match = false; break; }
            }
            starts[count] = text + start; sizes[count++] = at - start;
            while (--group) { starts[count] = text + at; sizes[count++] = 0; }
            format = end;
        }
        if (!match || *format || at != length || !count) continue;
        u32 arg = 0; format = replacement;
        while (*format) {
            if (*format != '%') { append(out, format++, 1); continue; }
            if (format[1] == '%') { append(out, "%", 1); format += 2; continue; }
            char type; const char *end = format_end(format, &type);
            if (!end || arg == count) break; // Generator rejects this case.
            if (type == 's' && depth < 8 && !memchr(starts[arg], '/', sizes[arg]) &&
                !memchr(starts[arg], '\\', sizes[arg]) && !memchr(starts[arg], '.', sizes[arg]))
                translate_piece(out, starts[arg], sizes[arg], depth + 1);
            else append(out, starts[arg], sizes[arg]);
            ++arg; format = end;
        }
        out->changed = true; return true;
    }
    return false;
}

static void translate_piece(text_builder *out, const char *text, u32 length, u32 depth)
{
    if (!length || out->failed) return;
    const locale_entry *entry = lookup_span(text, length);
    if (entry) {
        u32 language = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
        const char *translated = entry->text[language];
        u32 translated_length = strlen(translated);
        append(out, translated, translated_length);
        if (length != translated_length || memcmp(text, translated, length)) out->changed = true;
        return;
    }
    if (depth < 8 && translate_pattern(out, text, length, depth)) return;
    if (depth > 32) { append(out, text, length); return; }
    /* Matrix width/state control byte, independent of UTF-8 text. */
    if (((u8)text[0] & 0xC0) == 0x80) {
        append(out, text, 1); translate_piece(out, text + 1, length - 1, depth + 1); return;
    }
    /* Private-use Hekate symbol U+F800..F831; preserve all three bytes. */
    if (length >= 3 && (u8)text[0] == 0xEF && (u8)text[1] == 0xA0) {
        append(out, text, 3); translate_piece(out, text + 3, length - 3, depth + 1); return;
    }
    u32 left = 0, right = length;
    while (left < right && (text[left] == ' ' || text[left] == '\t')) ++left;
    while (right > left && (text[right - 1] == ' ' || text[right - 1] == '\t')) --right;
    if (left || right != length) {
        append(out, text, left); translate_piece(out, text + left, right - left, depth + 1);
        append(out, text + right, length - right); return;
    }
    /* Lines are siblings, so even long option lists have bounded depth. */
    u32 line_start = 0;
    for (u32 i = 0; i < length; ++i) {
        if (text[i] != '\n') continue;
        translate_piece(out, text + line_start, i - line_start, depth + 1);
        append(out, "\n", 1); line_start = i + 1;
    }
    if (line_start) {
        translate_piece(out, text + line_start, length - line_start, depth + 1); return;
    }
    /* Technical tables often assemble a translated field name and value
     * separately. Only split at a recognized label, preserving arbitrary
     * user text and path-like values when the prefix is unknown. */
    for (u32 i = 0; i + 1 < length; ++i) {
        if (text[i] != ':') continue;
        text_builder prefix = {0};
        translate_piece(&prefix, text, i + 1, depth + 1);
        if (prefix.changed && !prefix.failed) {
            append(out, prefix.data, prefix.length); free(prefix.data);
            out->changed = true;
            translate_piece(out, text + i + 1, length - i - 1, depth + 1); return;
        }
        free(prefix.data);
    }
    /* Split only at presentation boundaries, never inside words or paths. */
    for (u32 i = 0; i < length; ++i) {
        if (i + 2 < length && (u8)text[i] == 0xEF && (u8)text[i + 1] == 0xA0) {
            translate_piece(out, text, i, depth + 1); append(out, text + i, 3);
            translate_piece(out, text + i + 3, length - i - 3, depth + 1); return;
        }
        if (text[i] == '#') {
            u32 color_end = i + 1;
            while (color_end < length && color_end < i + 7 &&
                ((text[color_end] >= '0' && text[color_end] <= '9') ||
                 (text[color_end] >= 'A' && text[color_end] <= 'F') ||
                 (text[color_end] >= 'a' && text[color_end] <= 'f'))) ++color_end;
            if (color_end == i + 7 && color_end < length && text[color_end] == ' ') {
                u32 end = color_end + 1;
                while (end < length && text[end] != '#') ++end;
                if (end < length) {
                    translate_piece(out, text, i, depth + 1);
                    append(out, text + i, color_end + 1 - i);
                    translate_piece(out, text + color_end + 1, end - color_end - 1, depth + 1);
                    append(out, "#", 1);
                    translate_piece(out, text + end + 1, length - end - 1, depth + 1); return;
                }
            }
        }
    }
    append(out, text, length);
}

char *nyx_translate_text(const char *text)
{
    if (!text) return NULL;
    const char *direct = nyx_tr(text);
    if (direct != text) {
        if (!strcmp(direct, text)) return NULL;
        u32 length = strlen(direct) + 1;
        char *copy = malloc(length); if (copy) memcpy(copy, direct, length);
        return copy;
    }
    bool letters = false;
    for (const char *at = text; *at; ++at)
        if ((*at >= 'A' && *at <= 'Z') || (*at >= 'a' && *at <= 'z')) { letters = true; break; }
    if (!letters) return NULL;
    text_builder out = {0};
    translate_piece(&out, text, strlen(text), 0);
    if (!out.changed || out.failed) { free(out.data); return NULL; }
    return out.data;
}
#define translated_copy nyx_translate_text

void nyx_label_set_text(lv_obj_t *label, const char *text)
{
    char *tr = translated_copy(text); lv_label_set_text(label, tr ? tr : text); free(tr);
    nyx_readability_request();
}
void nyx_label_set_static_text(lv_obj_t *label, const char *text)
{
    char *tr = translated_copy(text);
    if (tr) lv_label_set_text(label, tr);
    else lv_label_set_static_text(label, text);
    free(tr);
    nyx_readability_request();
}
void nyx_mbox_set_text(lv_obj_t *mbox, const char *text)
{
    char *tr = translated_copy(text); lv_mbox_set_text(mbox, tr ? tr : text); free(tr);
}
void nyx_ddlist_set_options(lv_obj_t *list, const char *options)
{
    char *tr = translated_copy(options); lv_ddlist_set_options(list, tr ? tr : options); free(tr);
}
void nyx_roller_set_options(lv_obj_t *roller, const char *options)
{
    char *tr = translated_copy(options); lv_roller_set_options(roller, tr ? tr : options); free(tr);
}
void nyx_win_set_title(lv_obj_t *win, const char *title)
{
    char *tr = translated_copy(title); lv_win_set_title(win, tr ? tr : title); free(tr);
    nyx_readability_request();
}
lv_obj_t *nyx_win_add_btn(lv_obj_t *win, const void *image, const char *label, lv_action_t action)
{
    char *tr = translated_copy(label);
    lv_obj_t *button = lv_win_add_btn(win, image, tr ? tr : label, action); free(tr);
    nyx_readability_request(); return button;
}
lv_obj_t *nyx_tabview_add_tab(lv_obj_t *tabs, const char *title)
{
    char *tr = translated_copy(title);
    lv_obj_t *tab = lv_tabview_add_tab(tabs, tr ? tr : title); free(tr);
    nyx_readability_request(); return tab;
}

/* Button matrices retain the map pointers, unlike labels. Keep translated
 * copies on the object lifetime; never mutate a caller's static/dynamic map. */
typedef struct owned_map {
    struct owned_map *next; lv_obj_t *object;
    lv_signal_func_t original_signal; const char **map; u32 count;
} owned_map;
static owned_map *maps;
static void free_map(owned_map *record)
{
    for (u32 i = 0; i < record->count; ++i) free((void *)record->map[i]);
    free(record->map); record->map = NULL; record->count = 0;
}
static lv_res_t map_signal(lv_obj_t *obj, lv_signal_t signal, void *param)
{
    owned_map **link = &maps;
    while (*link && (*link)->object != obj) link = &(*link)->next;
    if (!*link) return LV_RES_OK;
    owned_map *record = *link;
    lv_res_t result = record->original_signal(obj, signal, param);
    if (signal == LV_SIGNAL_CLEANUP) { *link = record->next; free_map(record); free(record); }
    return result;
}
void nyx_btnm_set_map(lv_obj_t *buttons, const char **map)
{
    if (!map) return;
    u32 count = 0;
    while (map[count][0]) ++count;
    const char **copy = calloc(count + 1, sizeof(char *));
    if (!copy) { lv_btnm_set_map(buttons, map); return; }
    bool changed = false;
    for (u32 i = 0; i < count; ++i) {
        char *tr = translated_copy(map[i]);
        if (tr) changed = true;
        else { tr = malloc(strlen(map[i]) + 1); if (tr) strcpy(tr, map[i]); }
        copy[i] = tr;
        if (!tr) {
            for (u32 j = 0; j < i; ++j) free((void *)copy[j]);
            free(copy); lv_btnm_set_map(buttons, map); return;
        }
    }
    copy[count] = "";
    owned_map *record = maps;
    while (record && record->object != buttons) record = record->next;
    if (!changed) {
        for (u32 i = 0; i < count; ++i) free((void *)copy[i]);
        free(copy);
        lv_btnm_set_map(buttons, map);
        if (record) free_map(record);
        return;
    }
    if (!record) {
        record = calloc(1, sizeof(*record));
        if (!record) {
            for (u32 i = 0; i < count; ++i) free((void *)copy[i]);
            free(copy);
            lv_btnm_set_map(buttons, map); return;
        }
        record->object = buttons; record->original_signal = lv_obj_get_signal_func(buttons);
        record->next = maps; maps = record; lv_obj_set_signal_func(buttons, map_signal);
    }
    lv_btnm_set_map(buttons, copy); free_map(record);
    record->map = copy; record->count = count;
}
void nyx_mbox_add_btns(lv_obj_t *mbox, const char **map, lv_btnm_action_t action)
{
    lv_mbox_add_btns(mbox, map, action);
    lv_mbox_ext_t *ext = lv_obj_get_ext_attr(mbox);
    nyx_btnm_set_map(ext->btnm, map);
}
static bool legacy_scale_font(const lv_font_t *source, u32 height, managed_font *target, bool icons, bool fixed)
{
    u32 codepoints[MAX_GLYPHS], count = make_codepoints(codepoints, icons), total = 0;
    u32 widths[MAX_GLYPHS], source_height = source->h_px;
    u32 fixed_width = (10u * height + source_height / 2) / source_height;
    for (u32 i = 0; i < count; ++i) {
        u32 source_width = lv_font_get_width(source, codepoints[i]);
        widths[i] = fixed ? fixed_width : (source_width * height + source_height / 2) / source_height;
        if (!widths[i]) widths[i] = height / 2;
        if (widths[i] > 128) return false;
        total += ((widths[i] + 1) / 2) * height;
    }
    if (!allocate_font(target, count, total, height)) return false;
    memset(target->bitmap, 0, total);
    u32 offset = 0;
    for (u32 i = 0; i < count; ++i) {
        u32 cp = codepoints[i], width = widths[i];
        const u8 *source_bitmap = lv_font_get_bitmap(source, cp);
        u32 source_width = lv_font_get_width(source, cp), source_bpp = lv_font_get_bpp(source, cp);
        u32 source_stride = source_bpp == 8 ? source_width : (source_width + 1) / 2;
        target->codepoints[i] = cp; target->glyphs[i].w_px = width; target->glyphs[i].glyph_index = offset;
        if (source_bitmap && source_width && (source_bpp == 4 || source_bpp == 8)) {
            for (u32 y = 0; y < height; ++y) {
                u32 source_y = y * source_height / height;
                for (u32 x = 0; x < width; ++x) {
                    u32 source_x = x * source_width / width;
                    u8 byte = source_bitmap[source_y * source_stride + (source_bpp == 8 ? source_x : source_x / 2)];
                    u8 pixel = source_bpp == 8 ? (byte + 8) / 17 : (source_x & 1 ? byte & 15 : byte >> 4);
                    target->bitmap[offset + y * ((width + 1) / 2) + x / 2] |= pixel << (x & 1 ? 0 : 4);
                }
            }
        }
        offset += ((width + 1) / 2) * height;
    }
    target->font.unicode_first = target->codepoints[0]; target->font.unicode_last = target->codepoints[count - 1];
    if (fixed) target->font.monospace = fixed_width;
    return true;
}

#define lv_label_set_text(...) nyx_label_set_text(__VA_ARGS__)
#define lv_label_set_static_text(...) nyx_label_set_static_text(__VA_ARGS__)
#define lv_mbox_set_text(...) nyx_mbox_set_text(__VA_ARGS__)
#define lv_ddlist_set_options(...) nyx_ddlist_set_options(__VA_ARGS__)
#define lv_roller_set_options(...) nyx_roller_set_options(__VA_ARGS__)
#define lv_mbox_add_btns(...) nyx_mbox_add_btns(__VA_ARGS__)
#define lv_btnm_set_map(...) nyx_btnm_set_map(__VA_ARGS__)
#define lv_win_set_title(...) nyx_win_set_title(__VA_ARGS__)
#define lv_win_add_btn(...) nyx_win_add_btn(__VA_ARGS__)
#define lv_tabview_add_tab(...) nyx_tabview_add_tab(__VA_ARGS__)
#define lv_obj_align(...) nyx_nova_align(__VA_ARGS__)
/* Fast white diagonal sheen and a bounded blue/violet selection border.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#define REFLECTION_TARGETS 32
#define REFLECTION_PERIOD 20
#define REFLECTION_SWEEP 12
#define REFLECTION_FIRST (REFLECTION_SWEEP + 150) /* 3 seconds after pass. */
#define REFLECTION_CYCLE (REFLECTION_FIRST + REFLECTION_SWEEP + 250) /* Then 5 seconds. */
#define REFLECTION_STRIPS 16
#define REFLECTION_HALF 64
typedef struct {
    lv_obj_t *target,*overlay;
    lv_area_t dirty[REFLECTION_STRIPS];
    u32 phase,dirty_count;
    bool painted;
} reflection_target;
static reflection_target reflection_targets[REFLECTION_TARGETS];
static lv_task_t *reflection_task;
static lv_signal_func_t reflection_base_signal;
static u32 reflection_phase,reflection_pause_depth,reflection_interaction_tick;
static bool reflection_interacting;
static bool reflection_visible(lv_obj_t *target, lv_area_t *visible)
{
    *visible = (lv_area_t){0, 0, LV_HOR_RES - 1, LV_VER_RES - 1};
    for (lv_obj_t *object = target; object; object = lv_obj_get_parent(object)) {
        if (lv_obj_get_hidden(object)) return false;
        lv_area_t coords, overlap;
        lv_obj_get_coords(object, &coords);
        if (!lv_area_intersect(&overlap, &coords, visible)) return false;
        *visible = overlap;
    }
    /* Screen-level windows/modal covers obscure the content below them.
     * The top navigation remains visible above a Nova window. */
    lv_obj_t *root = target;
    while (lv_obj_get_parent(root) && lv_obj_get_parent(root) != lv_scr_act()) root = lv_obj_get_parent(root);
    for (lv_obj_t *child = lv_obj_get_child(lv_scr_act(), NULL); child && child != root; child = lv_obj_get_child(lv_scr_act(), child)) {
        if (lv_obj_get_hidden(child)) continue;
        lv_area_t coords, overlap;
        lv_obj_get_coords(child, &coords);
        if (lv_area_intersect(&overlap, visible, &coords) &&
            coords.x1 <= visible->x1 && coords.x2 >= visible->x2 &&
            coords.y1 <= visible->y1 && coords.y2 >= visible->y2) {
            lv_obj_type_t type; lv_obj_get_type(child, &type);
            if ((type.type[0] && !strcmp(type.type[0], "lv_win")) ||
                (lv_obj_get_click(child) && lv_obj_get_width(child) >= LV_HOR_RES - 2 && lv_obj_get_height(child) >= NYX_NOVA_BOTTOM)) return false;
        }
    }
    return true;
}




/* One reusable 129-pixel row. Raised-cosine opacity reaches zero at both
 * edges. Only this 516-byte constant is sampled; no tile-sized image. */
static const u32 reflection_gradient_row[]={
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,
    0x02FFFFFF,0x03FFFFFF,0x03FFFFFF,0x04FFFFFF,0x04FFFFFF,0x05FFFFFF,0x05FFFFFF,0x06FFFFFF,
    0x06FFFFFF,0x07FFFFFF,0x08FFFFFF,0x08FFFFFF,0x09FFFFFF,0x0AFFFFFF,0x0BFFFFFF,0x0BFFFFFF,
    0x0CFFFFFF,0x0DFFFFFF,0x0EFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x10FFFFFF,0x11FFFFFF,0x12FFFFFF,
    0x13FFFFFF,0x14FFFFFF,0x15FFFFFF,0x16FFFFFF,0x17FFFFFF,0x17FFFFFF,0x18FFFFFF,0x19FFFFFF,
    0x1AFFFFFF,0x1BFFFFFF,0x1BFFFFFF,0x1CFFFFFF,0x1DFFFFFF,0x1EFFFFFF,0x1EFFFFFF,0x1FFFFFFF,
    0x20FFFFFF,0x20FFFFFF,0x21FFFFFF,0x21FFFFFF,0x22FFFFFF,0x22FFFFFF,0x23FFFFFF,0x23FFFFFF,
    0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,
    0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,
    0x24FFFFFF,0x23FFFFFF,0x23FFFFFF,0x22FFFFFF,0x22FFFFFF,0x21FFFFFF,0x21FFFFFF,0x20FFFFFF,
    0x20FFFFFF,0x1FFFFFFF,0x1EFFFFFF,0x1EFFFFFF,0x1DFFFFFF,0x1CFFFFFF,0x1BFFFFFF,0x1BFFFFFF,
    0x1AFFFFFF,0x19FFFFFF,0x18FFFFFF,0x17FFFFFF,0x17FFFFFF,0x16FFFFFF,0x15FFFFFF,0x14FFFFFF,
    0x13FFFFFF,0x12FFFFFF,0x11FFFFFF,0x10FFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0EFFFFFF,0x0DFFFFFF,
    0x0CFFFFFF,0x0BFFFFFF,0x0BFFFFFF,0x0AFFFFFF,0x09FFFFFF,0x08FFFFFF,0x08FFFFFF,0x07FFFFFF,
    0x06FFFFFF,0x06FFFFFF,0x05FFFFFF,0x05FFFFFF,0x04FFFFFF,0x04FFFFFF,0x03FFFFFF,0x03FFFFFF,
    0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,
};

static int reflection_center(const reflection_target *target)
{
    int width=lv_obj_get_width(target->target),height=lv_obj_get_height(target->target);
    u32 phase=target->phase%REFLECTION_CYCLE;
    if(phase>=REFLECTION_FIRST)phase-=REFLECTION_FIRST;
    return -REFLECTION_HALF+(int)phase*(width+height/2+2*REFLECTION_HALF)/(REFLECTION_SWEEP-1);
}
static bool reflection_sweeping(u32 phase)
{
    phase%=REFLECTION_CYCLE;
    return phase<REFLECTION_SWEEP||(phase>=REFLECTION_FIRST&&phase<REFLECTION_FIRST+REFLECTION_SWEEP);
}
static void reflection_bounds(reflection_target *target)
{
    lv_area_t coords;lv_obj_get_coords(target->target,&coords);
    int height=lv_obj_get_height(target->target),center=reflection_center(target);
    target->dirty_count=0;
    /* Bounded strips avoid a full-tile rectangle. Transitions merge the
     * old/new spans before using LVGL's 32-entry invalidation queue. */
    for(u32 i=0;i<REFLECTION_STRIPS;++i){
        int top=i*height/REFLECTION_STRIPS,bottom=(i+1)*height/REFLECTION_STRIPS-1;
        lv_area_t band={coords.x1+center-bottom/2-REFLECTION_HALF-2,coords.y1+top,
            coords.x1+center-top/2+REFLECTION_HALF+2,coords.y1+bottom};
        lv_area_t clipped;
        if(lv_area_intersect(&clipped,&coords,&band))target->dirty[target->dirty_count++]=clipped;
    }
}
static void reflection_invalidate(const reflection_target *target,const lv_area_t *visible)
{
    for(u32 i=0;i<target->dirty_count;++i){lv_area_t clipped;
        if(lv_area_intersect(&clipped,&target->dirty[i],visible))lv_inv_area(&clipped);
    }
}
static void reflection_transition(const reflection_target *before,const reflection_target *after,const lv_area_t *visible)
{
    bool merged[REFLECTION_STRIPS]={false};
    for(u32 i=0;before->painted&&i<before->dirty_count;++i){
        lv_area_t area=before->dirty[i],clip;
        for(u32 j=0;after->painted&&j<after->dirty_count;++j){
            if(after->dirty[j].y1!=area.y1||after->dirty[j].y2!=area.y2)continue;
            if(after->dirty[j].x1<area.x1)area.x1=after->dirty[j].x1;
            if(after->dirty[j].x2>area.x2)area.x2=after->dirty[j].x2;
            merged[j]=true;break;
        }
        if(lv_area_intersect(&clip,&area,visible))lv_inv_area(&clip);
    }
    for(u32 j=0;after->painted&&j<after->dirty_count;++j)if(!merged[j]){
        lv_area_t clip;if(lv_area_intersect(&clip,&after->dirty[j],visible))lv_inv_area(&clip);
    }
}
static void reflection_border_invalidate(const reflection_target *target,const lv_area_t *visible)
{
    lv_area_t c;lv_obj_get_coords(target->target,&c);
    const lv_area_t edges[]={
        {c.x1,c.y1,c.x2,c.y1+1},{c.x1,c.y2-1,c.x2,c.y2},
        {c.x1,c.y1+2,c.x1+7,c.y2-2},{c.x2-7,c.y1+2,c.x2,c.y2-2}
    };
    for(u32 i=0;i<4;++i){lv_area_t clip;if(lv_area_intersect(&clip,&edges[i],visible))lv_inv_area(&clip);}
}

static void reflection_border_draw(const reflection_target *target,const lv_area_t *coords,const lv_area_t *clip)
{
    /* A 1-KiB palette and two-pixel row spans; no texture, heap allocation
     * or full-window invalidation. Fixed-point perimeter positions avoid
     * integer division for each pixel on Tegra's ARM control processor. */
    static lv_color_t palette[256];static bool ready;
    if(!ready){for(u32 i=0;i<256;++i){u32 t=i<128?i:255-i;palette[i]=LV_COLOR_MAKE(53+((103*t)>>7),144-((45*t)>>7),255);}ready=true;}
    int width=coords->x2-coords->x1+1,height=coords->y2-coords->y1+1;
    u32 step=(256u<<16)/(2u*(width+height)),phase=target->phase*4u;
    static const u8 rounded[]={7,5,4,3,2,1,1,0};
    for(int y=clip->y1;y<=clip->y2;++y){
        int row=y-coords->y1,edge=row<height-1-row?row:height-1-row;
        int inset=edge<8?rounded[edge]:0;
        if(edge<2){
            int left=coords->x1+inset,right=coords->x2-inset;
            if(left<clip->x1)left=clip->x1;if(right>clip->x2)right=clip->x2;
            for(int x=left;x<=right;x+=4){
                u32 pos=row<height/2?x-coords->x1:2*width+height-1-(x-coords->x1);
                lv_area_t span={x,y,x+3>right?right:x+3,y};
                lv_vfill(&span,clip,palette[(((pos*step)>>16)+phase)&255u],LV_OPA_COVER);
            }
        }else{
            lv_area_t left={coords->x1+inset,y,coords->x1+inset+1,y};
            lv_area_t right={coords->x2-inset-1,y,coords->x2-inset,y};
            u32 lp=2*width+2*height-1-row,rp=width+row;
            lv_vfill(&left,clip,palette[(((lp*step)>>16)+phase)&255u],LV_OPA_COVER);
            lv_vfill(&right,clip,palette[(((rp*step)>>16)+phase)&255u],LV_OPA_COVER);
        }
    }
}
static bool reflection_design(lv_obj_t *object,const lv_area_t *mask,lv_design_mode_t mode)
{
    if(mode==LV_DESIGN_COVER_CHK)return false;
    if(mode!=LV_DESIGN_DRAW_MAIN||!n_cfg.nova_halo||reflection_pause_depth)return true;
    reflection_target *target=lv_obj_get_free_ptr(object);
    if(!target)return true;
    lv_area_t coords,clip;lv_obj_get_coords(object,&coords);
    if(!lv_area_intersect(&clip,&coords,mask))return true;
    reflection_border_draw(target,&coords,&clip);
    if(!target->painted||lv_obj_get_width(object)<100||lv_obj_get_height(object)<64)return true;
    int center=reflection_center(target),height=lv_obj_get_height(object);
    static const u8 corner_inset[]={7,5,4,3,2,2,2,2};
    /* Direct row spans: no large alpha image, no raster buffer, no per-frame
     * allocation. The border and the rounded corners remain untouched. */
    for(int y=clip.y1;y<=clip.y2;++y){
        int row=y-coords.y1,edge=row<height-1-row?row:height-1-row;
        if(edge<1)continue;
        int inset=edge<8?corner_inset[edge]:2;
        lv_area_t row_clip={coords.x1+inset,y,coords.x2-inset,y},effective;
        if(!lv_area_intersect(&effective,&row_clip,&clip))continue;
        int left=coords.x1+center-row/2-REFLECTION_HALF;
        lv_area_t span={left,y,left+2*REFLECTION_HALF,y};
        if(span.x2<effective.x1||span.x1>effective.x2)continue;
        /* Per-pixel alpha retains the transparent fade below 16/255. */
        lv_vmap(&span,&effective,(const u8 *)reflection_gradient_row,
            LV_OPA_COVER,false,true,LV_COLOR_WHITE,LV_OPA_TRANSP);
    }
    return true;
}
static lv_res_t reflection_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    reflection_target *target=lv_obj_get_free_ptr(object);
    lv_res_t result=reflection_base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP&&target)memset(target,0,sizeof(*target));
    return result;
}
static void reflection_frame(void *unused)
{
    (void)unused;if(reflection_pause_depth||!n_cfg.nova_halo)return;
    if(reflection_interacting&&lv_tick_elaps(reflection_interaction_tick)<160)return;
    reflection_interacting=false;++reflection_phase;
    for(u32 i=0;i<REFLECTION_TARGETS;++i){
        reflection_target *target=&reflection_targets[i];if(!target->target)continue;
        lv_area_t visible;
        if(!reflection_visible(target->target,&visible)){target->painted=false;continue;}
        reflection_target before=*target;
        reflection_border_invalidate(target,&visible);
        target->phase=reflection_phase;target->painted=reflection_sweeping(reflection_phase);
        if(lv_obj_get_width(target->target)<100||lv_obj_get_height(target->target)<64)target->painted=false;
        if(target->painted)reflection_bounds(target);
        reflection_transition(&before,target,&visible);
    }
}
void nyx_nova_reflect(lv_obj_t *object,bool selected)
{
    if(!object)return;
    selected=selected&&n_cfg.nova_halo;
    reflection_target *vacant=NULL;
    for(u32 i=0;i<REFLECTION_TARGETS;++i){
        reflection_target *target=&reflection_targets[i];
        if(target->target==object){if(!selected)lv_obj_del(target->overlay);return;}
        if(!target->target&&!vacant)vacant=target;
    }
    if(!selected||!vacant||lv_obj_get_width(object)<24||lv_obj_get_height(object)<24)return;
    lv_obj_t *overlay=lv_obj_create(object,NULL);if(!overlay)return;
    lv_obj_set_style(overlay,&lv_style_transp_tight);lv_obj_set_click(overlay,false);
    lv_obj_set_protect(overlay,LV_PROTECT_PARENT);
    lv_obj_set_size(overlay,lv_obj_get_width(object),lv_obj_get_height(object));lv_obj_set_pos(overlay,0,0);
    vacant->target=object;vacant->overlay=overlay;vacant->phase=reflection_phase;vacant->painted=reflection_sweeping(reflection_phase);
    reflection_bounds(vacant);lv_obj_set_free_ptr(overlay,vacant);
    if(!reflection_base_signal)reflection_base_signal=lv_obj_get_signal_func(overlay);
    lv_obj_set_signal_func(overlay,reflection_signal);lv_obj_set_design_func(overlay,reflection_design);
    if(!reflection_task)reflection_task=lv_task_create(reflection_frame,REFLECTION_PERIOD,LV_TASK_PRIO_LOWEST,NULL);
}
void nyx_nova_reflection_reset(void)
{
    if(reflection_task)lv_task_del(reflection_task);
    reflection_task=NULL;
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].overlay)lv_obj_del(reflection_targets[i].overlay);
    memset(reflection_targets,0,sizeof(reflection_targets));reflection_phase=reflection_pause_depth=0;reflection_interacting=false;
}
void nyx_nova_reflection_pause(bool pause)
{
    if(pause)++reflection_pause_depth;else if(reflection_pause_depth)--reflection_pause_depth;
    lv_area_t screen={0,0,LV_HOR_RES-1,LV_VER_RES-1};
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].target){
        if(reflection_targets[i].painted)reflection_invalidate(&reflection_targets[i],&screen);
        reflection_border_invalidate(&reflection_targets[i],&screen);reflection_targets[i].painted=false;
    }
    if(reflection_task)lv_task_set_prio(reflection_task,reflection_pause_depth?LV_TASK_PRIO_OFF:LV_TASK_PRIO_LOWEST);
}
void nyx_nova_reflection_interaction(void)
{reflection_interaction_tick=lv_tick_get();reflection_interacting=true;}
/* Native implementation of the selected Nova reference. */
/* SPDX-License-Identifier: GPL-2.0-or-later */

typedef struct { const char *id, *title, *description, *icon; } nova_item;
/* Text and icons extracted verbatim from the selected Nova mockup. */
static const nova_item nova_reference_home[] = {
{"launch","Démarrer","Choisir le système à lancer sur la console.","play"},
{"profiles","Configurations","Retrouver les autres profils de démarrage.","layers-2"},
{"payloads","Payloads","Ouvrir un outil depuis la carte microSD.","folder-open"},
{"emummc","emuMMC","Gérer la copie du système sur la carte SD.","database"},
{"diag","Diagnostic","Explorer les contrôles matériels.","activity"},
};
static const nova_item nova_reference_tools[] = {
{"backup","Sauvegarde eMMC","Créer une sauvegarde du stockage.","hard-drive-download"},
{"restore","Restauration eMMC","Restaurer une sauvegarde existante.","hard-drive-upload"},
{"partition","Partitions SD","Préparer les partitions de la carte SD.","columns-3"},
{"usb","Outils USB","Accéder aux fonctions de stockage USB.","usb"},
{"archive","Bit d’archive","Réparer les attributs des fichiers.","folder-check"},
{"touch","Écran tactile","Ajuster la calibration du tactile.","scan-line"},
{"rcm","AutoRCM","Consulter et modifier le mode AutoRCM.","power"},
{"package","Package1 / 2","Extraire les éléments du système.","boxes"},
};
static const nova_item nova_reference_info[] = {
{"hardware","Matériel et fusibles","Identité de la console et mémoire.","circuit-board"},
{"emmcinfo","eMMC","Informations sur le stockage interne.","hard-drive"},
{"sdinfo","microSD","Informations sur la carte mémoire.","card-sim"},
{"batteryinfo","Batterie","Charge, températures et alimentation.","battery-charging"},
{"bootrom","BootROM","Informations et extraction.","cpu"},
{"kfuse","KFuses","Données des clés HDCP.","key-round"},
{"lockpick","Lockpick","Outil présent sur la carte SD.","lock-keyhole"},
};
static const nova_item nova_reference_diagnostics[] = {
{"overview","Bilan HWTest","Vue d’ensemble des groupes.","clipboard-list"},
{"identity","Identité","SoC, fusibles et KFUSE.","fingerprint"},
{"power","Alimentation","Batterie, charge et températures.","battery-charging"},
{"memory","Mémoire","DRAM et horloges.","memory-stick"},
{"storage","Stockage","SD, eMMC et lecteur.","hard-drive"},
{"wireless","Sans fil","Bluetooth et Wi-Fi.","wifi"},
{"audio","Audio","Codec et horloges audio.","audio-lines"},
{"sound","Test sonore","Haut-parleurs et prise casque.","volume-2"},
{"screen","Écran","Dalle et rétroéclairage.","monitor"},
{"input","Commandes","Tactile, Joy-Con et boutons.","gamepad-2"},
{"raw","Bas niveau","GPIO, UART et démarrage.","circuit-board"},
{"ram","Test RAM isolé","Test séparé, retour à Hekate.","cpu"},
};

/* Generated from Lucide 1.8.0 SVG paths, ISC license. Do not hand edit. */
static const uint8_t nova_layout_grid_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,102,102,102,98,24,0,0,25,98,102,102,102,46,0,0,0,46,252,221,221,221,227,231,6,6,231,227,221,221,221,252,46,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,95,231,34,34,34,65,255,32,32,255,65,34,34,34,231,95,0,0,24,231,255,255,255,255,191,0,0,192,255,255,255,255,230,25,0,0,0,6,34,34,34,31,0,0,0,0,31,34,34,34,6,0,0,0,0,6,34,34,34,31,0,0,0,0,31,34,34,34,6,0,0,0,25,231,255,255,255,255,191,0,0,192,255,255,255,255,231,24,0,0,95,231,34,34,34,65,255,32,32,255,65,34,34,34,231,95,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,99,226,0,0,0,35,255,35,35,255,35,0,0,0,226,99,0,0,46,253,221,221,221,227,231,6,6,231,227,221,221,221,253,46,0,0,0,46,102,102,102,98,24,0,0,25,98,102,102,102,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layout_grid_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_layout_grid_18_pixels};
static const uint8_t nova_layout_grid_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,188,221,221,221,221,221,187,41,0,0,42,188,221,221,221,221,221,187,41,0,0,0,0,188,251,221,221,221,221,221,251,186,0,0,188,251,221,221,221,221,221,251,186,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,187,251,221,221,221,221,221,251,186,0,0,187,251,221,221,221,221,221,251,186,0,0,0,0,41,187,221,221,221,221,221,186,40,0,0,41,187,221,221,221,221,221,186,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,188,221,221,221,221,221,187,41,0,0,42,188,221,221,221,221,221,187,41,0,0,0,0,188,251,221,221,221,221,221,251,186,0,0,188,251,221,221,221,221,221,251,186,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,217,217,0,0,0,0,187,251,221,221,221,221,221,251,186,0,0,187,251,221,221,221,221,221,251,186,0,0,0,0,41,187,221,221,221,221,221,186,40,0,0,41,187,221,221,221,221,221,186,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layout_grid_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_layout_grid_24_pixels};
static const uint8_t nova_layout_grid_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,94,102,102,102,102,102,102,102,102,81,16,0,0,0,0,0,0,16,82,102,102,102,102,102,102,102,102,94,37,0,0,0,0,0,0,0,0,0,114,251,255,255,255,255,255,255,255,255,255,255,230,49,0,0,0,0,50,230,255,255,255,255,255,255,255,255,255,255,251,112,0,0,0,0,0,0,0,38,252,255,255,255,255,255,255,255,255,255,255,255,255,203,0,0,0,0,205,255,255,255,255,255,255,255,255,255,255,255,255,251,37,0,0,0,0,0,0,95,255,255,170,102,102,102,102,102,102,102,102,221,255,253,12,0,0,12,254,255,220,102,102,102,102,102,102,102,102,170,255,255,95,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,81,255,255,219,187,187,187,187,187,187,187,187,243,255,246,5,0,0,5,247,255,242,187,187,187,187,187,187,187,187,220,255,255,80,0,0,0,0,0,0,16,230,255,255,255,255,255,255,255,255,255,255,255,255,160,0,0,0,0,161,255,255,255,255,255,255,255,255,255,255,255,255,229,15,0,0,0,0,0,0,0,49,203,253,255,255,255,255,255,255,255,255,246,160,13,0,0,0,0,14,161,246,255,255,255,255,255,255,255,255,253,203,48,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,4,0,0,0,0,0,0,0,0,4,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,4,0,0,0,0,0,0,0,0,5,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,50,204,254,255,255,255,255,255,255,255,255,247,162,14,0,0,0,0,14,162,247,255,255,255,255,255,255,255,255,254,204,49,0,0,0,0,0,0,0,17,231,255,255,255,255,255,255,255,255,255,255,255,255,161,0,0,0,0,163,255,255,255,255,255,255,255,255,255,255,255,255,230,16,0,0,0,0,0,0,81,255,255,219,187,187,187,187,187,187,187,187,242,255,246,5,0,0,5,247,255,242,187,187,187,187,187,187,187,187,220,255,255,80,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,95,255,255,170,102,102,102,102,102,102,102,102,221,255,253,12,0,0,12,254,255,221,102,102,102,102,102,102,102,102,171,255,255,95,0,0,0,0,0,0,37,251,255,255,255,255,255,255,255,255,255,255,255,255,202,0,0,0,0,204,255,255,255,255,255,255,255,255,255,255,255,255,251,36,0,0,0,0,0,0,0,112,251,255,255,255,255,255,255,255,255,255,255,229,48,0,0,0,0,49,230,255,255,255,255,255,255,255,255,255,255,251,111,0,0,0,0,0,0,0,0,0,37,93,102,102,102,102,102,102,102,102,80,16,0,0,0,0,0,0,16,80,102,102,102,102,102,102,102,102,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layout_grid_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_layout_grid_40_pixels};
static const uint8_t nova_wrench_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,20,17,0,0,0,0,0,0,0,0,0,0,0,0,0,3,115,226,255,255,191,5,0,0,0,0,0,0,0,0,0,0,3,175,239,120,53,197,228,1,0,0,0,0,0,0,0,0,0,0,113,241,43,3,163,239,53,0,2,5,0,0,0,0,0,0,0,0,222,122,0,127,239,53,0,53,228,190,0,0,0,0,0,0,0,21,255,51,0,179,179,3,53,239,199,255,15,0,0,0,0,0,0,21,255,52,0,50,239,180,239,163,56,255,17,0,0,0,0,0,0,3,236,112,0,0,50,178,127,3,123,223,0,0,0,0,0,0,3,163,239,48,0,0,0,0,0,43,240,112,0,0,0,0,0,3,163,239,53,0,48,113,56,56,124,241,175,3,0,0,0,0,3,163,239,53,0,53,239,236,255,255,220,111,3,0,0,0,0,3,163,239,53,0,53,239,163,3,15,16,0,0,0,0,0,0,3,161,240,55,0,50,237,168,4,0,0,0,0,0,0,0,0,0,147,239,53,0,53,239,163,3,0,0,0,0,0,0,0,0,0,16,254,75,0,53,239,163,3,0,0,0,0,0,0,0,0,0,0,10,248,119,76,239,163,3,0,0,0,0,0,0,0,0,0,0,0,0,100,248,254,146,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wrench_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_wrench_18_pixels};
static const uint8_t nova_wrench_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,84,163,204,198,164,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,214,255,255,233,235,255,193,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,236,251,137,26,5,173,255,145,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,204,251,76,0,5,174,255,175,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,255,138,0,2,173,255,174,5,0,5,145,192,45,0,0,0,0,0,0,0,0,0,0,161,255,30,0,82,255,175,5,0,5,174,255,255,163,0,0,0,0,0,0,0,0,0,0,201,231,0,0,81,255,175,5,5,174,255,174,237,197,0,0,0,0,0,0,0,0,0,0,202,231,0,0,2,174,255,175,175,255,174,5,234,201,0,0,0,0,0,0,0,0,0,0,160,255,32,0,0,5,173,255,255,173,5,28,255,161,0,0,0,0,0,0,0,0,0,5,182,255,81,0,0,0,2,81,80,2,0,138,255,82,0,0,0,0,0,0,0,0,5,174,255,174,5,0,0,0,0,0,0,0,76,251,212,2,0,0,0,0,0,0,0,5,174,255,174,5,0,5,78,33,0,0,31,139,251,236,39,0,0,0,0,0,0,0,5,174,255,174,5,0,5,174,255,255,239,229,255,255,204,46,0,0,0,0,0,0,0,5,174,255,174,5,0,5,174,255,182,159,204,204,159,84,3,0,0,0,0,0,0,0,5,174,255,180,7,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,3,174,255,174,5,0,5,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,115,255,174,5,0,5,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,198,237,8,0,5,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,186,246,32,9,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,255,246,238,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,186,197,113,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wrench_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_wrench_24_pixels};
static const uint8_t nova_wrench_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,72,160,220,255,255,255,235,179,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,212,255,255,255,255,255,255,255,255,245,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,139,254,255,255,255,251,213,191,206,253,255,255,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,156,255,255,255,210,93,13,0,0,128,255,255,251,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,117,255,255,253,124,3,0,0,0,128,255,255,255,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,250,255,255,100,0,0,0,0,128,255,255,255,129,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,148,0,0,0,0,128,255,255,255,129,0,0,0,0,0,44,85,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,251,255,237,15,0,0,0,118,255,255,255,129,0,0,0,0,0,122,252,255,245,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,98,255,255,145,0,0,0,34,250,255,255,129,0,0,0,0,0,128,255,255,255,255,179,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,255,255,74,0,0,0,88,255,255,160,0,0,0,0,0,128,255,255,255,253,255,233,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,255,255,34,0,0,0,58,255,255,241,57,0,0,0,128,255,255,255,128,206,255,255,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,255,255,23,0,0,0,1,183,255,255,241,57,0,128,255,255,255,128,0,196,255,255,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,174,255,255,43,0,0,0,0,14,198,255,255,241,161,255,255,255,128,0,0,216,255,252,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,134,255,255,94,0,0,0,0,0,14,198,255,255,255,255,255,128,0,0,14,252,255,219,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,74,255,255,179,0,0,0,0,0,0,14,182,255,255,249,117,0,0,0,96,255,255,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,129,255,255,235,0,0,0,0,0,0,0,0,56,87,32,0,0,0,4,214,255,255,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,129,255,255,208,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,102,253,255,253,66,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,18,152,255,255,255,134,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,119,234,179,97,45,27,36,76,149,239,255,255,255,154,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,128,255,255,255,255,255,255,255,255,255,255,255,249,115,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,128,255,255,255,255,255,255,255,255,255,255,250,168,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,128,255,255,255,129,72,131,170,187,177,150,95,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,122,255,255,255,128,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,252,255,255,127,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,255,255,133,0,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,243,255,226,3,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,250,255,210,0,0,0,0,128,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,209,255,255,87,0,3,134,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,99,255,255,255,211,227,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,169,255,255,255,255,255,252,122,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,99,208,250,242,190,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wrench_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_wrench_40_pixels};
static const uint8_t nova_info_18_pixels[] = {0,0,0,0,0,0,0,1,23,23,1,0,0,0,0,0,0,0,0,0,0,0,1,81,181,247,255,255,247,181,81,1,0,0,0,0,0,0,0,22,191,250,161,84,46,46,84,162,250,191,22,0,0,0,0,0,22,221,206,36,0,0,0,0,0,0,38,207,220,21,0,0,0,1,192,206,12,0,0,0,0,0,0,0,0,12,207,189,0,0,0,82,250,37,0,0,0,0,77,79,0,0,0,0,38,250,79,0,0,181,161,0,0,0,0,0,77,79,0,0,0,0,0,163,179,0,2,247,83,0,0,0,0,0,0,0,0,0,0,0,0,85,246,1,23,255,46,0,0,0,0,0,77,77,0,0,0,0,0,48,255,22,22,255,47,0,0,0,0,0,162,162,0,0,0,0,0,49,255,21,2,247,83,0,0,0,0,0,162,162,0,0,0,0,0,85,246,1,0,180,162,0,0,0,0,0,162,162,0,0,0,0,0,164,178,0,0,80,250,37,0,0,0,0,77,77,0,0,0,0,39,250,78,0,0,1,190,207,12,0,0,0,0,0,0,0,0,12,208,188,0,0,0,0,22,220,207,38,0,0,0,0,0,0,39,208,220,21,0,0,0,0,0,21,189,250,163,85,47,47,86,164,250,188,21,0,0,0,0,0,0,0,0,79,179,247,255,255,247,178,78,0,0,0,0,0,0,0,0,0,0,0,0,0,23,23,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_info_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_info_18_pixels};
static const uint8_t nova_info_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,58,137,179,205,205,179,136,57,2,0,0,0,0,0,0,0,0,0,0,0,0,0,79,214,255,255,252,230,230,252,255,255,213,78,0,0,0,0,0,0,0,0,0,0,8,164,255,246,144,59,2,0,0,3,60,145,246,255,163,8,0,0,0,0,0,0,0,8,196,255,182,28,0,0,0,0,0,0,0,0,28,183,255,195,8,0,0,0,0,0,0,164,255,151,3,0,0,0,0,0,0,0,0,0,0,3,152,255,162,0,0,0,0,0,79,255,182,3,0,0,0,0,0,0,0,0,0,0,0,0,3,184,255,76,0,0,0,2,214,246,28,0,0,0,0,0,0,139,141,0,0,0,0,0,0,29,247,212,2,0,0,58,255,144,0,0,0,0,0,0,0,139,141,0,0,0,0,0,0,0,146,255,56,0,0,136,255,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,255,134,0,0,179,252,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,252,178,0,0,205,229,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,230,204,0,0,204,230,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,231,203,0,0,179,252,3,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,4,252,177,0,0,135,255,60,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,62,255,133,0,0,57,255,145,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,147,255,55,0,0,2,213,246,28,0,0,0,0,0,0,138,138,0,0,0,0,0,0,30,247,212,2,0,0,0,78,255,183,3,0,0,0,0,0,0,0,0,0,0,0,0,3,185,255,76,0,0,0,0,0,163,255,152,3,0,0,0,0,0,0,0,0,0,0,3,153,255,161,0,0,0,0,0,0,8,195,255,184,29,0,0,0,0,0,0,0,0,30,185,255,194,8,0,0,0,0,0,0,0,8,162,255,247,146,61,3,0,0,4,62,146,247,255,161,8,0,0,0,0,0,0,0,0,0,0,76,212,255,255,253,230,230,253,255,255,212,75,0,0,0,0,0,0,0,0,0,0,0,0,0,2,55,134,178,204,204,178,134,55,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_info_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_info_24_pixels};
static const uint8_t nova_info_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,17,17,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,58,129,189,229,255,255,255,255,228,189,128,57,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,115,221,255,255,255,255,255,255,255,255,255,255,255,255,220,113,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,96,233,255,255,255,255,255,245,210,198,198,210,246,255,255,255,255,255,232,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,181,255,255,255,255,199,109,41,1,0,0,0,0,1,42,110,200,255,255,255,255,179,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,222,255,255,255,184,54,0,0,0,0,0,0,0,0,0,0,0,0,55,186,255,255,255,220,35,0,0,0,0,0,0,0,0,0,0,0,0,0,37,232,255,255,239,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,240,255,255,230,35,0,0,0,0,0,0,0,0,0,0,0,20,222,255,255,219,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,220,255,255,220,18,0,0,0,0,0,0,0,0,0,1,181,255,255,219,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,220,255,255,178,1,0,0,0,0,0,0,0,0,97,255,255,239,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,241,255,255,92,0,0,0,0,0,0,0,12,233,255,255,87,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,91,255,255,231,10,0,0,0,0,0,0,114,255,255,184,0,0,0,0,0,0,0,0,0,0,0,37,234,236,40,0,0,0,0,0,0,0,0,0,0,0,188,255,255,112,0,0,0,0,0,1,220,255,255,57,0,0,0,0,0,0,0,0,0,0,0,92,255,255,96,0,0,0,0,0,0,0,0,0,0,0,55,255,255,219,1,0,0,0,0,58,255,255,199,0,0,0,0,0,0,0,0,0,0,0,0,10,154,156,12,0,0,0,0,0,0,0,0,0,0,0,0,201,255,255,55,0,0,0,0,129,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,255,255,127,0,0,0,0,189,255,255,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,255,255,187,0,0,0,0,230,255,245,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,246,255,227,0,0,0,4,254,255,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,214,255,254,3,0,0,16,255,255,197,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,198,255,255,15,0,0,16,255,255,197,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,198,255,255,15,0,0,4,254,255,212,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,214,255,254,3,0,0,0,229,255,246,2,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,2,246,255,226,0,0,0,0,189,255,255,42,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,44,255,255,186,0,0,0,0,128,255,255,110,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,113,255,255,126,0,0,0,0,57,255,255,200,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,202,255,255,54,0,0,0,0,0,217,255,255,58,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,56,255,255,219,1,0,0,0,0,0,113,255,255,186,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,190,255,255,111,0,0,0,0,0,0,11,232,255,255,90,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,94,255,255,230,10,0,0,0,0,0,0,0,95,255,255,240,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,242,255,255,90,0,0,0,0,0,0,0,0,1,179,255,255,220,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,222,255,255,176,1,0,0,0,0,0,0,0,0,0,18,220,255,255,221,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,222,255,255,218,17,0,0,0,0,0,0,0,0,0,0,0,35,230,255,255,241,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,242,255,255,230,34,0,0,0,0,0,0,0,0,0,0,0,0,0,35,220,255,255,255,188,56,0,0,0,0,0,0,0,0,0,0,0,0,58,189,255,255,255,218,34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,178,255,255,255,255,201,113,43,2,0,0,0,0,2,44,113,202,255,255,255,255,176,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,93,231,255,255,255,255,255,247,211,199,199,212,247,255,255,255,255,255,230,91,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,112,219,255,255,255,255,255,255,255,255,255,255,255,255,218,111,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,127,187,226,255,255,255,255,226,187,126,54,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_info_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_info_40_pixels};
static const uint8_t nova_activity_18_pixels[] = {0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,236,126,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,85,255,213,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,225,255,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,226,110,233,101,0,0,0,0,0,0,0,0,0,0,0,0,0,46,255,37,165,172,0,0,0,0,0,0,0,0,0,0,0,0,0,118,222,0,94,241,5,0,0,0,0,0,0,0,0,0,0,0,0,191,149,0,23,254,61,0,0,0,0,0,0,0,0,8,154,170,182,251,54,0,0,205,133,0,0,0,69,156,170,154,8,8,154,170,156,67,0,0,0,133,205,0,0,56,251,181,170,154,8,0,0,0,0,0,0,0,0,61,254,23,0,149,190,0,0,0,0,0,0,0,0,0,0,0,0,4,240,93,0,220,117,0,0,0,0,0,0,0,0,0,0,0,0,0,173,164,38,255,45,0,0,0,0,0,0,0,0,0,0,0,0,0,102,235,112,228,1,0,0,0,0,0,0,0,0,0,0,0,0,0,30,255,225,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,213,255,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,124,234,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,0,0,0,0,0,0};
static const lv_img_dsc_t nova_activity_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_activity_18_pixels};
static const uint8_t nova_activity_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,147,141,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,255,255,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,255,255,125,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,197,245,245,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,251,180,181,252,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,255,109,110,255,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,37,38,255,156,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,228,222,0,0,220,230,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,255,148,0,0,149,255,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,255,76,0,0,77,255,117,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,226,255,211,4,0,0,12,249,189,0,0,0,0,27,158,214,221,219,138,0,0,141,221,221,213,155,25,0,0,0,0,189,249,12,0,0,4,214,255,225,221,219,138,0,0,0,0,0,0,0,0,0,0,0,0,117,255,77,0,0,76,255,137,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,255,148,0,0,149,255,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,228,222,0,0,220,230,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,37,38,255,156,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,255,109,110,255,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,251,180,181,252,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,197,245,245,197,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,255,255,125,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,255,255,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,149,144,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_activity_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_activity_24_pixels};
static const uint8_t nova_activity_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65,231,233,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,209,255,255,209,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,255,255,255,255,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,101,255,255,255,255,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,255,255,171,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,240,255,244,244,255,240,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,255,255,179,180,255,255,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,255,255,108,109,255,255,131,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,204,255,255,36,37,255,255,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,253,255,222,0,0,220,255,253,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,93,255,255,147,0,0,148,255,255,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,164,255,255,76,0,0,77,255,255,163,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,233,255,249,12,0,0,11,248,255,235,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,255,255,187,0,0,0,0,188,255,255,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,255,116,0,0,0,0,117,255,255,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,212,255,255,44,0,0,0,0,44,255,255,196,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,102,110,194,255,255,200,0,0,0,0,0,0,227,255,251,16,0,0,0,0,0,0,0,0,7,65,98,102,102,102,102,36,0,0,0,5,235,255,255,255,255,255,255,255,248,62,0,0,0,0,0,0,156,255,255,84,0,0,0,0,0,0,0,73,224,255,255,255,255,255,255,235,5,0,0,5,235,255,255,255,255,255,255,222,71,0,0,0,0,0,0,0,84,255,255,156,0,0,0,0,0,0,64,248,255,255,255,255,255,255,255,235,5,0,0,0,36,102,102,102,102,98,62,6,0,0,0,0,0,0,0,0,16,251,255,227,0,0,0,0,0,0,201,255,255,192,109,102,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,196,255,255,44,0,0,0,0,44,255,255,211,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,255,116,0,0,0,0,117,255,255,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,255,255,187,0,0,0,0,188,255,255,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,233,255,249,12,0,0,11,248,255,235,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,164,255,255,76,0,0,77,255,255,163,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,93,255,255,147,0,0,148,255,255,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,253,255,222,0,0,220,255,253,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,204,255,255,36,37,255,255,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,255,255,108,109,255,255,131,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,255,255,179,180,255,255,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,240,255,244,244,255,240,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,255,255,171,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,101,255,255,255,255,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,255,255,255,255,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,209,255,255,209,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65,230,230,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_activity_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_activity_40_pixels};
static const uint8_t nova_settings_2_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,123,145,74,0,0,0,0,0,0,0,0,0,0,0,0,0,20,228,217,179,250,119,0,0,0,0,0,0,0,0,0,0,0,0,123,217,7,0,99,248,105,102,102,102,102,102,65,0,0,0,0,0,146,179,0,0,64,255,224,221,221,221,221,221,171,0,0,0,0,0,74,250,98,52,190,201,0,0,0,0,0,0,0,0,0,0,0,0,0,120,246,255,201,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,202,255,246,120,0,0,0,0,0,0,0,0,0,0,0,0,0,202,189,51,97,250,74,0,0,0,0,0,171,221,221,221,221,221,224,255,63,0,0,179,146,0,0,0,0,0,65,102,102,102,102,102,105,249,98,0,8,218,122,0,0,0,0,0,0,0,0,0,0,0,0,120,250,180,218,227,19,0,0,0,0,0,0,0,0,0,0,0,0,0,74,145,122,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_settings_2_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_settings_2_18_pixels};
static const uint8_t nova_settings_2_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,124,191,191,124,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,139,3,3,140,255,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,241,3,0,0,3,249,246,221,221,221,221,221,221,221,219,138,0,0,0,0,0,0,0,191,241,3,0,0,3,249,246,221,221,221,221,221,221,221,219,138,0,0,0,0,0,0,0,124,255,140,3,3,140,255,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,123,191,191,123,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,124,191,191,124,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,139,3,3,140,255,124,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,221,246,249,3,0,0,3,241,191,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,221,246,249,3,0,0,3,241,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,124,255,140,3,3,140,255,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,123,191,191,123,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_settings_2_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_settings_2_24_pixels};
static const uint8_t nova_settings_2_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,89,158,184,170,116,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,220,255,255,255,255,255,245,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,246,255,255,255,255,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,220,255,255,205,75,29,52,157,255,255,254,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,255,255,205,9,0,0,0,0,131,255,255,173,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,158,255,255,76,0,0,0,0,0,8,243,255,252,187,187,187,187,187,187,187,187,187,187,187,187,187,187,103,0,0,0,0,0,0,0,0,0,0,0,0,184,255,255,28,0,0,0,0,0,0,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,10,0,0,0,0,0,0,0,0,0,0,0,170,255,255,52,0,0,0,0,0,0,236,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,186,0,0,0,0,0,0,0,0,0,0,0,0,116,255,255,157,0,0,0,0,0,74,255,255,203,17,17,17,17,17,17,17,17,17,17,17,17,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,26,245,255,255,131,7,0,0,73,237,255,255,99,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,255,255,255,239,198,223,255,255,255,192,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,254,255,255,255,255,255,255,192,18,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,172,242,255,251,200,99,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,102,202,253,255,244,174,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,196,255,255,255,255,255,255,254,129,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,196,255,255,255,221,198,239,255,255,255,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,255,255,235,69,0,0,7,131,255,255,245,25,0,0,0,0,0,0,0,0,0,0,0,0,0,17,17,17,17,17,17,17,17,17,17,17,17,17,17,206,255,255,70,0,0,0,0,0,158,255,255,115,0,0,0,0,0,0,0,0,0,0,0,0,186,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,235,0,0,0,0,0,0,53,255,255,169,0,0,0,0,0,0,0,0,0,0,0,10,253,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,238,0,0,0,0,0,0,29,255,255,184,0,0,0,0,0,0,0,0,0,0,0,0,103,187,187,187,187,187,187,187,187,187,187,187,187,187,187,252,255,242,7,0,0,0,0,0,79,255,255,156,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,174,255,255,131,0,0,0,0,10,208,255,255,86,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,254,255,255,157,53,29,79,208,255,255,217,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,129,255,255,255,255,255,255,255,255,245,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,245,255,255,255,255,255,217,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,114,169,184,156,86,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_settings_2_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_settings_2_40_pixels};
static const uint8_t nova_play_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,78,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,99,253,243,236,105,3,0,0,0,0,0,0,0,0,0,0,0,0,207,127,8,122,244,208,63,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,26,162,254,179,32,0,0,0,0,0,0,0,0,0,226,99,0,0,0,0,53,199,246,136,12,0,0,0,0,0,0,0,226,99,0,0,0,0,0,2,94,232,227,94,1,0,0,0,0,0,226,99,0,0,0,0,0,0,0,16,144,250,165,3,0,0,0,0,226,99,0,0,0,0,0,0,0,0,0,47,249,65,0,0,0,0,226,99,0,0,0,0,0,0,0,0,0,47,249,65,0,0,0,0,226,99,0,0,0,0,0,0,0,16,143,250,166,3,0,0,0,0,226,99,0,0,0,0,0,2,94,232,227,94,1,0,0,0,0,0,226,99,0,0,0,0,53,199,246,136,12,0,0,0,0,0,0,0,226,99,0,0,26,162,254,179,32,0,0,0,0,0,0,0,0,0,207,127,8,122,244,209,63,0,0,0,0,0,0,0,0,0,0,0,98,253,245,236,106,3,0,0,0,0,0,0,0,0,0,0,0,0,0,45,77,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_play_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_play_18_pixels};
static const uint8_t nova_play_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,85,183,184,101,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,254,244,245,255,211,66,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,183,244,23,14,139,249,255,172,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,37,179,255,249,145,13,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,68,213,255,228,94,1,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,5,111,241,255,201,54,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,25,153,253,253,162,22,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,55,201,255,238,92,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,2,96,231,254,84,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,23,244,183,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,24,245,183,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,2,97,231,254,84,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,55,201,255,237,92,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,25,153,253,253,162,22,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,5,111,241,255,193,47,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,68,213,255,228,94,1,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,37,179,255,246,136,12,0,0,0,0,0,0,0,0,0,0,0,0,0,183,244,23,14,139,249,255,172,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,85,254,245,245,255,211,66,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,85,182,182,100,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_play_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_play_24_pixels};
static const uint8_t nova_play_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,62,98,78,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,216,255,255,255,242,131,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,242,255,255,255,255,255,255,227,89,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,208,116,162,252,255,255,255,194,49,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,247,255,236,14,0,0,49,194,255,255,255,252,152,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,255,255,193,0,0,0,0,1,89,227,255,255,255,238,110,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,10,128,247,255,255,255,216,73,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,28,162,255,255,255,255,179,34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,65,210,255,255,255,249,145,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,4,102,235,255,255,255,229,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,18,145,252,255,255,255,201,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,194,255,255,255,253,162,22,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,89,227,255,255,255,238,109,3,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,132,247,255,255,255,190,15,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,174,255,255,255,168,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,71,241,255,254,35,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,133,255,255,90,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,133,255,255,90,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,72,241,255,254,35,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,174,255,255,255,168,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,132,247,255,255,255,190,15,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,89,227,255,255,255,238,110,4,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49,194,255,255,255,253,162,22,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,18,145,252,255,255,255,201,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,4,102,235,255,255,255,228,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,65,210,255,255,255,249,145,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,29,162,255,255,255,255,179,34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,10,128,247,255,255,255,216,73,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,255,255,193,0,0,0,0,1,89,227,255,255,255,238,110,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,247,255,236,14,0,0,49,194,255,255,255,252,152,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,209,118,162,252,255,255,255,194,49,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,241,255,255,255,255,255,255,227,89,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,216,255,255,255,242,131,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,60,97,77,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_play_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_play_40_pixels};
static const uint8_t nova_layers_2_18_pixels[] = {0,0,0,0,0,0,0,0,10,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,156,248,248,156,23,0,0,0,0,0,0,0,0,0,0,7,119,241,215,80,80,215,241,119,7,0,0,0,0,0,0,0,77,220,238,119,6,0,0,4,102,234,226,94,0,0,0,0,43,190,251,148,19,0,0,0,0,0,0,19,148,251,190,43,0,10,239,186,43,0,0,0,0,0,0,0,0,0,0,43,186,239,9,10,239,186,43,0,0,0,0,0,0,0,0,0,0,43,186,238,9,0,43,190,251,148,19,0,0,0,0,0,0,19,148,251,190,43,0,0,0,0,77,220,238,119,7,0,0,4,102,234,226,94,0,0,0,0,0,0,0,7,119,241,215,80,80,215,241,119,7,0,0,0,0,0,42,184,121,0,0,23,156,247,247,156,23,0,0,121,184,42,0,10,239,186,38,0,0,0,0,10,9,0,0,0,0,38,187,239,9,10,239,186,43,0,0,0,0,0,0,0,0,0,0,43,186,238,9,0,43,190,251,148,19,0,0,0,0,0,0,19,148,251,190,43,0,0,0,0,77,220,238,119,7,0,0,4,102,234,226,94,0,0,0,0,0,0,0,7,119,241,215,80,80,215,241,119,7,0,0,0,0,0,0,0,0,0,0,23,156,247,247,156,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,9,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layers_2_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_layers_2_18_pixels};
static const uint8_t nova_layers_2_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,103,185,184,103,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,73,215,255,243,244,255,215,73,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,179,255,249,145,14,11,128,247,255,187,44,0,0,0,0,0,0,0,0,0,0,17,136,249,255,179,34,0,0,0,0,29,162,254,251,153,21,0,0,0,0,0,0,4,102,234,255,206,63,0,0,0,0,0,0,0,0,55,199,255,238,119,6,0,0,0,37,212,255,232,100,2,0,0,0,0,0,0,0,0,0,0,2,100,232,255,212,37,0,0,184,250,137,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,137,250,183,0,0,184,250,137,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,137,250,184,0,0,36,212,255,232,100,2,0,0,0,0,0,0,0,0,0,0,2,100,232,255,212,36,0,0,0,4,102,233,255,206,63,0,0,0,0,0,0,0,0,56,200,255,238,119,6,0,0,0,0,0,0,17,136,249,255,179,34,0,0,0,0,29,162,254,251,153,21,0,0,0,0,0,0,0,0,0,0,38,170,255,249,145,14,11,128,247,255,187,44,0,0,0,0,0,0,0,0,4,97,80,0,0,0,73,215,255,243,244,255,215,73,0,0,0,80,97,4,0,0,0,37,212,255,191,0,0,0,0,5,103,185,184,103,5,0,0,0,0,191,255,212,37,0,0,184,250,139,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,139,250,183,0,0,184,250,137,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,137,250,184,0,0,36,212,255,232,100,2,0,0,0,0,0,0,0,0,0,0,2,100,232,255,212,36,0,0,0,4,102,233,255,206,63,0,0,0,0,0,0,0,0,56,200,255,238,119,6,0,0,0,0,0,0,17,136,249,255,179,34,0,0,0,0,29,162,254,251,153,21,0,0,0,0,0,0,0,0,0,0,38,170,255,249,145,14,11,128,247,255,187,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,73,215,255,243,244,255,215,73,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,103,185,184,103,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layers_2_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_layers_2_24_pixels};
static const uint8_t nova_layers_2_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,97,208,255,255,208,97,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,206,255,255,255,255,255,255,206,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,171,255,255,255,255,213,214,255,255,255,255,171,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,128,247,255,255,255,218,85,0,0,68,212,255,255,255,249,145,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,94,230,255,255,255,243,128,7,0,0,0,0,5,111,239,255,255,255,235,111,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,198,255,255,255,253,162,23,0,0,0,0,0,0,0,0,18,145,251,255,255,255,205,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,162,254,255,255,255,190,47,0,0,0,0,0,0,0,0,0,0,0,0,40,182,255,255,255,255,179,33,0,0,0,0,0,0,0,0,0,0,10,128,246,255,255,255,219,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,214,255,255,255,249,145,14,0,0,0,0,0,0,0,84,228,255,255,255,241,118,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,118,241,255,255,255,228,84,0,0,0,0,0,98,253,255,255,253,155,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,155,253,255,255,253,97,0,0,0,0,222,255,255,192,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,184,255,255,223,0,0,0,9,252,255,228,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,229,255,252,9,0,0,0,199,255,255,246,131,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,131,246,255,255,198,0,0,0,0,40,229,255,255,255,228,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,94,228,255,255,255,229,39,0,0,0,0,0,19,145,252,255,255,255,204,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,197,255,255,255,253,162,24,0,0,0,0,0,0,0,0,42,184,255,255,255,255,179,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27,162,254,255,255,255,192,48,0,0,0,0,0,0,0,0,0,0,0,0,77,221,255,255,255,246,136,13,0,0,0,0,0,0,0,0,0,0,10,119,243,255,255,255,226,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,111,241,255,255,255,229,102,2,0,0,0,0,0,0,1,85,224,255,255,255,244,128,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,145,252,255,255,255,203,59,0,0,0,0,52,196,255,255,255,254,162,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,194,255,255,255,254,163,44,45,163,254,255,255,255,194,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,143,72,0,0,0,0,0,1,86,223,255,255,255,255,255,255,255,255,223,86,1,0,0,0,0,0,72,143,44,0,0,0,0,0,0,0,0,23,152,252,255,247,9,0,0,0,0,0,0,6,111,241,255,255,255,255,245,128,9,0,0,0,0,0,0,9,247,255,252,152,23,0,0,0,0,0,41,230,255,255,255,212,2,0,0,0,0,0,0,0,0,23,122,177,178,121,23,0,0,0,0,0,0,0,0,2,212,255,255,255,229,40,0,0,0,0,199,255,255,247,135,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,135,247,255,255,197,0,0,0,9,252,255,228,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,229,255,252,9,0,0,0,222,255,255,200,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49,192,255,255,223,0,0,0,0,97,253,255,255,253,155,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,155,253,255,255,253,96,0,0,0,0,0,84,228,255,255,255,241,118,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,118,241,255,255,255,228,84,0,0,0,0,0,0,0,10,119,243,255,255,255,225,94,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,220,255,255,255,246,136,13,0,0,0,0,0,0,0,0,0,0,28,162,254,255,255,255,190,47,0,0,0,0,0,0,0,0,0,0,0,0,41,182,255,255,255,255,179,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,198,255,255,255,253,162,23,0,0,0,0,0,0,0,0,18,145,252,255,255,255,205,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,85,225,255,255,255,243,128,7,0,0,0,0,5,111,239,255,255,255,230,102,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,128,247,255,255,255,219,85,0,0,68,213,255,255,255,249,145,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,171,255,255,255,255,214,214,255,255,255,255,171,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,206,255,255,255,255,255,255,206,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,97,207,255,255,207,97,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_layers_2_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_layers_2_40_pixels};
static const uint8_t nova_folder_open_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,73,102,102,102,62,1,0,0,0,0,0,0,0,0,0,0,0,170,248,221,221,221,250,154,0,0,0,0,0,0,0,0,0,0,21,255,61,0,0,0,57,251,114,34,34,34,34,29,0,0,0,0,35,255,35,0,0,0,0,98,246,255,255,255,255,255,212,24,0,0,35,255,35,0,0,0,0,0,8,34,34,34,34,42,212,135,0,0,35,255,35,0,0,0,11,34,34,34,34,34,34,34,175,162,0,0,35,255,35,0,0,114,247,255,255,255,255,255,255,255,255,250,110,0,35,255,35,0,48,254,99,34,34,34,34,34,34,34,34,110,247,12,35,255,35,0,179,187,0,0,0,0,0,0,0,0,0,62,254,13,35,255,35,16,250,56,0,0,0,0,0,0,0,0,0,126,210,0,35,255,35,0,16,0,0,0,0,0,0,0,0,0,0,191,144,0,35,255,35,0,0,0,0,0,0,0,0,0,0,0,8,247,79,0,29,255,43,0,0,0,0,0,0,0,0,0,0,0,69,252,16,0,1,211,212,170,170,170,170,170,170,170,170,170,170,170,226,174,0,0,0,23,134,170,170,170,170,170,170,170,170,170,170,167,123,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_open_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_folder_open_18_pixels};
static const uint8_t nova_folder_open_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,220,179,73,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,255,242,221,221,221,221,246,252,82,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,242,21,0,0,0,0,33,237,236,31,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,84,252,246,221,221,221,221,221,221,192,93,0,0,0,0,0,217,217,0,0,0,0,0,0,0,75,180,221,221,221,221,221,221,242,255,92,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,47,163,218,221,221,221,221,221,221,221,221,221,250,241,88,0,0,0,217,217,0,0,0,33,234,253,223,221,221,221,221,221,221,221,221,221,221,243,255,87,0,0,217,217,0,0,0,169,255,75,0,0,0,0,0,0,0,0,0,0,0,22,243,190,0,0,217,217,0,0,47,253,187,0,0,0,0,0,0,0,0,0,0,0,0,2,239,194,0,0,217,217,0,0,173,255,55,0,0,0,0,0,0,0,0,0,0,0,0,49,255,143,0,0,217,217,0,0,138,143,0,0,0,0,0,0,0,0,0,0,0,0,0,115,255,78,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,181,252,16,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,241,201,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,255,135,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,144,255,69,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,227,255,208,2,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,213,154,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_open_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_folder_open_24_pixels};
static const uint8_t nova_folder_open_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,102,102,95,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,208,255,255,255,255,255,255,255,255,255,255,200,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,240,255,255,255,255,255,255,255,255,255,255,255,255,225,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,169,255,255,209,116,102,102,102,102,102,102,119,222,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,30,240,255,255,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,96,255,255,248,143,102,102,102,102,102,102,102,102,102,102,81,29,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,167,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,154,13,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,8,151,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,180,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,28,86,102,102,102,102,102,102,102,102,102,102,135,242,255,255,82,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,255,255,148,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,255,255,188,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,71,157,186,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,193,255,255,221,53,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,6,166,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,127,0,0,0,0,22,255,255,191,0,0,0,0,0,0,132,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,87,0,0,0,22,255,255,191,0,0,0,0,0,31,248,255,251,99,23,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,28,135,255,255,207,0,0,0,22,255,255,191,0,0,0,0,0,156,255,255,148,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,215,255,255,8,0,0,22,255,255,191,0,0,0,0,37,251,255,245,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,210,255,254,9,0,0,22,255,255,191,0,0,0,0,165,255,255,139,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,253,255,218,0,0,0,22,255,255,191,0,0,0,42,253,255,242,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,82,255,255,153,0,0,0,22,255,255,191,0,0,0,92,255,255,129,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,148,255,255,87,0,0,0,22,255,255,191,0,0,0,10,154,154,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,213,255,254,23,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,254,255,211,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,90,255,255,145,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,155,255,255,80,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,221,255,253,18,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,255,255,204,0,0,0,0,0,5,247,255,219,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,255,255,139,0,0,0,0,0,0,198,255,255,141,32,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,20,71,231,255,255,58,0,0,0,0,0,0,70,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,190,0,0,0,0,0,0,0,0,115,249,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,206,19,0,0,0,0,0,0,0,0,0,45,130,178,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,158,93,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_open_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_folder_open_40_pixels};
static const uint8_t nova_database_18_pixels[] = {0,0,0,0,0,0,0,10,26,26,10,0,0,0,0,0,0,0,0,0,0,49,142,199,247,255,255,255,255,247,199,142,48,0,0,0,0,5,165,254,196,133,81,61,43,43,61,81,133,197,254,163,5,0,0,121,253,57,0,0,0,0,0,0,0,0,0,0,58,253,120,0,0,121,254,185,70,11,0,0,0,0,0,0,11,70,185,254,120,0,0,99,231,174,253,251,211,189,171,171,189,212,251,253,174,231,99,0,0,99,226,0,15,71,116,137,154,154,137,116,71,15,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,123,239,0,0,0,0,0,0,0,0,0,0,0,0,239,123,0,0,135,255,121,13,0,0,0,0,0,0,0,0,13,123,255,134,0,0,99,237,230,248,198,146,125,108,108,125,146,198,248,229,237,99,0,0,99,226,9,77,134,182,200,217,217,200,182,134,77,8,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,78,244,57,0,0,0,0,0,0,0,0,0,0,58,245,77,0,0,5,163,254,197,134,83,61,44,44,61,83,134,198,254,163,5,0,0,0,0,48,141,198,246,255,255,255,255,246,198,141,47,0,0,0,0,0,0,0,0,0,0,9,26,26,9,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_database_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_database_18_pixels};
static const uint8_t nova_database_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,46,104,153,174,191,209,209,191,174,153,104,46,3,0,0,0,0,0,0,0,0,24,147,238,255,255,255,255,242,225,226,242,255,255,255,255,239,146,23,0,0,0,0,0,48,238,255,215,141,82,29,4,0,0,0,0,4,29,83,141,216,255,237,47,0,0,0,0,221,252,88,0,0,0,0,0,0,0,0,0,0,0,0,0,1,89,252,220,0,0,0,0,244,254,89,1,0,0,0,0,0,0,0,0,0,0,0,0,1,91,254,244,0,0,0,0,224,252,255,216,141,83,29,5,0,0,0,0,5,30,84,142,216,255,252,224,0,0,0,0,217,220,145,238,255,255,255,255,243,226,226,243,255,255,255,255,238,145,220,217,0,0,0,0,217,217,0,3,45,103,152,173,191,209,209,191,173,152,104,46,3,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,238,238,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,238,238,0,0,0,0,244,254,89,1,0,0,0,0,0,0,0,0,0,0,0,0,1,91,254,244,0,0,0,0,224,252,255,216,142,84,29,5,0,0,0,0,5,30,83,141,216,255,252,224,0,0,0,0,217,220,146,239,255,255,255,255,243,226,226,243,255,255,255,255,238,145,220,217,0,0,0,0,217,217,0,3,46,104,152,173,191,209,209,191,173,152,103,45,3,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,179,249,89,1,0,0,0,0,0,0,0,0,0,0,0,0,1,91,249,179,0,0,0,0,48,237,255,216,142,84,29,5,0,0,0,0,5,30,83,141,216,255,236,46,0,0,0,0,0,23,146,239,255,255,255,255,243,226,226,243,255,255,255,255,238,145,23,0,0,0,0,0,0,0,0,3,46,104,152,173,191,209,209,191,173,152,103,45,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_database_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_database_24_pixels};
static const uint8_t nova_database_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,60,107,153,189,214,242,255,255,255,255,255,255,242,214,189,153,106,59,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,95,180,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,244,180,94,9,0,0,0,0,0,0,0,0,0,0,0,0,5,120,238,255,255,255,255,255,255,255,253,230,211,204,195,195,204,211,230,253,255,255,255,255,255,255,255,237,119,5,0,0,0,0,0,0,0,0,0,19,207,255,255,255,255,239,173,117,70,28,4,0,0,0,0,0,0,0,0,4,28,70,118,173,240,255,255,255,255,206,19,0,0,0,0,0,0,0,0,199,255,255,253,172,70,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,71,174,254,255,255,199,2,0,0,0,0,0,0,75,255,255,234,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,234,255,255,95,0,0,0,0,0,0,155,255,255,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,189,255,255,160,0,0,0,0,0,0,119,255,255,254,144,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,146,254,255,255,130,0,0,0,0,0,0,107,255,255,255,255,245,158,76,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,77,159,245,255,255,255,255,107,0,0,0,0,0,0,107,255,255,251,255,255,255,255,250,205,158,116,90,62,45,34,27,27,34,46,62,90,116,159,205,250,255,255,255,255,251,255,255,107,0,0,0,0,0,0,107,255,255,130,159,246,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,245,158,129,255,255,107,0,0,0,0,0,0,107,255,255,107,0,16,92,162,227,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,226,162,91,16,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,19,66,101,127,154,170,177,187,187,177,170,153,127,100,65,18,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,148,255,255,148,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,148,255,255,148,0,0,0,0,0,0,147,255,255,216,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,216,255,255,157,0,0,0,0,0,0,111,255,255,255,220,91,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,92,221,255,255,255,116,0,0,0,0,0,0,107,255,255,255,255,255,236,161,91,34,1,0,0,0,0,0,0,0,0,0,0,0,0,1,35,92,161,236,255,255,255,255,255,107,0,0,0,0,0,0,107,255,255,223,255,255,255,255,255,255,242,201,174,146,130,119,111,111,119,130,146,175,201,242,255,255,255,255,255,255,222,255,255,107,0,0,0,0,0,0,107,255,255,108,74,177,250,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,250,177,73,108,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,12,78,142,189,236,255,255,255,255,255,255,255,255,255,255,255,255,236,188,141,77,12,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,16,43,69,85,93,102,102,93,85,69,42,16,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,103,255,255,112,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,255,255,104,0,0,0,0,0,0,45,255,255,231,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,232,255,255,67,0,0,0,0,0,0,0,197,255,255,254,175,73,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,73,176,254,255,255,196,2,0,0,0,0,0,0,0,19,205,255,255,255,255,241,176,119,72,30,5,0,0,0,0,0,0,0,0,5,31,73,120,177,242,255,255,255,255,204,18,0,0,0,0,0,0,0,0,0,5,118,236,255,255,255,255,255,255,255,254,231,214,204,196,196,204,214,231,254,255,255,255,255,255,255,255,236,116,4,0,0,0,0,0,0,0,0,0,0,0,0,8,93,178,243,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,242,177,92,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,57,104,152,186,213,240,255,255,255,255,255,255,239,213,186,151,104,57,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,17,17,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_database_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_database_40_pixels};
static const uint8_t nova_hard_drive_download_18_pixels[] = {0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,151,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,162,162,0,8,8,0,0,0,0,0,0,0,0,0,0,140,209,20,162,162,20,208,140,0,0,0,0,0,0,0,0,0,0,20,209,209,169,169,209,209,20,0,0,0,0,0,0,0,0,0,0,0,20,209,238,238,209,20,0,0,0,0,0,0,0,0,0,0,0,0,0,20,236,236,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,16,0,0,0,0,0,0,0,0,0,0,12,34,34,34,34,34,34,34,34,34,34,34,34,12,0,0,0,112,251,255,255,255,255,255,255,255,255,255,255,255,255,251,112,0,12,251,109,34,34,34,34,34,34,34,34,34,34,34,34,110,251,12,35,255,35,0,16,0,0,16,0,0,0,0,0,0,0,35,255,35,35,255,35,16,247,18,16,247,18,0,0,0,0,0,0,35,255,35,35,255,35,0,16,0,0,16,0,0,0,0,0,0,0,35,255,35,12,251,109,34,34,34,34,34,34,34,34,34,34,34,34,111,250,12,0,111,251,255,255,255,255,255,255,255,255,255,255,255,255,251,110,0,0,0,12,34,34,34,34,34,34,34,34,34,34,34,34,12,0,0};
static const lv_img_dsc_t nova_hard_drive_download_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_hard_drive_download_18_pixels};
static const uint8_t nova_hard_drive_download_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,217,217,0,5,156,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,217,217,5,174,255,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,218,218,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,243,243,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,209,209,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_download_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_hard_drive_download_24_pixels};
static const uint8_t nova_hard_drive_download_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,93,10,0,0,0,107,255,255,107,0,0,0,10,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,198,14,0,0,107,255,255,107,0,0,14,198,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,198,14,0,107,255,255,107,0,14,198,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,238,255,255,204,17,107,255,255,107,14,199,255,255,241,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,238,255,255,204,117,255,255,115,199,255,255,241,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,238,255,255,225,255,255,222,255,255,241,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,247,255,255,247,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,253,253,86,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,5,127,218,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,217,125,5,0,0,0,0,0,5,196,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,195,5,0,0,0,0,126,255,255,250,201,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,201,250,255,255,125,0,0,0,0,217,255,250,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,251,255,216,0,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,70,71,0,0,0,0,10,93,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,70,255,255,74,0,0,0,154,255,237,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,70,255,255,74,0,0,0,154,255,237,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,70,71,0,0,0,0,10,93,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,0,217,255,251,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,251,255,216,0,0,0,0,125,255,255,251,201,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,201,251,255,255,123,0,0,0,0,5,195,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,193,4,0,0,0,0,0,5,125,216,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,215,124,4,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_download_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_hard_drive_download_40_pixels};
static const uint8_t nova_hard_drive_upload_18_pixels[] = {0,0,0,0,0,0,0,0,16,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,236,236,20,0,0,0,0,0,0,0,0,0,0,0,0,0,20,209,238,238,209,20,0,0,0,0,0,0,0,0,0,0,0,20,209,209,169,169,209,209,20,0,0,0,0,0,0,0,0,0,0,140,209,20,162,162,20,208,140,0,0,0,0,0,0,0,0,0,0,8,8,0,162,162,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,151,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,12,34,34,34,34,34,34,34,34,34,34,34,34,12,0,0,0,112,251,255,255,255,255,255,255,255,255,255,255,255,255,251,112,0,12,251,109,34,34,34,34,34,34,34,34,34,34,34,34,110,251,12,35,255,35,0,16,0,0,16,0,0,0,0,0,0,0,35,255,35,35,255,35,16,247,18,16,247,18,0,0,0,0,0,0,35,255,35,35,255,35,0,16,0,0,16,0,0,0,0,0,0,0,35,255,35,12,251,109,34,34,34,34,34,34,34,34,34,34,34,34,111,250,12,0,111,251,255,255,255,255,255,255,255,255,255,255,255,255,251,110,0,0,0,12,34,34,34,34,34,34,34,34,34,34,34,34,12,0,0};
static const lv_img_dsc_t nova_hard_drive_upload_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_hard_drive_upload_18_pixels};
static const uint8_t nova_hard_drive_upload_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,210,210,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,243,243,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,218,218,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,217,217,5,174,255,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,217,217,0,5,156,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_upload_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_hard_drive_upload_24_pixels};
static const uint8_t nova_hard_drive_upload_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,253,253,86,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,247,255,255,247,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,222,255,255,219,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,115,255,255,113,193,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,107,255,255,107,11,193,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,198,14,0,107,255,255,107,0,14,198,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,198,14,0,0,107,255,255,107,0,0,14,198,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,93,10,0,0,0,107,255,255,107,0,0,0,10,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,5,127,218,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,217,125,5,0,0,0,0,0,5,196,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,195,5,0,0,0,0,126,255,255,250,201,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,201,250,255,255,125,0,0,0,0,217,255,250,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,251,255,216,0,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,70,71,0,0,0,0,10,93,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,70,255,255,74,0,0,0,154,255,237,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,70,255,255,74,0,0,0,154,255,237,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,70,71,0,0,0,0,10,93,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,0,217,255,251,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,251,255,216,0,0,0,0,125,255,255,251,201,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,201,251,255,255,123,0,0,0,0,5,195,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,193,4,0,0,0,0,0,5,125,216,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,215,124,4,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_upload_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_hard_drive_upload_40_pixels};
static const uint8_t nova_columns_3_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,86,102,102,139,112,102,102,112,139,102,102,86,14,0,0,0,15,219,239,221,221,251,234,221,221,234,251,221,221,239,219,14,0,0,84,240,12,0,0,226,99,0,0,99,226,0,0,12,241,84,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,99,226,0,0,0,226,99,0,0,85,241,12,0,0,226,99,0,0,99,226,0,0,13,241,84,0,0,14,219,239,221,221,251,234,221,221,234,251,221,221,240,218,13,0,0,0,13,85,102,102,139,112,102,102,112,139,102,102,85,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_columns_3_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_columns_3_18_pixels};
static const uint8_t nova_columns_3_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,239,239,221,221,221,221,239,239,221,221,221,192,93,0,0,0,0,0,94,255,242,221,221,221,250,250,221,221,221,221,250,250,221,221,221,242,255,92,0,0,0,0,192,242,21,0,0,0,217,217,0,0,0,0,217,217,0,0,0,22,243,191,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,192,242,22,0,0,0,217,217,0,0,0,0,217,217,0,0,0,23,243,191,0,0,0,0,92,255,243,221,221,221,250,250,221,221,221,221,250,250,221,221,221,243,255,90,0,0,0,0,0,92,191,221,221,221,239,239,221,221,221,221,239,239,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_columns_3_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_columns_3_24_pixels};
static const uint8_t nova_columns_3_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,64,101,102,102,102,102,102,144,144,102,102,102,102,102,102,102,102,144,144,102,102,102,102,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,85,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,241,102,0,0,0,0,0,0,0,0,0,99,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,97,0,0,0,0,0,0,0,14,240,255,255,168,106,102,102,102,102,166,255,255,166,102,102,102,102,102,102,166,255,255,166,102,102,102,102,106,169,255,255,240,12,0,0,0,0,0,0,64,255,255,168,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,170,255,255,63,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,109,255,255,104,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,64,255,255,169,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,13,240,255,255,170,106,102,102,102,102,166,255,255,166,102,102,102,102,102,102,166,255,255,166,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,96,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,93,0,0,0,0,0,0,0,0,0,85,237,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,102,0,0,0,0,0,0,0,0,0,0,0,13,63,101,102,102,102,102,102,144,144,102,102,102,102,102,102,102,102,144,144,102,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_columns_3_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_columns_3_40_pixels};
static const uint8_t nova_usb_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,78,17,0,0,0,0,0,0,0,0,0,0,0,0,0,8,168,246,255,78,0,0,0,0,0,0,0,0,12,0,0,0,0,8,227,255,245,11,0,0,0,0,0,0,0,130,252,129,0,0,0,103,253,227,168,0,0,0,0,0,0,0,4,232,250,193,0,0,100,252,106,8,8,0,0,0,0,0,0,4,169,239,157,43,0,100,252,106,0,0,0,0,0,0,0,0,3,164,239,52,0,0,100,252,106,0,19,64,0,0,0,0,0,0,154,239,52,0,0,100,252,106,0,8,214,254,91,0,0,0,0,0,186,167,0,0,110,252,106,0,0,59,252,254,91,0,0,0,0,0,87,247,18,110,255,223,99,11,53,239,170,66,0,0,0,0,0,0,6,233,168,252,133,150,241,241,241,163,3,0,0,0,0,0,0,0,0,183,254,106,0,0,11,99,126,3,0,0,0,0,0,0,0,46,128,252,142,0,0,0,0,0,0,0,0,0,0,0,0,0,46,252,255,130,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,253,253,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_usb_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_usb_18_pixels};
static const uint8_t nova_usb_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,101,182,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,142,251,255,255,183,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,159,255,255,255,101,0,0,0,0,0,0,0,0,0,0,42,188,187,41,0,0,0,0,5,176,255,255,251,20,0,0,0,0,0,0,0,0,0,0,189,251,251,186,0,0,0,5,174,255,181,159,141,0,0,0,0,0,0,0,0,0,0,8,236,255,251,186,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,7,182,255,230,186,40,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,6,177,255,170,4,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,5,176,255,171,4,0,0,5,174,255,180,7,0,5,154,154,5,0,0,0,0,0,0,0,157,255,171,4,0,0,5,174,255,180,7,0,0,160,255,255,158,0,0,0,0,0,0,0,179,255,37,0,0,5,174,255,180,7,0,0,5,224,255,255,158,0,0,0,0,0,0,0,81,255,131,0,5,218,255,204,11,0,0,5,174,255,223,156,5,0,0,0,0,0,0,0,4,229,231,10,174,255,254,255,229,131,37,174,255,174,5,0,0,0,0,0,0,0,0,0,0,132,255,199,255,180,38,131,229,255,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,36,254,255,180,7,0,0,4,80,178,156,5,0,0,0,0,0,0,0,0,0,0,0,5,174,255,221,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,189,233,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,188,251,255,233,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,187,251,251,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,187,186,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_usb_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_usb_24_pixels};
static const uint8_t nova_usb_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,80,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,83,168,244,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,158,244,255,255,255,255,255,79,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,255,255,255,255,255,255,243,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,241,255,255,254,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,93,63,1,0,0,0,0,0,0,0,0,0,0,153,255,255,255,255,255,82,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,114,251,255,255,187,7,0,0,0,0,0,0,0,0,128,255,255,255,255,255,243,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,252,255,255,255,255,118,0,0,0,0,0,0,0,128,255,255,255,156,241,255,159,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,255,255,213,255,255,177,0,0,0,0,0,0,128,255,255,255,127,0,36,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,118,255,255,255,255,255,146,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,71,247,255,255,255,255,239,32,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,246,255,255,217,178,146,32,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,243,255,255,186,9,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,243,255,255,194,11,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,58,242,255,255,195,12,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,124,252,193,15,0,0,0,0,0,0,0,0,0,0,0,0,0,57,242,255,255,196,12,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,128,255,255,255,199,14,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,196,12,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,68,255,255,255,255,255,153,0,0,0,0,0,0,0,0,0,0,0,5,242,255,255,35,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,79,255,255,255,255,255,153,0,0,0,0,0,0,0,0,0,0,0,0,152,255,255,117,0,0,0,0,0,133,255,255,255,127,0,0,0,0,0,0,15,199,255,255,255,255,199,14,0,0,0,0,0,0,0,0,0,0,0,0,47,255,255,220,1,0,0,0,180,255,255,255,138,0,0,0,0,0,0,15,199,255,255,248,253,193,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,255,66,0,0,128,255,255,255,255,249,169,67,1,0,0,15,199,255,255,241,57,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,101,255,255,168,0,128,255,255,255,255,255,255,255,255,218,117,36,199,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,241,255,249,139,255,255,255,127,49,151,241,255,255,255,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,152,255,255,255,255,255,127,0,0,0,12,100,203,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49,255,255,255,255,127,0,0,0,0,0,0,0,50,152,241,234,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,132,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,179,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,119,177,202,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,188,255,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,255,255,255,201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,255,255,206,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,251,255,255,255,255,117,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,251,255,255,185,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,92,61,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_usb_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_usb_40_pixels};
static const uint8_t nova_folder_check_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,72,102,102,102,63,1,0,0,0,0,0,0,0,0,0,0,0,170,248,221,221,221,250,156,0,0,0,0,0,0,0,0,0,0,21,255,61,0,0,0,55,250,116,34,34,34,34,34,34,12,0,0,35,255,35,0,0,0,0,96,246,255,255,255,255,255,255,251,111,0,35,255,35,0,0,0,0,0,7,34,34,34,34,34,34,110,251,12,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,18,62,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,20,209,189,0,0,0,35,255,35,35,255,35,0,0,0,170,101,20,209,209,20,0,0,0,35,255,35,35,255,35,0,0,0,101,253,225,209,20,0,0,0,0,35,255,35,35,255,35,0,0,0,0,100,190,20,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,29,255,43,0,0,0,0,0,0,0,0,0,0,0,0,43,255,28,1,211,212,170,170,170,170,170,170,170,170,170,170,170,170,212,210,1,0,24,134,170,170,170,170,170,170,170,170,170,170,170,170,134,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_check_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_folder_check_18_pixels};
static const uint8_t nova_folder_check_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,180,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,93,255,242,221,221,221,221,246,252,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,242,21,0,0,0,0,31,236,237,33,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,82,251,247,221,221,221,221,221,221,221,221,192,92,0,0,0,217,217,0,0,0,0,0,0,0,73,179,220,221,221,221,221,221,221,221,242,255,92,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,242,191,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,5,156,138,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,5,174,255,157,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,138,156,5,5,174,255,174,5,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,157,255,174,174,255,174,5,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,5,156,156,5,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_check_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_folder_check_24_pixels};
static const uint8_t nova_folder_check_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,102,102,96,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,208,255,255,255,255,255,255,255,255,255,255,201,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,29,240,255,255,255,255,255,255,255,255,255,255,255,255,227,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,169,255,255,209,116,102,102,102,102,102,102,119,221,255,255,180,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,28,237,255,255,100,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,91,255,255,249,145,102,102,102,102,102,102,102,102,102,102,102,102,102,94,47,3,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,162,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,207,46,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,6,145,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,239,27,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,26,86,102,102,102,102,102,102,102,102,102,102,102,102,102,116,210,255,255,167,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,238,255,233,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,194,255,255,19,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,234,234,37,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,92,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,11,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,10,154,154,14,0,0,0,57,241,255,255,198,14,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,92,255,255,198,14,0,57,241,255,255,198,14,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,37,241,255,255,198,71,241,255,255,198,14,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,51,238,255,255,255,255,255,198,14,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,198,14,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,57,241,255,198,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,36,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,5,247,255,219,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,220,255,246,5,0,0,0,198,255,255,141,32,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,32,143,255,255,197,0,0,0,0,67,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,72,0,0,0,0,0,115,249,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,249,113,0,0,0,0,0,0,0,45,130,178,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,178,130,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_folder_check_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_folder_check_40_pixels};
static const uint8_t nova_scan_line_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,86,102,65,0,0,0,0,0,0,65,102,86,14,0,0,0,15,219,239,221,171,0,0,0,0,0,0,171,221,239,219,14,0,0,84,240,12,0,0,0,0,0,0,0,0,0,0,12,241,84,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,64,173,0,0,0,0,0,0,0,0,0,0,0,0,173,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,166,170,170,170,170,170,170,166,39,0,0,0,0,0,0,0,0,39,166,170,170,170,170,170,170,166,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,173,0,0,0,0,0,0,0,0,0,0,0,0,173,64,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,85,241,12,0,0,0,0,0,0,0,0,0,0,13,241,84,0,0,14,219,239,221,171,0,0,0,0,0,0,171,221,240,218,13,0,0,0,13,85,102,65,0,0,0,0,0,0,65,102,85,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_scan_line_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_scan_line_18_pixels};
static const uint8_t nova_scan_line_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,219,138,0,0,0,0,0,0,0,0,141,221,221,192,93,0,0,0,0,0,94,255,242,221,219,138,0,0,0,0,0,0,0,0,141,221,221,242,255,92,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,221,221,219,138,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,221,221,219,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,92,255,243,221,219,138,0,0,0,0,0,0,0,0,141,221,221,243,255,90,0,0,0,0,0,92,191,221,219,138,0,0,0,0,0,0,0,0,141,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_scan_line_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_scan_line_24_pixels};
static const uint8_t nova_scan_line_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,64,101,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,98,240,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,240,97,0,0,0,0,0,0,0,0,0,98,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,255,255,96,0,0,0,0,0,0,0,14,240,255,255,168,106,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,106,169,255,255,240,12,0,0,0,0,0,0,64,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,255,63,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,255,255,104,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,93,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,154,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,154,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,93,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,64,255,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,13,240,255,255,170,106,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,97,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,255,255,95,0,0,0,0,0,0,0,0,0,96,240,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,239,95,0,0,0,0,0,0,0,0,0,0,0,13,63,101,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_scan_line_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_scan_line_40_pixels};
static const uint8_t nova_power_18_pixels[] = {0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,151,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,54,117,0,0,0,162,162,0,0,0,120,62,0,0,0,0,0,25,231,151,0,0,0,162,162,0,0,0,139,237,31,0,0,0,0,151,212,7,0,0,0,162,162,0,0,0,4,203,163,0,0,0,8,245,90,0,0,0,0,162,162,0,0,0,0,78,250,14,0,0,61,254,15,0,0,0,0,162,162,0,0,0,0,8,250,72,0,0,86,239,0,0,0,0,0,77,77,0,0,0,0,0,228,96,0,0,74,249,2,0,0,0,0,0,0,0,0,0,0,0,241,84,0,0,31,255,51,0,0,0,0,0,0,0,0,0,0,42,255,40,0,0,0,203,156,0,0,0,0,0,0,0,0,0,0,145,213,0,0,0,0,74,252,70,0,0,0,0,0,0,0,0,60,250,85,0,0,0,0,0,144,242,83,0,0,0,0,0,0,75,239,156,0,0,0,0,0,0,2,131,253,190,99,57,57,97,184,254,141,3,0,0,0,0,0,0,0,0,45,161,235,255,255,237,165,51,0,0,0,0,0,0,0,0,0,0,0,0,0,12,14,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_power_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_power_18_pixels};
static const uint8_t nova_power_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,2,0,0,0,0,217,217,0,0,0,0,4,41,0,0,0,0,0,0,0,0,0,89,254,101,0,0,0,0,217,217,0,0,0,0,96,255,104,0,0,0,0,0,0,0,43,245,232,27,0,0,0,0,217,217,0,0,0,0,20,224,250,55,0,0,0,0,0,0,179,254,62,0,0,0,0,0,217,217,0,0,0,0,0,49,252,194,0,0,0,0,0,42,255,175,0,0,0,0,0,0,217,217,0,0,0,0,0,0,160,255,57,0,0,0,0,119,255,75,0,0,0,0,0,0,217,217,0,0,0,0,0,0,62,255,134,0,0,0,0,173,254,11,0,0,0,0,0,0,217,217,0,0,0,0,0,0,4,246,187,0,0,0,0,196,237,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,220,213,0,0,0,0,196,239,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,211,0,0,0,0,171,255,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,247,186,0,0,0,0,114,255,82,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,255,128,0,0,0,0,35,254,185,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,49,0,0,0,0,0,168,255,77,0,0,0,0,0,0,0,0,0,0,0,0,62,254,183,0,0,0,0,0,0,34,240,239,42,0,0,0,0,0,0,0,0,0,0,33,232,245,45,0,0,0,0,0,0,0,73,252,239,78,0,0,0,0,0,0,0,0,67,234,254,89,0,0,0,0,0,0,0,0,0,74,239,255,187,84,15,0,0,13,80,178,255,244,85,0,0,0,0,0,0,0,0,0,0,0,34,166,253,255,255,242,239,255,255,255,175,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,111,169,194,196,169,116,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_power_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_power_24_pixels};
static const uint8_t nova_power_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,77,8,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,10,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,227,255,159,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,154,255,240,46,0,0,0,0,0,0,0,0,0,0,0,0,0,10,212,255,255,174,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,155,255,255,228,21,0,0,0,0,0,0,0,0,0,0,0,0,151,255,255,226,28,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,16,209,255,255,178,0,0,0,0,0,0,0,0,0,0,0,42,252,255,248,48,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,31,238,255,255,66,0,0,0,0,0,0,0,0,0,0,172,255,255,129,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,103,255,255,200,1,0,0,0,0,0,0,0,0,43,254,255,236,14,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,5,219,255,255,69,0,0,0,0,0,0,0,0,124,255,255,124,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,97,255,255,150,0,0,0,0,0,0,0,0,199,255,255,40,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,18,251,255,226,1,0,0,0,0,0,0,15,252,255,221,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,194,255,255,39,0,0,0,0,0,0,43,255,255,169,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,144,255,255,69,0,0,0,0,0,0,66,255,255,145,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,119,255,255,94,0,0,0,0,0,0,86,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,98,255,255,115,0,0,0,0,0,0,72,255,255,142,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,115,255,255,98,0,0,0,0,0,0,46,255,255,167,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,140,255,255,73,0,0,0,0,0,0,19,255,255,207,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,255,255,47,0,0,0,0,0,0,0,214,255,254,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,245,255,237,3,0,0,0,0,0,0,0,137,255,255,105,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,255,255,164,0,0,0,0,0,0,0,0,59,255,255,218,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,196,255,255,88,0,0,0,0,0,0,0,0,0,198,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,76,255,255,223,6,0,0,0,0,0,0,0,0,0,64,255,255,234,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,220,255,255,93,0,0,0,0,0,0,0,0,0,0,0,181,255,255,197,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,176,255,255,208,3,0,0,0,0,0,0,0,0,0,0,0,26,234,255,255,186,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,164,255,255,245,43,0,0,0,0,0,0,0,0,0,0,0,0,0,59,246,255,255,208,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,193,255,255,252,81,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,243,255,255,248,145,23,0,0,0,0,0,0,0,0,0,0,15,131,243,255,255,250,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,222,255,255,255,244,163,89,21,0,0,0,0,17,82,157,237,255,255,255,233,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,144,248,255,255,255,255,255,246,223,218,245,255,255,255,255,255,252,158,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,155,246,255,255,255,255,255,255,255,255,255,255,249,171,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,75,150,197,221,246,249,225,197,159,84,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_power_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_power_40_pixels};
static const uint8_t nova_boxes_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,87,191,191,87,0,0,0,0,0,0,0,0,0,0,0,0,37,189,250,147,147,250,189,37,0,0,0,0,0,0,0,0,0,39,248,207,42,0,0,42,208,248,38,0,0,0,0,0,0,0,0,92,248,254,165,25,25,165,254,248,92,0,0,0,0,0,0,0,0,99,226,62,210,248,248,210,62,226,99,0,0,0,0,0,0,0,0,99,226,0,4,217,217,4,0,226,99,0,0,0,0,0,0,0,3,163,249,69,0,162,162,0,69,249,163,3,0,0,0,0,0,57,206,245,243,255,196,169,169,196,255,243,245,206,57,0,0,0,152,254,171,29,2,153,253,254,254,253,153,2,29,171,254,152,0,10,253,245,107,3,0,37,191,255,255,191,37,0,3,107,245,253,10,35,255,138,244,208,172,248,195,227,227,195,248,172,208,244,138,255,35,35,255,35,28,216,254,94,1,221,221,1,94,254,216,28,35,255,35,23,255,49,0,99,226,0,0,222,222,0,0,226,99,0,50,255,23,0,189,222,75,99,226,18,152,254,254,152,18,226,99,75,222,188,0,0,13,151,253,209,240,235,220,75,75,220,235,240,209,253,151,13,0,0,0,0,51,193,238,121,8,0,0,8,121,238,193,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_boxes_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_boxes_18_pixels};
static const uint8_t nova_boxes_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,97,97,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,125,246,255,255,246,125,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65,219,255,220,89,89,220,255,219,65,0,0,0,0,0,0,0,0,0,0,0,0,0,113,252,247,126,8,0,0,8,126,247,252,113,0,0,0,0,0,0,0,0,0,0,0,0,209,255,247,126,8,0,0,8,126,247,255,208,0,0,0,0,0,0,0,0,0,0,0,0,217,228,214,255,220,75,67,215,255,220,229,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,7,124,245,255,255,245,124,7,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,32,249,249,32,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,224,224,0,0,0,217,217,0,0,0,224,224,0,0,0,0,0,0,0,0,0,0,18,151,255,255,213,35,0,217,217,0,35,213,255,255,151,18,0,0,0,0,0,0,2,100,235,255,211,244,255,253,161,218,218,161,253,255,244,211,255,235,100,2,0,0,0,15,193,255,235,101,2,4,161,253,255,254,254,255,253,161,4,2,101,235,255,193,15,0,0,196,255,217,35,0,0,0,0,51,238,255,255,238,51,0,0,0,0,35,217,255,196,0,0,211,249,255,235,100,2,2,100,235,255,254,254,255,235,100,2,2,100,235,255,249,211,0,0,217,217,98,234,255,217,218,255,235,100,249,249,100,235,255,218,217,255,234,98,217,217,0,0,217,217,0,17,150,255,255,151,18,0,249,249,0,18,151,255,255,150,17,0,217,217,0,0,217,218,0,0,0,228,228,0,0,0,249,249,0,0,0,228,228,0,0,0,218,217,0,0,174,249,42,0,0,217,217,0,0,33,252,252,33,0,0,217,217,0,0,42,249,174,0,0,65,248,246,126,8,217,217,8,126,246,255,255,246,126,8,217,217,8,126,246,248,65,0,0,0,65,219,255,220,230,230,220,255,220,75,75,220,255,220,230,230,220,255,219,65,0,0,0,0,0,8,125,246,255,255,246,125,8,0,0,8,125,246,255,255,246,125,8,0,0,0,0,0,0,0,0,23,142,142,23,0,0,0,0,0,0,23,142,142,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_boxes_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_boxes_24_pixels};
static const uint8_t nova_boxes_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,118,118,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,203,255,255,255,255,203,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,157,254,255,255,255,255,255,255,254,157,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,106,238,255,255,255,220,104,104,220,255,255,255,238,106,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27,197,255,255,255,246,125,8,0,0,8,125,246,255,255,255,197,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,238,255,255,255,176,33,0,0,0,0,0,0,33,176,255,255,255,238,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,142,255,255,255,188,23,0,0,0,0,0,0,0,0,23,188,255,255,255,140,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,174,255,255,255,255,239,109,3,0,0,0,0,3,109,239,255,255,255,255,173,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,255,255,243,255,255,255,208,59,0,0,52,201,255,255,255,246,255,255,190,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,47,162,255,255,255,254,171,157,253,255,255,255,179,51,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,71,218,255,255,255,255,255,255,218,71,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,7,123,251,255,255,251,123,7,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,130,255,255,130,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,107,255,255,107,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,220,255,255,63,0,0,0,0,107,255,255,107,0,0,0,0,63,255,255,220,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,212,255,255,255,254,179,10,0,0,107,255,255,107,0,0,10,179,254,255,255,255,212,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,166,255,255,255,255,255,255,255,248,113,0,107,255,255,107,0,113,248,255,255,255,255,255,255,255,166,26,0,0,0,0,0,0,0,0,0,0,5,115,242,255,255,255,213,99,223,255,255,255,255,229,138,255,255,135,223,255,255,255,255,228,102,213,255,255,255,242,115,5,0,0,0,0,0,0,0,42,211,255,255,255,242,116,5,0,0,113,248,255,255,255,254,255,255,254,255,255,255,248,113,0,0,5,116,242,255,255,255,211,41,0,0,0,0,0,56,248,255,255,255,167,27,0,0,0,0,0,10,178,254,255,255,255,255,255,255,254,178,10,0,0,0,0,0,27,167,255,255,255,248,54,0,0,0,0,219,255,255,249,116,0,0,0,0,0,0,0,0,0,70,244,255,255,255,255,244,70,0,0,0,0,0,0,0,0,0,116,249,255,255,218,0,0,0,2,250,255,255,255,255,168,27,0,0,0,0,0,5,115,242,255,255,255,255,255,255,242,115,5,0,0,0,0,0,27,168,255,255,255,255,248,1,0,0,20,255,255,244,255,255,255,245,128,7,0,0,63,211,255,255,255,252,255,255,250,255,255,255,217,70,0,0,4,111,241,255,255,255,246,255,255,19,0,0,22,255,255,192,111,241,255,255,255,217,89,162,254,255,255,255,179,179,255,255,177,162,254,255,255,255,179,86,211,255,255,255,244,128,193,255,255,22,0,0,22,255,255,191,0,26,165,255,255,255,255,255,255,255,213,65,0,169,255,255,169,0,65,213,255,255,255,255,255,255,255,165,26,0,191,255,255,22,0,0,22,255,255,191,0,0,0,64,212,255,255,255,244,116,5,0,0,169,255,255,169,0,0,5,116,244,255,255,255,212,64,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,5,230,255,255,46,0,0,0,0,169,255,255,169,0,0,0,0,46,255,255,230,5,0,0,0,0,191,255,255,22,0,0,21,255,255,192,0,0,0,0,0,191,255,255,22,0,0,0,0,169,255,255,169,0,0,0,0,22,255,255,191,0,0,0,0,0,192,255,255,21,0,0,3,250,255,229,7,0,0,0,0,191,255,255,22,0,0,0,0,171,255,255,171,0,0,0,0,22,255,255,191,0,0,0,0,8,231,255,249,3,0,0,0,185,255,255,186,32,0,0,0,191,255,255,22,0,0,7,123,249,255,255,249,123,7,0,0,22,255,255,191,0,0,0,32,186,255,255,182,0,0,0,0,52,248,255,255,246,125,8,0,191,255,255,22,0,73,219,255,255,255,255,255,255,219,73,0,22,255,255,191,0,8,125,246,255,255,248,50,0,0,0,0,0,72,237,255,255,255,219,74,191,255,255,51,175,255,255,255,254,165,165,254,255,255,255,175,51,255,255,191,74,219,255,255,255,237,71,0,0,0,0,0,0,0,21,157,254,255,255,255,236,255,255,247,255,255,255,205,57,0,0,57,205,255,255,255,247,255,255,236,255,255,255,254,157,21,0,0,0,0,0,0,0,0,0,0,57,205,255,255,255,255,255,255,255,238,106,3,0,0,0,0,3,106,238,255,255,255,255,255,255,255,205,57,0,0,0,0,0,0,0,0,0,0,0,0,0,3,106,236,255,255,255,252,156,21,0,0,0,0,0,0,0,0,21,156,252,255,255,255,236,106,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,100,189,138,30,0,0,0,0,0,0,0,0,0,0,0,0,31,138,189,100,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_boxes_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_boxes_40_pixels};
static const uint8_t nova_circuit_board_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,86,102,102,102,102,102,102,102,102,139,112,86,14,0,0,0,15,219,239,221,221,221,221,221,221,221,221,251,234,239,219,14,0,0,84,240,12,0,0,0,0,0,0,0,0,226,99,12,241,84,0,0,99,226,0,0,14,83,51,0,0,0,0,226,99,0,226,99,0,0,99,226,0,15,219,242,253,106,0,0,13,241,84,0,226,99,0,0,99,226,0,83,242,12,158,249,221,221,240,218,14,0,226,99,0,0,99,226,0,51,253,127,205,207,102,102,85,13,0,0,226,99,0,0,99,226,0,0,105,210,176,28,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,29,178,211,106,0,0,226,99,0,0,99,226,0,0,14,86,102,102,208,204,126,253,50,0,226,99,0,0,99,226,0,15,219,239,221,221,249,158,13,243,82,0,226,99,0,0,99,226,0,84,240,12,0,0,106,252,243,218,13,0,226,99,0,0,99,226,0,99,226,0,0,0,0,50,83,13,0,0,226,99,0,0,85,241,12,99,226,0,0,0,0,0,0,0,0,13,241,84,0,0,14,219,239,234,251,221,221,221,221,221,221,221,221,240,218,13,0,0,0,13,85,112,139,102,102,102,102,102,102,102,102,85,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_circuit_board_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_circuit_board_18_pixels};
static const uint8_t nova_circuit_board_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,239,239,221,192,93,0,0,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,250,250,221,242,255,92,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,217,217,0,22,243,191,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,217,217,0,0,0,94,192,192,93,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,217,217,0,0,94,255,242,242,255,92,0,0,0,23,243,191,0,0,217,217,0,0,0,0,217,217,0,0,192,242,21,22,250,246,221,221,221,243,255,90,0,0,217,217,0,0,0,0,217,217,0,0,192,242,22,23,250,246,221,221,221,191,90,0,0,0,217,217,0,0,0,0,217,217,0,0,92,255,243,243,255,90,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,92,191,191,90,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,94,192,192,93,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,94,255,242,242,255,92,0,0,217,217,0,0,0,0,217,217,0,0,0,94,192,221,221,221,246,249,21,22,243,191,0,0,217,217,0,0,0,0,217,217,0,0,94,255,242,221,221,221,246,249,22,23,243,191,0,0,217,217,0,0,0,0,217,217,0,0,192,242,21,0,0,0,92,255,243,243,255,90,0,0,217,217,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,92,191,191,90,0,0,0,217,217,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,192,242,22,0,217,217,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,92,255,243,221,250,250,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,92,191,221,239,239,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_circuit_board_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_circuit_board_24_pixels};
static const uint8_t nova_circuit_board_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,64,101,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,124,158,108,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,85,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,241,102,0,0,0,0,0,0,0,0,0,99,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,97,0,0,0,0,0,0,0,14,240,255,255,168,106,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,115,255,255,217,102,106,169,255,255,240,12,0,0,0,0,0,0,64,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,170,255,255,63,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,109,255,255,104,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,15,113,166,166,113,14,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,55,225,255,255,255,255,224,54,0,0,0,0,0,0,0,25,255,255,188,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,14,225,255,255,255,255,255,255,227,16,0,0,0,0,0,0,86,255,255,147,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,113,255,255,200,50,51,201,255,255,169,102,102,102,102,102,137,243,255,255,81,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,166,255,255,50,0,0,56,255,255,255,255,255,255,255,255,255,255,255,179,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,166,255,255,51,0,0,56,255,255,255,255,255,255,255,255,255,254,152,12,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,113,255,255,201,51,52,202,255,255,169,102,102,102,102,102,80,28,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,14,224,255,255,255,255,255,255,223,13,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,54,224,255,255,255,255,223,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,14,112,165,165,111,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,113,166,166,113,14,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,225,255,255,255,255,224,54,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,14,225,255,255,255,255,255,255,227,16,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,29,81,102,102,102,102,102,170,255,255,200,50,51,201,255,255,112,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,14,155,254,255,255,255,255,255,255,255,255,255,54,0,0,52,255,255,165,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,1,182,255,255,255,255,255,255,255,255,255,255,255,55,0,0,52,255,255,165,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,84,255,255,241,135,102,102,102,102,102,170,255,255,201,51,52,202,255,255,111,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,149,255,255,84,0,0,0,0,0,0,14,224,255,255,255,255,255,255,223,13,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,188,255,255,25,0,0,0,0,0,0,0,54,224,255,255,255,255,223,53,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,14,112,165,165,111,14,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,109,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,64,255,255,169,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,13,240,255,255,170,106,102,217,255,255,115,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,96,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,93,0,0,0,0,0,0,0,0,0,85,237,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,102,0,0,0,0,0,0,0,0,0,0,0,13,63,101,102,108,158,124,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_circuit_board_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_circuit_board_40_pixels};
static const uint8_t nova_hard_drive_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,86,160,170,170,170,170,170,170,160,85,0,0,0,0,0,0,0,102,247,174,170,170,170,170,170,170,173,247,102,0,0,0,0,0,11,234,119,0,0,0,0,0,0,0,0,119,234,11,0,0,0,0,118,236,13,0,0,0,0,0,0,0,0,11,233,118,0,0,0,11,233,119,0,0,0,0,0,0,0,0,0,0,119,235,13,0,0,118,236,13,0,0,0,0,0,0,0,0,0,0,11,233,118,0,5,244,201,153,153,153,153,153,153,153,153,153,153,153,153,201,244,5,35,255,183,170,170,170,170,170,170,170,170,170,170,170,170,183,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,8,139,9,8,139,9,0,0,0,0,0,0,35,255,35,35,255,35,8,139,9,8,139,9,0,0,0,0,0,0,35,255,35,29,255,43,0,0,0,0,0,0,0,0,0,0,0,0,43,255,28,1,211,212,170,170,170,170,170,170,170,170,170,170,170,170,212,210,1,0,24,134,170,170,170,170,170,170,170,170,170,170,170,170,134,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_hard_drive_18_pixels};
static const uint8_t nova_hard_drive_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,183,219,221,221,221,221,221,221,221,221,219,182,60,0,0,0,0,0,0,0,0,0,45,249,249,221,221,221,221,221,221,221,221,221,221,249,248,44,0,0,0,0,0,0,0,0,178,255,61,0,0,0,0,0,0,0,0,0,0,61,255,178,0,0,0,0,0,0,0,50,254,179,0,0,0,0,0,0,0,0,0,0,0,0,180,255,53,0,0,0,0,0,0,179,255,54,0,0,0,0,0,0,0,0,0,0,0,0,51,254,178,0,0,0,0,0,50,254,179,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,255,53,0,0,0,0,179,255,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,254,178,0,0,0,50,254,178,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,179,255,53,0,0,205,255,228,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,228,255,205,0,0,233,251,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,251,233,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,139,141,0,0,139,141,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_hard_drive_24_pixels};
static const uint8_t nova_hard_drive_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,115,171,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,171,114,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,227,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,227,47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,223,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,222,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,255,255,208,52,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,53,209,255,255,126,0,0,0,0,0,0,0,0,0,0,0,0,0,15,238,255,253,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,253,255,238,15,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,126,0,0,0,0,0,0,0,0,0,0,0,15,238,255,254,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,253,255,240,16,0,0,0,0,0,0,0,0,0,0,127,255,255,170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,126,0,0,0,0,0,0,0,0,0,15,238,255,254,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,253,255,240,16,0,0,0,0,0,0,0,0,127,255,255,170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,126,0,0,0,0,0,0,0,15,238,255,254,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,253,255,240,16,0,0,0,0,0,0,127,255,255,170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,126,0,0,0,0,0,15,238,255,254,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,253,255,240,16,0,0,0,0,138,255,255,204,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,204,255,255,137,0,0,0,0,250,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,250,0,0,0,13,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,12,0,0,22,255,255,221,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,221,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,10,154,156,12,0,0,0,48,178,105,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,92,255,255,96,0,0,0,177,255,253,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,37,234,236,40,0,0,0,104,252,189,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,6,6,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,5,247,255,219,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,220,255,246,5,0,0,0,198,255,255,141,32,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,32,143,255,255,197,0,0,0,0,67,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,72,0,0,0,0,0,115,249,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,249,113,0,0,0,0,0,0,0,45,130,178,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,178,130,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_hard_drive_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_hard_drive_40_pixels};
static const uint8_t nova_card_sim_18_pixels[] = {0,0,0,1,29,34,34,34,34,34,31,3,0,0,0,0,0,0,0,0,24,212,255,255,255,255,255,255,255,225,53,0,0,0,0,0,0,0,136,211,42,34,34,34,34,34,38,165,239,53,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,3,161,240,55,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,3,163,239,37,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,3,199,143,0,0,0,0,162,162,0,0,14,34,34,34,34,14,0,0,162,162,0,0,0,0,162,162,0,74,246,255,255,255,255,246,73,0,162,162,0,0,0,0,162,162,0,159,175,34,34,34,34,176,159,0,162,162,0,0,0,0,162,162,0,165,174,34,41,41,34,174,165,0,162,162,0,0,0,0,162,162,0,213,255,255,255,255,255,255,213,0,162,162,0,0,0,0,162,162,0,165,174,34,174,174,34,174,165,0,162,162,0,0,0,0,162,162,0,159,176,34,174,174,34,176,159,0,162,162,0,0,0,0,162,162,0,74,246,255,255,255,255,245,74,0,162,162,0,0,0,0,162,162,0,0,14,34,41,41,34,14,0,0,162,162,0,0,0,0,136,211,43,34,34,34,34,34,34,34,34,43,212,134,0,0,0,0,24,211,255,255,255,255,255,255,255,255,255,255,211,23,0,0,0,0,0,0,29,34,34,34,34,34,34,34,34,29,0,0,0,0};
static const lv_img_dsc_t nova_card_sim_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_card_sim_18_pixels};
static const uint8_t nova_card_sim_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,201,118,4,0,0,0,0,0,0,0,0,0,0,94,255,242,221,221,221,221,221,221,221,221,237,255,174,5,0,0,0,0,0,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,7,174,255,174,5,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,5,174,255,174,4,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,117,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,7,236,199,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,42,188,221,221,221,221,221,221,187,41,0,0,217,217,0,0,0,0,0,0,217,217,0,0,188,251,221,221,221,221,221,221,251,186,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,238,250,221,221,239,239,221,221,250,238,0,0,217,217,0,0,0,0,0,0,217,217,0,0,238,250,221,221,250,250,221,221,250,238,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,187,251,221,221,250,250,221,221,251,186,0,0,217,217,0,0,0,0,0,0,217,217,0,0,41,187,221,221,239,239,221,221,186,40,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_card_sim_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_card_sim_24_pixels};
static const uint8_t nova_card_sim_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,17,17,17,17,17,17,17,17,17,17,17,17,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,71,199,247,255,255,255,255,255,255,255,255,255,255,255,255,255,255,233,163,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,117,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,235,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,250,255,255,219,187,187,187,187,187,187,187,187,187,187,187,187,187,192,240,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,255,255,139,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,199,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,180,255,255,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,199,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,199,255,255,241,53,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,199,255,255,219,10,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,203,255,255,109,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,255,255,163,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,76,166,187,187,187,187,187,187,187,187,187,187,166,77,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,105,255,255,255,255,255,255,255,255,255,255,255,255,255,255,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,234,255,255,255,255,255,255,255,255,255,255,255,255,255,255,232,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,19,255,255,199,17,17,17,17,17,17,17,17,17,17,200,255,255,19,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,195,17,17,17,17,23,23,17,17,17,17,195,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,31,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,31,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,238,187,187,187,216,255,255,216,187,187,187,238,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,12,254,255,221,102,102,102,166,255,255,166,102,102,102,221,255,253,12,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,204,255,255,255,255,255,255,255,255,255,255,255,255,255,255,202,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,49,230,255,255,255,255,255,255,255,255,255,255,255,255,229,48,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,16,80,102,102,102,102,144,144,102,102,102,102,80,16,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,180,255,255,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,255,255,180,0,0,0,0,0,0,0,0,0,0,131,255,255,141,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,143,255,255,130,0,0,0,0,0,0,0,0,0,0,45,249,255,255,220,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,220,255,255,249,44,0,0,0,0,0,0,0,0,0,0,0,115,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,113,0,0,0,0,0,0,0,0,0,0,0,0,0,69,197,246,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,246,197,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_card_sim_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_card_sim_40_pixels};
static const uint8_t nova_battery_charging_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,34,34,15,0,0,0,0,3,34,12,0,0,0,0,0,0,112,251,255,255,191,0,17,62,0,104,255,251,112,0,0,0,0,12,251,109,34,34,15,0,164,197,0,3,34,110,251,12,0,0,0,35,255,35,0,0,0,60,254,65,0,0,0,35,255,35,0,16,0,35,255,35,0,0,4,209,164,0,0,0,0,35,255,35,26,251,26,35,255,35,0,0,109,255,191,170,77,0,0,35,255,35,35,255,35,35,255,35,0,0,77,170,191,255,109,0,0,35,255,35,35,255,35,35,255,35,0,0,0,0,164,212,5,0,0,35,255,35,26,251,26,35,255,35,0,0,0,60,254,65,0,0,0,35,255,35,0,16,0,12,251,109,34,3,0,197,164,0,15,34,34,111,250,12,0,0,0,0,111,251,255,103,0,62,17,0,191,255,255,251,110,0,0,0,0,0,0,12,34,3,0,0,0,0,15,34,34,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_charging_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_battery_charging_18_pixels};
static const uint8_t nova_battery_charging_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,217,126,0,0,0,0,0,0,173,221,192,93,0,0,0,0,0,0,0,94,255,242,221,221,217,126,0,0,145,138,0,0,173,221,242,255,92,0,0,0,0,0,0,192,242,21,0,0,0,0,0,75,255,170,0,0,0,0,22,243,191,0,0,0,0,0,0,217,217,0,0,0,0,0,7,217,246,33,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,126,255,126,0,0,0,0,0,0,217,217,0,0,138,138,0,0,217,217,0,0,0,0,30,244,220,8,0,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,170,255,240,221,221,141,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,141,221,221,240,255,169,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,7,217,246,33,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,126,255,126,0,0,0,0,0,217,217,0,0,138,138,0,0,217,217,0,0,0,0,0,30,244,220,8,0,0,0,0,0,217,217,0,0,0,0,0,0,192,242,22,0,0,0,0,170,255,75,0,0,0,0,0,23,243,191,0,0,0,0,0,0,92,255,243,221,167,0,0,138,145,0,0,126,221,221,221,243,255,90,0,0,0,0,0,0,0,92,191,221,167,0,0,0,0,0,0,126,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_charging_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_battery_charging_24_pixels};
static const uint8_t nova_battery_charging_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,100,26,0,0,0,0,0,0,0,0,0,0,5,89,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,208,255,255,255,255,255,255,255,212,0,0,0,0,0,0,0,0,0,0,130,255,255,255,255,240,97,0,0,0,0,0,0,0,0,0,0,0,0,29,240,255,255,255,255,255,255,255,255,212,0,0,0,0,104,178,48,0,0,0,130,255,255,255,255,255,255,96,0,0,0,0,0,0,0,0,0,0,0,169,255,255,209,116,102,102,102,102,100,26,0,0,0,58,254,255,177,0,0,0,5,89,102,106,169,255,255,240,12,0,0,0,0,0,0,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,0,3,206,255,255,119,0,0,0,0,0,0,0,0,170,255,255,63,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,108,255,255,219,8,0,0,0,0,0,0,0,0,109,255,255,104,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,22,238,255,255,74,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,159,255,255,174,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,48,178,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,58,254,255,246,33,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,179,255,253,12,0,0,22,255,255,191,0,0,0,0,0,0,0,3,206,255,255,124,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,107,255,255,249,109,102,102,102,70,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,5,236,255,255,255,255,255,255,255,255,70,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,5,235,255,255,255,255,255,255,255,255,73,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,36,102,102,102,102,205,255,255,192,1,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,43,250,255,251,46,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,190,255,255,140,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,179,255,253,12,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,91,255,255,231,15,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,48,178,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,13,229,255,255,90,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,141,255,255,194,1,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,0,0,0,0,234,255,238,16,0,0,0,0,0,0,0,39,250,255,250,43,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,0,0,0,0,168,255,255,210,117,102,100,27,0,0,0,92,255,255,141,0,0,0,4,88,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,0,0,0,28,239,255,255,255,255,255,214,0,0,0,10,154,154,11,0,0,0,127,255,255,255,255,255,255,255,255,255,95,0,0,0,0,0,0,0,0,0,0,0,0,45,206,255,255,255,255,214,0,0,0,0,0,0,0,0,0,0,127,255,255,255,255,255,255,255,239,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,46,93,102,100,27,0,0,0,0,0,0,0,0,0,0,4,88,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_charging_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_battery_charging_40_pixels};
static const uint8_t nova_cpu_18_pixels[] = {0,0,0,0,1,15,0,0,8,8,0,0,15,1,0,0,0,0,0,0,0,0,87,215,0,0,151,151,0,0,215,87,0,0,0,0,0,0,0,85,196,245,170,170,224,224,170,170,245,196,85,0,0,0,0,0,85,249,186,209,170,170,196,196,170,170,209,186,250,84,0,0,1,89,196,183,0,0,0,0,0,0,0,0,0,0,184,196,89,1,15,212,243,204,0,19,134,170,170,170,170,134,19,0,204,243,212,15,0,0,162,162,0,133,224,170,170,170,170,224,132,0,162,162,0,0,0,0,162,162,0,162,162,0,0,0,0,162,162,0,162,162,0,0,8,154,224,190,0,162,162,0,0,0,0,162,162,0,190,224,154,8,8,154,224,190,0,162,162,0,0,0,0,162,162,0,190,224,154,8,0,0,162,162,0,162,162,0,0,0,0,162,162,0,162,162,0,0,0,0,162,162,0,133,224,170,170,170,170,225,133,0,162,162,0,0,15,212,243,204,0,19,134,170,170,170,170,134,19,0,204,243,212,15,1,89,196,183,0,0,0,0,0,0,0,0,0,0,184,196,89,1,0,0,85,250,187,209,170,170,196,196,170,170,209,187,250,83,0,0,0,0,0,84,196,245,170,170,224,224,170,170,245,196,83,0,0,0,0,0,0,0,87,215,0,0,151,151,0,0,215,87,0,0,0,0,0,0,0,0,1,15,0,0,8,8,0,0,15,1,0,0,0,0};
static const lv_img_dsc_t nova_cpu_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_cpu_18_pixels};
static const uint8_t nova_cpu_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,138,138,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,94,192,250,250,221,221,221,250,250,221,221,221,250,250,192,93,0,0,0,0,0,0,0,94,255,242,239,239,221,221,221,239,239,221,221,221,239,239,242,255,92,0,0,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,141,221,250,238,0,0,0,0,0,0,0,0,0,0,0,0,0,0,238,250,219,138,0,0,141,221,250,238,0,0,42,188,221,221,221,221,221,221,187,41,0,0,238,250,219,138,0,0,0,0,217,217,0,0,188,251,221,221,221,221,221,221,251,186,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,141,221,250,238,0,0,217,217,0,0,0,0,0,0,217,217,0,0,238,250,219,138,0,0,141,221,250,238,0,0,217,217,0,0,0,0,0,0,217,217,0,0,238,250,219,138,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,187,251,221,221,221,221,221,221,251,186,0,0,217,217,0,0,0,0,141,221,250,238,0,0,41,187,221,221,221,221,221,221,186,40,0,0,238,250,219,138,0,0,141,221,250,238,0,0,0,0,0,0,0,0,0,0,0,0,0,0,238,250,219,138,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,0,0,92,255,243,239,239,221,221,221,239,239,221,221,221,239,239,243,255,90,0,0,0,0,0,0,0,92,191,250,250,221,221,221,250,250,221,221,221,250,250,191,90,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,138,138,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_cpu_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_cpu_24_pixels};
static const uint8_t nova_cpu_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,252,186,0,0,0,0,0,37,234,234,37,0,0,0,0,0,186,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,189,255,255,19,0,0,0,0,104,255,255,104,0,0,0,0,19,255,255,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,107,255,255,107,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,113,166,238,255,255,193,187,187,187,187,216,255,255,216,187,187,187,187,193,255,255,238,166,113,14,0,0,0,0,0,0,0,0,0,0,0,0,0,55,225,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,224,54,0,0,0,0,0,0,0,0,0,0,0,14,225,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,227,16,0,0,0,0,0,0,0,0,0,0,113,255,255,200,50,17,27,17,17,17,17,17,17,17,23,23,17,17,17,17,17,17,17,27,17,51,201,255,255,112,0,0,0,0,0,0,0,0,0,0,166,255,255,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,255,255,165,0,0,0,0,0,0,0,103,187,187,238,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,238,187,187,103,0,0,0,10,253,255,255,255,255,255,31,0,0,0,0,0,5,17,17,17,17,17,17,17,17,17,17,4,0,0,0,0,0,31,255,255,255,255,255,253,10,0,0,0,186,255,255,255,255,255,22,0,0,0,14,162,247,255,255,255,255,255,255,255,255,255,255,247,162,14,0,0,0,22,255,255,255,255,255,186,0,0,0,0,0,17,17,195,255,255,22,0,0,0,163,255,255,255,255,255,255,255,255,255,255,255,255,255,255,161,0,0,0,22,255,255,195,17,17,0,0,0,0,0,0,0,0,191,255,255,22,0,0,5,247,255,242,187,187,187,187,187,187,187,187,187,187,242,255,246,5,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,36,102,102,217,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,217,102,102,36,0,0,0,5,235,255,255,255,255,255,27,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,27,255,255,255,255,255,235,5,0,0,5,235,255,255,255,255,255,27,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,27,255,255,255,255,255,235,5,0,0,0,36,102,102,217,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,217,102,102,36,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,5,247,255,242,187,187,187,187,187,187,187,187,187,187,243,255,246,5,0,0,22,255,255,191,0,0,0,0,0,0,0,0,17,17,195,255,255,22,0,0,0,161,255,255,255,255,255,255,255,255,255,255,255,255,255,255,160,0,0,0,22,255,255,195,17,17,0,0,0,0,0,186,255,255,255,255,255,22,0,0,0,14,161,246,255,255,255,255,255,255,255,255,255,255,246,160,13,0,0,0,22,255,255,255,255,255,186,0,0,0,10,253,255,255,255,255,255,31,0,0,0,0,0,4,17,17,17,17,17,17,17,17,17,17,4,0,0,0,0,0,31,255,255,255,255,255,253,10,0,0,0,103,187,187,238,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,238,187,187,103,0,0,0,0,0,0,0,166,255,255,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,255,255,165,0,0,0,0,0,0,0,0,0,0,113,255,255,201,51,17,27,17,17,17,17,17,17,17,23,23,17,17,17,17,17,17,17,27,17,52,202,255,255,111,0,0,0,0,0,0,0,0,0,0,14,224,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,223,13,0,0,0,0,0,0,0,0,0,0,0,54,224,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,223,53,0,0,0,0,0,0,0,0,0,0,0,0,0,14,112,165,238,255,255,193,187,187,187,187,216,255,255,216,187,187,187,187,193,255,255,238,165,111,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,107,255,255,107,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,189,255,255,19,0,0,0,0,104,255,255,104,0,0,0,0,19,255,255,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,252,186,0,0,0,0,0,37,234,234,37,0,0,0,0,0,186,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_cpu_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_cpu_40_pixels};
static const uint8_t nova_key_round_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,10,23,17,0,0,0,0,0,0,0,0,0,0,0,0,0,42,165,248,255,252,200,76,0,0,0,0,0,0,0,0,0,0,66,240,190,79,42,62,148,252,133,0,0,0,0,0,0,0,0,17,237,148,2,0,0,0,0,75,252,76,0,0,0,0,0,0,0,127,221,6,0,0,15,71,3,0,150,198,0,0,0,0,0,0,0,184,141,0,0,0,134,226,70,0,63,252,16,0,0,0,0,0,0,208,117,0,0,0,42,133,14,0,43,255,22,0,0,0,0,0,0,180,152,0,0,0,0,0,0,0,81,247,7,0,0,0,0,0,103,253,87,0,0,0,0,0,0,2,190,163,0,0,0,0,0,103,253,103,0,0,0,0,0,0,8,152,240,42,0,0,0,0,103,253,103,0,0,0,88,153,119,143,223,235,63,0,0,0,0,103,253,103,0,0,74,175,253,180,206,183,125,15,0,0,0,0,97,253,103,0,0,14,246,167,67,0,0,0,0,0,0,0,0,8,245,114,0,0,46,123,253,23,0,0,0,0,0,0,0,0,0,35,255,35,0,46,252,219,129,0,0,0,0,0,0,0,0,0,0,32,255,65,34,121,223,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,253,129,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,34,23,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_key_round_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_key_round_18_pixels};
static const uint8_t nova_key_round_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,149,204,221,207,154,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,166,255,255,235,207,233,255,255,179,19,0,0,0,0,0,0,0,0,0,0,0,0,14,205,255,184,53,0,0,0,45,172,255,216,19,0,0,0,0,0,0,0,0,0,0,0,161,255,137,1,0,0,0,0,0,0,122,255,177,0,0,0,0,0,0,0,0,0,0,44,254,188,1,0,0,0,0,0,0,0,0,174,255,59,0,0,0,0,0,0,0,0,0,135,255,65,0,0,0,0,75,191,75,0,0,49,255,150,0,0,0,0,0,0,0,0,0,187,247,4,0,0,0,0,191,229,191,0,0,0,234,204,0,0,0,0,0,0,0,0,0,204,230,0,0,0,0,0,74,191,74,0,0,0,213,219,0,0,0,0,0,0,0,0,0,185,249,7,0,0,0,0,0,0,0,0,0,1,238,200,0,0,0,0,0,0,0,0,5,193,255,50,0,0,0,0,0,0,0,0,0,57,255,145,0,0,0,0,0,0,0,5,174,255,174,4,0,0,0,0,0,0,0,0,1,185,254,51,0,0,0,0,0,0,5,174,255,174,5,0,0,0,0,0,0,0,0,1,141,255,165,0,0,0,0,0,0,5,174,255,174,5,0,0,0,3,46,7,0,5,69,193,255,202,13,0,0,0,0,0,5,174,255,174,5,0,0,0,8,174,255,248,236,248,255,253,156,13,0,0,0,0,0,5,174,255,174,5,0,0,42,188,237,255,192,181,203,183,131,41,0,0,0,0,0,0,4,174,255,174,5,0,0,0,188,251,200,116,4,0,0,0,0,0,0,0,0,0,0,0,118,255,174,5,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,200,235,7,0,0,42,188,221,251,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,188,251,221,186,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,187,251,221,221,221,251,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,187,221,221,221,186,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_key_round_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_key_round_24_pixels};
static const uint8_t nova_key_round_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,34,23,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,79,161,227,255,255,255,252,212,149,54,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,80,214,255,255,255,255,255,255,255,255,255,255,194,49,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,155,255,255,255,255,247,208,181,187,217,254,255,255,255,252,116,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,190,255,255,255,206,87,12,0,0,0,0,20,110,225,255,255,255,147,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,174,255,255,248,107,1,0,0,0,0,0,0,0,0,10,145,255,255,255,116,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,98,255,255,246,64,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,252,49,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,237,255,255,89,0,0,0,0,0,0,0,0,0,0,0,0,0,0,145,255,255,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,178,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,226,255,255,53,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,200,255,255,57,0,0,0,0,0,0,0,4,100,170,102,3,0,0,0,0,111,255,255,147,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,251,255,222,0,0,0,0,0,0,0,0,100,255,252,255,97,0,0,0,0,22,254,255,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,255,255,161,0,0,0,0,0,0,0,0,169,250,255,252,168,0,0,0,0,0,219,255,251,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,80,255,255,134,0,0,0,0,0,0,0,0,97,255,252,255,100,0,0,0,0,0,188,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,82,255,255,131,0,0,0,0,0,0,0,0,3,101,170,95,3,0,0,0,0,0,184,255,255,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,255,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,211,255,253,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,254,255,211,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,249,255,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,249,255,254,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,90,255,255,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,241,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,208,255,255,75,0,0,0,0,0,0,0,0,0,0,0,0,0,56,241,255,255,199,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,255,255,212,2,0,0,0,0,0,0,0,0,0,0,0,0,56,241,255,255,199,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,249,255,255,79,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,199,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,248,255,255,153,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,199,14,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,60,182,255,255,255,187,8,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,0,0,0,0,0,0,0,15,193,254,211,156,132,136,164,225,255,255,255,255,170,7,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,0,0,0,0,0,0,0,20,199,255,255,255,255,255,255,255,255,255,255,234,92,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,0,0,0,0,0,33,148,191,241,255,255,250,254,255,255,255,255,251,197,105,10,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,0,0,0,0,0,35,241,255,255,255,255,235,57,23,60,80,77,54,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,198,14,0,0,0,0,0,0,148,255,255,255,232,161,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,235,255,255,198,14,0,0,0,0,0,0,0,188,255,255,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,164,255,255,198,14,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,233,255,240,20,0,0,0,0,0,33,148,186,187,243,255,246,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,255,255,195,0,0,0,0,0,35,241,255,255,255,255,255,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,148,255,255,255,255,246,160,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,188,255,255,41,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,247,255,242,187,187,187,187,187,243,255,246,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,255,255,255,255,255,255,255,255,255,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,161,246,255,255,255,255,255,246,160,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,17,17,17,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_key_round_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_key_round_40_pixels};
static const uint8_t nova_lock_keyhole_18_pixels[] = {0,0,0,0,0,0,0,0,23,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,67,216,255,255,215,67,0,0,0,0,0,0,0,0,0,0,0,86,252,147,50,51,148,252,84,0,0,0,0,0,0,0,0,0,14,236,125,0,0,0,0,126,235,13,0,0,0,0,0,0,0,0,77,247,8,0,0,0,0,9,248,76,0,0,0,0,0,0,0,0,98,227,0,0,0,0,0,0,227,98,0,0,0,0,0,0,0,21,120,230,34,34,34,34,34,34,230,120,21,0,0,0,0,2,170,255,255,255,255,255,255,255,255,255,255,255,255,170,2,0,0,72,248,61,35,47,34,34,34,34,34,34,47,35,61,248,71,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,46,46,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,46,252,252,46,0,0,0,0,226,99,0,0,99,226,0,0,0,0,46,253,253,46,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,46,45,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,72,248,61,34,34,34,34,34,34,34,34,34,34,62,248,71,0,0,2,169,255,255,255,255,255,255,255,255,255,255,255,255,168,2,0,0,0,0,20,34,34,34,34,34,34,34,34,34,34,20,0,0,0};
static const lv_img_dsc_t nova_lock_keyhole_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_lock_keyhole_18_pixels};
static const uint8_t nova_lock_keyhole_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,161,205,205,161,67,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,168,255,255,230,230,255,255,166,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,168,255,180,38,0,0,39,181,255,166,0,0,0,0,0,0,0,0,0,0,0,0,0,68,255,180,2,0,0,0,0,2,182,255,66,0,0,0,0,0,0,0,0,0,0,0,0,162,255,39,0,0,0,0,0,0,41,255,160,0,0,0,0,0,0,0,0,0,0,0,0,205,230,0,0,0,0,0,0,0,0,231,204,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,94,192,221,250,250,221,221,221,221,221,221,221,221,250,250,221,192,93,0,0,0,0,0,94,255,242,221,239,239,221,221,221,221,221,221,221,221,239,239,221,242,255,92,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,42,188,187,41,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,188,251,251,186,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,187,251,251,186,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,41,187,186,40,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_lock_keyhole_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_lock_keyhole_24_pixels};
static const uint8_t nova_lock_keyhole_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,104,186,240,255,255,239,185,103,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,230,255,255,255,255,255,255,255,255,229,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,142,255,255,255,255,236,204,203,236,255,255,255,255,141,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,185,61,2,0,0,2,62,187,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,254,255,254,114,0,0,0,0,0,0,0,0,115,254,255,254,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,200,255,255,130,0,0,0,0,0,0,0,0,0,0,132,255,255,199,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,255,255,221,6,0,0,0,0,0,0,0,0,0,0,5,219,255,255,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,123,255,255,117,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,122,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,255,255,52,0,0,0,0,0,0,0,0,0,0,0,0,54,255,255,161,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,186,255,255,27,0,0,0,0,0,0,0,0,0,0,0,0,28,255,255,185,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,84,149,186,187,238,255,255,193,187,187,187,187,187,187,187,187,187,187,187,187,193,255,255,238,187,186,148,83,0,0,0,0,0,0,0,0,0,0,14,182,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,181,13,0,0,0,0,0,0,0,0,156,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,153,0,0,0,0,0,0,0,29,254,255,241,83,21,17,17,27,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,27,17,17,21,84,242,255,254,28,0,0,0,0,0,0,81,255,255,135,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,136,255,255,80,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,16,82,81,16,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,50,230,255,255,230,49,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,205,255,255,255,255,203,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,9,254,255,229,230,255,253,9,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,233,255,255,255,255,232,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,103,255,255,255,255,102,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,76,165,164,75,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,95,255,255,118,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,118,255,255,95,0,0,0,0,0,0,47,255,255,210,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,212,255,255,46,0,0,0,0,0,0,3,206,255,255,238,191,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,191,238,255,255,205,3,0,0,0,0,0,0,0,45,239,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,238,44,0,0,0,0,0,0,0,0,0,27,168,233,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,233,167,27,0,0,0,0,0,0,0,0,0,0,0,0,0,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_lock_keyhole_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_lock_keyhole_40_pixels};
static const uint8_t nova_clipboard_list_18_pixels[] = {0,0,0,0,0,0,14,34,34,34,34,14,0,0,0,0,0,0,0,0,0,0,0,74,246,255,255,255,255,246,73,0,0,0,0,0,0,0,0,85,159,223,199,34,34,34,34,200,223,159,85,0,0,0,0,0,85,249,174,223,200,34,34,34,34,200,223,174,250,84,0,0,0,0,156,170,0,74,246,255,255,255,255,245,74,0,171,156,0,0,0,0,162,162,0,0,14,34,34,34,34,14,0,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,162,162,0,39,40,0,39,102,102,102,39,0,162,162,0,0,0,0,162,162,0,116,118,0,116,221,221,221,116,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,162,162,0,77,79,0,77,170,170,170,77,0,162,162,0,0,0,0,162,162,0,77,79,0,77,170,170,170,77,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,136,211,43,34,34,34,34,34,34,34,34,43,212,134,0,0,0,0,24,211,255,255,255,255,255,255,255,255,255,255,211,23,0,0,0,0,0,0,29,34,34,34,34,34,34,34,34,29,0,0,0,0};
static const lv_img_dsc_t nova_clipboard_list_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_clipboard_list_18_pixels};
static const uint8_t nova_clipboard_list_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,188,221,221,221,221,221,221,187,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,188,251,221,221,221,221,221,221,251,186,0,0,0,0,0,0,0,0,0,0,0,94,192,221,250,238,0,0,0,0,0,0,238,250,221,192,93,0,0,0,0,0,0,0,94,255,242,221,250,238,0,0,0,0,0,0,238,250,221,242,255,92,0,0,0,0,0,0,192,242,21,0,187,251,221,221,221,221,221,221,251,186,0,22,243,191,0,0,0,0,0,0,217,217,0,0,41,187,221,221,221,221,221,221,186,40,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,139,141,0,0,141,221,221,221,219,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,139,141,0,0,141,221,221,221,219,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,139,141,0,0,141,221,221,221,219,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,139,141,0,0,141,221,221,221,219,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_clipboard_list_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_clipboard_list_24_pixels};
static const uint8_t nova_clipboard_list_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,17,17,17,17,17,17,17,17,17,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,162,247,255,255,255,255,255,255,255,255,255,255,247,162,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,163,255,255,255,255,255,255,255,255,255,255,255,255,255,255,161,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,247,255,242,187,187,187,187,187,187,187,187,187,187,242,255,246,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,113,166,187,193,255,255,203,0,0,0,0,0,0,0,0,0,0,203,255,255,193,187,166,113,14,0,0,0,0,0,0,0,0,0,0,0,0,0,55,225,255,255,255,255,255,255,235,0,0,0,0,0,0,0,0,0,0,235,255,255,255,255,255,255,224,54,0,0,0,0,0,0,0,0,0,0,0,15,225,255,255,255,255,255,255,255,217,0,0,0,0,0,0,0,0,0,0,217,255,255,255,255,255,255,255,224,14,0,0,0,0,0,0,0,0,0,0,113,255,255,200,50,17,28,254,255,221,102,102,102,102,102,102,102,102,102,102,221,255,253,28,17,51,201,255,255,112,0,0,0,0,0,0,0,0,0,0,166,255,255,50,0,0,0,204,255,255,255,255,255,255,255,255,255,255,255,255,255,255,202,0,0,0,52,255,255,165,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,49,230,255,255,255,255,255,255,255,255,255,255,255,255,229,48,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,16,80,102,102,102,102,102,102,102,102,102,102,80,16,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,11,0,0,0,0,0,6,17,17,17,17,17,17,17,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,186,253,108,0,0,0,37,234,255,255,255,255,255,255,255,186,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,10,253,255,182,0,0,0,92,255,255,255,255,255,255,255,255,253,10,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,103,179,51,0,0,0,10,154,187,187,187,187,187,187,187,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,103,179,51,0,0,0,10,154,187,187,187,187,187,187,187,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,10,253,255,182,0,0,0,92,255,255,255,255,255,255,255,255,253,10,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,186,253,108,0,0,0,37,234,255,255,255,255,255,255,255,186,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,11,0,0,0,0,0,6,17,17,17,17,17,17,17,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,180,255,255,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,255,255,180,0,0,0,0,0,0,0,0,0,0,131,255,255,141,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,143,255,255,130,0,0,0,0,0,0,0,0,0,0,45,249,255,255,220,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,220,255,255,249,44,0,0,0,0,0,0,0,0,0,0,0,115,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,113,0,0,0,0,0,0,0,0,0,0,0,0,0,69,197,246,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,246,197,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_clipboard_list_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_clipboard_list_40_pixels};
static const uint8_t nova_fingerprint_18_pixels[] = {0,0,0,0,0,0,0,6,26,26,5,0,0,0,0,0,0,0,0,0,0,0,1,83,184,246,255,255,245,185,83,1,0,0,0,0,0,0,0,25,192,248,161,82,43,44,82,161,248,192,24,0,0,0,0,0,25,223,202,36,0,0,20,20,0,0,36,204,222,24,0,0,0,1,192,203,11,0,104,226,255,255,225,114,2,12,203,141,0,0,0,84,248,37,0,0,139,120,50,51,121,241,175,2,8,8,0,0,0,185,160,0,15,2,0,0,12,8,0,42,241,113,0,0,16,0,5,246,81,0,220,82,0,112,251,140,0,0,122,223,0,59,241,0,26,255,43,14,254,56,12,251,109,8,0,0,52,255,17,60,255,8,8,139,8,45,255,25,44,255,25,14,180,14,35,255,35,52,255,17,0,0,0,64,255,7,61,255,9,40,255,29,26,251,26,60,255,9,8,139,9,83,243,0,77,248,0,47,255,22,0,16,0,68,254,2,8,139,9,113,217,0,31,123,0,55,255,15,0,16,0,25,130,0,0,0,0,169,163,0,0,0,0,72,251,4,70,234,0,0,0,0,0,0,0,211,108,0,91,64,0,105,224,0,119,209,0,0,0,0,0,0,0,36,7,0,226,110,0,162,174,0,158,167,0,0,0,0,0,0,0,0,0,21,251,37,0,207,100,0,45,38,0,0,0,0,0,0,0,0,0,0,16,0,0,15,1,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_fingerprint_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_fingerprint_18_pixels};
static const uint8_t nova_fingerprint_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,64,134,186,210,208,187,136,64,1,0,0,0,0,0,0,0,0,0,0,0,0,0,84,215,255,255,247,226,226,247,255,255,216,81,0,0,0,0,0,0,0,0,0,0,10,170,255,242,145,56,4,0,0,4,55,145,243,255,167,9,0,0,0,0,0,0,0,10,198,255,178,26,0,0,0,0,0,0,0,0,27,180,255,195,9,0,0,0,0,0,0,169,255,147,2,0,4,82,168,203,202,167,82,4,0,2,150,255,160,0,0,0,0,0,83,255,179,2,0,0,181,255,255,231,231,255,255,212,41,0,2,151,138,0,0,0,0,1,217,242,26,0,0,0,96,124,26,0,0,27,137,252,241,41,0,0,0,0,0,0,0,64,255,145,0,0,0,0,0,0,0,0,0,0,0,74,252,212,4,0,0,0,0,0,0,134,255,56,0,71,192,16,0,0,94,191,138,0,0,0,137,255,81,0,1,178,98,0,0,185,248,5,0,169,254,21,0,93,255,240,138,0,0,0,29,255,164,0,5,255,170,0,0,208,226,0,0,206,227,0,0,192,242,21,0,0,0,0,0,235,200,0,0,249,183,0,0,138,138,0,0,222,212,0,0,227,208,0,0,115,115,0,0,217,217,0,0,240,194,0,0,0,0,0,0,232,201,0,0,243,191,0,0,220,214,0,0,217,217,0,0,247,188,0,0,0,0,0,1,247,188,0,5,254,174,0,0,228,207,0,0,138,138,0,1,254,179,0,0,139,141,0,21,255,159,0,20,255,158,0,0,235,199,0,0,0,0,0,8,255,171,0,0,139,141,0,56,255,128,0,7,185,85,0,0,242,192,0,0,0,0,0,1,178,98,0,0,0,0,0,111,255,75,0,0,0,0,0,2,250,183,0,2,179,95,0,0,0,0,0,0,0,0,0,166,254,20,0,0,0,0,0,27,255,154,0,40,255,145,0,0,0,0,0,0,0,0,0,202,218,0,0,94,180,3,0,60,255,123,0,82,255,102,0,0,0,0,0,0,0,0,0,40,40,0,0,205,239,4,0,119,255,74,0,123,255,60,0,0,0,0,0,0,0,0,0,0,0,0,22,254,171,0,0,187,248,11,0,82,197,10,0,0,0,0,0,0,0,0,0,0,0,0,17,192,69,0,0,138,140,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_fingerprint_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_fingerprint_24_pixels};
static const uint8_t nova_fingerprint_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,52,117,180,227,248,255,255,248,229,179,120,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,103,210,255,255,255,255,255,255,255,255,255,255,255,255,209,107,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,84,229,255,255,255,255,255,246,220,198,201,219,246,255,255,255,255,255,225,84,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,178,255,255,255,255,208,116,51,3,0,0,0,0,4,51,114,209,255,255,255,255,177,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,213,255,255,255,191,61,0,0,0,0,0,0,0,0,0,0,0,0,58,195,255,255,255,212,35,0,0,0,0,0,0,0,0,0,0,0,0,0,34,231,255,255,239,95,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,93,242,255,255,228,29,0,0,0,0,0,0,0,0,0,0,0,14,211,255,255,228,47,0,0,0,0,0,0,33,78,101,101,78,33,0,0,0,0,0,0,45,222,255,255,213,17,0,0,0,0,0,0,0,0,0,0,178,255,255,222,25,0,0,0,0,15,124,218,255,255,255,255,255,255,217,123,15,0,0,0,0,27,226,255,255,154,0,0,0,0,0,0,0,0,0,85,255,255,241,43,0,0,0,0,37,235,255,255,255,255,255,255,255,255,255,255,237,93,0,0,0,0,44,239,255,154,0,0,0,0,0,0,0,0,12,228,255,255,94,0,0,0,0,0,92,255,255,255,202,141,113,113,142,203,255,255,255,255,149,3,0,0,0,36,93,10,0,0,0,0,0,0,0,0,108,255,255,194,1,0,0,0,0,0,10,154,158,44,0,0,0,0,0,0,45,180,255,255,255,149,0,0,0,0,0,0,0,0,0,0,0,0,0,0,212,255,255,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,252,255,255,93,0,0,0,0,0,0,0,0,0,0,0,0,54,255,255,207,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,237,15,0,0,0,0,0,0,0,0,0,0,0,119,255,255,114,0,0,0,64,180,85,0,0,0,0,0,15,113,166,154,10,0,0,0,0,0,181,255,255,121,0,0,0,0,103,178,48,0,0,0,0,181,255,255,50,0,0,0,215,255,237,0,0,0,0,55,225,255,255,255,92,0,0,0,0,0,47,255,255,215,0,0,0,10,253,255,179,0,0,0,0,226,255,244,3,0,0,28,255,255,206,0,0,0,15,225,255,255,255,234,37,0,0,0,0,0,0,206,255,255,30,0,0,7,255,255,199,0,0,0,0,247,255,220,0,0,0,75,255,255,145,0,0,0,113,255,255,200,50,6,0,0,0,0,0,0,0,144,255,255,76,0,0,0,251,255,212,0,0,0,13,255,255,200,0,0,0,97,255,255,116,0,0,0,166,255,255,50,0,0,0,0,0,0,0,0,0,115,255,255,98,0,0,0,240,255,226,0,0,0,5,235,255,154,0,0,0,112,255,255,101,0,0,0,197,255,255,16,0,0,0,60,127,23,0,0,0,107,255,255,107,0,0,0,228,255,240,0,0,0,0,36,93,10,0,0,0,123,255,255,91,0,0,0,207,255,255,6,0,0,8,244,255,167,0,0,0,107,255,255,107,0,0,0,227,255,241,0,0,0,0,0,0,0,0,0,0,132,255,255,81,0,0,0,216,255,251,0,0,0,24,255,255,189,0,0,0,107,255,255,107,0,0,0,230,255,238,0,0,0,0,0,0,0,0,0,0,144,255,255,71,0,0,0,233,255,237,0,0,0,26,255,255,187,0,0,0,95,255,255,95,0,0,0,233,255,234,0,0,0,0,0,0,0,0,0,0,169,255,255,48,0,0,3,253,255,214,0,0,0,28,255,255,185,0,0,0,10,154,154,10,0,0,0,245,255,223,0,0,0,0,103,179,51,0,0,0,196,255,255,20,0,0,23,255,255,192,0,0,0,32,255,255,181,0,0,0,0,0,0,0,0,0,5,255,255,208,0,0,0,10,253,255,182,0,0,0,224,255,247,1,0,0,40,255,255,170,0,0,0,45,255,255,169,0,0,0,0,0,0,0,0,0,16,255,255,192,0,0,0,0,186,253,108,0,0,8,251,255,218,0,0,0,6,205,249,82,0,0,0,57,255,255,157,0,0,0,0,0,0,0,0,0,0,186,252,104,0,0,0,0,0,11,0,0,0,50,255,255,172,0,0,0,0,2,9,0,0,0,0,69,255,255,145,0,0,0,11,94,34,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,95,255,255,128,0,0,0,0,0,0,0,0,0,0,88,255,255,129,0,0,0,160,255,232,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,153,255,255,80,0,0,0,0,0,0,0,0,0,0,121,255,255,98,0,0,0,221,255,250,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,221,255,252,18,0,0,0,0,11,0,0,0,0,0,152,255,255,66,0,0,10,253,255,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,255,255,202,0,0,0,0,178,253,112,0,0,0,0,184,255,255,34,0,0,51,255,255,170,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,143,215,75,0,0,0,33,255,255,189,0,0,0,0,231,255,248,5,0,0,94,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,99,255,255,135,0,0,0,35,255,255,197,0,0,0,134,255,255,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,178,255,255,69,0,0,0,112,255,255,135,0,0,0,120,255,251,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,242,255,239,5,0,0,0,178,255,255,53,0,0,0,4,90,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,254,124,0,0,0,0,104,252,186,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_fingerprint_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_fingerprint_40_pixels};
static const uint8_t nova_memory_stick_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,34,34,34,34,34,34,34,34,34,34,34,34,12,0,0,0,112,251,255,255,255,255,255,255,255,255,255,255,255,255,251,112,0,12,251,109,34,34,34,34,34,34,34,34,34,34,34,34,110,251,12,35,255,35,0,0,8,8,0,8,8,0,8,8,0,0,35,255,35,36,255,118,7,0,151,151,0,151,151,0,151,151,0,7,118,255,36,48,255,223,36,0,162,162,0,162,162,0,162,162,0,36,223,255,48,35,255,35,0,0,77,77,0,77,77,0,77,77,0,0,35,255,35,29,255,43,0,0,0,0,0,0,0,0,0,0,0,0,43,255,28,1,211,225,196,170,196,196,170,196,196,170,196,196,170,196,225,210,1,0,23,211,224,170,224,224,170,224,224,170,224,224,170,224,211,23,0,0,0,151,151,0,151,151,0,151,151,0,151,151,0,151,151,0,0,0,0,8,8,0,8,8,0,8,8,0,8,8,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_memory_stick_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_memory_stick_18_pixels};
static const uint8_t nova_memory_stick_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,138,138,0,0,138,138,0,0,138,138,0,0,0,0,217,217,0,0,238,250,206,40,0,0,217,217,0,0,217,217,0,0,217,217,0,0,40,211,250,238,0,0,238,250,206,40,0,0,217,217,0,0,217,217,0,0,217,217,0,0,40,211,250,238,0,0,217,217,0,0,0,0,138,138,0,0,138,138,0,0,138,138,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,249,239,221,221,239,239,221,221,239,239,221,221,239,239,221,221,239,249,255,90,0,0,0,92,245,250,221,221,250,250,221,221,250,250,221,221,250,250,221,221,250,245,90,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,138,138,0,0,138,138,0,0,138,138,0,0,138,138,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_memory_stick_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_memory_stick_24_pixels};
static const uint8_t nova_memory_stick_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,94,47,3,0,0,0,0,0,0,0,44,205,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,210,49,0,0,0,0,0,27,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,30,0,0,0,0,169,255,255,209,116,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,116,210,255,255,168,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,238,255,233,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,194,255,255,19,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,103,178,48,0,0,0,10,154,154,10,0,0,0,48,178,103,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,195,17,3,0,0,0,0,12,253,255,179,0,0,0,95,255,255,95,0,0,0,179,255,253,12,0,0,0,0,3,17,195,255,255,22,0,0,22,255,255,255,255,216,13,0,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,13,216,255,255,255,255,22,0,0,31,255,255,255,255,255,49,0,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,49,255,255,255,255,255,31,0,0,22,255,255,238,187,133,1,0,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,1,133,187,238,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,5,235,255,154,0,0,0,70,255,255,70,0,0,0,154,255,235,5,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,36,93,10,0,0,0,0,70,70,0,0,0,0,10,93,36,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,0,217,255,251,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,251,255,216,0,0,0,0,125,255,255,252,239,214,187,187,187,187,214,234,200,187,187,187,190,228,228,190,187,187,187,200,234,214,187,187,187,187,214,239,252,255,255,123,0,0,0,0,5,195,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,193,4,0,0,0,0,0,5,125,245,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,245,124,4,0,0,0,0,0,0,0,0,191,255,255,38,17,17,38,255,255,195,17,17,17,117,255,255,117,17,17,17,195,255,255,38,17,17,38,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,154,255,235,5,0,0,5,235,255,154,0,0,0,70,255,255,70,0,0,0,154,255,235,5,0,0,5,235,255,154,0,0,0,0,0,0,0,0,0,0,10,93,36,0,0,0,0,36,93,10,0,0,0,0,70,70,0,0,0,0,10,93,36,0,0,0,0,36,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_memory_stick_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_memory_stick_40_pixels};
static const uint8_t nova_wifi_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,82,153,194,215,214,193,152,81,11,0,0,0,0,0,0,14,127,237,246,184,132,110,110,133,185,246,236,126,13,0,0,0,52,220,227,110,10,0,0,0,0,0,0,11,111,228,219,52,0,16,248,169,14,0,0,0,3,24,24,2,0,0,0,15,170,248,16,0,42,0,0,1,80,183,247,255,255,246,182,79,1,0,0,42,0,0,0,0,22,191,250,160,83,45,45,84,161,250,189,22,0,0,0,0,0,0,198,207,36,0,0,0,0,0,0,38,208,198,0,0,0,0,0,0,40,10,0,0,25,76,75,24,0,0,10,40,0,0,0,0,0,0,0,0,4,147,252,246,246,252,146,4,0,0,0,0,0,0,0,0,0,0,40,219,75,4,4,76,219,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,79,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,79,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wifi_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_wifi_18_pixels};
static const uint8_t nova_wifi_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,106,160,182,207,207,182,160,105,35,0,0,0,0,0,0,0,0,0,0,0,6,102,216,255,255,255,249,228,229,250,255,255,255,215,100,6,0,0,0,0,0,0,0,88,227,255,240,157,87,24,0,0,0,0,24,87,158,240,255,226,86,0,0,0,0,9,155,255,237,124,16,0,0,0,0,0,0,0,0,0,0,16,125,237,255,153,9,0,0,180,255,191,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27,192,255,180,0,0,103,118,3,0,0,0,2,60,135,181,206,206,180,134,59,2,0,0,0,3,118,103,0,0,0,0,0,0,0,82,211,255,255,251,229,229,251,255,255,210,80,0,0,0,0,0,0,0,0,0,0,8,161,255,246,145,60,3,0,0,4,62,146,247,255,159,8,0,0,0,0,0,0,0,0,175,255,184,26,0,0,0,0,0,0,0,0,27,185,255,175,0,0,0,0,0,0,0,0,109,124,3,0,0,0,0,0,0,0,0,0,0,3,124,109,0,0,0,0,0,0,0,0,0,0,0,0,0,67,161,206,206,160,66,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,167,255,255,230,230,255,255,165,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,71,255,181,39,0,0,40,182,255,71,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,55,2,0,0,0,0,2,55,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,139,141,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,139,141,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wifi_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_wifi_24_pixels};
static const uint8_t nova_wifi_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,17,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,68,126,177,212,246,255,255,255,255,246,212,176,126,67,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,80,170,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,244,169,79,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,115,224,255,255,255,255,255,255,253,227,204,197,197,204,228,254,255,255,255,255,255,255,223,114,8,0,0,0,0,0,0,0,0,0,0,0,0,1,94,226,255,255,255,255,243,169,105,47,7,0,0,0,0,0,0,7,48,106,170,243,255,255,255,255,225,92,0,0,0,0,0,0,0,0,0,0,30,184,255,255,255,255,194,85,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,86,195,255,255,255,255,182,28,0,0,0,0,0,0,0,72,239,255,255,255,193,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,194,255,255,255,238,70,0,0,0,0,0,108,252,255,255,235,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,236,255,255,252,108,0,0,0,10,252,255,255,188,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,190,255,255,252,10,0,0,1,193,254,144,4,0,0,0,0,0,0,0,0,1,44,103,131,158,180,180,157,131,102,43,1,0,0,0,0,0,0,0,0,4,144,254,193,1,0,0,0,1,13,0,0,0,0,0,0,0,0,33,138,226,255,255,255,255,255,255,255,255,255,255,225,136,31,0,0,0,0,0,0,0,0,13,1,0,0,0,0,0,0,0,0,0,0,0,0,25,160,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,158,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,242,255,255,255,253,195,126,79,57,32,32,57,80,127,196,254,255,255,255,241,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,155,255,255,255,243,138,30,0,0,0,0,0,0,0,0,0,0,31,140,244,255,255,255,153,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,255,173,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,175,255,255,255,172,0,0,0,0,0,0,0,0,0,0,0,0,0,11,255,255,251,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,252,255,255,11,0,0,0,0,0,0,0,0,0,0,0,0,0,127,198,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,74,198,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,71,94,94,70,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,71,191,252,255,255,255,255,251,190,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,176,255,255,255,255,255,255,255,255,255,255,174,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,212,255,255,255,226,154,117,118,155,227,255,255,255,212,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,255,255,242,100,4,0,0,0,0,4,101,242,255,255,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,146,184,42,0,0,0,0,0,0,0,0,42,184,146,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,236,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,255,255,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,154,156,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_wifi_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_wifi_40_pixels};
static const uint8_t nova_audio_lines_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,76,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,255,33,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,35,255,35,0,0,0,17,203,17,0,0,0,0,0,0,26,251,26,35,255,35,0,0,0,35,255,35,0,0,0,0,0,0,35,255,35,35,255,35,8,139,8,35,255,35,0,0,0,0,16,0,35,255,35,35,255,35,35,255,35,35,255,35,0,16,0,26,251,26,35,255,35,35,255,35,35,255,35,35,255,35,26,251,26,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,33,255,33,35,255,35,35,255,35,35,255,35,35,255,35,33,255,33,1,76,1,35,255,35,35,255,35,35,255,35,35,255,35,1,76,1,0,0,0,35,255,35,35,255,35,17,203,17,35,255,35,0,0,0,0,0,0,33,255,33,35,255,35,0,0,0,35,255,35,0,0,0,0,0,0,1,76,1,35,255,35,0,0,0,26,251,26,0,0,0,0,0,0,0,0,0,35,255,35,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,33,255,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,76,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_audio_lines_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_audio_lines_18_pixels};
static const uint8_t nova_audio_lines_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,138,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,138,138,0,0,217,217,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,138,138,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,138,138,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,138,138,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,138,138,0,0,0,0,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,138,138,0,0,217,217,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,138,138,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_audio_lines_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_audio_lines_24_pixels};
static const uint8_t nova_audio_lines_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,154,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,37,234,234,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,104,255,255,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,0,11,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,186,252,104,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,19,255,255,189,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,103,178,48,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,48,178,103,0,0,0,12,253,255,179,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,179,255,253,12,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,19,255,255,189,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,189,255,255,19,0,0,0,186,252,104,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,104,252,186,0,0,0,0,0,11,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,5,235,255,154,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,36,93,10,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,255,255,95,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,154,154,10,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,154,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,93,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_audio_lines_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_audio_lines_40_pixels};
static const uint8_t nova_volume_2_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,138,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,209,249,216,0,0,0,0,1,82,4,0,0,0,0,0,0,21,209,209,119,226,0,0,0,0,11,237,143,0,0,0,74,164,170,219,209,20,99,226,0,0,0,0,0,71,254,55,0,14,246,182,170,135,17,0,99,226,0,0,116,130,0,0,179,170,0,35,255,35,0,0,0,0,99,226,0,0,81,249,26,0,90,242,2,35,255,35,0,0,0,0,99,226,0,0,2,244,78,0,47,255,22,35,255,35,0,0,0,0,99,226,0,0,2,245,78,0,49,255,21,35,255,35,0,0,0,0,99,226,0,0,82,249,25,0,91,241,2,14,246,182,170,136,17,0,99,226,0,0,116,130,0,0,180,170,0,0,74,164,170,219,209,20,99,226,0,0,0,0,0,72,254,54,0,0,0,0,0,17,204,214,122,226,0,0,0,0,11,237,142,0,0,0,0,0,0,0,20,209,249,215,0,0,0,0,1,82,4,0,0,0,0,0,0,0,0,20,138,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_volume_2_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_volume_2_18_pixels};
static const uint8_t nova_volume_2_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,141,196,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,198,0,0,0,0,0,0,3,36,0,0,0,0,0,0,0,0,0,0,5,174,255,174,222,217,0,0,0,0,0,0,103,254,88,0,0,0,0,0,0,0,0,5,174,255,180,7,217,217,0,0,0,0,0,0,28,232,245,43,0,0,0,42,188,221,221,232,255,174,5,0,217,217,0,0,0,0,0,0,0,62,254,180,0,0,0,188,251,221,221,210,131,5,0,0,217,217,0,0,0,138,142,0,0,0,173,255,45,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,171,254,53,0,0,72,255,123,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,47,255,153,0,0,8,252,179,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,0,238,195,0,0,0,230,205,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,0,238,195,0,0,0,231,203,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,47,255,152,0,0,8,252,178,0,0,217,217,0,0,0,0,0,0,0,217,217,0,0,0,171,254,52,0,0,74,255,121,0,0,187,251,221,221,210,131,5,0,0,217,217,0,0,0,138,142,0,0,0,174,255,44,0,0,41,187,221,221,231,255,174,5,0,217,217,0,0,0,0,0,0,0,63,254,179,0,0,0,0,0,0,0,5,174,255,180,7,217,217,0,0,0,0,0,0,28,232,245,43,0,0,0,0,0,0,0,0,5,174,255,174,222,217,0,0,0,0,0,0,103,254,88,0,0,0,0,0,0,0,0,0,0,5,174,255,255,197,0,0,0,0,0,0,3,36,0,0,0,0,0,0,0,0,0,0,0,0,5,140,195,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_volume_2_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_volume_2_24_pixels};
static const uint8_t nova_volume_2_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,148,159,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,252,72,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,164,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,213,255,255,191,0,0,0,0,0,0,0,0,0,0,2,183,244,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,204,39,255,255,191,0,0,0,0,0,0,0,0,0,0,24,255,255,251,72,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,204,17,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,158,255,255,236,28,0,0,0,0,0,0,0,0,5,17,17,17,17,72,241,255,255,199,15,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,5,188,255,255,167,0,0,0,0,0,0,14,163,247,255,255,255,255,255,255,255,199,15,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,24,239,255,253,48,0,0,0,0,0,162,255,255,255,255,255,255,255,255,191,14,0,0,0,22,255,255,191,0,0,0,0,0,10,93,36,0,0,0,0,0,122,255,255,180,0,0,0,0,5,247,255,242,187,187,187,187,167,99,2,0,0,0,0,22,255,255,191,0,0,0,0,0,154,255,237,23,0,0,0,0,11,231,255,253,23,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,155,255,255,159,0,0,0,0,0,143,255,255,97,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,28,242,255,249,24,0,0,0,0,68,255,255,172,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,146,255,255,101,0,0,0,0,6,251,255,215,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,69,255,255,156,0,0,0,0,0,228,255,240,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,32,255,255,181,0,0,0,0,0,204,255,255,10,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,33,255,255,180,0,0,0,0,0,205,255,255,9,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,70,255,255,156,0,0,0,0,0,230,255,239,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,147,255,255,100,0,0,0,0,7,251,255,215,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,28,243,255,249,23,0,0,0,0,69,255,255,171,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,155,255,255,158,0,0,0,0,0,145,255,255,95,0,0,0,5,247,255,242,187,187,187,187,167,99,2,0,0,0,0,22,255,255,191,0,0,0,0,0,154,255,237,22,0,0,0,0,11,233,255,253,22,0,0,0,0,162,255,255,255,255,255,255,255,255,190,14,0,0,0,22,255,255,191,0,0,0,0,0,10,93,36,0,0,0,0,0,123,255,255,179,0,0,0,0,0,14,161,246,255,255,255,255,255,255,255,198,14,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,25,239,255,253,47,0,0,0,0,0,0,0,4,17,17,17,17,72,241,255,255,198,14,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,5,190,255,255,165,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,238,255,255,204,17,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,158,255,255,235,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,238,255,255,204,39,255,255,191,0,0,0,0,0,0,0,0,0,0,24,255,255,250,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,212,255,255,191,0,0,0,0,0,0,0,0,0,0,2,183,244,91,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,255,255,163,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,252,71,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,148,160,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_volume_2_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_volume_2_40_pixels};
static const uint8_t nova_monitor_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,73,102,102,102,102,102,102,102,102,102,102,102,102,72,2,0,0,170,248,221,221,221,221,221,221,221,221,221,221,221,221,248,169,0,21,255,61,0,0,0,0,0,0,0,0,0,0,0,0,62,255,20,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,35,255,35,0,0,0,0,0,0,0,0,0,0,0,0,35,255,35,21,255,61,0,0,0,0,0,0,0,0,0,0,0,0,62,255,20,0,169,248,221,221,221,221,221,237,237,221,221,221,221,221,248,168,0,0,2,71,102,102,102,102,102,199,199,102,102,102,102,102,71,2,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,116,221,221,242,242,221,221,116,0,0,0,0,0,0,0,0,0,0,39,102,102,125,125,102,102,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_monitor_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_monitor_18_pixels};
static const uint8_t nova_monitor_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,92,255,243,221,221,221,221,221,221,221,239,239,221,221,221,221,221,221,221,243,255,90,0,0,0,92,191,221,221,221,221,221,221,221,250,250,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,250,250,221,221,219,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,239,239,221,221,219,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_monitor_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_monitor_24_pixels};
static const uint8_t nova_monitor_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,94,47,3,0,0,0,0,0,0,0,44,205,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,210,49,0,0,0,0,0,27,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,30,0,0,0,0,169,255,255,209,116,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,116,210,255,255,168,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,238,255,233,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,194,255,255,19,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,5,247,255,219,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,220,255,246,5,0,0,0,198,255,255,141,32,17,17,17,17,17,17,17,17,17,17,17,17,23,23,17,17,17,17,17,17,17,17,17,17,17,17,32,143,255,255,197,0,0,0,0,67,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,71,0,0,0,0,0,115,249,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,249,113,0,0,0,0,0,0,0,45,130,178,187,187,187,187,187,187,187,187,187,187,187,216,255,255,216,187,187,187,187,187,187,187,187,187,187,187,178,130,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,102,102,166,255,255,166,102,102,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,255,255,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,235,255,255,255,255,255,255,255,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,102,102,102,102,102,102,144,144,102,102,102,102,102,102,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_monitor_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_monitor_40_pixels};
static const uint8_t nova_gamepad_2_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,128,202,221,221,221,221,221,221,221,221,202,128,10,0,0,0,4,193,228,124,102,102,102,102,102,102,102,102,124,229,193,4,0,0,89,242,29,0,0,0,0,0,0,0,0,0,0,30,242,87,0,0,155,175,0,0,118,118,0,0,0,0,0,0,16,0,175,155,0,0,188,139,1,89,199,199,89,1,0,0,0,16,247,18,140,187,0,0,216,111,15,212,243,243,212,15,0,39,117,0,16,0,111,216,0,0,240,86,0,0,160,160,0,0,0,39,117,0,0,0,86,240,0,6,254,64,0,0,39,39,0,0,0,0,0,0,0,0,65,255,6,25,255,45,0,0,2,100,164,170,170,164,100,2,0,0,45,255,25,12,252,68,0,3,163,242,171,170,170,171,242,163,3,0,69,252,12,0,166,226,116,182,239,53,0,0,0,0,53,239,182,116,226,165,0,0,10,138,209,175,44,0,0,0,0,0,0,44,175,209,137,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_gamepad_2_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_gamepad_2_18_pixels};
static const uint8_t nova_gamepad_2_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,68,161,207,221,221,221,221,221,221,221,221,221,221,207,161,68,0,0,0,0,0,0,0,149,255,255,229,221,221,221,221,221,221,221,221,221,221,229,255,255,148,0,0,0,0,0,105,255,192,33,0,0,0,0,0,0,0,0,0,0,0,0,34,192,255,103,0,0,0,0,216,231,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,232,215,0,0,0,30,255,155,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,155,255,30,0,0,66,255,114,0,0,0,217,217,0,0,0,0,0,0,0,0,139,141,0,116,255,65,0,0,95,255,85,0,141,221,250,250,219,138,0,0,0,0,0,0,139,141,0,87,255,94,0,0,123,255,57,0,141,221,250,250,219,138,0,0,0,139,141,0,0,0,0,59,255,122,0,0,147,255,32,0,0,0,217,217,0,0,0,0,0,139,141,0,0,0,0,33,255,146,0,0,168,255,11,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,13,255,166,0,0,188,246,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,187,0,0,207,227,0,0,0,0,4,118,201,221,221,221,221,201,118,4,0,0,0,0,227,207,0,0,191,241,3,0,0,5,174,255,236,221,221,221,221,237,255,174,5,0,0,4,241,191,0,0,124,255,140,3,10,177,255,174,7,0,0,0,0,7,174,255,177,10,3,140,255,124,0,0,8,206,255,241,239,255,170,5,0,0,0,0,0,0,5,170,255,239,241,255,206,8,0,0,0,8,123,191,200,113,2,0,0,0,0,0,0,0,0,2,113,200,191,123,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_gamepad_2_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_gamepad_2_24_pixels};
static const uint8_t nova_gamepad_2_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,158,229,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,229,158,49,0,0,0,0,0,0,0,0,0,0,0,0,8,152,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,150,8,0,0,0,0,0,0,0,0,0,7,186,255,255,255,248,206,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,207,249,255,255,255,184,6,0,0,0,0,0,0,0,0,142,255,255,249,122,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,124,249,255,255,138,0,0,0,0,0,0,0,37,252,255,250,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,251,255,251,35,0,0,0,0,0,0,135,255,255,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,140,255,255,133,0,0,0,0,0,0,197,255,255,32,0,0,0,0,0,36,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,255,255,196,0,0,0,0,0,0,240,255,234,0,0,0,0,0,5,235,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,234,255,240,0,0,0,0,0,15,255,255,202,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,10,154,156,12,0,0,204,255,255,14,0,0,0,0,44,255,255,173,0,0,0,6,17,38,255,255,195,17,17,0,0,0,0,0,0,0,0,0,0,0,92,255,255,96,0,0,175,255,255,42,0,0,0,0,73,255,255,144,0,0,37,234,255,255,255,255,255,255,255,186,0,0,0,0,0,0,0,0,0,0,37,234,236,40,0,0,146,255,255,71,0,0,0,0,102,255,255,115,0,0,92,255,255,255,255,255,255,255,255,253,10,0,0,0,0,0,70,71,0,0,0,6,6,0,0,0,117,255,255,100,0,0,0,0,130,255,255,88,0,0,10,154,187,193,255,255,238,187,187,103,0,0,0,0,0,70,255,255,74,0,0,0,0,0,0,0,88,255,255,129,0,0,0,0,157,255,255,60,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,70,255,255,74,0,0,0,0,0,0,0,61,255,255,155,0,0,0,0,182,255,255,34,0,0,0,0,0,19,255,255,189,0,0,0,0,0,0,0,0,0,70,71,0,0,0,0,0,0,0,0,36,255,255,179,0,0,0,0,207,255,255,10,0,0,0,0,0,0,186,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,255,255,203,0,0,0,0,230,255,241,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,242,255,227,0,0,0,0,247,255,222,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,255,245,0,0,0,6,255,255,208,0,0,0,0,0,0,0,0,0,78,147,185,187,187,187,187,187,187,185,147,77,0,0,0,0,0,0,0,0,0,208,255,255,5,0,0,17,255,255,196,0,0,0,0,0,0,0,14,172,255,255,255,255,255,255,255,255,255,255,255,255,172,14,0,0,0,0,0,0,0,196,255,255,17,0,0,4,252,255,221,0,0,0,0,0,0,14,198,255,255,255,255,255,255,255,255,255,255,255,255,255,255,198,14,0,0,0,0,0,0,223,255,251,3,0,0,0,201,255,255,71,0,0,0,0,14,198,255,255,241,90,22,17,17,17,17,17,17,22,91,241,255,255,198,14,0,0,0,0,74,255,255,200,0,0,0,0,101,255,255,236,72,0,0,28,207,255,255,241,57,0,0,0,0,0,0,0,0,0,0,57,241,255,255,207,27,0,0,74,237,255,255,99,0,0,0,0,4,194,255,255,255,223,200,246,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,57,241,255,255,246,200,223,255,255,255,192,3,0,0,0,0,0,19,194,255,255,255,255,255,255,225,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,226,255,255,255,255,255,255,192,18,0,0,0,0,0,0,0,4,100,201,252,255,236,150,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,152,236,255,251,200,99,3,0,0,0,0,0,0,0,0,0,0,0,3,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_gamepad_2_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_gamepad_2_40_pixels};
static const uint8_t nova_arrow_up_right_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,99,102,102,102,102,102,102,138,33,0,0,0,0,0,0,0,0,62,220,221,221,221,221,221,235,255,135,0,0,0,0,0,0,0,0,0,0,0,0,0,0,100,252,238,99,0,0,0,0,0,0,0,0,0,0,0,0,0,100,252,106,226,99,0,0,0,0,0,0,0,0,0,0,0,0,100,252,106,0,226,99,0,0,0,0,0,0,0,0,0,0,0,100,252,106,0,0,226,99,0,0,0,0,0,0,0,0,0,0,100,252,106,0,0,0,226,99,0,0,0,0,0,0,0,0,0,100,252,106,0,0,0,0,226,99,0,0,0,0,0,0,0,0,64,253,103,0,0,0,0,0,224,96,0,0,0,0,0,0,0,0,17,64,0,0,0,0,0,0,62,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_arrow_up_right_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_arrow_up_right_18_pixels};
static const uint8_t nova_arrow_up_right_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,221,222,242,203,0,0,0,0,0,0,0,0,0,0,0,0,141,221,221,221,221,221,221,221,222,244,255,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,244,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,218,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_arrow_up_right_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_arrow_up_right_24_pixels};
static const uint8_t nova_arrow_up_right_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,215,232,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,232,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,252,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,218,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,17,17,17,17,17,17,17,17,17,17,17,17,136,255,255,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,138,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,12,253,255,179,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,178,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_arrow_up_right_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_arrow_up_right_40_pixels};
static const uint8_t nova_clock_3_18_pixels[] = {0,0,0,0,0,0,0,1,23,23,1,0,0,0,0,0,0,0,0,0,0,0,1,81,181,247,255,255,247,181,81,1,0,0,0,0,0,0,0,22,191,250,161,84,46,46,84,162,250,191,22,0,0,0,0,0,22,221,206,36,0,0,8,8,0,0,38,207,220,21,0,0,0,1,192,206,12,0,0,0,151,151,0,0,0,12,207,189,0,0,0,82,250,37,0,0,0,0,162,162,0,0,0,0,38,250,79,0,0,181,161,0,0,0,0,0,162,162,0,0,0,0,0,163,179,0,2,247,83,0,0,0,0,0,162,162,0,0,0,0,0,85,246,1,23,255,46,0,0,0,0,0,162,224,170,170,77,0,0,48,255,22,22,255,47,0,0,0,0,0,77,170,170,170,77,0,0,49,255,21,2,247,83,0,0,0,0,0,0,0,0,0,0,0,0,85,246,1,0,180,162,0,0,0,0,0,0,0,0,0,0,0,0,164,178,0,0,80,250,37,0,0,0,0,0,0,0,0,0,0,39,250,78,0,0,1,190,207,12,0,0,0,0,0,0,0,0,12,208,188,0,0,0,0,22,220,207,38,0,0,0,0,0,0,39,208,220,21,0,0,0,0,0,21,189,250,163,85,47,47,86,164,250,188,21,0,0,0,0,0,0,0,0,79,179,247,255,255,247,178,78,0,0,0,0,0,0,0,0,0,0,0,0,0,23,23,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_clock_3_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_clock_3_18_pixels};
static const uint8_t nova_clock_3_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,58,137,179,205,205,179,136,57,2,0,0,0,0,0,0,0,0,0,0,0,0,0,79,214,255,255,252,230,230,252,255,255,213,78,0,0,0,0,0,0,0,0,0,0,8,164,255,246,144,59,2,0,0,3,60,145,246,255,163,8,0,0,0,0,0,0,0,8,196,255,182,28,0,0,0,0,0,0,0,0,28,183,255,195,8,0,0,0,0,0,0,164,255,151,3,0,0,0,0,138,138,0,0,0,0,3,152,255,162,0,0,0,0,0,79,255,182,3,0,0,0,0,0,217,217,0,0,0,0,0,3,184,255,76,0,0,0,2,214,246,28,0,0,0,0,0,0,217,217,0,0,0,0,0,0,29,247,212,2,0,0,58,255,144,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,146,255,56,0,0,136,255,59,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,62,255,134,0,0,179,252,3,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,4,252,178,0,0,205,229,0,0,0,0,0,0,0,0,217,250,221,221,219,138,0,0,0,0,230,204,0,0,204,230,0,0,0,0,0,0,0,0,141,221,221,221,219,138,0,0,0,0,231,203,0,0,179,252,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,252,177,0,0,135,255,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,255,133,0,0,57,255,145,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,147,255,55,0,0,2,213,246,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,247,212,2,0,0,0,78,255,183,3,0,0,0,0,0,0,0,0,0,0,0,0,3,185,255,76,0,0,0,0,0,163,255,152,3,0,0,0,0,0,0,0,0,0,0,3,153,255,161,0,0,0,0,0,0,8,195,255,184,29,0,0,0,0,0,0,0,0,30,185,255,194,8,0,0,0,0,0,0,0,8,162,255,247,146,61,3,0,0,4,62,146,247,255,161,8,0,0,0,0,0,0,0,0,0,0,76,212,255,255,253,230,230,253,255,255,212,75,0,0,0,0,0,0,0,0,0,0,0,0,0,2,55,134,178,204,204,178,134,55,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_clock_3_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_clock_3_24_pixels};
static const uint8_t nova_clock_3_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,17,17,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,58,129,189,229,255,255,255,255,228,189,128,57,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,115,221,255,255,255,255,255,255,255,255,255,255,255,255,220,113,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,96,233,255,255,255,255,255,245,210,198,198,210,246,255,255,255,255,255,232,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,181,255,255,255,255,199,109,41,1,0,0,0,0,1,42,110,200,255,255,255,255,179,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,222,255,255,255,184,54,0,0,0,0,0,0,0,0,0,0,0,0,55,186,255,255,255,220,35,0,0,0,0,0,0,0,0,0,0,0,0,0,37,232,255,255,239,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,240,255,255,230,35,0,0,0,0,0,0,0,0,0,0,0,20,222,255,255,219,35,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,37,220,255,255,220,18,0,0,0,0,0,0,0,0,0,1,181,255,255,219,24,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,25,220,255,255,178,1,0,0,0,0,0,0,0,0,97,255,255,239,35,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,37,241,255,255,92,0,0,0,0,0,0,0,12,233,255,255,87,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,91,255,255,231,10,0,0,0,0,0,0,114,255,255,184,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,188,255,255,112,0,0,0,0,0,1,220,255,255,57,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,55,255,255,219,1,0,0,0,0,58,255,255,199,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,201,255,255,55,0,0,0,0,129,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,112,255,255,127,0,0,0,0,189,255,255,41,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,43,255,255,187,0,0,0,0,230,255,245,2,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,2,246,255,227,0,0,0,4,254,255,212,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,166,102,102,102,102,102,36,0,0,0,0,0,0,0,214,255,254,3,0,0,16,255,255,197,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,198,255,255,15,0,0,16,255,255,197,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,255,255,255,255,255,255,235,5,0,0,0,0,0,0,198,255,255,15,0,0,4,254,255,212,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,102,102,102,102,102,102,102,36,0,0,0,0,0,0,0,214,255,254,3,0,0,0,229,255,246,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,246,255,226,0,0,0,0,189,255,255,42,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,255,255,186,0,0,0,0,128,255,255,110,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,113,255,255,126,0,0,0,0,57,255,255,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,202,255,255,54,0,0,0,0,0,217,255,255,58,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,255,255,219,1,0,0,0,0,0,113,255,255,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,255,255,111,0,0,0,0,0,0,11,232,255,255,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,255,255,230,10,0,0,0,0,0,0,0,95,255,255,240,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,242,255,255,90,0,0,0,0,0,0,0,0,1,179,255,255,220,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,222,255,255,176,1,0,0,0,0,0,0,0,0,0,18,220,255,255,221,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,222,255,255,218,17,0,0,0,0,0,0,0,0,0,0,0,35,230,255,255,241,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,242,255,255,230,34,0,0,0,0,0,0,0,0,0,0,0,0,0,35,220,255,255,255,188,56,0,0,0,0,0,0,0,0,0,0,0,0,58,189,255,255,255,218,34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,178,255,255,255,255,201,113,43,2,0,0,0,0,2,44,113,202,255,255,255,255,176,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,93,231,255,255,255,255,255,247,211,199,199,212,247,255,255,255,255,255,230,91,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,112,219,255,255,255,255,255,255,255,255,255,255,255,255,218,111,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,127,187,226,255,255,255,255,226,187,126,54,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_clock_3_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_clock_3_40_pixels};
static const uint8_t nova_battery_full_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,34,34,34,34,34,34,34,34,34,12,0,0,0,0,0,0,112,251,255,255,255,255,255,255,255,255,255,251,112,0,0,0,0,12,251,109,34,34,34,34,34,34,34,34,34,110,251,12,0,0,0,35,255,35,0,16,0,0,16,0,0,16,0,35,255,35,0,16,0,35,255,35,26,251,26,26,251,26,26,251,26,35,255,35,26,251,26,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,35,255,35,26,251,26,26,251,26,26,251,26,35,255,35,26,251,26,35,255,35,0,16,0,0,16,0,0,16,0,35,255,35,0,16,0,12,251,109,34,34,34,34,34,34,34,34,34,111,250,12,0,0,0,0,111,251,255,255,255,255,255,255,255,255,255,251,110,0,0,0,0,0,0,12,34,34,34,34,34,34,34,34,34,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_full_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_battery_full_18_pixels};
static const uint8_t nova_battery_full_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,0,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,138,138,0,0,138,138,0,0,138,138,0,0,217,217,0,0,138,138,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,217,217,0,0,138,138,0,0,138,138,0,0,138,138,0,0,217,217,0,0,138,138,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_full_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_battery_full_24_pixels};
static const uint8_t nova_battery_full_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,94,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,205,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,241,102,0,0,0,0,0,0,0,0,0,0,0,0,27,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,97,0,0,0,0,0,0,0,0,0,0,0,169,255,255,209,116,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,169,255,255,240,12,0,0,0,0,0,0,0,0,0,0,234,255,237,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,255,63,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,255,255,104,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,10,154,154,10,0,0,0,48,178,103,0,0,0,0,103,178,48,0,0,0,107,255,255,107,0,0,0,48,178,103,0,0,0,22,255,255,191,0,0,0,95,255,255,95,0,0,0,179,255,253,12,0,0,12,253,255,179,0,0,0,107,255,255,107,0,0,0,179,255,253,12,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,107,255,255,107,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,95,255,255,95,0,0,0,179,255,253,12,0,0,12,253,255,179,0,0,0,107,255,255,107,0,0,0,179,255,253,12,0,0,22,255,255,191,0,0,0,10,154,154,10,0,0,0,48,178,103,0,0,0,0,103,178,48,0,0,0,107,255,255,107,0,0,0,48,178,103,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,19,255,255,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,0,0,0,0,234,255,238,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,0,0,0,0,168,255,255,210,117,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,0,0,0,26,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,93,0,0,0,0,0,0,0,0,0,0,0,0,43,204,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,102,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,46,93,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_battery_full_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_battery_full_40_pixels};
static const uint8_t nova_toggle_right_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,107,187,218,221,221,221,221,218,187,106,7,0,0,0,0,0,38,218,237,147,106,102,102,102,102,107,148,237,216,38,0,0,0,19,224,185,17,0,0,0,0,0,0,0,0,18,186,223,18,0,0,142,218,11,0,0,0,0,0,63,186,210,138,10,12,221,140,0,2,233,104,0,0,0,0,0,45,249,161,115,226,166,0,106,232,1,23,255,47,0,0,0,0,0,137,196,0,0,69,252,12,48,255,22,23,255,47,0,0,0,0,0,137,196,0,0,69,252,12,49,255,22,2,233,105,0,0,0,0,0,45,249,162,115,226,165,0,107,231,1,0,141,220,11,0,0,0,0,0,62,186,210,137,10,12,222,139,0,0,19,223,186,18,0,0,0,0,0,0,0,0,19,187,223,18,0,0,0,38,215,238,149,107,102,102,102,102,107,150,238,215,37,0,0,0,0,0,7,105,186,217,221,221,221,221,217,185,104,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_toggle_right_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_toggle_right_18_pixels};
static const uint8_t nova_toggle_right_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,104,175,204,221,221,221,221,221,221,204,174,104,17,0,0,0,0,0,0,0,0,0,104,238,255,252,230,221,221,221,221,221,221,230,252,255,237,102,0,0,0,0,0,0,0,144,255,227,103,18,0,0,0,0,0,0,0,0,18,104,228,255,142,0,0,0,0,0,105,255,197,19,0,0,0,0,0,0,0,0,0,0,0,0,20,198,255,102,0,0,0,17,238,228,19,0,0,0,0,0,0,0,8,124,191,191,124,8,0,20,229,237,16,0,0,105,255,103,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,105,255,103,0,0,174,252,18,0,0,0,0,0,0,0,125,255,139,3,3,140,255,124,0,19,253,172,0,0,205,229,0,0,0,0,0,0,0,0,191,241,3,0,0,3,241,191,0,0,231,204,0,0,204,230,0,0,0,0,0,0,0,0,191,241,3,0,0,3,241,191,0,0,232,203,0,0,174,252,18,0,0,0,0,0,0,0,124,255,140,3,3,140,255,123,0,19,253,172,0,0,104,255,104,0,0,0,0,0,0,0,8,206,255,241,241,255,206,8,0,106,255,102,0,0,16,237,228,20,0,0,0,0,0,0,0,8,123,191,191,123,8,0,21,230,236,16,0,0,0,102,255,198,20,0,0,0,0,0,0,0,0,0,0,0,0,21,199,255,100,0,0,0,0,0,142,255,229,105,19,0,0,0,0,0,0,0,0,20,106,230,255,141,0,0,0,0,0,0,0,101,237,255,253,230,221,221,221,221,221,221,230,253,255,236,100,0,0,0,0,0,0,0,0,0,16,103,172,204,221,221,221,221,221,221,204,172,102,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_toggle_right_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_toggle_right_24_pixels};
static const uint8_t nova_toggle_right_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,17,17,17,17,17,17,17,17,17,17,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,63,142,211,240,255,255,255,255,255,255,255,255,255,255,255,255,239,210,141,61,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,212,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,210,75,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,177,255,255,255,255,253,229,204,187,187,187,187,187,187,187,187,187,187,203,229,253,255,255,255,255,176,13,0,0,0,0,0,0,0,0,0,0,0,30,211,255,255,255,208,101,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,102,209,255,255,255,210,29,0,0,0,0,0,0,0,0,0,13,211,255,255,242,95,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,97,243,255,255,209,12,0,0,0,0,0,0,0,0,177,255,255,225,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,226,255,255,174,0,0,0,0,0,0,0,78,255,255,243,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,56,94,94,55,2,0,0,0,0,42,241,255,255,74,0,0,0,0,0,3,211,255,255,95,0,0,0,0,0,0,0,0,0,0,0,0,0,0,90,225,255,255,255,255,224,88,0,0,0,0,98,255,255,210,2,0,0,0,0,64,255,255,209,2,0,0,0,0,0,0,0,0,0,0,0,0,0,136,255,255,255,255,255,255,255,255,133,0,0,0,2,207,255,255,61,0,0,0,0,143,255,255,100,0,0,0,0,0,0,0,0,0,0,0,0,0,90,255,255,255,195,120,120,196,255,255,255,87,0,0,0,103,255,255,140,0,0,0,0,210,255,253,23,0,0,0,0,0,0,0,0,0,0,0,0,2,225,255,255,119,0,0,0,0,122,255,255,223,2,0,0,25,253,255,209,0,0,0,0,240,255,227,0,0,0,0,0,0,0,0,0,0,0,0,0,56,255,255,195,0,0,0,0,0,0,197,255,255,54,0,0,0,230,255,238,0,0,0,10,255,255,203,0,0,0,0,0,0,0,0,0,0,0,0,0,94,255,255,120,0,0,0,0,0,0,122,255,255,93,0,0,0,204,255,255,9,0,0,9,255,255,204,0,0,0,0,0,0,0,0,0,0,0,0,0,94,255,255,121,0,0,0,0,0,0,122,255,255,93,0,0,0,205,255,255,9,0,0,0,240,255,229,0,0,0,0,0,0,0,0,0,0,0,0,0,55,255,255,196,0,0,0,0,0,0,198,255,255,53,0,0,0,231,255,237,0,0,0,0,210,255,253,24,0,0,0,0,0,0,0,0,0,0,0,0,2,224,255,255,122,0,0,0,0,125,255,255,222,2,0,0,26,253,255,208,0,0,0,0,141,255,255,102,0,0,0,0,0,0,0,0,0,0,0,0,0,88,255,255,255,197,121,122,198,255,255,255,85,0,0,0,105,255,255,138,0,0,0,0,62,255,255,210,2,0,0,0,0,0,0,0,0,0,0,0,0,0,133,255,255,255,255,255,255,255,255,131,0,0,0,3,212,255,255,59,0,0,0,0,3,211,255,255,97,0,0,0,0,0,0,0,0,0,0,0,0,0,0,87,223,255,255,255,255,222,85,0,0,0,0,100,255,255,209,2,0,0,0,0,0,76,255,255,243,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,53,93,93,52,2,0,0,0,0,47,244,255,255,71,0,0,0,0,0,0,0,175,255,255,226,46,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,47,227,255,255,171,0,0,0,0,0,0,0,0,13,210,255,255,243,98,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,100,244,255,255,208,12,0,0,0,0,0,0,0,0,0,29,209,255,255,255,210,103,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,104,212,255,255,255,208,28,0,0,0,0,0,0,0,0,0,0,0,12,173,255,255,255,255,253,229,204,187,187,187,187,187,187,187,187,187,187,204,230,253,255,255,255,255,172,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,74,210,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,209,72,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,61,140,209,238,255,255,255,255,255,255,255,255,255,255,255,255,238,208,139,60,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,17,17,17,17,17,17,17,17,17,17,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_toggle_right_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_toggle_right_40_pixels};
static const uint8_t nova_paintbrush_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,148,254,248,100,0,0,0,0,0,0,0,0,0,0,0,0,3,163,239,74,120,248,9,0,0,0,0,0,0,45,185,212,125,6,163,239,53,0,76,254,15,0,0,0,0,0,38,240,178,118,239,224,239,53,0,53,239,147,0,0,0,0,0,0,91,253,87,0,49,159,50,0,53,239,163,3,0,0,0,0,0,0,132,251,242,54,0,0,0,50,239,163,3,0,0,0,0,0,45,173,249,82,162,240,54,0,0,159,224,6,0,0,0,1,120,218,255,200,50,0,3,162,240,54,0,49,239,124,0,0,0,4,247,183,47,0,0,0,0,3,162,240,54,0,119,211,0,0,0,0,122,248,107,9,0,0,0,0,3,162,242,87,179,182,0,0,0,0,1,210,255,247,186,116,46,1,0,3,228,254,240,44,0,0,0,0,0,9,206,219,155,220,255,231,163,129,252,133,38,0,0,0,0,0,0,0,27,229,165,1,35,104,180,253,210,0,0,0,0,0,0,0,0,0,0,55,247,121,0,15,203,211,16,0,0,0,0,0,0,0,0,0,0,0,92,252,108,211,218,25,0,0,0,0,0,0,0,0,0,0,0,0,0,125,252,190,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_paintbrush_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_paintbrush_18_pixels};
static const uint8_t nova_paintbrush_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,115,198,186,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,175,255,237,246,255,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,175,255,173,8,32,246,185,0,0,0,0,0,0,0,0,0,2,109,194,194,110,2,5,175,255,173,5,0,8,238,197,0,0,0,0,0,0,0,0,6,175,255,240,240,255,176,175,255,173,5,0,5,173,255,113,0,0,0,0,0,0,0,0,112,255,173,10,10,172,255,255,173,5,0,5,173,255,175,3,0,0,0,0,0,0,0,0,111,255,219,6,0,5,109,110,5,0,5,173,255,175,5,0,0,0,0,0,0,0,0,0,135,255,255,181,7,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,9,148,255,204,176,255,176,6,0,0,0,110,255,175,5,0,0,0,0,0,0,2,46,127,227,255,195,24,5,173,255,176,6,0,0,110,255,176,2,0,0,0,0,0,79,236,255,255,231,119,3,0,0,5,173,255,176,6,0,5,172,255,110,0,0,0,0,0,182,255,167,78,5,0,0,0,0,0,5,173,255,176,6,0,10,239,195,0,0,0,0,0,87,255,201,8,0,0,0,0,0,0,0,5,173,255,176,6,10,240,194,0,0,0,0,0,0,142,255,232,120,51,2,0,0,0,0,0,5,173,255,219,173,255,109,0,0,0,0,0,0,3,226,255,255,255,234,167,97,27,0,0,0,16,251,255,255,175,2,0,0,0,0,0,0,0,16,218,254,172,204,254,255,255,221,153,85,134,255,174,111,6,0,0,0,0,0,0,0,0,0,38,238,242,45,28,98,167,235,255,255,255,226,9,0,0,0,0,0,0,0,0,0,0,0,0,69,251,223,21,0,0,2,74,239,253,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,255,190,5,0,25,218,254,96,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,153,255,149,46,224,255,115,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,195,255,249,249,101,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,151,175,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_paintbrush_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_paintbrush_24_pixels};
static const uint8_t nova_paintbrush_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,192,243,251,208,97,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,122,252,255,255,255,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,225,210,255,255,255,97,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,133,3,0,89,255,255,207,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,212,255,250,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,186,239,255,223,157,30,0,0,0,128,255,255,255,127,0,0,0,0,3,227,255,242,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,254,255,255,255,255,255,236,59,0,128,255,255,255,127,0,0,0,0,0,133,255,255,190,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,131,255,255,255,230,201,245,255,255,242,158,255,255,255,127,0,0,0,0,0,127,255,255,252,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79,255,255,255,132,3,0,22,197,255,255,255,255,255,127,0,0,0,0,0,127,255,255,255,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,255,255,157,0,0,0,0,13,196,255,255,255,127,0,0,0,0,0,127,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,255,255,254,87,0,0,0,0,10,116,173,62,0,0,0,0,0,127,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,246,255,255,247,58,0,0,0,0,0,0,0,0,0,0,0,127,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,186,255,255,255,255,244,63,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,190,255,255,245,207,255,255,242,58,0,0,0,0,0,0,0,76,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,87,229,255,255,248,69,13,197,255,255,242,58,0,0,0,0,0,0,161,255,255,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,114,214,255,255,255,237,68,0,0,13,197,255,255,242,58,0,0,0,0,0,113,255,255,242,59,0,0,0,0,0,0,0,0,0,0,3,89,166,227,255,255,255,255,255,191,34,0,0,0,0,13,197,255,255,242,58,0,0,0,0,9,196,255,255,236,30,0,0,0,0,0,0,0,0,0,159,255,255,255,255,255,255,207,86,1,0,0,0,0,0,0,13,197,255,255,242,58,0,0,0,0,13,196,255,255,157,0,0,0,0,0,0,0,0,3,251,255,255,250,199,137,52,0,0,0,0,0,0,0,0,0,0,13,197,255,255,242,58,0,0,0,0,21,245,255,222,0,0,0,0,0,0,0,0,2,221,255,255,144,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,197,255,255,242,58,0,0,0,0,202,255,254,12,0,0,0,0,0,0,0,0,78,253,255,255,94,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,197,255,255,242,58,0,0,3,232,255,237,1,0,0,0,0,0,0,0,0,0,123,255,255,251,105,5,0,0,0,0,0,0,0,0,0,0,0,0,0,13,197,255,255,247,86,0,131,255,255,184,0,0,0,0,0,0,0,0,0,0,1,167,255,255,255,244,177,108,38,0,0,0,0,0,0,0,0,0,0,0,13,197,255,255,254,157,255,255,254,67,0,0,0,0,0,0,0,0,0,0,0,9,239,255,255,255,255,255,255,224,154,85,18,0,0,0,0,0,0,0,0,13,247,255,255,255,255,255,125,0,0,0,0,0,0,0,0,0,0,0,0,0,40,240,255,255,255,255,255,255,255,255,252,201,131,62,5,0,0,0,0,63,255,255,255,255,255,130,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,51,245,255,255,185,187,250,255,255,255,255,255,255,245,187,119,51,2,197,255,255,172,165,74,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,92,255,255,254,82,18,84,154,224,255,255,255,255,255,255,255,238,255,255,224,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,137,255,255,246,51,0,0,0,38,108,177,242,255,255,255,255,255,255,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,179,255,255,224,22,0,0,0,0,0,5,61,148,250,255,255,210,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,213,255,255,195,6,0,0,0,0,0,27,222,255,255,215,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,234,255,255,160,0,0,0,0,20,213,255,255,220,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,249,255,255,111,0,0,35,213,255,255,228,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,106,255,255,252,74,61,234,255,255,235,42,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,255,255,243,251,255,255,220,42,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,190,255,255,255,255,190,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,156,245,237,139,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_paintbrush_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_paintbrush_40_pixels};
static const uint8_t nova_shield_check_18_pixels[] = {0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,99,237,237,98,0,0,0,0,0,0,0,0,0,0,0,4,73,192,250,116,116,250,190,72,4,0,0,0,0,0,0,43,198,249,250,173,42,0,0,43,174,251,249,198,43,0,0,0,0,149,198,73,20,0,0,0,0,0,0,20,73,198,149,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,162,162,0,0,0,0,0,0,1,15,0,0,162,162,0,0,0,0,162,162,0,0,0,0,0,3,162,203,0,0,162,162,0,0,0,0,162,162,0,0,116,52,3,163,239,53,0,0,162,162,0,0,0,0,162,163,0,0,153,239,179,239,53,0,0,0,163,162,0,0,0,0,145,179,0,0,3,162,239,53,0,0,0,0,180,145,0,0,0,0,106,230,1,0,0,1,15,0,0,0,0,1,230,104,0,0,0,0,28,250,83,0,0,0,0,0,0,0,0,84,249,28,0,0,0,0,0,142,233,35,0,0,0,0,0,0,36,233,141,0,0,0,0,0,0,6,187,233,78,0,0,0,0,79,234,187,6,0,0,0,0,0,0,0,6,143,253,202,89,87,202,253,142,6,0,0,0,0,0,0,0,0,0,0,41,152,245,246,153,41,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,9,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_shield_check_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_shield_check_18_pixels};
static const uint8_t nova_shield_check_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,170,189,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,127,251,248,248,251,125,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,112,225,255,214,45,45,215,255,224,112,25,0,0,0,0,0,0,0,0,0,42,188,235,255,255,240,121,9,0,0,10,122,240,255,255,235,187,41,0,0,0,0,0,0,188,251,200,163,91,11,0,0,0,0,0,0,12,92,164,200,251,186,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,5,156,138,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,5,174,255,157,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,138,156,5,5,174,255,174,5,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,157,255,174,174,255,174,5,0,0,0,0,217,217,0,0,0,0,0,0,206,228,0,0,0,5,174,255,255,174,5,0,0,0,0,0,230,205,0,0,0,0,0,0,183,250,5,0,0,0,5,156,156,5,0,0,0,0,0,6,250,182,0,0,0,0,0,0,130,255,65,0,0,0,0,0,0,0,0,0,0,0,0,67,255,128,0,0,0,0,0,0,50,255,172,0,0,0,0,0,0,0,0,0,0,0,0,173,255,49,0,0,0,0,0,0,0,182,254,78,0,0,0,0,0,0,0,0,0,0,79,254,179,0,0,0,0,0,0,0,0,35,237,242,69,0,0,0,0,0,0,0,0,70,243,236,33,0,0,0,0,0,0,0,0,0,64,241,250,130,8,0,0,0,0,8,132,251,241,63,0,0,0,0,0,0,0,0,0,0,0,46,214,255,228,113,17,16,111,228,255,213,45,0,0,0,0,0,0,0,0,0,0,0,0,0,7,121,236,255,247,246,255,236,121,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,101,180,181,103,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_shield_check_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_shield_check_24_pixels};
static const uint8_t nova_shield_check_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,172,250,249,171,29,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,236,255,255,255,255,236,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,151,254,255,255,234,235,255,255,254,149,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,125,241,255,255,255,186,22,23,187,255,255,255,241,124,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,47,99,181,250,255,255,255,248,122,2,0,0,2,124,249,255,255,255,250,180,98,47,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,105,234,255,255,255,255,255,255,253,165,33,0,0,0,0,0,0,34,165,253,255,255,255,255,255,255,233,103,0,0,0,0,0,0,0,0,0,0,0,78,255,255,255,255,255,255,233,148,48,0,0,0,0,0,0,0,0,0,0,48,149,234,255,255,255,255,255,255,75,0,0,0,0,0,0,0,0,0,0,166,255,255,196,167,129,70,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,71,130,168,197,255,255,165,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,154,154,10,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,14,199,255,255,92,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,15,199,255,255,241,37,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,70,70,0,0,0,0,15,199,255,255,241,57,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,70,255,255,127,0,0,15,199,255,255,241,57,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,70,255,255,255,128,15,199,255,255,241,57,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,190,255,255,23,0,0,0,0,0,128,255,255,255,227,255,255,241,57,0,0,0,0,0,0,0,0,23,255,255,190,0,0,0,0,0,0,0,0,0,0,172,255,255,40,0,0,0,0,0,0,128,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,42,255,255,170,0,0,0,0,0,0,0,0,0,0,149,255,255,63,0,0,0,0,0,0,0,128,255,255,241,57,0,0,0,0,0,0,0,0,0,0,65,255,255,146,0,0,0,0,0,0,0,0,0,0,125,255,255,97,0,0,0,0,0,0,0,0,106,178,52,0,0,0,0,0,0,0,0,0,0,0,99,255,255,124,0,0,0,0,0,0,0,0,0,0,70,255,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,172,255,255,67,0,0,0,0,0,0,0,0,0,0,7,243,255,239,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,241,255,242,6,0,0,0,0,0,0,0,0,0,0,0,173,255,255,117,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,171,0,0,0,0,0,0,0,0,0,0,0,0,53,254,255,237,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,238,255,254,51,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,196,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,192,255,255,167,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,235,255,255,162,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,163,255,255,234,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65,251,255,255,186,16,0,0,0,0,0,0,0,0,0,0,0,0,16,187,255,255,250,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,250,255,255,227,77,0,0,0,0,0,0,0,0,0,0,77,228,255,255,250,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,241,255,255,255,178,52,0,0,0,0,0,0,51,178,255,255,255,241,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,196,255,255,255,255,184,82,5,4,79,183,254,255,255,255,195,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,107,235,255,255,255,255,233,229,255,255,255,255,235,107,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,117,232,255,255,255,255,255,255,233,119,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,83,182,249,251,188,87,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_shield_check_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_shield_check_40_pixels};
static const uint8_t nova_badge_check_18_pixels[] = {0,0,0,0,0,0,0,0,19,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,199,255,255,199,44,0,0,0,0,0,0,0,0,0,20,140,198,240,179,57,57,181,240,198,140,20,0,0,0,0,0,20,221,216,128,117,6,0,0,6,117,128,217,221,20,0,0,0,0,140,216,12,0,0,0,0,0,0,0,0,12,217,139,0,0,0,0,199,128,0,0,0,0,0,0,0,0,0,0,128,199,0,0,0,46,239,117,0,0,0,0,0,0,1,15,0,0,117,239,43,0,0,202,178,6,0,0,0,0,0,3,162,203,0,0,6,181,197,0,22,255,54,0,0,0,116,52,3,163,239,53,0,0,0,58,255,17,22,255,54,0,0,0,153,239,179,239,53,0,0,0,0,59,255,17,0,201,179,6,0,0,3,162,239,53,0,0,0,0,6,182,196,0,0,45,239,117,0,0,0,1,15,0,0,0,0,0,117,238,42,0,0,0,199,128,0,0,0,0,0,0,0,0,0,0,130,197,0,0,0,0,139,217,12,0,0,0,0,0,0,0,0,12,219,137,0,0,0,0,19,220,218,129,117,6,0,0,6,117,129,218,220,19,0,0,0,0,0,19,137,198,239,180,56,56,181,240,198,137,19,0,0,0,0,0,0,0,0,0,43,199,255,255,199,43,0,0,0,0,0,0,0,0,0,0,0,0,0,0,20,20,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_badge_check_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_badge_check_18_pixels};
static const uint8_t nova_badge_check_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,147,200,200,146,37,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,88,246,255,235,235,255,245,86,14,0,0,0,0,0,0,0,0,0,0,0,7,129,231,255,255,231,68,0,0,70,232,255,255,231,129,7,0,0,0,0,0,0,0,7,194,255,224,161,169,51,0,0,0,0,52,169,161,224,255,194,7,0,0,0,0,0,0,128,255,153,4,0,0,0,0,0,0,0,0,0,0,4,153,255,128,0,0,0,0,0,0,229,224,4,0,0,0,0,0,0,0,0,0,0,0,0,4,225,229,0,0,0,0,0,15,255,164,0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,255,13,0,0,0,0,90,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,85,0,0,0,41,247,230,50,0,0,0,0,0,0,0,0,5,156,138,0,0,0,52,232,245,37,0,0,151,255,65,0,0,0,0,0,0,0,0,5,174,255,157,0,0,0,0,70,255,145,0,0,204,231,0,0,0,0,0,138,156,5,5,174,255,174,5,0,0,0,0,0,236,199,0,0,204,231,0,0,0,0,0,157,255,174,174,255,174,5,0,0,0,0,0,1,236,199,0,0,150,255,66,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,71,255,144,0,0,40,247,230,51,0,0,0,0,5,156,156,5,0,0,0,0,0,0,52,233,245,36,0,0,0,88,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,84,0,0,0,0,15,255,164,0,0,0,0,0,0,0,0,0,0,0,0,0,0,165,255,13,0,0,0,0,0,229,224,4,0,0,0,0,0,0,0,0,0,0,0,0,5,225,228,0,0,0,0,0,0,127,255,154,4,0,0,0,0,0,0,0,0,0,0,4,154,255,126,0,0,0,0,0,0,6,193,255,225,164,169,52,0,0,0,0,51,168,164,225,255,193,6,0,0,0,0,0,0,0,6,127,229,255,255,232,69,0,0,69,232,255,255,228,126,6,0,0,0,0,0,0,0,0,0,0,0,13,86,246,255,234,234,255,246,87,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,147,201,201,147,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_badge_check_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_badge_check_24_pixels};
static const uint8_t nova_badge_check_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,162,228,253,253,227,162,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,163,255,255,255,255,255,255,255,255,162,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,30,29,205,255,255,255,245,209,209,245,255,255,255,205,29,30,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,87,193,248,255,255,255,255,243,109,12,0,0,12,109,244,255,255,255,255,248,193,87,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,198,255,255,255,255,255,255,244,49,0,0,0,0,0,0,50,244,255,255,255,255,255,255,198,28,0,0,0,0,0,0,0,0,0,0,0,0,0,28,230,255,255,255,228,182,191,199,73,0,0,0,0,0,0,0,0,73,199,190,181,228,255,255,255,230,27,0,0,0,0,0,0,0,0,0,0,0,4,199,255,255,228,76,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,76,228,255,255,197,4,0,0,0,0,0,0,0,0,0,0,87,255,255,228,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,228,255,255,86,0,0,0,0,0,0,0,0,0,0,193,255,255,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,78,255,255,192,0,0,0,0,0,0,0,0,0,2,246,255,229,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,229,255,246,2,0,0,0,0,0,0,0,0,30,255,255,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,184,255,255,29,0,0,0,0,0,0,0,0,29,255,255,190,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,255,255,27,0,0,0,0,0,0,0,16,211,255,255,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,201,255,255,204,13,0,0,0,0,0,0,171,255,255,242,69,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,154,154,10,0,0,0,0,0,72,244,255,255,161,0,0,0,0,0,67,255,255,241,45,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,199,255,255,92,0,0,0,0,0,0,50,244,255,255,58,0,0,0,0,169,255,255,102,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,199,255,255,241,37,0,0,0,0,0,0,0,111,255,255,160,0,0,0,0,233,255,241,9,0,0,0,0,0,0,0,0,70,70,0,0,0,0,15,199,255,255,241,57,0,0,0,0,0,0,0,0,13,246,255,226,0,0,0,10,255,255,203,0,0,0,0,0,0,0,0,70,255,255,127,0,0,15,199,255,255,241,57,0,0,0,0,0,0,0,0,0,0,210,255,253,7,0,0,10,255,255,203,0,0,0,0,0,0,0,0,70,255,255,255,128,15,199,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,210,255,253,7,0,0,0,233,255,242,9,0,0,0,0,0,0,0,0,128,255,255,255,227,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,13,246,255,225,0,0,0,0,168,255,255,103,0,0,0,0,0,0,0,0,0,128,255,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,111,255,255,160,0,0,0,0,66,255,255,241,46,0,0,0,0,0,0,0,0,0,128,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,51,244,255,255,58,0,0,0,0,0,169,255,255,242,69,0,0,0,0,0,0,0,0,0,106,178,52,0,0,0,0,0,0,0,0,0,0,0,0,72,244,255,255,160,0,0,0,0,0,0,16,210,255,255,200,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,201,255,255,204,12,0,0,0,0,0,0,0,30,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,190,255,255,27,0,0,0,0,0,0,0,0,30,255,255,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,185,255,255,29,0,0,0,0,0,0,0,0,2,246,255,228,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,229,255,245,2,0,0,0,0,0,0,0,0,0,192,255,255,77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,79,255,255,191,0,0,0,0,0,0,0,0,0,0,86,255,255,228,26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,229,255,255,85,0,0,0,0,0,0,0,0,0,0,4,198,255,255,228,78,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,79,229,255,255,196,4,0,0,0,0,0,0,0,0,0,0,0,27,230,255,255,255,228,183,192,199,72,0,0,0,0,0,0,0,0,70,198,192,185,229,255,255,255,230,27,0,0,0,0,0,0,0,0,0,0,0,0,0,27,197,255,255,255,255,255,255,243,48,0,0,0,0,0,0,47,242,255,255,255,255,255,255,196,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,85,192,246,255,255,255,255,243,108,11,0,0,11,106,242,255,255,255,255,245,191,84,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,29,28,206,255,255,255,244,207,207,244,255,255,255,207,28,28,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,164,255,255,255,255,255,255,255,255,166,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,61,163,229,254,254,229,163,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_badge_check_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_badge_check_40_pixels};
static const uint8_t nova_sparkles_18_pixels[] = {0,0,0,0,0,0,0,0,7,7,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,29,236,236,29,0,0,0,151,151,0,0,0,0,0,0,0,0,0,117,216,216,117,0,8,154,224,224,154,8,0,0,0,0,0,0,0,166,164,165,165,0,8,154,224,224,154,8,0,0,0,0,0,0,0,215,116,117,214,0,0,0,151,151,0,0,0,0,0,0,0,0,10,252,67,68,252,10,0,0,8,8,0,0,0,0,0,0,0,10,112,243,16,16,243,112,10,0,0,0,0,0,0,29,117,166,214,252,243,100,0,0,102,243,252,214,166,117,29,0,7,236,216,165,117,68,16,0,0,0,0,16,68,117,165,216,235,7,7,235,216,165,117,68,16,0,0,0,0,16,68,117,165,216,235,7,0,29,117,166,214,252,243,102,0,0,103,243,252,214,166,117,28,0,0,0,0,0,0,10,111,243,16,16,244,111,10,0,0,0,0,0,0,0,12,12,0,0,9,252,68,69,252,10,0,0,0,0,0,0,0,112,251,251,112,0,0,214,117,118,213,0,0,0,0,0,0,0,12,251,109,110,251,12,0,165,165,166,164,0,0,0,0,0,0,0,12,251,109,111,250,12,0,117,216,216,117,0,0,0,0,0,0,0,0,111,251,251,110,0,0,29,235,234,29,0,0,0,0,0,0,0,0,0,12,12,0,0,0,0,7,7,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_sparkles_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_sparkles_18_pixels};
static const uint8_t nova_sparkles_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,172,172,35,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,180,251,251,180,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,4,247,190,191,247,4,0,0,141,221,250,250,219,138,0,0,0,0,0,0,0,0,0,0,44,255,142,143,255,43,0,0,141,221,250,250,219,138,0,0,0,0,0,0,0,0,0,0,92,255,94,95,255,91,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,140,255,46,47,255,139,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,189,248,5,5,248,188,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,241,201,0,0,201,241,21,0,0,0,0,0,0,0,0,0,0,0,4,43,91,139,187,242,255,94,0,0,96,255,241,187,139,91,43,4,0,0,0,0,35,180,246,255,255,255,249,200,93,0,0,0,0,95,200,249,255,255,255,246,180,35,0,0,172,251,191,143,95,47,5,0,0,0,0,0,0,0,0,5,47,95,143,191,251,171,0,0,172,251,191,143,95,47,5,0,0,0,0,0,0,0,0,5,47,95,143,191,251,172,0,0,35,180,246,255,255,255,249,201,95,0,0,0,0,97,201,249,255,255,255,246,180,34,0,0,0,0,4,43,91,139,187,241,255,95,0,0,97,255,241,187,139,91,43,4,0,0,0,0,0,0,0,0,0,0,0,21,241,201,0,0,201,241,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,188,249,6,6,249,187,0,0,0,0,0,0,0,0,0,0,0,94,192,192,93,0,0,0,139,255,47,48,255,138,0,0,0,0,0,0,0,0,0,0,94,255,242,242,255,92,0,0,91,255,95,96,255,90,0,0,0,0,0,0,0,0,0,0,192,242,21,22,243,191,0,0,43,255,143,144,255,42,0,0,0,0,0,0,0,0,0,0,192,242,22,23,243,191,0,0,3,245,191,192,246,4,0,0,0,0,0,0,0,0,0,0,92,255,243,243,255,90,0,0,0,180,251,251,180,0,0,0,0,0,0,0,0,0,0,0,0,92,191,191,90,0,0,0,0,35,172,171,34,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_sparkles_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_sparkles_24_pixels};
static const uint8_t nova_sparkles_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,5,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,166,250,250,165,12,0,0,0,0,0,0,0,0,0,186,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,164,255,255,255,255,162,0,0,0,0,0,0,0,0,19,255,255,189,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,249,255,242,242,255,249,12,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,255,255,165,166,255,255,59,0,0,0,0,10,154,187,193,255,255,238,187,187,103,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,117,118,255,255,107,0,0,0,0,92,255,255,255,255,255,255,255,255,253,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,156,255,255,68,69,255,255,155,0,0,0,0,37,234,255,255,255,255,255,255,255,186,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,205,255,255,20,19,255,255,204,0,0,0,0,0,6,17,38,255,255,195,17,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,247,255,227,0,0,228,255,248,5,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,255,255,178,0,0,179,255,255,45,0,0,0,0,0,0,5,235,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,95,255,255,130,0,0,131,255,255,94,0,0,0,0,0,0,0,36,93,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,142,255,255,82,0,0,83,255,255,141,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,255,255,34,0,0,34,255,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,251,255,227,1,0,0,1,228,255,251,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,45,93,142,192,251,255,255,130,0,0,0,0,132,255,255,251,192,142,93,45,4,0,0,0,0,0,0,0,0,0,0,0,0,0,12,59,107,155,204,248,255,255,255,255,255,255,197,7,0,0,0,0,7,198,255,255,255,255,255,255,248,204,155,107,59,12,0,0,0,0,0,0,13,165,249,255,255,255,255,255,255,255,255,255,227,130,7,0,0,0,0,0,0,7,131,227,255,255,255,255,255,255,255,255,255,249,163,13,0,0,0,0,166,255,255,255,255,255,255,228,179,131,83,34,1,0,0,0,0,0,0,0,0,0,0,1,34,83,131,179,228,255,255,255,255,255,255,165,0,0,0,6,250,255,242,166,118,69,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,69,118,166,242,255,249,6,0,0,6,250,255,242,166,118,69,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,21,69,118,166,243,255,249,6,0,0,0,164,255,255,255,255,255,255,228,179,131,83,34,1,0,0,0,0,0,0,0,0,0,0,1,34,83,131,179,228,255,255,255,255,255,255,163,0,0,0,0,12,162,249,255,255,255,255,255,255,255,255,255,228,132,7,0,0,0,0,0,0,8,133,228,255,255,255,255,255,255,255,255,255,249,161,12,0,0,0,0,0,0,12,59,107,155,204,248,255,255,255,255,255,255,198,7,0,0,0,0,8,200,255,255,255,255,255,255,248,204,155,107,59,12,0,0,0,0,0,0,0,0,0,0,0,0,0,4,45,93,142,192,251,255,255,131,0,0,0,0,133,255,255,250,192,142,93,45,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57,251,255,227,1,0,0,1,228,255,250,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,255,255,34,0,0,34,255,255,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,141,255,255,83,0,0,84,255,255,140,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,47,92,64,13,0,0,0,0,0,0,94,255,255,131,0,0,132,255,255,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,205,255,255,255,241,102,0,0,0,0,0,45,255,255,179,0,0,180,255,255,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27,238,255,255,255,255,255,255,97,0,0,0,0,4,247,255,228,0,0,229,255,248,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,169,255,255,209,121,169,255,255,240,12,0,0,0,0,204,255,255,22,21,255,255,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,234,255,237,16,0,0,170,255,255,63,0,0,0,0,155,255,255,69,70,255,255,154,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,254,255,205,0,0,0,121,255,255,92,0,0,0,0,107,255,255,118,119,255,255,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,251,60,0,17,212,255,255,46,0,0,0,0,59,255,255,166,167,255,255,58,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,255,251,206,238,255,255,205,3,0,0,0,0,12,249,255,242,243,255,249,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,195,255,255,255,255,255,238,44,0,0,0,0,0,0,163,255,255,255,255,161,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,125,216,254,233,167,27,0,0,0,0,0,0,0,13,165,249,249,163,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,0,0,0,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_sparkles_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_sparkles_40_pixels};
static const uint8_t nova_chevron_right_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,204,162,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,239,152,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,53,239,152,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,50,237,168,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,204,162,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_chevron_right_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_chevron_right_18_pixels};
static const uint8_t nova_chevron_right_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,157,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_chevron_right_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_chevron_right_24_pixels};
static const uint8_t nova_chevron_right_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_chevron_right_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_chevron_right_40_pixels};
static const uint8_t nova_refresh_cw_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,54,87,87,55,3,0,0,0,62,17,0,0,0,0,0,8,121,233,255,239,239,255,237,134,16,0,224,96,0,0,0,0,24,202,234,113,24,0,0,22,105,225,223,47,226,99,0,0,0,9,202,206,25,0,0,0,0,0,0,12,173,239,232,99,0,0,0,121,234,25,0,0,0,0,0,0,77,170,171,224,254,124,0,0,3,233,113,0,0,0,0,0,0,0,77,170,170,171,210,72,0,0,55,255,25,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,87,238,0,0,0,0,0,0,0,0,0,0,0,0,116,39,0,0,39,116,0,0,0,0,0,0,0,0,0,0,0,0,241,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27,255,52,0,0,72,210,171,170,170,77,0,0,0,0,0,0,0,116,231,2,0,0,124,254,224,171,170,77,0,0,0,0,0,0,27,236,117,0,0,0,99,232,239,173,12,0,0,0,0,0,0,27,207,199,7,0,0,0,99,226,48,223,224,105,23,0,0,27,116,236,199,22,0,0,0,0,96,224,0,16,134,236,255,240,240,255,231,117,7,0,0,0,0,0,17,62,0,0,0,3,54,86,86,53,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_refresh_cw_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_refresh_cw_18_pixels};
static const uint8_t nova_refresh_cw_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,126,179,204,205,179,130,53,1,0,0,0,138,138,0,0,0,0,0,0,0,0,45,184,255,255,252,230,229,251,255,255,200,66,0,0,217,217,0,0,0,0,0,0,0,90,247,254,170,70,6,0,0,4,66,156,250,254,136,3,217,217,0,0,0,0,0,0,90,254,231,60,0,0,0,0,0,0,0,0,37,204,255,174,218,217,0,0,0,0,0,45,247,231,32,0,0,0,0,0,0,0,0,0,0,9,175,255,243,218,0,0,0,0,0,184,254,60,0,0,0,0,0,0,0,0,0,141,221,221,222,244,255,240,0,0,0,0,46,255,170,0,0,0,0,0,0,0,0,0,0,141,221,221,221,222,242,203,0,0,0,0,127,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,179,252,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,205,229,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,231,203,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,252,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,74,255,122,0,0,0,0,203,242,222,221,221,219,138,0,0,0,0,0,0,0,0,0,0,173,255,44,0,0,0,0,240,255,244,222,221,219,138,0,0,0,0,0,0,0,0,0,63,254,181,0,0,0,0,0,218,243,255,175,9,0,0,0,0,0,0,0,0,0,0,32,232,246,42,0,0,0,0,0,217,218,174,255,205,38,0,0,0,0,0,0,0,0,63,232,254,88,0,0,0,0,0,0,217,217,3,136,254,251,156,66,5,0,0,7,73,173,254,246,89,0,0,0,0,0,0,0,217,217,0,0,65,199,255,255,252,230,230,253,255,255,181,42,0,0,0,0,0,0,0,0,138,138,0,0,0,0,52,128,178,204,204,178,123,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_refresh_cw_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_refresh_cw_24_pixels};
static const uint8_t nova_refresh_cw_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,53,84,102,94,69,43,13,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,103,183,248,255,255,255,255,255,255,251,190,115,35,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,136,246,255,255,255,255,255,255,255,255,255,255,255,255,253,164,36,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,91,237,255,255,255,255,233,167,128,113,119,144,170,228,255,255,255,255,250,148,11,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,2,136,255,255,255,251,154,60,3,0,0,0,0,0,0,1,50,133,240,255,255,255,216,39,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,2,170,255,255,255,177,39,0,0,0,0,0,0,0,0,0,0,0,0,18,136,247,255,255,237,56,107,255,255,107,0,0,0,0,0,0,0,0,0,0,135,255,255,253,121,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,217,255,255,241,140,255,255,107,0,0,0,0,0,0,0,0,0,91,255,255,253,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19,200,255,255,247,255,255,107,0,0,0,0,0,0,0,0,16,236,255,255,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,17,17,17,27,197,255,255,255,255,107,0,0,0,0,0,0,0,0,135,255,255,177,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,252,255,255,255,255,255,255,255,255,255,128,0,0,0,0,0,0,0,23,246,255,251,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,255,255,255,255,255,255,255,255,153,0,0,0,0,0,0,0,104,255,255,153,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,187,187,187,187,187,187,191,228,215,20,0,0,0,0,0,0,0,184,255,255,59,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,248,255,233,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,255,255,167,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,87,255,255,129,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,101,255,255,113,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,114,255,255,100,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,255,255,85,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,255,51,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,235,255,247,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,179,0,0,0,0,0,0,0,20,215,228,191,187,187,187,187,187,187,178,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,255,101,0,0,0,0,0,0,0,153,255,255,255,255,255,255,255,255,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,252,255,244,21,0,0,0,0,0,0,0,128,255,255,255,255,255,255,255,255,255,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,182,255,255,132,0,0,0,0,0,0,0,0,107,255,255,255,255,207,33,17,17,17,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,235,14,0,0,0,0,0,0,0,0,107,255,255,247,255,255,200,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,254,255,254,86,0,0,0,0,0,0,0,0,0,107,255,255,140,241,255,255,228,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,125,254,255,255,132,0,0,0,0,0,0,0,0,0,0,107,255,255,107,56,233,255,255,248,138,19,0,0,0,0,0,0,0,0,0,0,0,0,41,182,255,255,255,168,2,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,29,202,255,255,255,241,134,52,1,0,0,0,0,0,0,3,62,157,251,255,255,254,132,2,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,8,146,250,255,255,255,255,230,167,130,114,114,131,170,235,255,255,255,255,234,85,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,31,153,252,255,255,255,255,255,255,255,255,255,255,255,255,245,136,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,34,113,189,250,255,255,255,255,255,255,247,181,101,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,13,53,83,101,101,83,52,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_refresh_cw_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_refresh_cw_40_pixels};
static const uint8_t nova_rotate_ccw_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,62,0,0,0,4,56,87,87,54,3,0,0,0,0,0,0,0,96,224,0,17,135,237,255,239,239,255,233,120,8,0,0,0,0,0,99,226,48,223,224,104,22,0,0,25,114,234,201,23,0,0,0,0,99,232,239,173,12,0,0,0,0,0,0,26,207,200,7,0,0,0,124,254,224,171,170,77,0,0,0,0,0,0,27,235,118,0,0,0,72,210,171,170,170,77,0,0,0,0,0,0,0,116,232,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,255,53,0,0,39,116,0,0,0,0,0,0,0,0,0,0,0,0,239,86,0,0,86,239,0,0,0,0,0,0,0,0,0,0,0,0,241,85,0,0,54,255,25,0,0,0,0,0,0,0,0,0,0,27,255,52,0,0,3,233,114,0,0,0,0,0,0,0,0,0,0,116,231,2,0,0,0,121,234,26,0,0,0,0,0,0,0,0,27,236,117,0,0,0,0,8,201,207,26,0,0,0,0,0,0,27,207,199,7,0,0,0,0,0,23,200,236,115,26,0,0,27,116,236,199,22,0,0,0,0,0,0,0,7,118,232,255,240,240,255,231,117,7,0,0,0,0,0,0,0,0,0,0,2,53,86,86,53,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_rotate_ccw_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_rotate_ccw_18_pixels};
static const uint8_t nova_rotate_ccw_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,1,53,130,179,205,204,179,125,46,0,0,0,0,0,0,0,0,0,0,217,217,0,0,67,201,255,255,251,229,230,252,255,255,183,44,0,0,0,0,0,0,0,0,217,217,3,137,254,250,155,65,4,0,0,6,71,171,254,246,89,0,0,0,0,0,0,0,217,218,174,255,203,36,0,0,0,0,0,0,0,0,61,231,254,89,0,0,0,0,0,0,218,243,255,175,8,0,0,0,0,0,0,0,0,0,0,32,232,246,43,0,0,0,0,0,240,255,244,222,221,219,138,0,0,0,0,0,0,0,0,0,62,254,181,0,0,0,0,0,203,242,222,221,221,219,138,0,0,0,0,0,0,0,0,0,0,172,255,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,73,255,124,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,252,177,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,230,204,0,0,0,0,204,230,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,231,203,0,0,0,0,179,252,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,252,177,0,0,0,0,125,255,71,0,0,0,0,0,0,0,0,0,0,0,0,0,0,74,255,122,0,0,0,0,46,255,171,0,0,0,0,0,0,0,0,0,0,0,0,0,0,173,255,44,0,0,0,0,0,183,254,61,0,0,0,0,0,0,0,0,0,0,0,0,63,254,180,0,0,0,0,0,0,44,246,231,32,0,0,0,0,0,0,0,0,0,0,32,232,246,42,0,0,0,0,0,0,0,90,254,231,62,0,0,0,0,0,0,0,0,64,232,254,89,0,0,0,0,0,0,0,0,0,89,246,254,172,73,7,0,0,7,73,173,254,246,89,0,0,0,0,0,0,0,0,0,0,0,43,181,255,255,253,230,230,253,255,255,181,42,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,124,178,204,204,178,123,44,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_rotate_ccw_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_rotate_ccw_24_pixels};
static const uint8_t nova_rotate_ccw_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,14,44,69,94,102,84,53,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,35,115,191,251,255,255,255,255,255,255,248,183,103,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,32,153,252,255,255,255,255,255,255,255,255,255,255,255,255,245,136,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,8,148,251,255,255,255,255,228,169,144,119,114,129,168,234,255,255,255,255,235,88,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,29,203,255,255,255,240,132,50,1,0,0,0,0,0,0,3,60,155,251,255,255,254,133,2,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,56,233,255,255,248,136,18,0,0,0,0,0,0,0,0,0,0,0,0,39,179,255,255,255,168,2,0,0,0,0,0,0,0,0,0,0,107,255,255,140,241,255,255,228,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,122,253,255,255,133,0,0,0,0,0,0,0,0,0,0,107,255,255,247,255,255,200,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,87,253,255,254,88,0,0,0,0,0,0,0,0,0,107,255,255,255,255,202,30,17,17,17,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,123,255,255,235,15,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,255,255,255,252,104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,179,255,255,133,0,0,0,0,0,0,0,0,153,255,255,255,255,255,255,255,255,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,251,255,245,22,0,0,0,0,0,0,0,20,215,228,191,187,187,187,187,187,187,178,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,156,255,255,102,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,255,255,181,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,234,255,247,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,169,255,255,52,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,131,255,255,85,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,113,255,255,100,0,0,0,0,0,0,101,255,255,113,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,114,255,255,100,0,0,0,0,0,0,86,255,255,130,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,255,255,85,0,0,0,0,0,0,53,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,255,52,0,0,0,0,0,0,11,248,255,233,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,235,255,247,10,0,0,0,0,0,0,0,182,255,255,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,255,255,179,0,0,0,0,0,0,0,0,103,255,255,155,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,157,255,255,101,0,0,0,0,0,0,0,0,23,245,255,251,40,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,251,255,244,21,0,0,0,0,0,0,0,0,0,134,255,255,180,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,182,255,255,131,0,0,0,0,0,0,0,0,0,0,15,236,255,255,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,255,255,234,14,0,0,0,0,0,0,0,0,0,0,0,89,254,255,253,87,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,254,255,254,85,0,0,0,0,0,0,0,0,0,0,0,0,0,134,255,255,254,136,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,119,253,255,255,130,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,168,255,255,255,180,40,0,0,0,0,0,0,0,0,0,0,0,0,42,182,255,255,255,168,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,133,254,255,255,251,156,62,3,0,0,0,0,0,0,3,62,157,252,255,255,254,132,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,89,235,255,255,255,255,235,169,131,114,114,130,169,235,255,255,255,255,234,86,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,119,242,255,255,255,255,255,255,255,255,255,255,255,255,245,136,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,102,181,248,255,255,255,255,255,255,247,180,101,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,52,83,101,101,83,51,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_rotate_ccw_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_rotate_ccw_40_pixels};
static const uint8_t nova_x_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,16,250,103,0,0,0,0,0,0,103,249,16,0,0,0,0,0,0,0,99,252,107,0,0,0,0,103,253,103,0,0,0,0,0,0,0,0,0,99,252,107,0,0,100,252,106,0,0,0,0,0,0,0,0,0,0,0,99,252,107,100,252,106,0,0,0,0,0,0,0,0,0,0,0,0,0,99,253,253,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,100,253,253,107,0,0,0,0,0,0,0,0,0,0,0,0,0,100,252,106,99,252,107,0,0,0,0,0,0,0,0,0,0,0,100,252,106,0,0,99,252,107,0,0,0,0,0,0,0,0,0,100,252,106,0,0,0,0,103,253,103,0,0,0,0,0,0,0,16,250,103,0,0,0,0,0,0,103,249,16,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_x_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_x_18_pixels};
static const uint8_t nova_x_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,0,0,0,0,0,0,5,156,138,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,0,0,0,0,0,5,174,255,157,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,181,176,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,176,255,255,182,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,176,255,255,182,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,181,176,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,0,0,0,0,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,0,0,0,0,0,5,174,255,157,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,0,0,0,0,0,0,5,156,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_x_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_x_24_pixels};
static const uint8_t nova_x_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,127,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_x_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_x_40_pixels};
static const uint8_t nova_brand_mark_18_pixels[] = {0,0,0,0,0,0,0,0,25,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,137,246,248,164,46,0,0,11,66,0,0,0,0,0,0,5,109,235,191,51,35,151,246,165,0,92,167,2,0,0,0,0,81,217,215,78,0,0,0,0,24,79,127,248,255,177,26,0,31,194,233,106,4,0,0,0,0,0,0,0,0,77,154,0,0,0,117,198,14,3,118,13,0,0,0,0,1,117,3,7,55,0,0,0,118,169,0,22,255,208,29,0,0,0,10,255,22,0,79,49,0,0,118,169,0,22,255,163,230,52,0,0,10,255,22,0,169,118,0,0,118,169,0,22,255,10,111,245,88,0,10,255,22,0,169,118,0,0,118,169,0,22,255,10,0,81,244,128,10,255,22,0,169,118,0,0,118,169,0,22,255,10,0,0,52,230,163,255,22,0,169,118,0,0,118,169,0,22,255,10,0,0,0,29,208,255,22,0,169,118,0,0,117,198,14,3,117,1,0,0,0,0,13,119,3,14,198,117,0,0,31,194,233,106,4,0,0,0,0,0,0,4,106,233,194,31,0,0,0,0,68,212,217,85,0,0,0,0,68,210,218,85,1,0,0,0,0,0,0,5,102,233,198,58,51,190,237,119,7,0,0,0,0,0,0,0,0,0,0,16,137,246,246,137,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,25,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_brand_mark_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_brand_mark_18_pixels};
static const uint8_t nova_brand_mark_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,83,214,224,109,10,0,0,0,0,0,31,0,0,0,0,0,0,0,0,0,0,0,56,196,255,210,194,255,235,126,18,0,0,44,144,0,0,0,0,0,0,0,0,0,34,169,254,229,99,3,0,58,177,254,242,34,2,140,228,15,0,0,0,0,0,0,18,141,249,244,127,11,0,0,0,0,0,43,124,92,226,255,255,248,151,13,0,0,5,102,234,253,162,25,0,0,0,0,0,0,0,0,0,0,27,172,240,59,2,0,0,0,202,255,183,44,0,0,0,0,0,0,0,0,0,0,0,0,0,60,160,0,0,0,0,0,242,154,0,0,54,165,22,0,0,0,0,0,0,1,163,54,0,2,53,0,0,0,0,0,242,140,0,0,115,255,223,43,0,0,0,0,0,13,255,115,0,0,32,73,0,0,0,0,242,140,0,0,115,255,233,240,70,0,0,0,0,13,255,115,0,0,138,239,0,0,0,0,242,140,0,0,115,255,39,204,252,108,0,0,0,13,255,115,0,0,140,242,0,0,0,0,242,140,0,0,115,255,13,11,176,255,145,3,0,13,255,115,0,0,140,242,0,0,0,0,242,140,0,0,115,255,13,0,1,128,255,183,14,13,255,115,0,0,140,242,0,0,0,0,242,140,0,0,115,255,13,0,0,0,100,250,209,43,255,115,0,0,140,242,0,0,0,0,242,140,0,0,115,255,13,0,0,0,0,70,240,233,255,115,0,0,140,242,0,0,0,0,242,140,0,0,115,255,13,0,0,0,0,0,43,223,255,115,0,0,140,242,0,0,0,0,242,154,0,0,54,163,1,0,0,0,0,0,0,22,165,54,0,0,154,242,0,0,0,0,202,255,183,44,0,0,0,0,0,0,0,0,0,0,0,0,44,183,255,202,0,0,0,0,5,102,234,253,162,25,0,0,0,0,0,0,0,0,20,145,251,238,119,8,0,0,0,0,0,0,17,136,247,245,136,13,0,0,0,0,10,119,241,250,153,21,0,0,0,0,0,0,0,0,0,0,30,162,254,229,102,3,2,85,224,255,179,35,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56,196,255,210,210,255,197,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,83,213,213,83,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_brand_mark_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_brand_mark_24_pixels};
static const uint8_t nova_brand_mark_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,72,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,195,255,255,214,97,6,0,0,0,0,0,0,0,0,0,38,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,168,254,255,255,255,255,255,227,114,12,0,0,0,0,0,0,0,182,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,140,248,255,255,247,137,114,227,255,255,255,238,131,21,0,0,0,0,19,251,99,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,112,237,255,255,254,165,31,0,0,6,96,214,255,255,255,246,91,0,0,0,101,255,184,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,84,220,255,255,255,193,53,0,0,0,0,0,0,2,79,198,255,255,175,0,19,99,218,255,250,128,43,0,0,0,0,0,0,0,0,0,0,0,57,198,255,255,255,216,79,0,0,0,0,0,0,0,0,0,0,0,62,136,69,180,251,255,255,255,255,255,255,213,85,0,0,0,0,0,0,0,30,162,254,255,255,236,119,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,94,179,249,255,255,213,136,60,0,0,0,0,0,0,17,136,248,255,255,249,145,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,212,0,0,0,0,0,0,0,0,59,237,255,255,253,162,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,255,127,0,0,0,0,0,0,0,0,146,255,255,190,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,212,42,0,0,0,0,0,0,0,0,148,255,235,0,0,0,0,0,163,209,54,0,0,0,0,0,0,0,0,0,0,0,0,32,209,163,0,0,0,0,85,0,0,0,0,0,0,0,0,0,148,255,234,0,0,0,0,18,255,255,246,84,0,0,0,0,0,0,0,0,0,0,0,104,255,255,18,0,0,0,0,0,0,0,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,255,254,121,0,0,0,0,0,0,0,0,0,0,107,255,255,21,0,0,0,0,91,175,38,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,255,255,255,159,6,0,0,0,0,0,0,0,0,107,255,255,21,0,0,0,0,228,255,142,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,149,222,255,255,191,18,0,0,0,0,0,0,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,19,194,255,255,219,39,0,0,0,0,0,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,7,164,255,255,237,64,0,0,0,0,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,1,128,254,255,249,94,0,0,0,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,87,247,255,255,145,2,0,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,0,58,234,255,255,171,9,0,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,0,0,35,215,255,255,199,23,107,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,0,0,0,18,191,255,255,222,149,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,0,0,0,0,4,151,255,255,255,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,21,255,255,107,0,0,0,0,0,0,0,0,0,0,121,254,255,255,255,21,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,234,0,0,0,0,18,255,255,104,0,0,0,0,0,0,0,0,0,0,0,84,246,255,255,18,0,0,0,0,234,255,148,0,0,0,0,0,0,148,255,235,0,0,0,0,0,163,209,32,0,0,0,0,0,0,0,0,0,0,0,0,54,209,163,0,0,0,0,0,235,255,148,0,0,0,0,0,0,146,255,255,190,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,190,255,255,146,0,0,0,0,0,0,59,237,255,255,253,162,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,30,162,253,255,255,237,59,0,0,0,0,0,0,0,17,136,248,255,255,249,145,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,128,246,255,255,250,153,21,0,0,0,0,0,0,0,0,0,0,30,162,254,255,255,236,119,6,0,0,0,0,0,0,0,0,0,0,0,0,4,102,232,255,255,255,179,36,0,0,0,0,0,0,0,0,0,0,0,0,0,0,54,194,255,255,255,217,85,0,0,0,0,0,0,0,0,0,0,68,211,255,255,255,201,61,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,219,255,255,255,199,59,0,0,0,0,0,0,52,191,255,255,255,225,94,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,102,233,255,255,254,170,34,0,0,28,153,253,255,255,238,119,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17,140,248,255,255,247,137,137,247,255,255,248,140,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33,168,254,255,255,255,255,254,168,33,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,55,195,255,255,195,55,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_brand_mark_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_brand_mark_40_pixels};
static const uint8_t nova_download_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,39,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,160,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,162,0,0,0,0,0,0,0,0,0,0,0,0,1,15,0,0,162,162,0,0,15,1,0,0,0,0,0,0,0,0,77,239,53,0,162,162,0,53,239,76,0,0,0,0,0,0,0,0,3,163,239,53,162,162,53,239,163,3,0,0,0,0,0,0,0,0,0,3,161,240,182,180,237,168,4,0,0,0,0,0,0,17,62,0,0,0,3,163,249,249,163,3,0,0,0,62,17,0,0,96,224,0,0,0,0,3,200,200,3,0,0,0,0,224,96,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,85,241,12,0,0,0,0,0,0,0,0,0,0,13,241,84,0,0,14,219,239,221,221,221,221,221,221,221,221,221,221,240,218,13,0,0,0,13,85,102,102,102,102,102,102,102,102,102,102,85,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_download_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_download_18_pixels};
static const uint8_t nova_download_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,217,217,0,0,5,156,138,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,217,217,0,5,174,255,157,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,217,217,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,218,218,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,243,243,255,174,5,0,0,0,0,0,0,0,0,0,0,138,138,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,138,138,0,0,0,0,217,217,0,0,0,0,0,0,5,210,210,5,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_download_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_download_24_pixels};
static const uint8_t nova_download_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,106,0,0,0,0,0,107,255,255,107,0,0,0,0,0,106,178,48,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,128,0,0,0,0,107,255,255,107,0,0,0,0,128,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,255,128,0,0,0,107,255,255,107,0,0,0,128,255,255,255,108,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,107,255,255,107,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,107,255,255,107,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,107,255,255,107,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,186,255,255,181,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,0,0,0,119,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,70,70,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,70,255,255,70,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,162,255,255,162,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,121,121,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,64,255,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,13,240,255,255,170,106,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,96,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,93,0,0,0,0,0,0,0,0,0,85,237,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,102,0,0,0,0,0,0,0,0,0,0,0,13,63,101,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_download_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_download_40_pixels};
static const uint8_t nova_bluetooth_18_pixels[] = {0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,209,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,255,214,24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,162,182,209,209,20,0,0,0,0,0,0,0,0,0,17,64,0,0,162,162,20,209,209,19,0,0,0,0,0,0,0,0,64,253,103,0,162,162,0,117,254,64,0,0,0,0,0,0,0,0,0,99,252,107,162,162,100,252,106,0,0,0,0,0,0,0,0,0,0,0,103,253,229,229,253,103,0,0,0,0,0,0,0,0,0,0,0,0,0,103,254,254,103,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,254,254,103,0,0,0,0,0,0,0,0,0,0,0,0,0,103,253,229,229,253,103,0,0,0,0,0,0,0,0,0,0,0,100,252,106,162,162,99,252,107,0,0,0,0,0,0,0,0,0,64,253,103,0,162,162,0,117,254,64,0,0,0,0,0,0,0,0,17,64,0,0,162,162,20,209,209,19,0,0,0,0,0,0,0,0,0,0,0,0,162,182,209,209,20,0,0,0,0,0,0,0,0,0,0,0,0,0,162,255,209,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,151,209,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_bluetooth_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_bluetooth_18_pixels};
static const uint8_t nova_bluetooth_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,154,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,222,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,217,217,0,5,174,255,158,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,217,217,0,5,174,255,158,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,217,217,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,174,222,222,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,255,255,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,174,222,222,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,5,174,255,180,7,217,217,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,157,255,174,5,0,217,217,0,5,174,255,158,0,0,0,0,0,0,0,0,0,0,0,0,138,156,5,0,0,217,217,0,5,174,255,158,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,5,174,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,222,174,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,255,180,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,174,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,138,154,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_bluetooth_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_bluetooth_24_pixels};
static const uint8_t nova_bluetooth_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,235,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,255,255,244,63,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,247,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,121,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,14,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,14,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,106,0,0,0,0,0,107,255,255,107,0,0,15,199,255,255,241,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,128,0,0,0,0,107,255,255,107,0,0,0,15,227,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,255,128,0,0,0,107,255,255,107,0,0,0,128,255,255,255,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,0,107,255,255,107,0,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,0,107,255,255,107,0,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,136,107,255,255,107,128,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,212,255,255,212,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,119,255,255,255,255,255,255,127,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,255,255,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,212,255,255,212,255,255,255,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,107,255,255,107,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,107,255,255,107,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,255,255,255,127,0,0,107,255,255,107,0,0,119,255,255,255,136,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,108,255,255,255,128,0,0,0,107,255,255,107,0,0,0,128,255,255,255,106,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,177,255,255,128,0,0,0,0,107,255,255,107,0,0,0,15,227,255,255,177,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,178,106,0,0,0,0,0,107,255,255,107,0,0,15,199,255,255,241,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,15,199,255,255,241,56,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,14,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,121,199,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,247,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,255,255,244,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,104,255,255,241,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,234,235,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_bluetooth_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_bluetooth_40_pixels};
static const uint8_t nova_square_18_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,86,102,102,102,102,102,102,102,102,102,102,86,14,0,0,0,15,219,239,221,221,221,221,221,221,221,221,221,221,239,219,14,0,0,84,240,12,0,0,0,0,0,0,0,0,0,0,12,241,84,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,99,226,0,0,0,0,0,0,0,0,0,0,0,0,226,99,0,0,85,241,12,0,0,0,0,0,0,0,0,0,0,13,241,84,0,0,14,219,239,221,221,221,221,221,221,221,221,221,221,240,218,13,0,0,0,13,85,102,102,102,102,102,102,102,102,102,102,85,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_square_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_square_18_pixels};
static const uint8_t nova_square_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,221,221,221,221,221,221,221,221,221,221,192,93,0,0,0,0,0,94,255,242,221,221,221,221,221,221,221,221,221,221,221,221,221,221,242,255,92,0,0,0,0,192,242,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,243,191,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_square_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_square_24_pixels};
static const uint8_t nova_square_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,64,101,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,64,13,0,0,0,0,0,0,0,0,0,0,0,85,238,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,241,102,0,0,0,0,0,0,0,0,0,99,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,97,0,0,0,0,0,0,0,14,240,255,255,168,106,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,169,255,255,240,12,0,0,0,0,0,0,64,255,255,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,170,255,255,63,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,109,255,255,104,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,107,255,255,107,0,0,0,0,0,0,104,255,255,109,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,110,255,255,104,0,0,0,0,0,0,64,255,255,169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,171,255,255,63,0,0,0,0,0,0,13,240,255,255,170,106,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,106,171,255,255,239,12,0,0,0,0,0,0,0,96,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,93,0,0,0,0,0,0,0,0,0,85,237,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,240,102,0,0,0,0,0,0,0,0,0,0,0,13,63,101,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,101,63,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_square_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_square_40_pixels};
static const uint8_t nova_trash_2_18_pixels[] = {0,0,0,0,0,0,1,29,34,34,29,0,0,0,0,0,0,0,0,0,0,0,0,24,212,255,255,255,255,212,24,0,0,0,0,0,0,0,0,0,0,136,211,42,34,34,42,212,135,0,0,0,0,0,0,1,32,47,35,174,174,34,34,34,34,174,174,35,47,32,1,0,0,76,255,255,255,255,255,255,255,255,255,255,255,255,255,255,76,0,0,1,32,230,120,41,41,34,34,34,34,41,41,120,230,32,1,0,0,0,0,226,99,0,0,0,0,0,0,0,0,99,226,0,0,0,0,0,0,226,99,0,1,76,1,1,76,1,0,99,226,0,0,0,0,0,0,226,99,0,33,255,33,33,255,33,0,99,226,0,0,0,0,0,0,226,99,0,35,255,35,35,255,35,0,99,226,0,0,0,0,0,0,226,99,0,35,255,35,35,255,35,0,99,226,0,0,0,0,0,0,226,99,0,35,255,35,35,255,35,0,99,226,0,0,0,0,0,0,226,99,0,33,255,33,33,255,33,0,99,226,0,0,0,0,0,0,226,99,0,1,76,1,1,76,1,0,99,226,0,0,0,0,0,0,226,99,0,0,0,0,0,0,0,0,99,226,0,0,0,0,0,0,199,163,36,34,34,34,34,34,34,36,164,198,0,0,0,0,0,0,62,236,255,255,255,255,255,255,255,255,236,61,0,0,0,0,0,0,0,4,33,34,34,34,34,34,34,33,4,0,0,0,0};
static const lv_img_dsc_t nova_trash_2_18 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=18,.h=18},.data_size=324,.data=nova_trash_2_18_pixels};
static const uint8_t nova_trash_2_24_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,192,221,221,221,221,192,93,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,94,255,242,221,221,221,221,242,255,92,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,242,21,0,0,0,0,22,243,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,141,221,239,239,221,250,250,221,221,221,221,221,221,250,250,221,239,239,219,138,0,0,0,0,141,221,250,250,221,239,239,221,221,221,221,221,221,239,239,221,250,250,219,138,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,138,138,0,0,138,138,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,217,217,0,0,217,217,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,138,138,0,0,138,138,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,0,0,0,0,217,217,0,0,0,0,0,0,0,0,192,242,22,0,0,0,0,0,0,0,0,0,0,23,243,191,0,0,0,0,0,0,0,0,92,255,243,221,221,221,221,221,221,221,221,221,221,243,255,90,0,0,0,0,0,0,0,0,0,92,191,221,221,221,221,221,221,221,221,221,221,191,90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_trash_2_24 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=24,.h=24},.data_size=576,.data=nova_trash_2_24_pixels};
static const uint8_t nova_trash_2_40_pixels[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,127,218,254,255,255,255,255,255,255,254,217,125,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,196,255,255,255,255,255,255,255,255,255,255,255,255,195,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,126,255,255,250,201,187,187,187,187,187,187,201,250,255,255,125,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,250,59,0,0,0,0,0,0,0,0,60,251,255,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,70,102,102,124,158,108,102,115,255,255,217,102,102,102,102,102,102,102,102,102,102,217,255,255,115,102,108,158,124,102,102,70,0,0,0,0,0,0,0,70,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,70,0,0,0,0,0,0,70,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,70,0,0,0,0,0,0,0,70,102,115,255,255,217,102,102,124,158,108,102,102,102,102,102,102,102,102,102,102,108,158,124,102,102,217,255,255,115,102,70,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,11,0,0,0,0,0,0,11,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,104,252,186,0,0,0,0,186,252,104,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,189,255,255,19,0,0,19,255,255,189,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,22,255,255,191,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,179,255,253,12,0,0,12,253,255,179,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,48,178,103,0,0,0,0,103,178,48,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,22,255,255,191,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,191,255,255,22,0,0,0,0,0,0,0,0,0,0,0,0,12,254,255,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,203,255,253,12,0,0,0,0,0,0,0,0,0,0,0,0,0,217,255,251,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,251,255,216,0,0,0,0,0,0,0,0,0,0,0,0,0,0,125,255,255,251,201,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,187,201,251,255,255,123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,195,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,193,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,125,216,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,253,215,124,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const lv_img_dsc_t nova_trash_2_40 = {.header={.cf=LV_IMG_CF_ALPHA_8BIT,.w=40,.h=40},.data_size=1600,.data=nova_trash_2_40_pixels};
static const struct {const char *name; const lv_img_dsc_t *small,*normal,*large;} nova_icon_assets[] = {
{"layout-grid",&nova_layout_grid_18,&nova_layout_grid_24,&nova_layout_grid_40},
{"wrench",&nova_wrench_18,&nova_wrench_24,&nova_wrench_40},
{"info",&nova_info_18,&nova_info_24,&nova_info_40},
{"activity",&nova_activity_18,&nova_activity_24,&nova_activity_40},
{"settings-2",&nova_settings_2_18,&nova_settings_2_24,&nova_settings_2_40},
{"play",&nova_play_18,&nova_play_24,&nova_play_40},
{"layers-2",&nova_layers_2_18,&nova_layers_2_24,&nova_layers_2_40},
{"folder-open",&nova_folder_open_18,&nova_folder_open_24,&nova_folder_open_40},
{"database",&nova_database_18,&nova_database_24,&nova_database_40},
{"hard-drive-download",&nova_hard_drive_download_18,&nova_hard_drive_download_24,&nova_hard_drive_download_40},
{"hard-drive-upload",&nova_hard_drive_upload_18,&nova_hard_drive_upload_24,&nova_hard_drive_upload_40},
{"columns-3",&nova_columns_3_18,&nova_columns_3_24,&nova_columns_3_40},
{"usb",&nova_usb_18,&nova_usb_24,&nova_usb_40},
{"folder-check",&nova_folder_check_18,&nova_folder_check_24,&nova_folder_check_40},
{"scan-line",&nova_scan_line_18,&nova_scan_line_24,&nova_scan_line_40},
{"power",&nova_power_18,&nova_power_24,&nova_power_40},
{"boxes",&nova_boxes_18,&nova_boxes_24,&nova_boxes_40},
{"circuit-board",&nova_circuit_board_18,&nova_circuit_board_24,&nova_circuit_board_40},
{"hard-drive",&nova_hard_drive_18,&nova_hard_drive_24,&nova_hard_drive_40},
{"card-sim",&nova_card_sim_18,&nova_card_sim_24,&nova_card_sim_40},
{"battery-charging",&nova_battery_charging_18,&nova_battery_charging_24,&nova_battery_charging_40},
{"cpu",&nova_cpu_18,&nova_cpu_24,&nova_cpu_40},
{"key-round",&nova_key_round_18,&nova_key_round_24,&nova_key_round_40},
{"lock-keyhole",&nova_lock_keyhole_18,&nova_lock_keyhole_24,&nova_lock_keyhole_40},
{"clipboard-list",&nova_clipboard_list_18,&nova_clipboard_list_24,&nova_clipboard_list_40},
{"fingerprint",&nova_fingerprint_18,&nova_fingerprint_24,&nova_fingerprint_40},
{"memory-stick",&nova_memory_stick_18,&nova_memory_stick_24,&nova_memory_stick_40},
{"wifi",&nova_wifi_18,&nova_wifi_24,&nova_wifi_40},
{"audio-lines",&nova_audio_lines_18,&nova_audio_lines_24,&nova_audio_lines_40},
{"volume-2",&nova_volume_2_18,&nova_volume_2_24,&nova_volume_2_40},
{"monitor",&nova_monitor_18,&nova_monitor_24,&nova_monitor_40},
{"gamepad-2",&nova_gamepad_2_18,&nova_gamepad_2_24,&nova_gamepad_2_40},
{"arrow-up-right",&nova_arrow_up_right_18,&nova_arrow_up_right_24,&nova_arrow_up_right_40},
{"clock-3",&nova_clock_3_18,&nova_clock_3_24,&nova_clock_3_40},
{"battery-full",&nova_battery_full_18,&nova_battery_full_24,&nova_battery_full_40},
{"toggle-right",&nova_toggle_right_18,&nova_toggle_right_24,&nova_toggle_right_40},
{"paintbrush",&nova_paintbrush_18,&nova_paintbrush_24,&nova_paintbrush_40},
{"shield-check",&nova_shield_check_18,&nova_shield_check_24,&nova_shield_check_40},
{"badge-check",&nova_badge_check_18,&nova_badge_check_24,&nova_badge_check_40},
{"sparkles",&nova_sparkles_18,&nova_sparkles_24,&nova_sparkles_40},
{"chevron-right",&nova_chevron_right_18,&nova_chevron_right_24,&nova_chevron_right_40},
{"refresh-cw",&nova_refresh_cw_18,&nova_refresh_cw_24,&nova_refresh_cw_40},
{"rotate-ccw",&nova_rotate_ccw_18,&nova_rotate_ccw_24,&nova_rotate_ccw_40},
{"x",&nova_x_18,&nova_x_24,&nova_x_40},
{"brand-mark",&nova_brand_mark_18,&nova_brand_mark_24,&nova_brand_mark_40},
{"download",&nova_download_18,&nova_download_24,&nova_download_40},
{"bluetooth",&nova_bluetooth_18,&nova_bluetooth_24,&nova_bluetooth_40},
{"square",&nova_square_18,&nova_square_24,&nova_square_40},
{"trash-2",&nova_trash_2_18,&nova_trash_2_24,&nova_trash_2_40},
};

static lv_style_t nova_panel, nova_selected, nova_hero, nova_clear, nova_primary;
static lv_style_t nova_text[5], nova_dim, nova_white, nova_accent_text;
static lv_style_t nova_frame, nova_frame_active, nova_frame_halo;
static lv_style_t nova_logo_image, nova_ink_image, nova_image, nova_image_white, nova_image_dim;
static lv_style_t nova_dot, nova_dot_active, nova_dot_on, nova_separator, nova_tag;
static lv_style_t nova_track_off, nova_track_on, nova_thumb_off, nova_thumb_on;
static lv_obj_t *nova_tabs, *nova_nav_buttons[6], *nova_nav_labels[6], *nova_nav_icons[6], *nova_underline;
static lv_obj_t *nova_battery_icon;
static lv_action_t nova_quick_actions[4];
static lv_obj_t *nova_quick_buttons[4];
static lv_obj_t *nova_brand_button;
static u32 nova_navigation_locks;
static lv_obj_t *nova_grid_pages[3], *nova_home_icons[5], *nova_home_cards[5];
static u32 nova_home_selected;
static const char *const nova_navigation_names[] = {"hekate", "Accueil", "Outils", "Infos", "Réglages", "Diagnostic"};
static const char *const nova_navigation_icons[] = {"info", "layout-grid", "wrench", "info", "settings-2", "activity"};

void nyx_nova_init(lv_theme_t *theme)
{
    nyx_nova_reflection_reset();
    memset(nova_grid_pages, 0, sizeof(nova_grid_pages));
    memset(nova_nav_buttons, 0, sizeof(nova_nav_buttons));
    memset(nova_quick_buttons, 0, sizeof(nova_quick_buttons));
    nova_navigation_locks=0;nova_brand_button=NULL;nova_underline=NULL;
    lv_color_t bg = nyx_nova_color(NOVA_COLOR_BACKGROUND);
    lv_color_t text = nyx_nova_color(NOVA_COLOR_TEXT);
    lv_color_t border = nyx_nova_color(NOVA_COLOR_BORDER);
    lv_color_t accent = nyx_nova_color(NOVA_COLOR_ICONS);
    lv_color_t panel = n_cfg.theme_bg == 0x141A19 ? LV_COLOR_HEX(0x19212D) : lv_color_mix(LV_COLOR_WHITE, bg, 14);
    lv_color_t dim = n_cfg.nova_text_color == 0xF2F5FA ? LV_COLOR_HEX(0x9BAABD) : lv_color_mix(text, bg, 166);
    memset(nova_home_icons, 0, sizeof(nova_home_icons));
    memset(nova_home_cards,0,sizeof(nova_home_cards));nova_home_selected=0;
    lv_style_copy(&nova_panel, theme->panel);
    nova_panel.body.empty = 0;
    nova_panel.body.opa = n_cfg.nova_menu_opacity * 255 / 100;
    nova_panel.body.main_color = nova_panel.body.grad_color = panel;
    nova_panel.body.radius = 8;
    nova_panel.body.border.width = 1;
    nova_panel.body.border.color = border;
    nova_panel.body.border.opa = LV_OPA_COVER;
    nova_panel.body.shadow.width = 0;
    nova_panel.body.padding.hor = nova_panel.body.padding.ver = nova_panel.body.padding.inner = 0;
    lv_style_copy(&nova_selected, &nova_panel);
    nova_selected.body.main_color = nova_selected.body.grad_color = nyx_nova_color(NOVA_COLOR_SELECTION);
    nova_selected.body.border.color = n_cfg.nova_border_color==0x334050?lv_color_mix(accent,border,160):border;
    lv_style_copy(&nova_hero, &nova_panel);
    nova_hero.body.main_color = LV_COLOR_HEX(0x1E303F);
    nova_hero.body.grad_color = LV_COLOR_HEX(0x19212D);
    nova_hero.body.border.color = LV_COLOR_HEX(0x428398);
    lv_style_copy(&nova_clear, &lv_style_transp_tight);
    nova_clear.body.border.width = nova_clear.body.shadow.width = 0;
    nova_clear.body.radius = 0;
    const u32 sizes[] = {14,16,18,20,36};
    for (u32 i = 0; i < 5; ++i) {
        lv_style_copy(&nova_text[i], theme->label.prim);
        nova_text[i].text.font = nyx_font_ui(sizes[i]);
        nova_text[i].text.letter_space = 0;
        nova_text[i].text.line_space = 3;
        nova_text[i].text.color = text;
    }
    nova_text[NOVA_KICKER].text.letter_space = 2;
    nova_text[NOVA_KICKER].text.color = dim;
    lv_style_copy(&nova_dim, &nova_text[NOVA_SMALL]);
    nova_dim.text.color = dim;
    lv_style_copy(&nova_white, &nova_text[NOVA_BODY]);
    lv_style_copy(&nova_accent_text, &nova_text[NOVA_SMALL]);
    nova_accent_text.text.color = accent;
    lv_style_copy(&nova_frame, &nova_panel);
    nova_frame.body.radius = 8;
    nova_frame.body.opa = n_cfg.nova_icon_opacity * 255 / 100;
    nova_frame.body.main_color = nova_frame.body.grad_color = n_cfg.theme_bg == 0x141A19 ? LV_COLOR_HEX(0x1E303F) : lv_color_mix(accent, panel, 22);
    nova_frame.body.border.color = n_cfg.nova_border_color==0x334050?lv_color_mix(accent,border,38):border;
    lv_style_copy(&nova_frame_active, &nova_frame);
    nova_frame_active.body.border.color = n_cfg.nova_border_color==0x334050?accent:border;
    lv_style_copy(&nova_frame_halo, &nova_frame_active);
    nova_frame_halo.body.shadow.width = 0; /* The moving reflection replaces the expensive blurred halo. */
    nova_frame_halo.body.shadow.type = LV_SHADOW_FULL;
    nova_frame_halo.body.shadow.color = LV_COLOR_HEX(0x29546A);
    lv_style_copy(&nova_image, &lv_style_plain);
    nova_image.image.color = accent;
    nova_image.image.opa = LV_OPA_COVER;
    lv_style_copy(&nova_logo_image, &nova_image); nova_logo_image.image.color = nyx_nova_color(NOVA_COLOR_LOGO);
    lv_style_copy(&nova_image_dim, &nova_image); nova_image_dim.image.color = dim;
    lv_style_copy(&nova_image_white, &nova_image); nova_image_white.image.color = text;
    lv_style_copy(&nova_ink_image, &nova_image); nova_ink_image.image.color = LV_COLOR_HEX(0x0C1721);
    lv_style_copy(&nova_primary, &nova_panel);
    nova_primary.body.main_color = nova_primary.body.grad_color = accent;
    nova_primary.body.opa = LV_OPA_COVER;
    nova_primary.body.border.width = 0; nova_primary.body.radius = 6;
    nova_primary.text.color = LV_COLOR_HEX(0x0C1721);
    nova_primary.text.font = nyx_font_ui(18);
    lv_style_copy(&nova_dot, &nova_clear);
    nova_dot.body.empty = 0; nova_dot.body.opa = LV_OPA_COVER;
    nova_dot.body.radius = LV_RADIUS_CIRCLE;
    nova_dot.body.main_color = nova_dot.body.grad_color = border;
    lv_style_copy(&nova_dot_active, &nova_dot);
    nova_dot_active.body.main_color = nova_dot_active.body.grad_color = accent;
    lv_style_copy(&nova_dot_on, &nova_dot_active);
    nova_dot_on.body.main_color = nova_dot_on.body.grad_color = LV_COLOR_HEX(0x8AE6B6);
    lv_style_copy(&nova_separator, &nova_dot); nova_separator.body.radius = 0;
    lv_style_copy(&nova_tag, &nova_frame); nova_tag.body.border.width = 0; nova_tag.body.radius = 4;
    nova_tag.body.main_color = nova_tag.body.grad_color = LV_COLOR_HEX(0x162530);
    lv_style_copy(&nova_track_off, &nova_dot);
    nova_track_off.body.main_color = nova_track_off.body.grad_color = panel;
    nova_track_off.body.radius = 8; nova_track_off.body.border.width = 1;
    nova_track_off.body.border.color = border; nova_track_off.body.border.opa = LV_OPA_COVER;
    lv_style_copy(&nova_track_on, &nova_track_off);
    nova_track_on.body.main_color = nova_track_on.body.grad_color = nyx_nova_color(NOVA_COLOR_SELECTION);
    nova_track_on.body.border.color = accent;
    lv_style_copy(&nova_thumb_off, &nova_track_off);
    nova_thumb_off.body.main_color = nova_thumb_off.body.grad_color = dim;
    nova_thumb_off.body.radius = 5; nova_thumb_off.body.border.width = 0;
    lv_style_copy(&nova_thumb_on, &nova_thumb_off);
    nova_thumb_on.body.main_color = nova_thumb_on.body.grad_color = accent;
}

const lv_style_t *nyx_nova_text_style(u32 role) { return role < 5 ? &nova_text[role] : &nova_dim; }

lv_obj_t *nyx_nova_text(lv_obj_t *parent, const char *text, int x, int y, int width, u32 role)
{
    if(getenv("NYX_TRACE"))fprintf(stderr,"NOVA TEXT: %s\n",text);
    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_style(label, (lv_style_t *)nyx_nova_text_style(role));
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, width); lv_label_set_text(label, text); lv_obj_set_pos(label, x, y);
    return label;
}

void nyx_nova_fit_text(lv_obj_t *label, int height)
{
    if (lv_obj_get_height(label) > height) lv_label_set_style(label, &nova_text[NOVA_SMALL]);
    if (lv_obj_get_height(label) > height) lv_label_set_style(label, &nova_text[NOVA_KICKER]);
    if (lv_obj_get_height(label) > height) {
        /* Fixed text boxes use ellipsis rather than perpetual marquees. The
         * selected card's complete help is also available in the detail bar. */
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_height(label, height);
    }
}

lv_obj_t *nyx_nova_panel(lv_obj_t *parent, int x, int y, int width, int height)
{
    lv_obj_t *panel = lv_cont_create(parent, NULL);
    lv_cont_set_style(panel, &nova_panel); lv_cont_set_fit(panel, false, false);
    lv_cont_set_layout(panel, LV_LAYOUT_OFF); lv_obj_set_size(panel, width, height);
    lv_obj_set_pos(panel, x, y); lv_obj_set_click(panel, false); return panel;
}

lv_obj_t *nyx_nova_glyph(lv_obj_t *parent, const char *name, int x, int y, u32 pixels)
{
    const lv_img_dsc_t *asset = nova_icon_assets[0].normal;
    for (u32 i = 0; i < ARRAY_SIZE(nova_icon_assets); ++i) if (!strcmp(name, nova_icon_assets[i].name)) {
        asset = pixels <= 18 ? nova_icon_assets[i].small : pixels >= 40 ? nova_icon_assets[i].large : nova_icon_assets[i].normal;
        break;
    }
    lv_obj_t *image = lv_img_create(parent, NULL); lv_img_set_src(image, asset);
    lv_img_set_style(image, !strcmp(name,"brand-mark") ? &nova_logo_image : &nova_image); lv_obj_set_pos(image, x, y); lv_obj_set_click(image, false);
    return image;
}

lv_obj_t *nyx_nova_icon_box(lv_obj_t *parent, const char *name, int x, int y, int size, bool selected)
{
    lv_obj_t *frame = nyx_nova_panel(parent, x, y, size, size);
    lv_cont_set_style(frame, selected ? (n_cfg.nova_halo ? &nova_frame_halo : &nova_frame_active) : &nova_frame);
    u32 pixels = size >= 70 ? 40 : 24;
    nyx_nova_glyph(frame, name, (size - pixels) / 2, (size - pixels) / 2, pixels);
    nyx_nova_reflect(frame,selected);
    return frame;
}

void nyx_nova_icon_select(lv_obj_t *icon, bool selected)
{
    lv_cont_set_style(icon, selected ? (n_cfg.nova_halo ? &nova_frame_halo : &nova_frame_active) : &nova_frame);
    nyx_nova_reflect(icon,selected);
}

static lv_res_t nova_navigate(lv_obj_t *button)
{
    if(nova_navigation_locks)return LV_RES_OK;
    u32 tab = lv_obj_get_free_num(button);
    if (nova_tabs && tab < 6) { lv_tabview_set_tab_act(nova_tabs, tab, false); nyx_nova_navigation_select(tab); }
    return LV_RES_OK;
}

lv_obj_t *nyx_nova_tabview(lv_obj_t *screen)
{
    lv_obj_t *tabs = lv_tabview_create(screen, NULL);
    lv_tabview_set_sliding(tabs, false); lv_tabview_set_btns_hidden(tabs, true);
    lv_obj_set_size(tabs, LV_HOR_RES, NYX_NOVA_BOTTOM - NYX_NOVA_HEADER);
    lv_obj_set_pos(tabs, 0, NYX_NOVA_HEADER); return tabs;
}

void nyx_nova_navigation_select(u32 tab)
{
    for (u32 i = 1; i < 6; ++i) if (nova_nav_buttons[i]) {
        lv_label_set_style(nova_nav_labels[i], i == tab&&!nova_navigation_locks ? &nova_white : &nova_dim);
        lv_img_set_style(nova_nav_icons[i], i == tab&&!nova_navigation_locks ? &nova_image_white : &nova_image_dim);
        nyx_nova_reflect(nova_nav_buttons[i],i==tab&&!nova_navigation_locks);
    }
    if (!nova_underline) return;
    lv_obj_set_hidden(nova_underline, !tab || tab >= 6);
    if (tab && tab < 6 && nova_nav_buttons[tab]) {
        lv_obj_set_x(nova_underline, lv_obj_get_x(nova_nav_buttons[tab]));
        lv_obj_set_width(nova_underline, lv_obj_get_width(nova_nav_buttons[tab]));
    }
}

void nyx_nova_navigation(lv_obj_t *screen, lv_obj_t *tabs, const char *version, lv_action_t brand_action)
{
    (void)version; nova_tabs = tabs;
    lv_obj_t *brand = lv_btn_create(screen, NULL);
    nova_brand_button=brand;
    lv_btn_set_fit(brand, false, false); lv_btn_set_layout(brand, LV_LAYOUT_OFF);
    lv_btn_set_style(brand, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(brand, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(brand, 178, 60); lv_obj_set_pos(brand, 26, 8);
    (void)brand_action; lv_btn_set_action(brand, LV_BTN_ACTION_CLICK, nova_navigate);
    lv_btn_set_action(brand, LV_BTN_ACTION_LONG_PR, nova_navigate); lv_obj_set_free_num(brand, 0);
    nyx_nova_glyph(brand, "brand-mark", 0, 10, 40);
    lv_obj_t *name = nyx_nova_text(brand, "HEKATE", 50, 8, 122, NOVA_BODY);
    static lv_style_t brand_text; lv_style_copy(&brand_text, &nova_text[NOVA_BODY]); brand_text.text.letter_space = 2;
    lv_label_set_style(name, &brand_text);
    nyx_nova_text(brand, "NOVA PRO", 50, 34, 122, NOVA_KICKER);
    const u32 order[] = {1,2,3,5,4};
    int widths[5], total = 0, gap = 18, available = 778;
    for (u32 row = 0; row < 5; ++row) {
        const char *text = nyx_tr(nova_navigation_names[order[row]]);
        widths[row] = lv_txt_get_width(text, strlen(text), nova_white.text.font, 0, LV_TXT_FLAG_NONE) + 60;
        total += widths[row];
    }
    if (total + 4 * gap > available) gap = 10;
    int x = 218;
    for (u32 row = 0; row < 5; ++row) {
        u32 tab = order[row];
        int width = total + 4 * gap > available ? widths[row] * (available - 4 * gap) / total : widths[row];
        lv_obj_t *button = lv_btn_create(screen, NULL); nova_nav_buttons[tab] = button;
        lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
        lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_clear);
        lv_obj_set_size(button, width, 52); lv_obj_set_pos(button, x, 12);
        lv_obj_set_free_num(button, tab); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_navigate);
        nova_nav_icons[tab] = nyx_nova_glyph(button, nova_navigation_icons[tab], 12, 17, 18);
        nova_nav_labels[tab] = nyx_nova_text(button, nova_navigation_names[tab], 40, 14, width - 52, NOVA_BODY);
        nyx_nova_fit_text(nova_nav_labels[tab], 26);
        x += width + gap;
    }
    const char *quick_icons[] = {"settings-2", "refresh-cw", "rotate-ccw", "power"};
    for (u32 i = 0; i < 4; ++i) {
        lv_obj_t *button = lv_btn_create(screen, NULL); nova_quick_buttons[i] = button;
        lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
        lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_frame); lv_btn_set_style(button, LV_BTN_STYLE_PR, lv_theme_get_current()->btn.pr);
        lv_obj_set_size(button, 44, 44); lv_obj_set_pos(button, 1034 + i * 56, 16);
        nyx_nova_glyph(button, quick_icons[i], 10, 10, 24);
        lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_quick_actions[i]);
    }
    lv_obj_t *separator = lv_cont_create(screen, NULL); lv_cont_set_style(separator, &nova_separator);
    lv_cont_set_fit(separator, false, false); lv_obj_set_size(separator, LV_HOR_RES, 1);
    lv_obj_set_pos(separator, 0, NYX_NOVA_HEADER - 1); lv_obj_set_click(separator, false);
    nova_underline = lv_cont_create(screen, NULL); lv_cont_set_style(nova_underline, &nova_dot_active);
    lv_cont_set_fit(nova_underline, false, false); lv_obj_set_size(nova_underline, 80, 2);
    lv_obj_set_y(nova_underline, NYX_NOVA_HEADER - 2); lv_obj_set_click(nova_underline, false);
    nyx_nova_navigation_select(1);
}

void nyx_nova_footer(lv_obj_t *layer, lv_action_t save)
{
    lv_obj_t *bar = lv_cont_create(layer, NULL); lv_cont_set_style(bar, &nova_clear); lv_cont_set_fit(bar, false, false);
    lv_obj_set_size(bar, LV_HOR_RES, LV_VER_RES - NYX_NOVA_BOTTOM); lv_obj_set_pos(bar, 0, NYX_NOVA_BOTTOM);
    status_bar.bar_bg = bar;
    lv_obj_t *separator = lv_cont_create(bar, NULL); lv_cont_set_style(separator, &nova_separator);
    lv_cont_set_fit(separator, false, false); lv_obj_set_size(separator, LV_HOR_RES, 1); lv_obj_set_pos(separator, 0, 0); lv_obj_set_click(separator, false);
    lv_obj_t *clock = nyx_nova_glyph(bar, "clock-3", 34, 23, 18); lv_img_set_style(clock, &nova_image_dim);
    status_bar.time_temp = nyx_nova_text(bar, "00:00 · 00.0 °C", 64, 22, 450, NOVA_SMALL); lv_label_set_style(status_bar.time_temp, &nova_dim);
    static lv_style_t compact; lv_style_copy(&compact, &nova_dim); compact.text.font = nyx_font_ui(14);
    lv_obj_t *version = nyx_nova_text(bar, "hekate 6.5.5", 972, 24, 152, NOVA_SMALL); lv_label_set_style(version, &compact);
    nova_battery_icon = nyx_nova_glyph(bar, "battery-full", 1138, 20, 24); lv_img_set_style(nova_battery_icon, &nova_image_dim);
    status_bar.battery = nyx_nova_text(bar, "00.0%", 1174, 24, 72, NOVA_SMALL); lv_label_set_style(status_bar.battery, &compact);
    lv_label_set_align(status_bar.battery, LV_LABEL_ALIGN_LEFT); lv_label_set_recolor(status_bar.battery, true);
    lv_label_set_long_mode(status_bar.battery, LV_LABEL_LONG_CROP);
    lv_obj_set_size(status_bar.battery, 72, 26); lv_obj_set_pos(status_bar.battery, 1174, 24);
    status_bar.battery_more = nyx_nova_text(bar, "", 34, 42, 500, NOVA_SMALL); lv_obj_set_hidden(status_bar.battery_more, true);
    status_bar.mid = lv_btn_create(bar, NULL); lv_btn_set_fit(status_bar.mid, false, false); lv_btn_set_layout(status_bar.mid, LV_LAYOUT_OFF);
    lv_obj_set_size(status_bar.mid, 248, 44); lv_obj_set_pos(status_bar.mid, 516, 10);
    lv_obj_t *label = nyx_nova_text(status_bar.mid, "Save Options", 12, 12, 224, NOVA_SMALL); lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    nyx_nova_fit_text(label, 28); lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, save);
    lv_obj_set_opa_scale_enable(status_bar.mid, true); lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP); lv_obj_set_click(status_bar.mid, false);
}

void nyx_nova_battery(u32 percent, bool charging)
{
    if (!nova_battery_icon) return;
    const char *name = charging ? "battery-charging" : "battery-full";
    for (u32 i = 0; i < ARRAY_SIZE(nova_icon_assets); ++i) if (!strcmp(name, nova_icon_assets[i].name)) {
        lv_img_set_src(nova_battery_icon, nova_icon_assets[i].normal); break;
    }
    static lv_style_t low; lv_style_copy(&low, &nova_image_dim); low.image.color = LV_COLOR_HEX(0xFF6B6B);
    lv_img_set_style(nova_battery_icon, percent <= 15 && !charging ? &low : &nova_image_dim);
}

static void nova_uppercase_label(lv_obj_t *label, const char *text)
{
    const char *translated = nyx_tr(text); char upper[256]; uint32_t at = 0; u32 used = 0;
    while (translated[at] && used < sizeof(upper) - 5) {
        u32 cp = lv_txt_encoded_next(translated, &at);
        if ((cp >= 'a' && cp <= 'z') || (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) || (cp >= 0x430 && cp <= 0x44F)) cp -= 32;
        if (cp == 0x451) cp = 0x401;
        u32 encoded = lv_txt_unicode_to_encoded(cp);
        u32 length = lv_txt_encoded_size((const char *)&encoded);
        if (!length) break;
        memcpy(upper + used, &encoded, length); used += length;
    }
    upper[used] = 0; (lv_label_set_text)(label, upper);
}

void nyx_nova_page(lv_obj_t *page, u32 tab)
{
    static const char *const kickers[] = {"hekate", "Bienvenue dans Hekate", "Outils", "Infos", "Réglages", "Diagnostic"};
    static const char *const headings[] = {"Hekate Nova Pro", "Votre console, simplement.", "Outils et maintenance.", "Informations console.", "Apparence et options.", "Diagnostic matériel."};
    static const char *const subtitles[] = {"", "Démarrer, gérer et vérifier.", "Stockage, maintenance et connexions.", "Matériel, stockage et alimentation.", "Lecture, options et effets de sélection.", "42 contrôles natifs et un service RAM isolé."};

    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF); lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, lv_obj_get_width(page)); lv_page_set_scrl_height(page, lv_obj_get_height(page));
    lv_page_set_sb_mode(page, LV_SB_MODE_OFF); lv_obj_set_pos(lv_page_get_scrl(page), 0, 0); lv_obj_set_drag(lv_page_get_scrl(page), false);
    if (tab >= 6) return;
    lv_obj_t *kicker = nyx_nova_text(page, kickers[tab], 34, 22, 940, NOVA_KICKER); nova_uppercase_label(kicker, kickers[tab]);
    lv_obj_t *heading = nyx_nova_text(page, headings[tab], 34, 46, 940, NOVA_HEADING);
    nyx_nova_fit_text(heading, 44);
    lv_obj_t *subtitle = nyx_nova_text(page, subtitles[tab], 34, 98, 1040, NOVA_BODY);
    lv_label_set_style(subtitle, &nova_dim); nyx_nova_fit_text(subtitle, 32);

}

typedef struct {
    lv_page_ext_t page;
    u32 kind, count, selected;
    bool diagnostics_opened, armed;
    lv_obj_t *buttons[12], *cards[12], *icons[12], *dots[12], *descriptions[12];
    const char *help[12];
    lv_action_t actions[12], holds[12];
    lv_obj_t *detail_title, *detail_text, *open;
} nova_grid_ext;
typedef struct { lv_btn_ext_t button; lv_obj_t *page; u32 index; } nova_card_ext;
static lv_signal_func_t nova_page_signal_base, nova_button_signal_base, nova_toggle_signal_base;
static lv_res_t nova_page_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_page_signal_base(object, signal, parameter); }
static lv_res_t nova_card_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_button_signal_base(object, signal, parameter); }
static const nova_item *nova_grid_items(u32 kind)
{ return kind == NOVA_TOOLS ? nova_reference_tools : kind == NOVA_INFO ? nova_reference_info : nova_reference_diagnostics; }

static void nova_grid_select(lv_obj_t *page, u32 index)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    if (index >= ext->count || !lv_obj_get_click(ext->buttons[index])) return;
    ext->selected = index;
    const nova_item *items = nova_grid_items(ext->kind);
    for (u32 i = 0; i < ext->count; ++i) {
        lv_cont_set_style(ext->cards[i], i == index ? &nova_selected : &nova_panel);
        nyx_nova_reflect(ext->cards[i],i==index);

        nyx_nova_icon_select(ext->icons[i], i == index);
        lv_cont_set_style(ext->dots[i], i == index ? &nova_dot_active : &nova_dot);
        if (ext->kind == NOVA_TOOLS && i == 6)
            lv_cont_set_style(ext->dots[i], lv_btn_get_state(ext->buttons[i]) & LV_BTN_STATE_TGL_REL ? &nova_dot_on : &nova_dot);
    }
    lv_label_set_text(ext->detail_title, items[index].title);
    if (ext->kind == NOVA_DIAGNOSTICS && !ext->diagnostics_opened) {
        char description[384];
        s_printf(description, "%s %s", nyx_tr(items[index].description), nyx_tr("Aucun test lancé."));
        lv_label_set_text(ext->detail_text, description);
    } else lv_label_set_text(ext->detail_text, items[index].description);
    nyx_nova_fit_text(ext->detail_text, 26);
}

static lv_res_t nova_card_select(lv_obj_t *button)
{
    nova_card_ext *card = lv_obj_get_ext_attr(button);
    nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
    if (!lv_obj_get_click(button)) return LV_RES_OK;
    if (ext->selected == card->index && ext->armed) {
        ext->armed = false;
        if (ext->kind == NOVA_DIAGNOSTICS) ext->diagnostics_opened = true;
        return ext->actions[card->index] ? ext->actions[card->index](button) : LV_RES_OK;
    }
    nova_grid_select(card->page, card->index); ext->armed = true;
    return LV_RES_OK;
}
static lv_res_t nova_card_hold(lv_obj_t *button)
{
    nova_card_ext *card = lv_obj_get_ext_attr(button); nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
    return ext->holds[card->index] ? ext->holds[card->index](button) : LV_RES_OK;
}
static lv_res_t nova_grid_help(lv_obj_t *button)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(lv_obj_get_free_ptr(button));
    if (ext->selected < ext->count) nyx_show_help(ext->help[ext->selected]);
    return LV_RES_OK;
}
lv_res_t nyx_nova_grid_open(lv_obj_t *button)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(lv_obj_get_free_ptr(button));
    u32 index = ext->selected;
    if (index >= ext->count || !lv_obj_get_click(ext->buttons[index]) || !ext->actions[index]) return LV_RES_OK;
    if (ext->kind == NOVA_DIAGNOSTICS) ext->diagnostics_opened = true;
    /* Retained for callers outside the tile view; visible tiles use two taps. */
    return ext->actions[index](ext->buttons[index]);
}

void nyx_nova_grid_begin(lv_obj_t *page, u32 kind)
{
    if (!nova_page_signal_base) nova_page_signal_base = lv_obj_get_signal_func(page);
    nova_grid_ext *ext = lv_obj_allocate_ext_attr(page, sizeof(*ext));
    if (!ext) return;
    memset((u8 *)ext + sizeof(lv_page_ext_t), 0, sizeof(*ext) - sizeof(lv_page_ext_t));
    ext->kind = kind; lv_obj_set_signal_func(page, nova_page_signal); nova_grid_pages[kind] = page;
    nyx_nova_page(page, kind == NOVA_TOOLS ? 2 : kind == NOVA_INFO ? 3 : 5);
    int y = lv_obj_get_height(page) - 104;
    lv_obj_t *detail = nyx_nova_panel(page, 34, y, LV_HOR_RES - 68, 76);
    ext->detail_title = nyx_nova_text(detail, "", 24, 13, 1020, NOVA_BODY);
    ext->detail_text = nyx_nova_text(detail, "", 24, 42, 1020, NOVA_SMALL); lv_label_set_style(ext->detail_text, &nova_dim);
    lv_obj_t *help = lv_btn_create(detail, NULL); lv_btn_set_fit(help, false, false); lv_btn_set_layout(help, LV_LAYOUT_OFF);
    lv_btn_set_style(help, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(help, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(help, 1080, 76); lv_obj_set_pos(help, 4, 0); lv_obj_set_free_ptr(help, page);
    lv_btn_set_action(help, LV_BTN_ACTION_CLICK, nova_grid_help);

}

lv_obj_t *nyx_nova_grid_add(lv_obj_t *page, const char *help, lv_action_t action, lv_action_t hold, u32 id)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    u32 index = ext->count, total = ext->kind == NOVA_TOOLS ? 8 : ext->kind == NOVA_INFO ? 7 : 12;
    if (index >= total) return NULL;
    const nova_item *item = &nova_grid_items(ext->kind)[index];
    int gap = ext->kind == NOVA_DIAGNOSTICS ? 12 : 16;
    int rows = ext->kind == NOVA_DIAGNOSTICS ? 3 : 2;
    int width = (LV_HOR_RES - 68 - 3 * gap) / 4;
    int height = (lv_obj_get_height(page) - 104 - 20 - 140 - (rows - 1) * gap) / rows;
    lv_obj_t *card = nyx_nova_panel(page, 34 + index % 4 * (width + gap), 140 + index / 4 * (height + gap), width, height);
    ext->cards[index] = card;
    lv_obj_t *button = lv_btn_create(card, NULL);
    if (!nova_button_signal_base) nova_button_signal_base = lv_obj_get_signal_func(button);
    nova_card_ext *card_ext = lv_obj_allocate_ext_attr(button, sizeof(*card_ext));
    if (!card_ext) { lv_obj_del(card); return NULL; }
    card_ext->page = page; card_ext->index = index; lv_obj_set_signal_func(button, nova_card_signal);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    for (u32 state = LV_BTN_STYLE_REL; state <= LV_BTN_STYLE_INA; ++state) lv_btn_set_style(button, state, &nova_clear);
    lv_obj_set_size(button, width, height); lv_obj_set_pos(button, 0, 0); lv_obj_set_free_num(button, id);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_card_select);
    if (hold) lv_btn_set_action(button, LV_BTN_ACTION_LONG_PR, nova_card_hold);
    ext->buttons[index] = button; ext->actions[index] = action; ext->holds[index] = hold; ext->help[index] = help;
    int text_x = 72, text_width = width - text_x - 18;
    lv_obj_t *title = nyx_nova_text(button, item->title, text_x, 18, text_width, NOVA_BODY);
    nyx_nova_fit_text(title, ext->kind == NOVA_DIAGNOSTICS ? height - 24 : 52);
    ext->descriptions[index] = nyx_nova_text(card, item->description, text_x, 20 + lv_obj_get_height(title), text_width, NOVA_SMALL);
    lv_label_set_style(ext->descriptions[index], &nova_dim);
    if (ext->kind == NOVA_DIAGNOSTICS) lv_obj_set_hidden(ext->descriptions[index], true);
    else nyx_nova_fit_text(ext->descriptions[index], height - lv_obj_get_y(ext->descriptions[index]) - 18);
    int group_height = lv_obj_get_height(title) + (ext->kind == NOVA_DIAGNOSTICS ? 0 : lv_obj_get_height(ext->descriptions[index]) + 3);
    int title_y = (height - group_height) / 2;
    if (title_y < 12) title_y = 12;
    lv_obj_set_y(title, title_y); lv_obj_set_y(ext->descriptions[index], title_y + lv_obj_get_height(title) + 3);
    ext->icons[index] = nyx_nova_icon_box(card, item->icon, 18, (height - 42) / 2, 42, false);
    ext->dots[index] = lv_cont_create(card, NULL); lv_cont_set_style(ext->dots[index], &nova_dot);
    lv_cont_set_fit(ext->dots[index], false, false); lv_obj_set_size(ext->dots[index], 5, 5);
    lv_obj_set_pos(ext->dots[index], width - 20, 16); lv_obj_set_click(ext->dots[index], false);
    ++ext->count; return button;
}

void nyx_nova_grid_finish(lv_obj_t *page)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    for (u32 i = 0; i < ext->count; ++i) if (!lv_obj_get_click(ext->buttons[i])) {
        lv_obj_t *title = lv_obj_get_child(ext->buttons[i], NULL); lv_label_set_style(title, &nova_dim);
        lv_obj_t *glyph = lv_obj_get_child(ext->icons[i], NULL); lv_img_set_style(glyph, &nova_image_dim);
        lv_label_set_text(ext->descriptions[i], "Unavailable");
    }
    nova_grid_select(page, 0); ext->armed = false;
}

void nyx_nova_diagnostic_result(void)
{
    lv_obj_t *page = nova_grid_pages[NOVA_DIAGNOSTICS];
    if (!page) return;
    nova_grid_ext *ext = lv_obj_get_ext_attr(page); ext->diagnostics_opened = true;
    nova_grid_select(page, ext->selected);
}

void nyx_nova_refresh_halos(void)
{
    if(nova_tabs)nyx_nova_navigation_select(lv_tabview_get_tab_act(nova_tabs));
    for (u32 i = 0; i < 3; ++i) if (nova_grid_pages[i]) {
        nova_grid_ext *ext = lv_obj_get_ext_attr(nova_grid_pages[i]); nova_grid_select(nova_grid_pages[i], ext->selected);
    }
    for (u32 i = 0; i < 5; ++i) {
        if(nova_home_icons[i])nyx_nova_icon_select(nova_home_icons[i],i==nova_home_selected);
        if(nova_home_cards[i])nyx_nova_reflect(nova_home_cards[i],i==nova_home_selected);
    }
}

static lv_obj_t *nova_home_action(lv_obj_t *parent, u32 index, lv_action_t action, int x, int y, int width, int height, bool hero)
{
    const nova_item *item = &nova_reference_home[index];
    lv_obj_t *card = nyx_nova_panel(parent, x, y, width, height);
    lv_cont_set_style(card, hero ? &nova_selected : &nova_panel);
    nova_home_cards[index]=card;
    if (!index) nyx_nova_launch_backdrop(card);

    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(button, width, height); lv_obj_set_pos(button, 0, 0); lv_obj_set_free_num(button, index);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    nova_home_icons[index] = nyx_nova_icon_box(card, item->icon, hero ? 32 : 22, hero ? 32 : 22, hero ? 82 : 44, hero);
    nyx_nova_custom_icon(nova_home_icons[index],index);
    int title_y = hero ? height - 184 : 84;
    lv_obj_t *title = nyx_nova_text(card, item->title, hero ? 32 : 22, title_y, width - (hero ? 64 : 44), hero ? NOVA_HEADING : NOVA_TITLE);
    nyx_nova_fit_text(title, hero ? 48 : 56);
    int description_width = hero ? 14 * lv_font_get_height(nova_text[NOVA_BODY].text.font) : width - 44;
    if (description_width > width - 64 && hero) description_width = width - 64;
    lv_obj_t *description = nyx_nova_text(card, item->description, hero ? 32 : 22, title_y + lv_obj_get_height(title) + 10, description_width, hero ? NOVA_BODY : NOVA_SMALL);
    lv_label_set_style(description, &nova_dim);
    nyx_nova_fit_text(description, hero ? 50 : height - lv_obj_get_y(description) - 20);
    if (hero) {
        lv_obj_t *launch = lv_btn_create(card, NULL); lv_btn_set_fit(launch, false, false); lv_btn_set_layout(launch, LV_LAYOUT_OFF);
        lv_btn_set_style(launch, LV_BTN_STYLE_REL, &nova_primary); lv_btn_set_style(launch, LV_BTN_STYLE_PR, &nova_primary);
        const char *caption = nyx_tr("Choisir un système");
        int launch_width = lv_txt_get_width(caption, strlen(caption), nova_text[NOVA_BODY].text.font, 0, LV_TXT_FLAG_NONE) + 72;
        if (launch_width > width - 64) launch_width = width - 64;
        lv_obj_set_size(launch, launch_width, 48); lv_obj_set_pos(launch, 32, height - 72);
        lv_obj_set_free_num(launch, index); lv_btn_set_action(launch, LV_BTN_ACTION_CLICK, action);
        lv_obj_t *label = nyx_nova_text(launch, "Choisir un système", 16, 13, launch_width - 64, NOVA_BODY);
        static lv_style_t ink_text; lv_style_copy(&ink_text, &nova_text[NOVA_BODY]); ink_text.text.color = LV_COLOR_HEX(0x0C1721);
        lv_label_set_style(label, &ink_text); nyx_nova_fit_text(label, 26);
        lv_obj_t *arrow = nyx_nova_glyph(launch, "arrow-up-right", launch_width - 40, 13, 24); lv_img_set_style(arrow, &nova_ink_image);
    } else {
        lv_obj_t *dot = lv_cont_create(card, NULL); lv_cont_set_style(dot, &nova_dot); lv_cont_set_fit(dot, false, false);
        lv_obj_set_size(dot, 5, 5); lv_obj_set_pos(dot, width - 22, 18); lv_obj_set_click(dot, false);
    }
    nyx_nova_reflect(card,hero);
    return button;
}

void nyx_nova_home(lv_obj_t *page, lv_action_t activate)
{
    nyx_nova_page(page, 1);
    int height = lv_obj_get_height(page) - 140 - 28;
    int width = LV_HOR_RES - 68, hero_width = (width - 22) * 11 / 21;
    nova_home_action(page, 0, activate, 34, 140, hero_width, height, true);
    int tile_width = (width - hero_width - 22 - 16) / 2, tile_height = (height - 16) / 2;
    for (u32 i = 1; i < 5; ++i) nova_home_action(page, i, activate,
        34 + hero_width + 22 + (i - 1) % 2 * (tile_width + 16), 140 + (i - 1) / 2 * (tile_height + 16), tile_width, tile_height, false);
}

static struct {
    u32 selected;
    lv_obj_t *track, *cards[5], *icons[5], *title, *detail, *open, *counter;
} home_carousel;

static void nova_carousel_update(void)
{
    u32 selected = home_carousel.selected;nova_home_selected=selected;
    lv_obj_set_x(home_carousel.track, 372 - selected * 344);
    for (u32 i = 0; i < 5; ++i) {
        lv_btn_set_style(home_carousel.cards[i], LV_BTN_STYLE_REL, i == selected ? &nova_selected : &nova_panel);
        nyx_nova_icon_select(home_carousel.icons[i], i == selected);
        nyx_nova_reflect(home_carousel.cards[i],i==selected);

    }
    lv_label_set_text(home_carousel.title, nova_reference_home[selected].title);
    lv_label_set_text(home_carousel.detail, nova_reference_home[selected].description);
    nyx_nova_fit_text(home_carousel.title, 44); nyx_nova_fit_text(home_carousel.detail, 58);
    lv_obj_set_free_num(home_carousel.open, selected);
    char position[16]; s_printf(position, "%d / 5", selected + 1); lv_label_set_text(home_carousel.counter, position);
}

static lv_res_t nova_carousel_select(lv_obj_t *button)
{
    u32 index = lv_obj_get_free_num(button);
    if (index < 5) { home_carousel.selected = index; nova_carousel_update(); }
    return LV_RES_OK;
}
static lv_res_t nova_carousel_step(lv_obj_t *button)
{
    home_carousel.selected = (home_carousel.selected + (lv_obj_get_free_num(button) ? 1 : 4)) % 5;
    nova_carousel_update(); return LV_RES_OK;
}

void nyx_nova_carousel(lv_obj_t *page, lv_action_t activate)
{
    nyx_nova_page(page, 1); home_carousel.selected = 0;
    for (u32 i = 0; i < 2; ++i) {
        lv_obj_t *arrow = lv_btn_create(page, NULL); lv_btn_set_fit(arrow, false, false); lv_btn_set_layout(arrow, LV_LAYOUT_OFF);
        lv_obj_set_size(arrow, 50, 60); lv_obj_set_pos(arrow, i ? 1196 : 34, 226);
        lv_obj_set_free_num(arrow, i); lv_btn_set_action(arrow, LV_BTN_ACTION_CLICK, nova_carousel_step);
        lv_obj_t *label = nyx_nova_text(arrow, i ? ">" : "<", 16, 16, 22, NOVA_TITLE); (void)label;
    }
    lv_obj_t *viewport = nyx_nova_panel(page, 108, 140, 1064, 244); lv_cont_set_style(viewport, &nova_clear);
    home_carousel.track = nyx_nova_panel(viewport, 0, 0, 5 * 344 - 24, 244); lv_cont_set_style(home_carousel.track, &nova_clear);
    for (u32 i = 0; i < 5; ++i) {
        lv_obj_t *card = lv_btn_create(home_carousel.track, NULL); home_carousel.cards[i] = card;nova_home_cards[i]=card;
        lv_btn_set_fit(card, false, false); lv_btn_set_layout(card, LV_LAYOUT_OFF);
        lv_btn_set_style(card, LV_BTN_STYLE_REL, &nova_panel); lv_btn_set_style(card, LV_BTN_STYLE_PR, &nova_selected);
        lv_obj_set_size(card, 320, 244); lv_obj_set_pos(card, i * 344, 0);
        if(!i)nyx_nova_launch_backdrop(card);
        lv_obj_set_free_num(card, i); lv_btn_set_action(card, LV_BTN_ACTION_CLICK, nova_carousel_select);
        home_carousel.icons[i] = nyx_nova_icon_box(card, nova_reference_home[i].icon, 124, 26, 72, i == 0);
        nyx_nova_custom_icon(home_carousel.icons[i],i);
        lv_obj_t *title = nyx_nova_text(card, nova_reference_home[i].title, 22, 128, 276, NOVA_TITLE); nyx_nova_fit_text(title, 54);
        lv_obj_t *description = nyx_nova_text(card, nova_reference_home[i].description, 22, 186, 276, NOVA_SMALL);
        lv_label_set_style(description, &nova_dim); nyx_nova_fit_text(description, 44);
    }
    lv_obj_t *detail = nyx_nova_panel(page, 108, 406, 1064, 146);
    home_carousel.title = nyx_nova_text(detail, "", 24, 20, 730, NOVA_TITLE);
    home_carousel.detail = nyx_nova_text(detail, "", 24, 66, 730, NOVA_BODY); lv_label_set_style(home_carousel.detail, &nova_dim);
    home_carousel.open = lv_btn_create(detail, NULL); lv_btn_set_fit(home_carousel.open, false, false); lv_btn_set_layout(home_carousel.open, LV_LAYOUT_OFF);
    lv_btn_set_style(home_carousel.open, LV_BTN_STYLE_REL, &nova_primary); lv_btn_set_style(home_carousel.open, LV_BTN_STYLE_PR, &nova_primary);
    lv_obj_set_size(home_carousel.open, 220, 54); lv_obj_set_pos(home_carousel.open, 820, 46);
    lv_btn_set_action(home_carousel.open, LV_BTN_ACTION_CLICK, activate);
    lv_obj_t *label = nyx_nova_text(home_carousel.open, "Open", 18, 14, 180, NOVA_BODY);
    static lv_style_t carousel_ink; lv_style_copy(&carousel_ink, &nova_text[NOVA_BODY]); carousel_ink.text.color = LV_COLOR_HEX(0x0C1721);
    lv_label_set_style(label, &carousel_ink);
    home_carousel.counter = nyx_nova_text(page, "", 34, 406, 62, NOVA_SMALL);
    nova_carousel_update();
}

typedef struct { lv_btn_ext_t button; lv_obj_t *track, *knob, *power, *state; } nova_toggle_ext;
static lv_res_t nova_toggle_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_toggle_signal_base(object, signal, parameter); }

bool nyx_nova_toggle_refresh(lv_obj_t *button)
{
    if (lv_obj_get_signal_func(button) == nova_card_signal) {
        nova_card_ext *card = lv_obj_get_ext_attr(button); nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
        if (ext->kind == NOVA_TOOLS && card->index == 6) {
            lv_cont_set_style(ext->dots[card->index], lv_btn_get_state(button) & LV_BTN_STATE_TGL_REL ? &nova_dot_on : &nova_dot);
            return true;
        }
        return false;
    }
    if (lv_obj_get_signal_func(button) != nova_toggle_signal) return false;
    nova_toggle_ext *ext = lv_obj_get_ext_attr(button); bool enabled = (lv_btn_get_state(button) & LV_BTN_STATE_TGL_REL) != 0;
    lv_cont_set_style(ext->track, enabled ? &nova_track_on : &nova_track_off);
    lv_cont_set_style(ext->knob, enabled ? &nova_thumb_on : &nova_thumb_off);
    lv_obj_set_x(ext->knob, enabled ? 35 : 5);
    lv_label_set_style(ext->state, enabled ? &nova_accent_text : &nova_dim);
    lv_label_set_text(ext->state, enabled ? "Actif" : "Inactif");
    if (status_bar.mid && nyx_options_has_changes()) {
        lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    }
    return true;
}

static lv_obj_t *nova_toggle_base(lv_obj_t *parent, lv_action_t action, bool enabled, int width, int height, int track_x, int track_y, int state_width)
{
    lv_obj_t *button = lv_btn_create(parent, NULL);
    if (!nova_toggle_signal_base) nova_toggle_signal_base = lv_obj_get_signal_func(button);
    nova_toggle_ext *ext = lv_obj_allocate_ext_attr(button, sizeof(*ext));
    if (!ext) { lv_obj_del(button); return NULL; }
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, lv_theme_get_current()->btn.pr);
    lv_btn_set_style(button, LV_BTN_STYLE_TGL_REL, lv_theme_get_current()->btn.tgl_rel); lv_btn_set_style(button, LV_BTN_STYLE_TGL_PR, lv_theme_get_current()->btn.tgl_pr);
    lv_obj_set_size(button, width, height); lv_btn_set_toggle(button, true); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    ext->track = lv_cont_create(button, NULL); lv_cont_set_style(ext->track, &nova_track_off); lv_cont_set_fit(ext->track, false, false);
    lv_obj_set_size(ext->track, 64, 34); lv_obj_set_pos(ext->track, track_x, track_y); lv_obj_set_click(ext->track, false);
    ext->knob = lv_cont_create(ext->track, NULL); lv_cont_set_style(ext->knob, &nova_thumb_off); lv_cont_set_fit(ext->knob, false, false);
    lv_obj_set_size(ext->knob, 24, 24); lv_obj_set_pos(ext->knob, 5, 5); lv_obj_set_click(ext->knob, false);
    ext->power = nyx_nova_glyph(ext->knob, "power", 3, 3, 18); lv_img_set_style(ext->power, &nova_ink_image);
    ext->state = nyx_nova_text(button, "", track_x + 72, track_y + 6, state_width, NOVA_SMALL); lv_label_set_style(ext->state, &nova_dim);
    lv_obj_set_signal_func(button, nova_toggle_signal); lv_btn_set_state(button, enabled ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_nova_toggle_refresh(button); return button;
}

lv_obj_t *nyx_nova_toggle(lv_obj_t *parent, const char *title, lv_action_t action, bool enabled, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *button = nova_toggle_base(parent, action, enabled, width, height, 14, height - 48, width - 100);
    if (!button) return NULL;
    lv_obj_t *label = nyx_nova_text(button, title, 12, 10, width - 24, NOVA_TITLE); nyx_nova_fit_text(label, height - 66);
    return button;
}

lv_obj_t *nyx_nova_switch_row(lv_obj_t *parent, const char *title, const char *help, const char *icon, lv_action_t action, bool enabled, int x, int y, int width, int height)
{
    int track_x = width - 158;
    lv_obj_t *button = nova_toggle_base(parent, action, enabled, width, height, track_x, (height - 34) / 2, 82);
    if (!button) return NULL;
    lv_obj_set_pos(button, x, y); nyx_nova_glyph(button, icon, 0, 18, 18);
    lv_obj_t *label = nyx_nova_text(button, title, 30, 12, track_x - 42, NOVA_BODY); nyx_nova_fit_text(label, 26);
    if (!strcmp(title, "Auto NoGC")) (lv_label_set_text)(label, title);
    lv_obj_t *description = nyx_nova_text(button, help, 30, 42, track_x - 42, NOVA_SMALL);
    lv_label_set_style(description, &nova_dim); nyx_nova_fit_text(description, height - 46);
    lv_obj_t *line = lv_cont_create(parent, NULL); lv_cont_set_style(line, &nova_separator); lv_cont_set_fit(line, false, false);
    lv_obj_set_size(line, width, 1); lv_obj_set_pos(line, x, y + height - 1); lv_obj_set_click(line, false);
    return button;
}

void nyx_nova_quick_actions(const lv_action_t actions[4])
{
    for (u32 i = 0; i < 4; ++i) {
        nova_quick_actions[i] = actions[i];
        if (nova_quick_buttons[i]) lv_btn_set_action(nova_quick_buttons[i], LV_BTN_ACTION_CLICK, actions[i]);
    }
}

void nyx_nova_navigation_lock(bool lock)
{
    if(lock)++nova_navigation_locks;else if(nova_navigation_locks)--nova_navigation_locks;
    bool enabled=!nova_navigation_locks;
    if(nova_brand_button)lv_obj_set_click(nova_brand_button,enabled);
    for(u32 i=1;i<6;++i)if(nova_nav_buttons[i])lv_obj_set_click(nova_nav_buttons[i],enabled);
    for(u32 i=0;i<4;++i)if(nova_quick_buttons[i]){
        lv_obj_set_click(nova_quick_buttons[i],enabled);
        lv_obj_t *glyph=lv_obj_get_child(nova_quick_buttons[i],NULL);if(glyph)lv_img_set_style(glyph,enabled?&nova_image:&nova_image_dim);
    }
    if(nova_tabs)nyx_nova_navigation_select(lv_tabview_get_tab_act(nova_tabs));
}

lv_color_t nyx_nova_color(u32 role)
{
    if (role == NOVA_COLOR_LOGO) return LV_COLOR_HEX(n_cfg.nova_logo_color);
    if (role == NOVA_COLOR_ICONS) return LV_COLOR_HEX(n_cfg.nova_icon_color);
    if (role == NOVA_COLOR_SELECTION) return LV_COLOR_HEX(n_cfg.nova_selection_color);
    if (role == NOVA_COLOR_TEXT) return LV_COLOR_HEX(n_cfg.nova_text_color);
    if (role == NOVA_COLOR_BORDER) return LV_COLOR_HEX(n_cfg.nova_border_color);
    if (role == NOVA_COLOR_REFLECTION) return LV_COLOR_HEX(n_cfg.nova_reflection_color);
    return LV_COLOR_HEX(n_cfg.theme_bg == 0x141A19 ? 0x10151D : n_cfg.theme_bg);
}

/* Visual bands only: no changes to fan policy or hardware protection. */
u32 nyx_nova_temperature_rgb(u32 degrees)
{ return degrees < 45 ? 0x8AE6B6 : degrees < 55 ? 0xF3D36A : degrees < 65 ? 0xF4A261 : 0xFF6B6B; }

void nyx_nova_about(lv_obj_t *page)
{
    nyx_nova_page(page,0);
    static const char *titles[]={"Créateurs","Remerciements","Logiciels libres","Changelog"};
    static const char *icons[]={"brand-mark","badge-check","layers-2"};
    static const char *credits[]={
        "hekate\n(c) 2018, naehrwert, st4rk\n(c) 2018-2026, CTCaer\n\nNyx\n(c) 2019-2026, CTCaer\n\nHekate 6.5.5 / Nyx 1.9.5",
        "derrek, nedwill, plutoo\nshuffle2, smea, thexyz\nyellows8\n\nfincs, hexkyz, SciresM\nShiny Quagsire, WinterMute\nswitchbrew",
        "Littlev Graphics Library\n(c) 2016-2018, Gabor Kiss-Vamosi\nFatFs R0.13c\n(c) 2006-2018, ChaN\n(c) 2018-2022, CTCaer\nbcl-1.2.0\n(c) 2003-2006, Marcus Geelnard\nblz (c) 2018, SciresM\nelfload (c) 2014, Owen Shepherd\n(c) 2018, M4xw\n\nLodePNG 20261102\n(c) 2005-2026, Lode Vandevenne\nBTstack (c) BlueKitchen GmbH\nLicence personnelle / non commerciale"
    };
    (void)icons;
    /* A real card beneath the transparent tabs keeps wallpaper behind credits. */
    nyx_nova_panel(page,28,134,1224,378);
    lv_obj_t *tabs=lv_tabview_create(page,NULL);
    lv_obj_set_size(tabs,1212,366);lv_obj_set_pos(tabs,34,140);lv_tabview_set_sliding(tabs,false);
    for(u32 i=0;i<5;++i){
        lv_obj_t *section=lv_tabview_add_tab(tabs,i?titles[i-1]:"Nova Pro");
        nyx_nova_fixed_page(section);
        if(i==4){
            static const char *changes[]={
                "v29 · Pairing and reconnection\nValidated standard host profiles accepted.\nSD pairing backup and persistent reconnect attempts.",
                "v20 · TV / Dock\nAutomatic dock detection.\nTV only or TV + console.",
                "v19 · Persistent TV output\nTV output stays active across menus.",
                "v18 · Customization\nPNG icons and SD partition menus."
            };
            for(u32 j=0;j<4;j++){
                lv_obj_t *entry=nyx_nova_text(section,changes[j],20+(j%2)*606,12+(j/2)*142,566,NOVA_BODY);
                nyx_nova_fit_text(entry,132);
            }
            continue;
        }
        const char *text=i?credits[i-1]:
            "Hekate Nova Pro " NYX_NOVA_VERSION "\n\n"
            "Retrogen : conception et adaptation\n"
            "ChatGPT : assistance à la conception et à l'adaptation\n\n"
            "Basé sur Hekate 6.5.5 / Nyx 1.9.5\n"
            "Les crédits et licences d'origine sont conservés dans les autres onglets.";
        if(i==3){
            const char *second=strstr(text,"bcl-1.2.0");char first[512];
            u32 length=second?(u32)(second-text):strlen(text);if(length>=sizeof(first))length=sizeof(first)-1;
            memcpy(first,text,length);first[length]=0;
            nyx_nova_text(section,first,20,12,566,NOVA_SMALL);
            if(second)nyx_nova_text(section,second,626,12,566,NOVA_SMALL);
        }else nyx_nova_text(section,text,20,12,1172,NOVA_SMALL);
    }
    lv_obj_t *resources=nyx_nova_text(page,"HWTest-RCM 0.15 · Noto Sans (OFL) · Lucide (ISC) · Nova Pro (GPL)",34,520,1212,NOVA_SMALL);nyx_nova_fit_text(resources,44);
}

const char *nyx_nova_caption(const char *text)
{
    if(!text)return "";
    uint32_t at=0;u32 cp=lv_txt_encoded_next(text,&at);
    if(cp>=0xE000&&cp<=0xF8FF){text+=at;while(*text==' ')++text;}
    return text;
}
static lv_res_t nova_action_help(lv_obj_t *button)
{
    const char *help=lv_obj_get_free_ptr(lv_obj_get_parent(button));if(help&&*help)nyx_show_help(help);return LV_RES_OK;
}
typedef struct {lv_cont_ext_t container;char *help;} nova_action_ext;
static lv_signal_func_t nova_action_base_signal;
static lv_res_t nova_action_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    if(signal==LV_SIGNAL_CLEANUP){nova_action_ext *ext=lv_obj_get_ext_attr(object);free(ext->help);ext->help=NULL;}
    return nova_action_base_signal(object,signal,parameter);
}
lv_obj_t *nyx_nova_action(lv_obj_t *parent,const char *title,const char *description,const char *icon,
    lv_action_t action,u32 id,int x,int y,int width,int height)
{
    lv_obj_t *card=nyx_nova_panel(parent,x,y,width,height);
    if(!nova_action_base_signal)nova_action_base_signal=lv_obj_get_signal_func(card);
    nova_action_ext *ext=lv_obj_allocate_ext_attr(card,sizeof(*ext));
    if(ext){ext->help=NULL;if(description&&*description){ext->help=malloc(strlen(description)+1);if(ext->help)strcpy(ext->help,description);}lv_obj_set_free_ptr(card,ext->help);lv_obj_set_signal_func(card,nova_action_signal);}
    lv_obj_t *button=lv_btn_create(card,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    for(u32 state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;++state)lv_btn_set_style(button,state,&nova_clear);
    lv_obj_set_size(button,width,height);lv_obj_set_pos(button,0,0);lv_obj_set_free_num(button,id);
    lv_btn_set_action(button,LV_BTN_ACTION_CLICK,action);lv_btn_set_action(button,LV_BTN_ACTION_LONG_PR,nova_action_help);
    lv_obj_t *label=nyx_nova_text(button,nyx_nova_caption(title),76,18,width-94,NOVA_BODY);nyx_nova_fit_text(label,height<120?44:58);
    nyx_nova_icon_box(card,icon,18,20,42,false);
    if(description&&*description){int top=22+lv_obj_get_height(label);lv_obj_t *detail=nyx_nova_text(card,description,76,top,width-94,NOVA_SMALL);lv_label_set_style(detail,&nova_dim);nyx_nova_fit_text(detail,height-top-16);}
    return button;
}
/* Conservative large-font label fitting. SPDX-License-Identifier: GPL-2.0-or-later */

static lv_task_t *pending;
static bool refreshing;

static bool is_type(lv_obj_t *object, const char *name)
{
    lv_obj_type_t type;
    lv_obj_get_type(object, &type);
    return type.type[0] && !strcmp(type.type[0], name);
}

void nyx_label_adapt(lv_obj_t *label)
{
    if (!label || !is_type(label, "lv_label")) return;
    lv_obj_t *parent = lv_obj_get_parent(label);
    if (!parent || !is_type(parent, "lv_btn") || lv_cont_get_hor_fit(parent) || lv_obj_get_width(parent) < 160) return;
    lv_label_long_mode_t mode = lv_label_get_long_mode(label);
    if (mode != LV_LABEL_LONG_EXPAND && mode != LV_LABEL_LONG_ROLL) return;
    const char *text = lv_label_get_text(label);
    if (!text || !text[0] || strchr(text, '\n')) return;
    lv_style_t *style = lv_label_get_style(label);
    if (!style || !style->text.font) return;
    lv_coord_t height = lv_font_get_height(style->text.font);
    if (lv_obj_get_height(parent) < height) return;

    /* Fixed action buttons with one label, or the existing title/state rows.
     * Cards, compound controls and icon-bearing rows retain their own layout. */
    lv_obj_t *other = NULL, *child = NULL;
    u32 count = 0;
    while ((child = lv_obj_get_child(parent, child))) {
        if (++count > 2 || !is_type(child, "lv_label")) return;
        if (child != label) other = child;
    }
    if (!count) return;
    lv_style_t *parent_style = lv_obj_get_style(parent);
    lv_coord_t padding = parent_style ? parent_style->body.padding.hor : 12;
    if (padding < 12) padding = 12;
    lv_coord_t x = padding, available = lv_obj_get_width(parent) - 2 * padding;
    if (other) {
        lv_coord_t delta_y = lv_obj_get_y(other) - lv_obj_get_y(label);
        if (delta_y < -height || delta_y > height || lv_obj_get_x(other) <= lv_obj_get_x(label)) return;
        x = lv_obj_get_x(label);
        if (x < padding) x = padding;
        available = lv_obj_get_x(other) - x - 12;
    }
    if (available < 48) return;
    lv_txt_flag_t flags = lv_label_get_recolor(label) ? LV_TXT_FLAG_RECOLOR : LV_TXT_FLAG_NONE;
    lv_coord_t width = lv_txt_get_width(text, strlen(text), style->text.font, style->text.letter_space, flags);
    if (width <= available) return;
    /* A status-bar update must not restart every other scrolling label. */
    if (mode == LV_LABEL_LONG_ROLL && lv_obj_get_width(label) == available) return;
    /* Rolling keeps the full translated text available without increasing a
     * row's height and colliding with controls positioned below it. */
    lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
    lv_obj_set_width(label, available);
    if (lv_btn_get_layout(parent) == LV_LAYOUT_OFF) lv_obj_set_x(label, x);
}

static void visit(lv_obj_t *root, u32 depth, u32 *remaining)
{
    if (!root || depth > 32 || !*remaining || lv_obj_get_hidden(root)) return;
    --*remaining;
    if (is_type(root, "lv_label")) nyx_label_adapt(root);
    lv_obj_t *child = NULL;
    while (*remaining && (child = lv_obj_get_child(root, child))) visit(child, depth + 1, remaining);
}
void nyx_readability_refresh(lv_obj_t *root)
{
    u32 remaining = 2500;
    visit(root, 0, &remaining);
}
static void refresh_task(void *context)
{
    (void)context;
    pending = NULL;
    refreshing = true;
    nyx_readability_refresh(lv_scr_act());
    refreshing = false;
}
void nyx_readability_request(void)
{
    if (pending || refreshing) return;
    pending = lv_task_create(refresh_task, 1, LV_TASK_PRIO_LOW, NULL);
    if (pending) { lv_task_once(pending); lv_task_ready(pending); }
}
/* Visible, stable-index choices without dropdown overlays. */
/* SPDX-License-Identifier: GPL-2.0-or-later */

typedef struct {
    lv_cont_ext_t container;
    lv_action_t action;
} choices_ext;

void nyx_choices_set_selected(lv_obj_t *choices, u32 selected)
{
    lv_obj_t *button = NULL;
    while ((button = lv_obj_get_child(choices, button)))
        lv_btn_set_state(button, lv_obj_get_free_num(button) == selected ?
            LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
}

static lv_res_t choice_pressed(lv_obj_t *button)
{
    lv_obj_t *choices = lv_obj_get_parent(button);
    choices_ext *ext = lv_obj_get_ext_attr(choices);
    lv_action_t action = ext->action;
    nyx_background_interaction();
    nyx_choices_set_selected(choices, lv_obj_get_free_num(button));
    /* The action may replace the window: do not access its objects afterward. */
    return action ? action(button) : LV_RES_OK;
}

lv_obj_t *nyx_choices_create(lv_obj_t *parent, const char *options, u32 selected,
    u32 columns, lv_coord_t width, lv_coord_t row_height, lv_action_t action,
    u32 group, bool raw)
{
    if (!columns) columns = 1;
    lv_obj_t *choices = lv_cont_create(parent, NULL);
    choices_ext *ext = lv_obj_allocate_ext_attr(choices, sizeof(*ext));
    if (!ext) { lv_obj_del(choices); return NULL; }
    ext->action = action;
    lv_cont_set_style(choices, &lv_style_transp_tight);
    lv_cont_set_fit(choices, false, false);
    lv_cont_set_layout(choices, LV_LAYOUT_OFF);
    lv_obj_set_click(choices, false);
    lv_obj_set_free_num(choices, group);
    lv_coord_t cell_width = (width - (columns - 1) * 8) / columns;
    u32 count = 0;
    for (const char *at = options; at && *at;) {
        const char *end = strchr(at, '\n');
        u32 bytes = end ? (u32)(end - at) : strlen(at);
        char text[256];
        if (bytes >= sizeof(text)) bytes = sizeof(text) - 1;
        memcpy(text, at, bytes); text[bytes] = 0;
        lv_obj_t *button = lv_btn_create(choices, NULL);
        lv_btn_set_fit(button, false, false);
        lv_btn_set_layout(button, LV_LAYOUT_CENTER);
        lv_obj_set_size(button, cell_width, row_height);
        lv_obj_set_pos(button, count % columns * (cell_width + 8), count / columns * (row_height + 8));
        lv_obj_set_free_num(button, count);
        lv_btn_set_action(button, LV_BTN_ACTION_CLICK, choice_pressed);
        lv_obj_t *label = lv_label_create(button, NULL);
        lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        lv_obj_set_width(label, cell_width - 16);
        if (raw) (lv_label_set_text)(label, text);
        else lv_label_set_text(label, text);
        /* A wide face may wrap a translated option to two lines. Keep its
         * family and use the caption size before resorting to a marquee. */
        if (lv_obj_get_height(label) > row_height - 8)
            lv_label_set_style(label, &hint_small_style);
        if (lv_obj_get_height(label) > row_height - 8) {
            lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
            lv_obj_set_height(label, lv_font_get_height(hint_small_style.text.font));
        }
        ++count;
        if (!end) break;
        at = end + 1;
    }
    lv_obj_set_size(choices, width, count ? ((count + columns - 1) / columns) * (row_height + 8) - 8 : 0);
    nyx_choices_set_selected(choices, selected);
    return choices;
}

void nyx_show_help(const char *message)
{
    static const char *buttons[] = {"\251", "\222OK", "\251", ""};
    lv_obj_t *box = lv_mbox_create(lv_scr_act(), NULL);
    lv_obj_set_width(box, 960);
    lv_mbox_set_recolor_text(box, true);
    lv_mbox_set_text(box, message);
    lv_mbox_add_btns(box, buttons, NULL);
    lv_obj_align(box, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_top(box, true);
}
/* Shared Nova action grids. SPDX-License-Identifier: GPL-2.0-or-later */

void nyx_action_grid_init(nyx_action_grid *grid, lv_theme_t *theme, lv_obj_t *page, u32 kind)
{
    memset(grid, 0, sizeof(*grid)); grid->theme = theme; grid->page = page;
    nyx_nova_grid_begin(page, kind);
}

lv_obj_t *nyx_action_grid_add(nyx_action_grid *grid, const char *title,
    const char *description, lv_action_t action, lv_action_t long_action, u32 id)
{
    (void)title;
    /* Captions/icons come verbatim from the mockup. Full original help,
     * action pointers and destructive-action IDs remain attached. */
    ++grid->count;
    return nyx_nova_grid_add(grid->page, description, action, long_action, id);
}

void nyx_action_grid_finish(nyx_action_grid *grid) { nyx_nova_grid_finish(grid->page); }
/* Unified language/readability controls and restrained native backgrounds.
 * SPDX-License-Identifier: GPL-2.0-or-later */

/* Static wallpaper only. Compatibility hooks now control the selected-tile
 * sheen; waves, particles and aurora are removed from this build. */
void nyx_background_create(lv_obj_t *screen) { nyx_nova_backdrop(screen, 0); }
void nyx_background_pause(bool pause) { nyx_nova_reflection_pause(pause); }
void nyx_background_interaction(void) { nyx_nova_reflection_interaction(); }

static nyx_config appearance_pending;
static lv_obj_t *appearance_parent_close;
static bool appearance_open;
static lv_obj_t *appearance_tabs, *appearance_language_choice, *appearance_size_choice;

enum { AP_LANGUAGE, AP_FONT, AP_SIZE, AP_LAYOUT };
static u32 bounded(u32 value,u32 low,u32 high,u32 fallback) { return value>=low&&value<=high?value:fallback; }

static void appearance_notice(const char *message)
{
    static const char *buttons[] = {"\251", "\222OK", "\251", ""};
    lv_obj_t *box = lv_mbox_create(lv_scr_act(), NULL);
    lv_obj_set_width(box, 840);
    lv_mbox_set_text(box, message);
    lv_mbox_add_btns(box, buttons, NULL);
    lv_obj_align(box, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_top(box, true);
}

static lv_res_t appearance_close(lv_obj_t *button)
{
    appearance_open = false;
    lv_res_t result = nyx_win_close_action(button);
    close_btn = appearance_parent_close; appearance_parent_close = NULL;
    nyx_options_appearance_active(false, false);
    return result;
}

static lv_res_t appearance_set_selection(u32 id, u32 selected)
{
    switch (id) {
    case AP_LANGUAGE:
        appearance_pending.language = selected < I18N_LANGUAGE_COUNT ? selected : I18N_EN; break;
    case AP_FONT: {
        if (selected >= nyx_font_count()) return LV_RES_OK;
        const char *file = nyx_font_filename(selected);
        if (!file || strlen(file) >= sizeof(appearance_pending.font_file)) return LV_RES_OK;
        strcpy(appearance_pending.font_file, file); break;
    }
    case AP_SIZE: appearance_pending.font_size = selected <= 8 ? 20 + selected * 2 : 30; break;
    case AP_LAYOUT: appearance_pending.home_layout = selected == HOME_LAYOUT_CAROUSEL ? selected : HOME_LAYOUT_TILES; break;
    }
    return LV_RES_OK;
}

static void appearance_apply(const nyx_config *settings)
{
    n_cfg.language = settings->language;
    memcpy(n_cfg.font_file, settings->font_file, sizeof(n_cfg.font_file));
    n_cfg.font_size = settings->font_size;
    n_cfg.home_layout = settings->home_layout;
}

static lv_res_t appearance_save(lv_obj_t *button)
{
    (void)button;
    bool mounted_before = sd_get_card_mounted();
    if (sd_mount()) { appearance_notice("SD card unavailable. Nothing was saved."); return LV_RES_OK; }
    if (f_stat("bootloader/sys/nyx.bin", NULL) != FR_OK) {
        if (!mounted_before) sd_unmount();
        appearance_notice("Nyx is missing. Nothing was saved."); return LV_RES_OK;
    }
    if (appearance_pending.font_file[0] && !nyx_font_available_at_size(appearance_pending.font_file, appearance_pending.font_size)) {
        if (!mounted_before) sd_unmount();
        appearance_notice("The selected font is missing or invalid. Choose another font."); return LV_RES_OK;
    }
    nyx_config previous = n_cfg;
    if (!nyx_options_commit_boot()) {
        if (!mounted_before) sd_unmount();
        appearance_notice("Could not save settings. Check the SD card and retry."); return LV_RES_OK;
    }
    appearance_apply(&appearance_pending);
    if (create_nyx_config_entry(true)) {
        appearance_apply(&previous);
        appearance_notice("Could not save appearance settings. Check the SD card and try again.");
        return LV_RES_OK;
    }
    nyx_options_appearance_active(true, true);
    reload_nyx(NULL, false);
    appearance_notice("Settings saved. Reload Nyx to apply the changes.");
    return LV_RES_OK;
}

static lv_obj_t *appearance_text(lv_obj_t *parent, const char *text,
    int x, int y, int width, bool small)
{
    return nyx_nova_text(parent,text,x,y,width,small?NOVA_SMALL:NOVA_TITLE);
}

static lv_obj_t *appearance_font_name, *appearance_font_counter;

static lv_res_t appearance_select(lv_obj_t *button)
{
    return appearance_set_selection(lv_obj_get_free_num(lv_obj_get_parent(button)), lv_obj_get_free_num(button));
}

static lv_res_t appearance_font_step(lv_obj_t *button)
{
    u32 count = nyx_font_count();
    if (!count) return LV_RES_OK;
    u32 current = nyx_font_selected_index(appearance_pending.font_file);
    current = (current + (lv_obj_get_free_num(button) ? 1 : count - 1)) % count;
    appearance_set_selection(AP_FONT, current);
    const char *file = nyx_font_filename(current);
    (lv_label_set_text)(appearance_font_name, file && *file ? file : nyx_tr("Default font"));
    char counter[24]; s_printf(counter, "%d / %d", current + 1, count);
    (lv_label_set_text)(appearance_font_counter, counter);
    nyx_background_interaction();
    return LV_RES_OK;
}

static void appearance_page(lv_obj_t *page)
{
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF); lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, lv_obj_get_width(page));
    lv_page_set_scrl_height(page, lv_obj_get_height(page));
    lv_page_set_sb_mode(page, LV_SB_MODE_OFF);
    lv_obj_set_pos(lv_page_get_scrl(page), 0, 0);
    lv_obj_set_drag(lv_page_get_scrl(page), false);
}

static lv_obj_t *appearance_choices(lv_obj_t *page, const char *options, u32 selected,
    u32 columns, int x, int y, int width, int height, u32 id, bool raw)
{
    lv_obj_t *choices = nyx_choices_create(page, options, selected, columns, width, height, appearance_select, id, raw);
    if (choices) lv_obj_set_pos(choices, x, y);
    return choices;
}

static void appearance_step_button(lv_obj_t *page, const char *title, int x, u32 next)
{
    lv_obj_t *button = lv_btn_create(page, NULL);
    lv_btn_set_fit(button, false, false); lv_obj_set_size(button, 246, 64);
    lv_obj_set_pos(button, x, 210); lv_obj_set_free_num(button, next);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, appearance_font_step);
    lv_obj_t *label = lv_label_create(button, NULL); lv_label_set_text(label, title);
}

lv_res_t nyx_create_appearance_window(lv_obj_t *button)
{
    (void)button;
    if (appearance_open) return LV_RES_OK;
    appearance_open = true; appearance_pending = n_cfg;
    if (!nyx_font_selected_index(n_cfg.font_file)) appearance_pending.font_file[0] = 0;
    appearance_parent_close = close_btn;
    lv_obj_t *win = nyx_create_standard_window("Appearance", appearance_close);
    lv_win_add_btn(win, NULL, SYMBOL_SAVE" Save & reload", appearance_save);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win); appearance_page(ext->page);
    lv_obj_t *tabs = lv_tabview_create(ext->page, NULL);
    appearance_tabs = tabs;
    lv_obj_set_size(tabs, lv_obj_get_width(ext->page), lv_obj_get_height(ext->page));
    lv_obj_set_pos(tabs, 0, 0); lv_tabview_set_sliding(tabs, false);
    lv_obj_t *language = lv_tabview_add_tab(tabs, "Language"); appearance_page(language);
    lv_obj_t *reading = lv_tabview_add_tab(tabs, "Font"); appearance_page(reading);
    lv_obj_t *layout = lv_tabview_add_tab(tabs, "Layout"); appearance_page(layout);
    int height=lv_obj_get_height(language)-28;
    lv_obj_t *language_card=nyx_nova_panel(language,34,14,1212,height);
    appearance_text(language_card, "Interface language", 24, 20, 1164, false);
    appearance_language_choice = appearance_choices(language_card, "English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês",
        bounded(n_cfg.language, 0, I18N_LANGUAGE_COUNT - 1, I18N_EN), 3, 24, 78, 1164, 82, AP_LANGUAGE, true);
    lv_obj_t *note=appearance_text(language_card, "Save & reload applies all choices. Closing cancels the pending changes.", 24, height-62, 1164, true);nyx_nova_fit_text(note,52);
    lv_obj_t *font_card=nyx_nova_panel(reading,34,14,594,height);
    lv_obj_t *size_card=nyx_nova_panel(reading,652,14,594,height);
    appearance_text(font_card, "Font", 24, 20, 546, false);
    const char *file = appearance_pending.font_file;
    appearance_font_name = appearance_text(font_card, "", 24, 82, 546, true);
    (lv_label_set_text)(appearance_font_name, *file ? file : nyx_tr("Default font"));
    appearance_step_button(font_card, "<", 24, 0); appearance_step_button(font_card, ">", 294, 1);
    u32 count = nyx_font_count();
    char counter[24]; s_printf(counter, "%d / %d", nyx_font_selected_index(file) + 1, count);
    appearance_font_counter = appearance_text(font_card, counter, 24, 286, 546, true);
    note=appearance_text(font_card, "To add a font, copy a Nyx font file into /fonts on the SD card, then reload Nyx.", 24, 332, 546, true);nyx_nova_fit_text(note,height-344);
    appearance_text(size_card, "Text size", 24, 20, 546, false);
    u32 size = bounded(n_cfg.font_size, 20, 36, 30);
    appearance_size_choice = appearance_choices(size_card, "20\n22\n24\n26\n28\n30\n32\n34\n36", (size - 20) / 2, 3, 24, 82, 546, 82, AP_SIZE, true);
    lv_obj_t *layout_card=nyx_nova_panel(layout,34,14,1212,height);
    appearance_text(layout_card, "Home layout", 24, 20, 1164, false);
    appearance_choices(layout_card, "Tiles\nCarousel", n_cfg.home_layout == HOME_LAYOUT_CAROUSEL ? 1 : 0,
        2, 24, 104, 1164, 152, AP_LAYOUT, false);
    note=appearance_text(layout_card, "Save & reload applies all choices. Closing cancels the pending changes.", 24, height-62, 1164, true);nyx_nova_fit_text(note,52);
    nyx_options_appearance_active(true, false);
    return LV_RES_OK;
}

void nyx_appearance_open_language(u32 language)
{
    nyx_create_appearance_window(NULL);
    appearance_set_selection(AP_LANGUAGE, language);
    nyx_choices_set_selected(appearance_language_choice, appearance_pending.language);
    lv_tabview_set_tab_act(appearance_tabs, 0, false);
}

void nyx_appearance_open_font(void)
{
    nyx_create_appearance_window(NULL); lv_tabview_set_tab_act(appearance_tabs, 1, false);
}

void nyx_appearance_open_size(u32 pixels)
{
    nyx_create_appearance_window(NULL);
    u32 index = pixels >= 20 && pixels <= 36 && !(pixels & 1) ? (pixels - 20) / 2 : 5;
    appearance_set_selection(AP_SIZE, index); nyx_choices_set_selected(appearance_size_choice, index);
    lv_tabview_set_tab_act(appearance_tabs, 1, false);
}
void create_tab_tools(lv_theme_t *th, lv_obj_t *parent)
{
    nyx_action_grid grid;
    nyx_action_grid_init(&grid, th, parent, NOVA_TOOLS);
    nyx_action_grid_add(&grid, SYMBOL_UPLOAD"  Backup eMMC",
        "Back up eMMC or emuMMC partitions to the SD card, individually or as a raw image.",
        create_window_backup_restore_tool, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_DOWNLOAD"  Restore eMMC",
        "Restore eMMC or emuMMC from an SD backup. Existing data may be overwritten.",
        create_window_backup_restore_tool, NULL, 1);
    nyx_action_grid_add(&grid, SYMBOL_SD"  Partition SD Card",
        "Partition the SD card for emuMMC, Android or Linux. Hold to open eMMC partition tools.",
        create_window_sd_partition_manager, create_window_emmc_partition_manager, 0);
    nyx_action_grid_add(&grid, SYMBOL_USB"  USB Tools",
        "Expose SD, eMMC or emuMMC as USB storage, or use the console as a USB gamepad.",
        _create_window_usb_tools, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_DIRECTORY"  Fix Archive Bit",
        "Repair archive attributes, including Nintendo and emuMMC folders.",
        _create_window_unset_abit_tool, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_KEYBOARD"  Calibrate Touchscreen",
        "Calibrate the touch panel to correct input problems in Nyx or HOS.",
        _create_mbox_fix_touchscreen, NULL, 0);
    autorcm_btn = nyx_action_grid_add(&grid, SYMBOL_REFRESH"  AutoRCM #00FFC9   ON #",
        h_cfg.rcm_patched ?
            "AutoRCM is disabled because this console is patched." :
            "Enter RCM without a jig. Modifies the BCT: a custom bootloader is required to boot.",
        _create_mbox_autorcm_status, NULL, 0);
    // Keep the original state guard and one-label contract for the toggle.
    lv_btn_set_state(autorcm_btn, get_set_autorcm_status(false) ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(autorcm_btn);
    if (h_cfg.rcm_patched) {
        lv_obj_set_click(autorcm_btn, false);
        lv_btn_set_state(autorcm_btn, LV_BTN_STATE_INA);
    }
    nyx_action_grid_add(&grid, SYMBOL_MODULES"  Dump Package1/2",
        "Decrypt and extract Package1, Package2 and their components, including KIP1.",
        _create_window_dump_pk12_tool, NULL, 0);
    nyx_action_grid_finish(&grid);
}
void create_tab_info(lv_theme_t *th, lv_obj_t *parent)
{
    nyx_action_grid grid;
    nyx_action_grid_init(&grid, th, parent, NOVA_INFO);
    nyx_action_grid_add(&grid, SYMBOL_CIRCUIT"  HW & Fuses",
        "View hardware identity, cached fuses, DRAM, display and touch-panel information.",
        _create_window_hw_info_status, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_CHIP"  eMMC",
        "View eMMC identification, partitions and read-speed benchmarks.",
        _create_window_emmc_info_status, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_SD"  microSD",
        "View microSD identification, partitions and read-speed benchmarks.",
        _create_window_sdcard_info_status, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_BATTERY_FULL"  Battery",
        "View battery, charger and USB-PD information, or dump charger registers.",
        _create_window_battery_status, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_CHIP"  Bootrom",
        "Inspect Ipatches and dump patched or unpatched BootROM.",
        _create_window_bootrom_info_status, NULL, 0);
    nyx_action_grid_add(&grid, SYMBOL_SHUFFLE"  KFuses",
        "Dump cached KFuses containing HDCP key data.",
        _kfuse_dump_window_action, NULL, 0);
    bool lockpick_found = _lockpick_exists_check();
    lv_obj_t *lockpick = nyx_action_grid_add(&grid, SYMBOL_KEY"  Lockpick",
        lockpick_found ? "Launch Lockpick_RCM from the payloads folder." :
            "bootloader/payloads/Lockpick_RCM.bin is missing or outdated.",
        _create_mbox_lockpick, NULL, 0);
    if (!lockpick_found) {
        lv_btn_set_state(lockpick, LV_BTN_STATE_INA);
        lv_obj_set_click(lockpick, false);
    }
    nyx_action_grid_finish(&grid);
}
lv_obj_t *nyx_create_standard_window(const char *win_title, lv_action_t close_action)
{
	static lv_style_t win_bg_style;

	lv_style_copy(&win_bg_style, &lv_style_plain);
	win_bg_style.body.main_color = lv_theme_get_current()->bg->body.main_color;
	win_bg_style.body.grad_color = win_bg_style.body.main_color;
	win_bg_style.body.opa = LV_OPA_COVER;
	win_bg_style.body.padding.hor = 0;
	win_bg_style.body.padding.ver = 0;
	win_bg_style.body.padding.inner = 0;

	lv_obj_t *win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, win_title);
	lv_win_set_style(win, LV_WIN_STYLE_BG, &win_bg_style);
	/* The persistent status bar lives on the top layer. Keep the window's
	 * page inside it, rather than drawing/scrolling controls behind the bar. */
	nyx_nova_window(win);
	lv_win_ext_t *content = lv_obj_get_ext_attr(win);
	lv_page_set_style(content->page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
	lv_page_set_style(content->page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
	lv_obj_set_pos(lv_page_get_scrl(content->page), 0, 0);

	if (!close_action)
		close_btn = lv_win_add_btn(win, NULL, SYMBOL_CLOSE" Close", nyx_win_close_action);
	else
		close_btn = lv_win_add_btn(win, NULL, SYMBOL_CLOSE" Close", close_action);

	return win;
}
lv_res_t nyx_generic_onoff_toggle(lv_obj_t *btn)
{
	if (nyx_nova_toggle_refresh(btn)) return LV_RES_OK;
	lv_obj_t *label_btn = lv_obj_get_child(btn, NULL);
	lv_obj_t *label_btn2 = lv_obj_get_child(btn, label_btn);

	const char *state = nyx_tr((lv_btn_get_state(btn) & LV_BTN_STATE_TGL_REL) ? "ON" : "OFF");
	const char *color = (lv_btn_get_state(btn) & LV_BTN_STATE_TGL_REL) ? text_color : "#D0D0D0";
	if (!label_btn2)
	{
		const char *current = lv_label_get_text(label_btn);
		/* The final recolored span is the state. Byte counts vary by locale. */
		u32 prefix = strlen(current);
		if (prefix && current[prefix - 1] == '#') {
			--prefix;
			while (prefix && current[prefix - 1] != '#') --prefix;
			if (prefix) --prefix;
		}
		char *label_text = malloc(prefix + strlen(color) + strlen(state) + 8);
		if (label_text) {
			memcpy(label_text, current, prefix); label_text[prefix] = 0;
			strcat(label_text, color); strcat(label_text, "    ");
			strcat(label_text, state); strcat(label_text, "#");
			lv_label_set_text(label_btn, label_text); free(label_text);
		}
	}
	else
	{
		char *label_text = malloc(strlen(color) + strlen(state) + 4);
		if (label_text) {
			strcpy(label_text, color); strcat(label_text, " ");
			strcat(label_text, state); strcat(label_text, "#");
			lv_label_set_text(label_btn, label_text); free(label_text);
		}
	}

	return LV_RES_OK;
}

#ifndef NOVA_BT_H
#define NOVA_BT_H
/* Fixed-width ABI: BDK and BTstack use different definitions of bool. */
typedef struct {
    uint8_t valid, mac[6], host[6], key[16]; /* key in HCI wire order */
} nova_bt_peer;
typedef struct {
    uint32_t buttons, updated;
    uint16_t x, y;
    uint8_t connected, battery;
    uint8_t report_id, mode_ack, hid_open, last_error;
    uint32_t reports;
    uint16_t last_psm;
    uint8_t stage, auth_error;
    uint8_t auth_seen, encryption_seen, encryption_error, encrypted;
    uint8_t disconnect_reason, role; /* role: 0 central, 1 peripheral, 0xff unknown */
    uint8_t br_only, radio_mode, mode_seen;
    uint32_t buttons_seen, first_report_ms;
} nova_bt_pad;
enum { NOVA_BT_OFF, NOVA_BT_STARTING, NOVA_BT_READY, NOVA_BT_ERROR };
typedef struct {
    int state, error;
    uint8_t host_identity; /* 0 unknown, 1 controller matches imported host, 2 mismatch */
    uint8_t controller_hci_version;
    uint16_t controller_manufacturer, controller_revision, controller_subversion;
    uint32_t rx_packets, tx_packets;
    uint32_t rx_acl, tx_acl;
    uint32_t rx_bytes, rx_polls, rx_max_us;
    nova_bt_pad pad[2];
} nova_bt_status;
typedef struct {uint32_t ms;uint16_t kind,a,b,c;} nova_bt_trace;
unsigned nova_bt_trace_count(void);
int nova_bt_trace_get(unsigned index,nova_bt_trace *record);
/* Metadata only: never put addresses, link keys or packet payloads here. */
void nova_bt_trace_reset(void);
void nova_bt_trace_add(uint16_t kind,uint16_t a,uint16_t b,uint16_t c);
int nova_bt_start(const nova_bt_peer peers[2]);
void nova_bt_stop(void);
void nova_bt_poll(void);
void nova_bt_poll_fast(void); /* No quiet wait; short RX budget while drawing. */
const nova_bt_status *nova_bt_get_status(void);
/* Pure protocol helpers are exercised on the host as well as on ARM. */
int nova_bt_decode(nova_bt_pad *pad, unsigned side, const uint8_t *report, unsigned len, uint32_t now);
int nova_bt_validate_peers(const nova_bt_peer peers[2]);
#define NOVA_BT_PAIRING_SIZE 72
/* Private SD format, independent of C structure padding. CRC detects damage;
 * this file contains plain pairing keys and is never included in reports. */
int nova_bt_pairing_encode(uint8_t bytes[NOVA_BT_PAIRING_SIZE], const nova_bt_peer peers[2]);
int nova_bt_pairing_decode(nova_bt_peer peers[2], const uint8_t *bytes, unsigned size);
/* Radio operations never wait for a byte. Startup alone has a bounded POR. */
uint32_t nova_bt_now(void);
uint32_t nova_bt_now_us(void);
int nova_bt_radio_open(void);
void nova_bt_radio_close(void);
int nova_bt_radio_read(void); /* -1 empty, -2 UART fault */
int nova_bt_radio_write(uint8_t value); /* 0 backpressure */
#endif

static nova_bt_status bt_fixture;
const nova_bt_status *nova_bt_get_status(void){return &bt_fixture;}
static struct {lv_obj_t *win,*label;lv_task_t *task;lv_signal_func_t signal;u32 shown;char note[512];} ui;
static lv_res_t import_pairing(lv_obj_t *b){(void)b;return LV_RES_OK;}
static lv_res_t start(lv_obj_t *b){(void)b;return LV_RES_OK;}
static lv_res_t stop(lv_obj_t *b){(void)b;return LV_RES_OK;}
static lv_res_t forget(lv_obj_t *b){(void)b;return LV_RES_OK;}
static lv_res_t store_pairing(lv_obj_t *b){(void)b;return LV_RES_OK;}
static void refresh(void)
{
    if(!ui.label)return;
    const nova_bt_status *s=nova_bt_get_status();char text[3072], detail[512];
    s_printf(detail,"%s: %d / %d | ID: %02X / %02X | HID: %d / %d | ACK: %d / %d | ERR: %02X / %02X | BTN: %06X / %06X | XY: %d,%d / %d,%d | PSM: %04X / %04X | STEP: %d / %d",nyx_tr("Joy-Con reports"),(int)s->pad[0].reports,(int)s->pad[1].reports,s->pad[0].report_id,s->pad[1].report_id,s->pad[0].hid_open,s->pad[1].hid_open,s->pad[0].mode_ack,s->pad[1].mode_ack,s->pad[0].last_error,s->pad[1].last_error,(unsigned)s->pad[0].buttons,(unsigned)s->pad[1].buttons,s->pad[0].x,s->pad[0].y,s->pad[1].x,s->pad[1].y,(unsigned)s->pad[0].last_psm,(unsigned)s->pad[1].last_psm,s->pad[0].stage,s->pad[1].stage);
    const char *states[]={"Bluetooth off","Starting Bluetooth...","Bluetooth ready","Bluetooth error"};
    const char *state=s->state==NOVA_BT_READY?
        (s->pad[0].connected||s->pad[1].connected?"Joy-Con input active":"Radio ready - waiting for Joy-Con"):states[s->state<4?s->state:3];
    s_printf(text,"%s\n%s\n\n%s : %s    %s : %s\n%s : %d / %d\n%s\n\n%s\n%s",
        nyx_tr(state),ui.note,
        nyx_tr("Left Joy-Con"),nyx_tr(s->pad[0].connected?"Receiving inputs":"No input"),
        nyx_tr("Right Joy-Con"),nyx_tr(s->pad[1].connected?"Receiving inputs":"No input"),
        nyx_tr("HCI packets RX / TX"),(int)s->rx_packets,(int)s->tx_packets,detail,
        nyx_tr("Attach Joy-Con, read pairing, detach them, then start Bluetooth. Press a button to wake them."),
        nyx_tr("Stop Bluetooth saves a session report to SD. The radio service remains experimental."));
    if(s->state==NOVA_BT_ERROR){char error[64];s_printf(error,"\n%s : %d",nyx_tr("Error code"),s->error);strcat(text,error);}
    lv_label_set_text(ui.label,text);nyx_nova_fit_text(ui.label,250);
}
static lv_res_t signal(lv_obj_t *obj,lv_signal_t sign,void *param)
{
    lv_signal_func_t previous=ui.signal;
    if(sign==LV_SIGNAL_CLEANUP){ui.win=NULL;ui.label=NULL;}
    return previous(obj,sign,param);
}
lv_res_t gui_bluetooth_open(lv_obj_t *button)
{
    (void)button;if(ui.win)return LV_RES_OK;
    ui.win=nyx_create_standard_window(nyx_tr("Wireless Joy-Con"),NULL);
    ui.signal=lv_obj_get_signal_func(ui.win);lv_obj_set_signal_func(ui.win,signal);
    lv_obj_t *page=lv_win_get_content(ui.win);nyx_nova_fixed_page(page);
    lv_obj_t *scrl=lv_page_get_scrl(page);
    lv_obj_t *panel=nyx_nova_panel(scrl,24,16,lv_obj_get_width(page)-48,286);
    ui.label=nyx_nova_text(panel,"",20,18,lv_obj_get_width(panel)-40,NOVA_BODY);
    const char *titles[]={"Read pairing","Start Bluetooth","Stop Bluetooth","Save pairing","Load pairing","Clear session keys"};
    const char *icons[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
    lv_action_t actions[]={import_pairing,start,stop,store_pairing,store_pairing,forget};
    int width=(lv_obj_get_width(page)-72)/3;
    for(unsigned i=0;i<6;i++)nyx_nova_action(scrl,titles[i],NULL,icons[i],actions[i],i==3,24+(i%3)*(width+12),318+(i/3)*88,width,76);
    refresh();return LV_RES_OK;
}

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
static void _update_status_bar(void *params)
{
	static char *label = NULL;

	u16 soc_temp = 0;
	u32 batt_percent = 0;
	int charge_status = 0;
	int batt_volt = 0;
	int batt_curr = 0;
	rtc_time_t time;

	// Get sensor data.
	max77620_rtc_get_time_adjusted(&time);
	soc_temp = tmp451_get_soc_temp(false);
	bq24193_get_property(BQ24193_ChargeStatus, &charge_status);
	max17050_get_property(MAX17050_RepSOC, (int *)&batt_percent);
	max17050_get_property(MAX17050_VCELL, &batt_volt);
	max17050_get_property(MAX17050_Current, &batt_curr);

	// Enable fan if more than 41 oC.
	u32 soc_temp_dec = soc_temp >> 8;
	fan_set_from_temp(soc_temp_dec);

	if (!label)
		label = (char *)malloc(512);

	// Set time and SoC temperature.
	s_printf(label, "%02d:%02d · #%06X %02d.%d °C#",
		time.hour, time.min, nyx_nova_temperature_rgb(soc_temp_dec), soc_temp_dec, (soc_temp & 0xFF) / 10);
	lv_label_set_recolor(status_bar.time_temp, true);

	lv_label_set_text(status_bar.time_temp, label);


	// Compact Nova footer uses the live percentage and SVG charging state.
	s_printf(label, "%d.%d%%", (batt_percent >> 8) & 0xFF, (batt_percent & 0xFF) / 26);
	nyx_nova_battery((batt_percent >> 8) & 0xFF, charge_status != 0);

	lv_label_set_text(status_bar.battery, label);
	lv_obj_set_pos(status_bar.battery, 1174, 24);

	// Set battery current draw and voltage.
	s_printf(label, "#%s%d", batt_curr >= 0 ? "96FF00 +" : "FF3C28 ", batt_curr / 1000);

	bool voltage_empty = batt_volt < 3200;
	s_printf(label + strlen(label), " mA# (%s%d mV%s)",
		voltage_empty ? "#FF8000 " : "", batt_volt,  voltage_empty ? " "SYMBOL_WARNING"#" : "");

	lv_label_set_text(status_bar.battery_more, label);
	lv_obj_realign(status_bar.battery_more);
}

ACTION(_joycon_info_dump_action)
ACTION(_autoboot_hide_delay_action)
static void display_backlight_brightness(int value,int delay){(void)delay;assert(value>=10&&value<=200);}

static lv_res_t _slider_brightness_action(lv_obj_t * slider)
{
	if (!gui_tv_console_dark())
		display_backlight_brightness(lv_slider_get_value(slider) - 20, 0);
	h_cfg.backlight = lv_slider_get_value(slider);
	ini_changes_made = true;

	return LV_RES_OK;
}
static lv_res_t _save_nyx_options_action(lv_obj_t *btn)
{
	return nyx_options_save_all(btn);
}
static lv_res_t _autoboot_delay_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (h_cfg.bootwait != new_selection)
	{
		h_cfg.bootwait = new_selection;
		ini_changes_made = true;
	}

	return LV_RES_OK;
}
static lv_res_t _data_verification_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (n_cfg.verification != new_selection)
	{
		n_cfg.verification = new_selection;
		nyx_changes_made = true;
	}

	return LV_RES_OK;
}
static lv_res_t _home_screen_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (n_cfg.home_screen != new_selection)
	{
		n_cfg.home_screen = new_selection;
		nyx_changes_made = true;
	}

	return LV_RES_OK;
}
static lv_res_t auto_nogc_toggle(lv_obj_t *btn)
{
	h_cfg.autonogc = !h_cfg.autonogc;
	ini_changes_made = true;

	if (!h_cfg.autonogc)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}
static lv_res_t auto_hos_poweroff_toggle(lv_obj_t *btn)
{
	h_cfg.autohosoff = !h_cfg.autohosoff;
	ini_changes_made = true;

	if (!h_cfg.autohosoff)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}
static lv_res_t _update_r2p_action(lv_obj_t *btn)
{
	h_cfg.updater2p = !h_cfg.updater2p;
	ini_changes_made = true;

	if (!h_cfg.updater2p)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}
static lv_res_t _entries_columns_action(lv_obj_t *btn)
{
	n_cfg.entries_5_col = !n_cfg.entries_5_col;
	nyx_changes_made = true;

	if (!n_cfg.entries_5_col)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static void _check_nyx_changes(void){assert(!nyx_changes_made); }
static lv_res_t _action_win_nyx_options_close(lv_obj_t *btn)
{
	lv_res_t res = nyx_win_close_action(btn);
	// Brightness is stored in hekate_ipl.ini; keep both kinds of changes
	// available after closing or canceling the save prompt.
	lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, nyx_options_save_all);
	bool pending = nyx_options_has_changes();
	lv_obj_set_opa_scale(status_bar.mid, pending ? LV_OPA_COVER : LV_OPA_TRANSP);
	lv_obj_set_click(status_bar.mid, pending);

	_check_nyx_changes();

	return res;
}
void nyx_options_appearance_active(bool active, bool saved)
{
	if (saved)
		nyx_changes_made = false;
	if (active) {
		lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP);
		lv_obj_set_click(status_bar.mid, false);
	} else {
		lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, nyx_options_save_all);
		bool visible = close_btn != NULL || nyx_options_has_changes();
		lv_obj_set_opa_scale(status_bar.mid, visible ? LV_OPA_COVER : LV_OPA_TRANSP);
		lv_obj_set_click(status_bar.mid, visible);
	}
}
static const u16 theme_colors[18] = {
	0, 4, 13, 23, 33, 43, 54, 66, 89, 124, 167, 187, 200, 208, 231, 261, 291, 341
};

void create_flat_button(lv_obj_t *btn, int color_idx, lv_action_t action)
{
	/* Palette swatches represent a color, so they must keep their fill even
	 * when ordinary menu buttons use a transparent body. Reuse the fixed
	 * palette's styles instead of leaking two allocations on every opening. */
	static lv_style_t palette_rel[ARRAY_SIZE(theme_colors)];
	static lv_style_t palette_pr[ARRAY_SIZE(theme_colors)];
	u32 index = 0;
	for (u32 i = 0; i < ARRAY_SIZE(theme_colors); ++i)
		if (theme_colors[i] == color_idx) { index = i; break; }
	lv_color_t color = color_idx ? lv_color_hsv_to_rgb(color_idx, 100, 100) : nyx_nova_color(NOVA_COLOR_REFLECTION);
	lv_style_t *btn_onoff_rel_hos_style = &palette_rel[index];
	lv_style_t *btn_onoff_pr_hos_style  = &palette_pr[index];
	lv_style_copy(btn_onoff_rel_hos_style, lv_theme_get_current()->btn.rel);
	btn_onoff_rel_hos_style->body.empty       = 0;
	btn_onoff_rel_hos_style->body.opa         = LV_OPA_COVER;
	btn_onoff_rel_hos_style->body.shadow.width = 0;
	btn_onoff_rel_hos_style->body.main_color  = color;
	btn_onoff_rel_hos_style->body.grad_color  = btn_onoff_rel_hos_style->body.main_color;
	btn_onoff_rel_hos_style->body.padding.hor = 0;
	btn_onoff_rel_hos_style->body.radius      = 28;

	if (color_idx == 167)
	{
		btn_onoff_rel_hos_style->body.border.color = LV_COLOR_HEX(0x000000);
		btn_onoff_rel_hos_style->body.border.opa = LV_OPA_20;
		btn_onoff_rel_hos_style->body.border.width = 3;
	}

	lv_style_copy(btn_onoff_pr_hos_style, lv_theme_get_current()->btn.pr);
	btn_onoff_pr_hos_style->body.empty        = 0;
	btn_onoff_pr_hos_style->body.opa          = LV_OPA_COVER;
	btn_onoff_pr_hos_style->body.shadow.width = 0;
	btn_onoff_pr_hos_style->body.main_color   = color;
	btn_onoff_pr_hos_style->body.grad_color   = btn_onoff_pr_hos_style->body.main_color;
	btn_onoff_pr_hos_style->body.padding.hor  = 0;
	btn_onoff_pr_hos_style->body.border.color = LV_COLOR_HEX(0xFFFFFF);
	btn_onoff_pr_hos_style->body.border.opa   = LV_OPA_50;
	btn_onoff_pr_hos_style->body.border.width = 4;
	btn_onoff_pr_hos_style->body.radius       = 28;

	lv_btn_set_style(btn, LV_BTN_STYLE_REL,     btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_PR,      btn_onoff_pr_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_REL, btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_PR,  btn_onoff_pr_hos_style);

	lv_btn_set_fit(btn, false, false);
	lv_obj_set_size(btn, LV_DPI * 7 / 11, LV_DPI * 7 / 11);
	lv_btn_set_toggle(btn, true);

	if (action)
		lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);
}

static lv_res_t _create_window_nyx_colors(lv_obj_t *button)
{
    return nyx_nova_theme_window(button);
}

/* Compact pages keep controls above the persistent status bar. */
static void settings_page(lv_obj_t *page)
{
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF);
    lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, lv_obj_get_width(page));
    lv_page_set_scrl_height(page, lv_obj_get_height(page));
    lv_page_set_sb_mode(page, LV_SB_MODE_OFF);
    lv_obj_set_pos(lv_page_get_scrl(page), 0, 0);
    lv_obj_set_drag(lv_page_get_scrl(page), false);
}

static lv_obj_t *_settings_card(lv_obj_t *parent, u32 index, u32 rows)
{
    lv_coord_t width = (lv_obj_get_width(parent) - 80) / 2;
    lv_coord_t height = (lv_obj_get_height(parent) - 32 - (rows - 1) * 12) / rows;
    lv_obj_t *card = lv_cont_create(parent, NULL);
    lv_cont_set_style(card, lv_theme_get_current()->panel);
    lv_cont_set_fit(card, false, false); lv_cont_set_layout(card, LV_LAYOUT_OFF);
    lv_obj_set_size(card, width, height);
    lv_obj_set_pos(card, 32 + index % 2 * (width + 16), 16 + index / 2 * (height + 12));
    return card;
}

static lv_res_t settings_help_pressed(lv_obj_t *button)
{
    nyx_show_help(lv_obj_get_free_ptr(button));
    return LV_RES_OK;
}

static void _settings_help(lv_obj_t *card, const char *help)
{
    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false);
    lv_obj_set_size(button, 36, 36);
    lv_obj_set_pos(button, lv_obj_get_width(card) - 46, 10);
    lv_obj_set_free_ptr(button, (void *)help);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, settings_help_pressed);
    lv_obj_t *mark = lv_label_create(button, NULL); lv_label_set_style(mark, &hint_small_style);
    lv_label_set_text(mark, "?");
}

static lv_obj_t *_settings_heading(lv_obj_t *card, const char *title)
{
    lv_obj_t *label = nyx_nova_text(card,nyx_nova_caption(title),16,12,lv_obj_get_width(card)-80,NOVA_TITLE);
    nyx_nova_fit_text(label,48);
    return label;
}

static lv_obj_t *_settings_choices(lv_obj_t *card, const char *title,
    const char *options, u32 selected, u32 columns, lv_action_t action, bool raw)
{
    lv_obj_t *heading = _settings_heading(card, title);
    int y = 20 + lv_obj_get_height(heading);
    int rows = 1; for (const char *p = options; *p; ++p) if (*p == '\n') ++rows;
    rows = (rows + columns - 1) / columns;
    int height = (lv_obj_get_height(card) - y - 14 - (rows - 1) * 8) / rows;
    lv_obj_t *choices = nyx_choices_create(card, options, selected, columns,
        lv_obj_get_width(card) - 32, height, action, 0, raw);
    if (choices) lv_obj_set_pos(choices, 16, y);
    return choices;
}

static lv_obj_t *_settings_button(lv_obj_t *card, const char *title, lv_action_t action)
{
    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    static lv_style_t clear;lv_style_copy(&clear,&lv_style_transp_tight);
    lv_btn_set_style(button,LV_BTN_STYLE_REL,&clear);lv_btn_set_style(button,LV_BTN_STYLE_PR,lv_theme_get_current()->btn.pr);
    lv_obj_set_size(button, lv_obj_get_width(card) - 74, lv_obj_get_height(card) - 28);
    lv_obj_set_pos(button, 14, 14); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    const char *icon=(strstr(title,"Color")||strstr(title,"couleurs"))?"paintbrush":strstr(title,"Clock")?"clock-3":strstr(title,"TV")?"monitor":strstr(title,"Joy-Con")?"gamepad-2":"settings-2";
    nyx_nova_icon_box(button,icon,10,(lv_obj_get_height(button)-44)/2,44,false);
    lv_obj_t *label=nyx_nova_text(button,nyx_nova_caption(title),76,0,lv_obj_get_width(button)-92,NOVA_TITLE);
    nyx_nova_fit_text(label,lv_obj_get_height(button)-24);lv_obj_set_y(label,(lv_obj_get_height(button)-lv_obj_get_height(label))/2);
    return button;
}

static lv_obj_t *_settings_toggle(lv_obj_t *card, const char *title, lv_action_t action, bool enabled)
{
    lv_obj_t *button = nyx_nova_toggle(card, nyx_nova_caption(title), action, enabled,
        lv_obj_get_width(card) - 74, lv_obj_get_height(card) - 28);
    if (button) lv_obj_set_pos(button, 14, 14);
    return button;
}

static lv_res_t _tv_auto_action(lv_obj_t *button)
{
    n_cfg.tv_auto = !n_cfg.tv_auto;
    nyx_changes_made = true;
    lv_btn_set_state(button, n_cfg.tv_auto ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(button);
    gui_tv_auto_configure();
    return LV_RES_OK;
}

static lv_res_t _tv_screen_action(lv_obj_t *button)
{
    u32 selected = lv_obj_get_free_num(button);
    if (selected > 1)
        return LV_RES_OK;
    n_cfg.tv_keep_screen = selected;
    nyx_changes_made = true;
    gui_tv_screen_configure();
    return LV_RES_OK;
}

lv_res_t create_win_nyx_options(lv_obj_t *parent_button)
{
    (void)parent_button;
    lv_obj_t *win = nyx_create_standard_window(SYMBOL_HOME" Nyx Settings", _action_win_nyx_options_close);
    lv_win_add_btn(win, NULL, nyx_tr("Appearance"), nyx_create_appearance_window);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win);
    settings_page(ext->page);
    lv_obj_t *tabs = lv_tabview_create(ext->page, NULL);
    lv_obj_set_size(tabs, lv_obj_get_width(ext->page), lv_obj_get_height(ext->page));
    lv_obj_set_pos(tabs, 0, 0); lv_tabview_set_sliding(tabs, false);
    lv_obj_t *interface_page = lv_tabview_add_tab(tabs, "Interface"); settings_page(interface_page);
    lv_obj_t *system = lv_tabview_add_tab(tabs, "System"); settings_page(system);
    lv_obj_t *tv_page = lv_tabview_add_tab(tabs, "TV / Dock"); settings_page(tv_page);
    lv_obj_t *tv_card = _settings_card(tv_page, 0, 2);
    _settings_button(tv_card, "USB-C TV / Dock", gui_tv_open);
    _settings_help(tv_card, "Connect the console directly to the official dock, then start TV output.");
    tv_card = _settings_card(tv_page, 1, 2);
    _settings_toggle(tv_card, "Automatic dock detection", _tv_auto_action, n_cfg.tv_auto);
    _settings_help(tv_card, "Enable automatic detection, then select Save to keep your choice after restarting Nyx.");
    tv_card = _settings_card(tv_page, 2, 2);
    _settings_button(tv_card, "Wireless Joy-Con", gui_bluetooth_open);
    _settings_help(tv_card, "Experimental Bluetooth. Read the existing pairing from attached Joy-Con, then detach them and start the service. VOL- stops TV output.");
    tv_card = _settings_card(tv_page, 3, 2);
    _settings_choices(tv_card, "TV display", "TV only\nTV + console", n_cfg.tv_keep_screen, 2, _tv_screen_action, false);
    _settings_help(tv_card, "TV only turns off the console screen after TV output starts. The screen returns when TV output stops. Select Save to remember this choice.");
    lv_obj_t *card = _settings_card(interface_page, 0, 2);
    _settings_button(card, SYMBOL_EDIT" Appearance & language", nyx_create_appearance_window);
    _settings_help(card, "Choose your language, reading comfort and background in one place.");
    card = _settings_card(interface_page, 1, 2);
    _settings_button(card, SYMBOL_COPY" Thèmes et couleurs", _create_window_nyx_colors);
    _settings_help(card, "Couleurs indépendantes, fond d’écran et transparence.");
    card = _settings_card(interface_page, 2, 2);
    _settings_choices(card, SYMBOL_HOME" Home Screen", "Main menu\nAll Configs\nLaunch\nMore Configs", n_cfg.home_screen, 2, _home_screen_action, false);
    _settings_help(card, "Select what screen to show on Nyx boot.\nAll Configs combines extra configurations into the launch screen.");
    card = _settings_card(interface_page, 3, 2);
    _settings_toggle(card, SYMBOL_GPS" Extended Boot Entries", _entries_columns_action, n_cfg.entries_5_col);
    _settings_help(card, "Increase the number of boot entries shown on each row.");
    card = _settings_card(system, 0, 2);
    _settings_button(card, SYMBOL_DOWNLOAD" Dump Joy-Con BT", _joycon_info_dump_action);
    _settings_help(card, "Save Switch and Joy-Con MAC addresses and pairing keys for Android and Linux.");
    card = _settings_card(system, 1, 2);
    _settings_choices(card, SYMBOL_MODULES_ALT" Data Verification", "Off\nSparse\nFull\nHashes", n_cfg.verification, 2, _data_verification_action, false);
    _settings_help(card, "Set the type of data verification done for backup and restore.\nCan be canceled without losing the backup/restore.");
    card = _settings_card(system, 2, 2);
    _settings_button(card, SYMBOL_CLOCK" Clock (Offset)", _create_mbox_clock_edit);
    _settings_help(card, "Change the clock offset manually. The date and time are also used for SD card operations.");
    card = _settings_card(system, 3, 2);
    lv_obj_t *heading = _settings_heading(card, SYMBOL_BRIGHTNESS" Backlight");
    lv_obj_t *brightness = lv_slider_create(card, NULL);
    lv_obj_set_size(brightness, lv_obj_get_width(card) - 48, 40);
    lv_obj_set_pos(brightness, 24, lv_obj_get_height(heading) + 40);
    lv_bar_set_range(brightness, 30, 220); lv_bar_set_value(brightness, h_cfg.backlight);
    lv_slider_set_action(brightness, _slider_brightness_action);
    _settings_help(card, "Set backlight brightness.");
    lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, _save_nyx_options_action);
    lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    return LV_RES_OK;
}

void create_tab_boot_options(lv_theme_t *theme, lv_obj_t *parent)
{
    (void)theme; settings_page(parent);
    lv_obj_t *card = _settings_card(parent, 0, 3);
    autoboot_btn = _settings_toggle(card, SYMBOL_GPS" Auto Boot", _autoboot_hide_delay_action, h_cfg.autoboot);
    autoboot_first_time = false;
    _settings_help(card, "Choose which boot entry or payload to automatically boot when injecting.");
    card = _settings_card(parent, 1, 3);
    _settings_choices(card, SYMBOL_CLOCK" Boot Time Delay", "0\n1\n2\n3\n4\n5\n6", h_cfg.bootwait, 7, _autoboot_delay_action, true);
    _settings_help(card, "Set how long to show the boot logo when automatic boot is enabled.\nPress VOL- during that time to enter the Hekate menu.");
    card = _settings_card(parent, 2, 3);
    _settings_toggle(card, SYMBOL_CHIP" Auto NoGC", auto_nogc_toggle, h_cfg.autonogc);
    _settings_help(card, "Check the fuses and automatically apply the game-card protection patch when needed.");
    card = _settings_card(parent, 3, 3);
    _settings_toggle(card, SYMBOL_POWER" Auto HOS Power Off", auto_hos_poweroff_toggle, h_cfg.autohosoff);
    _settings_help(card, "Automatically power off on the next payload injection after shutting down from HOS.");
    card = _settings_card(parent, 4, 3);
    lv_obj_t *heading = _settings_heading(card, SYMBOL_BRIGHTNESS" Backlight");
    lv_obj_t *slider = lv_slider_create(card, NULL);
    lv_obj_set_size(slider, lv_obj_get_width(card) - 48, 40);
    lv_obj_set_pos(slider, 24, lv_obj_get_height(heading) + 34);
    lv_bar_set_range(slider, 30, 220); lv_bar_set_value(slider, h_cfg.backlight);
    lv_slider_set_action(slider, _slider_brightness_action);
    _settings_help(card, "Set backlight brightness.");
    card = _settings_card(parent, 5, 3);
    _settings_toggle(card, SYMBOL_REFRESH" Update Reboot 2 Payload", _update_r2p_action, h_cfg.updater2p);
    _settings_help(card, "When FSS0 is used, check and update the reboot-to-payload image to Hekate.");
}

bool nyx_options_has_changes(void) { return ini_changes_made || nyx_changes_made; }

bool nyx_options_commit_boot(void)
{
    if (!ini_changes_made) return true;
    if (create_config_entry()) return false;
    ini_changes_made = false; return true;
}

lv_res_t nyx_options_save_all(lv_obj_t *button)
{
    (void)button;
    if (sd_mount()) { nyx_show_help("SD card unavailable. Nothing was saved."); return LV_RES_OK; }
    bool boot_saved = !create_config_entry();
    if (boot_saved) ini_changes_made = false;
    bool nyx_saved = boot_saved && !create_nyx_config_entry(true);
    if (!boot_saved) sd_unmount();
    if (boot_saved && nyx_saved) {
        nyx_changes_made = false;
        if (status_bar.mid) { lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP); lv_obj_set_click(status_bar.mid, false); }
        nyx_show_help("Settings saved.");
    } else nyx_show_help(boot_saved ? "Boot options were saved. Nyx settings could not be saved. Check the SD card and retry." : "Could not save settings. Check the SD card and retry.");
    return LV_RES_OK;
}

static lv_obj_t *nova_contour_value;
static lv_res_t _nova_verification_toggle(lv_obj_t *button)
{
    n_cfg.verification = n_cfg.verification ? 0 : 2; nyx_changes_made = true;
    lv_btn_set_state(button, n_cfg.verification ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(button); return LV_RES_OK;
}
static lv_res_t _nova_halo_toggle(lv_obj_t *button)
{
    n_cfg.nova_halo = !n_cfg.nova_halo; nyx_changes_made = true;
    lv_btn_set_state(button, n_cfg.nova_halo ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(button); nyx_nova_refresh_halos();
    if (nova_contour_value) lv_label_set_text(nova_contour_value, n_cfg.nova_halo ? "Reflet blanc diagonal" : "Sans reflet");
    return LV_RES_OK;
}
static lv_res_t _nova_language_action(lv_obj_t *button)
{ nyx_appearance_open_language(lv_obj_get_free_num(button)); return LV_RES_OK; }
static lv_res_t _nova_size_action(lv_obj_t *button)
{ nyx_appearance_open_size(lv_obj_get_free_num(button)); return LV_RES_OK; }
static lv_res_t _nova_font_action(lv_obj_t *button)
{ (void)button; nyx_appearance_open_font(); return LV_RES_OK; }
static lv_res_t _nova_language_menu(lv_obj_t *button)
{ (void)button; nyx_appearance_open_language(n_cfg.language); return LV_RES_OK; }
static lv_res_t _nova_boot_options(lv_obj_t *button)
{
    (void)button; lv_obj_t *win = nyx_create_standard_window("Options système", NULL);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win); create_tab_boot_options(lv_theme_get_current(), ext->page);
    if (status_bar.mid) {
        lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, nyx_options_save_all);
        lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    }
    return LV_RES_OK;
}

static lv_obj_t *_nova_heading_button(lv_obj_t *panel, const char *title, const char *icon, lv_action_t action)
{
    static lv_style_t transparent; lv_style_copy(&transparent, &lv_style_transp_tight);
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &transparent); lv_btn_set_style(button, LV_BTN_STYLE_PR, lv_theme_get_current()->btn.pr);
    lv_obj_set_size(button, lv_obj_get_width(panel) - 48, 42); lv_obj_set_pos(button, 24, 16);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action); nyx_nova_glyph(button, icon, 0, 8, 24);
    lv_obj_t *label = nyx_nova_text(button, title, 38, 8, lv_obj_get_width(button) - 40, NOVA_TITLE); nyx_nova_fit_text(label, 30);
    return button;
}

static lv_obj_t *_nova_setting_line(lv_obj_t *panel, const char *title, const char *value, int y, lv_action_t action)
{
    static lv_style_t transparent, accent, line;
    lv_style_copy(&transparent, &lv_style_transp_tight);
    lv_style_copy(&accent, nyx_nova_text_style(NOVA_SMALL)); accent.text.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_style_copy(&line, &lv_style_plain); line.body.main_color = line.body.grad_color = nyx_nova_color(NOVA_COLOR_BORDER); line.body.border.width = 0;
    int width = lv_obj_get_width(panel) - 48;
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &transparent); lv_btn_set_style(button, LV_BTN_STYLE_PR, lv_theme_get_current()->btn.pr);
    lv_obj_set_size(button, width, 44); lv_obj_set_pos(button, 24, y); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *title_label = nyx_nova_text(button, title, 0, 12, width / 2 - 10, NOVA_BODY); nyx_nova_fit_text(title_label, 28);
    lv_obj_t *value_label = nyx_nova_text(button, value, width / 2, 12, width / 2, NOVA_SMALL);
    lv_label_set_style(value_label, &accent); lv_label_set_align(value_label, LV_LABEL_ALIGN_RIGHT); nyx_nova_fit_text(value_label, 28);
    lv_obj_t *separator = lv_cont_create(panel, NULL); lv_cont_set_style(separator, &line); lv_cont_set_fit(separator, false, false);
    lv_obj_set_size(separator, width, 1); lv_obj_set_pos(separator, 24, y + 43); lv_obj_set_click(separator, false);
    return value_label;
}

static lv_obj_t *_nova_chip(lv_obj_t *panel, const char *text, int x, int y, int width, bool selected, lv_action_t action, u32 id)
{
    static lv_style_t normal, active, label_active;
    lv_style_copy(&normal, lv_theme_get_current()->btn.rel);
    normal.body.empty = 0; normal.body.main_color = normal.body.grad_color = nyx_nova_color(NOVA_COLOR_BACKGROUND);
    normal.body.radius = 5; normal.body.border.width = 1; normal.body.border.color = nyx_nova_color(NOVA_COLOR_BORDER);
    normal.body.padding.hor = normal.body.padding.ver = 0;
    lv_style_copy(&active, &normal); active.body.border.width = 2; active.body.border.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_style_copy(&label_active, nyx_nova_text_style(NOVA_SMALL)); label_active.text.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, selected ? &active : &normal); lv_btn_set_style(button, LV_BTN_STYLE_PR, &active);
    lv_obj_set_size(button, width, 36); lv_obj_set_pos(button, x, y); lv_obj_set_free_num(button, id); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *label = nyx_nova_text(button, text, 4, 9, width - 8, NOVA_SMALL);
    if (selected) lv_label_set_style(label, &label_active);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); nyx_nova_fit_text(label, 24); return button;
}

/* The system column keeps its fill and switch state, with no surrounding
 * outlines. Other settings retain their normal selection feedback. */
static void nova_system_flat(lv_obj_t *button)
{
    static lv_style_t styles[4];
    const u32 states[]={LV_BTN_STYLE_REL,LV_BTN_STYLE_PR,LV_BTN_STYLE_TGL_REL,LV_BTN_STYLE_TGL_PR};
    for(u32 i=0;i<4;++i){lv_style_copy(&styles[i],lv_btn_get_style(button,states[i]));styles[i].body.border.width=0;lv_btn_set_style(button,states[i],&styles[i]);}
}
void create_tab_options(lv_theme_t *theme, lv_obj_t *parent)
{
    (void)theme; nyx_nova_page(parent, 4);
    int height = lv_obj_get_height(parent) - 168;
    int left_width = (LV_HOR_RES - 68 - 22) * 113 / 213;
    int right_width = LV_HOR_RES - 68 - 22 - left_width;
    lv_obj_t *system = nyx_nova_panel(parent, 34, 140, left_width, height);
    lv_obj_t *reading = nyx_nova_panel(parent, 34 + left_width + 22, 140, right_width, height);
    static lv_style_t flat_panel,flat_heading;
    lv_style_copy(&flat_panel,lv_cont_get_style(system));flat_panel.body.border.width=0;lv_cont_set_style(system,&flat_panel);
    lv_obj_t *system_heading=_nova_heading_button(system,"Options système","toggle-right",_nova_boot_options);
    lv_style_copy(&flat_heading,lv_btn_get_style(system_heading,LV_BTN_STYLE_PR));flat_heading.body.border.width=0;lv_btn_set_style(system_heading,LV_BTN_STYLE_PR,&flat_heading);
    _nova_heading_button(reading, "Apparence et lecture", "paintbrush", create_win_nyx_options);
    int row_height = (height - 64 - 16) / 4;
    nova_system_flat(nyx_nova_switch_row(system, "Auto NoGC", "Protection du lecteur de cartes.", "shield-check", auto_nogc_toggle, h_cfg.autonogc, 24, 64, left_width - 48, row_height));
    nova_system_flat(nyx_nova_switch_row(system, "Extinction HOS", "Arrêt automatique après HOS.", "power", auto_hos_poweroff_toggle, h_cfg.autohosoff, 24, 64 + row_height, left_width - 48, row_height));
    nova_system_flat(nyx_nova_switch_row(system, "Vérification des sauvegardes", "Contrôler les données écrites.", "badge-check", _nova_verification_toggle, n_cfg.verification, 24, 64 + 2 * row_height, left_width - 48, row_height));
    nova_system_flat(nyx_nova_switch_row(system, "Effets de sélection", "Reflet blanc discret sur la tuile sélectionnée.", "sparkles", _nova_halo_toggle, n_cfg.nova_halo, 24, 64 + 3 * row_height, left_width - 48, row_height));
    const char *language_names[] = {"English", "Français", "Español", "Italiano", "Русский", "Deutsch", "Português"};
    _nova_setting_line(reading, "Language", language_names[n_cfg.language < 7 ? n_cfg.language : 0], 64, _nova_language_menu);
    const u32 order[] = {1,0,2,3,4,5,6}; const char *codes[] = {"EN","FR","ES","IT","RU","DE","PT"};
    for (u32 i = 0; i < 7; ++i) _nova_chip(reading, codes[order[i]], 24 + i * 50, 118, 42, n_cfg.language == order[i], _nova_language_action, order[i]);
    char face[64]; strcpy(face, n_cfg.font_file[0] ? n_cfg.font_file : "Inter"); char *extension = strstr(face, ".nyxf"); if (extension) *extension = 0;
    lv_obj_t *font_name = _nova_setting_line(reading, "Font", face, 170, _nova_font_action); (lv_label_set_text)(font_name, face);
    char size[20]; s_printf(size, "%d px", n_cfg.font_size);
    _nova_setting_line(reading, "Text size", size, 226, _nova_font_action);
    const u32 sizes[] = {24,28,32};
    for (u32 i = 0; i < 3; ++i) { char number[12]; s_printf(number, "%d", sizes[i]); _nova_chip(reading, number, 24 + i * 66, 280, 58, n_cfg.font_size == sizes[i], _nova_size_action, sizes[i]); }
    nova_contour_value = _nova_setting_line(reading, "Sélection", n_cfg.nova_halo ? "Reflet blanc diagonal" : "Sans reflet", 334, NULL);
}
/* Hue X, saturated color -> white Y, independent value slider. No presets. */

typedef struct {
    lv_cont_ext_t container;
    lv_signal_func_t base_signal,plane_signal;
    lv_obj_t *plane,*hit,*cursor,*value,*swatch,*hex,*channels;
    lv_img_dsc_t image;
    lv_color_t *pixels;
    lv_style_t swatch_style,cursor_style;
    nyx_color_changed_t changed;
    u32 rgb,hue,saturation,brightness,cached_brightness;
    int width,height;
} color_picker_ext;

static void color_plane_fill(lv_obj_t *picker)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    if(!ext->pixels||ext->cached_brightness==ext->brightness)return;
    /* Cache the plane only when brightness changes. Moving the cursor or
     * switching targets at the same brightness never regenerates it. */
    for(int x=0;x<ext->width;++x){
        lv_color_t hue=lv_color_hsv_to_rgb(x*359u/(ext->width-1),100,100);
        for(int y=0;y<ext->height;++y){
            u32 s=(ext->height-1-y)*255u/(ext->height-1);
            u32 r=(255-((255-hue.red)*s+127)/255)*ext->brightness/100;
            u32 g=(255-((255-hue.green)*s+127)/255)*ext->brightness/100;
            u32 b=(255-((255-hue.blue)*s+127)/255)*ext->brightness/100;
            ext->pixels[y*ext->width+x]=LV_COLOR_MAKE(r,g,b);
        }
    }
    ext->cached_brightness=ext->brightness;lv_obj_invalidate(ext->plane);
}

static void color_picker_refresh(lv_obj_t *picker,bool notify)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    color_plane_fill(picker);
    lv_obj_set_pos(ext->cursor,ext->hue*(ext->width-1)/359-6,(100-ext->saturation)*(ext->height-1)/100-6);
    lv_bar_set_value(ext->value,ext->brightness);
    ext->swatch_style.body.main_color=ext->swatch_style.body.grad_color=LV_COLOR_HEX(ext->rgb);
    lv_cont_set_style(ext->swatch,&ext->swatch_style);
    char text[96];s_printf(text,"#%06X",ext->rgb);(lv_label_set_text)(ext->hex,text);
    s_printf(text,"R   %d\nG   %d\nB   %d",(ext->rgb>>16)&255,(ext->rgb>>8)&255,ext->rgb&255);
    (lv_label_set_text)(ext->channels,text);
    if(notify&&ext->changed)ext->changed(picker,ext->rgb);
}

static void color_picker_update(lv_obj_t *picker)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    lv_color_t c=lv_color_hsv_to_rgb(ext->hue,ext->saturation,ext->brightness);
    ext->rgb=((u32)c.red<<16)|((u32)c.green<<8)|c.blue;
    color_picker_refresh(picker,true);
}

static lv_res_t color_plane_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    lv_obj_t *picker=lv_obj_get_free_ptr(object);color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    lv_res_t result=ext->plane_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_PRESSED||signal==LV_SIGNAL_PRESSING){
        lv_point_t point;lv_indev_t *input=lv_indev_get_act();if(!input)return result;
        lv_indev_get_point(input,&point);lv_area_t area;lv_obj_get_coords(object,&area);
        int x=point.x-area.x1,y=point.y-area.y1;
        if(x<0)x=0;
        if(x>=ext->width)x=ext->width-1;
        if(y<0)y=0;
        if(y>=ext->height)y=ext->height-1;
        u32 hue=x*359u/(ext->width-1),saturation=100u-y*100u/(ext->height-1);
        if(hue!=ext->hue||saturation!=ext->saturation){ext->hue=hue;ext->saturation=saturation;color_picker_update(picker);}
    }
    return result;
}

static lv_res_t color_value_action(lv_obj_t *slider)
{
    lv_obj_t *picker=lv_obj_get_parent(slider);color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    u32 value=lv_slider_get_value(slider);
    if(value!=ext->brightness){ext->brightness=value;color_picker_update(picker);}
    return LV_RES_OK;
}

static lv_res_t color_picker_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(object);
    lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP){free(ext->pixels);ext->pixels=NULL;}
    return result;
}

void nyx_color_picker_set(lv_obj_t *picker,u32 rgb)
{
    if(!picker)return;
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    ext->rgb=rgb&0xFFFFFF;
    /* This LVGL version's rgb_to_hsv uses byte units whereas hsv_to_rgb
     * takes degrees/percent. Convert explicitly to avoid out-of-range
     * cursor positions and brightness for saturated or pale colors. */
    int r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;
    int maximum=r>g?r:g;if(b>maximum)maximum=b;
    int minimum=r<g?r:g;if(b<minimum)minimum=b;
    int delta=maximum-minimum,hue=0;
    if(delta){
        if(maximum==r)hue=60*(g-b)/delta;
        else if(maximum==g)hue=120+60*(b-r)/delta;
        else hue=240+60*(r-g)/delta;
        if(hue<0)hue+=360;
    }
    ext->hue=hue;ext->saturation=maximum?(delta*100+maximum/2)/maximum:0;
    ext->brightness=(maximum*100+127)/255;
    color_picker_refresh(picker,false); /* Selecting a target preserves exact RGB bytes. */
}
u32 nyx_color_picker_rgb(lv_obj_t *picker)
{return picker?((color_picker_ext *)lv_obj_get_ext_attr(picker))->rgb:0;}

lv_obj_t *nyx_color_picker_create(lv_obj_t *parent,int x,int y,int width,int height,nyx_color_changed_t changed)
{
    lv_obj_t *picker=lv_cont_create(parent,NULL);lv_signal_func_t base=lv_obj_get_signal_func(picker);
    color_picker_ext *ext=lv_obj_allocate_ext_attr(picker,sizeof(*ext));if(!ext){lv_obj_del(picker);return NULL;}
    memset((u8 *)ext+sizeof(ext->container),0,sizeof(*ext)-sizeof(ext->container));
    ext->base_signal=base;ext->changed=changed;ext->width=width*3/5;ext->height=height-64;
    ext->cached_brightness=101;
    if(ext->width<2||ext->height<2||ext->width>640||ext->height>360){lv_obj_del(picker);return NULL;}
    ext->pixels=malloc(ext->width*ext->height*sizeof(lv_color_t));if(!ext->pixels){lv_obj_del(picker);return NULL;}
    lv_obj_set_signal_func(picker,color_picker_signal);
    lv_cont_set_style(picker,&lv_style_transp_tight);lv_cont_set_layout(picker,LV_LAYOUT_OFF);lv_cont_set_fit(picker,false,false);
    lv_obj_set_size(picker,width,height);lv_obj_set_pos(picker,x,y);lv_obj_set_click(picker,false);
    ext->image.header.cf=LV_IMG_CF_TRUE_COLOR;ext->image.header.w=ext->width;ext->image.header.h=ext->height;
    ext->image.data_size=ext->width*ext->height*sizeof(lv_color_t);ext->image.data=(const u8 *)ext->pixels;
    ext->plane=lv_img_create(picker,NULL);lv_img_set_src(ext->plane,&ext->image);lv_obj_set_pos(ext->plane,0,0);lv_obj_set_click(ext->plane,false);
    ext->hit=lv_obj_create(picker,NULL);lv_obj_set_style(ext->hit,&lv_style_transp_tight);lv_obj_set_size(ext->hit,ext->width,ext->height);
    lv_obj_set_pos(ext->hit,0,0);lv_obj_set_free_ptr(ext->hit,picker);ext->plane_signal=lv_obj_get_signal_func(ext->hit);lv_obj_set_signal_func(ext->hit,color_plane_signal);
    lv_obj_set_protect(ext->hit,LV_PROTECT_PRESS_LOST);
    ext->cursor=lv_cont_create(ext->hit,NULL);lv_cont_set_fit(ext->cursor,false,false);
    lv_style_copy(&ext->cursor_style,&lv_style_plain);ext->cursor_style.body.empty=1;ext->cursor_style.body.radius=LV_RADIUS_CIRCLE;
    ext->cursor_style.body.border.width=2;ext->cursor_style.body.border.color=LV_COLOR_WHITE;ext->cursor_style.body.border.opa=LV_OPA_COVER;
    lv_cont_set_style(ext->cursor,&ext->cursor_style);lv_obj_set_size(ext->cursor,13,13);lv_obj_set_click(ext->cursor,false);
    ext->value=lv_slider_create(picker,NULL);lv_slider_set_range(ext->value,0,100);lv_obj_set_size(ext->value,ext->width,20);
    lv_obj_set_pos(ext->value,0,height-22);lv_slider_set_action(ext->value,color_value_action);
    nyx_nova_text(picker,"Luminosité couleur",0,height-55,ext->width,NOVA_SMALL);
    int right=ext->width+24,right_width=width-right;
    ext->swatch=lv_cont_create(picker,NULL);lv_cont_set_fit(ext->swatch,false,false);lv_obj_set_click(ext->swatch,false);
    lv_style_copy(&ext->swatch_style,&lv_style_plain);ext->swatch_style.body.opa=LV_OPA_COVER;ext->swatch_style.body.radius=3;
    ext->swatch_style.body.border.width=1;ext->swatch_style.body.border.color=LV_COLOR_HEX(0x8B99A9);
    lv_obj_set_pos(ext->swatch,right,0);lv_obj_set_size(ext->swatch,right_width,48);
    ext->hex=nyx_nova_text(picker,"",right,68,right_width,NOVA_BODY);
    ext->channels=nyx_nova_text(picker,"",right,112,right_width,NOVA_SMALL);
    nyx_color_picker_set(picker,0x55C9F1);return picker;
}
/* Nova palette, static wallpapers and per-surface opacity.
 * SPDX-License-Identifier: GPL-2.0-or-later */

#define WALLPAPER_LIMIT 32
static lv_img_dsc_t *nova_wallpaper;
static char wallpaper_names[WALLPAPER_LIMIT][64];
static u32 wallpaper_count,launch_count;
static char launch_names[WALLPAPER_LIMIT][64];
static void theme_to_front(lv_obj_t *object)
{
    lv_obj_t *parent=lv_obj_get_parent(object);if(!parent)return;
    lv_ll_move_before(&parent->child_ll,object,lv_ll_get_head(&parent->child_ll));lv_obj_invalidate(object);
}
static void theme_to_back(lv_obj_t *object)
{
    lv_obj_t *parent=lv_obj_get_parent(object);if(!parent)return;
    lv_ll_move_before(&parent->child_ll,object,NULL);lv_obj_invalidate(object);
}

static u32 wallpaper_u32(const u8 *bytes)
{ return (u32)bytes[0] | (u32)bytes[1]<<8 | (u32)bytes[2]<<16 | (u32)bytes[3]<<24; }
static void wallpaper_free(lv_img_dsc_t *image)
{ if(image){free((void *)image->data);free(image);} }

/* Decode a bounded uncompressed 24/32-bit BMP and crop/scale it ONCE. A single
 * source row and the final opaque image are allocated; no per-frame SD I/O.
 * The preview and the real background share this exact checked decoder. */
static lv_img_dsc_t *wallpaper_read_bmp(const char *file, u32 width, u32 height)
{
    if(!file||!*file||strlen(file)>=64||strchr(file,'/')||strchr(file,'\\')||strstr(file,".."))return NULL;
    char path[96];s_printf(path,"wallpapers/%s",file);
    FIL stream; if(f_open(&stream,path,FA_READ)!=FR_OK)return NULL;
    u8 header[54];UINT actual=0;lv_img_dsc_t *image=NULL;u8 *row=NULL;
    u32 bytes=f_size(&stream);
    if(bytes<54||bytes>16*1024*1024||f_read(&stream,header,54,&actual)!=FR_OK||actual!=54)goto done;
    if(header[0]!='B'||header[1]!='M'||wallpaper_u32(header+14)<40||header[26]!=1||header[27]||header[29]||wallpaper_u32(header+30))goto done;
    u32 offset=wallpaper_u32(header+10),declared=wallpaper_u32(header+2),source_width=wallpaper_u32(header+18);
    int signed_height=(int)wallpaper_u32(header+22);
    u32 source_height=signed_height<0?-(u32)signed_height:(u32)signed_height;
    u32 depth=header[28];
    if(!source_width||source_width>1920||!source_height||source_height>1080||(depth!=24&&depth!=32))goto done;
    u32 stride=(source_width*(depth/8)+3)&~3u;
    if(offset<54||offset>bytes||stride*source_height>bytes-offset||declared>bytes||declared<offset+stride*source_height)goto done;
    image=calloc(1,sizeof(*image));if(!image)goto done;
    image->data=malloc(width*height*4);row=malloc(stride);
    if(!image->data||!row){wallpaper_free(image);image=NULL;goto done;}
    image->header.cf=LV_IMG_CF_TRUE_COLOR;image->header.w=width;image->header.h=height;image->data_size=width*height*4;
    u32 crop_width=source_width,crop_height=source_width*height/width;
    if(crop_height>source_height){crop_height=source_height;crop_width=source_height*width/height;}
    if(!crop_width)crop_width=1;
    if(!crop_height)crop_height=1;
    u32 left=(source_width-crop_width)/2,top=(source_height-crop_height)/2,previous=0xFFFFFFFF;
    u32 *pixels=(u32 *)image->data;
    for(u32 y=0;y<height;++y){
        u32 source_y=top+y*crop_height/height;
        if(source_y!=previous){
            u32 physical=signed_height<0?source_y:source_height-1-source_y;
            if(f_lseek(&stream,offset+physical*stride)!=FR_OK||f_read(&stream,row,stride,&actual)!=FR_OK||actual!=stride){wallpaper_free(image);image=NULL;goto done;}
            previous=source_y;
        }
        for(u32 x=0;x<width;++x){const u8 *pixel=row+(left+x*crop_width/width)*(depth/8);pixels[y*width+x]=0xFF000000|((u32)pixel[2]<<16)|((u32)pixel[1]<<8)|pixel[0];}
    }
done:
    free(row);f_close(&stream);return image;
}

/* PNG assets are separate from the full-screen BMP wallpaper folder. */
static lv_img_dsc_t *icon_read(const char *file,u32 width,u32 height,bool cover)
{
    if(!file||!*file||strlen(file)>=64||strchr(file,'/')||strchr(file,'\\')||strstr(file,".."))return NULL;
    char path[96];s_printf(path,"icone/%s",file);FIL stream;
    if(f_open(&stream,path,FA_READ)!=FR_OK)return NULL;
    u32 bytes=f_size(&stream);UINT actual=0;u8 *data=NULL;u32 *pixels=NULL;lv_img_dsc_t *image=NULL;
    if(bytes<33||bytes>8u*1024u*1024u)goto done;
    data=malloc(bytes);if(!data)goto done;
    if(f_read(&stream,data,bytes,&actual)!=FR_OK||actual!=bytes||!nova_image_png(data,bytes,width,height,cover,&pixels))goto done;
    image=calloc(1,sizeof(*image));if(!image){free(pixels);goto done;}
    image->header.cf=LV_IMG_CF_TRUE_COLOR_ALPHA;image->header.w=width;image->header.h=height;
    image->data=(u8 *)pixels;image->data_size=width*height*4;
done:
    free(data);f_close(&stream);return image;
}
static bool image_png_name(const char *file)
{u32 n=strlen(file);return n>=5&&file[n-4]=='.'&&(file[n-3]|32)=='p'&&(file[n-2]|32)=='n'&&(file[n-1]|32)=='g';}
static lv_img_dsc_t *wallpaper_read(const char *file,u32 width,u32 height)
{return image_png_name(file)?icon_read(file,width,height,true):wallpaper_read_bmp(file,width,height);}
/* The image owns its decoded buffer. Its lifetime follows the card, including
 * theme previews and menu reloads; there is no per-frame decode or SD access. */
typedef struct {lv_img_ext_t image;lv_signal_func_t base;lv_img_dsc_t *pixels;} launch_image_ext;
static lv_res_t launch_image_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    launch_image_ext *ext=lv_obj_get_ext_attr(object);
    lv_signal_func_t base=ext->base;lv_img_dsc_t *pixels=ext->pixels;
    lv_res_t result=base(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP)wallpaper_free(pixels);
    return result;
}
static bool image_owned(lv_obj_t *image,lv_img_dsc_t *pixels)
{
    lv_signal_func_t base=lv_obj_get_signal_func(image);
    launch_image_ext *ext=lv_obj_allocate_ext_attr(image,sizeof(*ext));if(!ext)return false;
    ext->base=base;ext->pixels=pixels;lv_obj_set_signal_func(image,launch_image_signal);
    lv_img_set_src(image,pixels);return true;
}
void nyx_nova_custom_icon(lv_obj_t *box,u32 index)
{
    static const char *names[]={"launch.png","configs.png","payloads.png","emummc.png","diagnostic.png"};
    if(index>=5)return;
    bool mounted=sd_get_card_mounted();lv_img_dsc_t *pixels=NULL;
    u32 size=lv_obj_get_width(box)-8;
    if(!sd_mount())pixels=icon_read(names[index],size,size,false);
    if(!mounted)sd_unmount();if(!pixels)return;
    lv_obj_t *old=NULL;
    for(lv_obj_t *child=lv_obj_get_child(box,NULL);child;child=lv_obj_get_child(box,child)){
        lv_obj_type_t type;lv_obj_get_type(child,&type);
        if(type.type[0]&&!strcmp(type.type[0],"lv_img")){old=child;break;}
    }
    lv_obj_t *image=lv_img_create(box,NULL);
    if(!image_owned(image,pixels)){lv_obj_del(image);wallpaper_free(pixels);return;}
    static lv_style_t natural;lv_style_copy(&natural,&lv_style_plain);natural.image.intense=0;natural.image.opa=LV_OPA_COVER;
    lv_img_set_style(image,&natural);lv_obj_set_pos(image,4,4);lv_obj_set_click(image,false);
    if(old)lv_obj_del(old);
}
static bool launch_image_place(lv_obj_t *card,const char *file)
{
    if(!file||!*file)return true;
    bool mounted=sd_get_card_mounted();lv_img_dsc_t *pixels=NULL;
    if(!sd_mount())pixels=wallpaper_read(file,lv_obj_get_width(card)-4,lv_obj_get_height(card)-4);
    if(!mounted)sd_unmount();
    if(!pixels)return false;
    lv_obj_t *image=lv_img_create(card,NULL);lv_signal_func_t base=lv_obj_get_signal_func(image);
    launch_image_ext *ext=lv_obj_allocate_ext_attr(image,sizeof(*ext));
    if(!ext){lv_obj_del(image);wallpaper_free(pixels);return false;}
    ext->base=base;ext->pixels=pixels;lv_obj_set_signal_func(image,launch_image_signal);
    lv_img_set_src(image,pixels);lv_obj_set_pos(image,2,2);lv_obj_set_click(image,false);
    static lv_style_t shade;lv_style_copy(&shade,&lv_style_plain);
    shade.body.main_color=shade.body.grad_color=LV_COLOR_BLACK;shade.body.opa=150;
    shade.body.border.width=shade.body.shadow.width=shade.body.radius=0;
    lv_obj_t *scrim=lv_cont_create(card,NULL);lv_cont_set_style(scrim,&shade);
    lv_cont_set_fit(scrim,false,false);lv_obj_set_size(scrim,lv_obj_get_width(card)-4,lv_obj_get_height(card)-4);
    lv_obj_set_pos(scrim,2,2);lv_obj_set_click(scrim,false);
    return true;
}
void nyx_nova_launch_backdrop(lv_obj_t *card)
{ launch_image_place(card,n_cfg.launch_image); }

void nyx_nova_wallpaper_load(void)
{
    wallpaper_free(nova_wallpaper);nova_wallpaper=NULL;
    if(n_cfg.wallpaper_file[0])nova_wallpaper=wallpaper_read(n_cfg.wallpaper_file,LV_HOR_RES,LV_VER_RES);
}
void nyx_nova_backdrop(lv_obj_t *parent, int y)
{
    if(!nova_wallpaper)return;
    lv_obj_t *image=lv_img_create(lv_scr_act(),NULL);
    lv_obj_set_protect(image,LV_PROTECT_PARENT);
    if(lv_obj_get_parent(image)!=parent)lv_obj_set_parent(image,parent);
    lv_img_set_src(image,nova_wallpaper);
    lv_obj_set_pos(image,0,y);lv_obj_set_click(image,false);lv_obj_set_top(image,false);
    theme_to_back(image);
}

void nyx_nova_screen(lv_obj_t *screen)
{
    /* One opaque root, with the wallpaper explicitly at the back. A separate
     * opaque base child can mask a wallpaper when siblings are reordered. */
    static lv_style_t root_style;
    lv_style_copy(&root_style,&lv_style_plain);
    root_style.body.main_color=root_style.body.grad_color=nyx_nova_color(NOVA_COLOR_BACKGROUND);
    root_style.body.empty=0;root_style.body.opa=LV_OPA_COVER;
    root_style.body.border.width=root_style.body.shadow.width=root_style.body.radius=0;
    root_style.body.padding.hor=root_style.body.padding.ver=root_style.body.padding.inner=0;
    lv_cont_set_style(screen,&root_style);lv_cont_set_layout(screen,LV_LAYOUT_OFF);lv_cont_set_fit(screen,false,false);
    lv_obj_set_size(screen,LV_HOR_RES,LV_VER_RES);nyx_nova_backdrop(screen,0);
}

typedef struct {lv_win_ext_t window;lv_signal_func_t base_signal;} nova_window_ext;
static lv_res_t nova_window_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    nova_window_ext *ext=lv_obj_get_ext_attr(object);
    lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP)nyx_nova_navigation_lock(false);
    return result;
}
void nyx_nova_window(lv_obj_t *win)
{
    if(lv_obj_get_signal_func(win)!=nova_window_signal){
        lv_signal_func_t base=lv_obj_get_signal_func(win);
        nova_window_ext *ext=lv_obj_allocate_ext_attr(win,sizeof(*ext));
        if(ext){ext->base_signal=base;lv_obj_set_signal_func(win,nova_window_signal);nyx_nova_navigation_lock(true);}
    }
    static lv_style_t background;
    lv_style_copy(&background,lv_theme_get_current()->win.bg);
    background.body.empty=0;background.body.opa=LV_OPA_COVER;
    background.body.radius=8;background.body.border.width=1;
    background.body.main_color=background.body.grad_color=nyx_nova_color(NOVA_COLOR_BACKGROUND);
    background.body.border.color=nyx_nova_color(NOVA_COLOR_BORDER);
    lv_win_set_style(win,LV_WIN_STYLE_BG,&background);
    lv_obj_set_pos(win,0,NYX_NOVA_HEADER);lv_obj_set_size(win,LV_HOR_RES,NYX_NOVA_BOTTOM-NYX_NOVA_HEADER);
    lv_win_set_btn_size(win,52);
    lv_page_set_scrl_layout(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page,LV_LAYOUT_OFF);
    if(nova_wallpaper){
        nyx_nova_backdrop(win,-NYX_NOVA_HEADER);
        /* The shared background image hides underlying menu cards. */
        lv_win_ext_t *ext=lv_obj_get_ext_attr(win);
        theme_to_front(ext->page);theme_to_front(ext->header);
    }
    /* Window borders are static. */
}

static struct {
    bool open;
    nyx_config pending;
    lv_obj_t *window,*parent_close,*tabs,*target_choices,*menu_choices,*icon_choices,*picker,*file,*counter;
    lv_obj_t *preview,*sample,*sample_text,*sample_icon,*sample_glyph,*sample_option_glyph,*preview_image,*preview_caption;
    lv_style_t preview_style,sample_style,sample_text_style,sample_icon_style,reflection_style,glyph_style,option_style;
    lv_img_dsc_t *thumbnail;
    u32 target,wallpaper,boot_target,launch;
    lv_obj_t *boot_picker,*boot_logo,*boot_bar,*boot_choices,*launch_file,*launch_counter,*launch_preview,*launch_card;
    lv_style_t boot_logo_style,boot_bar_style;
} nova_theme;
enum { THEME_TARGET=40,THEME_MENU_OPACITY,THEME_ICON_OPACITY,THEME_BOOT_TARGET };

static void theme_notice(const char *message)
{ nyx_show_help(message); }
static u32 *theme_target_color(void)
{
    return nova_theme.target==NOVA_COLOR_TEXT?&nova_theme.pending.nova_text_color:
        nova_theme.target==NOVA_COLOR_BORDER?&nova_theme.pending.nova_border_color:
        nova_theme.target==NOVA_COLOR_LOGO?&nova_theme.pending.nova_logo_color:
        nova_theme.target==NOVA_COLOR_ICONS?&nova_theme.pending.nova_icon_color:
        nova_theme.target==NOVA_COLOR_SELECTION?&nova_theme.pending.nova_selection_color:&nova_theme.pending.theme_bg;
}
static void theme_preview_refresh(void)
{
    u32 bg=nova_theme.pending.theme_bg==0x141A19?0x10151D:nova_theme.pending.theme_bg;
    lv_style_copy(&nova_theme.preview_style,lv_theme_get_current()->panel);
    nova_theme.preview_style.body.empty=0;nova_theme.preview_style.body.opa=LV_OPA_COVER;
    nova_theme.preview_style.body.main_color=nova_theme.preview_style.body.grad_color=LV_COLOR_HEX(bg);
    nova_theme.preview_style.body.border.color=LV_COLOR_HEX(nova_theme.pending.nova_border_color);
    lv_cont_set_style(nova_theme.preview,&nova_theme.preview_style);
    lv_style_copy(&nova_theme.sample_style,&nova_theme.preview_style);
    nova_theme.sample_style.body.main_color=nova_theme.sample_style.body.grad_color=LV_COLOR_HEX(nova_theme.pending.nova_selection_color);
    nova_theme.sample_style.body.opa=nova_theme.pending.nova_menu_opacity*255/100;
    lv_cont_set_style(nova_theme.sample,&nova_theme.sample_style);
    lv_style_copy(&nova_theme.sample_icon_style,&nova_theme.sample_style);
    nova_theme.sample_icon_style.body.opa=nova_theme.pending.nova_icon_opacity*255/100;
    lv_cont_set_style(nova_theme.sample_icon,&nova_theme.sample_icon_style);
    lv_style_copy(&nova_theme.glyph_style,&lv_style_plain);
    nova_theme.glyph_style.image.color=LV_COLOR_HEX(nova_theme.pending.nova_logo_color);
    nova_theme.glyph_style.image.opa=LV_OPA_COVER;
    lv_img_set_style(nova_theme.sample_glyph,&nova_theme.glyph_style);
    lv_style_copy(&nova_theme.option_style,&nova_theme.glyph_style);
    nova_theme.option_style.image.color=LV_COLOR_HEX(nova_theme.pending.nova_icon_color);
    lv_img_set_style(nova_theme.sample_option_glyph,&nova_theme.option_style);
    lv_style_copy(&nova_theme.sample_text_style,nyx_nova_text_style(NOVA_TITLE));
    nova_theme.sample_text_style.text.color=LV_COLOR_HEX(nova_theme.pending.nova_text_color);
    lv_label_set_style(nova_theme.sample_text,&nova_theme.sample_text_style);
    lv_style_copy(&nova_theme.reflection_style,nyx_nova_text_style(NOVA_SMALL));
    nova_theme.reflection_style.text.color=LV_COLOR_WHITE;
    lv_label_set_style(nova_theme.preview_caption,&nova_theme.reflection_style);
}
static void theme_controls_refresh(void)
{
    u32 value=*theme_target_color();if(nova_theme.target==NOVA_COLOR_BACKGROUND&&value==0x141A19)value=0x10151D;
    nyx_color_picker_set(nova_theme.picker,value);
    theme_preview_refresh();
}
static void boot_preview_refresh(void)
{
    lv_style_copy(&nova_theme.boot_logo_style,&lv_style_plain);
    nova_theme.boot_logo_style.image.color=LV_COLOR_HEX(nova_theme.pending.boot_logo_color);
    nova_theme.boot_logo_style.image.opa=LV_OPA_COVER;
    lv_img_set_style(nova_theme.boot_logo,&nova_theme.boot_logo_style);
    lv_style_copy(&nova_theme.boot_bar_style,&lv_style_plain);
    nova_theme.boot_bar_style.body.main_color=nova_theme.boot_bar_style.body.grad_color=LV_COLOR_HEX(nova_theme.pending.boot_bar_color);
    nova_theme.boot_bar_style.body.border.width=nova_theme.boot_bar_style.body.radius=0;
    lv_cont_set_style(nova_theme.boot_bar,&nova_theme.boot_bar_style);
}
static void boot_controls_refresh(void)
{
    nyx_color_picker_set(nova_theme.boot_picker,nova_theme.boot_target?nova_theme.pending.boot_bar_color:nova_theme.pending.boot_logo_color);
    boot_preview_refresh();
}
static void boot_picker_changed(lv_obj_t *picker,u32 color)
{
    (void)picker;
    if(nova_theme.boot_target)nova_theme.pending.boot_bar_color=color;
    else nova_theme.pending.boot_logo_color=color;
    boot_preview_refresh();
}
static lv_res_t theme_select(lv_obj_t *button)
{
    u32 group=lv_obj_get_free_num(lv_obj_get_parent(button)),value=lv_obj_get_free_num(button);
    if(group==THEME_TARGET){static const u32 roles[]={NOVA_COLOR_BACKGROUND,NOVA_COLOR_TEXT,NOVA_COLOR_BORDER,NOVA_COLOR_LOGO,NOVA_COLOR_ICONS,NOVA_COLOR_SELECTION};nova_theme.target=value<6?roles[value]:0;theme_controls_refresh();}
    else if(group==THEME_BOOT_TARGET){nova_theme.boot_target=value==1;boot_controls_refresh();}
    else if(group==THEME_MENU_OPACITY){nova_theme.pending.nova_menu_opacity=value<=4?value*25:100;theme_preview_refresh();}
    else if(group==THEME_ICON_OPACITY){nova_theme.pending.nova_icon_opacity=value<=4?value*25:100;theme_preview_refresh();}
    return LV_RES_OK;
}
static void theme_picker_changed(lv_obj_t *picker,u32 color)
{
    (void)picker;*theme_target_color()=color;theme_preview_refresh();
}

static void wallpaper_scan(void)
{
    wallpaper_count=0;DIR directory;FILINFO entry;
    if(f_opendir(&directory,"wallpapers")!=FR_OK)return;
    while(wallpaper_count<WALLPAPER_LIMIT&&f_readdir(&directory,&entry)==FR_OK&&entry.fname[0]){
        if(entry.fattrib&(AM_DIR|AM_HID))continue;
        u32 bytes=strlen(entry.fname);
        if(bytes<5||bytes>=64||entry.fname[0]=='.'||strstr(entry.fname,".."))continue;
        if(entry.fname[bytes-4]!='.'||(entry.fname[bytes-3]|32)!='b'||(entry.fname[bytes-2]|32)!='m'||(entry.fname[bytes-1]|32)!='p')continue;
        bool valid=true;for(u32 i=0;i<bytes;++i){char c=entry.fname[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'||c==' ')){valid=false;break;}}
        if(valid)strcpy(wallpaper_names[wallpaper_count++],entry.fname);
    }
    f_closedir(&directory);
    for(u32 i=1;i<wallpaper_count;++i)for(u32 j=i;j>0&&strcmp(wallpaper_names[j-1],wallpaper_names[j])>0;--j){char temp[64];strcpy(temp,wallpaper_names[j]);strcpy(wallpaper_names[j],wallpaper_names[j-1]);strcpy(wallpaper_names[j-1],temp);}
}
static void launch_scan(void)
{
    launch_count=0;
    DIR directory;FILINFO entry;
    if(f_opendir(&directory,"icone")!=FR_OK)goto legacy_bmp;
    while(launch_count<WALLPAPER_LIMIT&&f_readdir(&directory,&entry)==FR_OK&&entry.fname[0]){
        if(entry.fattrib&(AM_DIR|AM_HID)||!image_png_name(entry.fname)||strlen(entry.fname)>=64||entry.fname[0]=='.'||strstr(entry.fname,".."))continue;
        bool valid=true;for(u32 i=0;entry.fname[i];++i){char c=entry.fname[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'||c==' ')){valid=false;break;}}
        if(valid)strcpy(launch_names[launch_count++],entry.fname);
    }
    f_closedir(&directory);
legacy_bmp:
    for(u32 i=0;i<wallpaper_count&&launch_count<WALLPAPER_LIMIT;++i)strcpy(launch_names[launch_count++],wallpaper_names[i]);
}
static void wallpaper_preview_refresh(void)
{
    if(nova_theme.preview_image){lv_obj_del(nova_theme.preview_image);nova_theme.preview_image=NULL;}
    wallpaper_free(nova_theme.thumbnail);nova_theme.thumbnail=NULL;
    const char *file=nova_theme.pending.wallpaper_file;
    (lv_label_set_text)(nova_theme.file,*file?file:nyx_tr("Aucun fond d’écran"));
    char position[24];s_printf(position,"%d / %d",nova_theme.wallpaper+1,wallpaper_count+1);(lv_label_set_text)(nova_theme.counter,position);
    bool mounted=sd_get_card_mounted();
    if(*file&&!sd_mount())nova_theme.thumbnail=wallpaper_read(file,516,405);
    if(!mounted)sd_unmount();
    if(nova_theme.thumbnail){nova_theme.preview_image=lv_img_create(nova_theme.preview,NULL);lv_img_set_src(nova_theme.preview_image,nova_theme.thumbnail);lv_obj_set_pos(nova_theme.preview_image,0,0);lv_obj_set_click(nova_theme.preview_image,false);theme_to_back(nova_theme.preview_image);}
    else if(*file)theme_notice("Image introuvable ou invalide. Utilisez un PNG dans icone ou un BMP non compressé dans wallpapers.");
}
static lv_res_t wallpaper_step(lv_obj_t *button)
{
    nova_theme.wallpaper=(nova_theme.wallpaper+(lv_obj_get_free_num(button)?1:wallpaper_count))%(wallpaper_count+1);
    nova_theme.pending.wallpaper_file[0]=0;
    if(nova_theme.wallpaper)strcpy(nova_theme.pending.wallpaper_file,wallpaper_names[nova_theme.wallpaper-1]);
    wallpaper_preview_refresh();return LV_RES_OK;
}
static void launch_preview_refresh(void)
{
    if(nova_theme.launch_card)lv_obj_del(nova_theme.launch_card);
    nova_theme.launch_card=nyx_nova_panel(nova_theme.launch_preview,20,70,476,292);
    bool valid=launch_image_place(nova_theme.launch_card,nova_theme.pending.launch_image);
    nyx_nova_icon_box(nova_theme.launch_card,"rocket",24,24,56,false);
    nyx_nova_text(nova_theme.launch_card,"Launch",24,166,428,NOVA_TITLE);
    lv_obj_t *caption=nyx_nova_text(nova_theme.launch_card,"Choisir un système",24,214,428,NOVA_BODY);nyx_nova_fit_text(caption,50);
    (lv_label_set_text)(nova_theme.launch_file,nova_theme.pending.launch_image[0]?nova_theme.pending.launch_image:nyx_tr("Aucune image"));
    char count[24];s_printf(count,"%d / %d",nova_theme.launch+1,launch_count+1);(lv_label_set_text)(nova_theme.launch_counter,count);
    if(!valid)theme_notice("Image introuvable ou invalide. Utilisez un PNG dans icone ou un BMP non compressé dans wallpapers.");
}
static lv_res_t launch_step(lv_obj_t *button)
{
    nova_theme.launch=(nova_theme.launch+(lv_obj_get_free_num(button)?1:launch_count))%(launch_count+1);
    nova_theme.pending.launch_image[0]=0;
    if(nova_theme.launch)strcpy(nova_theme.pending.launch_image,launch_names[nova_theme.launch-1]);
    launch_preview_refresh();return LV_RES_OK;
}
static void theme_apply(const nyx_config *source)
{
    n_cfg.theme_bg=source->theme_bg;n_cfg.nova_text_color=source->nova_text_color;
    n_cfg.nova_logo_color=source->nova_logo_color;n_cfg.nova_icon_color=source->nova_icon_color;n_cfg.nova_selection_color=source->nova_selection_color;
    n_cfg.nova_border_color=source->nova_border_color;n_cfg.nova_reflection_color=source->nova_reflection_color;
    n_cfg.nova_menu_opacity=source->nova_menu_opacity;n_cfg.nova_icon_opacity=source->nova_icon_opacity;
    memcpy(n_cfg.wallpaper_file,source->wallpaper_file,sizeof(n_cfg.wallpaper_file));
    memcpy(n_cfg.launch_image,source->launch_image,sizeof(n_cfg.launch_image));
    n_cfg.boot_logo_color=source->boot_logo_color;n_cfg.boot_bar_color=source->boot_bar_color;
}
static lv_res_t theme_save(lv_obj_t *button)
{
    (void)button;bool mounted=sd_get_card_mounted();
    if(sd_mount()){theme_notice("SD card unavailable. Nothing was saved.");return LV_RES_OK;}
    if(f_stat("bootloader/sys/nyx.bin",NULL)!=FR_OK){if(!mounted)sd_unmount();theme_notice("Nyx is missing. Nothing was saved.");return LV_RES_OK;}
    if(nova_theme.pending.launch_image[0]){
        lv_img_dsc_t *check=wallpaper_read(nova_theme.pending.launch_image,32,18);
        if(!check){if(!mounted)sd_unmount();theme_notice("Image introuvable ou invalide. Utilisez un PNG dans icone ou un BMP non compressé dans wallpapers.");return LV_RES_OK;}
        wallpaper_free(check);
    }
    if(nova_theme.pending.wallpaper_file[0]){
        lv_img_dsc_t *check=wallpaper_read(nova_theme.pending.wallpaper_file,32,18);
        if(!check){if(!mounted)sd_unmount();theme_notice("Image introuvable ou invalide. Utilisez un PNG dans icone ou un BMP non compressé dans wallpapers.");return LV_RES_OK;}
        wallpaper_free(check);
    }
    if(!nyx_options_commit_boot()){if(!mounted)sd_unmount();theme_notice("Could not save settings. Check the SD card and retry.");return LV_RES_OK;}
    nyx_config previous=n_cfg;theme_apply(&nova_theme.pending);
    if(create_nyx_config_entry(true)){theme_apply(&previous);theme_notice("Could not save appearance settings. Check the SD card and try again.");return LV_RES_OK;}
    nyx_options_appearance_active(true,true);reload_nyx(NULL,false);
    theme_notice("Settings saved. Reload Nyx to apply the changes.");return LV_RES_OK;
}
static lv_res_t theme_close(lv_obj_t *button)
{
    lv_res_t result=nyx_win_close_action(button);
    wallpaper_free(nova_theme.thumbnail);nova_theme.thumbnail=NULL;
    nova_theme.open=false;close_btn=nova_theme.parent_close;nova_theme.parent_close=NULL;
    nyx_options_appearance_active(false,false);return result;
}
static lv_res_t theme_reset(lv_obj_t *button)
{
    (void)button;nova_theme.pending.theme_bg=0x10151D;nova_theme.pending.nova_text_color=0xF2F5FA;
    nova_theme.pending.nova_border_color=0x334050;nova_theme.pending.nova_reflection_color=0x55C9F1;
    nova_theme.pending.nova_logo_color=nova_theme.pending.nova_icon_color=0x55C9F1;nova_theme.pending.nova_selection_color=0x1C2937;
    nova_theme.pending.nova_menu_opacity=25;nova_theme.pending.nova_icon_opacity=100;
    strcpy(nova_theme.pending.wallpaper_file,"Nova-Carbon.bmp");nova_theme.wallpaper=0;
    nova_theme.pending.launch_image[0]=0;nova_theme.launch=0;
    nova_theme.pending.boot_logo_color=nova_theme.pending.boot_bar_color=0x55C9F1;
    for(u32 i=0;i<wallpaper_count;++i)if(!strcmp(wallpaper_names[i],"Nova-Carbon.bmp"))nova_theme.wallpaper=i+1;
    nyx_choices_set_selected(nova_theme.menu_choices,1);nyx_choices_set_selected(nova_theme.icon_choices,4);
    theme_controls_refresh();wallpaper_preview_refresh();boot_controls_refresh();launch_preview_refresh();return LV_RES_OK;
}
static void theme_page(lv_obj_t *page)
{
    lv_page_set_style(page,LV_PAGE_STYLE_BG,&lv_style_transp_tight);lv_page_set_style(page,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
    lv_page_set_scrl_fit(page,false,false);lv_page_set_scrl_layout(page,LV_LAYOUT_OFF);lv_page_set_scrl_width(page,LV_HOR_RES);lv_page_set_scrl_height(page,lv_obj_get_height(page));
    lv_obj_set_pos(lv_page_get_scrl(page),0,0);lv_page_set_sb_mode(page,LV_SB_MODE_OFF);lv_obj_set_drag(lv_page_get_scrl(page),false);
}
static lv_obj_t *theme_button(lv_obj_t *parent,const char *text,lv_action_t action,int x,int y,int width,u32 id)
{
    lv_obj_t *button=lv_btn_create(parent,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    lv_obj_set_size(button,width,48);lv_obj_set_pos(button,x,y);lv_obj_set_free_num(button,id);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,action);
    lv_obj_t *label=nyx_nova_text(button,text,12,13,width-24,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);return button;
}

lv_res_t nyx_nova_theme_window(lv_obj_t *button)
{
    (void)button;if(nova_theme.open)return LV_RES_OK;
    memset(&nova_theme,0,sizeof(nova_theme));nova_theme.open=true;nova_theme.pending=n_cfg;nova_theme.parent_close=close_btn;
    wallpaper_count=0;
    bool mounted=sd_get_card_mounted();if(!sd_mount()){wallpaper_scan();launch_scan();}if(!mounted)sd_unmount();
    for(u32 i=0;i<wallpaper_count;++i)if(!strcmp(wallpaper_names[i],n_cfg.wallpaper_file))nova_theme.wallpaper=i+1;
    for(u32 i=0;i<launch_count;++i)if(!strcmp(launch_names[i],n_cfg.launch_image))nova_theme.launch=i+1;
    lv_obj_t *win=nyx_create_standard_window("Thèmes et couleurs",theme_close);nova_theme.window=win;
    lv_win_add_btn(win,NULL,nyx_tr("Réinitialiser"),theme_reset);lv_win_add_btn(win,NULL,nyx_tr("Save & reload"),theme_save);
    lv_win_ext_t *ext=lv_obj_get_ext_attr(win);theme_page(ext->page);
    lv_obj_t *tabs=lv_tabview_create(ext->page,NULL);nova_theme.tabs=tabs;
    lv_obj_set_size(tabs,LV_HOR_RES,lv_obj_get_height(ext->page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *colors=lv_tabview_add_tab(tabs,"Couleurs");theme_page(colors);
    lv_obj_t *wallpaper=lv_tabview_add_tab(tabs,"Fond et transparence");theme_page(wallpaper);
    lv_obj_t *boot=lv_tabview_add_tab(tabs,"Démarrage");theme_page(boot);
    lv_obj_t *launch_tab=lv_tabview_add_tab(tabs,"Tuile Launch");theme_page(launch_tab);
    int panel_height=lv_obj_get_height(colors)-28;if(panel_height>405)panel_height=405;
    lv_obj_t *palette=nyx_nova_panel(colors,34,14,672,panel_height);
    nova_theme.preview=nyx_nova_panel(colors,730,14,516,panel_height);
    nova_theme.sample=nyx_nova_panel(nova_theme.preview,28,94,460,218);
    nova_theme.sample_icon=nyx_nova_icon_box(nova_theme.sample,"brand-mark",24,24,56,false);
    nova_theme.sample_glyph=lv_obj_get_child(nova_theme.sample_icon,NULL);
    nova_theme.sample_option_glyph=nyx_nova_glyph(nova_theme.sample,"settings-2",386,40,24);
    nova_theme.sample_text=nyx_nova_text(nova_theme.sample,"Hekate Nova Pro",24,106,410,NOVA_TITLE);nyx_nova_fit_text(nova_theme.sample_text,52);
    nova_theme.preview_caption=nyx_nova_text(nova_theme.sample,"Reflet blanc discret",24,174,410,NOVA_SMALL);
    nyx_nova_text(nova_theme.preview,"Aperçu",28,24,400,NOVA_TITLE);
    nova_theme.target_choices=nyx_choices_create(palette,"Fond\nTextes\nContours\nLogo\nIcônes\nSélection",0,3,624,42,theme_select,THEME_TARGET,false);lv_obj_set_pos(nova_theme.target_choices,24,20);
    nova_theme.picker=nyx_color_picker_create(palette,24,132,624,250,theme_picker_changed);
    lv_obj_t *left=nyx_nova_panel(wallpaper,34,14,672,panel_height);
    nyx_nova_text(left,"Fond d’écran fixe",24,22,620,NOVA_TITLE);
    nova_theme.file=nyx_nova_text(left,"",24,78,620,NOVA_BODY);nyx_nova_fit_text(nova_theme.file,52);
    theme_button(left,"Précédent",wallpaper_step,24,148,270,0);theme_button(left,"Suivant",wallpaper_step,318,148,270,1);
    nova_theme.counter=nyx_nova_text(left,"",24,218,620,NOVA_SMALL);
    lv_obj_t *help=nyx_nova_text(left,"Copiez vos images BMP dans le dossier wallpapers à la racine de la carte SD.",24,268,620,NOVA_SMALL);nyx_nova_fit_text(help,76);
    help=nyx_nova_text(left,"BMP 24/32 bits · 1920 × 1080 maximum · recadrage automatique",24,354,620,NOVA_KICKER);nyx_nova_fit_text(help,38);
    lv_obj_t *right=nyx_nova_panel(wallpaper,730,14,516,panel_height);
    nyx_nova_text(right,"Transparence",24,22,460,NOVA_TITLE);
    nyx_nova_text(right,"Fond des menus",24,88,460,NOVA_BODY);
    lv_obj_t *choices=nyx_choices_create(right,"0%\n25%\n50%\n75%\n100%",n_cfg.nova_menu_opacity/25,5,468,52,theme_select,THEME_MENU_OPACITY,true);lv_obj_set_pos(choices,24,130);
    nova_theme.menu_choices=choices;
    nyx_nova_text(right,"Fond des icônes",24,220,460,NOVA_BODY);
    choices=nyx_choices_create(right,"0%\n25%\n50%\n75%\n100%",n_cfg.nova_icon_opacity/25,5,468,52,theme_select,THEME_ICON_OPACITY,true);lv_obj_set_pos(choices,24,262);
    nova_theme.icon_choices=choices;
    help=nyx_nova_text(right,"0% : transparent · 100% : opaque. Les textes et symboles restent lisibles.",24,338,468,NOVA_SMALL);nyx_nova_fit_text(help,60);
    left=nyx_nova_panel(boot,34,14,672,panel_height);
    nova_theme.boot_choices=nyx_choices_create(left,"Logo de démarrage\nBarre de chargement",0,2,624,52,theme_select,THEME_BOOT_TARGET,false);lv_obj_set_pos(nova_theme.boot_choices,24,20);
    nova_theme.boot_picker=nyx_color_picker_create(left,24,106,624,250,boot_picker_changed);
    right=nyx_nova_panel(boot,730,14,516,panel_height);
    nyx_nova_text(right,"Aperçu du démarrage",24,22,468,NOVA_TITLE);
    nova_theme.boot_logo=nyx_nova_glyph(right,"brand-mark",210,100,96);
    nyx_nova_text(right,"Hekate Nova Pro",24,220,468,NOVA_TITLE);
    nova_theme.boot_bar=lv_cont_create(right,NULL);lv_cont_set_fit(nova_theme.boot_bar,false,false);lv_obj_set_size(nova_theme.boot_bar,300,4);lv_obj_set_pos(nova_theme.boot_bar,108,280);
    help=nyx_nova_text(right,"Les deux couleurs sont indépendantes. Le changement s’applique au prochain démarrage.",24,316,468,NOVA_SMALL);nyx_nova_fit_text(help,72);
    left=nyx_nova_panel(launch_tab,34,14,672,panel_height);
    nyx_nova_text(left,"Image de la tuile Launch",24,22,620,NOVA_TITLE);
    nova_theme.launch_file=nyx_nova_text(left,"",24,88,620,NOVA_BODY);nyx_nova_fit_text(nova_theme.launch_file,54);
    theme_button(left,"Précédent",launch_step,24,160,270,0);theme_button(left,"Suivant",launch_step,318,160,270,1);
    nova_theme.launch_counter=nyx_nova_text(left,"",24,228,620,NOVA_SMALL);
    help=nyx_nova_text(left,"Choisissez un PNG transparent du dossier icone. L’image couvre uniquement la tuile Launch. Les anciens BMP restent disponibles.",24,276,620,NOVA_SMALL);nyx_nova_fit_text(help,104);
    nova_theme.launch_preview=nyx_nova_panel(launch_tab,730,14,516,panel_height);
    nyx_nova_text(nova_theme.launch_preview,"Aperçu",24,22,468,NOVA_TITLE);
    theme_controls_refresh();wallpaper_preview_refresh();boot_controls_refresh();launch_preview_refresh();nyx_options_appearance_active(true,false);return LV_RES_OK;
}
/* Bounded Nova reports, properties and transfer views. GPL-2.0-or-later. */

void nyx_nova_fixed_page(lv_obj_t *page)
{
    lv_page_set_style(page,LV_PAGE_STYLE_BG,&lv_style_transp_tight);
    lv_page_set_style(page,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
    lv_page_set_scrl_fit(page,false,false);lv_page_set_scrl_layout(page,LV_LAYOUT_OFF);
    lv_page_set_scrl_width(page,lv_obj_get_width(page));lv_page_set_scrl_height(page,lv_obj_get_height(page));
    lv_obj_set_pos(lv_page_get_scrl(page),0,0);lv_page_set_sb_mode(page,LV_SB_MODE_OFF);lv_obj_set_drag(lv_page_get_scrl(page),false);
}

void nyx_nova_align(lv_obj_t *object,const lv_obj_t *base,lv_align_t alignment,lv_coord_t x,lv_coord_t y)
{
    lv_obj_type_t type;lv_obj_get_type(object,&type);
    bool modal=!base&&alignment==LV_ALIGN_CENTER&&type.type[0]&&!strcmp(type.type[0],"lv_mbox");
    int available=NYX_NOVA_BOTTOM-NYX_NOVA_HEADER-24;
    if(modal&&lv_obj_get_height(object)>available){
        lv_obj_set_width(object,LV_HOR_RES-68);
        lv_mbox_ext_t *ext=lv_obj_get_ext_attr(object);
        if(lv_obj_get_height(object)>available)lv_label_set_style(ext->text,(lv_style_t *)nyx_nova_text_style(NOVA_SMALL));
        if(lv_obj_get_height(object)>available)lv_label_set_style(ext->text,(lv_style_t *)nyx_nova_text_style(NOVA_KICKER));
    }
    (lv_obj_align)(object,base,alignment,x,y);
    if(modal){
        int height=lv_obj_get_height(object),top=NYX_NOVA_HEADER+(NYX_NOVA_BOTTOM-NYX_NOVA_HEADER-height)/2+y;
        if(top<NYX_NOVA_HEADER+12)top=NYX_NOVA_HEADER+12;
        if(top+height>NYX_NOVA_BOTTOM-12)top=NYX_NOVA_BOTTOM-12-height;
        lv_obj_set_y(object,top);
    }
}

typedef struct {
    lv_cont_ext_t container;
    lv_signal_func_t base_signal;
    char *text,*slice;
    u32 starts[128],page,next;
    int text_height;
    lv_obj_t *label,*counter,*previous,*following;
} report_ext;
static void report_fill(lv_obj_t *panel)
{
    report_ext *ext=lv_obj_get_ext_attr(panel);u32 start=ext->starts[ext->page],at=start,last=start;
    size_t length=strlen(ext->text);
    while(at<length){
        const char *newline=strchr(ext->text+at,'\n');u32 end=newline?(u32)(newline-ext->text)+1:length;
        memcpy(ext->slice,ext->text+start,end-start);ext->slice[end-start]=0;
        (lv_label_set_text)(ext->label,ext->slice);
        if(lv_obj_get_height(ext->label)>ext->text_height&&last>start)break;
        last=end;at=end;
    }
    if(last==start&&at<length)last=length;
    memcpy(ext->slice,ext->text+start,last-start);ext->slice[last-start]=0;
    (lv_label_set_text)(ext->label,ext->slice);ext->next=last;
    bool has_next=last<length&&ext->page+1<128;
    lv_obj_set_click(ext->previous,ext->page!=0);lv_btn_set_state(ext->previous,ext->page?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    lv_obj_set_click(ext->following,has_next);lv_btn_set_state(ext->following,has_next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    char position[32];s_printf(position,nyx_tr("Page %d"),ext->page+1);(lv_label_set_text)(ext->counter,position);
}
static lv_res_t report_step(lv_obj_t *button)
{
    lv_obj_t *panel=lv_obj_get_parent(button);report_ext *ext=lv_obj_get_ext_attr(panel);
    if(lv_obj_get_free_num(button)){
        if(ext->next>=strlen(ext->text)||ext->page+1>=128)return LV_RES_OK;
        ext->starts[++ext->page]=ext->next;
    }else if(ext->page)--ext->page;
    report_fill(panel);return LV_RES_OK;
}
static lv_res_t report_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    report_ext *ext=lv_obj_get_ext_attr(object);lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP){free(ext->text);free(ext->slice);}
    return result;
}
static lv_obj_t *report_button(lv_obj_t *parent,const char *text,int x,int y,int width,u32 id)
{
    lv_obj_t *button=lv_btn_create(parent,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    lv_obj_set_pos(button,x,y);lv_obj_set_size(button,width,42);lv_obj_set_free_num(button,id);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,report_step);
    lv_obj_t *label=nyx_nova_text(button,text,8,10,width-16,NOVA_SMALL);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);return button;
}
lv_obj_t *nyx_nova_report(lv_obj_t *parent,const char *text,int x,int y,int width,int height)
{
    lv_obj_t *panel=nyx_nova_panel(parent,x,y,width,height);lv_signal_func_t base=lv_obj_get_signal_func(panel);
    report_ext *ext=lv_obj_allocate_ext_attr(panel,sizeof(*ext));if(!ext)return panel;
    /* Preserve the inherited container extension when initializing our tail. */
    memset((u8 *)ext+sizeof(ext->container),0,sizeof(*ext)-sizeof(ext->container));ext->base_signal=base;
    char *translated=nyx_translate_text(text);ext->text=translated;
    if(!ext->text){ext->text=malloc(strlen(text)+1);if(ext->text)strcpy(ext->text,text);}
    ext->slice=ext->text?malloc(strlen(ext->text)+1):NULL;
    if(!ext->text||!ext->slice){free(ext->text);free(ext->slice);ext->text=ext->slice=NULL;return panel;}
    lv_obj_set_signal_func(panel,report_signal);ext->text_height=height-92;
    ext->label=lv_label_create(panel,NULL);lv_label_set_style(ext->label,&monospace_text);lv_label_set_recolor(ext->label,true);lv_label_set_long_mode(ext->label,LV_LABEL_LONG_BREAK);
    lv_obj_set_width(ext->label,width-48);lv_obj_set_pos(ext->label,24,18);
    ext->previous=report_button(panel,"Précédent",24,height-60,180,0);
    ext->following=report_button(panel,"Suivant",width-204,height-60,180,1);
    ext->counter=nyx_nova_text(panel,"",220,height-48,width-440,NOVA_SMALL);lv_label_set_align(ext->counter,LV_LABEL_ALIGN_CENTER);
    report_fill(panel);return panel;
}

void nyx_nova_report_style(lv_obj_t *panel,const lv_style_t *style)
{
    report_ext *ext=lv_obj_get_ext_attr(panel);
    if(!ext->text||!ext->slice)return;
    lv_label_set_style(ext->label,(lv_style_t *)style);ext->page=0;report_fill(panel);
}

static int property_rows(lv_obj_t *parent,const char *labels,const char *values,int width)
{
    int y=20;
    while(*labels){
        const char *label_end=strchr(labels,'\n'),*value_end=strchr(values,'\n');
        u32 ll=label_end?(u32)(label_end-labels):strlen(labels),vl=value_end?(u32)(value_end-values):strlen(values);
        char label[192],value[512];if(ll>=sizeof(label))ll=sizeof(label)-1;if(vl>=sizeof(value))vl=sizeof(value)-1;
        memcpy(label,labels,ll);label[ll]=0;memcpy(value,values,vl);value[vl]=0;
        int row=8;
        if(ll||vl){
            lv_obj_t *name=nyx_nova_text(parent,label,24,y,width*48/100-36,NOVA_BODY);lv_label_set_recolor(name,true);
            lv_obj_t *data=nyx_nova_text(parent,value,width*48/100,y,width*52/100-24,NOVA_BODY);lv_label_set_recolor(data,true);
            row=lv_obj_get_height(name)>lv_obj_get_height(data)?lv_obj_get_height(name):lv_obj_get_height(data);row+=7;
        }
        y+=row;if(!label_end)break;labels=label_end+1;values=value_end?value_end+1:values+strlen(values);
    }
    return y;
}
lv_obj_t *nyx_nova_emmc_summary(lv_obj_t *win,const char *values)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *tabs=lv_tabview_create(page,NULL);lv_obj_set_size(tabs,lv_obj_get_width(page),lv_obj_get_height(page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *device=lv_tabview_add_tab(tabs,"Informations"),*partitions=lv_tabview_add_tab(tabs,"Partitions");nyx_nova_fixed_page(device);nyx_nova_fixed_page(partitions);
    int height=lv_obj_get_height(device)-36;
    lv_obj_t *left=nyx_nova_panel(device,34,18,594,height),*right=nyx_nova_panel(device,652,18,594,height);
    const char *rest=values;for(u32 i=0;i<7&&*rest;++i){const char *next=strchr(rest,'\n');rest=next?next+1:rest+strlen(rest);}
    property_rows(left,"#00DDFF CID:#\nVendor ID:\nModel:\nProd Rev:\nS/N:\nMonth/Year:",values,594);
    property_rows(right,"#00DDFF Ext CSD:#\nCmd Classes:\nMax Bus Rate:\nCurrent Rate:\nEnhanced Area:\nWrite Cache:\n\nMaintenance:\nEstimated Life:\nReserved Used:",rest,594);
    return partitions;
}

void nyx_nova_transfer(lv_obj_t *win,emmc_tool_gui_t *context)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;
    lv_obj_t *left=nyx_nova_panel(page,34,18,680,height),*right=nyx_nova_panel(page,738,18,508,height);
    context->label_info=nyx_nova_text(left,"",24,20,632,NOVA_BODY);lv_label_set_recolor(context->label_info,true);
    context->bar=lv_bar_create(left,NULL);lv_obj_set_size(context->bar,400,20);lv_obj_set_pos(context->bar,24,height-116);lv_bar_set_range(context->bar,0,100);
    context->label_pct=nyx_nova_text(left,"0%",438,height-116,210,NOVA_BODY);lv_label_set_recolor(context->label_pct,true);
    context->label_finish=nyx_nova_text(left,"",24,height-72,632,NOVA_BODY);lv_label_set_recolor(context->label_finish,true);
    lv_obj_t *log=lv_page_create(right,NULL);lv_obj_set_pos(log,20,20);lv_obj_set_size(log,468,height-40);
    lv_page_set_style(log,LV_PAGE_STYLE_BG,&lv_style_transp_tight);lv_page_set_style(log,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);lv_page_set_scrl_layout(log,LV_LAYOUT_OFF);lv_page_set_scrl_fit(log,false,true);
    context->label_log=lv_label_create(log,NULL);lv_label_set_style(context->label_log,&monospace_text);lv_label_set_recolor(context->label_log,true);lv_label_set_long_mode(context->label_log,LV_LABEL_LONG_BREAK);lv_obj_set_width(context->label_log,448);lv_obj_set_pos(context->label_log,0,0);
    static lv_style_t background,indicator,orange_bg,orange_ind,white;
    lv_style_copy(&background,lv_theme_get_current()->bar.bg);lv_style_copy(&indicator,lv_theme_get_current()->bar.indic);
    lv_style_copy(&orange_bg,&background);lv_style_copy(&orange_ind,&indicator);orange_ind.body.main_color=orange_ind.body.grad_color=LV_COLOR_HEX(0xF4A261);
    lv_style_copy(&white,&indicator);white.body.main_color=white.body.grad_color=LV_COLOR_HEX(0xF2F5FA);
    context->bar_teal_bg=&background;context->bar_teal_ind=&indicator;context->bar_orange_bg=&orange_bg;context->bar_orange_ind=&orange_ind;context->bar_white_bg=&background;context->bar_white_ind=&white;
}

void nyx_nova_package_logs(lv_obj_t *win,lv_obj_t **logs)
{
	lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;
	nyx_nova_fixed_page(page);
	lv_obj_t *tabs=lv_tabview_create(page,NULL);
	lv_obj_set_size(tabs,lv_obj_get_width(page),lv_obj_get_height(page));
	lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
	lv_obj_t *main_page=lv_tabview_add_tab(tabs,"Main");
	lv_obj_t *safe_page=lv_tabview_add_tab(tabs,"Safe");
	lv_obj_t *log_pages[]={main_page,safe_page};
	for(u32 i=0;i<2;++i){
		lv_page_set_style(log_pages[i],LV_PAGE_STYLE_BG,&lv_style_transp_tight);
		lv_page_set_style(log_pages[i],LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
		lv_page_set_scrl_fit(log_pages[i],false,true);
		lv_page_set_scrl_width(log_pages[i],lv_obj_get_width(log_pages[i]));
		lv_page_set_sb_mode(log_pages[i],LV_SB_MODE_AUTO);
		logs[i]=lv_label_create(log_pages[i],NULL);
		lv_label_set_style(logs[i],&monospace_text);
		lv_label_set_long_mode(logs[i],LV_LABEL_LONG_BREAK);
		lv_label_set_recolor(logs[i],true);
		lv_obj_set_width(logs[i],lv_obj_get_width(log_pages[i])-68);
		lv_obj_set_pos(logs[i],34,20);lv_label_set_text(logs[i]," ");
	}
}

void nyx_nova_archive_view(lv_obj_t *win,lv_obj_t **labels)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;
    nyx_nova_fixed_page(page);
    int height=lv_obj_get_height(page);
    lv_obj_t *panel=nyx_nova_panel(page,34,22,1212,height-44);
    nyx_nova_icon_box(panel,"folder-open",574,20,64,false);
    lv_obj_t *status=nyx_nova_text(panel,"",36,102,1140,NOVA_BODY);
    lv_label_set_align(status,LV_LABEL_ALIGN_CENTER);lv_label_set_recolor(status,true);
    lv_obj_t *current=nyx_nova_text(panel,"",36,180,1140,NOVA_SMALL);
    lv_label_set_align(current,LV_LABEL_ALIGN_CENTER);
    /* The complete path remains in the traversal buffer. A single-line
     * marquee prevents even a 1024-character path hiding the result below. */
    lv_label_set_long_mode(current,LV_LABEL_LONG_ROLL);
    lv_obj_set_height(current,nyx_font_small()->h_px);
    lv_obj_t *result=nyx_nova_text(panel,"",36,260,1140,NOVA_BODY);
    lv_label_set_align(result,LV_LABEL_ALIGN_CENTER);lv_label_set_recolor(result,true);
    labels[0]=status;labels[1]=current;labels[2]=result;
}
u32 max77620_rtc_date_to_epoch(const rtc_time_t *time)
{
	u32 year, month, epoch;

	//Year
	year = time->year;
	//Month of year
	month = time->month;

	// Month/Year offset.
	if (month < 3)
	{
		month += 12;
		year--;
	}

	epoch  = (365 * year) + (year >> 2) - (year / 100) + (year / 400); // Years to days.

	epoch += (30 * month) + (3 * (month + 1) / 5) + time->day; // Months to days.
	epoch -= 719561; // Epoch time is 1/1/1970.

	epoch *= 86400; // Days to seconds.
	epoch += (3600 * time->hour) + (60 * time->min) + time->sec; // Add hours, minutes and seconds.

	return epoch;
}
#define CLOCK_MIN_YEAR 2025
#define CLOCK_MAX_YEAR (CLOCK_MIN_YEAR + 10)
#define CLOCK_YEARLIST "2025\n2026\n2027\n2028\n2029\n2030\n2031\n2032\n2033\n2034\n2035"
void nyx_create_onoff_button(lv_theme_t *th, lv_obj_t *parent, lv_obj_t *btn, const char *btn_name, lv_action_t action, bool transparent)
{
	// Create buttons that are flat and text, plus On/Off switch.
	static lv_style_t btn_onoff_rel_hos_style, btn_onoff_pr_hos_style;
	lv_style_copy(&btn_onoff_rel_hos_style, th->btn.rel);
	btn_onoff_rel_hos_style.body.shadow.width = 0;
	btn_onoff_rel_hos_style.body.border.width = 0;
	btn_onoff_rel_hos_style.body.padding.hor = 0;
	btn_onoff_rel_hos_style.body.radius = 8;
	btn_onoff_rel_hos_style.body.empty = 0;

	lv_style_copy(&btn_onoff_pr_hos_style, &btn_onoff_rel_hos_style);
	if (transparent)
	{
		btn_onoff_pr_hos_style.body.main_color = LV_COLOR_HEX(0xFFFFFF);
		btn_onoff_pr_hos_style.body.opa = 35;
	}
	else
		btn_onoff_pr_hos_style.body.main_color = th->btn.pr->body.main_color; // COLOR_HOS_BG_LIGHT.
	btn_onoff_pr_hos_style.body.grad_color = btn_onoff_pr_hos_style.body.main_color;
	btn_onoff_pr_hos_style.text.color = th->btn.pr->text.color;
	btn_onoff_pr_hos_style.body.empty = 0;
	btn_onoff_pr_hos_style.body.border.width = 2;
	btn_onoff_pr_hos_style.body.border.color = th->btn.pr->text.color;
	btn_onoff_pr_hos_style.body.border.opa = LV_OPA_COVER;

	lv_obj_t *label_btn = lv_label_create(btn, NULL);
	lv_obj_t *label_btnsw = NULL;

	lv_label_set_recolor(label_btn, true);
	label_btnsw = lv_label_create(btn, NULL);
	lv_label_set_recolor(label_btnsw, true);
	lv_btn_set_layout(btn, LV_LAYOUT_OFF);

	lv_btn_set_style(btn, LV_BTN_STYLE_REL, &btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_PR, &btn_onoff_pr_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_REL, &btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_PR, &btn_onoff_pr_hos_style);

	lv_btn_set_fit(btn, false, true);
	lv_obj_set_width(btn, lv_obj_get_width(parent));
	lv_btn_set_toggle(btn, true);

	lv_label_set_text(label_btn, btn_name);

	lv_label_set_text(label_btnsw, "#D0D0D0 OFF#");
	lv_obj_align(label_btn, btn, LV_ALIGN_IN_LEFT_MID, LV_DPI / 4, 0);
	lv_obj_align(label_btnsw, btn, LV_ALIGN_IN_RIGHT_MID, -LV_DPI / 4, -LV_DPI / 10);

	if (action)
		lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);
}
lv_res_t nyx_mbox_action(lv_obj_t *btns, const char *txt)
{
	lv_obj_t *mbox = lv_mbox_get_from_btn(btns);
	lv_obj_t *dark_bg = lv_obj_get_parent(mbox);

	lv_obj_del(dark_bg); // Deletes children also (mbox).

	return LV_RES_INV;
}
typedef struct _time_edit_obj_t
{
	lv_obj_t *year;
	lv_obj_t *month;
	lv_obj_t *day;
	lv_obj_t *hour;
	lv_obj_t *min;
} time_edit_obj_t;

time_edit_obj_t clock_ctxt;
static u32 clock_language;

static void _clock_apply_selection(void)
{
	rtc_time_t time;
	max77620_rtc_get_time(&time);
	u32 epoch = max77620_rtc_date_to_epoch(&time);

	u32 year  = lv_roller_get_selected(clock_ctxt.year) + CLOCK_MIN_YEAR;
	u32 month = lv_roller_get_selected(clock_ctxt.month) + 1;
	u32 day   = lv_roller_get_selected(clock_ctxt.day) + 1;
	u32 hour  = lv_roller_get_selected(clock_ctxt.hour);
	u32 min   = lv_roller_get_selected(clock_ctxt.min);

	switch (month)
	{
	case 2:
		if (day > ((!(year % 4) && ((year % 100) || !(year % 400))) ? 29u : 28u))
			day = (!(year % 4) && ((year % 100) || !(year % 400))) ? 29 : 28;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if (day > 30)
			day = 30;
		break;
	}

	time.year  = year;
	time.month = month;
	time.day   = day;
	time.hour  = hour;
	time.min   = min;

	u32 new_epoch = max77620_rtc_date_to_epoch(&time);

	// Stored in u32 and allow overflow for integer offset casting.
	n_cfg.timeoffset = new_epoch - epoch;

	// If canceled set 1 for invalidating first boot clock edit.
	if (!n_cfg.timeoffset)
		n_cfg.timeoffset = 1;
	else
	{
		// Adjust for DST between 28 march and 28 october.
		// Good enough to cover all years as week info is not valid.
		u16 md = (time.month << 8) | time.day;
		if (n_cfg.timedst && md >= 0x31C && md < 0xA1C)
			n_cfg.timeoffset -= 3600; // Store time in non DST.
		max77620_rtc_set_epoch_offset((int)n_cfg.timeoffset);
	}

	nyx_changes_made = true;
}

static lv_res_t _action_clock_edit(lv_obj_t *btns, const char *txt)
{
    if(lv_btnm_get_pressed(btns)==1)_clock_apply_selection();
    nyx_mbox_action(btns,txt);return LV_RES_INV;
}

static lv_res_t _action_clock_edit_save(lv_obj_t *btns, const char *txt)
{
    /* Keep Hekate's original first-start dialog; only add a pending language.
     * Save and reload once, after its existing Done button is accepted. */
    if(lv_btnm_get_pressed(btns)!=1)return _action_clock_edit(btns,txt);
    bool mounted=sd_get_card_mounted();
    if(sd_mount()){nyx_show_help("SD card unavailable. Nothing was saved.");return LV_RES_OK;}
    if(f_stat("bootloader/sys/nyx.bin",NULL)!=FR_OK){if(!mounted)sd_unmount();nyx_show_help("Nyx is missing. Nothing was saved.");return LV_RES_OK;}
    if(!nyx_options_commit_boot()){if(!mounted)sd_unmount();nyx_show_help("Could not save settings. Check the SD card and retry.");return LV_RES_OK;}
    nyx_config previous=n_cfg;bool was_dirty=nyx_changes_made;
    _clock_apply_selection();n_cfg.language=clock_language<I18N_LANGUAGE_COUNT?clock_language:I18N_EN;
    if(create_nyx_config_entry(true)){
        n_cfg=previous;nyx_changes_made=was_dirty;
        max77620_rtc_set_epoch_offset(previous.timeoffset==1?0:(int)previous.timeoffset);
        nyx_show_help("Could not save settings. Check the SD card and retry.");return LV_RES_OK;
    }
    nyx_changes_made=false;reload_nyx(NULL,false);
    nyx_mbox_action(btns,txt);return LV_RES_INV;
}

static lv_res_t _clock_language_select(lv_obj_t *button)
{
    u32 language=lv_obj_get_free_num(button);
    if(language<I18N_LANGUAGE_COUNT)clock_language=language;
    return LV_RES_OK;
}

static lv_res_t _action_auto_dst_toggle(lv_obj_t *btn)
{
	n_cfg.timedst = !n_cfg.timedst;
	max77620_rtc_set_auto_dst(n_cfg.timedst);

	if (!n_cfg.timedst)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static const u32 month_days[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static lv_res_t _action_date_validation(lv_obj_t *roller)
{
	u32 year  = lv_roller_get_selected(clock_ctxt.year) + CLOCK_MIN_YEAR;
	u32 month = lv_roller_get_selected(clock_ctxt.month) + 1;
	u32 day   = lv_roller_get_selected(clock_ctxt.day) + 1;

	// Adjust max day based on year and month.
	u32 max_mon_day = month_days[month - 1];
	u32 max_feb_day = !(year % 4) ? 29 : 28;
	if (month == 2 && day > max_feb_day)
		lv_roller_set_selected(clock_ctxt.day, max_feb_day - 1, false);
	else if (day > max_mon_day)
		lv_roller_set_selected(clock_ctxt.day, max_mon_day - 1, false);

	return LV_RES_OK;
}

static lv_res_t _create_mbox_clock_edit(lv_obj_t *btn)
{
	static lv_style_t mbox_style;
	lv_theme_t *th = lv_theme_get_current();
	lv_style_copy(&mbox_style, th->mbox.bg);
	mbox_style.body.padding.inner = LV_DPI / 10;

	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\251", "\222Done", "\222Cancel", "\251", "" };
    static const char *first_btn_map[] = { "\251", "\222Save & reload", "\222Cancel", "\251", "" };
	lv_obj_t *mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_style(mbox, LV_MBOX_STYLE_BG, &mbox_style);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	lv_mbox_set_text(mbox, "Enter #C7EA46 Date# and #C7EA46 Time# for Nyx\n"
						   "Used in all file operations and menu.\n"
						   "This doesn't alter the actual HW clock!");

	lv_obj_t *padding = lv_cont_create(mbox, NULL);
	lv_cont_set_fit(padding, true, false);
	lv_cont_set_style(padding, &lv_style_transp);
	lv_obj_set_height(padding, LV_DPI / 10);

	// Get current time.
	rtc_time_t time;
	max77620_rtc_get_time_adjusted(&time);

	// Normalize year if out of range.
	if (time.year < CLOCK_MIN_YEAR)
		time.year = CLOCK_MIN_YEAR;
	else if (time.year > CLOCK_MAX_YEAR)
		time.year = CLOCK_MAX_YEAR;

	time.year -= CLOCK_MIN_YEAR;

	lv_obj_t *h1 = lv_cont_create(mbox, NULL);
	lv_cont_set_fit(h1, true, true);

	// Create year roller.
	lv_obj_t *roller_year = lv_roller_create(h1, NULL);
	lv_roller_set_options(roller_year, CLOCK_YEARLIST);
	lv_roller_set_selected(roller_year, time.year, false);
	lv_roller_set_visible_row_count(roller_year, 3);
	lv_roller_set_action(roller_year, _action_date_validation);
	clock_ctxt.year = roller_year;

	// Create month roller.
	lv_obj_t *roller_month = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_month,
		"January\n"
		"February\n"
		"March\n"
		"April\n"
		"May\n"
		"June\n"
		"July\n"
		"August\n"
		"September\n"
		"October\n"
		"November\n"
		"December");
	lv_roller_set_selected(roller_month, time.month - 1, false);
	lv_obj_align(roller_month, roller_year, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_roller_set_action(roller_month, _action_date_validation);
	clock_ctxt.month = roller_month;

	// Create day roller.
	static char days[256];
	days[0] = 0;
	for (u32 i = 1; i < 32; i++)
		s_printf(days + strlen(days), " %d \n", i);
	days[strlen(days) - 1] = 0;
	lv_obj_t *roller_day = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_day, days);
	lv_roller_set_selected(roller_day, time.day - 1, false);
	lv_roller_set_action(roller_day, _action_date_validation);
	lv_obj_align(roller_day, roller_month, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	clock_ctxt.day = roller_day;

	// Create hours roller.
	static char hours[256];
	hours[0] = 0;
	for (u32 i = 0; i < 24; i++)
		s_printf(hours + strlen(hours), " %d \n", i);
	hours[strlen(hours) - 1] = 0;
	lv_obj_t *roller_hour = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_hour, hours);
	lv_roller_set_selected(roller_hour, time.hour, false);
	lv_obj_align(roller_hour, roller_day, LV_ALIGN_OUT_RIGHT_MID, LV_DPI / 2, 0);
	clock_ctxt.hour = roller_hour;

	// Create minutes roller.
	static char minutes[512];
	minutes[0] = 0;
	for (u32 i = 0; i < 60; i++)
		s_printf(minutes + strlen(minutes), " %02d \n", i);
	minutes[strlen(minutes) - 1] = 0;
	lv_obj_t *roller_minute = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_minute, minutes);
	lv_roller_set_selected(roller_minute, time.min, false);
	lv_obj_align(roller_minute, roller_hour, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	clock_ctxt.min = roller_minute;

	// Add DST option.
	lv_obj_t *btn_dst = lv_btn_create(mbox, NULL);
	nyx_create_onoff_button(th, h1, btn_dst, SYMBOL_BRIGHTNESS" Auto Daylight Saving Time", _action_auto_dst_toggle, true);
	if (n_cfg.timedst)
		lv_btn_set_state(btn_dst, LV_BTN_STATE_TGL_REL);
	nyx_generic_onoff_toggle(btn_dst);

    clock_language=n_cfg.language;
    if(!btn){
        lv_obj_t *label=lv_label_create(mbox,NULL);lv_label_set_text(label,"Langue");
        lv_label_set_style(label,&hint_small_style);
        nyx_choices_create(mbox,"English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês",clock_language,4,LV_HOR_RES/9*6-48,46,_clock_language_select,0,true);
    }

	// If btn is empty, save options also because it was launched from boot.
	lv_mbox_add_btns(mbox, btn?mbox_btn_map:first_btn_map, btn ? _action_clock_edit : _action_clock_edit_save);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	return LV_RES_OK;
}

void first_time_clock_edit(void *param)
{
	(void)param;
	_create_mbox_clock_edit(NULL);
}

typedef struct _emmc_backup_buttons_t
{
	lv_obj_t *emmc_boot;
	lv_obj_t *emmc_raw_gpp;
	lv_obj_t *emmc_sys;
	lv_obj_t *emmc_usr;
	bool raw_emummc;
	bool restore;
} emmc_backup_buttons_t;

static emmc_backup_buttons_t emmc_btn_ctxt;


typedef int emmcPartType_t;
#define PART_BOOT 0
static emmc_tool_gui_t preview_transfer;
static void dump_emmc_selected(emmcPartType_t type,emmc_tool_gui_t *context){
 (void)type;preview_transfer=*context;char text[2048];
 s_printf(text,"01: BOOT1\n%s: 0x00000000 - 0x00001FFF\n\n",nyx_tr("Range"));lv_label_set_text(context->label_info,text);
 s_printf(text,nyx_tr("#96FF00 SD Card free space:# %d MiB\n#96FF00 Total backup size:# %d MiB\n\n"),803971,4);lv_label_ins_text(context->label_info,LV_LABEL_POS_LAST,text);
 s_printf(text,nyx_tr("#96FF00 Filepath:#\n%s\n#96FF00 Filename:# #FF8000 %s#"),"backup/1034b3fe/","BOOT1");lv_label_ins_text(context->label_info,LV_LABEL_POS_LAST,text);
 lv_bar_set_value(context->bar,100);lv_label_set_text(context->label_pct,"100%");lv_label_set_text(context->label_finish,"Finished and verified!");
 lv_label_set_text(context->label_log,"BOOT0: OK\nBOOT1: OK\nSHA-256: 0123456789ABCDEF\nVérification · Проверка");
}
static void restore_emmc_selected(emmcPartType_t type,emmc_tool_gui_t *context){dump_emmc_selected(type,context);}
static void _create_window_backup_restore(emmcPartType_t type, const char* win_label)
{
	emmc_tool_gui_t emmc_tool_gui_ctxt;

	emmc_tool_gui_ctxt.raw_emummc = emmc_btn_ctxt.raw_emummc;

	char win_label_full[80];

	s_printf(win_label_full,nyx_tr(emmc_btn_ctxt.restore?"Restore %s":"Backup %s"),nyx_nova_caption(win_label));

	lv_obj_t *win = nyx_create_standard_window(win_label_full, NULL);

	//Disable buttons.
	nyx_window_toggle_buttons(win, true);

    nyx_nova_transfer(win,&emmc_tool_gui_ctxt);

	if (!emmc_btn_ctxt.restore)
		dump_emmc_selected(type, &emmc_tool_gui_ctxt);
	else
		restore_emmc_selected(type, &emmc_tool_gui_ctxt);

	nyx_window_toggle_buttons(win, false);

	// Refresh AutoRCM button.
	if (emmc_btn_ctxt.restore && (type == PART_BOOT) && !emmc_btn_ctxt.raw_emummc)
	{
		if (get_set_autorcm_status(false))
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_TGL_REL);
		else
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_REL);
		nyx_generic_onoff_toggle(autorcm_btn);

		if (h_cfg.rcm_patched)
		{
			lv_obj_set_click(autorcm_btn, false);
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_INA);
		}
	}
}

ACTION(_emmc_backup_buttons_decider)
static lv_res_t _emmc_backup_buttons_raw_toggle(lv_obj_t *btn)
{
	nyx_generic_onoff_toggle(btn);

	lv_obj_set_click(emmc_btn_ctxt.emmc_boot, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_boot, LV_BTN_STATE_REL);
	lv_obj_set_click(emmc_btn_ctxt.emmc_raw_gpp, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_raw_gpp, LV_BTN_STATE_REL);

	lv_obj_set_click(emmc_btn_ctxt.emmc_sys, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_sys, LV_BTN_STATE_REL);

	// Backup/Restore from and to eMMC.
	if (!(lv_btn_get_state(btn) & LV_BTN_STATE_TGL_REL))
	{
		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_UPLOAD"  eMMC BOOT0 & BOOT1");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_DOWNLOAD"  eMMC BOOT0 & BOOT1");
		lv_obj_realign(emmc_btn_ctxt.emmc_boot);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_UPLOAD"  eMMC RAW GPP");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_DOWNLOAD"  eMMC RAW GPP");
		lv_obj_realign(emmc_btn_ctxt.emmc_raw_gpp);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_sys, NULL), SYMBOL_MODULES"  eMMC SYS");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_sys, NULL), SYMBOL_MODULES"  eMMC ALL");
		lv_obj_realign(emmc_btn_ctxt.emmc_sys);

		if (!emmc_btn_ctxt.restore)
		{
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_usr, NULL), SYMBOL_MODULES_ALT"  eMMC USER");
			lv_obj_realign(emmc_btn_ctxt.emmc_usr);

			lv_obj_set_click(emmc_btn_ctxt.emmc_usr, true);
			lv_btn_set_state(emmc_btn_ctxt.emmc_usr, LV_BTN_STATE_REL);
		}

		emmc_btn_ctxt.raw_emummc = false;
	}
	else // Backup/Restore from and to emuMMC.
	{
		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_UPLOAD"  SD emuMMC BOOT0 & BOOT1");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_DOWNLOAD"  SD emuMMC BOOT0 & BOOT1");
		lv_obj_realign(emmc_btn_ctxt.emmc_boot);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_UPLOAD"  SD emuMMC RAW GPP");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_DOWNLOAD"  SD emuMMC RAW GPP");
		lv_obj_realign(emmc_btn_ctxt.emmc_raw_gpp);

		lv_obj_set_click(emmc_btn_ctxt.emmc_sys, false);
		lv_btn_set_state(emmc_btn_ctxt.emmc_sys, LV_BTN_STATE_INA);

		if (!emmc_btn_ctxt.restore)
		{
			lv_obj_set_click(emmc_btn_ctxt.emmc_usr, false);
			lv_btn_set_state(emmc_btn_ctxt.emmc_usr, LV_BTN_STATE_INA);
		}

		emmc_btn_ctxt.raw_emummc = true;
	}
	/* Keep the new card titles and disabled state consistent when the target
	 * changes. The single-label button contract is preserved for the decider. */
	lv_obj_t *cards[]={emmc_btn_ctxt.emmc_boot,emmc_btn_ctxt.emmc_raw_gpp,emmc_btn_ctxt.emmc_sys,emmc_btn_ctxt.emmc_usr};
	for(u32 i=0;i<4;++i)if(cards[i]){
		lv_obj_t *label=lv_obj_get_child(cards[i],NULL);
		const char *caption=nyx_nova_caption(lv_label_get_text(label));
		char title[160];u32 length=strlen(caption);if(length>=sizeof(title))length=sizeof(title)-1;
		memcpy(title,caption,length);title[length]=0;lv_label_set_text(label,title);
		lv_label_set_style(label,(lv_style_t *)nyx_nova_text_style(lv_obj_get_click(cards[i])?NOVA_BODY:NOVA_MUTED));
		nyx_nova_fit_text(label,58);
	}

	return LV_RES_OK;
}
lv_res_t preview_backup_restore(lv_obj_t *btn)
{
    emmc_btn_ctxt.restore=lv_obj_get_free_num(btn)==1;
    lv_obj_t *win=nyx_create_standard_window(emmc_btn_ctxt.restore?SYMBOL_SD" Restore":SYMBOL_SD" Backup",NULL);
    nyx_nova_fixed_page(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page);
    const char *descriptions[]={
        "Partitions BOOT0/BOOT1, à conserver avec RAW GPP.",
        "Stockage complet : CAL0, SYSTEM, USER et fichiers de démarrage.",
        "Partitions système de RAW GPP.",
        "Partition USER. Cette copie seule est incomplète."
    };
    int height=(lv_obj_get_height(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page)-132)/2;
    emmc_btn_ctxt.emmc_boot=nyx_nova_action(win,"eMMC BOOT0 & BOOT1",descriptions[0],"hard-drive-download",_emmc_backup_buttons_decider,0,34,18,594,height);
    emmc_btn_ctxt.emmc_raw_gpp=nyx_nova_action(win,"eMMC RAW GPP",descriptions[1],"hard-drive",_emmc_backup_buttons_decider,0,652,18,594,height);
    emmc_btn_ctxt.emmc_sys=nyx_nova_action(win,emmc_btn_ctxt.restore?"eMMC ALL":"eMMC SYS",descriptions[2],"boxes",_emmc_backup_buttons_decider,0,34,34+height,594,height);
    emmc_btn_ctxt.emmc_usr=emmc_btn_ctxt.restore?NULL:nyx_nova_action(win,"eMMC USER",descriptions[3],"folder-open",_emmc_backup_buttons_decider,0,652,34+height,594,height);
    lv_obj_t *panel=nyx_nova_panel(win,34,50+height*2,1212,64);
    nyx_nova_switch_row(panel,"SD emuMMC Raw Partition","Utiliser emuMMC sur la carte SD à la place de l’eMMC physique.","database",_emmc_backup_buttons_raw_toggle,false,18,0,1176,64);
    emmc_btn_ctxt.raw_emummc=false;
    return LV_RES_OK;
}

static int usb_msc_emmc_read_only;
ACTION(action_ums_sd) ACTION(_action_ums_emmc_gpp) ACTION(_action_ums_emmc_boot0) ACTION(_action_ums_emmc_boot1) ACTION(_action_ums_emuemmc_gpp) ACTION(_action_ums_emuemmc_boot0) ACTION(_action_ums_emuemmc_boot1) ACTION(_action_hid_jc)
static lv_res_t _emmc_read_only_toggle(lv_obj_t *btn)
{
	nyx_generic_onoff_toggle(btn);

	usb_msc_emmc_read_only = lv_btn_get_state(btn) & LV_BTN_STATE_TGL_REL ? 1 : 0;

	return LV_RES_OK;
}
static lv_res_t preview_usb_tools(lv_obj_t *parent)
{
    (void)parent;
    lv_obj_t *win=nyx_create_standard_window(SYMBOL_USB" USB Tools",NULL);
    nyx_nova_fixed_page(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page);
    lv_obj_t *panel=nyx_nova_panel(win,34,22,1212,72);
    lv_obj_t *readonly=nyx_nova_switch_row(panel,"Read-Only","eMMC / emuMMC : accès protégé en lecture seule par défaut.","shield-check",_emmc_read_only_toggle,!n_cfg.ums_emmc_rw,20,0,1172,72);
    /* The surrounding panel owns the frame; the row only owns the switch. */
    for(u32 state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;++state)
        lv_btn_set_style(readonly,state,&lv_style_transp_tight);
    _emmc_read_only_toggle(readonly);
    static const char *titles[]={"SD Card","eMMC RAW GPP","eMMC BOOT0","eMMC BOOT1","emuMMC RAW GPP","emuMMC BOOT0","emuMMC BOOT1","Gamepad"};
    static const char *icons[]={"card-sim","hard-drive","memory-stick","memory-stick","database","memory-stick","memory-stick","gamepad-2"};
    lv_action_t actions[]={action_ums_sd,_action_ums_emmc_gpp,_action_ums_emmc_boot0,_action_ums_emmc_boot1,_action_ums_emuemmc_gpp,_action_ums_emuemmc_boot0,_action_ums_emuemmc_boot1,_action_hid_jc};
    int height=(lv_obj_get_height(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page)-128)/2;
    for(u32 i=0;i<8;++i)nyx_nova_action(win,titles[i],i==0?"Connexion au PC ou téléphone : lecture et écriture.":i==7?"Connectez les deux Joy-Con pour utiliser la console comme manette USB.":"Monter cette partition par USB.",icons[i],actions[i],0,34+(i%4)*307,110+(i/4)*(height+16),291,height-4);
    return LV_RES_OK;
}

#define AU_ALIGN_SECTORS 0x8000
#define ALIGN_DOWN(x,a) ((x)&~((a)-1))
#define HOS_USER_SECTOR 0x53C000
#define EMU_SLIDER_MIN     0
#define EMU_SLIDER_MAX     44 // 24 GB. Always an even number.
#define EMU_SLIDER_1X_MAX  (EMU_SLIDER_MAX / 2)
#define EMU_SLIDER_1X_FULL EMU_SLIDER_1X_MAX
#define EMU_SLIDER_2X_MIN  (EMU_SLIDER_1X_MAX + 1)
#define EMU_SLIDER_2X_FULL EMU_SLIDER_MAX
#define EMU_SLIDER_OFFSET  3 // Min 4GB.
#define EMU_RSVD_MB        (4 + 4 + 16 + 8) // BOOT0 + BOOT1 + 16MB offset + 8MB alignment.
#define EMU_32GB_FULL      29856 // Actual: 29820 MB.
#define EMU_64GB_FULL      59680 // Actual: 59640 MB.
#define AND_SYS_SIZE_MB     6144 // 6 GB. Fits both Legacy (4912MB) and Dynamic (6144MB) partition schemes.

typedef struct{u32 sec_cnt;}sdmmc_storage_t;
typedef struct{u32 unused;}mbr_t;
static sdmmc_storage_t sd_storage,emmc_storage;
typedef struct _partition_ctxt_t
{
	bool emmc;
	sdmmc_storage_t *storage;

	u32 total_sct;
	u32 alignment;
	u32 emmc_size_mb;
	int backup_possible;

	s32 hos_min_size;

	s32 hos_size;
	u32 emu_size;
	u32 l4t_size;
	u32 and_size;

	bool emu_double;
	bool emmc_is_64gb;

	bool and_dynamic;

	mbr_t mbr_old;

	lv_obj_t *bar_hos;
	lv_obj_t *bar_emu;
	lv_obj_t *bar_l4t;
	lv_obj_t *bar_and;

	lv_obj_t *sep_emu;
	lv_obj_t *sep_l4t;
	lv_obj_t *sep_and;

	lv_obj_t *slider_bar_hos;

	lv_obj_t *cont_lbl;

	lv_obj_t *lbl_hos;
	lv_obj_t *lbl_emu;
	lv_obj_t *lbl_l4t;
	lv_obj_t *lbl_and;

	lv_obj_t *partition_button;
} partition_ctxt_t;


static partition_ctxt_t part_info;static lv_obj_t *btn_flash_l4t,*btn_flash_android;
static u32 fixture_model;
enum{FUSE_NX_HW_TYPE_IOWA=1,FUSE_NX_HW_TYPE_HOAG=2,FUSE_NX_HW_TYPE_AULA=3,EMMC_GPP=0};
static u32 fuse_read_hw_type(void){return fixture_model;}
static int emmc_initialize(bool b){(void)b;return 0;}
static void emmc_set_partition(u32 p){(void)p;}
static void emmc_end(void){}
ACTION(_action_part_manager_ums_sd) ACTION(_action_check_flash_linux) ACTION(_action_flash_android) ACTION(_action_fix_mbr_gpt) ACTION(_create_mbox_partitioning_next_legacy)
static void nova_partition_refresh(void);
static void _update_partition_bar()
{
	nova_partition_refresh();
}
static lv_res_t _action_slider_emu(lv_obj_t *slider)
{
	u32 size;
	char lbl_text[64];
	bool prev_emu_double = part_info.emu_double;
	int slide_val = lv_slider_get_value(slider);
	u32 max_emmc_size = !part_info.emmc_is_64gb ? EMU_32GB_FULL : EMU_64GB_FULL;

	part_info.emu_double = false;

	// Check that eMMC exists.
	if (!part_info.emmc_size_mb)
	{
		lv_slider_set_value(slider, 0);
		return LV_RES_OK;
	}

	// In case of upgraded eMMC, do not allow FULL sizes. Max size is always bigger than official eMMCs.
	if (max_emmc_size < part_info.emmc_size_mb)
	{
		if (slide_val == EMU_SLIDER_1X_FULL)
		{
			if (prev_emu_double)
				slide_val--;
			else
				slide_val++;
			lv_slider_set_value(slider, slide_val);
		}
		else if (slide_val == EMU_SLIDER_2X_FULL)
		{
			slide_val--;
			lv_slider_set_value(slider, slide_val);
		}
	}

	size  = (slide_val > EMU_SLIDER_1X_MAX ? (slide_val - EMU_SLIDER_1X_MAX) : slide_val) + EMU_SLIDER_OFFSET;
	size *= 1024;        // Convert to GB.
	size += EMU_RSVD_MB; // Add reserved size.

	if (slide_val == EMU_SLIDER_MIN)
		size = 0; // Reset if 0.
	else if (slide_val >= EMU_SLIDER_2X_MIN)
	{
		size *= 2;
		part_info.emu_double = true;
	}

	// Handle special cases. 2nd value is for 64GB Aula. Values already include reserved space.
	if (slide_val == EMU_SLIDER_1X_FULL)
		size = max_emmc_size;
	else if (slide_val == EMU_SLIDER_2X_FULL)
		size = 2 * max_emmc_size;

	// Sanitize sizes based on new HOS size.
	s32 hos_size = (part_info.total_sct >> 11) - 16 - size - part_info.l4t_size - part_info.and_size;
	if (hos_size > part_info.hos_min_size)
	{
		part_info.emu_size = size;
		part_info.hos_size = hos_size;

		s_printf(lbl_text, "#96FF00 %4d GiB#", hos_size >> 10);
		lv_label_set_text(part_info.lbl_hos, lbl_text);
		lv_bar_set_value(part_info.slider_bar_hos, hos_size >> 10);

		if (!part_info.emu_double)
		{
			if (slide_val != EMU_SLIDER_1X_FULL)
				s_printf(lbl_text, "#FF3C28 %4d GiB#", size >> 10);
			else
				s_printf(lbl_text, "#FF3C28 %d FULL#", size >> 10);
		}
		else
			s_printf(lbl_text, "#FFDD00 2x##FF3C28 %d GiB#", size >> 11);
		lv_label_set_text(part_info.lbl_emu, lbl_text);
	}
	else
	{
		u32 emu_size = part_info.emu_size;

		if (emu_size == max_emmc_size)
			emu_size = EMU_SLIDER_1X_FULL;
		else if (emu_size == 2 * max_emmc_size)
			emu_size = EMU_SLIDER_2X_FULL;
		else if (emu_size)
		{
			if (prev_emu_double)
				emu_size /= 2;
			emu_size -= EMU_RSVD_MB;
			emu_size /= 1024;
			emu_size -= EMU_SLIDER_OFFSET;

			if (prev_emu_double)
				emu_size += EMU_SLIDER_2X_MIN;
		}

		int new_slider_val = emu_size;
		part_info.emu_double = prev_emu_double ? true : false;

		lv_slider_set_value(slider, new_slider_val);
	}

	_update_partition_bar();

	return LV_RES_OK;
}
static lv_res_t _action_slider_l4t(lv_obj_t *slider)
{
	char lbl_text[64];

	u32 size = (u32)lv_slider_get_value(slider) << 10;
	if (size < 4096)
		size = 0;
	else if (size < 8192)
		size = 8192;

	s32 hos_size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - size - part_info.and_size;

	// Sanitize sizes based on new HOS size.
	if (hos_size > part_info.hos_min_size)
	{
		if (size <= 8192)
			lv_slider_set_value(slider, size >> 10);
	}
	else
	{
		size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - part_info.and_size - 2048;
		hos_size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - part_info.and_size - size;
		if (hos_size < part_info.hos_min_size || size < 8192)
		{
			lv_slider_set_value(slider, part_info.l4t_size >> 10);
			goto out;
		}
		lv_slider_set_value(slider, size >> 10);
	}

	part_info.l4t_size = size;
	part_info.hos_size = hos_size;

	s_printf(lbl_text, "#96FF00 %4d GiB#", hos_size >> 10);
	lv_label_set_text(part_info.lbl_hos, lbl_text);
	lv_bar_set_value(part_info.slider_bar_hos, hos_size >> 10);
	s_printf(lbl_text, "#00DDFF %4d GiB#", size >> 10);
	lv_label_set_text(part_info.lbl_l4t, lbl_text);

	_update_partition_bar();

out:
	return LV_RES_OK;
}
static lv_res_t _action_slider_and(lv_obj_t *slider)
{
	char lbl_text[64];

	u32 user_size = (u32)lv_slider_get_value(slider) << 10;
	if (user_size < 2048)
		user_size = 0;
	else if (user_size < 4096)
		user_size = 4096;

	u32 and_size = user_size ? (user_size + AND_SYS_SIZE_MB) : 0;
	s32 hos_size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - part_info.l4t_size - and_size;

	// Sanitize sizes based on new HOS size.
	if (hos_size > part_info.hos_min_size)
	{
		if (user_size <= 4096)
			lv_slider_set_value(slider, user_size >> 10);
	}
	else
	{
		and_size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - part_info.l4t_size - 2048;
		hos_size = (part_info.total_sct >> 11) - 16 - part_info.emu_size - part_info.l4t_size - and_size;
		if (hos_size < part_info.hos_min_size || and_size < 8192)
		{
			lv_slider_set_value(slider, part_info.and_size >> 10);
			goto out;
		}
		user_size = and_size - AND_SYS_SIZE_MB;
		lv_slider_set_value(slider, user_size >> 10);
	}

	part_info.and_size = and_size;
	part_info.hos_size = hos_size;

	s_printf(lbl_text, "#96FF00 %4d GiB#", hos_size >> 10);
	lv_label_set_text(part_info.lbl_hos, lbl_text);
	lv_bar_set_value(part_info.slider_bar_hos, hos_size >> 10);
	s_printf(lbl_text, "#FF8000 %4d GiB#", user_size >> 10);
	lv_label_set_text(part_info.lbl_and, lbl_text);

	_update_partition_bar();

out:
	return LV_RES_OK;
}
/* Pure capacity arithmetic; never performs storage I/O. GPL-2.0-or-later. */

nova_plan_result nova_partition_validate(nova_partition_plan *plan)
{
    if(!plan)return NOVA_PLAN_INVALID;
    plan->fat_mib=0;
    plan->alignment_sectors=plan->total_sectors%NOVA_DISK_ALIGN_SECTORS;
    u64 total=(plan->total_sectors-plan->alignment_sectors)>>11;
    if(!plan->fat_min_mib||total<=16u)return NOVA_PLAN_CAPACITY;
    if(plan->linux_mib&&plan->linux_mib<NOVA_LINUX_MIN_MIB)return NOVA_PLAN_LINUX_SIZE;
    if(plan->android_mib&&plan->android_mib<NOVA_ANDROID_MIN_MIB)return NOVA_PLAN_ANDROID_SIZE;
    /* Expert sliders can clamp to the remaining aligned capacity. Preserve
     * those valid 16-MiB endpoints as well as whole-GiB presets. */
    if((plan->linux_mib|plan->android_mib)&15u)return NOVA_PLAN_INVALID;
    if(plan->emu_mib){
        u32 copies=plan->emu_double?2u:1u;
        if(plan->emu_mib%copies)return NOVA_PLAN_EMUMMC_SIZE;
        u32 size=plan->emu_mib/copies;
        if(size!=NOVA_EMU_32_MIB&&size!=NOVA_EMU_64_MIB&&
            (size<4096u+NOVA_EMU_RESERVE_MIB||((size-NOVA_EMU_RESERVE_MIB)&1023u)))
            return NOVA_PLAN_EMUMMC_SIZE;
    }else if(plan->emu_double)return NOVA_PLAN_EMUMMC_SIZE;
    u64 used=(u64)plan->emu_mib+plan->linux_mib+plan->android_mib+16u;
    if(used>total||total-used<plan->fat_min_mib)return NOVA_PLAN_CAPACITY;
    plan->fat_mib=(u32)(total-used);
    return NOVA_PLAN_OK;
}

nova_plan_result nova_partition_simplified(nova_partition_plan *plan,
    u32 actual_model,u32 chosen_model,u32 emmc_data_mib,u32 linux_choice,u32 android_choice)
{
    if(!plan)return NOVA_PLAN_INVALID;
    plan->fat_mib=0;
    if(actual_model>3u||chosen_model>3u||linux_choice>2u||android_choice>2u)return NOVA_PLAN_INVALID;
    plan->emu_double=false;
    plan->emu_mib=chosen_model==3u?NOVA_EMU_64_MIB:NOVA_EMU_32_MIB;
    plan->linux_mib=linux_choice?(linux_choice==1u?16384u:32768u):0u;
    plan->android_mib=android_choice?(android_choice==1u?16384u:32768u):0u;
    /* Keep the proposed free capacity visible even when a different model
     * is previewed. Validation for destructive actions remains separate. */
    nova_plan_result capacity=nova_partition_validate(plan);
    bool same_family=actual_model==chosen_model||(actual_model<2u&&chosen_model<2u);
    if(!same_family)return NOVA_PLAN_MODEL;
    if(!emmc_data_mib)return NOVA_PLAN_EMMC_UNAVAILABLE;
    if((u64)emmc_data_mib+NOVA_EMU_RESERVE_MIB>plan->emu_mib)return NOVA_PLAN_EMMC_UPGRADED;
    return capacity;
}

void nova_partition_widths(const nova_partition_plan *plan,u32 width,u32 out[4])
{
    const u32 sizes[]={plan->fat_mib,plan->emu_mib,plan->linux_mib,plan->android_mib};
    u64 sum=(u64)sizes[0]+sizes[1]+sizes[2]+sizes[3],prefix=0;
    u32 previous=0;
    for(u32 i=0;i<4;++i){
        prefix+=sizes[i];u32 end=sum?(u32)(prefix*width/sum):0;
        out[i]=end-previous;previous=end;
    }
}
/* Nova partition manager UI. Included after the existing storage actions.
 * Storage/format/flash implementations remain in gui_tools_partition_manager.c. */
static struct {
    lv_obj_t *win,*page,*content,*models,*enabled[2],*presets[2];
    lv_obj_t *sliders[3],*summary,*message,*capacity,*segments[4];
    lv_obj_t *mode_choices;
    lv_style_t segment_style[4],slider_bg[3],slider_ind[3],slider_knob[3];
    u32 mode,model,actual_model,option[2],preset[2];
    bool ready;
    nova_plan_result result;
} partition_nova;

static const char *nova_partition_error(nova_plan_result result)
{
    switch(result){
    case NOVA_PLAN_OK:return "Disposition valide. Les réserves techniques sont incluses.";
    case NOVA_PLAN_CAPACITY:return "Capacité insuffisante : conserver l'espace minimum pour FAT32 / USER.";
    case NOVA_PLAN_MODEL:return "Le modèle sélectionné ne correspond pas à cette console.";
    case NOVA_PLAN_EMMC_UNAVAILABLE:return "eMMC indisponible : sa taille ne peut pas être vérifiée.";
    case NOVA_PLAN_EMMC_UPGRADED:return "eMMC modifiée : utiliser le mode expert pour cette console.";
    case NOVA_PLAN_LINUX_SIZE:return "La partition Linux est trop petite.";
    case NOVA_PLAN_ANDROID_SIZE:return "Android doit inclure ses partitions système et ses données.";
    default:return "Disposition incompatible. Vérifier les tailles avant de continuer.";
    }
}

static nova_plan_result nova_partition_current(nova_partition_plan *plan)
{
    memset(plan,0,sizeof(*plan));
    plan->total_sectors=part_info.total_sct;
    plan->fat_min_mib=part_info.hos_min_size;
    if(!part_info.emmc&&!partition_nova.mode)
        return nova_partition_simplified(plan,partition_nova.actual_model,partition_nova.model,
            part_info.emmc_size_mb,partition_nova.option[0],partition_nova.option[1]);
    plan->emu_mib=part_info.emu_size;plan->emu_double=part_info.emu_double;
    plan->linux_mib=part_info.l4t_size;plan->android_mib=part_info.and_size;
    if(plan->emu_mib&&!part_info.emmc_size_mb)return NOVA_PLAN_EMMC_UNAVAILABLE;
    return nova_partition_validate(plan);
}

static bool nova_partition_preflight(void)
{
    nova_partition_plan plan;
    if(!partition_nova.ready||nova_partition_current(&plan)!=NOVA_PLAN_OK){
        nyx_show_help("Disposition incompatible. Vérifier les tailles avant de continuer.");return false;
    }
    u32 sectors=0;
    if(part_info.emmc){
        if(!emmc_initialize(false)){
            emmc_set_partition(EMMC_GPP);
            if(emmc_storage.sec_cnt>HOS_USER_SECTOR)sectors=emmc_storage.sec_cnt-HOS_USER_SECTOR;
            emmc_end();
        }
    }else if(!sd_mount()){sectors=sd_storage.sec_cnt;sd_unmount();}
    if(!sectors||ALIGN_DOWN(sectors,AU_ALIGN_SECTORS)!=part_info.total_sct){
        nyx_show_help("Support indisponible ou capacité modifiée. Rouvrir le gestionnaire de partitions.");return false;
    }
    part_info.hos_size=plan.fat_mib;
    return true;
}

static void nova_partition_refresh(void)
{
    if(!partition_nova.ready)return;
    nova_partition_plan plan;partition_nova.result=nova_partition_current(&plan);
    bool valid=partition_nova.result==NOVA_PLAN_OK;
    part_info.emu_size=plan.emu_mib;part_info.l4t_size=plan.linux_mib;
    part_info.and_size=plan.android_mib;part_info.emu_double=plan.emu_double;
    if(valid)part_info.hos_size=plan.fat_mib;
    char text[320];
    s_printf(text,"%s: %d MiB\nemuMMC%s: %d MiB\nLinux: %d MiB\nAndroid: %d MiB",part_info.emmc?"USER":"FAT32",plan.fat_mib,plan.emu_double?" (2x)":"",plan.emu_mib,plan.linux_mib,plan.android_mib);
    lv_label_set_text(partition_nova.summary,text);
    lv_label_set_text(partition_nova.message,nova_partition_error(partition_nova.result));
    nyx_nova_fit_text(partition_nova.message,70);
    lv_obj_set_click(part_info.partition_button,valid);
    lv_btn_set_state(part_info.partition_button,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    s_printf(text,nyx_tr("Capacité : %d MiB (%d,%02d Go) · réserves : %d MiB"),(part_info.total_sct>>11),(part_info.total_sct>>11)/1024,((part_info.total_sct>>11)%1024)*100/1024,16);
    lv_label_set_text(partition_nova.capacity,text);
    u32 widths[4];nova_partition_widths(&plan,1164,widths);int x=24;
    for(u32 i=0;i<4;++i){
        lv_obj_set_hidden(partition_nova.segments[i],!plan.fat_mib||!widths[i]);
        lv_obj_set_pos(partition_nova.segments[i],x,42);
        lv_obj_set_width(partition_nova.segments[i],widths[i]?widths[i]:1);x+=widths[i];
    }
    lv_obj_t *labels[]={part_info.lbl_hos,part_info.lbl_emu,part_info.lbl_l4t,part_info.lbl_and};
    const u32 values[]={plan.fat_mib,plan.emu_mib,plan.linux_mib,plan.android_mib};
    for(u32 i=0;i<4;++i)if(labels[i]){s_printf(text,nyx_tr("%d,%02d Go"),values[i]/1024,(values[i]%1024)*100/1024);lv_label_set_text(labels[i],text);}
    if(!partition_nova.mode&&!part_info.emmc){
        for(u32 i=0;i<2;++i)if(partition_nova.presets[i])
            for(lv_obj_t *button=lv_obj_get_child(partition_nova.presets[i],NULL);button;button=lv_obj_get_child(partition_nova.presets[i],button)){
                bool on=partition_nova.option[i]!=0;lv_obj_set_click(button,on);
                lv_btn_set_state(button,on?(lv_obj_get_free_num(button)==partition_nova.preset[i]?LV_BTN_STATE_TGL_REL:LV_BTN_STATE_REL):LV_BTN_STATE_INA);
            }
    }
}

static lv_res_t nova_partition_simple_choice(lv_obj_t *button)
{
    u32 group=lv_obj_get_free_num(lv_obj_get_parent(button)),value=lv_obj_get_free_num(button);
    if(group==70)partition_nova.model=value==0?(partition_nova.actual_model<2?partition_nova.actual_model:0):value+1;
    else if(group==71||group==72){u32 index=group-71;partition_nova.option[index]=value?partition_nova.preset[index]+1:0;}
    else if(group==73||group==74){u32 index=group-73;partition_nova.preset[index]=value;partition_nova.option[index]=value+1;}
    nova_partition_refresh();return LV_RES_OK;
}

static lv_res_t nova_partition_expert_slider(lv_obj_t *slider)
{
    u32 index=lv_obj_get_free_num(slider);
    if(index==0)return _action_slider_emu(slider);
    if(index==1)return _action_slider_l4t(slider);
    /* The original callback takes Android USER size; this UI shows the
     * total allocation, including the existing 6-GiB system reservation. */
    int total=lv_slider_get_value(slider);
    lv_slider_set_value(slider,total>=10?total-6:0);
    _action_slider_and(slider);
    lv_slider_set_value(slider,part_info.and_size>>10);
    return LV_RES_OK;
}

static u32 nova_partition_emu_position(void)
{
    u32 full=part_info.emmc_is_64gb?EMU_64GB_FULL:EMU_32GB_FULL;
    if(!part_info.emu_size)return 0;
    if(part_info.emu_size==full)return EMU_SLIDER_1X_FULL;
    if(part_info.emu_size==full*2)return EMU_SLIDER_2X_FULL;
    u32 size=part_info.emu_double?part_info.emu_size/2:part_info.emu_size;
    return ((size-EMU_RSVD_MB)>>10)-EMU_SLIDER_OFFSET+(part_info.emu_double?EMU_SLIDER_1X_MAX:0);
}

static void nova_partition_build_controls(void)
{
    lv_obj_clean(partition_nova.content);
    part_info.lbl_hos=part_info.lbl_emu=part_info.lbl_l4t=part_info.lbl_and=NULL;
    memset(partition_nova.sliders,0,sizeof(partition_nova.sliders));
    memset(partition_nova.presets,0,sizeof(partition_nova.presets));
    lv_obj_t *panel=partition_nova.content;
    if(!partition_nova.mode&&!part_info.emmc){
        nyx_nova_text(panel,"Modèle de console",24,12,696,NOVA_SMALL);
        partition_nova.models=nyx_choices_create(panel,"Switch V1 V2\nSwitch Lite\nSwitch OLED",partition_nova.model<2?0:partition_nova.model-1,3,696,38,nova_partition_simple_choice,70,true);
        lv_obj_set_pos(partition_nova.models,24,40);
        const char *titles[]={"Installer Linux ?","Installer Android ?"};
        for(u32 i=0;i<2;++i){
            int y=100+i*72;
            lv_obj_t *title=nyx_nova_text(panel,titles[i],24,y+7,205,NOVA_BODY);nyx_nova_fit_text(title,38);
            partition_nova.enabled[i]=nyx_choices_create(panel,"Non\nOui",partition_nova.option[i]?1:0,2,180,40,nova_partition_simple_choice,71+i,false);
            lv_obj_set_pos(partition_nova.enabled[i],240,y);
            partition_nova.presets[i]=nyx_choices_create(panel,"16 Go\n32 Go",partition_nova.preset[i],2,282,58,nova_partition_simple_choice,73+i,false);
            lv_obj_set_pos(partition_nova.presets[i],438,y);
            for(lv_obj_t *choice=lv_obj_get_child(partition_nova.presets[i],NULL);choice;choice=lv_obj_get_child(partition_nova.presets[i],choice)){
                lv_obj_t *size=lv_obj_get_child(choice,NULL);lv_obj_set_y(size,4);nyx_nova_fit_text(size,24);
                lv_obj_t *hint=nyx_nova_text(choice,lv_obj_get_free_num(choice)?"Recommandé":"Minimum",4,32,lv_obj_get_width(choice)-8,NOVA_KICKER);
                static lv_style_t caption;lv_style_copy(&caption,nyx_nova_text_style(NOVA_KICKER));caption.text.letter_space=0;lv_label_set_style(hint,&caption);
                lv_label_set_align(hint,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(hint,20);
            }
        }
    }else{
        const char *titles[]={"emuMMC","Linux","Android (total)"};
        const u32 values[]={nova_partition_emu_position(),part_info.l4t_size>>10,part_info.and_size>>10};
        const u32 maximum[]={EMU_SLIDER_MAX,(part_info.total_sct>>21)>2?(part_info.total_sct>>21)-2:0,(part_info.total_sct>>21)>2?(part_info.total_sct>>21)-2:0};
        for(u32 i=0;i<3;++i){
            int y=22+i*72;
            if(i==0&&part_info.emmc){nyx_nova_text(panel,"eMMC USER",24,y,185,NOVA_BODY);part_info.lbl_hos=nyx_nova_text(panel,"",572,y,148,NOVA_SMALL);continue;}
            lv_obj_t *title=nyx_nova_text(panel,titles[i],24,y,185,NOVA_BODY);nyx_nova_fit_text(title,38);
            lv_obj_t *slider=lv_slider_create(panel,NULL);partition_nova.sliders[i]=slider;
            lv_obj_set_pos(slider,220,y+3);lv_obj_set_size(slider,326,28);
            lv_slider_set_range(slider,0,maximum[i]>32760?32760:maximum[i]);lv_slider_set_value(slider,values[i]);
            lv_obj_set_free_num(slider,i);lv_slider_set_action(slider,nova_partition_expert_slider);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_BG,&partition_nova.slider_bg[i]);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_INDIC,&partition_nova.slider_ind[i]);
            lv_slider_set_style(slider,LV_SLIDER_STYLE_KNOB,&partition_nova.slider_knob[i]);
            lv_obj_t *value=nyx_nova_text(panel,"",572,y,148,NOVA_SMALL);nyx_nova_fit_text(value,32);
            if(i==0)part_info.lbl_emu=value;else if(i==1)part_info.lbl_l4t=value;else part_info.lbl_and=value;
        }
    }
    if(!part_info.lbl_hos){part_info.lbl_hos=lv_label_create(panel,NULL);lv_obj_set_hidden(part_info.lbl_hos,true);}
    nova_partition_refresh();
}

static lv_res_t nova_partition_mode(lv_obj_t *button)
{
    partition_nova.mode=lv_obj_get_free_num(button);
    if(partition_nova.mode&&partition_nova.result!=NOVA_PLAN_OK){
        part_info.emu_size=part_info.l4t_size=part_info.and_size=0;part_info.emu_double=false;
    }
    nova_partition_build_controls();return LV_RES_OK;
}

static lv_res_t nova_partition_notes(lv_obj_t *button)
{
    (void)button;
    nyx_show_help(part_info.emmc?
        "Le mode expert conserve le partitionnement manuel. La partition USER sera formatée après confirmation.\nLes fonctions d'installation Linux et Android restent disponibles.":
        "Le mode simplifié utilise une emuMMC complète : 29 Go pour V1/V2/Lite, 58 Go pour OLED (valeurs arrondies).\nLinux et Android sont indépendants : Minimal = 16 Go, Recommandé = 32 Go par système. Android inclut ses partitions système.\nAu moins 2 Go restent pour FAT32. Les alignements et réserves techniques sont comptés.\nL'emuMMC ne sera créée qu'avec l'outil dédié après le partitionnement. Réserver Linux/Android n'installe pas leurs fichiers.\nLa sauvegarde automatique existante est limitée à 1,2 Go ; au-delà, une copie manuelle sera demandée. Une emuMMC redimensionnée reformate USER.");
    return LV_RES_OK;
}

static lv_res_t nova_partition_review_action(lv_obj_t *buttons,const char *text)
{
    u32 selected=lv_btnm_get_pressed(buttons);nyx_mbox_action(buttons,text);
    if(!selected)_create_mbox_partitioning_next_legacy(NULL);
    return LV_RES_INV;
}

static lv_res_t nova_partition_review(lv_obj_t *button)
{
    (void)button;nova_partition_plan plan;nova_plan_result result=nova_partition_current(&plan);
    if(result!=NOVA_PLAN_OK){nyx_show_help(nova_partition_error(result));return LV_RES_OK;}
    part_info.hos_size=plan.fat_mib;
    char message[768];s_printf(message,nyx_tr("Vérifier la disposition\n\nFAT32 / USER : %d MiB\nemuMMC : %d MiB\nLinux : %d MiB\nAndroid : %d MiB\nRéserves : 16 MiB + alignement\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante."),plan.fat_mib,plan.emu_mib,plan.linux_mib,plan.android_mib);
    lv_obj_t *background=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_style(background,&mbox_darken);lv_obj_set_size(background,LV_HOR_RES,LV_VER_RES);
    lv_obj_t *box=lv_mbox_create(background,NULL);lv_obj_set_width(box,1000);lv_mbox_set_text(box,message);
    static const char *buttons[]={"\222Continue","\222Cancel",""};lv_mbox_add_btns(box,buttons,nova_partition_review_action);
    lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);return LV_RES_OK;
}

static void nova_partition_create_ui(lv_obj_t *win)
{
    memset(&partition_nova,0,sizeof(partition_nova));partition_nova.win=win;
    partition_nova.page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(partition_nova.page);
    switch(fuse_read_hw_type()){
    case FUSE_NX_HW_TYPE_IOWA:partition_nova.actual_model=1;break;
    case FUSE_NX_HW_TYPE_HOAG:partition_nova.actual_model=2;break;
    case FUSE_NX_HW_TYPE_AULA:partition_nova.actual_model=3;break;
    default:partition_nova.actual_model=0;break;
    }
    partition_nova.model=partition_nova.actual_model;partition_nova.mode=part_info.emmc?1:0;
    int height=lv_obj_get_height(partition_nova.page);
    if(!part_info.emmc){
        partition_nova.mode_choices=nyx_choices_create(partition_nova.page,"Mode simplifié\nMode expert",0,2,1212,42,nova_partition_mode,75,false);
        lv_obj_set_pos(partition_nova.mode_choices,34,14);
    }else nyx_nova_text(partition_nova.page,"Mode expert — eMMC interne",34,22,1212,NOVA_TITLE);
    lv_obj_t *overview=nyx_nova_panel(partition_nova.page,34,72,1212,84);
    partition_nova.capacity=nyx_nova_text(overview,"",24,12,1164,NOVA_SMALL);
    const u32 colors[]={0x55C9F1,0xA98BFA,0x8AE6B6,0xF4B85F};
    for(u32 i=0;i<4;++i){
        lv_style_copy(&partition_nova.segment_style[i],&lv_style_plain);
        partition_nova.segment_style[i].body.main_color=partition_nova.segment_style[i].body.grad_color=LV_COLOR_HEX(colors[i]);
        partition_nova.segment_style[i].body.radius=0;partition_nova.segment_style[i].body.border.width=0;
        lv_obj_t *segment=lv_cont_create(overview,NULL);lv_cont_set_fit(segment,false,false);lv_cont_set_style(segment,&partition_nova.segment_style[i]);
        lv_obj_set_size(segment,1,12);lv_obj_set_click(segment,false);partition_nova.segments[i]=segment;
    }
    nyx_nova_text(overview,"FAT32  ·  emuMMC  ·  Linux  ·  Android",24,61,1164,NOVA_KICKER);
    for(u32 i=0;i<3;++i){
        lv_style_copy(&partition_nova.slider_bg[i],lv_theme_get_current()->slider.bg);
        lv_style_copy(&partition_nova.slider_ind[i],lv_theme_get_current()->slider.indic);
        lv_style_copy(&partition_nova.slider_knob[i],lv_theme_get_current()->slider.knob);
        partition_nova.slider_bg[i].body.radius=2;partition_nova.slider_bg[i].body.border.width=0;
        partition_nova.slider_ind[i].body.radius=2;partition_nova.slider_ind[i].body.main_color=partition_nova.slider_ind[i].body.grad_color=LV_COLOR_HEX(colors[i+1]);
        partition_nova.slider_knob[i].body.radius=3;partition_nova.slider_knob[i].body.border.width=1;
    }
    partition_nova.content=nyx_nova_panel(partition_nova.page,34,172,744,height-256);
    lv_obj_t *summary_panel=nyx_nova_panel(partition_nova.page,802,172,444,height-256);
    nyx_nova_text(summary_panel,"Récapitulatif",22,14,400,NOVA_TITLE);
    partition_nova.summary=nyx_nova_text(summary_panel,"",22,56,400,NOVA_SMALL);
    partition_nova.message=nyx_nova_text(summary_panel,"",22,172,400,NOVA_KICKER);
    part_info.slider_bar_hos=lv_bar_create(partition_nova.page,NULL);lv_obj_set_hidden(part_info.slider_bar_hos,true);lv_bar_set_range(part_info.slider_bar_hos,0,32760);
    lv_obj_t *tools=lv_btn_create(partition_nova.page,NULL);lv_btn_set_fit(tools,false,false);lv_obj_set_size(tools,370,46);lv_obj_set_pos(tools,34,height-64);
    lv_btn_set_action(tools,LV_BTN_ACTION_CLICK,part_info.emmc?_action_check_flash_linux:_action_part_manager_ums_sd);
    lv_obj_t *label=lv_label_create(tools,NULL);lv_label_set_text(label,part_info.emmc?"Flash Linux":"SD UMS");
    btn_flash_l4t=lv_btn_create(partition_nova.page,tools);lv_obj_set_pos(btn_flash_l4t,418,height-64);lv_btn_set_action(btn_flash_l4t,LV_BTN_ACTION_CLICK,_action_check_flash_linux);
    label=lv_label_create(btn_flash_l4t,NULL);lv_label_set_text(label,"Flash Linux");
    btn_flash_android=lv_btn_create(partition_nova.page,tools);lv_obj_set_pos(btn_flash_android,802,height-64);lv_btn_set_action(btn_flash_android,LV_BTN_ACTION_CLICK,_action_flash_android);
    label=lv_label_create(btn_flash_android,NULL);lv_label_set_text(label,"Flash Android");
    if(part_info.emmc)lv_obj_set_hidden(tools,true);
    /* Existing install actions stay visible below the allocation controls. */
    lv_win_add_btn(win,NULL,"Notes",nova_partition_notes);
    part_info.partition_button=lv_win_add_btn(win,NULL,"Next Step",nova_partition_review);
    partition_nova.ready=true;nova_partition_build_controls();
}

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

typedef struct{u32 enabled,sector;char*path,*nintendo_path;}emummc_cfg_t;
static lv_obj_t*emummc_manage_window;static lv_res_t(*emummc_tools)(lv_obj_t*);
static void load_emummc_cfg(emummc_cfg_t*c){c->enabled=1;c->sector=0x100000;c->path=strdup("emuMMC/RAW1");c->nintendo_path=strdup("emuMMC/RAW1/Nintendo");}
ACTION(_create_change_emummc_window) ACTION(_create_mbox_emummc_create) ACTION(_create_mbox_emummc_migrate)
static lv_res_t create_win_emummc_tools(lv_obj_t*);
lv_res_t preview_emummc_tools(lv_obj_t *btn)
{
    (void)btn;
    lv_obj_t *win=nyx_create_standard_window(SYMBOL_EDIT" emuMMC Manage",NULL);
    emummc_manage_window=win;emummc_tools=(void *)create_win_emummc_tools;
    sd_mount();emummc_cfg_t info;load_emummc_cfg(&info);sd_unmount();
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    int height=lv_obj_get_height(page)-36;
    lv_obj_t *panel=nyx_nova_panel(win,34,18,594,height);
    nyx_nova_icon_box(panel,"database",24,24,48,false);
    lv_obj_t *state=nyx_nova_text(panel,info.enabled?"Enabled":"Disabled",90,38,456,NOVA_TITLE);
    nyx_nova_fit_text(state,36);
    char details[1024],paths[2][256];
    const char *sources[]={info.path?info.path:"",info.nintendo_path?info.nintendo_path:""};
    for(u32 i=0;i<2;++i){u32 length=strlen(sources[i]),copy=length>252?252:length;while(copy&&((u8)sources[i][copy]&0xC0)==0x80)--copy;memcpy(paths[i],sources[i],copy);paths[i][copy]=0;if(copy<length)strcat(paths[i],"…");}
    if(info.enabled&&info.sector)s_printf(details,nyx_tr("Type : partition SD\nSecteur : 0x%08X\nDossier Nintendo : %s"),info.sector,paths[1]);
    else if(info.enabled)s_printf(details,nyx_tr("Type : fichiers SD\nDossier : %s\nDossier Nintendo : %s"),paths[0],paths[1]);
    else strcpy(details,nyx_tr("emuMMC is disabled and eMMC will be used for boot."));
    lv_obj_t *text=nyx_nova_text(panel,details,24,112,546,NOVA_BODY);nyx_nova_fit_text(text,height-140);
    free(info.path);free(info.nintendo_path);
    int tile_height=(height-32)/3;
    nyx_nova_action(win,"Change emuMMC","Choisir une image emuMMC sur la carte SD.","settings-2",_create_change_emummc_window,0,652,18,594,tile_height);
    nyx_nova_action(win,"Create emuMMC","Créer une copie sous forme de fichiers ou dans une partition SD.","database",_create_mbox_emummc_create,0,652,34+tile_height,594,tile_height);
    nyx_nova_action(win,"Migrate emuMMC","Migrer une sauvegarde ou réparer une partition emuMMC existante.","refresh-cw",_create_mbox_emummc_migrate,0,652,50+tile_height*2,594,tile_height);
    return LV_RES_OK;
}

typedef struct{char *name[64];char data[64*256];}dirlist_t;
typedef struct _emummc_images_t
{
	dirlist_t *dirlist;
	u32 part_sector[3];
	u32 part_type[3];
	u32 part_end[3];
	char part_path[3 * 128];
	lv_obj_t *win;
    lv_obj_t *file_items,*file_counter;
    u32 file_count,file_page;
} emummc_images_t;
static emummc_images_t *emummc_img;
static u32 preview_saved_emu_index,preview_saved_emu_sector;
static char preview_saved_emu_path[512];
static void save_emummc_cfg(u32 index,u32 sector,const char *path){preview_saved_emu_index=index;preview_saved_emu_sector=sector;strcpy(preview_saved_emu_path,path);}
static void _create_emummc_saved_mbox(void){}
static lv_res_t _save_raw_emummc_cfg_action(lv_obj_t * btn)
{
	lv_btn_ext_t *ext = lv_obj_get_ext_attr(btn);
	switch (ext->idx)
	{
	case 0:
		save_emummc_cfg(1, emummc_img->part_sector[0], &emummc_img->part_path[0]);
		break;
	case 1:
		save_emummc_cfg(2, emummc_img->part_sector[1], &emummc_img->part_path[128]);
		break;
	case 2:
		save_emummc_cfg(3, emummc_img->part_sector[2], &emummc_img->part_path[256]);
		break;
	}

	_create_emummc_saved_mbox();
	sd_unmount();

	return LV_RES_INV;
}
static lv_res_t _save_file_emummc_cfg_action(lv_obj_t *btn)
{
	u32 index=lv_obj_get_free_num(btn);
    if(!emummc_img->dirlist||index>=emummc_img->file_count)return LV_RES_OK;
    char path[512];s_printf(path,"emuMMC/%s",emummc_img->dirlist->name[index]);
    save_emummc_cfg(0,0,path);
	_create_emummc_saved_mbox();
	sd_unmount();

	return LV_RES_INV;
}
static void _fill_emummc_files(void)
{
    lv_obj_t *holder=emummc_img->file_items;lv_obj_clean(holder);
    int width=lv_obj_get_width(holder),height=lv_obj_get_height(holder),row_height=(height-36)/4;
    for(u32 row=0;row<4;++row){
        u32 index=emummc_img->file_page*4+row;if(index>=emummc_img->file_count)break;
        lv_obj_t *button=lv_btn_create(holder,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
        lv_obj_set_pos(button,0,row*(row_height+12));lv_obj_set_size(button,width,row_height);lv_obj_set_free_num(button,index);
        lv_btn_set_action(button,LV_BTN_ACTION_CLICK,_save_file_emummc_cfg_action);
        lv_obj_t *label=nyx_nova_text(button,"",16,(row_height-26)/2,width-32,NOVA_BODY);
        (lv_label_set_text)(label,emummc_img->dirlist->name[index]);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,30);
    }
    if(!emummc_img->file_count){lv_obj_t *label=nyx_nova_text(holder,"No emuMMC found.",0,height/2-30,width,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,60);}
    char position[32];s_printf(position,"%d / %d",emummc_img->file_page+1,(emummc_img->file_count+3)/4?(emummc_img->file_count+3)/4:1);(lv_label_set_text)(emummc_img->file_counter,position);
}
static lv_res_t _step_emummc_files(lv_obj_t *button)
{
    u32 pages=(emummc_img->file_count+3)/4;if(!pages)pages=1;
    emummc_img->file_page=(emummc_img->file_page+(lv_obj_get_free_num(button)?1:pages-1))%pages;_fill_emummc_files();return LV_RES_OK;
}
static void _create_change_emummc_ui(lv_obj_t *win,u32 file_count)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;
    emummc_img->file_count=file_count;emummc_img->file_page=0;
    lv_obj_t *left=nyx_nova_panel(page,34,18,594,height),*right=nyx_nova_panel(page,652,18,594,height);
    lv_obj_t *title=nyx_nova_text(left,"SD Raw Partitions",24,22,546,NOVA_TITLE);lv_label_set_align(title,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(title,38);
    title=nyx_nova_text(right,"SD File Based",24,22,546,NOVA_TITLE);lv_label_set_align(title,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(title,38);
    int row_height=(height-112)/3;
    for(u32 i=0;i<3;++i){
        char name[32],details[256];s_printf(name,"SD RAW %d",i+1);
        s_printf(details,nyx_tr("Sector start: 0x%08X\nFolder: %s"),emummc_img->part_sector[i],&emummc_img->part_path[i*128]);
        lv_obj_t *button=nyx_nova_action(left,name,details,"database",_save_raw_emummc_cfg_action,i,24,78+i*(row_height+12),546,row_height);
        ((lv_btn_ext_t *)lv_obj_get_ext_attr(button))->idx=i;
        if(!emummc_img->part_sector[i]||emummc_img->part_type[i]==0x83||!emummc_img->part_path[i*128]){lv_obj_set_click(button,false);lv_btn_set_state(button,LV_BTN_STATE_INA);}
    }
    emummc_img->file_items=nyx_nova_panel(right,24,78,546,height-164);lv_cont_set_style(emummc_img->file_items,&lv_style_transp_tight);
    for(u32 i=0;i<2;++i){
        lv_obj_t *button=lv_btn_create(right,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);lv_obj_set_size(button,170,42);lv_obj_set_pos(button,i?400:24,height-60);lv_obj_set_free_num(button,i);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,_step_emummc_files);
        lv_obj_t *label=nyx_nova_text(button,i?"Suivant":"Précédent",8,10,154,NOVA_SMALL);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);
    }
    emummc_img->file_counter=nyx_nova_text(right,"",210,height-48,170,NOVA_SMALL);lv_label_set_align(emummc_img->file_counter,LV_LABEL_ALIGN_CENTER);_fill_emummc_files();
}
typedef struct _jc_bt_conn_t
{
	u8 type;
	u8 mac[6];
	u8 host_mac[6];
	u8 ltk[16];
} jc_bt_conn_t;

typedef struct _jc_gamepad_rpt_t
{
	union
	{
		struct
		{
			// Joy-Con (R).
/*00*/		u32 y:1;
/*01*/		u32 x:1;
/*02*/		u32 b:1;
/*03*/		u32 a:1;
/*04*/		u32 sr_r:1;
/*05*/		u32 sl_r:1;
/*06*/		u32 r:1;
/*07*/		u32 zr:1;

			// Shared
/*08*/		u32 minus:1;
/*09*/		u32 plus:1;
/*10*/		u32 r3:1;
/*11*/		u32 l3:1;
/*12*/		u32 home:1;
/*13*/		u32 cap:1;
/*14*/		u32 pad:1;
/*15*/		u32 wired:1;

			// Joy-Con (L).
/*16*/		u32 down:1;
/*17*/		u32 up:1;
/*18*/		u32 right:1;
/*19*/		u32 left:1;
/*20*/		u32 sr_l:1;
/*21*/		u32 sl_l:1;
/*22*/		u32 l:1;
/*23*/		u32 zl:1;
		};
		u32 buttons;
	};

	u16 lstick_x;
	u16 lstick_y;
	u16 rstick_x;
	u16 rstick_y;
	bool center_stick_l;
	bool center_stick_r;
	bool conn_l;
	bool conn_r;
	bool sio_mode;
	u8 batt_info_l; // Also Sio Connected status.
	u8 batt_info_r; // Also Sio IRQ.
	u8 batt_chrg_l;
	u8 batt_chrg_r;
	jc_bt_conn_t bt_conn_l;
	jc_bt_conn_t bt_conn_r;
} jc_gamepad_rpt_t;


#define FTS4_EVENT_SIZE 8
typedef struct _touch_event_t {
	u8   raw[FTS4_EVENT_SIZE];
	u16  x, y; // Coordinates.
	u32  z;    // Orientation.
	bool touch;
	int  finger;
} touch_event_t;


#define BTN_POWER 1
#define BTN_VOL_UP 2
#define BTN_VOL_DOWN 4
static u32 fixture_power;static u32 btn_read(void){return fixture_power;}
static u32 get_tmr_ms(void){return lv_tick_get();}
static bool gui_diagnostics_inputs_ready(void);
/*
 * hwtest GUI - the stick model behind the Joy-Con test.
 *
 * Plain arithmetic on the raw 12-bit readings, with no LVGL in it, so the
 * host can check it (the emulator has no Joy-Con).
 *
 * A Joy-Con's stick is not centred on 0x800: each unit's resting point is
 * its own, up to a tenth of the travel away, and how far it reads from
 * there to the edge is the unit's own too - the factory calibration that
 * says what "full" is lives only in the Joy-Con's flash, and known-good
 * units read about 0x480 here, short of the figures published for that
 * calibration. So the test learns both: the resting point is wherever the
 * stick stays put for a second, the travel is the furthest it is pushed
 * each way. Once it has been pushed well out in all four directions, each
 * direction is judged against the stick's own best one (wear is lopsided:
 * a tired stick comes up short one way, not evenly all round), the middle
 * of that travel is where a calibrated stick would rest, and how far the
 * resting point is from it is the drift an owner feels.
 */
#ifndef HWTEST_GUI_STICK_H
#define HWTEST_GUI_STICK_H


#define STICK_SPAN   0x480   /* a good stick's travel, rest to edge: the dot's scale */
#define STICK_COVER  0x280   /* pushed this far: that direction is covered (a
                                stick drifting a third of the way out still
                                gets there on its short side)             */
#define STICK_SHORT  0x400   /* no direction reaches this: short all round */
#define STICK_STILL  40      /* no further than this for a second: at rest */

typedef struct {
    bool started;
    bool rest_known;
    u16 rest_x, rest_y;              /* where it sits when let go          */
    u16 min_x, max_x, min_y, max_y;  /* the furthest it has been pushed    */
    u16 still_x, still_y;            /* where it last stopped moving...    */
    u32 still_since;                 /* ...and since when (0: moving)      */
} gui_stick_t;

enum {
    STICK_RESTED = 1,   /* it has just settled: the resting point is new  */
    STICK_GREW   = 2,   /* it reached further than before                 */
};

void gui_stick_reset(gui_stick_t *s);
/* One reading; now in ms. Returns STICK_* flags for what changed. */
u32  gui_stick_feed(gui_stick_t *s, u16 x, u16 y, u32 now);
/* Where to draw the dot for a reading, in pixels from the ring's centre,
 * for a ring of radius half. */
void gui_stick_dot(const gui_stick_t *s, u16 x, u16 y, s32 half, s32 *px, s32 *py);
/* How far it has gone each way, in STICK_LEFT.. order, raw (reach) and as
 * a share of the furthest direction (pct, the best one reads 100). Returns
 * how many of the four are covered; 4 means the travel is known. */
enum { STICK_LEFT, STICK_RIGHT, STICK_UP, STICK_DOWN };
u32  gui_stick_range(const gui_stick_t *s, u32 reach[4], u32 pct[4]);
/* The resting point's distance from the middle of the travel, as a share
 * of the travel (half the full extent, the larger axis). Meaningful once
 * gui_stick_range() is 4. */
u32  gui_stick_drift(const gui_stick_t *s);

#endif
/*
 * hwtest GUI - the stick model behind the Joy-Con test (see gui_stick.h).
 */

static u32 _isqrt(u32 v)
{
    u32 r = 0, b = 1u << 30;
    while (b > v)
        b >>= 2;
    while (b) {
        if (v >= r + b) {
            v -= r + b;
            r = (r >> 1) + b;
        } else {
            r >>= 1;
        }
        b >>= 2;
    }
    return r;
}

void gui_stick_reset(gui_stick_t *s)
{
    memset(s, 0, sizeof(*s));
}

u32 gui_stick_feed(gui_stick_t *s, u16 x, u16 y, u32 now)
{
    u32 ev = 0;

    /* The first reading stands in for the resting point until it has
     * really rested, so the dot can be drawn from the start. */
    if (!s->started) {
        s->started = true;
        s->rest_x = s->min_x = s->max_x = s->still_x = x;
        s->rest_y = s->min_y = s->max_y = s->still_y = y;
        s->still_since = now;
        return 0;
    }

    if (x < s->min_x) { s->min_x = x; ev |= STICK_GREW; }
    if (x > s->max_x) { s->max_x = x; ev |= STICK_GREW; }
    if (y < s->min_y) { s->min_y = y; ev |= STICK_GREW; }
    if (y > s->max_y) { s->max_y = y; ev |= STICK_GREW; }

    s32 mx = (s32)x - s->still_x, my = (s32)y - s->still_y;
    if (mx * mx + my * my > STICK_STILL * STICK_STILL) {
        s->still_x = x;
        s->still_y = y;
        s->still_since = now;
    } else if (s->still_since && now - s->still_since > 1000) {
        s->still_since = 0;
        /* Held against the edge is not resting: a resting point is well
         * inside the travel. */
        if (x > 0x400 && x < 0xC00 && y > 0x400 && y < 0xC00) {
            s->rest_x = x;
            s->rest_y = y;
            s->rest_known = true;
            ev |= STICK_RESTED;
        }
    }
    return ev;
}

void gui_stick_dot(const gui_stick_t *s, u16 x, u16 y, s32 half, s32 *px, s32 *py)
{
    s32 dx = (s32)x - s->rest_x, dy = (s32)y - s->rest_y;
    s32 ox = dx * half / STICK_SPAN, oy = -dy * half / STICK_SPAN;
    if (ox > half) ox = half;
    if (ox < -half) ox = -half;
    if (oy > half) oy = half;
    if (oy < -half) oy = -half;
    *px = ox;
    *py = oy;
}

/* How far it has been pushed each way from where it rests, and the
 * furthest of the four - what the shares are measured against, never less
 * than STICK_COVER so an untouched stick does not read as full. */
static u32 _reach(const gui_stick_t *s, u32 reach[4])
{
    s32 r[4] = {
        (s32)s->rest_x - s->min_x,
        (s32)s->max_x - s->rest_x,
        (s32)s->max_y - s->rest_y,
        (s32)s->rest_y - s->min_y,
    };
    u32 span = STICK_COVER;
    for (u32 i = 0; i < 4; i++) {
        reach[i] = r[i] < 0 ? 0 : (u32)r[i];
        if (reach[i] > span)
            span = reach[i];
    }
    return span;
}

u32 gui_stick_range(const gui_stick_t *s, u32 reach[4], u32 pct[4])
{
    u32 span = _reach(s, reach), covered = 0;
    for (u32 i = 0; i < 4; i++) {
        pct[i] = reach[i] * 100 / span;
        covered += reach[i] >= STICK_COVER;
    }
    return covered;
}

u32 gui_stick_drift(const gui_stick_t *s)
{
    /* Against half the full extent, so a resting point that is off to one
     * side does not stretch its own yardstick. */
    u32 half_x = ((u32)s->max_x - s->min_x) / 2;
    u32 half_y = ((u32)s->max_y - s->min_y) / 2;
    u32 half = half_x > half_y ? half_x : half_y;
    if (half < STICK_COVER)
        half = STICK_COVER;
    s32 mid_x = ((s32)s->min_x + s->max_x) / 2;
    s32 mid_y = ((s32)s->min_y + s->max_y) / 2;
    s32 dx = (s32)s->rest_x - mid_x, dy = (s32)s->rest_y - mid_y;
    u32 p = _isqrt((u32)(dx * dx + dy * dy)) * 100 / half;
    return p > 100 ? 100 : p;
}
/* Native Nyx host for the supplied HWTest-RCM 0.15 diagnostics. */
#ifndef NYX_DIAGNOSTICS_H
#define NYX_DIAGNOSTICS_H

#define NYX_DIAG_G_IDENTITY (1u << 0)
#define NYX_DIAG_G_POWER    (1u << 1)
#define NYX_DIAG_G_MEMORY   (1u << 2)
#define NYX_DIAG_G_STORAGE  (1u << 3)
#define NYX_DIAG_G_WIRELESS (1u << 4)
#define NYX_DIAG_G_AUDIO    (1u << 5)
#define NYX_DIAG_G_SOUND    (1u << 6)
#define NYX_DIAG_G_DISPLAY  (1u << 7)
#define NYX_DIAG_G_INPUTS   (1u << 8)
#define NYX_DIAG_G_RAW      (1u << 9)
#define NYX_DIAG_G_ALL      ((1u << 10) - 1)
#define NYX_DIAG_PROBE_COUNT 42

typedef enum {
    NYX_DIAG_NOT_RUN, NYX_DIAG_OBSERVATION, NYX_DIAG_PASS,
    NYX_DIAG_WARN, NYX_DIAG_FAIL, NYX_DIAG_ERROR
} nyx_diag_state_t;
typedef struct {
    const char *name;
    const char *function;
    u32 group;
    nyx_diag_state_t state;
    u32 report_offset;
    u32 report_length;
} nyx_diag_probe_t;
typedef struct {
    const char *key;
    u32 severity; /* HWTest registry: 0 PASS, 1 WARN, 2 FAIL. */
    char detail[48];
} nyx_diag_finding_t;
typedef struct {
    /* Must suspend input polling and hardware maintenance before a probe.
     * Progress may call lv_refr_now(), never the normal LVGL task loop. */
    void (*suspend)(u32 groups, void *context);
    void (*progress)(u32 current, u32 total, const char *message, void *context);
    void (*resume)(u32 groups, void *context);
    void *context;
} nyx_diag_hooks_t;

const nyx_diag_probe_t *nyx_diag_probe(u32 index);
/* Return 0 for a completed execution, even if hardware findings FAIL.
 * 1 busy; 2 memory allocation; 3 invalid selection; 4 restoration issue. */
int nyx_diag_run(u32 groups, const nyx_diag_hooks_t *hooks);
int nyx_diag_run_probe(u32 index, const nyx_diag_hooks_t *hooks);
bool nyx_diag_running(void);
bool nyx_diag_truncated(void);
const char *nyx_diag_report(void);
u32 nyx_diag_finding_count(void);
bool nyx_diag_finding(u32 index, nyx_diag_finding_t *out);
bool nyx_diag_storage_ready(bool emmc);
/* Exports only the captured report, never reruns probes. Output path is
 * populated on success. Requires 128 bytes at path, may be NULL. */
int nyx_diag_export(char *path);
/* Normal runs call cleanup before resume. Available to the host as well. */
void nyx_diag_cleanup(void);
#endif
/* Scoped diagnostic evidence, never a whole-chip health certificate. GPL-2.0-or-later. */
#ifndef NYX_COMPONENT_SUMMARY_H
#define NYX_COMPONENT_SUMMARY_H
enum {NYX_COMPONENT_NOT_TESTED,NYX_COMPONENT_PARTIAL,NYX_COMPONENT_PASS,NYX_COMPONENT_WARN,NYX_COMPONENT_ANOMALY,NYX_COMPONENT_TEST_ERROR};
typedef struct {
    const char *name,*scope,*hint;
    const char *keys[8];
    u8 probes[8];
    u8 key_count,required_count,probe_count;
    u8 storage; /* 0 none, 1 eMMC, 2 microSD. */
} nyx_component_definition;
typedef struct {
    const nyx_component_definition *definition;
    u32 state,found,required_found,executed;
    bool partial;
    char measurement[160];
} nyx_component_result;
enum {NYX_MANUAL_COMMANDS,NYX_MANUAL_TOUCH,NYX_MANUAL_DISPLAY,NYX_MANUAL_RAM,NYX_MANUAL_COUNT};
/* Last captured measurements in this Nyx session, separate from auto probes. */
void nyx_component_observation(u32 kind,const char *text,bool warning);
u32 nyx_component_count(void);
bool nyx_component_get(u32 index,nyx_component_result *out);
const char *nyx_component_state(u32 state);
char *nyx_component_summary_text(void); /* Owned allocation, includes justified leads. */
char *nyx_component_detail_text(u32 index); /* Owned allocation, evidence and scope. */
#endif
/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_DIAG_FORMAT_H
#define NYX_DIAG_FORMAT_H
void nyx_diag_vsnprintf(char *out, u32 capacity, const char *format, va_list arguments);
void nyx_diag_snprintf(char *out, u32 capacity, const char *format, ...);
#endif
/* Bounded localized report formatter, with BDK's integer/padding syntax.
 * SPDX-License-Identifier: GPL-2.0-or-later */
typedef struct { char *out; u32 capacity, used; bool truncated; } sink;
static void put(sink *buffer, char value)
{
    if (buffer->used + 1 < buffer->capacity) buffer->out[buffer->used++] = value;
    else buffer->truncated = true;
}
static void diagnostic_format_text(sink *buffer, const char *value)
{
    while (*value) put(buffer, *value++);
}
static void spaces(sink *buffer, int count) { while (count-- > 0) put(buffer, ' '); }
void nyx_diag_vsnprintf(char *out, u32 capacity, const char *format, va_list arguments)
{
    if (!out || !capacity) return;
    sink buffer = {out, capacity, 0, false};
    char *translated = nyx_translate_text(format);
    const char *cursor = translated ? translated : format;
    while (cursor && *cursor) {
        if (*cursor++ != '%') { put(&buffer, cursor[-1]); continue; }
        int width = 0; char fill = 0;
        if ((*cursor >= '0' && *cursor <= '9') || *cursor == ' ' || *cursor == '.') {
            char first = *cursor++;
            if (*cursor >= '0' && *cursor <= '9') {
                fill = first;
                do { width = width * 10 + *cursor++ - '0'; } while (*cursor >= '0' && *cursor <= '9' && width < 10000);
            } else { width = first - '0'; fill = ' '; }
        }
        if (width > 4096) width = 4096;
        char type = *cursor;
        if (!type) break;
        ++cursor;
        if (type == '%') { put(&buffer, '%'); continue; }
        if (type == 'c') { char c = (char)va_arg(arguments, unsigned int); if (c) put(&buffer, c); continue; }
        if (type == 's') {
            const char *original = va_arg(arguments, const char *);
            if (!original) original = "";
            char *localized = nyx_translate_text(original);
            const char *value = localized ? localized : original;
            int padding = width - (int)strlen(value);
            if (fill != '.') spaces(&buffer, padding);
            diagnostic_format_text(&buffer, value);
            if (fill == '.') spaces(&buffer, padding);
            free(localized); continue;
        }
        if (type == 'd' || type == 'u' || type == 'x' || type == 'X' || type == 'p' || type == 'P') {
            u32 number = va_arg(arguments, u32);
            u32 base = type == 'd' || type == 'u' ? 10 : 16;
            bool negative = type == 'd' && (number & 0x80000000u);
            if (negative) number = 0u - number;
            char digits[34]; u32 used = 0;
            do { digits[used++] = "0123456789ABCDEF"[number % base]; number /= base; } while (number);
            int padding = width - (int)used - negative;
            while (padding-- > 0) put(&buffer, fill ? fill : ' ');
            if (negative) put(&buffer, '-');
            while (used) put(&buffer, digits[--used]);
            continue;
        }
        put(&buffer, '%'); put(&buffer, type);
    }
    free(translated);
    /* Retain only complete UTF-8 characters on a bounded report field. */
    if (buffer.truncated && buffer.used) {
        u32 start = buffer.used - 1;
        while (start && ((u8)out[start] & 0xC0) == 0x80) --start;
        u8 lead = out[start];
        u32 bytes = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        if (start + bytes > buffer.used) buffer.used = start;
    }
    out[buffer.used] = 0;
}
void nyx_diag_snprintf(char *out, u32 capacity, const char *format, ...)
{
    va_list arguments; va_start(arguments, format);
    nyx_diag_vsnprintf(out, capacity, format, arguments); va_end(arguments);
}
#ifndef NYX_INTERACTIVE_H
#define NYX_INTERACTIVE_H
bool nyx_interactive_joycon_active(void);
bool nyx_interactive_joycon(const jc_gamepad_rpt_t *pad);
bool nyx_interactive_touch(const touch_event_t *event);
lv_res_t nyx_joycon_test(lv_obj_t *button);
lv_res_t nyx_touch_test(lv_obj_t *button);
lv_res_t nyx_screen_patterns(lv_obj_t *button);
#endif

/* Hands-on logic adapted from the supplied HWTest-RCM GUI. GPL-2.0. */
#define GUI_W 1280
#define GUI_H 720
#define GUI_CONTENT_W 1212
#define GUI_CONTENT_H 480
#define GUI_MARGIN 34
#define GUI_RIGHT (GUI_W-GUI_MARGIN)
#define GUI_TOP 16
#define GUI_COL_BG_DARK nyx_nova_color(NOVA_COLOR_BACKGROUND)
#define GUI_COL_BG_LIGHT LV_COLOR_HEX(0x334050)
#define GUI_COL_LINE nyx_nova_color(NOVA_COLOR_BORDER)
#define GUI_COL_TEXT nyx_nova_color(NOVA_COLOR_TEXT)
#define GUI_COL_ACCENT nyx_nova_color(NOVA_COLOR_ICONS)
#define GUI_COL_OK LV_COLOR_HEX(0x8AE6B6)
#define GUI_COL_WARN LV_COLOR_HEX(0xF4B85F)
#define GUI_COL_FAIL LV_COLOR_HEX(0xFF6B6B)
#define GUI_HEX_ACCENT "55C9F1"
#define GUI_HEX_OK "8AE6B6"
#define GUI_HEX_WARN "F4B85F"
#define GUI_HEX_FAIL "FF6B6B"
#define ICON_OK "+"
#define ICON_WARN "!"
#define ICON_FAIL "!"
#define ICON_GAMEPAD ""
#define ICON_TOUCH ""
#define ICON_CHECK ""
#define gui_font_20 (*nyx_font_small())
#define gui_font_30 (*nyx_font_ui(30))
static lv_obj_t *interactive_window,*interactive_overlay;
static lv_task_t *interactive_power;
static bool interactive_jc,interactive_power_held;
static jc_gamepad_rpt_t interactive_pad;
static bool (*interactive_sink)(const touch_event_t *);
static void (*interactive_leave)(void);
static lv_signal_func_t interactive_base_signal;
static u32 interactive_mire,interactive_started,mire_visited,mire_observed,mire_anomalies;
static lv_obj_t *mire_controls,*mire_previous,*mire_next;
static bool mire_active;
static u32 mire_buttons;
static lv_res_t mire_change(lv_obj_t *button);
void gui_touch_test(void);
void gui_joycon_test(void);
static void _tt_result(void);
static bool s_tt_over;
static void gui_back(void);
static void gui_on_leave(void (*leave)(void)){interactive_leave=leave;}
static void gui_jc_nav(bool enabled){interactive_jc=!enabled;}
static jc_gamepad_rpt_t *gui_jc_state(void){return &interactive_pad;}
bool nyx_interactive_joycon_active(void){return interactive_jc;}
bool nyx_interactive_joycon(const jc_gamepad_rpt_t *pad){if(!interactive_jc)return false;if(pad)interactive_pad=*pad;return true;}
bool nyx_interactive_touch(const touch_event_t *event){return interactive_sink?interactive_sink(event):false;}
static void gui_touch_sink(bool (*sink)(const touch_event_t *)){interactive_sink=sink;}
static void gui_statusbar_hide(bool hidden){(void)hidden;}
static bool gui_touch_ok(void){return gui_diagnostics_inputs_ready();}
static void gui_uart_puts(const char *text){(void)text;}
static lv_theme_t *gui_th(void){return lv_theme_get_current();}
static lv_obj_t *gui_panel(lv_obj_t *p,int x,int y,int w,int h){return nyx_nova_panel(p,x,y,w,h);}
static lv_obj_t *gui_label(lv_obj_t *p,const lv_font_t *font,lv_color_t color,const char *text){
 (void)color;lv_obj_t *label=nyx_nova_text(p,text,0,0,0,font==nyx_font_small()?NOVA_SMALL:NOVA_BODY);lv_label_set_recolor(label,true);lv_label_set_long_mode(label,LV_LABEL_LONG_EXPAND);return label;
}
static lv_obj_t *gui_label_wrap(lv_obj_t *p,const lv_font_t *font,lv_color_t color,int w,const char *text){lv_obj_t *label=gui_label(p,font,color,text);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,w);return label;}
static lv_obj_t *gui_hint(lv_obj_t *p,int width,const char *text){return width?gui_label_wrap(p,nyx_font_small(),GUI_COL_TEXT,width,text):gui_label(p,nyx_font_small(),GUI_COL_TEXT,text);}
static lv_obj_t *gui_button(lv_obj_t *p,const char *text,lv_action_t action){lv_obj_t *b=lv_btn_create(p,NULL);lv_btn_set_fit(b,true,true);lv_btn_set_action(b,LV_BTN_ACTION_CLICK,action);lv_obj_t *l=lv_label_create(b,NULL);lv_label_set_text(l,text);return b;}
static void gui_show(void (*view)(void)){gui_back();view();}
static void gui_mbox(const char *text,const char **map,lv_btnm_action_t action){lv_obj_t *box=lv_mbox_create(lv_layer_top(),NULL);lv_obj_set_width(box,1000);lv_mbox_set_text(box,text);lv_mbox_add_btns(box,map,action);lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);lv_obj_set_free_num(box,0x1747);}
static lv_res_t interactive_window_signal(lv_obj_t *obj,lv_signal_t signal,void *parameter){
 if(signal==LV_SIGNAL_CLEANUP&&obj==interactive_window){
  interactive_sink=NULL;interactive_jc=false;mire_active=false;if(interactive_leave)interactive_leave();interactive_leave=NULL;
  if(interactive_power)lv_task_del(interactive_power);
  interactive_power=NULL;
  if(interactive_overlay)lv_obj_del(interactive_overlay);
  interactive_overlay=NULL;interactive_window=NULL;
 }
 return interactive_base_signal(obj,signal,parameter);
}
static lv_res_t interactive_close(lv_obj_t *button){return nyx_win_close_action(button);}
static void interactive_power_tick(void *context){
 (void)context;u32 buttons=btn_read();bool held=(buttons&BTN_POWER)!=0;
 if(mire_active){
  if((buttons&BTN_VOL_UP)&&!(mire_buttons&BTN_VOL_UP))mire_change(mire_next);
  if((buttons&BTN_VOL_DOWN)&&!(mire_buttons&BTN_VOL_DOWN))mire_change(mire_previous);
  mire_buttons=buttons;
 }
 if(held&&!interactive_power_held){if(interactive_sink&&!s_tt_over)_tt_result();else gui_back();}
 interactive_power_held=held;
}
static lv_obj_t *gui_win_new(const char *title){
 interactive_window=nyx_create_standard_window(title,interactive_close);
 nyx_nova_fixed_page(((lv_win_ext_t *)lv_obj_get_ext_attr(interactive_window))->page);
 interactive_base_signal=lv_obj_get_signal_func(interactive_window);lv_obj_set_signal_func(interactive_window,interactive_window_signal);
 interactive_started=get_tmr_ms();
 interactive_power_held=(btn_read()&BTN_POWER)!=0;
 interactive_power=lv_task_create(interactive_power_tick,40,LV_TASK_PRIO_MID,NULL);return interactive_window;
}
static lv_obj_t *gui_blank_new(void){
 gui_win_new("Grille tactile");interactive_overlay=lv_cont_create(lv_layer_top(),NULL);lv_cont_set_fit(interactive_overlay,false,false);lv_cont_set_layout(interactive_overlay,LV_LAYOUT_OFF);
 lv_obj_set_size(interactive_overlay,1280,720);lv_obj_set_pos(interactive_overlay,0,0);return interactive_overlay;
}
static void gui_back(void){
 /* Remove only this test's result box; never unrelated application dialogs. */
 for(lv_obj_t *child=lv_obj_get_child(lv_layer_top(),NULL);child;){lv_obj_t *next=lv_obj_get_child(lv_layer_top(),child);if(lv_obj_get_free_num(child)==0x1747)lv_obj_del(child);child=next;}
 if(interactive_window&&close_btn)nyx_win_close_action(close_btn);
}
/*
 * hwtest GUI - the hands-on tests: Joy-Con and touchscreen.
 *
 * Both read the hardware directly, many times a second, instead of going
 * through LVGL's input: every Joy-Con button is under test in the first,
 * every touch in the second. So the way out is always something the test
 * does not use - POWER (the window's focused Close button, the touch test's
 * Finish), or the touch test's message box once it is over.
 */



static lv_style_t *_box_style(lv_style_t *st, lv_color_t bg, lv_color_t border,
                              lv_coord_t radius, lv_coord_t border_w)
{
    lv_style_copy(st, &lv_style_plain);
    st->body.main_color = bg;
    st->body.grad_color = bg;
    st->body.radius = radius;
    st->body.border.color = border;
    st->body.border.width = border_w;
    st->body.border.opa = LV_OPA_COVER;
    return st;
}

static lv_obj_t *_box(lv_obj_t *parent, lv_style_t *st, lv_coord_t x,
                      lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *o = lv_obj_create(parent, NULL);
    lv_obj_set_style(o, st);
    lv_obj_set_click(o, false);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    return o;
}

/* ======================================================================= */
/* Joy-Con                                                                 */

#define STICK_RING  170
#define STICK_DOT   22
#define STICK_DEAD  56

/* The panel: two side by side across the content, 24 px in from their
 * frame for everything inside. */
#define JC_PANEL_W  ((GUI_CONTENT_W - 36) / 2)
#define JC_IN       24
#define JC_TEXT_X   (JC_IN + STICK_RING + 24)

typedef struct {
    const char *name;
    u8 bit;
} jc_btn_t;

/* Bit numbers of jc_gamepad_rpt_t.buttons (bdk input/joycon.h). Not SL
 * and SR: they sit inside the rail once a Joy-Con is on it, and the test
 * only reads Joy-Cons on the rails. */
static const jc_btn_t s_btn_l[] = {
    { "Up", 17 }, { "Down", 16 }, { "Left", 19 }, { "Right", 18 },
    { "L", 22 }, { "ZL", 23 }, { "-", 8 }, { "Capture", 13 },
    { "Stick", 11 },
};
static const jc_btn_t s_btn_r[] = {
    { "A", 3 }, { "B", 2 }, { "X", 1 }, { "Y", 0 },
    { "R", 6 }, { "ZR", 7 }, { "+", 9 }, { "Home", 12 },
    { "Stick", 10 },
};
#define JC_BTNS 9

typedef struct {
    lv_obj_t *status, *dot, *centre, *range, *count;
    lv_obj_t *btn[JC_BTNS];
    u32 lit;                 /* held right now                            */
    u32 done;                /* pressed at some point                     */
    bool seen;               /* connected at the last tick                */
    bool ever;               /* connected at some point                   */
    u32 drops;               /* times the connection went since then      */
    u8 batt;
    gui_stick_t stick;
} jc_side_t;

static jc_side_t s_jc[2];
static lv_task_t *s_jc_task;
static lv_style_t st_ring, st_dead, st_dot, st_pill, st_pill_on, st_pill_done;

/* How many of the buttons have been pressed so far, on the title line,
 * against the panel's right inset. */
static void _jc_count(jc_side_t *s)
{
    u32 n = 0;
    for (u32 b = s->done; b; b >>= 1)
        n += b & 1;
    char buf[48];
    nyx_diag_snprintf(buf,sizeof(buf),"Boutons : %d/%d",n,JC_BTNS);
    lv_label_set_text(s->count, buf);
    lv_obj_align(s->count, NULL, LV_ALIGN_IN_TOP_RIGHT, -JC_IN,
                 26 + (gui_font_30.h_px - gui_font_20.h_px) / 2);
}

static void _jc_side_build(lv_obj_t *win, int side, lv_coord_t x0)
{
    jc_side_t *s = &s_jc[side];
    memset(s, 0, sizeof(*s));
    lv_obj_t *panel = gui_panel(win, x0, GUI_TOP, JC_PANEL_W, GUI_CONTENT_H);

    lv_obj_t *t = gui_label(panel, &gui_font_30, GUI_COL_TEXT,
        side ? "Joy-Con droit" : "Joy-Con gauche");
    lv_obj_set_pos(t, JC_IN, 26);
    s->count = gui_hint(panel, 0, "");
    _jc_count(s);
    s->status = gui_hint(panel, 0, "Non connecté · placer sur le rail");
    lv_label_set_long_mode(s->status,LV_LABEL_LONG_BREAK);lv_obj_set_width(s->status,JC_PANEL_W-2*JC_IN);nyx_nova_fit_text(s->status,32);
    lv_obj_set_pos(s->status, JC_IN, 68);

    lv_obj_t *ring = _box(panel, &st_ring, JC_IN, 108, STICK_RING, STICK_RING);
    _box(ring, &st_dead, (STICK_RING - STICK_DEAD) / 2, (STICK_RING - STICK_DEAD) / 2,
         STICK_DEAD, STICK_DEAD);
    s->dot = _box(ring, &st_dot, (STICK_RING - STICK_DOT) / 2,
                  (STICK_RING - STICK_DOT) / 2, STICK_DOT, STICK_DOT);

    lv_coord_t tw = JC_PANEL_W - JC_TEXT_X - JC_IN;
    lv_obj_t *sh = gui_label(panel, &gui_font_30, GUI_COL_TEXT, "Stick");
    lv_obj_set_pos(sh, JC_TEXT_X, 112);
    s->centre = gui_hint(panel, tw, "Relâcher le stick une seconde, puis parcourir les quatre directions.");
    lv_obj_set_pos(s->centre, JC_TEXT_X, 146);
    s->range = gui_hint(panel, tw, "");
    lv_obj_set_pos(s->range, JC_TEXT_X, 214);

    /* The buttons, drawn as theme buttons in four even columns: one held
     * on the Joy-Con looks pressed here, and once let go it keeps the
     * theme's toggled look, so the plain ones are what is left to press. */
    const jc_btn_t *btns = side ? s_btn_r : s_btn_l;
    lv_coord_t pw = (JC_PANEL_W - 2 * JC_IN - 3 * 12) / 4;
    for (u32 i = 0; i < JC_BTNS; i++) {
        lv_obj_t *p = lv_cont_create(panel, NULL);
        lv_cont_set_style(p, &st_pill);
        lv_obj_set_click(p, false);
        lv_obj_set_size(p, pw, 42);
        lv_obj_set_pos(p, JC_IN + (i % 4) * (pw + 12), 292 + (i / 4) * 50);
        lv_obj_t *l = lv_label_create(p, NULL);
        lv_label_set_text(l, btns[i].name);
        lv_obj_align(l, NULL, LV_ALIGN_CENTER, 0, 0);
        s->btn[i] = p;
    }
}

static void _jc_status(jc_side_t *s)
{
    static const char *batt_name[] = { "empty", "very low", "low", "half", "full" };
    u8 lvl = s->batt >> 1;
    char buf[128];
    if(s->seen)nyx_diag_snprintf(buf,sizeof(buf),"Connecté · batterie %s",nyx_tr(lvl<5?batt_name[lvl]:"unknown"));
    else nyx_diag_snprintf(buf,sizeof(buf),"%s",nyx_tr("Connexion perdue · vérifier le rail"));
    if(s->ever)nyx_diag_snprintf(buf+strlen(buf),sizeof(buf)-strlen(buf)," · pertes %d / retours %d",s->drops+!s->seen,s->drops);
    lv_label_set_text(s->status, buf);
    nyx_nova_fit_text(s->status,32);
}

/* The travel: how many directions it has been round, then whether each
 * reaches as far as the stick's own best one. */
static void _jc_range(jc_side_t *s, int side)
{
    (void)side;u32 reach[4],pct[4];u32 covered=gui_stick_range(&s->stick,reach,pct);
    char text[192];nyx_diag_snprintf(text,sizeof(text),"Amplitude G %d / D %d\nH %d / B %d · %d/4",reach[0],reach[1],reach[2],reach[3],covered);
    lv_label_set_text(s->range,text);nyx_nova_fit_text(s->range,72);
}

static void _jc_judge(jc_side_t *s)
{
    u32 reach[4],pct[4],covered=gui_stick_range(&s->stick,reach,pct);char text[192];
    if(covered<4)nyx_diag_snprintf(text,sizeof(text),"Repos %d, %d\nDérive non évaluée : parcourir les 4 directions.",s->stick.rest_x,s->stick.rest_y);
    else nyx_diag_snprintf(text,sizeof(text),"Repos %d, %d\nDécalage mesuré : %d%% · %s",s->stick.rest_x,s->stick.rest_y,gui_stick_drift(&s->stick),nyx_tr(gui_stick_drift(&s->stick)>8?"À contrôler":"Centré"));
    lv_label_set_text(s->centre,text);nyx_nova_fit_text(s->centre,64);
}

static void _jc_side_update(int side, bool conn, u16 sx, u16 sy, u32 buttons,
                            u8 batt, bool inv_y)
{
    jc_side_t *s = &s_jc[side];

    if (conn != s->seen) {
        s->seen = conn;
        if (conn) {
            if (s->ever)
                s->drops++;
            s->ever = true;
            s->batt = batt;
            _jc_status(s);
        } else {
            /* bdk's driver drops a Joy-Con it got no report from for 1.8 s
             * or whose rail says it is gone, and brings it up again; what
             * was learnt about the stick is kept across that. */
            lv_label_set_text(s->status, s->ever ?
                "Connexion perdue · vérifier le rail" :
                "Non connecté · placer sur le rail");
            _jc_status(s);
        }
    }
    if (!conn)
        return;
    if (inv_y)
        sy = 0xFFF - sy;

    u32 ev = gui_stick_feed(&s->stick, sx, sy, get_tmr_ms());
    s32 half = (STICK_RING - STICK_DOT) / 2, px, py;
    gui_stick_dot(&s->stick, sx, sy, half, &px, &py);
    lv_obj_set_pos(s->dot, half + px, half + py);
    if (ev & STICK_RESTED)
        _jc_judge(s);
    if ((ev & STICK_GREW) && s->stick.rest_known) {
        _jc_range(s, side);
        /* The verdict, once the travel is complete and it has rested. */
        u32 reach[4], pct[4];
        if (gui_stick_range(&s->stick, reach, pct) == 4 && !s->stick.still_since)
            _jc_judge(s);
    }

    const jc_btn_t *btns = side ? s_btn_r : s_btn_l;
    for (u32 i = 0; i < JC_BTNS; i++) {
        bool on = (buttons >> btns[i].bit) & 1;
        bool was = (s->lit >> i) & 1;
        if (on == was)
            continue;
        s->lit ^= BIT(i);
        lv_cont_set_style(s->btn[i], on ? &st_pill_on : &st_pill_done);
        if (on && !(s->done & BIT(i))) {
            s->done |= BIT(i);
            _jc_count(s);
        }
    }
}

static void _jc_tick(void *param)
{
    jc_gamepad_rpt_t *jc = gui_jc_state();
    if (!jc)
        return;
    if (jc->sio_mode) {
        /* Lite: one controller, both halves always present. */
        _jc_side_update(0, jc->conn_l, jc->lstick_x, jc->lstick_y,
                        jc->buttons, jc->batt_info_l, true);
        _jc_side_update(1, jc->conn_l, jc->rstick_x, jc->rstick_y,
                        jc->buttons, jc->batt_info_l, true);
        return;
    }
    _jc_side_update(0, jc->conn_l, jc->lstick_x, jc->lstick_y, jc->buttons,
                    jc->batt_info_l, false);
    _jc_side_update(1, jc->conn_r, jc->rstick_x, jc->rstick_y, jc->buttons,
                    jc->batt_info_r, false);
}

static void _jc_leave(void)
{
    char record[1000],side[2][450];
    for(u32 i=0;i<2;++i){
        jc_side_t *s=&s_jc[i];u32 buttons=0,reach[4],percent[4];
        for(u32 bits=s->done;bits;bits>>=1)buttons+=bits&1;
        u32 directions=gui_stick_range(&s->stick,reach,percent);
        nyx_diag_snprintf(side[i],sizeof(side[i]),"%s : connecté une fois=%d, connecté à la fin=%d, boutons=%d/9, pertes=%d, reconnexions=%d. Repos connu=%d (%d, %d), directions=%d/4, amplitudes brutes G/D/H/B=%d/%d/%d/%d.",
            nyx_tr(i?"Droite":"Gauche"),s->ever,s->seen,buttons,s->drops+(s->ever&&!s->seen),s->drops,s->stick.rest_known,s->stick.rest_x,s->stick.rest_y,directions,reach[0],reach[1],reach[2],reach[3]);
        if(directions==4&&s->stick.rest_known){u32 used=strlen(side[i]);nyx_diag_snprintf(side[i]+used,sizeof(side[i])-used," Décalage mesuré=%d%%.",gui_stick_drift(&s->stick));}
    }
    nyx_diag_snprintf(record,sizeof(record),"Joy-Con/Lite (%d ms, Lite=%d).\n%s\n%s\nBoutons non pressés et directions non parcourues : NON TESTÉS. Aucun verdict de panne automatique.",get_tmr_ms()-interactive_started,interactive_pad.sio_mode,side[0],side[1]);
    nyx_component_observation(NYX_MANUAL_COMMANDS,record,false);
    if (s_jc_task)
        lv_task_del(s_jc_task);
    s_jc_task = NULL;
}

void gui_joycon_test(void)
{
    lv_obj_t *win = gui_win_new("Test Joy-Con");
    gui_jc_nav(false);
    gui_on_leave(_jc_leave);

    lv_theme_t *th = gui_th();
    _box_style(&st_ring, GUI_COL_BG_DARK, GUI_COL_LINE, LV_RADIUS_CIRCLE, 3);
    _box_style(&st_dead, GUI_COL_BG_LIGHT, GUI_COL_LINE, LV_RADIUS_CIRCLE, 2);
    _box_style(&st_dot, GUI_COL_ACCENT, GUI_COL_ACCENT, LV_RADIUS_CIRCLE, 0);
    lv_style_copy(&st_pill, th->btn.rel);
    st_pill.text.font = &gui_font_20;
    lv_style_copy(&st_pill_on, th->btn.pr);
    st_pill_on.text.font = &gui_font_20;
    lv_style_copy(&st_pill_done, th->btn.tgl_rel);
    st_pill_done.text.font = &gui_font_20;
    st_pill_done.text.color = th->btn.pr->text.color;

    _jc_side_build(win, 0, GUI_MARGIN);
    _jc_side_build(win, 1, GUI_RIGHT - JC_PANEL_W);

    if (s_jc_task)
        lv_task_del(s_jc_task);
    s_jc_task = lv_task_create(_jc_tick, 20, LV_TASK_PRIO_MID, NULL);
    _jc_tick(NULL);
}

/* ======================================================================= */
/* Touchscreen                                                             */

#define TT_CELL 80
#define TT_COLS (GUI_W / TT_CELL)
#define TT_ROWS (GUI_H / TT_CELL)

/* The panel in the middle: half the screen wide, 32 px in from its frame. */
#define TT_PANEL_W  (GUI_W / 2)
#define TT_PANEL_H  300
#define TT_IN       32

static lv_obj_t *s_cell[TT_ROWS][TT_COLS];
static bool s_hit[TT_ROWS][TT_COLS];
static u32 s_hits;
static bool s_tt_full;
static bool s_tt_pass;           /* this stroke is LVGL's: it began on Finish */
static int s_last_x = -1, s_last_y;
static u32 s_tt_up_at;           /* when the finger last lifted               */
static lv_task_t *s_tt_task;
static lv_obj_t *s_tt_info, *s_tt_count, *s_tt_finger, *s_tt_fin;
static lv_style_t st_cell, st_cell_hit, st_cell_miss, st_finger;

/* The panel comes back this long after the finger lifts. */
#define TT_PANEL_BACK_MS 1000

static void _tt_mark(int x, int y)
{
    int c = x / TT_CELL, r = y / TT_CELL;
    if (c < 0 || r < 0 || c >= TT_COLS || r >= TT_ROWS || s_hit[r][c])
        return;
    s_hit[r][c] = true;
    s_hits++;
    lv_obj_set_style(s_cell[r][c], &st_cell_hit);
}

static void _tt_count(void)
{
    char buf[64];
    nyx_diag_snprintf(buf,sizeof(buf),"Zones : %d/%d", s_hits, TT_COLS * TT_ROWS);
    lv_label_set_text(s_tt_count, buf);
}

static void _tt_result(void);

/* Whether a touch lands on the Finish button, while there is one to see. */
static bool _tt_on_finish(int x, int y)
{
    if (lv_obj_get_hidden(s_tt_info))
        return false;
    lv_area_t a;
    lv_obj_get_coords(s_tt_fin, &a);
    return x >= a.x1 && x <= a.x2 && y >= a.y1 && y <= a.y2;
}

static bool _tt_sink(const touch_event_t *ev)
{
    if (s_tt_over)
        return false;            /* finished: taps go to the message box */
    if (!ev->touch) {
        bool pass = s_tt_pass;
        s_tt_pass = false;
        if (s_last_x >= 0) {
            s_last_x = -1;
            s_tt_up_at = get_tmr_ms();
            lv_obj_set_hidden(s_tt_finger, true);
        }
        return !pass;            /* LVGL must see the end of its tap too */
    }
    if (s_tt_pass)
        return false;
    int x = ev->x, y = ev->y;
    /* Samples arrive at the input rate, so a quick stroke leaves gaps:
     * fill in along the line from the last one. */
    if (s_last_x >= 0) {
        int dx = x - s_last_x, dy = y - s_last_y;
        int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
        int steps = (adx > ady ? adx : ady) / (TT_CELL / 4) + 1;
        for (int i = 1; i <= steps; i++)
            _tt_mark(s_last_x + dx * i / steps, s_last_y + dy * i / steps);
    } else {
        /* A new stroke. One that begins on Finish is a tap, LVGL's to
         * handle; any other takes the panel away while the finger is
         * down, so the squares under it can be seen turning. */
        if (_tt_on_finish(x, y)) {
            s_tt_pass = true;
            return false;
        }
        lv_obj_set_hidden(s_tt_info, true);
        _tt_mark(x, y);
    }
    s_last_x = x;
    s_last_y = y;
    lv_obj_set_hidden(s_tt_finger, false);
    lv_obj_set_pos(s_tt_finger, x - 30, y - 30);
    _tt_count();
    /* Not _tt_result() here: this runs inside LVGL's input read, and the
     * verdict deletes and creates objects. _tt_tick picks it up. */
    if (s_hits == TT_COLS * TT_ROWS)
        s_tt_full = true;
    return true;
}

static void _tt_tick(void *param)
{
    if (s_tt_full) {
        _tt_result();
        return;
    }
    /* The panel comes back a moment after the finger lifts. */
    if (!s_tt_over && s_last_x < 0 && lv_obj_get_hidden(s_tt_info)
        && get_tmr_ms() - s_tt_up_at >= TT_PANEL_BACK_MS) {
        lv_obj_set_hidden(s_tt_info, false);
    }
}

static void touch_record(void)
{
    char record[512];nyx_diag_snprintf(record,sizeof(record),"Grille tactile : %d/%d zones parcourues, contrôleur initialisé=%d, fin demandée=%d, durée=%d ms. Les zones non parcourues sont NON TESTÉES. Ce contrôle manuel ne remplace pas le test matériel ITO.",s_hits,TT_COLS*TT_ROWS,gui_touch_ok(),s_tt_over,get_tmr_ms()-interactive_started);
    nyx_component_observation(NYX_MANUAL_TOUCH,record,false);
}
static void _tt_leave(void)
{
    if(s_tt_task)touch_record();
    if (s_tt_task)
        lv_task_del(s_tt_task);
    s_tt_task = NULL;
}

static lv_res_t _act_tt_finish(lv_obj_t *b)
{
    _tt_result();
    return LV_RES_OK;
}

static lv_res_t _tt_action(lv_obj_t *btns, const char *txt)
{
    if (lv_btnm_get_pressed(btns)==0)
        gui_show(gui_touch_test);
    else
        gui_back();
    return LV_RES_INV;
}

/* The verdict: what was never touched turns red, and a message box says
 * how much of the screen answered. */
static void _tt_result(void)
{
    if (s_tt_over)
        return;
    s_tt_over = true;
    touch_record();
    u32 total = TT_COLS * TT_ROWS;
    for (int r = 0; r < TT_ROWS; r++)
        for (int c = 0; c < TT_COLS; c++)
            if (!s_hit[r][c])
                lv_obj_set_style(s_cell[r][c], &st_cell_miss);
    lv_obj_set_hidden(s_tt_finger, true);
    lv_obj_set_hidden(s_tt_info, true);

    static char text[512];
    nyx_diag_snprintf(text,sizeof(text),"%d / %d zones parcourues.\n\nLes zones restantes sont NON TESTÉES. Ce contrôle manuel ne remplace pas le test matériel du contrôleur tactile.",s_hits,total);
    static const char *map[] = { "\221Again", "\221Done", "" };
    gui_mbox(text, map, _tt_action);
}

void gui_touch_test(void)
{
    lv_obj_t *root = gui_blank_new();
    gui_statusbar_hide(true);
    gui_touch_sink(_tt_sink);
    gui_on_leave(_tt_leave);

    _box_style(&st_cell, GUI_COL_BG_DARK, GUI_COL_BG_LIGHT, 0, 1);
    _box_style(&st_cell_hit, LV_COLOR_HEX(0x00775C), GUI_COL_ACCENT, 0, 1);
    _box_style(&st_cell_miss, LV_COLOR_HEX(0x493D26), GUI_COL_WARN, 0, 1);
    _box_style(&st_finger, GUI_COL_ACCENT, LV_COLOR_WHITE, LV_RADIUS_CIRCLE, 4);
    st_finger.body.opa = LV_OPA_60;

    /* The grid covers the whole panel: dead spots are often along an
     * edge. */
    memset(s_hit, 0, sizeof(s_hit));
    s_hits = 0;
    s_tt_over = false;
    s_tt_full = false;
    s_tt_pass = false;
    s_last_x = -1;
    _tt_leave();
    s_tt_task = lv_task_create(_tt_tick, 50, LV_TASK_PRIO_MID, NULL);
    for (int r = 0; r < TT_ROWS; r++)
        for (int c = 0; c < TT_COLS; c++)
            s_cell[r][c] = _box(root, &st_cell, c * TT_CELL, r * TT_CELL,
                                TT_CELL, TT_CELL);

    /* One line taller when the touchscreen failed, so its warning gets a
     * line of its own instead of crowding the bottom inset. */
    bool touch_ok = gui_touch_ok();
    lv_coord_t ph = TT_PANEL_H + (touch_ok ? 0 : 36);
    s_tt_info = gui_panel(root, (GUI_W - TT_PANEL_W) / 2, (GUI_H - ph) / 2,
                          TT_PANEL_W, ph);
    lv_obj_t *t = gui_label(s_tt_info, &gui_font_30, GUI_COL_TEXT,
                            "Test tactile");
    lv_obj_set_pos(t, TT_IN, 26);
    lv_obj_t *d = gui_hint(s_tt_info, TT_PANEL_W - 2 * TT_IN,
        "Parcourir chaque case avec un doigt. Une case sombre n’a pas été testée ; cela ne prouve pas un défaut.");
    lv_obj_set_pos(d, TT_IN, 80);
    lv_obj_t *fin = gui_button(s_tt_info, "Terminer", _act_tt_finish);
    lv_obj_align(fin, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -TT_IN, -TT_IN);
    s_tt_fin = fin;

    /* The short lines on the left sit 36 px apart, the last of them on the
     * Finish button's centre line. */
    lv_coord_t y = lv_obj_get_y(fin) + lv_obj_get_height(fin) / 2
                   - gui_font_20.h_px / 2 - (touch_ok ? 1 : 2) * 36;
    s_tt_count = gui_label(s_tt_info, &gui_font_20, GUI_COL_TEXT, "");
    lv_obj_set_pos(s_tt_count, TT_IN, y);
    _tt_count();
    lv_obj_t *hint = gui_hint(s_tt_info, 0, "POWER permet aussi de terminer.");
    lv_obj_set_pos(hint, TT_IN, y + 36);
    if (!touch_ok) {
        lv_obj_t *w = gui_label(s_tt_info, &gui_font_20, GUI_COL_WARN,
                                "Le contrôleur tactile n’a pas démarré.");
        lv_obj_set_pos(w, TT_IN, y + 72);
    }

    s_tt_finger = _box(root, &st_finger, 0, 0, 60, 60);
    lv_obj_set_hidden(s_tt_finger, true);
}

lv_res_t nyx_joycon_test(lv_obj_t *button){(void)button;if(!interactive_window)gui_joycon_test();return LV_RES_OK;}
lv_res_t nyx_touch_test(lv_obj_t *button){(void)button;if(!interactive_window)gui_touch_test();return LV_RES_OK;}
static const u32 mire_colors[]={0xFF0000,0x00FF00,0x0000FF,0xFFFFFF,0x000000};
static lv_style_t mire_style;
static lv_obj_t *mire_area,*mire_observation;
static void mire_record(void)
{
    char record[512];nyx_diag_snprintf(record,sizeof(record),"Mires R/V/B/blanc/noir : masques affichés=0x%02X, observés=0x%02X, anomalies signalées=0x%02X, durée=%d ms. Les masques sont dans cet ordre, bit 0=rouge. Déclaration visuelle de l’utilisateur, aucune détection automatique des pixels.",mire_visited,mire_observed,mire_anomalies,get_tmr_ms()-interactive_started);
    nyx_component_observation(NYX_MANUAL_DISPLAY,record,mire_anomalies!=0);
}
static lv_res_t mire_change(lv_obj_t *button){
 interactive_mire=(interactive_mire+(lv_obj_get_free_num(button)?1:4))%5;
 mire_visited|=BIT(interactive_mire);mire_record();
 mire_style.body.main_color=mire_style.body.grad_color=LV_COLOR_HEX(mire_colors[interactive_mire]);lv_btn_set_style(mire_area,LV_BTN_STYLE_REL,&mire_style);lv_btn_set_style(mire_area,LV_BTN_STYLE_PR,&mire_style);return LV_RES_OK;
}
static lv_res_t mire_observe(lv_obj_t *button){
 mire_observed|=BIT(interactive_mire);
 if(lv_obj_get_free_num(button))mire_anomalies|=BIT(interactive_mire);else mire_anomalies&=~BIT(interactive_mire);
 mire_record();
 lv_label_set_text(mire_observation,lv_obj_get_free_num(button)?"Observation manuelle : anomalie visuelle signalée.":"Observation manuelle : aucune anomalie visuelle signalée. Les pixels ne sont pas vérifiés automatiquement.");return LV_RES_OK;
}
static lv_res_t mire_toggle_controls(lv_obj_t *button)
{
    (void)button;lv_obj_set_hidden(mire_controls,!lv_obj_get_hidden(mire_controls));return LV_RES_OK;
}
lv_res_t nyx_screen_patterns(lv_obj_t *button)
{
    (void)button;if(interactive_window)return LV_RES_OK;
    gui_win_new("Mires de l'écran");interactive_mire=0;mire_active=true;mire_buttons=btn_read();
    mire_visited=1;mire_observed=mire_anomalies=0;gui_on_leave(mire_record);mire_record();
    interactive_overlay=lv_cont_create(lv_layer_top(),NULL);lv_cont_set_fit(interactive_overlay,false,false);lv_cont_set_layout(interactive_overlay,LV_LAYOUT_OFF);
    lv_obj_set_size(interactive_overlay,1280,720);lv_obj_set_pos(interactive_overlay,0,0);
    mire_area=lv_btn_create(interactive_overlay,NULL);lv_btn_set_fit(mire_area,false,false);lv_obj_set_size(mire_area,1280,720);lv_obj_set_pos(mire_area,0,0);
    lv_style_copy(&mire_style,&lv_style_plain);mire_style.body.border.width=0;mire_style.body.radius=0;
    mire_style.body.main_color=mire_style.body.grad_color=LV_COLOR_HEX(0xFF0000);lv_btn_set_style(mire_area,LV_BTN_STYLE_REL,&mire_style);lv_btn_set_style(mire_area,LV_BTN_STYLE_PR,&mire_style);
    lv_btn_set_action(mire_area,LV_BTN_ACTION_CLICK,mire_toggle_controls);
    mire_controls=nyx_nova_panel(interactive_overlay,34,526,1212,172);lv_obj_set_click(mire_controls,true);
    mire_previous=gui_button(mire_controls,"Précédent",mire_change);lv_obj_set_free_num(mire_previous,0);lv_obj_set_pos(mire_previous,20,14);
    mire_next=gui_button(mire_controls,"Suivant",mire_change);lv_obj_set_free_num(mire_next,1);lv_obj_set_pos(mire_next,240,14);
    lv_obj_t *ok=gui_button(mire_controls,"Aucune anomalie observée",mire_observe);lv_obj_set_pos(ok,460,14);lv_obj_set_free_num(ok,0);
    lv_obj_t *fault=gui_button(mire_controls,"Anomalie observée",mire_observe);lv_obj_set_pos(fault,850,14);lv_obj_set_free_num(fault,1);
    lv_obj_t *commands[]={mire_previous,mire_next,ok,fault};
    const int widths[]={200,200,370,360},positions[]={20,234,448,832};
    for(u32 i=0;i<4;++i){
        lv_btn_set_fit(commands[i],false,false);lv_obj_set_size(commands[i],widths[i],60);lv_obj_set_pos(commands[i],positions[i],14);
        lv_obj_t *label=lv_obj_get_child(commands[i],NULL);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,widths[i]-24);lv_label_set_style(label,(lv_style_t *)nyx_nova_text_style(NOVA_SMALL));
    }
    mire_observation=nyx_nova_text(mire_controls,"Toucher la mire masque les commandes. VOL+/VOL- : changer. POWER : quitter. Observation visuelle uniquement, sans détection automatique des pixels.",20,84,1172,NOVA_SMALL);nyx_nova_fit_text(mire_observation,76);
    return LV_RES_OK;
}
/* Native Nyx host for the supplied HWTest-RCM 0.15 diagnostics. */
#ifndef NYX_DIAGNOSTICS_H
#define NYX_DIAGNOSTICS_H

#define NYX_DIAG_G_IDENTITY (1u << 0)
#define NYX_DIAG_G_POWER    (1u << 1)
#define NYX_DIAG_G_MEMORY   (1u << 2)
#define NYX_DIAG_G_STORAGE  (1u << 3)
#define NYX_DIAG_G_WIRELESS (1u << 4)
#define NYX_DIAG_G_AUDIO    (1u << 5)
#define NYX_DIAG_G_SOUND    (1u << 6)
#define NYX_DIAG_G_DISPLAY  (1u << 7)
#define NYX_DIAG_G_INPUTS   (1u << 8)
#define NYX_DIAG_G_RAW      (1u << 9)
#define NYX_DIAG_G_ALL      ((1u << 10) - 1)
#define NYX_DIAG_PROBE_COUNT 42

typedef enum {
    NYX_DIAG_NOT_RUN, NYX_DIAG_OBSERVATION, NYX_DIAG_PASS,
    NYX_DIAG_WARN, NYX_DIAG_FAIL, NYX_DIAG_ERROR
} nyx_diag_state_t;
typedef struct {
    const char *name;
    const char *function;
    u32 group;
    nyx_diag_state_t state;
    u32 report_offset;
    u32 report_length;
} nyx_diag_probe_t;
typedef struct {
    const char *key;
    u32 severity; /* HWTest registry: 0 PASS, 1 WARN, 2 FAIL. */
    char detail[48];
} nyx_diag_finding_t;
typedef struct {
    /* Must suspend input polling and hardware maintenance before a probe.
     * Progress may call lv_refr_now(), never the normal LVGL task loop. */
    void (*suspend)(u32 groups, void *context);
    void (*progress)(u32 current, u32 total, const char *message, void *context);
    void (*resume)(u32 groups, void *context);
    void *context;
} nyx_diag_hooks_t;

const nyx_diag_probe_t *nyx_diag_probe(u32 index);
/* Return 0 for a completed execution, even if hardware findings FAIL.
 * 1 busy; 2 memory allocation; 3 invalid selection; 4 restoration issue. */
int nyx_diag_run(u32 groups, const nyx_diag_hooks_t *hooks);
int nyx_diag_run_probe(u32 index, const nyx_diag_hooks_t *hooks);
bool nyx_diag_running(void);
bool nyx_diag_truncated(void);
const char *nyx_diag_report(void);
u32 nyx_diag_finding_count(void);
bool nyx_diag_finding(u32 index, nyx_diag_finding_t *out);
bool nyx_diag_storage_ready(bool emmc);
/* Exports only the captured report, never reruns probes. Output path is
 * populated on success. Requires 128 bytes at path, may be NULL. */
int nyx_diag_export(char *path);
/* Normal runs call cleanup before resume. Available to the host as well. */
void nyx_diag_cleanup(void);
#endif
/* Nyx <-> isolated RAM-service contract. The entire first 4 MiB is excluded
 * from the destructive test. This header and return payload live in its tail. */
#ifndef NYX_RAM_HANDOFF_H
#define NYX_RAM_HANDOFF_H
#define NYX_RAM_UI_ADDR      0x80000000u
#define NYX_RAM_UI_MAX       0x00300000u
#define NYX_RAM_UI_MAGIC     0x4955524Eu
#define NYX_RAM_HANDOFF_ADDR 0x803DF000u
#define NYX_RAM_RETURN_ADDR  0x803E0000u
#define NYX_RAM_RETURN_MAX   0x00020000u
#define NYX_RAM_HANDOFF_MAGIC 0x524D594Eu /* NYMR */
#define NYX_RAM_RESULT_MAGIC  0x5253594Eu /* NYSR */
#define NYX_RAM_VERSION 2u
#define NYX_RAM_MODE_QUICK 0u
#define NYX_RAM_MODE_ADVANCED 1u
#define NYX_RAM_MODE_CUSTOM 2u
#define NYX_RAM_PATTERN_ALL 0x7Fu
#define NYX_RAM_RESULT_WORDS 12u
#define NYX_RAM_RESULT_PASS 1u
#define NYX_RAM_RESULT_FAIL 2u
#define NYX_RAM_RESULT_ABORT 3u
#define NYX_RAM_RESULT_ERROR 4u
typedef struct {
    u32 magic;
    u32 version;
    u32 payload_size;
    u32 payload_hash;
    u32 auto_run;
    u32 result_magic;
    u32 result;
    u32 errors;
    u32 patterns_completed;
    u32 patterns_total;
    u32 tested_mib;
    u32 dram_mib;
    u32 elapsed_ms;
    u32 mtc_errors;
    u32 mode;
    u32 selected_mask;
    u32 completed_mask;
    u32 verified_mib; /* Read back with at least one selected pattern. */
    u32 result_hash;
    u32 request_hash;
} nyx_ram_handoff_t;
static inline u32 nyx_ram_hash(const void *data, u32 size)
{
    const u8 *p = (const u8 *)data;
    u32 hash = 2166136261u;
    while (size--) hash = (hash ^ *p++) * 16777619u;
    return hash;
}
static inline u32 nyx_ram_count(u32 mask)
{u32 count=0;while(mask){count+=mask&1u;mask>>=1;}return count;}
static inline bool nyx_ram_selection_valid(u32 mode,u32 mask)
{
    if(mode>NYX_RAM_MODE_CUSTOM||!mask||(mask&~NYX_RAM_PATTERN_ALL))return false;
    return mode==NYX_RAM_MODE_QUICK?mask==1u:mode==NYX_RAM_MODE_ADVANCED?mask==NYX_RAM_PATTERN_ALL:true;
}
static inline u32 nyx_ram_request_hash(const nyx_ram_handoff_t *h)
{
    const u32 words[]={h->version,h->payload_size,h->payload_hash,h->auto_run,h->mode,h->selected_mask};
    return nyx_ram_hash(words,sizeof(words));
}
static inline bool nyx_ram_request_valid(const nyx_ram_handoff_t *h)
{
    return h->magic==NYX_RAM_HANDOFF_MAGIC&&h->version==NYX_RAM_VERSION&&h->auto_run==1u&&
        h->payload_size>=0x100u&&h->payload_size<=NYX_RAM_RETURN_MAX&&
        nyx_ram_selection_valid(h->mode,h->selected_mask)&&h->request_hash==nyx_ram_request_hash(h);
}
static inline bool nyx_ram_result_valid(const nyx_ram_handoff_t *h)
{
    if(h->version!=NYX_RAM_VERSION||h->result_magic!=NYX_RAM_RESULT_MAGIC||
        h->result<NYX_RAM_RESULT_PASS||h->result>NYX_RAM_RESULT_ERROR||
        !nyx_ram_selection_valid(h->mode,h->selected_mask)||
        (h->completed_mask&~h->selected_mask)||h->patterns_total!=nyx_ram_count(h->selected_mask)||
        h->patterns_completed!=nyx_ram_count(h->completed_mask)||h->tested_mib>h->dram_mib||h->verified_mib>h->tested_mib||
        h->result_hash!=nyx_ram_hash(&h->result,NYX_RAM_RESULT_WORDS*sizeof(u32)))return false;
    return h->result!=NYX_RAM_RESULT_PASS||(!h->errors&&!h->mtc_errors&&h->tested_mib&&h->verified_mib==h->tested_mib&&h->completed_mask==h->selected_mask);
}
#endif


void nyx_components_window(lv_action_t,lv_action_t);
static const struct {
    const char *title;
    const char *detail;
    u32 groups;
} groups[] = {
    {"Bilan HWTest", "Tous les groupes, hors stress RAM", NYX_DIAG_G_ALL},
    {"Identite", "SoC, fusibles et KFUSE", NYX_DIAG_G_IDENTITY},
    {"Alimentation", "Batterie, charge, tensions, thermique", NYX_DIAG_G_POWER},
    {"Memoire / horloges", "Identite DRAM et horloges", NYX_DIAG_G_MEMORY},
    {"Stockage", "SD, eMMC, partitions et lecteur", NYX_DIAG_G_STORAGE},
    {"Sans fil", "Bluetooth et Wi-Fi / PCIe", NYX_DIAG_G_WIRELESS},
    {"Audio", "Codec et horloges audio", NYX_DIAG_G_AUDIO},
    {"Test sonore", "Haut-parleurs et prise casque", NYX_DIAG_G_SOUND},
    {"Ecran", "Dalle et retroeclairage LCD / OLED", NYX_DIAG_G_DISPLAY},
    {"Commandes", "Tactile, Joy-Con, boutons, lumiere", NYX_DIAG_G_INPUTS},
    {"Etat bas niveau", "GPIO, UART et etat de demarrage", NYX_DIAG_G_RAW},
    {"Test RAM isole", "Passe complete puis retour Hekate", 0},
};

static lv_obj_t *text(lv_obj_t *parent, const char *value, lv_coord_t x,
    lv_coord_t y, lv_coord_t width, bool small)
{
    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_style(label, small ? &hint_small_style : lv_theme_get_current()->label.prim);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, width);
    lv_label_set_text(label, value);
    lv_obj_set_pos(label, x, y);
    return label;
}
static lv_obj_t *button(lv_obj_t *parent, const char *title, lv_coord_t x,
    lv_coord_t y, lv_coord_t width, lv_action_t action, u32 id)
{
    lv_obj_t *btn = lv_btn_create(parent, NULL);
    lv_btn_set_fit(btn, false, false);
    lv_obj_set_size(btn, width, 64);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_free_num(btn, id);
    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *label = lv_label_create(btn, NULL);
    lv_label_set_text(label, title);
    return btn;
}

ACTION(show_group)

static nyx_diag_probe_t preview_probes[]={
{"Diagnostics","probe_verdict",0,2,0,0},
{"SoC","probe_soc",NYX_DIAG_G_IDENTITY,3,0,0},
{"Fuses","probe_fuses",NYX_DIAG_G_IDENTITY,4,0,0},
{"Fuses","probe_kfuse",NYX_DIAG_G_IDENTITY,5,0,0},
{"Power & charging","probe_pmic",NYX_DIAG_G_POWER,2,0,0},
{"Power & charging","probe_max77812",NYX_DIAG_G_POWER,3,0,0},
{"Power & charging","probe_pmic_gpios",NYX_DIAG_G_POWER,4,0,0},
{"Power & charging","probe_regulators",NYX_DIAG_G_POWER,5,0,0},
{"Power & charging","probe_5v",NYX_DIAG_G_POWER,2,0,0},
{"Power & charging","probe_battery",NYX_DIAG_G_POWER,3,0,0},
{"Power & charging","probe_charger",NYX_DIAG_G_POWER,4,0,0},
{"Power & charging","probe_usbpd",NYX_DIAG_G_POWER,5,0,0},
{"Power & charging","probe_thermal",NYX_DIAG_G_POWER,2,0,0},
{"Power & charging","probe_fan",NYX_DIAG_G_POWER,3,0,0},
{"Memory & clocks","probe_dram",NYX_DIAG_G_MEMORY,4,0,0},
{"Memory & clocks","probe_clocks",NYX_DIAG_G_MEMORY,5,0,0},
{"Storage","probe_storage",NYX_DIAG_G_STORAGE,2,0,0},
{"Storage","probe_serial",NYX_DIAG_G_STORAGE,3,0,0},
{"Storage","probe_partitions",NYX_DIAG_G_STORAGE,4,0,0},
{"Storage","probe_emmc_health",NYX_DIAG_G_STORAGE,5,0,0},
{"Storage","probe_gpt",NYX_DIAG_G_STORAGE,2,0,0},
{"Storage","probe_boot0_pkg1",NYX_DIAG_G_STORAGE,3,0,0},
{"Storage","probe_autorcm",NYX_DIAG_G_STORAGE,4,0,0},
{"Storage","probe_sd_content",NYX_DIAG_G_STORAGE,5,0,0},
{"Storage","probe_storage_errors",NYX_DIAG_G_STORAGE,2,0,0},
{"Storage","probe_gamecard",NYX_DIAG_G_STORAGE,3,0,0},
{"Wireless","probe_bt_radio",NYX_DIAG_G_WIRELESS,4,0,0},
{"Wireless","probe_wifi_pcie",NYX_DIAG_G_WIRELESS,5,0,0},
{"Wireless","probe_wifi_link",NYX_DIAG_G_WIRELESS,2,0,0},
{"Audio","probe_audio",NYX_DIAG_G_AUDIO,3,0,0},
{"Audio","probe_audio_clocks",NYX_DIAG_G_AUDIO,4,0,0},
{"Audio","probe_audio_beep",NYX_DIAG_G_SOUND,5,0,0},
{"Display","probe_display",NYX_DIAG_G_DISPLAY,2,0,0},
{"Display","probe_backlight",NYX_DIAG_G_DISPLAY,3,0,0},
{"Inputs","probe_touch",NYX_DIAG_G_INPUTS,4,0,0},
{"Inputs","probe_als",NYX_DIAG_G_INPUTS,5,0,0},
{"Inputs","probe_joycon",NYX_DIAG_G_INPUTS,2,0,0},
{"Inputs","probe_inputs",NYX_DIAG_G_INPUTS,3,0,0},
{"Raw state","probe_gpio_census",NYX_DIAG_G_RAW,4,0,0},
{"Raw state","probe_uart_b",NYX_DIAG_G_RAW,5,0,0},
{"Raw state","probe_reset",NYX_DIAG_G_RAW,2,0,0},
{"Raw state","probe_pmc_scratch",NYX_DIAG_G_RAW,3,0,0},
};
const nyx_diag_probe_t*nyx_diag_probe(u32 i){return i<42?&preview_probes[i]:NULL;}

#define ONE_PROBE (1u<<30)
static lv_obj_t*results_window,*diagnostic_last_holder;static nyx_ram_handoff_t last_ram;static bool last_ram_valid;
ACTION(run_selection) ACTION(show_report) ACTION(export_report) ACTION(launch_ram)
bool nyx_diag_truncated(void){return false;}static bool gui_diagnostics_inputs_ready(void){return true;}
static void notice(const char *title, const char *message)
{
    lv_obj_t *win = nyx_create_standard_window(title, NULL);
    text(win, message, 32, 28, 1180, false);
}
static const char *state_name(nyx_diag_state_t state)
{
    switch (state) {
    case NYX_DIAG_PASS: return nyx_tr("OK");
    case NYX_DIAG_WARN: return nyx_tr("AVERTISSEMENT");
    case NYX_DIAG_FAIL: return nyx_tr("DEFAUT");
    case NYX_DIAG_ERROR: return nyx_tr("ERREUR");
    case NYX_DIAG_OBSERVATION: return nyx_tr("OBSERVATION");
    default: return nyx_tr("NON EXECUTE");
    }
}

static lv_res_t close_results(lv_obj_t *btn)
{
    results_window = NULL;
    return nyx_win_close_action(btn);
}
/* Stable pages of native Nova cards replace the long vertical probe lists. */
static const char *const diagnostic_titles[NYX_DIAG_PROBE_COUNT]={
    "Bilan général","SoC","Fusibles","KFUSE","PMIC","Régulateur CPU","Entrées/sorties PMIC","Alimentations","Tension 5 V","Batterie","Chargeur","USB-C Power Delivery","Températures","Ventilateur",
    "Mémoire DRAM","Horloges","Stockage","Numéros de série","Partitions","Santé eMMC","Table GPT","Démarrage BOOT0","AutoRCM","Contenu de la carte SD","Erreurs de stockage","Lecteur de jeux",
    "Bluetooth","Wi-Fi PCIe","Connexion Wi-Fi","Codec audio","Horloges audio","Test sonore","Dalle écran","Rétroéclairage","Écran tactile","Capteur de luminosité","Joy-Con","Boutons","GPIO","UART B","État de réinitialisation","Registres PMC"
};
const char *nyx_diag_probe_caption(u32 index)
{
    return index<NYX_DIAG_PROBE_COUNT?nyx_tr(diagnostic_titles[index]):nyx_tr("Diagnostic");
}

typedef struct {
    lv_cont_ext_t container;
    u32 indices[NYX_DIAG_PROBE_COUNT],count,page;
    bool results;
    lv_obj_t *tiles,*position,*previous,*next;
} diag_tiles_ext;

static void diagnostic_tiles_fill(lv_obj_t *holder)
{
    diag_tiles_ext *ext=lv_obj_get_ext_attr(holder);
    lv_obj_clean(ext->tiles);
    int width=(lv_obj_get_width(holder)-24)/3,height=(lv_obj_get_height(ext->tiles)-24)/3;
    for(u32 slot=0;slot<9&&ext->page*9+slot<ext->count;++slot){
        u32 index=ext->indices[ext->page*9+slot];const nyx_diag_probe_t *probe=nyx_diag_probe(index);
        char detail[192];
        if(ext->results)s_printf(detail,"[%s] %s",state_name(probe->state),nyx_tr(probe->name));
        else s_printf(detail,"%s",nyx_tr(probe->name));
        nyx_nova_action(ext->tiles,diagnostic_titles[index],detail,"activity",ext->results?show_report:run_selection,
            ext->results?index:ONE_PROBE|index,slot%3*(width+12),slot/3*(height+12),width,height);
    }
    char position[64];s_printf(position,nyx_tr("Page %d / %d"),ext->page+1,ext->count?(ext->count+8)/9:1);lv_label_set_text(ext->position,position);
    lv_obj_set_click(ext->previous,ext->page>0);lv_btn_set_state(ext->previous,ext->page?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    bool next=(ext->page+1)*9<ext->count;lv_obj_set_click(ext->next,next);lv_btn_set_state(ext->next,next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t diagnostic_tiles_step(lv_obj_t *button)
{
    lv_obj_t *holder=lv_obj_get_free_ptr(button);diag_tiles_ext *ext=lv_obj_get_ext_attr(holder);
    if(lv_obj_get_free_num(button)){if((ext->page+1)*9<ext->count)++ext->page;}else if(ext->page)--ext->page;
    diagnostic_tiles_fill(holder);return LV_RES_OK;
}
static void diagnostic_tiles(lv_obj_t *win,u32 group,bool results)
{
    lv_win_ext_t *window=lv_obj_get_ext_attr(win);
    lv_obj_t *holder=lv_cont_create(win,NULL);diagnostic_last_holder=holder;lv_cont_set_fit(holder,false,false);lv_cont_set_layout(holder,LV_LAYOUT_OFF);lv_cont_set_style(holder,&lv_style_transp_tight);
    lv_obj_set_pos(holder,34,108);lv_obj_set_size(holder,1212,lv_obj_get_height(window->page)-122);
    diag_tiles_ext *ext=lv_obj_allocate_ext_attr(holder,sizeof(*ext));if(!ext){lv_obj_del(holder);return;}
    ext->count=ext->page=0;ext->results=results;
    for(u32 i=results?0:1;i<NYX_DIAG_PROBE_COUNT;++i){const nyx_diag_probe_t *probe=nyx_diag_probe(i);if(probe&&(results?probe->state!=NYX_DIAG_NOT_RUN:!!(probe->group&group)))ext->indices[ext->count++]=i;}
    ext->tiles=lv_cont_create(holder,NULL);lv_cont_set_fit(ext->tiles,false,false);lv_cont_set_layout(ext->tiles,LV_LAYOUT_OFF);lv_cont_set_style(ext->tiles,&lv_style_transp_tight);
    lv_obj_set_size(ext->tiles,1212,lv_obj_get_height(holder)-58);lv_obj_set_pos(ext->tiles,0,0);
    int y=lv_obj_get_height(holder)-46;
    ext->previous=button(holder,"Precedent",0,y,200,diagnostic_tiles_step,0);
    ext->next=button(holder,"Suivant",1012,y,200,diagnostic_tiles_step,1);
    lv_obj_set_height(ext->previous,44);lv_obj_set_height(ext->next,44);
    lv_obj_set_free_ptr(ext->previous,holder);lv_obj_set_free_ptr(ext->next,holder);
    ext->position=nyx_nova_text(holder,"",300,y+12,612,NOVA_SMALL);lv_label_set_align(ext->position,LV_LABEL_ALIGN_CENTER);
    diagnostic_tiles_fill(holder);
}

static lv_res_t show_components(lv_obj_t *button)
{
    (void)button;nyx_components_window(show_report,export_report);return LV_RES_OK;
}
static void show_results(int result)
{
    results_window = nyx_create_standard_window("Diagnostic - Derniere execution", close_results);
    lv_obj_t *full = lv_win_add_btn(results_window, NULL, "Rapport", show_report);
    lv_obj_set_free_num(full, NYX_DIAG_PROBE_COUNT);
    lv_win_add_btn(results_window, NULL, "Exporter", export_report);
    lv_win_add_btn(results_window, NULL, "Bilan par composant", show_components);
    u32 counts[6] = {0};
    for (u32 i = 0; i < NYX_DIAG_PROBE_COUNT; i++) {
        const nyx_diag_probe_t *p = nyx_diag_probe(i);
        if (p && p->state <= NYX_DIAG_ERROR) counts[p->state]++;
    }
    char summary[2048];
    s_printf(summary, nyx_tr("%d OK  /  %d avert.  /  %d defaut(s)  /  %d erreur(s)  /  %d observation(s)\n%s%s"),
        counts[NYX_DIAG_PASS], counts[NYX_DIAG_WARN], counts[NYX_DIAG_FAIL],
        counts[NYX_DIAG_ERROR], counts[NYX_DIAG_OBSERVATION],
        result ? nyx_tr("Execution ou restauration incomplete. ") : "",
        nyx_diag_truncated() ? nyx_tr("Rapport tronque : consultez le marqueur de fin.") :
        nyx_tr("Les observations sans verdict ne constituent pas un test reussi."));
    lv_obj_t *summary_label=text(results_window, summary, 34, 16, 1212, true);
    nyx_nova_fit_text(summary_label,80);
    diagnostic_tiles(results_window,0,true);
    if (!gui_diagnostics_inputs_ready())
        notice("Commandes tactiles", "La restauration du tactile a echoue.\nUtilisez les Joy-Con ou rechargez Nyx.");
}
/* Scoped diagnostic evidence, never a whole-chip health certificate. GPL-2.0-or-later. */
#ifndef NYX_COMPONENT_SUMMARY_H
#define NYX_COMPONENT_SUMMARY_H
enum {NYX_COMPONENT_NOT_TESTED,NYX_COMPONENT_PARTIAL,NYX_COMPONENT_PASS,NYX_COMPONENT_WARN,NYX_COMPONENT_ANOMALY,NYX_COMPONENT_TEST_ERROR};
typedef struct {
    const char *name,*scope,*hint;
    const char *keys[8];
    u8 probes[8];
    u8 key_count,required_count,probe_count;
    u8 storage; /* 0 none, 1 eMMC, 2 microSD. */
} nyx_component_definition;
typedef struct {
    const nyx_component_definition *definition;
    u32 state,found,required_found,executed;
    bool partial;
    char measurement[160];
} nyx_component_result;
enum {NYX_MANUAL_COMMANDS,NYX_MANUAL_TOUCH,NYX_MANUAL_DISPLAY,NYX_MANUAL_RAM,NYX_MANUAL_COUNT};
/* Last captured measurements in this Nyx session, separate from auto probes. */
void nyx_component_observation(u32 kind,const char *text,bool warning);
u32 nyx_component_count(void);
bool nyx_component_get(u32 index,nyx_component_result *out);
const char *nyx_component_state(u32 state);
char *nyx_component_summary_text(void); /* Owned allocation, includes justified leads. */
char *nyx_component_detail_text(u32 index); /* Owned allocation, evidence and scope. */
#endif

static const nyx_diag_finding_t component_fixture[]={
 {"charger_id",2,"I2C no reply"},{"usb_pd",2,"I2C no reply"},
 {"dram_sym",0,"2 channels"},{"plls",0,"locked"},{"fuel_devname",0,"0x00AC"}
};
u32 nyx_diag_finding_count(void){return sizeof(component_fixture)/sizeof(*component_fixture);}
bool nyx_diag_finding(u32 i,nyx_diag_finding_t *out){if(i>=nyx_diag_finding_count())return false;*out=component_fixture[i];return true;}
bool nyx_diag_storage_ready(bool emmc){(void)emmc;return true;}

static char manual_observations[NYX_MANUAL_COUNT][1024];
static bool manual_warnings[NYX_MANUAL_COUNT];
void nyx_component_observation(u32 kind,const char *text,bool warning)
{
    if(kind>=NYX_MANUAL_COUNT)return;
    nyx_diag_snprintf(manual_observations[kind],sizeof(manual_observations[kind]),"%s",text?text:"");
    manual_warnings[kind]=warning&&manual_observations[kind][0];
}
static int manual_kind(u32 component)
{return component==13?NYX_MANUAL_COMMANDS:component==12?NYX_MANUAL_TOUCH:component==11?NYX_MANUAL_DISPLAY:component==1?NYX_MANUAL_RAM:-1;}
static const nyx_component_definition components[]={
 {"Processeur / SoC","Identité SoC et démarrage du service CPU0. Aucun stress CPU.","Contrôler les alimentations et horloges CPU, puis les liaisons et les erreurs du service.",{"cpu_complex"},{1,27,28},1,1,3,0},
 {"Mémoire / horloges","Symétrie DRAM et verrouillage PLL. Le test RAM isolé est distinct.","Comparer les rails mémoire, l'identité DRAM et les horloges. Compléter par le service RAM isolé.",{"dram_sym","plls"},{14,15},2,2,2,0},
 {"eMMC","Santé déclarée, GPT, lectures et données logiques lorsqu'elles sont accessibles.","Vérifier alimentation, bus eMMC et contenu logique. Une GPT invalide ne prouve pas une panne de puce.",{"emmc_health","gpt","emmc_errors","cal0_hash","cal0_model","cal0_panel","fuses_pkg1"},{18,19,20,21,24},7,3,5,1},
 {"microSD","Initialisation et compteurs SD. Les reprises de lecture incluent SD et eMMC.","Comparer avec une carte connue fonctionnelle et examiner le lecteur. Les reprises communes ne désignent pas seules la microSD.",{"sd_errors"},{23,24},1,1,2,2},
 {"Alimentation / régulateurs","PMIC et rails mesurés, selon les puces présentes sur le modèle.","Contrôler tensions et liaisons I2C avant de conclure à une puce défectueuse.",{"soc_pmic_otp","pmic_rails","max77812","max77621_cpu","max77621_gpu"},{4,5,7},5,2,3,0},
 {"Contrôleur de charge","Identification, alimentation et défauts déclarés du chargeur.","Recouper chargeur, batterie et USB-C. Contrôler alimentation et bus commun avant remplacement.",{"charger_id","charger_pg","charger_fault"},{10},3,3,1,0},
 {"USB-C / Power Delivery","Réponse du contrôleur USB-PD. Le transfert USB complet n'est pas certifié.","Examiner connecteur, alimentation et liaison de contrôle USB-PD, puis recouper avec le chargeur.",{"usb_pd"},{11},1,1,1,0},
 {"Batterie / jauge","Santé déclarée, température et identification de la jauge.","Vérifier connecteur, sonde de température et mesures de la jauge avec la chaîne de charge.",{"batt_health","batt_ntc","fuel_devname"},{9},3,3,1,0},
 {"Wi-Fi","Énumération PCIe et lien avec le module. Le réseau complet n'est pas certifié.","Recouper PCIe, alimentation radio, horloges et service CPU0. Vérifier les prérequis avant de suspecter le module.",{"wifi_pcie"},{27,28},1,1,2,0},
 {"Bluetooth","Échange HCI avec la partie Bluetooth du module radio.","Contrôler alimentation radio, UART et liaisons communes Wi-Fi/Bluetooth.",{"bt_hci"},{26},1,1,1,0},
 {"Audio","Codec, horloges et essai sonore. L'écoute reste une observation manuelle.","Vérifier les rails et horloges du codec, puis les sorties et les connexions audio.",{"audio_codec","audio_clocks","audio_beep"},{29,30,31},3,3,3,0},
 {"Écran / rétroéclairage","Identification de dalle et contrôle du rétroéclairage. Les pixels restent à observer.","Comparer dalle, nappe et alimentation du rétroéclairage. Utiliser les mires pour l'observation des pixels.",{"dsi_id","backlight"},{32,33},2,2,2,0},
 {"Tactile","Identification et test électrique ITO. La grille manuelle est séparée.","Examiner nappe, alimentation et résultat ITO, puis la couverture manuelle de la grille.",{"touch_id","touch_ito"},{34},2,2,1,0},
 {"Commandes / Joy-Con","Inventaire matériel. Boutons, sticks et rails demandent le test interactif.","Compléter le test interactif : boutons, repos, amplitude et reconnexions. Vérifier les rails si des coupures sont observées.",{0},{36,37},0,0,2,0},
 {"Thermique / ventilation","Capteur thermique, températures et réponse du ventilateur.","Recouper températures, commande PWM, alimentation et réponse du ventilateur.",{"tmp451_id","soc_die_temp","pcb_temp","fan_stalled"},{12,13},4,4,2,0}
};

u32 nyx_component_count(void){return ARRAY_SIZE(components);}
const char *nyx_component_state(u32 state)
{
    static const char *names[]={"NON TESTÉ","PARTIEL","PASS — contrôles réussis","AVERTISSEMENT","ANOMALIE","ERREUR DE TEST"};
    return nyx_tr(names[state<ARRAY_SIZE(names)?state:0]);
}
static bool find(const char *key,nyx_diag_finding_t *out)
{
    for(u32 i=0;i<nyx_diag_finding_count();++i)if(nyx_diag_finding(i,out)&&!strcmp(out->key,key))return true;
    return false;
}
bool nyx_component_get(u32 index,nyx_component_result *out)
{
    if(!out||index>=nyx_component_count())return false;
    memset(out,0,sizeof(*out));const nyx_component_definition *d=&components[index];out->definition=d;
    bool error=false,probe_warning=false;u32 worst=0;bool measured=false;
    for(u32 i=0;i<d->probe_count;++i){
        const nyx_diag_probe_t *probe=nyx_diag_probe(d->probes[i]);
        if(probe&&probe->state!=NYX_DIAG_NOT_RUN)++out->executed;
        if(probe&&probe->state>=NYX_DIAG_ERROR)error=true;
        if(probe&&(probe->state==NYX_DIAG_WARN||probe->state==NYX_DIAG_FAIL)&&!(d->storage&&d->probes[i]==24))probe_warning=true;
    }
    for(u32 i=0;i<d->key_count;++i){
        nyx_diag_finding_t f;if(!find(d->keys[i],&f))continue;
        if(f.severity>2){error=true;continue;}
        ++out->found;if(i<d->required_count)++out->required_found;
        if(f.severity>worst||(f.severity==worst&&(!measured||f.detail[0]))){
            nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s : %s",f.key,f.detail[0]?f.detail:nyx_component_state(f.severity==0?NYX_COMPONENT_PASS:f.severity==1?NYX_COMPONENT_WARN:NYX_COMPONENT_ANOMALY));measured=true;
        }
        if(f.severity>worst)worst=f.severity;
    }
    bool ready=!d->storage||nyx_diag_storage_ready(d->storage==1);
    out->partial=!d->required_count||out->required_found<d->required_count||out->executed<d->probe_count||!ready;
    /* WARN may also encode an unavailable prerequisite. Keep coverage
     * explicitly partial instead of inferring English from translated text. */
    if(worst==1||probe_warning)out->partial=true;
    out->state=error?NYX_COMPONENT_TEST_ERROR:worst>=2?NYX_COMPONENT_ANOMALY:worst==1||probe_warning?NYX_COMPONENT_WARN:
        out->partial?(out->executed||out->found?NYX_COMPONENT_PARTIAL:NYX_COMPONENT_NOT_TESTED):NYX_COMPONENT_PASS;
    if(!measured){
        if(!d->required_count)nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s",nyx_tr("Contrôle interactif nécessaire."));
        else nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%d / %d résultats disponibles",out->required_found,d->required_count);
    }
    int manual=manual_kind(index);
    if(manual>=0&&manual_observations[manual][0]){
        if(out->state==NYX_COMPONENT_NOT_TESTED){out->state=NYX_COMPONENT_PARTIAL;out->partial=true;}
        if(manual_warnings[manual]&&out->state<NYX_COMPONENT_WARN){out->state=NYX_COMPONENT_WARN;out->partial=true;}
        if(!d->required_count)nyx_diag_snprintf(out->measurement,sizeof(out->measurement),"%s",nyx_tr("Mesures interactives enregistrées. Vérifier leur couverture dans le détail."));
    }
    return true;
}

#define SUMMARY_CAPACITY 32768u
static void add(char *buffer,const char *text)
{
    u32 used=strlen(buffer);if(used<SUMMARY_CAPACITY-1)nyx_diag_snprintf(buffer+used,SUMMARY_CAPACITY-used,"%s",text);
}
static void line(char *buffer,const char *text){add(buffer,nyx_tr(text));add(buffer,"\n");}
static void evidence(char *buffer,const nyx_component_result *result)
{
    const nyx_component_definition *d=result->definition;char row[512];
    nyx_diag_snprintf(row,sizeof(row),"%s — %s\n",nyx_tr(d->name),nyx_component_state(result->state));add(buffer,row);
    line(buffer,d->scope);line(buffer,result->measurement);
    if(result->partial)line(buffer,"Couverture partielle ou prérequis non vérifiés.");
    int kind=manual_kind(d-components);
    if(kind>=0&&manual_observations[kind][0]){
        line(buffer,"Dernier contrôle complémentaire de cette session, distinct des mesures automatiques :");
        add(buffer,manual_observations[kind]);add(buffer,"\n");
    }
}
char *nyx_component_summary_text(void)
{
    char *buffer=calloc(1,SUMMARY_CAPACITY);if(!buffer)return NULL;
    add(buffer,"\n\n");
    line(buffer,"Bilan par composant");
    line(buffer,"PASS concerne uniquement les contrôles réalisés. Il ne certifie pas toutes les fonctions d'une puce.");
    line(buffer,"Le bilan normal n'exécute pas les motifs du test RAM isolé.");
    if(nyx_diag_truncated())line(buffer,"Le rapport détaillé est tronqué. Les preuves disponibles et les contrôles non testés restent identifiés.");
    for(int state=NYX_COMPONENT_TEST_ERROR;state>=0;--state)for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result r;nyx_component_get(i,&r);if(r.state!=(u32)state)continue;evidence(buffer,&r);add(buffer,"\n");
    }
    line(buffer,"Pistes de panne");bool anomaly=false;
    nyx_component_result charge,pd,cpu,wifi;
    nyx_component_get(5,&charge);nyx_component_get(6,&pd);nyx_component_get(0,&cpu);nyx_component_get(8,&wifi);
    if(charge.state>=NYX_COMPONENT_ANOMALY&&pd.state>=NYX_COMPONENT_ANOMALY)
        line(buffer,"USB-PD et chargeur présentent des anomalies : vérifier leurs alimentations et liaisons de contrôle communes avant de conclure à deux puces défectueuses.");
    if(cpu.state>=NYX_COMPONENT_ANOMALY&&wifi.state>=NYX_COMPONENT_ANOMALY)
        line(buffer,"Le diagnostic Wi-Fi dépend du service CPU0. Son échec peut empêcher le contrôle radio ; le résultat Wi-Fi ne localise donc pas seul la panne.");
    for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result r;nyx_component_get(i,&r);if(r.state<NYX_COMPONENT_WARN)continue;
        anomaly=true;line(buffer,r.definition->name);line(buffer,r.measurement);line(buffer,r.definition->hint);add(buffer,"\n");
    }
    if(!anomaly)line(buffer,"Aucune anomalie structurée relevée. Compléter les contrôles partiels et les observations manuelles.");
    line(buffer,"Une absence de réponse peut venir de l'alimentation ou d'une liaison. Ces pistes ne prescrivent pas le remplacement d'une puce.");
    return buffer;
}
char *nyx_component_detail_text(u32 index)
{
    nyx_component_result r;if(!nyx_component_get(index,&r))return NULL;
    char *buffer=calloc(1,SUMMARY_CAPACITY);if(!buffer)return NULL;evidence(buffer,&r);
    line(buffer,"Preuves enregistrées");
    for(u32 i=0;i<r.definition->key_count;++i){
        char row[512];nyx_diag_finding_t f;
        if(find(r.definition->keys[i],&f))nyx_diag_snprintf(row,sizeof(row),"%s : %s — %s\n",f.key,nyx_component_state(f.severity==0?NYX_COMPONENT_PASS:f.severity==1?NYX_COMPONENT_WARN:NYX_COMPONENT_ANOMALY),f.detail);
        else nyx_diag_snprintf(row,sizeof(row),"%s : %s\n",r.definition->keys[i],nyx_component_state(NYX_COMPONENT_NOT_TESTED));
        add(buffer,row);
    }
    if(r.state>=NYX_COMPONENT_WARN){line(buffer,"Pistes de panne");line(buffer,r.definition->hint);}
    line(buffer,"Les rapports détaillés d'origine restent disponibles sans relancer les tests.");
    return buffer;
}

static lv_action_t component_report,component_export;
static lv_obj_t *component_holder,*component_counter,*component_previous,*component_next;
static u32 component_page;
static lv_style_t component_styles[4];

static lv_res_t component_leads(lv_obj_t *button)
{
    (void)button;char *summary=nyx_component_summary_text();
    if(!summary){nyx_show_help("Memoire insuffisante.");return LV_RES_OK;}
    lv_obj_t *win=nyx_create_standard_window("Bilan et pistes de panne",NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *report=nyx_nova_report(page,summary,34,18,1212,lv_obj_get_height(page)-36);
    nyx_nova_report_style(report,nyx_nova_text_style(NOVA_SMALL));free(summary);return LV_RES_OK;
}
static lv_res_t component_detail(lv_obj_t *button)
{
    u32 index=lv_obj_get_free_num(button);nyx_component_result result;
    if(!nyx_component_get(index,&result))return LV_RES_OK;
    char *summary=nyx_component_detail_text(index);if(!summary){nyx_show_help("Memoire insuffisante.");return LV_RES_OK;}
    lv_obj_t *win=nyx_create_standard_window(result.definition->name,NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *tabs=lv_tabview_create(page,NULL);lv_obj_set_size(tabs,1280,lv_obj_get_height(page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *facts=lv_tabview_add_tab(tabs,"Résultat et preuves");nyx_nova_fixed_page(facts);
    lv_obj_t *probes=lv_tabview_add_tab(tabs,"Rapports détaillés");nyx_nova_fixed_page(probes);
    lv_obj_t *report=nyx_nova_report(facts,summary,34,14,1212,lv_obj_get_height(facts)-28);
    nyx_nova_report_style(report,nyx_nova_text_style(NOVA_SMALL));free(summary);
    for(u32 i=0;i<result.definition->probe_count;++i){
        u32 probe_index=result.definition->probes[i];const nyx_diag_probe_t *probe=nyx_diag_probe(probe_index);
        if(!probe)continue;
        lv_obj_t *open=nyx_nova_action(probes,nyx_diag_probe_caption(probe_index),"Rapport enregistré", "file-text",component_report,probe_index,34+(i%3)*410,18+(i/3)*138,392,124);
        if(probe->state==NYX_DIAG_NOT_RUN||!probe->report_length){lv_obj_set_click(open,false);lv_btn_set_state(open,LV_BTN_STATE_INA);}
    }
    return LV_RES_OK;
}
static void component_fill(void)
{
    lv_obj_clean(component_holder);u32 ordered[32],count=0;
    for(int state=NYX_COMPONENT_TEST_ERROR;state>=0;--state)for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result result;nyx_component_get(i,&result);if(result.state==(u32)state&&count<32)ordered[count++]=i;
    }
    for(u32 slot=0;slot<4&&component_page*4+slot<count;++slot){
        u32 index=ordered[component_page*4+slot];nyx_component_result result;nyx_component_get(index,&result);
        char detail[512];s_printf(detail,"%s%s\n%s",nyx_component_state(result.state),result.partial?nyx_tr(" · couverture partielle"):"",result.measurement);
        lv_obj_t *open=nyx_nova_action(component_holder,result.definition->name,detail,"activity",component_detail,index,(slot%2)*618,(slot/2)*184,594,168);
        lv_style_copy(&component_styles[slot],lv_cont_get_style(lv_obj_get_parent(open)));
        u32 color=result.state>=NYX_COMPONENT_ANOMALY?0xFF6B6B:result.state==NYX_COMPONENT_WARN?0xF4B85F:result.state==NYX_COMPONENT_PASS?0x8AE6B6:0x667386;
        component_styles[slot].body.border.color=LV_COLOR_HEX(color);lv_cont_set_style(lv_obj_get_parent(open),&component_styles[slot]);
    }
    char position[64];s_printf(position,nyx_tr("Page %d / %d"),component_page+1,(count+3)/4);lv_label_set_text(component_counter,position);
    bool previous=component_page>0,next=(component_page+1)*4<count;
    lv_obj_set_click(component_previous,previous);lv_btn_set_state(component_previous,previous?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    lv_obj_set_click(component_next,next);lv_btn_set_state(component_next,next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t component_step(lv_obj_t *button)
{
    if(lv_obj_get_free_num(button)){if((component_page+1)*4<nyx_component_count())++component_page;}
    else if(component_page)--component_page;
    component_fill();return LV_RES_OK;
}
void nyx_components_window(lv_action_t report,lv_action_t export_action)
{
    component_report=report;component_export=export_action;component_page=0;
    lv_obj_t *win=nyx_create_standard_window("Bilan par composant",NULL);
    lv_obj_t *full=lv_win_add_btn(win,NULL,"Rapport",component_report);lv_obj_set_free_num(full,NYX_DIAG_PROBE_COUNT);
    lv_win_add_btn(win,NULL,"Exporter",component_export);lv_win_add_btn(win,NULL,"Pistes",component_leads);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    const char *guidance=nyx_diag_truncated()?"Rapport détaillé tronqué. PASS concerne uniquement les contrôles réellement réalisés.":"PASS : contrôles réalisés uniquement. Les contrôles partiels et le stress RAM restent distincts.";
    lv_obj_t *hint=nyx_nova_text(page,guidance,34,18,1212,NOVA_SMALL);nyx_nova_fit_text(hint,54);
    component_holder=lv_cont_create(page,NULL);lv_cont_set_fit(component_holder,false,false);lv_cont_set_layout(component_holder,LV_LAYOUT_OFF);lv_cont_set_style(component_holder,&lv_style_transp_tight);
    lv_obj_set_pos(component_holder,34,84);lv_obj_set_size(component_holder,1212,352);
    component_previous=lv_btn_create(page,NULL);lv_btn_set_fit(component_previous,false,false);lv_obj_set_size(component_previous,220,44);lv_obj_set_pos(component_previous,34,454);lv_obj_set_free_num(component_previous,0);lv_btn_set_action(component_previous,LV_BTN_ACTION_CLICK,component_step);
    lv_obj_t *label=lv_label_create(component_previous,NULL);lv_label_set_text(label,"Précédent");
    component_next=lv_btn_create(page,component_previous);lv_obj_set_pos(component_next,1026,454);lv_obj_set_free_num(component_next,1);label=lv_label_create(component_next,NULL);lv_label_set_text(label,"Suivant");
    lv_btn_set_action(component_next,LV_BTN_ACTION_CLICK,component_step);
    component_counter=nyx_nova_text(page,"",300,462,680,NOVA_SMALL);lv_label_set_align(component_counter,LV_LABEL_ALIGN_CENTER);component_fill();
}

static void preview_component_results(void){
 for(u32 i=0;i<42;++i)preview_probes[i].state=NYX_DIAG_OBSERVATION;
 preview_probes[10].state=preview_probes[11].state=NYX_DIAG_FAIL;
 preview_probes[14].state=preview_probes[15].state=NYX_DIAG_PASS;
 nyx_components_window(show_report,export_report);
}
static const char *ram_state(u32 state)
{
    if (state == NYX_RAM_RESULT_PASS) return nyx_tr("OK");
    if (state == NYX_RAM_RESULT_FAIL) return nyx_tr("DEFAUT");
    if (state == NYX_RAM_RESULT_ABORT) return nyx_tr("INTERROMPU");
    return nyx_tr("ERREUR");
}

static u32 ram_selected_mode=NYX_RAM_MODE_ADVANCED,ram_custom_mask=NYX_RAM_PATTERN_ALL;
static lv_obj_t *ram_mode_choices,*ram_pattern_choices,*ram_start;

static void ram_controls_refresh(void)
{
    bool custom=ram_selected_mode==NYX_RAM_MODE_CUSTOM;
    for(lv_obj_t *choice=lv_obj_get_child(ram_pattern_choices,NULL);choice;choice=lv_obj_get_child(ram_pattern_choices,choice)){
        u32 index=lv_obj_get_free_num(choice);bool selected=custom?!!(ram_custom_mask&(1u<<index)):ram_selected_mode==NYX_RAM_MODE_ADVANCED||index==0;
        lv_obj_set_click(choice,custom);lv_btn_set_state(choice,selected?LV_BTN_STATE_TGL_REL:LV_BTN_STATE_REL);
    }
    bool valid=!custom||ram_custom_mask;
    lv_obj_set_click(ram_start,valid);lv_btn_set_state(ram_start,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t ram_mode_choice(lv_obj_t *choice)
{
    ram_selected_mode=lv_obj_get_free_num(choice);ram_controls_refresh();return LV_RES_OK;
}
static lv_res_t ram_pattern_choice(lv_obj_t *choice)
{
    ram_custom_mask^=1u<<lv_obj_get_free_num(choice);ram_controls_refresh();return LV_RES_OK;
}
static lv_res_t show_ram_service(lv_obj_t *btn)
{
    (void)btn;
    lv_obj_t *win=nyx_create_standard_window("Test RAM isole",NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    ram_start=lv_win_add_btn(win,NULL,"Lancer le test RAM",launch_ram);
    ram_mode_choices=nyx_choices_create(page,"Rapide\nAvancé\nPersonnalisé",ram_selected_mode,3,1212,48,ram_mode_choice,80,false);
    lv_obj_set_pos(ram_mode_choices,34,18);
    lv_obj_t *label=nyx_nova_text(page,"Rapide : adresse. Avancé : 7 motifs. Personnalisé : choisir au moins un motif.",34,82,1212,NOVA_SMALL);nyx_nova_fit_text(label,48);
    ram_pattern_choices=nyx_choices_create(page,"Adresse\nAdresse inversée\nZéros\nUns\nAlternance AA\nAlternance 55\nPseudo-aléatoire",0,4,1212,54,ram_pattern_choice,81,false);
    lv_obj_set_pos(ram_pattern_choices,34,142);
    label=nyx_nova_text(page,"Programme séparé puis retour automatique à Hekate. Les zones réservées au retour restent exclues. Un matériel instable peut empêcher le retour.",34,278,1212,NOVA_SMALL);nyx_nova_fit_text(label,64);
    if(last_ram_valid){
        char message[1024];
        const char *mode=last_ram.mode==NYX_RAM_MODE_QUICK?"Rapide":last_ram.mode==NYX_RAM_MODE_ADVANCED?"Avancé":"Personnalisé";
        s_printf(message,"%s · %s · %s\n%d / %d motifs · zone prévue %d / %d MiB · %d s · %d erreur(s)\nMotifs choisis : %02X · terminés : %02X · MTC : %08X\nZone relue avec au moins un motif : %d MiB",
            nyx_tr("Dernier resultat"),ram_state(last_ram.result),nyx_tr(mode),last_ram.patterns_completed,last_ram.patterns_total,last_ram.tested_mib,last_ram.dram_mib,last_ram.elapsed_ms/1000,last_ram.errors,last_ram.selected_mask,last_ram.completed_mask,last_ram.mtc_errors,last_ram.verified_mib);
        label=nyx_nova_text(page,message,34,362,1212,NOVA_SMALL);nyx_nova_fit_text(label,126);
    }
    ram_controls_refresh();return LV_RES_OK;
}
static lv_res_t preview_show_group(lv_obj_t *btn)
{
    u32 index = lv_obj_get_free_num(btn);
    if (index >= ARRAY_SIZE(groups)) return LV_RES_OK;
    if (!groups[index].groups) return show_ram_service(btn);
    if (!index) {
        lv_obj_set_free_num(btn, NYX_DIAG_G_ALL);
        lv_res_t result = run_selection(btn);
        lv_obj_set_free_num(btn, 0);
        return result;
    }
    lv_obj_t *win = nyx_create_standard_window(groups[index].title, NULL);
    if(index==9){lv_win_add_btn(win,NULL,"Joy-Con interactif",nyx_joycon_test);lv_win_add_btn(win,NULL,"Grille tactile",nyx_touch_test);}
    if(index==8)lv_win_add_btn(win,NULL,"Mires écran",nyx_screen_patterns);
    button(win, "Executer ce groupe", 32, 20, 416, run_selection, groups[index].groups);
    text(win, groups[index].groups == NYX_DIAG_G_SOUND ?
        "Un son est emis sur chaque sortie. Verifiez le resultat a l'ecoute." :
        groups[index].detail, 480, 32, 720, true);
    diagnostic_tiles(win,groups[index].groups,false);
    return LV_RES_OK;
}
void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent)
{
    (void)th;
    nyx_nova_grid_begin(parent, NOVA_DIAGNOSTICS);
    for (u32 i = 0; i < ARRAY_SIZE(groups); i++)
        nyx_nova_grid_add(parent, groups[i].detail, show_group, NULL, i);
    nyx_nova_grid_finish(parent);
}

ACTION(_create_window_home_launch) ACTION(_create_mbox_payloads) ACTION(create_win_emummc_tools) ACTION(_create_mbox_reload) ACTION(_create_mbox_reboot) ACTION(_create_mbox_poweroff)
/* Both native home layouts use the Nova components and the original actions. */
enum { HOME_ACTION_BOOT, HOME_ACTION_CONFIGS, HOME_ACTION_PAYLOADS,
    HOME_ACTION_EMUMMC, HOME_ACTION_DIAGNOSTIC, HOME_ACTION_COUNT };
static lv_obj_t *home_tabview;
static lv_res_t _home_modern_activate(lv_obj_t *btn)
{
	switch (lv_obj_get_free_num(btn))
	{
	case HOME_ACTION_BOOT:
	case HOME_ACTION_CONFIGS:
		return _create_window_home_launch(btn);
	case HOME_ACTION_PAYLOADS:
		return _create_mbox_payloads(btn);
	case HOME_ACTION_EMUMMC:
		return create_win_emummc_tools(btn);
	case HOME_ACTION_DIAGNOSTIC:
		if (home_tabview)
		{
			lv_tabview_set_tab_act(home_tabview, 5, false);
			nyx_nova_navigation_select(5);
		}
		break;
	}
	return LV_RES_OK;
}

static void _create_tab_home(lv_theme_t *theme, lv_obj_t *parent)
{
    (void)theme;
    if (n_cfg.home_layout == HOME_LAYOUT_CAROUSEL) nyx_nova_carousel(parent, _home_modern_activate);
    else nyx_nova_home(parent, _home_modern_activate);
}

static lv_res_t _create_nova_quick_actions(lv_obj_t *button)
{
    (void)button;
    lv_obj_t *win = nyx_create_standard_window("hekate", NULL);
    const char *titles[] = {"Reglages Nyx", "Recharger", "Redemarrer", "Eteindre"};
    const char *icons[] = {"settings-2", "refresh-cw", "rotate-ccw", "power"};
    lv_action_t actions[] = {create_win_nyx_options, _create_mbox_reload, _create_mbox_reboot, _create_mbox_poweroff};
    for (u32 i = 0; i < 4; ++i) {
        lv_obj_t *panel = nyx_nova_panel(win, 34 + i % 2 * 614, 28 + i / 2 * 204, 598, 188);
        nyx_nova_icon_box(panel, icons[i], 28, 28, 48, false);
        lv_obj_t *action = lv_btn_create(panel, NULL); lv_btn_set_fit(action, false, false); lv_btn_set_layout(action, LV_LAYOUT_OFF);
        lv_obj_set_size(action, 480, 82); lv_obj_set_pos(action, 94, 52); lv_btn_set_action(action, LV_BTN_ACTION_CLICK, actions[i]);
        lv_obj_t *label = nyx_nova_text(action, titles[i], 20, 24, 440, NOVA_TITLE); nyx_nova_fit_text(label, 40);
    }
    return LV_RES_OK;
}


static lv_obj_t *preview_emmc_tabs,*preview_report_panel;
static lv_obj_t *ancestor_type(lv_obj_t *object,const char *name){for(;object;object=lv_obj_get_parent(object)){lv_obj_type_t type;lv_obj_get_type(object,&type);if(type.type[0]&&!strcmp(type.type[0],name))return object;}return NULL;}
static lv_res_t preview_emmc_info(void){
 lv_obj_t *win=nyx_create_standard_window("Internal eMMC Info",NULL);
 const char *values="\nSK Hynix (90)\nhC8aP> (4A)\n1.0\n1034B3FE\n04/2021\n\n#00DDFF V1.2 (rev 1.8)#\n8F5\nHS400\n400 MB/s (200 MHz)\n29816 MiB\n2 MiB\n\nOff: OK\nA: -, B: 100%\nNormal (< 80%)";
 lv_obj_t *partitions=nyx_nova_emmc_summary(win,values);preview_emmc_tabs=ancestor_type(partitions,"lv_tabview");assert(preview_emmc_tabs);
 char table[10000];strcpy(table,"#00DDFF eMMC Physical Partitions:#\n1: #96FF00 BOOT0#  Size: 4096 KiB\n2: #96FF00 BOOT1#  Size: 4096 KiB\n3: #96FF00 RPMB#   Size: 4096 KiB\n\n#00DDFF GPP (eMMC USER) Partition Table:#\n");
 for(u32 i=0;i<48;++i){char row[160];s_printf(row,"%02d: #96FF00 Partition-%02d#   %6d MiB   %08Xh   %08Xh\n",i,i,1024+i,i*0x10000,0x10000);strcat(table,row);}
 preview_report_panel=nyx_nova_report(partitions,table,34,18,1212,lv_obj_get_height(partitions)-36);return LV_RES_OK;
}
static lv_res_t preview_change_close(lv_obj_t *button){free(emummc_img->dirlist);free(emummc_img);emummc_img=NULL;return nyx_win_close_action(button);}
static lv_res_t preview_change_emu(u32 count){
 lv_obj_t *win=nyx_create_standard_window("Change emuMMC",preview_change_close);emummc_img=calloc(1,sizeof(*emummc_img));assert(emummc_img);emummc_img->win=win;
 emummc_img->dirlist=calloc(1,sizeof(dirlist_t));assert(emummc_img->dirlist);if(count>63)count=63;
 for(u32 i=0;i<count;++i){emummc_img->dirlist->name[i]=emummc_img->dirlist->data+i*256;s_printf(emummc_img->dirlist->name[i],"Image %02d - Español Русский",i);}
 for(u32 i=0;i<3;++i){emummc_img->part_sector[i]=0x100000*(i+1);emummc_img->part_type[i]=0xE0;s_printf(&emummc_img->part_path[128*i],"emuMMC/RAW%d",i+1);}
 if(getenv("NYX_RAW_DISABLED"))emummc_img->part_type[1]=0x83;
 _create_change_emummc_ui(win,count);return LV_RES_OK;
}
static lv_obj_t *preview_mbr_info(void){
 lv_obj_t *background=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_size(background,1280,720);lv_obj_set_style(background,&mbox_darken);
 lv_obj_t *box=lv_mbox_create(background,NULL);lv_obj_set_width(box,852);lv_mbox_set_recolor_text(box,true);
 lv_mbox_set_text(box,"#FFDD00 The SD Card cannot be backed up automatically!#\n#FFDD00 Any other partition will be also wiped!#\n\nYou will be asked to back up your files later via UMS.");
 lv_obj_t *holder=lv_cont_create(box,NULL);lv_cont_set_style(holder,&lv_style_transp_tight);lv_cont_set_fit(holder,false,true);lv_obj_set_width(holder,552);
 lv_obj_t *label=nyx_nova_text(holder,"Current MBR partition layout:",0,0,552,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);
 lv_obj_t *bar=lv_bar_create(holder,NULL);lv_obj_set_size(bar,552,33);lv_obj_set_pos(bar,0,40);lv_bar_set_value(bar,100);
 label=lv_label_create(holder,NULL);lv_label_set_style(label,&monospace_text);lv_label_set_long_mode(label,LV_LABEL_LONG_BREAK);lv_obj_set_width(label,552);lv_obj_set_pos(label,0,100);
 lv_label_set_text(label,"Partition 0 - Type: 0c, Start: 00008000, Size: 74700000\nPartition 1 - Type: e0, Start: 74708000, Size: 02000000\nPartition 2 - Type: 83, Start: 76708000, Size: 00010000\nPartition 3 - Type: ee, Start: 76718000, Size: 00010000");
 static const char *buttons[]={"\251","\222OK","\251",""};lv_mbox_add_btns(box,buttons,nyx_mbox_action);lv_obj_align(box,NULL,LV_ALIGN_CENTER,0,0);return box;
}
static managed_font legacy_mono;
static void verify_mono_pixels(void){
 managed_font normalized={0};assert(scale_font(&ubuntu_mono,20,&normalized,false,true));
 for(u32 cp=33;cp<127;++cp){u32 width=lv_font_get_real_width(&ubuntu_mono,cp),bpp=lv_font_get_bpp(&ubuntu_mono,cp),stride=bpp==8?width:(width+1)/2;const u8 *original=lv_font_get_bitmap(&ubuntu_mono,cp),*actual=get_bitmap(&normalized.font,cp);assert(width<=10&&actual&&original);
  for(u32 y=0;y<20;++y)for(u32 x=0;x<10;++x){u32 expected=0;if(x>=(10-width)/2&&x<(10-width)/2+width){u32 sx=x-(10-width)/2;u8 value=original[y*stride+(bpp==8?sx:sx/2)];expected=bpp==8?(value+8)/17:(sx&1?value&15:value>>4);}
   u8 packed=actual[y*5+x/2];u32 decoded=x&1?packed&15:packed>>4;assert(decoded==expected);}
 }
 for(u32 cp=0x410;cp<=0x44F;++cp){const u8 *bitmap=get_bitmap(&normalized.font,cp);assert(bitmap);bool nonzero=false;for(u32 i=0;i<100;++i)if(bitmap[i])nonzero=true;assert(nonzero);}
 release_font(&normalized);
}
static lv_res_t preview_font_comparison(void){
 verify_mono_pixels();lv_obj_t *win=nyx_create_standard_window("Police technique",NULL);lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;
 assert(legacy_scale_font(&ubuntu_mono,nyx_font_mono()->h_px,&legacy_mono,false,true));
 lv_obj_t *left=nyx_nova_panel(page,34,18,594,height),*right=nyx_nova_panel(page,652,18,594,height);
 nyx_nova_text(left,"Avant",24,24,546,NOVA_TITLE);nyx_nova_text(right,"Après",24,24,546,NOVA_TITLE);
 static lv_style_t legacy;lv_style_copy(&legacy,&monospace_text);legacy.text.font=&legacy_mono.font;
 const char *sample="eMMC Physical Partitions:\nBOOT0  4096 KiB  0x00000000\nBOOT1  4096 KiB  0x00002000\nGPP Partition Table:\n00: PRODINFO    0x00008000\n01: PRODINFOF   0x00010000\nSD backup: 0123456789ABCDEF\nÉcriture vérifiée · Русский";
 lv_obj_t *a=lv_label_create(left,NULL),*b=lv_label_create(right,NULL);lv_label_set_style(a,&legacy);lv_label_set_style(b,&monospace_text);lv_label_set_long_mode(a,LV_LABEL_LONG_BREAK);lv_label_set_long_mode(b,LV_LABEL_LONG_BREAK);lv_obj_set_width(a,546);lv_obj_set_width(b,546);lv_obj_set_pos(a,24,94);lv_obj_set_pos(b,24,94);lv_label_set_text(a,sample);lv_label_set_text(b,sample);return LV_RES_OK;
}
static int nonzero(const uint8_t *p, unsigned n)
{ unsigned v=0; while(n--) v |= *p++; return v!=0; }
static void observe(nova_bt_pad *p,uint32_t now)
{if(!p->reports)p->first_report_ms=now;p->buttons_seen|=p->buttons;p->reports++;}
int nova_bt_validate_peers(const nova_bt_peer p[2])
{
    int count=0;
    for(unsigned i=0;i<2;i++) if(p[i].valid) {
        if(!nonzero(p[i].mac,6)||!nonzero(p[i].host,6)||!nonzero(p[i].key,16)) return 0;
        if((p[i].mac[0]&1)||(p[i].host[0]&1)) return 0;
        count++;
    }
    if(count==2 && (memcmp(p[0].host,p[1].host,6)||!memcmp(p[0].mac,p[1].mac,6))) return 0;
    return count;
}
static uint32_t pairing_crc(const uint8_t *p,unsigned n)
{
    uint32_t crc=0xffffffffu;
    while(n--){crc^=*p++;for(unsigned b=0;b<8;b++)crc=(crc>>1)^(0xedb88320u & (0u-(crc&1)));}
    return ~crc;
}
int nova_bt_pairing_encode(uint8_t out[NOVA_BT_PAIRING_SIZE],const nova_bt_peer p[2])
{
    memset(out,0,NOVA_BT_PAIRING_SIZE);int count=nova_bt_validate_peers(p);if(!count)return 0;
    memcpy(out,"NJOYPAIR",8);out[8]=1;
    for(unsigned i=0;i<2;i++)if(p[i].valid){
        out[9]|=1u<<i;uint8_t *at=out+12+i*28;
        memcpy(at,p[i].mac,6);memcpy(at+6,p[i].host,6);memcpy(at+12,p[i].key,16);
    }
    uint32_t crc=pairing_crc(out,68);for(unsigned i=0;i<4;i++)out[68+i]=crc>>(i*8);
    return count;
}
int nova_bt_pairing_decode(nova_bt_peer p[2],const uint8_t *in,unsigned size)
{
    memset(p,0,2*sizeof(*p));
    if(size!=NOVA_BT_PAIRING_SIZE||memcmp(in,"NJOYPAIR",8)||in[8]!=1||!in[9]||(in[9]&~3)||in[10]||in[11])return 0;
    uint32_t crc=0;for(unsigned i=0;i<4;i++)crc|=(uint32_t)in[68+i]<<(i*8);
    if(crc!=pairing_crc(in,68))return 0;
    for(unsigned i=0;i<2;i++){
        const uint8_t *at=in+12+i*28;
        if(in[9]&(1u<<i)){p[i].valid=1;memcpy(p[i].mac,at,6);memcpy(p[i].host,at+6,6);memcpy(p[i].key,at+12,16);}
        else if(nonzero(at,28)){memset(p,0,2*sizeof(*p));return 0;}
    }
    int count=nova_bt_validate_peers(p);if(!count)memset(p,0,2*sizeof(*p));return count;
}
int nova_bt_decode(nova_bt_pad *p, unsigned side, const uint8_t *r, unsigned n, uint32_t now)
{
    /* BTstack delivers HID DATA header 0xa1 followed by the report ID. */
    if(n && r[0]==0xa1){r++;n--;}
    if(side>1||!n) return 0;
    if(r[0]==0x3f){
        if(n<12||r[3]>8)return 0;
        /* Simple Joy-Con reports, held vertically as a pair. The hat and
         * face-button orientations differ between left and right halves.
         * Protocol reference: SDL's combined simple Joy-Con handlers. */
        static const uint32_t face[2][6]={{1u<<19,1u<<16,1u<<17,1u<<18,1u<<21,1u<<20},
            {1u<<3,1u<<1,1u<<2,1u<<0,1u<<4,1u<<5}};
        uint32_t buttons=0;
        for(unsigned bit=0;bit<6;bit++)if(r[1]&(1u<<bit))buttons|=face[side][bit];
        if(!side){if(r[2]&1)buttons|=1u<<8;if(r[2]&4)buttons|=1u<<11;if(r[2]&32)buttons|=1u<<13;}
        else{if(r[2]&2)buttons|=1u<<9;if(r[2]&8)buttons|=1u<<10;if(r[2]&16)buttons|=1u<<12;}
        if(r[2]&64)buttons|=1u<<(side?6:22);
        if(r[2]&128)buttons|=1u<<(side?7:23);
        static const int8_t x[9]={1,1,0,-1,-1,-1,0,1,0};
        static const int8_t y[9]={0,-1,-1,-1,0,1,1,1,0};
        int sign=side?-1:1;
        p->x=2048+sign*x[r[3]]*1024;p->y=2048+sign*y[r[3]]*1024;
        p->buttons=buttons;p->updated=now;p->connected=1;p->report_id=0x3f;observe(p,now);
        return 1;
    }
    if(n<13||(r[0]!=0x30&&r[0]!=0x21)) return 0;
    uint32_t buttons=r[3]|((uint32_t)r[4]<<8)|((uint32_t)r[5]<<16);
    p->buttons=buttons & (side?0x0016ffu:0xff2900u);
    unsigned at=side?9:6;
    p->x=r[at]|((r[at+1]&15)<<8);p->y=(r[at+1]>>4)|(r[at+2]<<4);
    p->battery=(r[2]>>4)&0xf;p->updated=now;p->connected=1;p->report_id=r[0];observe(p,now);
    if(r[0]==0x30||(r[0]==0x21&&n>=16&&(r[13]&0x80)&&r[14]==3&&r[15]==0x30))p->mode_ack=1;
    return 1;
}
jc_gamepad_rpt_t *gui_bluetooth_merge(jc_gamepad_rpt_t *wired)
{
    static jc_gamepad_rpt_t merged;
    const nova_bt_status *s=nova_bt_get_status();
    if(!s->pad[0].connected&&!s->pad[1].connected)return wired;
    if(wired)memcpy(&merged,wired,sizeof(merged));else{
        memset(&merged,0,sizeof(merged));merged.lstick_x=merged.lstick_y=merged.rstick_x=merged.rstick_y=0x800;
    }
    for(unsigned i=0;i<2;i++){
        const nova_bt_pad *p=&s->pad[i];
        if(!p->connected||(u32)(get_tmr_ms()-p->updated)>500)continue;
        if(i?!merged.conn_r:!merged.conn_l){
            u32 mask=i?0x0016ff:0xff2900;
            merged.buttons=(merged.buttons&~mask)|p->buttons;
            if(i){merged.conn_r=true;merged.rstick_x=p->x;merged.rstick_y=p->y;merged.batt_info_r=p->battery;}
            else{merged.conn_l=true;merged.lstick_x=p->x;merged.lstick_y=p->y;merged.batt_info_l=p->battery;}
        }
    }
    return &merged;
}
typedef struct _jc_lv_driver_t
{
	lv_indev_t *indev_jc;
	lv_indev_t *indev_touch;
// LV_INDEV_READ_PERIOD * JC_CAL_MAX_STEPS = 264 ms.
#define JC_CAL_MAX_STEPS 8
	u32 calibration_step;
	u16 cx_max;
	u16 cx_min;
	u16 cy_max;
	u16 cy_min;
	s16 pos_x;
	s16 pos_y;
	s16 pos_last_x;
	s16 pos_last_y;
	lv_obj_t *cursor;
	u32 cursor_timeout;
	bool cursor_hidden;
	u32 console_timeout;
} jc_lv_driver_t;

static jc_lv_driver_t jc_drv_ctx;


static bool diagnostics_suspended,console_enabled;
static jc_gamepad_rpt_t rail_fixture;
static bool gui_bluetooth_active(void){return bt_fixture.state==NOVA_BT_READY;}
static jc_gamepad_rpt_t *joycon_poll(void){return &rail_fixture;}
static void _save_fb_to_bmp(void){}
static void display_window_d_console_enable(void){}
static void display_window_d_console_disable(void){}
static struct {int savedx,savedy,savedcol;u32 fntsz;} gfx_con;
static void gfx_con_getpos(int*x,int*y,int*c){*x=*y=*c=0;}
static void gfx_con_setpos(int x,int y,int c){(void)x;(void)y;(void)c;}
static void gfx_printf(const char *fmt,...){(void)fmt;}
#define GFX_COL_AUTO 0
static bool _jc_virt_mouse_read(lv_indev_data_t *data)
{
	static int bluetooth_side = -1;
	if (diagnostics_suspended || (n_cfg.jc_disable && !gui_bluetooth_active() && !nyx_interactive_joycon_active()))
	{
		data->state = LV_INDEV_STATE_REL;
		return false;
	}
	// Poll Joy-Con.
	jc_gamepad_rpt_t *jc_pad = gui_bluetooth_merge(n_cfg.jc_disable ? NULL : joycon_poll());
	if(nyx_interactive_joycon(jc_pad)){data->state=LV_INDEV_STATE_REL;return false;}

	if (!jc_pad)
	{
		data->state = LV_INDEV_STATE_REL;
		return false;
	}
	if (jc_pad->buttons)
		nyx_background_interaction();

	// Take a screenshot if Capture button is pressed.
	if (jc_pad->cap)
	{
		_save_fb_to_bmp();

		data->state = LV_INDEV_STATE_REL;
		return false;
	}

	/* Bluetooth may be used with just one half. Recenter when its source
     * changes, rather than waiting forever for the missing preferred half. */
    bool wireless = gui_bluetooth_active();
    bool use_right = n_cfg.jc_force_right;
    if (wireless) {
        if (use_right && !jc_pad->conn_r && jc_pad->conn_l) use_right = false;
        else if (!use_right && !jc_pad->conn_l && jc_pad->conn_r) use_right = true;
        if (bluetooth_side != (int)use_right) {
            jc_drv_ctx.calibration_step = 0;
            bluetooth_side = use_right;
        }
    } else bluetooth_side = -1;
    // Calibrate the selected stick.
	if (jc_drv_ctx.calibration_step != JC_CAL_MAX_STEPS)
	{
		if (use_right)
		{
			if (jc_pad->conn_r
				&& jc_pad->rstick_x > 0x400 && jc_pad->rstick_y > 0x400
				&& jc_pad->rstick_x < 0xC00 && jc_pad->rstick_y < 0xC00)
			{
				jc_drv_ctx.calibration_step++;
				jc_drv_ctx.cx_max = jc_pad->rstick_x + 0x96;
				jc_drv_ctx.cx_min = jc_pad->rstick_x - 0x96;
				jc_drv_ctx.cy_max = jc_pad->rstick_y + 0x96;
				jc_drv_ctx.cy_min = jc_pad->rstick_y - 0x96;
				jc_drv_ctx.cursor_timeout = 0;
			}
		}
		else if (jc_pad->conn_l
			     && jc_pad->lstick_x > 0x400 && jc_pad->lstick_y > 0x400
			     && jc_pad->lstick_x < 0xC00 && jc_pad->lstick_y < 0xC00)
		{
			jc_drv_ctx.calibration_step++;
			jc_drv_ctx.cx_max = jc_pad->lstick_x + 0x96;
			jc_drv_ctx.cx_min = jc_pad->lstick_x - 0x96;
			jc_drv_ctx.cy_max = jc_pad->lstick_y + 0x96;
			jc_drv_ctx.cy_min = jc_pad->lstick_y - 0x96;
			jc_drv_ctx.cursor_timeout = 0;
		}

		if (jc_drv_ctx.calibration_step != JC_CAL_MAX_STEPS)
		{
			if (jc_pad->plus || jc_pad->minus)
				goto handle_console;

			if (console_enabled)
				goto console;

			/* A button remains usable while the wireless stick is centering. */
            if (wireless) goto handle_buttons;
            data->state = LV_INDEV_STATE_REL;
            return false;
		}
	}

	// Re-calibrate on disconnection.
	if (use_right && !jc_pad->conn_r)
		jc_drv_ctx.calibration_step = 0;
	else if (!use_right && !jc_pad->conn_l)
		jc_drv_ctx.calibration_step = 0;

handle_buttons:
	// Set button presses.
	if (jc_pad->a || jc_pad->zl || jc_pad->zr)
		data->state = LV_INDEV_STATE_PR;
	else
		data->state = LV_INDEV_STATE_REL;

	// Enable console.
	if (jc_pad->plus || jc_pad->minus)
	{
handle_console:
		if (((u32)get_tmr_ms() - jc_drv_ctx.console_timeout) > 1000)
		{
			if (!console_enabled)
			{
				display_window_d_console_enable();
				console_enabled = true;
				gfx_con_getpos(&gfx_con.savedx, &gfx_con.savedy, &gfx_con.savedcol);
				gfx_con_setpos(964, 630, GFX_COL_AUTO);
				gfx_printf("Press -/+ to close");
				gfx_con_setpos(gfx_con.savedx, gfx_con.savedy, gfx_con.savedcol);
			}
			else
			{
				display_window_d_console_disable();
				console_enabled = false;
			}

			jc_drv_ctx.console_timeout = get_tmr_ms();
		}

		data->state = LV_INDEV_STATE_REL;

		return false;
	}

	if (console_enabled)
	{
console:
		// Print input debugging in console.
		gfx_con_getpos(&gfx_con.savedx, &gfx_con.savedy, &gfx_con.savedcol);
		gfx_con_setpos(32, 630, GFX_COL_AUTO);
		gfx_con.fntsz = 8;
		gfx_printf("x: %4X, y: %4X | rx: %4X, ry: %4X | b: %06X | c: %d (%d), %d (%d)",
			jc_pad->lstick_x, jc_pad->lstick_y, jc_pad->rstick_x, jc_pad->rstick_y,
			jc_pad->buttons, jc_pad->batt_info_l, jc_pad->batt_chrg_l,
			jc_pad->batt_info_r, jc_pad->batt_chrg_r);
		gfx_con_setpos(gfx_con.savedx, gfx_con.savedy, gfx_con.savedcol);
		gfx_con.fntsz = 16;

		data->state = LV_INDEV_STATE_REL;

		return false;
	}

	if (wireless && jc_drv_ctx.calibration_step != JC_CAL_MAX_STEPS) goto set_cursor;
	// Calculate new cursor position.
	if (!use_right)
	{
		// Left stick X.
		if (jc_pad->lstick_x <= jc_drv_ctx.cx_max && jc_pad->lstick_x >= jc_drv_ctx.cx_min)
			jc_drv_ctx.pos_x += 0;
		else if (jc_pad->lstick_x > jc_drv_ctx.cx_max)
			jc_drv_ctx.pos_x += ((jc_pad->lstick_x - jc_drv_ctx.cx_max) / 30);
		else
			jc_drv_ctx.pos_x -= ((jc_drv_ctx.cx_min - jc_pad->lstick_x) / 30);

		// Left stick Y.
		if (jc_pad->lstick_y <= jc_drv_ctx.cy_max && jc_pad->lstick_y >= jc_drv_ctx.cy_min)
			jc_drv_ctx.pos_y += 0;
		else if (jc_pad->lstick_y > jc_drv_ctx.cy_max)
		{
			s16 val = (jc_pad->lstick_y - jc_drv_ctx.cy_max) / 30;
			// Hoag has inverted Y axis.
			if (jc_pad->sio_mode)
				val *= -1;
			jc_drv_ctx.pos_y -= val;
		}
		else
		{
			s16 val = (jc_drv_ctx.cy_min - jc_pad->lstick_y) / 30;
			// Hoag has inverted Y axis.
			if (jc_pad->sio_mode)
				val *= -1;
			jc_drv_ctx.pos_y += val;
		}
	}
	else
	{
		// Right stick X.
		if (jc_pad->rstick_x <= jc_drv_ctx.cx_max && jc_pad->rstick_x >= jc_drv_ctx.cx_min)
			jc_drv_ctx.pos_x += 0;
		else if (jc_pad->rstick_x > jc_drv_ctx.cx_max)
			jc_drv_ctx.pos_x += ((jc_pad->rstick_x - jc_drv_ctx.cx_max) / 30);
		else
			jc_drv_ctx.pos_x -= ((jc_drv_ctx.cx_min - jc_pad->rstick_x) / 30);

		// Right stick Y.
		if (jc_pad->rstick_y <= jc_drv_ctx.cy_max && jc_pad->rstick_y >= jc_drv_ctx.cy_min)
			jc_drv_ctx.pos_y += 0;
		else if (jc_pad->rstick_y > jc_drv_ctx.cy_max)
		{
			s16 val = (jc_pad->rstick_y - jc_drv_ctx.cy_max) / 30;
			// Hoag has inverted Y axis.
			if (jc_pad->sio_mode)
				val *= -1;
			jc_drv_ctx.pos_y -= val;
		}
		else
		{
			s16 val = (jc_drv_ctx.cy_min - jc_pad->rstick_y) / 30;
			// Hoag has inverted Y axis.
			if (jc_pad->sio_mode)
				val *= -1;
			jc_drv_ctx.pos_y += val;
		}
	}

set_cursor:
	// Ensure value inside screen limits.
	if (jc_drv_ctx.pos_x < 0)
		jc_drv_ctx.pos_x = 0;
	else if (jc_drv_ctx.pos_x > 1279)
		jc_drv_ctx.pos_x = 1279;

	if (jc_drv_ctx.pos_y < 0)
		jc_drv_ctx.pos_y = 0;
	else if (jc_drv_ctx.pos_y > 719)
		jc_drv_ctx.pos_y = 719;

	// Set cursor position.
	data->point.x = jc_drv_ctx.pos_x;
	data->point.y = jc_drv_ctx.pos_y;

	// Auto hide cursor.
	if (jc_drv_ctx.pos_x != jc_drv_ctx.pos_last_x || jc_drv_ctx.pos_y != jc_drv_ctx.pos_last_y ||
        (wireless && (jc_pad->a || jc_pad->zl || jc_pad->zr)))
	{
		nyx_background_interaction();
		jc_drv_ctx.pos_last_x = jc_drv_ctx.pos_x;
		jc_drv_ctx.pos_last_y = jc_drv_ctx.pos_y;

		jc_drv_ctx.cursor_hidden = false;
		jc_drv_ctx.cursor_timeout = get_tmr_ms();
		lv_indev_set_cursor(jc_drv_ctx.indev_jc, jc_drv_ctx.cursor);

		// Un hide cursor.
		lv_obj_set_opa_scale_enable(jc_drv_ctx.cursor, false);
	}
	else
	{
		if (!jc_drv_ctx.cursor_hidden)
		{
			if (((u32)get_tmr_ms() - jc_drv_ctx.cursor_timeout) > 3000)
			{
				// Remove cursor and hide it.
				lv_indev_set_cursor(jc_drv_ctx.indev_jc, NULL);
				lv_obj_set_opa_scale_enable(jc_drv_ctx.cursor, true);
				lv_obj_set_opa_scale(jc_drv_ctx.cursor, LV_OPA_TRANSP);

				jc_drv_ctx.cursor_hidden = true;
			}
		}
		else
			data->state = LV_INDEV_STATE_REL; // Ensure that no clicks are allowed.
	}

	if (jc_pad->b && close_btn && lv_obj_get_click(close_btn))
	{
		lv_obj_t *button = close_btn;
		lv_action_t close_btn_action = lv_btn_get_action(button, LV_BTN_ACTION_CLICK);
		close_btn = NULL;
		if (close_btn_action)
			close_btn_action(button);
	}

	return false; // No buffering so no more data read.
}


static unsigned bt_test_clicks;
static lv_res_t bt_test_click(lv_obj_t *button){(void)button;bt_test_clicks++;return LV_RES_OK;}
static void bt_test_report(unsigned side,unsigned x,unsigned y,unsigned buttons){
 uint8_t r[13]={0x30,0,0x80};r[3]=buttons;r[4]=buttons>>8;r[5]=buttons>>16;
 unsigned at=side?9:6;r[at]=x;r[at+1]=((x>>8)&15)|((y&15)<<4);r[at+2]=y>>4;
 assert(nova_bt_decode(&bt_fixture.pad[side],side,r,sizeof r,lv_tick_get()));
}
static void bt_test_steps(unsigned count){
 for(unsigned i=0;i<count;i++){
  for(unsigned side=0;side<2;side++)if(bt_fixture.pad[side].connected)bt_fixture.pad[side].updated=lv_tick_get();
  lv_tick_inc(34);lv_task_handler();
 }
}
static void verify_bluetooth_navigation(void){
 memset(&bt_fixture,0,sizeof bt_fixture);bt_fixture.state=NOVA_BT_READY;
 memset(&rail_fixture,0,sizeof rail_fixture);memset(&jc_drv_ctx,0,sizeof jc_drv_ctx);
 n_cfg.jc_disable=0;n_cfg.jc_force_right=getenv("NYX_TEST_BT_PREFER_RIGHT")?1:0;console_enabled=false;diagnostics_suspended=false;
 lv_obj_t *target=lv_btn_create(lv_scr_act(),NULL);lv_btn_set_fit(target,false,false);lv_obj_set_size(target,300,200);lv_obj_set_pos(target,200,200);
 lv_btn_set_action(target,LV_BTN_ACTION_CLICK,bt_test_click);
 jc_drv_ctx.cursor=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_size(jc_drv_ctx.cursor,8,8);lv_obj_set_click(jc_drv_ctx.cursor,false);
 jc_drv_ctx.pos_x=jc_drv_ctx.pos_last_x=350;jc_drv_ctx.pos_y=jc_drv_ctx.pos_last_y=300;jc_drv_ctx.cursor_hidden=true;
 lv_indev_drv_t driver;lv_indev_drv_init(&driver);driver.type=LV_INDEV_TYPE_POINTER;driver.read=_jc_virt_mouse_read;jc_drv_ctx.indev_jc=lv_indev_drv_register(&driver);
 bt_test_report(1,2048,2048,0);bt_test_steps(12);assert(jc_drv_ctx.calibration_step==JC_CAL_MAX_STEPS);
 bt_test_report(1,3584,2048,0);bt_test_steps(3);assert(jc_drv_ctx.pos_x>350);
 bt_test_report(1,2048,2048,0);jc_drv_ctx.pos_x=jc_drv_ctx.pos_last_x=350;jc_drv_ctx.pos_y=jc_drv_ctx.pos_last_y=300;
 bt_test_steps(100);assert(jc_drv_ctx.cursor_hidden);
 bt_test_report(1,2048,2048,8);bt_test_steps(3);assert(!jc_drv_ctx.cursor_hidden);
 bt_test_report(1,2048,2048,0);bt_test_steps(3);assert(bt_test_clicks==1);
 /* Change to the opposite half while its non-neutral stick is still centering.
  * ZL must click without waiting for the preferred right half or a neutral stick. */
 n_cfg.jc_force_right=1;bt_fixture.pad[1].connected=0;bt_test_report(0,4000,2048,1u<<23);
 bt_test_steps(3);assert(jc_drv_ctx.calibration_step==0);
 bt_test_report(0,2048,2048,0);bt_test_steps(3);assert(bt_test_clicks==2);
 bt_test_steps(10);assert(jc_drv_ctx.calibration_step==JC_CAL_MAX_STEPS);
 lv_indev_data_t data={0};bt_fixture.pad[0].updated=lv_tick_get()-501;_jc_virt_mouse_read(&data);assert(data.state==LV_INDEV_STATE_REL);
 printf("PASS Bluetooth report -> real merge -> real pointer callback -> LVGL: right-only movement, hidden-cursor A click, left-only ZL before centering, stale release\n");
}
static lv_obj_t * preview_hit_search(const lv_indev_proc_t * proc, lv_obj_t * obj)
{
    lv_obj_t * found_p = NULL;

    /*If the point is on this object*/
    /*Check its children too*/
    if(lv_area_is_point_on(&obj->coords, &proc->act_point)) {
        lv_obj_t * i;

        LL_READ(obj->child_ll, i) {
            found_p = preview_hit_search(proc, i);

            /*If a child was found then break*/
            if(found_p != NULL) {
                break;
            }
        }

        /*If then the children was not ok, and this obj is clickable
         * and it or its parent is not hidden then save this object*/
        if(found_p == NULL && lv_obj_get_click(obj) != false) {
            lv_obj_t * hidden_i = obj;
            while(hidden_i != NULL) {
                if(lv_obj_get_hidden(hidden_i) == true) break;
                hidden_i = lv_obj_get_parent(hidden_i);
            }
            /*No parent found with hidden == true*/
            if(hidden_i == NULL) found_p = obj;
        }

    }

    return found_p;
}
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
static lv_obj_t*find_choice_action(lv_obj_t*object,lv_action_t action,u32 value){
 if(type_is(object,"lv_btn")&&lv_btn_get_action(object,LV_BTN_ACTION_CLICK)==choice_pressed&&lv_obj_get_free_num(object)==value){choices_ext*ext=lv_obj_get_ext_attr(lv_obj_get_parent(object));if(ext->action==action)return object;}
 for(lv_obj_t*c=lv_obj_get_child(object,NULL);c;c=lv_obj_get_child(object,c)){lv_obj_t*r=find_choice_action(c,action,value);if(r)return r;}return NULL;
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
static struct {lv_obj_t *win,*status,*panel;lv_signal_func_t signal;lv_task_t*task;char message[768];} tv;
static lv_res_t _signal(lv_obj_t*o,lv_signal_t s,void*p){return tv.signal(o,s,p);}
static lv_res_t _start(lv_obj_t*b){return LV_RES_OK;}
static lv_res_t _cancel(lv_obj_t*b){return LV_RES_OK;}
static void _tick(void*p){}
static void _show(const char *text)
{
    if (text != tv.message) {
        strncpy(tv.message, text, sizeof(tv.message) - 1);
        tv.message[sizeof(tv.message) - 1] = 0;
    }
    if (tv.status) {
        nyx_label_set_text(tv.status, tv.message);
        nyx_nova_fit_text(tv.status, lv_obj_get_height(tv.panel) - 48);
    }
}
static lv_res_t gui_tv_open(lv_obj_t *btn)
{
    if (tv.win)
        return LV_RES_OK;
    tv.win = nyx_create_standard_window(nyx_tr("USB-C TV / Dock"), NULL);
    tv.signal = lv_obj_get_signal_func(tv.win);
    lv_obj_set_signal_func(tv.win, _signal);
    lv_win_add_btn(tv.win, NULL, nyx_tr("Start TV"), _start);
    lv_win_add_btn(tv.win, NULL, nyx_tr("Stop TV"), _cancel);
    lv_obj_t *page = lv_win_get_content(tv.win);
    nyx_nova_fixed_page(page);
    lv_obj_t *scrl = lv_page_get_scrl(page);
    tv.panel = nyx_nova_panel(scrl, 24, 16, lv_obj_get_width(page) - 48, lv_obj_get_height(page) - 32);
    tv.status = nyx_nova_text(tv.panel, "", 24, 24, lv_obj_get_width(tv.panel) - 48, NOVA_BODY);
    if (!tv.message[0])
        _show("Use the console directly in the official dock.\nConnect dock power and HDMI, then select Start TV.\n\n"
              "You can start outside the dock, then dock within 45 seconds.\nClose this window to keep using Nova on your TV.\n"
              "VOL- stops TV output and restores the console screen.");
    else
        _show(tv.message);
    if (!tv.task)
        tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
    if (!tv.task)
        _show("TV service could not start. Close this window and try again.");
    return LV_RES_OK;
}

int main(int argc,char**argv){
SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
_set_error_mode(_OUT_TO_STDERR);
assert(argc==7);strcpy(font_root,argv[1]);
FILE*f=fopen(argv[2],"rb");assert(f);fseek(f,0,SEEK_END);u32 bytes=ftell(f);rewind(f);void*res=VirtualAlloc((void*)0xEE000000,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(res==(void*)0xEE000000);assert(fread(res,1,bytes,f)==bytes);fclose(f);
n_cfg.boot_logo_color=n_cfg.boot_bar_color=0x55C9F1;n_cfg.timedst=1;n_cfg.nova_logo_color=n_cfg.nova_icon_color=0x55C9F1;n_cfg.nova_selection_color=0x1C2937;n_cfg.theme_bg=0x141A19;n_cfg.nova_text_color=0xF2F5FA;n_cfg.nova_border_color=0x334050;n_cfg.nova_reflection_color=0x55C9F1;n_cfg.nova_menu_opacity=100;n_cfg.nova_icon_opacity=100;n_cfg.font_size=atoi(argv[5]);strcpy(n_cfg.font_file,getenv("NYX_PREVIEW_FONT")?getenv("NYX_PREVIEW_FONT"):"Lecture.nyxf");n_cfg.language=atoi(argv[4]);n_cfg.background_effect=0;n_cfg.background_speed=2;n_cfg.background_opacity=20;n_cfg.nova_halo=1;n_cfg.verification=1;h_cfg.autonogc=1;h_cfg.backlight=130;
h_cfg.rcm_patched=getenv("NYX_PREVIEW_PATCHED")!=NULL;
if(getenv("NYX_TEST_DOCK_AUTO"))n_cfg.tv_auto=atoi(getenv("NYX_TEST_DOCK_AUTO"));
if(getenv("NYX_TEST_DOCK_SCREEN"))n_cfg.tv_keep_screen=atoi(getenv("NYX_TEST_DOCK_SCREEN"));
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
else if(!strcmp(argv[3],"bluetooth")){
 unsigned bt_case=atoi(argv[6]);bt_fixture.state=bt_case<4?bt_case:0;bt_fixture.error=3;bt_fixture.rx_packets=12345678;bt_fixture.tx_packets=9876543;
 for(unsigned j=0;j<2;j++){bt_fixture.pad[j].reports=1234567;bt_fixture.pad[j].report_id=0x30;bt_fixture.pad[j].hid_open=bt_fixture.pad[j].mode_ack=1;bt_fixture.pad[j].buttons=0x800008;bt_fixture.pad[j].x=3072;bt_fixture.pad[j].y=2048;}
 s_printf(ui.note,"%s : 2 / 2",nyx_tr("Pairing records imported"));
 if(bt_case>=4&&bt_case<8){
  const char *reasons[]={"Rail connection not ready. Attach both Joy-Con and retry.","Pairing data could not be read from the rails.","Pairing checksum or host profile not recognized.","Pairing records did not pass compatibility checks."};
  assert(bt_case<8);s_printf(ui.note,"%s : 0 / 2\n%s",nyx_tr("Pairing records imported"),nyx_tr(reasons[bt_case-4]));
 }
 if(bt_case>=8){
  const char *messages[]={"Pairing saved to SD. Keep this file private.","Pairing loaded from SD.","Pairing could not be saved. Read valid pairing first.","No valid pairing file found. Current session keys kept.","Stop Bluetooth before saving or loading pairing."};
  assert(bt_case<13);s_printf(ui.note,"%s\n%s : 2 / 2",nyx_tr(messages[bt_case-8]),nyx_tr("Pairing records imported"));
 }
 if(bt_fixture.state==2){bt_fixture.pad[0].connected=bt_fixture.pad[1].connected=1;}
 gui_bluetooth_open(NULL);root=lv_win_get_content(ui.win);
}
else if(!strcmp(argv[3],"emummc"))preview_emummc_tools(NULL);
else if(!strcmp(argv[3],"diagnostic-group")){lv_obj_t b;memset(&b,0,sizeof b);lv_obj_set_free_num(&b,atoi(argv[6]));preview_show_group(&b);}
else if(!strcmp(argv[3],"diagnostic-results"))show_results(0);
else if(!strcmp(argv[3],"ram-service"))show_ram_service(NULL);
else if(!strcmp(argv[3],"setup")){first_time_clock_edit(NULL);root=lv_obj_get_child(lv_obj_get_child(lv_scr_act(),NULL),NULL);}
else if(!strcmp(argv[3],"appearance"))nyx_create_appearance_window(NULL);
else if(!strcmp(argv[3],"tv")){
 gui_tv_open(NULL);root=lv_win_get_content(tv.win);
 if(atoi(argv[6])==1){s_printf(tv.message,nyx_tr("1280x720 / 60 Hz: %s\nTV: %s %s\nLink: %d lanes, %s\n%s\n\nCheck the image on your TV. Close this window to use Nova.\nStop TV or VOL- ends TV output."),nyx_tr("Configured - check TV"),"Nintendo","TV Test 1280x720",2,"5.4 Gbps (HBR2)","");_show(tv.message);}
 if(atoi(argv[6])==2){s_printf(tv.message,nyx_tr("Failed at %s\n%s"),"DisplayPort configuration","The receiver did not respond to the command before the negotiated deadline. Check HDMI and power.");_show(tv.message);}
}
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
if(!strcmp(argv[3],"about")){lv_obj_t *tabs=find_type(root,"lv_tabview");assert(tabs);lv_tabview_set_tab_act(tabs,atoi(argv[6]),false);lv_tabview_ext_t *ext=lv_obj_get_ext_attr(tabs);verify_credit_labels(ext->content);assert(ext->tab_cnt==5);}
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
if(!strcmp(argv[3],"usb")){
 lv_indev_drv_t usb_pointer;lv_indev_drv_init(&usb_pointer);usb_pointer.type=LV_INDEV_TYPE_POINTER;usb_pointer.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&usb_pointer);
 lv_obj_t *readonly=find_action(root,_emmc_read_only_toggle);assert(readonly&&usb_msc_emmc_read_only==1);
 for(unsigned state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;state++)assert(lv_btn_get_style(readonly,state)==&lv_style_transp_tight);
 preview_click(readonly);assert(usb_msc_emmc_read_only==0);
 preview_click(readonly);assert(usb_msc_emmc_read_only==1);
}
if(!strcmp(argv[3],"bluetooth")){
 const char *names[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
 for(unsigned k=0;k<6;k++){bool found=false;for(unsigned i=0;i<ARRAY_SIZE(nova_icon_assets);i++)if(!strcmp(nova_icon_assets[i].name,names[k]))found=true;assert(found);}
}
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
if(!strcmp(argv[3],"usb")){
 lv_indev_drv_t usb_pointer;lv_indev_drv_init(&usb_pointer);usb_pointer.type=LV_INDEV_TYPE_POINTER;usb_pointer.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&usb_pointer);
 lv_obj_t *readonly=find_action(root,_emmc_read_only_toggle);assert(readonly&&usb_msc_emmc_read_only==1);
 for(unsigned state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;state++)assert(lv_btn_get_style(readonly,state)==&lv_style_transp_tight);
 preview_click(readonly);assert(usb_msc_emmc_read_only==0);
 preview_click(readonly);assert(usb_msc_emmc_read_only==1);
}
if(!strcmp(argv[3],"bluetooth")){
 const char *names[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
 for(unsigned k=0;k<6;k++){bool found=false;for(unsigned i=0;i<ARRAY_SIZE(nova_icon_assets);i++)if(!strcmp(nova_icon_assets[i].name,names[k]))found=true;assert(found);}
}
  if(end==strlen(report->text))break;report_step(report->following);
 }while(pages<128);
 assert(end==strlen(report->text)&&pages>=3);while(report->page)report_step(report->previous);assert(report->starts[0]==0);
 lv_tabview_set_tab_act(preview_emmc_tabs,atoi(argv[6]),false);lv_refr_now();
}
if(!strcmp(argv[3],"emummc-change")){
 u32 count=emummc_img->file_count,pages=(count+3)/4;if(!pages)pages=1;u32 visited=0;
 for(u32 page=0;page<pages;++page){emummc_img->file_page=page;_fill_emummc_files();lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);
if(!strcmp(argv[3],"usb")){
 lv_indev_drv_t usb_pointer;lv_indev_drv_init(&usb_pointer);usb_pointer.type=LV_INDEV_TYPE_POINTER;usb_pointer.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&usb_pointer);
 lv_obj_t *readonly=find_action(root,_emmc_read_only_toggle);assert(readonly&&usb_msc_emmc_read_only==1);
 for(unsigned state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;state++)assert(lv_btn_get_style(readonly,state)==&lv_style_transp_tight);
 preview_click(readonly);assert(usb_msc_emmc_read_only==0);
 preview_click(readonly);assert(usb_msc_emmc_read_only==1);
}
if(!strcmp(argv[3],"bluetooth")){
 const char *names[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
 for(unsigned k=0;k<6;k++){bool found=false;for(unsigned i=0;i<ARRAY_SIZE(nova_icon_assets);i++)if(!strcmp(nova_icon_assets[i].name,names[k]))found=true;assert(found);}
}
  for(lv_obj_t *button=lv_obj_get_child(emummc_img->file_items,NULL);button;button=lv_obj_get_child(emummc_img->file_items,button))if(type_is(button,"lv_btn")){
   u32 index=lv_obj_get_free_num(button);assert(index<count&&index/4==page);lv_btn_get_action(button,LV_BTN_ACTION_CLICK)(button);char path[512];sprintf(path,"emuMMC/%s",emummc_img->dirlist->name[index]);assert(!strcmp(preview_saved_emu_path,path)&&!preview_saved_emu_index&&!preview_saved_emu_sector);++visited;
  }
 }assert(visited==count);emummc_img->file_page=0;_fill_emummc_files();lv_refr_now();
}

if(!strcmp(argv[3],"setup")){assert(clock_language==n_cfg.language&&clock_ctxt.year&&clock_ctxt.month&&clock_ctxt.day&&clock_ctxt.hour&&clock_ctxt.min);assert(type_is(root,"lv_mbox"));}
if(diagnostic_last_holder){diag_tiles_ext*ext=lv_obj_get_ext_attr(diagnostic_last_holder);u32 total=ext->count;unsigned visits=0;
 for(u32 page=0;page<(total+8)/9;++page){ext->page=page;diagnostic_tiles_fill(diagnostic_last_holder);lv_refr_now();verify_visible(root,bounds);verify_reachable(root,bounds);
if(!strcmp(argv[3],"usb")){
 lv_indev_drv_t usb_pointer;lv_indev_drv_init(&usb_pointer);usb_pointer.type=LV_INDEV_TYPE_POINTER;usb_pointer.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&usb_pointer);
 lv_obj_t *readonly=find_action(root,_emmc_read_only_toggle);assert(readonly&&usb_msc_emmc_read_only==1);
 for(unsigned state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;state++)assert(lv_btn_get_style(readonly,state)==&lv_style_transp_tight);
 preview_click(readonly);assert(usb_msc_emmc_read_only==0);
 preview_click(readonly);assert(usb_msc_emmc_read_only==1);
}
if(!strcmp(argv[3],"bluetooth")){
 const char *names[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
 for(unsigned k=0;k<6;k++){bool found=false;for(unsigned i=0;i<ARRAY_SIZE(nova_icon_assets);i++)if(!strcmp(nova_icon_assets[i].name,names[k]))found=true;assert(found);}
}for(lv_obj_t*c=lv_obj_get_child(ext->tiles,NULL);c;c=lv_obj_get_child(ext->tiles,c))for(lv_obj_t*b=lv_obj_get_child(c,NULL);b;b=lv_obj_get_child(c,b))if(type_is(b,"lv_btn")){u32 id=lv_obj_get_free_num(b);assert((id&~ONE_PROBE)<42);lv_action_t action=lv_btn_get_action(b,LV_BTN_ACTION_CLICK);assert(action==(ext->results?show_report:run_selection));++visits;}}
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
if(getenv("NYX_TEST_BT_INPUT"))verify_bluetooth_navigation();
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
if(!strcmp(argv[3],"nyx")&&atoi(argv[6])==2){
 lv_indev_drv_t click_driver;lv_indev_drv_init(&click_driver);click_driver.type=LV_INDEV_TYPE_POINTER;click_driver.read=preview_pointer_read;preview_pointer_state=LV_INDEV_STATE_REL;lv_indev_drv_register(&click_driver);
 lv_obj_t*a=find_action(root,_tv_auto_action);assert(a);
 u32 automatic=n_cfg.tv_auto,keep=n_cfg.tv_keep_screen;
 preview_click(a);assert(n_cfg.tv_auto==!automatic&&nyx_changes_made);preview_click(a);assert(n_cfg.tv_auto==automatic);
 lv_obj_t*b=find_choice_action(root,_tv_screen_action,!keep);assert(b);preview_click(b);assert(n_cfg.tv_keep_screen==!keep);b=find_choice_action(root,_tv_screen_action,keep);assert(b);preview_click(b);assert(n_cfg.tv_keep_screen==keep);
}
printf("PASS UI %s language=%s size=%s tab=%s controls=%u\n",argv[3],argv[4],argv[5],argv[6],geometry_checks);return 0;}
