/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_NOVA_H
#define NYX_GUI_NOVA_H
#include <libs/lvgl/lvgl.h>

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
void nyx_nova_selectable(lv_obj_t *button,lv_obj_t *frame,lv_obj_t *icon);
void nyx_nova_pointer(int x,int y);
void nyx_nova_home_focus(lv_obj_t *frame);
lv_res_t nyx_nova_installation_wizard(lv_obj_t *button);
u32 nyx_nova_installation_model(void);
bool nyx_nova_installation_read_media(u32 *sectors,u32 *emmc_mib,u32 *model);
bool nyx_nova_installation_read_card(u8 *cid);
const char *nyx_nova_installation_storage_error(void);
lv_res_t nyx_nova_installation_partition(u32 model,u32 systems,u32 linux_mib,u32 android_mib,const u8 *cid);
bool nyx_nova_installation_copy(u32 index,u32 start,u32 size,u32 sectors,u32 gpp,
    const u8 *cid);
void nyx_nova_active_outline(lv_obj_t *frame,bool enabled);
void nyx_nova_halo_preview(lv_obj_t *frame,u32 color1,u32 color2);
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
