/* Unified language/readability controls and restrained native backgrounds.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui.h"
#include "gui_appearance.h"
#include "gui_choices.h"
#include "gui_options.h"
#include <utils/sprintf.h>
#include "../config.h"
#include "../fonts/font_manager.h"
#include <libs/lvgl/lv_misc/lv_math.h>
#include <string.h>

/* Static wallpaper only. Compatibility hooks now control the selected-icon
 * sheen; waves, particles and aurora are removed from this build. */
void nyx_background_create(lv_obj_t *screen) { nyx_nova_backdrop(screen, 0); }
void nyx_background_pause(bool pause) { nyx_nova_reflection_pause(pause); }
void nyx_background_interaction(void) { nyx_nova_reflection_interaction(); }

static nyx_config appearance_pending;
static lv_obj_t *appearance_parent_close;
static bool appearance_open;
static lv_obj_t *appearance_tabs, *appearance_language_choice, *appearance_size_choice;

enum { AP_LANGUAGE, AP_FONT, AP_SIZE, AP_LAYOUT };
static u32 bounded(u32 value,u32 low,u32 high,u32 fallback) { return value>=low&&value<=high?value:fallback; }

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
    }
    return LV_RES_OK;
}

static void appearance_apply(const nyx_config *settings)
{
    n_cfg.language = settings->language;
    memcpy(n_cfg.font_file, settings->font_file, sizeof(n_cfg.font_file));
    n_cfg.font_size = settings->font_size;
    n_cfg.home_layout = settings->home_layout;
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
    if (!nyx_options_commit_boot()) {
        if (!mounted_before) sd_unmount();
        appearance_notice("Could not save settings. Check the SD card and retry."); return LV_RES_OK;
    }
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
    return nyx_nova_text(parent,text,x,y,width,small?NOVA_SMALL:NOVA_TITLE);
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
    appearance_tabs = tabs;
    lv_obj_set_size(tabs, lv_obj_get_width(ext->page), lv_obj_get_height(ext->page));
    lv_obj_set_pos(tabs, 0, 0); lv_tabview_set_sliding(tabs, false);
    lv_obj_t *language = lv_tabview_add_tab(tabs, "Language"); appearance_page(language);
    lv_obj_t *reading = lv_tabview_add_tab(tabs, "Font"); appearance_page(reading);
    lv_obj_t *layout = lv_tabview_add_tab(tabs, "Layout"); appearance_page(layout);
    int height=lv_obj_get_height(language)-28;
    lv_obj_t *language_card=nyx_nova_panel(language,34,14,1212,height);
    appearance_text(language_card, "Interface language", 24, 20, 1164, false);
    appearance_language_choice = appearance_choices(language_card, "English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês",
        bounded(n_cfg.language, 0, I18N_LANGUAGE_COUNT - 1, I18N_EN), 3, 24, 78, 1164, 82, AP_LANGUAGE, true);
    lv_obj_t *note=appearance_text(language_card, "Save & reload applies all choices. Closing cancels the pending changes.", 24, height-62, 1164, true);nyx_nova_fit_text(note,52);
    lv_obj_t *font_card=nyx_nova_panel(reading,34,14,594,height);
    lv_obj_t *size_card=nyx_nova_panel(reading,652,14,594,height);
    appearance_text(font_card, "Font", 24, 20, 546, false);
    const char *file = appearance_pending.font_file;
    appearance_font_name = appearance_text(font_card, "", 24, 82, 546, true);
    (lv_label_set_text)(appearance_font_name, *file ? file : nyx_tr("Default font"));
    appearance_step_button(font_card, "<", 24, 0); appearance_step_button(font_card, ">", 294, 1);
    u32 count = nyx_font_count();
    char counter[24]; s_printf(counter, "%d / %d", nyx_font_selected_index(file) + 1, count);
    appearance_font_counter = appearance_text(font_card, counter, 24, 286, 546, true);
    note=appearance_text(font_card, "To add a font, copy a Nyx font file into /fonts on the SD card, then reload Nyx.", 24, 332, 546, true);nyx_nova_fit_text(note,height-344);
    appearance_text(size_card, "Text size", 24, 20, 546, false);
    u32 size = bounded(n_cfg.font_size, 20, 36, 30);
    appearance_size_choice = appearance_choices(size_card, "20\n22\n24\n26\n28\n30\n32\n34\n36", (size - 20) / 2, 3, 24, 82, 546, 82, AP_SIZE, true);
    lv_obj_t *layout_card=nyx_nova_panel(layout,34,14,1212,height);
    appearance_text(layout_card, "Home layout", 24, 20, 1164, false);
    appearance_choices(layout_card, "Tiles\nCarousel", n_cfg.home_layout == HOME_LAYOUT_CAROUSEL ? 1 : 0,
        2, 24, 104, 1164, 152, AP_LAYOUT, false);
    note=appearance_text(layout_card, "Save & reload applies all choices. Closing cancels the pending changes.", 24, height-62, 1164, true);nyx_nova_fit_text(note,52);
    nyx_options_appearance_active(true, false);
    return LV_RES_OK;
}

void nyx_appearance_open_language(u32 language)
{
    nyx_create_appearance_window(NULL);
    appearance_set_selection(AP_LANGUAGE, language);
    nyx_choices_set_selected(appearance_language_choice, appearance_pending.language);
    lv_tabview_set_tab_act(appearance_tabs, 0, false);
}

void nyx_appearance_open_font(void)
{
    nyx_create_appearance_window(NULL); lv_tabview_set_tab_act(appearance_tabs, 1, false);
}

void nyx_appearance_open_size(u32 pixels)
{
    nyx_create_appearance_window(NULL);
    u32 index = pixels >= 20 && pixels <= 36 && !(pixels & 1) ? (pixels - 20) / 2 : 5;
    appearance_set_selection(AP_SIZE, index); nyx_choices_set_selected(appearance_size_choice, index);
    lv_tabview_set_tab_act(appearance_tabs, 1, false);
}
