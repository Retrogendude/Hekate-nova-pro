/* Unified language/readability controls and restrained native backgrounds.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui.h"
#include "gui_appearance.h"
#include "../config.h"
#include "../fonts/font_manager.h"
#include <libs/lvgl/lv_misc/lv_math.h>
#include <string.h>

/* Background drawing owns fixed storage and one low-priority 5fps task.
 * Integer sine and a handful of short polylines keep the BPMP available for
 * input. No framebuffer allocations or per-frame heap calls are required. */
#define FX_LINES 20
#define FX_POINTS 49
static struct {
    lv_obj_t *holder, *lines[FX_LINES];
    lv_style_t styles[FX_LINES];
    lv_point_t points[FX_LINES][FX_POINTS];
    lv_task_t *task;
    u32 phase, effect, speed, opacity, pause_depth;
    lv_signal_func_t original_signal;
} background;

static u32 bounded(u32 value, u32 low, u32 high, u32 fallback)
{
    return value >= low && value <= high ? value : fallback;
}

static void background_frame(void *unused)
{
    (void)unused;
    if (!background.holder || background.pause_depth || !background.effect) return;
    /* Keep a monotonic phase so particles do not jump back every 360 steps;
     * the sine paths reduce their angle independently. */
    background.phase += background.speed * 2;
    u32 count = background.effect == 2 ? 20 : (background.effect == 3 ? 7 : 4);
    for (u32 line = 0; line < count; ++line) {
        u32 point_count = background.effect == 2 ? 2 : FX_POINTS;
        if (background.effect == 2) {
            u32 x = (line * 137 + background.phase * (line % 3 + 1)) % (LV_HOR_RES - 16);
            u32 y = (line * 89 + 35 + background.phase / 2) % (LV_VER_RES - 16);
            background.points[line][0] = (lv_point_t){x, y};
            background.points[line][1] = (lv_point_t){x + 4 + line % 4, y + 2};
        } else {
            for (u32 point = 0; point < FX_POINTS; ++point) {
                int x = point * LV_HOR_RES / (FX_POINTS - 1);
                int angle = (point * (background.effect == 3 ? 8 : 6) + background.phase + line * 19) % 360;
                int amplitude = background.effect == 3 ? 62 : 26;
                int baseline = background.effect == 3 ? 300 + line * 34 : 385 + line * 72;
                int y = baseline + lv_trigo_sin(angle) * amplitude / LV_TRIGO_SIN_MAX;
                background.points[line][point] = (lv_point_t){x, y};
            }
        }
        /* Limit invalidation to the actual drawing. In particular, twenty
         * moving particles must not each invalidate a full 1280x720 canvas. */
        int left = LV_HOR_RES, top = LV_VER_RES, right = 0, bottom = 0;
        for (u32 point = 0; point < point_count; ++point) {
            lv_point_t p = background.points[line][point];
            if (p.x < left) left = p.x;
            if (p.x > right) right = p.x;
            if (p.y < top) top = p.y;
            if (p.y > bottom) bottom = p.y;
        }
        int padding = background.effect == 3 ? 8 : 3;
        for (u32 point = 0; point < point_count; ++point) {
            background.points[line][point].x -= left - padding;
            background.points[line][point].y -= top - padding;
        }
        lv_obj_set_pos(background.lines[line], left - padding, top - padding);
        lv_obj_set_size(background.lines[line], right - left + 2 * padding + 1, bottom - top + 2 * padding + 1);
        lv_line_set_points(background.lines[line], background.points[line], point_count);
    }
}

static lv_res_t background_signal(lv_obj_t *object, lv_signal_t signal, void *param)
{
    lv_res_t result = background.original_signal(object, signal, param);
    if (signal == LV_SIGNAL_CLEANUP) {
        if (background.task) lv_task_del(background.task);
        background.task = NULL; background.holder = NULL;
    }
    return result;
}

