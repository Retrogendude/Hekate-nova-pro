/* Unified language/readability controls and restrained native backgrounds.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui.h"
#include "gui_appearance.h"
#include "gui_choices.h"
#include <utils/sprintf.h>
#include "../config.h"
#include "../fonts/font_manager.h"
#include <libs/lvgl/lv_misc/lv_math.h>
#include <string.h>

/* Keep animation below the menu viewport. One fixed drawing object replaces
 * moving lv_line children: their old/new invalidations overflowed LVGL's 32
 * entry FIFO and promoted small particle updates to whole-screen repaints.
 * Waves/aurora redraw one 64px strip; particles invalidate at most 16 tiny
 * rectangles. Input freezes the decorative frame until navigation settles. */
#define FX_LINES 8
#define FX_POINTS 17
#define FX_BAND_HEIGHT 64
#define FX_IDLE_MS 900
#define FX_PERIOD_MS 100
static struct {
    lv_obj_t *holder;
    lv_style_t styles[FX_LINES];
    lv_point_t points[FX_LINES][FX_POINTS];
    lv_task_t *task;
    u32 phase, effect, speed, opacity, pause_depth, interaction_tick;
    bool frame_ready, interaction_pending;
    lv_signal_func_t original_signal;
} background;

static u32 bounded(u32 value, u32 low, u32 high, u32 fallback)
{
    return value >= low && value <= high ? value : fallback;
}

static u32 background_line_count(void)
{
    return background.effect == 2 ? FX_LINES : (background.effect == 3 ? 3 : 2);
}

static void background_particle_invalidate(u32 line)
{
    lv_area_t coordinates;
    lv_obj_get_coords(background.holder, &coordinates);
    lv_point_t first = background.points[line][0], last = background.points[line][1];
    lv_area_t changed = {
        coordinates.x1 + first.x - 3, coordinates.y1 + first.y - 3,
        coordinates.x1 + last.x + 3, coordinates.y1 + last.y + 3
    };
    lv_inv_area(&changed);
}

static bool background_design(lv_obj_t *object, const lv_area_t *mask, lv_design_mode_t mode)
{
    if (mode == LV_DESIGN_COVER_CHK) return false;
    if (mode != LV_DESIGN_DRAW_MAIN || !background.frame_ready) return true;
    lv_area_t coordinates;
    lv_obj_get_coords(object, &coordinates);
    u32 points = background.effect == 2 ? 2 : FX_POINTS;
    for (u32 line = 0; line < background_line_count(); ++line) {
        for (u32 point = 1; point < points; ++point) {
            lv_point_t first = background.points[line][point - 1];
            lv_point_t last = background.points[line][point];
            first.x += coordinates.x1; first.y += coordinates.y1;
            last.x += coordinates.x1; last.y += coordinates.y1;
            lv_draw_line(&first, &last, mask, &background.styles[line], LV_OPA_COVER);
        }
    }
    return true;
}

