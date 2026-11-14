/* Compact action cards shared by the Tools and Information pages. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_layout.h"
#include "gui.h"
#include "gui_choices.h"
#include <string.h>

#define GRID_MARGIN 32
#define GRID_GAP 12
#define CARD_PADDING 12
static lv_style_t card_style;

static lv_res_t action_help(lv_obj_t *button)
{
    nyx_show_help(lv_obj_get_free_ptr(button));
    return LV_RES_OK;
}

void nyx_action_grid_init(nyx_action_grid *grid, lv_theme_t *theme, lv_obj_t *page)
{
    memset(grid, 0, sizeof(*grid));
    grid->theme = theme; grid->page = page; grid->y = 16;
    grid->row_height = (lv_obj_get_height(page) - 32 - 3 * GRID_GAP) / 4;
    lv_style_copy(&card_style, theme->panel);
    card_style.body.empty = 1;
    card_style.body.padding.hor = card_style.body.padding.ver = card_style.body.padding.inner = 0;
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

lv_obj_t *nyx_action_grid_add(nyx_action_grid *grid, const char *title,
    const char *description, lv_action_t action, lv_action_t long_action, u32 id)
{
    lv_coord_t width = (lv_obj_get_width(grid->page) - 2 * GRID_MARGIN - 16) / 2;
    lv_coord_t x = GRID_MARGIN + (grid->count & 1) * (width + 16);
    lv_obj_t *card = lv_cont_create(grid->page, NULL);
    lv_cont_set_style(card, &card_style); lv_cont_set_fit(card, false, false);
    lv_cont_set_layout(card, LV_LAYOUT_OFF); lv_obj_set_click(card, false);
    lv_obj_set_size(card, width, grid->row_height);
    lv_obj_set_pos(card, x, grid->y + grid->count / 2 * (grid->row_height + GRID_GAP));
    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_CENTER);
    lv_obj_set_width(button, width - 2 * CARD_PADDING - 88);
    lv_obj_set_pos(button, CARD_PADDING + 48, CARD_PADDING);
    lv_obj_set_free_num(button, id); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    if (long_action) lv_btn_set_action(button, LV_BTN_ACTION_LONG_PR, long_action);
    lv_obj_t *label = lv_label_create(button, NULL);
    lv_label_set_recolor(label, true); lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    char symbol[5];
    const char *caption = nyx_studio_action_title(title, symbol);
    lv_obj_set_width(label, lv_obj_get_width(button) - 16); lv_label_set_text(label, caption);
    if (lv_obj_get_height(label) + 16 > grid->row_height - 2 * CARD_PADDING)
        lv_label_set_style(label, &hint_small_style);
    lv_coord_t button_height = lv_obj_get_height(label) + 16;
    if (button_height < 44) button_height = 44;
    /* AutoRCM appends its translated state after constructing the grid.
     * Large/fixed-width faces can wrap that later text to a second line. */
    lv_coord_t font_height = lv_font_get_height(grid->theme->panel->text.font);
    if (font_height >= 28 && button_height < 2 * font_height + 18)
        button_height = 2 * font_height + 18;
    lv_obj_set_height(button, button_height);
    lv_obj_t *help = lv_label_create(card, NULL);
    lv_label_set_style(help, &hint_small_style); lv_label_set_recolor(help, true);
    lv_label_set_long_mode(help, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(help, width - 2 * CARD_PADDING); lv_label_set_text(help, description);
    int help_y = CARD_PADDING + button_height + 8;
    lv_obj_set_pos(help, CARD_PADDING, help_y);
    if (help_y + lv_obj_get_height(help) > grid->row_height - CARD_PADDING) lv_obj_set_hidden(help, true);
    lv_obj_t *info = lv_btn_create(card, NULL);
    lv_btn_set_fit(info, false, false); lv_obj_set_size(info, 32, 32);
    lv_obj_set_pos(info, width - 44, CARD_PADDING);
    lv_obj_set_free_ptr(info, (void *)description); lv_btn_set_action(info, LV_BTN_ACTION_CLICK, action_help);
    lv_obj_t *mark = lv_label_create(info, NULL); lv_label_set_style(mark, &hint_small_style);
    lv_label_set_text(mark, "?");
    nyx_studio_icon(card, symbol, CARD_PADDING, CARD_PADDING + (button_height - 36) / 2, false);
    ++grid->count;
    return button;
}

void nyx_action_grid_finish(nyx_action_grid *grid)
{
    lv_page_set_scrl_height(grid->page, lv_obj_get_height(grid->page));
}