void nyx_background_create(lv_obj_t *screen)
{
    if (background.holder) lv_obj_del(background.holder);
    memset(&background, 0, sizeof(background));
    background.effect = n_cfg.background_effect < 4 ? n_cfg.background_effect : 0;
    background.speed = bounded(n_cfg.background_speed, 1, 3, 2);
    background.opacity = bounded(n_cfg.background_opacity, 10, 50, 20);
    if (!background.effect) return;
    background.holder = lv_obj_create(screen, NULL);
    lv_obj_set_style(background.holder, &lv_style_transp);
    lv_obj_set_size(background.holder, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(background.holder, 0, 0);
    lv_obj_set_click(background.holder, false);
    background.original_signal = lv_obj_get_signal_func(background.holder);
    lv_obj_set_signal_func(background.holder, background_signal);
    u32 count = background.effect == 2 ? 20 : (background.effect == 3 ? 7 : 4);
    for (u32 i = 0; i < count; ++i) {
        lv_style_copy(&background.styles[i], &lv_style_plain);
        background.styles[i].body.empty = 1;
        background.styles[i].body.border.width = 0;
        background.styles[i].line.width = background.effect == 3 ? 12 : 2;
        background.styles[i].line.rounded = 1;
        background.styles[i].line.opa = background.opacity * 255 / 100;
        lv_color_t accent = lv_theme_get_current()->btn.pr->text.color;
        background.styles[i].line.color = background.effect == 3 ?
            lv_color_mix(accent, LV_COLOR_HEX(0x5878D8), 80 + i * 20) : accent;
        background.lines[i] = lv_line_create(background.holder, NULL);
        lv_line_set_style(background.lines[i], &background.styles[i]);
        lv_line_set_auto_size(background.lines[i], false);
        lv_obj_set_size(background.lines[i], LV_HOR_RES, LV_VER_RES);
        lv_obj_set_pos(background.lines[i], 0, 0);
        lv_obj_set_click(background.lines[i], false);
    }
    background_frame(NULL);
    background.task = lv_task_create(background_frame, 200, LV_TASK_PRIO_LOWEST, NULL);
}

void nyx_background_pause(bool pause)
{
    if (pause) ++background.pause_depth;
    else if (background.pause_depth) --background.pause_depth;
    if (background.task)
        lv_task_set_prio(background.task, background.pause_depth ? LV_TASK_PRIO_OFF : LV_TASK_PRIO_LOWEST);
}

static nyx_config appearance_pending;
static lv_obj_t *appearance_parent_close;
static bool appearance_open;

enum { AP_LANGUAGE, AP_FONT, AP_SIZE, AP_LAYOUT, AP_EFFECT, AP_SPEED, AP_OPACITY };

static void appearance_notice(const char *message)
{
    static const char *buttons[] = {"\251", "\222OK", "\251", ""};
    lv_obj_t *box = lv_mbox_create(lv_scr_act(), NULL);
    lv_obj_set_width(box, 840);
    lv_mbox_set_text(box, message);
    lv_mbox_add_btns(box, buttons, NULL);
    lv_obj_align(box, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_top(box, true);
}

static lv_res_t appearance_close(lv_obj_t *button)
{
    appearance_open = false;
    lv_res_t result = nyx_win_close_action(button);
    close_btn = appearance_parent_close; appearance_parent_close = NULL;
    nyx_options_appearance_active(false, false);
    return result;
}

static lv_res_t appearance_select(lv_obj_t *list)
{
    u32 selected = lv_ddlist_get_selected(list);
    switch (lv_obj_get_free_num(list)) {
    case AP_LANGUAGE:
        appearance_pending.language = selected < I18N_LANGUAGE_COUNT ? selected : I18N_EN; break;
    case AP_FONT: {
        const char *file = nyx_font_filename(selected);
        if (!file || strlen(file) >= sizeof(appearance_pending.font_file)) return LV_RES_OK;
        strcpy(appearance_pending.font_file, file); break;
    }
    case AP_SIZE: appearance_pending.font_size = selected <= 8 ? 20 + selected * 2 : 30; break;
    case AP_LAYOUT: appearance_pending.home_layout = selected == HOME_LAYOUT_CAROUSEL ? selected : HOME_LAYOUT_TILES; break;
    case AP_EFFECT: appearance_pending.background_effect = selected < 4 ? selected : 0; break;
    case AP_SPEED: appearance_pending.background_speed = selected < 3 ? selected + 1 : 2; break;
    case AP_OPACITY: appearance_pending.background_opacity = selected < 5 ? (selected + 1) * 10 : 20; break;
    }
    return LV_RES_OK;
}

static void appearance_apply(const nyx_config *settings)
{
    n_cfg.language = settings->language;
    memcpy(n_cfg.font_file, settings->font_file, sizeof(n_cfg.font_file));
    n_cfg.font_size = settings->font_size;
    n_cfg.home_layout = settings->home_layout;
    n_cfg.background_effect = settings->background_effect;
    n_cfg.background_speed = settings->background_speed;
    n_cfg.background_opacity = settings->background_opacity;
}

static lv_res_t appearance_save(lv_obj_t *button)
{
    (void)button;
    bool mounted_before = sd_get_card_mounted();
    if (sd_mount()) { appearance_notice("SD card unavailable. Nothing was saved."); return LV_RES_OK; }
    if (f_stat("bootloader/sys/nyx.bin", NULL) != FR_OK) {
        if (!mounted_before) sd_unmount();
        appearance_notice("Nyx is missing. Nothing was saved."); return LV_RES_OK;
    }
    if (appearance_pending.font_file[0] && !nyx_font_available_at_size(appearance_pending.font_file, appearance_pending.font_size)) {
        if (!mounted_before) sd_unmount();
        appearance_notice("The selected font is missing or invalid. Choose another font."); return LV_RES_OK;
    }
    nyx_config previous = n_cfg;
    appearance_apply(&appearance_pending);
    if (create_nyx_config_entry(true)) {
        appearance_apply(&previous);
        appearance_notice("Could not save appearance settings. Check the SD card and try again.");
        return LV_RES_OK;
    }
    nyx_options_appearance_active(true, true);
    reload_nyx(NULL, false);
    appearance_notice("Settings saved. Reload Nyx to apply the changes.");
    return LV_RES_OK;
}

static lv_obj_t *appearance_text(lv_obj_t *parent, const char *text,
    int x, int y, int width, bool small)
{
    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_style(label, small ? &hint_small_style : lv_theme_get_current()->label.prim);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, width); lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y); return label;
}

