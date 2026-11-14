/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_STUDIO_H
#define NYX_GUI_STUDIO_H
#include <libs/lvgl/lvgl.h>

#define NYX_STUDIO_SIDEBAR 224
#define NYX_STUDIO_HEADER 64
#define NYX_STUDIO_BOTTOM 656
#define NYX_STUDIO_HALO 28

void nyx_studio_init(lv_theme_t *theme);
lv_obj_t *nyx_studio_tabview(lv_obj_t *screen);
void nyx_studio_navigation(lv_obj_t *screen, lv_obj_t *tabs, const char *version, lv_action_t settings);
void nyx_studio_navigation_select(u32 tab);
lv_obj_t *nyx_studio_icon(lv_obj_t *parent, const char *symbol, lv_coord_t x, lv_coord_t y, bool selected);
void nyx_studio_icon_select(lv_obj_t *icon, bool selected);
void nyx_studio_fit_label(lv_obj_t *label, lv_coord_t max_height);
const char *nyx_studio_action_title(const char *title, char symbol[5]);
lv_obj_t *nyx_studio_toggle(lv_obj_t *parent, const char *title, lv_action_t action,
    bool enabled, lv_coord_t width, lv_coord_t height);
/* Recognizes only Studio switches; legacy one/two-label buttons stay intact. */
bool nyx_studio_toggle_refresh(lv_obj_t *button);
#endif
