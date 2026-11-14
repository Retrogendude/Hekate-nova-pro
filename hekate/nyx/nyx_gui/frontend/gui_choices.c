/* Visible, stable-index choices without dropdown overlays. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_choices.h"
#include "gui.h"
#include "gui_appearance.h"
#include <string.h>

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
