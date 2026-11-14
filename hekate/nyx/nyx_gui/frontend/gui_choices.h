/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_CHOICES_H
#define NYX_GUI_CHOICES_H
#include <libs/lvgl/lvgl.h>

/* Inline choices own their labels. Actions receive the clicked button, whose
 * free number is the stable option index; the parent's number is the group. */
lv_obj_t *nyx_choices_create(lv_obj_t *parent, const char *options, u32 selected,
    u32 columns, lv_coord_t width, lv_coord_t row_height, lv_action_t action,
    u32 group, bool raw);
void nyx_choices_set_selected(lv_obj_t *choices, u32 selected);
void nyx_show_help(const char *message);
#endif
