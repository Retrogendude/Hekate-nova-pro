/* Content-sized action cards shared by the Tools and Information pages. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_layout.h"
#include "gui.h"
#include "gui_readability.h"
#include <string.h>

#define GRID_MARGIN 32
#define GRID_GAP 16
#define CARD_PADDING 18

static lv_style_t card_style;

void nyx_action_grid_init(nyx_action_grid *grid, lv_theme_t *theme, lv_obj_t *page)
{
    memset(grid, 0, sizeof(*grid));
    grid->theme = theme; grid->page = page; grid->y = 24;
    lv_style_copy(&card_style, theme->panel);
    card_style.body.empty = 1;
    card_style.body.padding.hor = card_style.body.padding.ver = card_style.body.padding.inner = 0;
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF);
    lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, LV_HOR_RES);
    lv_page_set_sb_mode(page, LV_SB_MODE_AUTO);
}

lv_obj_t *nyx_action_grid_add(nyx_action_grid *grid, const char *title,
    const char *description, lv_action_t action, lv_action_t long_action, u32 id)
{
    lv_coord_t width = (LV_HOR_RES - 2 * GRID_MARGIN - GRID_GAP) / 2;
    lv_coord_t x = GRID_MARGIN + (grid->count & 1) * (width + GRID_GAP);
    lv_obj_t *card = lv_cont_create(grid->page, NULL);
    lv_cont_set_style(card, &card_style);
    lv_cont_set_fit(card, false, false);
    lv_cont_set_layout(card, LV_LAYOUT_OFF);
    lv_obj_set_click(card, false);
    lv_obj_set_width(card, width);
    lv_obj_set_pos(card, x, grid->y);

    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false);
    lv_btn_set_layout(button, LV_LAYOUT_CENTER);
    lv_obj_set_width(button, width - 2 * CARD_PADDING);
    lv_obj_set_pos(button, CARD_PADDING, CARD_PADDING);
    lv_obj_set_free_num(button, id);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    if (long_action) lv_btn_set_action(button, LV_BTN_ACTION_LONG_PR, long_action);
    lv_obj_t *label = lv_label_create(button, NULL);
    lv_label_set_recolor(label, true);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(label, width - 2 * CARD_PADDING - 28);
    lv_label_set_text(label, title);
    lv_coord_t button_height = lv_obj_get_height(label) + 28;
    if (button_height < 72) button_height = 72;
    lv_obj_set_height(button, button_height);

    lv_obj_t *help = lv_label_create(card, NULL);
    lv_label_set_style(help, &hint_small_style);
    lv_label_set_recolor(help, true);
    lv_label_set_long_mode(help, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(help, width - 2 * CARD_PADDING);
    lv_label_set_text(help, description);
    lv_obj_set_pos(help, CARD_PADDING, CARD_PADDING + button_height + 14);
    lv_coord_t height = CARD_PADDING + button_height + 14 + lv_obj_get_height(help) + CARD_PADDING;
    lv_obj_set_height(card, height);
    if (height > grid->row_height) grid->row_height = height;
    if (grid->count & 1) {
        lv_obj_set_height(card, grid->row_height);
        lv_obj_set_height(grid->previous_card, grid->row_height);
        grid->y += grid->row_height + GRID_GAP;
        grid->row_height = 0; grid->previous_card = NULL;
    } else grid->previous_card = card;
    ++grid->count;
    return button;
}

void nyx_action_grid_finish(nyx_action_grid *grid)
{
    lv_coord_t height = grid->y + grid->row_height + 24;
    lv_coord_t viewport = lv_obj_get_height(grid->page);
    if (height < viewport) height = viewport;
    lv_page_set_scrl_height(grid->page, height);
    nyx_readability_request();
}
