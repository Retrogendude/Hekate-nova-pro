/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_LAYOUT_H
#define NYX_GUI_LAYOUT_H
#include <libs/lvgl/lvgl.h>

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
