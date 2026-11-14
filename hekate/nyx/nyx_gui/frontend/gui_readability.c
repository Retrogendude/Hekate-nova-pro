/* Conservative large-font label fitting. SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_readability.h"
#include <string.h>

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