static void background_frame(void *unused)
{
    (void)unused;
    if (!background.holder || background.pause_depth || !background.effect) return;
    if (background.interaction_pending) {
        if (lv_tick_elaps(background.interaction_tick) < FX_IDLE_MS) return;
        background.interaction_pending = false;
    }
    /* This uses unsigned elapsed ticks, including wraparound. No allocation,
     * object movement or object-size updates happen in the frame callback. */
    background.phase += background.speed;
    for (u32 line = 0; line < background_line_count(); ++line) {
        if (background.effect == 2) {
            if (background.frame_ready) background_particle_invalidate(line);
            u32 x = (line * 137 + background.phase * (line % 3 + 1)) % (LV_HOR_RES - 20);
            u32 y = 8 + (line * 17 + background.phase / 3) % (FX_BAND_HEIGHT - 16);
            background.points[line][0] = (lv_point_t){x, y};
            background.points[line][1] = (lv_point_t){x + 4 + line % 3, y + 1};
            background_particle_invalidate(line);
        } else {
            for (u32 point = 0; point < FX_POINTS; ++point) {
                int angle = (point * (background.effect == 3 ? 19 : 15) + background.phase + line * 37) % 360;
                int amplitude = background.effect == 3 ? 9 : 8;
                int baseline = background.effect == 3 ? 12 + line * 18 : 17 + line * 27;
                int x = point * (LV_HOR_RES - 1) / (FX_POINTS - 1);
                int y = baseline + lv_trigo_sin(angle) * amplitude / LV_TRIGO_SIN_MAX;
                background.points[line][point] = (lv_point_t){x, y};
            }
        }
    }
    background.frame_ready = true;
    if (background.effect != 2) lv_obj_invalidate(background.holder);
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
    lv_obj_set_size(background.holder, LV_HOR_RES, FX_BAND_HEIGHT);
    lv_obj_set_pos(background.holder, 0, LV_VER_RES - FX_BAND_HEIGHT);
    lv_obj_set_click(background.holder, false);
    background.original_signal = lv_obj_get_signal_func(background.holder);
    lv_obj_set_signal_func(background.holder, background_signal);
    lv_obj_set_design_func(background.holder, background_design);
    for (u32 line = 0; line < background_line_count(); ++line) {
        lv_style_copy(&background.styles[line], &lv_style_plain);
        background.styles[line].body.empty = 1;
        background.styles[line].body.border.width = 0;
        background.styles[line].line.width = background.effect == 3 ? 6 : 2;
        background.styles[line].line.rounded = 0;
        background.styles[line].line.opa = background.opacity * 255 / 100;
        lv_color_t accent = lv_theme_get_current()->btn.pr->text.color;
        background.styles[line].line.color = background.effect == 3 ?
            lv_color_mix(accent, LV_COLOR_HEX(0x5878D8), 80 + line * 32) : accent;
    }
    background_frame(NULL);
    background.task = lv_task_create(background_frame, FX_PERIOD_MS, LV_TASK_PRIO_LOWEST, NULL);
}

void nyx_background_pause(bool pause)
{
    if (pause) ++background.pause_depth;
    else if (background.pause_depth) --background.pause_depth;
    if (background.task)
        lv_task_set_prio(background.task, background.pause_depth ? LV_TASK_PRIO_OFF : LV_TASK_PRIO_LOWEST);
}

