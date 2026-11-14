/* Shared Nova action grids. SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_layout.h"
#include "gui.h"
#include <string.h>

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