static lv_obj_t *appearance_section(lv_obj_t *parent, const char *title, int y, int height)
{
    lv_obj_t *card = lv_cont_create(parent, NULL);
    lv_cont_set_style(card, lv_theme_get_current()->panel);
    lv_cont_set_fit(card, false, false); lv_cont_set_layout(card, LV_LAYOUT_OFF);
    lv_obj_set_size(card, 1180, height); lv_obj_set_pos(card, 28, y);
    appearance_text(card, title, 24, 18, 1120, false); return card;
}

static void appearance_row(lv_obj_t *card, const char *title, const char *options,
    u32 selected, int y, u32 id, bool raw)
{
    appearance_text(card, title, 24, y + 10, 460, false);
    lv_obj_t *list = lv_ddlist_create(card, NULL);
    lv_ddlist_set_hor_fit(list, false); lv_obj_set_width(list, 620);
    lv_ddlist_set_draw_arrow(list, true); lv_ddlist_set_fix_height(list, 250);
    lv_ddlist_set_sb_mode(list, LV_SB_MODE_AUTO);
    if (raw) (lv_ddlist_set_options)(list, options);
    else lv_ddlist_set_options(list, options);
    lv_ddlist_set_selected(list, selected); lv_obj_set_free_num(list, id);
    lv_ddlist_set_action(list, appearance_select);
    lv_obj_set_pos(list, 520, y);
    /* An opened dropdown must extend over the following card, rather than
     * being clipped by the short card container. Reparent preserves position. */
    lv_obj_set_parent(list, lv_obj_get_parent(card));
    lv_obj_set_pos(list, lv_obj_get_x(card) + 520, lv_obj_get_y(card) + y);
    lv_obj_set_top(list, true);
}

lv_res_t nyx_create_appearance_window(lv_obj_t *button)
{
    (void)button;
    if (appearance_open) return LV_RES_OK;
    appearance_open = true; appearance_pending = n_cfg;
    if (!nyx_font_selected_index(n_cfg.font_file)) appearance_pending.font_file[0] = 0;
    appearance_parent_close = close_btn;
    lv_obj_t *win = nyx_create_standard_window("Appearance", appearance_close);
    lv_win_add_btn(win, NULL, SYMBOL_SAVE" Save & reload", appearance_save);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win);
    lv_page_set_scrl_layout(ext->page, LV_LAYOUT_OFF);
    lv_page_set_scrl_fit(ext->page, false, false);
    lv_page_set_scrl_width(ext->page, 1236); lv_page_set_scrl_height(ext->page, 1570);
    lv_win_set_sb_mode(win, LV_SB_MODE_AUTO);
    appearance_text(win, "Choose your language, reading comfort and background in one place.", 40, 16, 1150, true);
    lv_obj_t *card = appearance_section(win, "Language", 82, 174);
    appearance_row(card, "Interface language", "English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês",
        bounded(n_cfg.language, 0, I18N_LANGUAGE_COUNT - 1, I18N_EN), 86, AP_LANGUAGE, true);
    card = appearance_section(win, "Reading comfort", 280, 370);
    appearance_row(card, "Font", nyx_font_options(), nyx_font_selected_index(n_cfg.font_file), 86, AP_FONT, true);
    u32 size = bounded(n_cfg.font_size, 20, 36, 30);
    appearance_row(card, "Text size", "20\n22\n24\n26\n28\n30\n32\n34\n36", (size - 20) / 2, 180, AP_SIZE, true);
    appearance_text(card, "To add a font, copy a Nyx font file into /fonts on the SD card, then reload Nyx.", 24, 276, 1120, true);
    card = appearance_section(win, "Home layout", 674, 174);
    appearance_row(card, "Layout", "Tiles\nCarousel", n_cfg.home_layout == HOME_LAYOUT_CAROUSEL ? 1 : 0, 86, AP_LAYOUT, false);
    card = appearance_section(win, "Animated background", 872, 430);
    appearance_row(card, "Effect", "None\nWaves\nParticles\nAurora", n_cfg.background_effect < 4 ? n_cfg.background_effect : 0, 86, AP_EFFECT, false);
    appearance_row(card, "Speed", "Slow\nNormal\nFast", bounded(n_cfg.background_speed, 1, 3, 2) - 1, 180, AP_SPEED, false);
    appearance_row(card, "Intensity", "10%\n20%\n30%\n40%\n50%", bounded(n_cfg.background_opacity, 10, 50, 20) / 10 - 1, 274, AP_OPACITY, true);
    appearance_text(card, "Subtle effects run at 5 fps and pause during hardware diagnostics.", 24, 366, 1120, true);
    appearance_text(win, "Save & reload applies all choices. Closing cancels the pending changes.", 40, 1330, 1150, true);
    appearance_text(win, nyx_font_status(), 40, 1406, 1150, true);
    nyx_options_appearance_active(true, false);
    return LV_RES_OK;
}
