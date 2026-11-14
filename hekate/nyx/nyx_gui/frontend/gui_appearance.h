/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_APPEARANCE_H
#define NYX_GUI_APPEARANCE_H
#include <libs/lvgl/lvgl.h>

lv_res_t nyx_create_appearance_window(lv_obj_t *button);
void nyx_background_create(lv_obj_t *screen);
void nyx_background_pause(bool pause);
/* Called by the settings window so its generic Save action cannot commit a
 * stale pending choice. Appearance owns its own explicit Save & reload. */
void nyx_options_appearance_active(bool active, bool saved);
#endif