void nyx_background_interaction(void)
{
    if (!background.holder) return;
    background.interaction_tick = lv_tick_get();
    background.interaction_pending = true;
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

static lv_res_t appearance_set_selection(u32 id, u32 selected)
{
    switch (id) {
    case AP_LANGUAGE:
        appearance_pending.language = selected < I18N_LANGUAGE_COUNT ? selected : I18N_EN; break;
    case AP_FONT: {
        if (selected >= nyx_font_count()) return LV_RES_OK;
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

static lv_obj_t *appearance_font_name, *appearance_font_counter;

static lv_res_t appearance_select(lv_obj_t *button)
{
    return appearance_set_selection(lv_obj_get_free_num(lv_obj_get_parent(button)), lv_obj_get_free_num(button));
}

static lv_res_t appearance_font_step(lv_obj_t *button)
{
    u32 count = nyx_font_count();
    if (!count) return LV_RES_OK;
    u32 current = nyx_font_selected_index(appearance_pending.font_file);
    current = (current + (lv_obj_get_free_num(button) ? 1 : count - 1)) % count;
    appearance_set_selection(AP_FONT, current);
    const char *file = nyx_font_filename(current);
    (lv_label_set_text)(appearance_font_name, file && *file ? file : nyx_tr("Default font"));
    char counter[24]; s_printf(counter, "%d / %d", current + 1, count);
    (lv_label_set_text)(appearance_font_counter, counter);
    nyx_background_interaction();
    return LV_RES_OK;
}

static void appearance_page(lv_obj_t *page)
{
    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF); lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, lv_obj_get_width(page));
    lv_page_set_scrl_height(page, lv_obj_get_height(page));
    lv_page_set_sb_mode(page, LV_SB_MODE_OFF);
    lv_obj_set_pos(lv_page_get_scrl(page), 0, 0);
    lv_obj_set_drag(lv_page_get_scrl(page), false);
}

static lv_obj_t *appearance_choices(lv_obj_t *page, const char *options, u32 selected,
    u32 columns, int x, int y, int width, int height, u32 id, bool raw)
{
    lv_obj_t *choices = nyx_choices_create(page, options, selected, columns, width, height, appearance_select, id, raw);
    if (choices) lv_obj_set_pos(choices, x, y);
    return choices;
}

static void appearance_step_button(lv_obj_t *page, const char *title, int x, u32 next)
{
    lv_obj_t *button = lv_btn_create(page, NULL);
    lv_btn_set_fit(button, false, false); lv_obj_set_size(button, 246, 64);
    lv_obj_set_pos(button, x, 210); lv_obj_set_free_num(button, next);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, appearance_font_step);
    lv_obj_t *label = lv_label_create(button, NULL); lv_label_set_text(label, title);
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
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win); appearance_page(ext->page);
    lv_obj_t *tabs = lv_tabview_create(ext->page, NULL);
    lv_obj_set_size(tabs, lv_obj_get_width(ext->page), lv_obj_get_height(ext->page));
    lv_obj_set_pos(tabs, 0, 0); lv_tabview_set_sliding(tabs, false);
    lv_obj_t *language = lv_tabview_add_tab(tabs, "Language"); appearance_page(language);
    lv_obj_t *reading = lv_tabview_add_tab(tabs, "Font"); appearance_page(reading);
    lv_obj_t *layout = lv_tabview_add_tab(tabs, "Layout"); appearance_page(layout);
    lv_obj_t *effect = lv_tabview_add_tab(tabs, "Effect"); appearance_page(effect);
    appearance_text(language, "Interface language", 32, 18, 1150, false);
    appearance_choices(language, "English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês",
        bounded(n_cfg.language, 0, I18N_LANGUAGE_COUNT - 1, I18N_EN), 3, 32, 86, 1216, 104, AP_LANGUAGE, true);
    appearance_text(language, "Save & reload applies all choices. Closing cancels the pending changes.", 32, 438, 1216, true);
    appearance_text(reading, "Font", 32, 18, 540, false);
    const char *file = appearance_pending.font_file;
    appearance_font_name = appearance_text(reading, "", 32, 84, 540, true);
    (lv_label_set_text)(appearance_font_name, *file ? file : nyx_tr("Default font"));
    appearance_step_button(reading, "<", 32, 0); appearance_step_button(reading, ">", 294, 1);
    u32 count = nyx_font_count();
    char counter[24]; s_printf(counter, "%d / %d", nyx_font_selected_index(file) + 1, count);
    appearance_font_counter = appearance_text(reading, counter, 32, 294, 540, true);
    appearance_text(reading, "To add a font, copy a Nyx font file into /fonts on the SD card, then reload Nyx.", 32, 350, 540, true);
    appearance_text(reading, "Text size", 640, 18, 580, false);
    u32 size = bounded(n_cfg.font_size, 20, 36, 30);
    appearance_choices(reading, "20\n22\n24\n26\n28\n30\n32\n34\n36", (size - 20) / 2, 3, 640, 86, 608, 88, AP_SIZE, true);
    appearance_text(layout, "Home layout", 32, 18, 1150, false);
    appearance_choices(layout, "Tiles\nCarousel", n_cfg.home_layout == HOME_LAYOUT_CAROUSEL ? 1 : 0,
        2, 32, 104, 1216, 132, AP_LAYOUT, false);
    appearance_text(layout, "Save & reload applies all choices. Closing cancels the pending changes.", 32, 302, 1216, true);
    appearance_text(effect, "Effect", 32, 12, 1150, false);
    appearance_choices(effect, "None\nWaves\nParticles\nAurora", n_cfg.background_effect < 4 ? n_cfg.background_effect : 0,
        4, 32, 66, 1216, 80, AP_EFFECT, false);
    appearance_text(effect, "Speed", 32, 170, 1150, false);
    appearance_choices(effect, "Slow\nNormal\nFast", bounded(n_cfg.background_speed, 1, 3, 2) - 1,
        3, 32, 222, 1216, 72, AP_SPEED, false);
    appearance_text(effect, "Intensity", 32, 320, 1150, false);
    appearance_choices(effect, "10%\n20%\n30%\n40%\n50%", bounded(n_cfg.background_opacity, 10, 50, 20) / 10 - 1,
        5, 32, 372, 1216, 72, AP_OPACITY, true);
    appearance_text(effect, "Effects pause while navigating and during diagnostics.", 32, 472, 1216, true);
    nyx_options_appearance_active(true, false);
    return LV_RES_OK;
}
