/* Studio chrome. No timer, animation task or per-frame allocation. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_studio.h"
#include "gui.h"
#include "../fonts/font_manager.h"
#include <string.h>

static lv_style_t studio_panel, studio_nav, studio_selected, studio_caption;
static lv_style_t studio_icon_frame, studio_icon_selected, studio_symbol;
static lv_style_t studio_track_off, studio_track_on, studio_knob;
static lv_obj_t *studio_tabs, *studio_heading, *studio_buttons[6], *studio_icons[6];
static const char *const studio_names[] = {"hekate", "Home", "Tools", "Console Info", "Options", "Diagnostic"};
static const char *const studio_symbols[] = {SYMBOL_INFO, SYMBOL_HOME, SYMBOL_TOOLS, SYMBOL_INFO, SYMBOL_SETTINGS, SYMBOL_CHIP};

void nyx_studio_init(lv_theme_t *theme)
{
    lv_style_copy(&studio_panel, theme->panel);
    studio_panel.body.empty = 0;
    studio_panel.body.main_color = studio_panel.body.grad_color = LV_COLOR_HEX(0x151620);
    studio_panel.body.radius = 0;
    studio_panel.body.border.width = 0;
    studio_panel.body.padding.hor = studio_panel.body.padding.ver = studio_panel.body.padding.inner = 0;
    lv_style_copy(&studio_nav, theme->btn.rel);
    studio_nav.body.empty = 1;
    studio_nav.body.border.width = 0;
    studio_nav.body.radius = 6;
    studio_nav.body.shadow.width = 0;
    lv_style_copy(&studio_selected, &studio_nav);
    studio_selected.body.empty = 0;
    studio_selected.body.main_color = studio_selected.body.grad_color = LV_COLOR_HEX(0x252535);
    studio_selected.body.border.width = 1;
    studio_selected.body.border.color = LV_COLOR_HEX(0x68658A);
    lv_style_copy(&studio_caption, theme->label.prim);
    studio_caption.text.font = lv_font_get_height(nyx_font_regular()) <= 24 ? nyx_font_regular() : nyx_font_small();
    studio_caption.text.letter_space = 0;
    lv_style_copy(&studio_icon_frame, &studio_nav);
    studio_icon_frame.body.empty = 0;
    studio_icon_frame.body.main_color = studio_icon_frame.body.grad_color = LV_COLOR_HEX(0x242432);
    studio_icon_frame.body.border.width = 1;
    studio_icon_frame.body.border.opa = LV_OPA_COVER;
    studio_icon_frame.body.border.color = LV_COLOR_HEX(0x67658A);
    lv_style_copy(&studio_icon_selected, &studio_icon_frame);
    studio_icon_selected.body.border.color = LV_COLOR_HEX(0xA6A4FB);
    /* Shadow work is limited to the selected 36px icon. It is redrawn only
     * on input/exposure; the 28px halo never drives a refresh loop. */
    studio_icon_selected.body.shadow.width = NYX_STUDIO_HALO;
    studio_icon_selected.body.shadow.type = LV_SHADOW_FULL;
    studio_icon_selected.body.shadow.color = LV_COLOR_HEX(0x4A466A);
    lv_style_copy(&studio_symbol, &studio_caption);
    studio_symbol.text.font = &hekate_symbol_30;
    studio_symbol.text.color = LV_COLOR_HEX(0xBDBBFF);
    lv_style_copy(&studio_track_off, &studio_icon_frame);
    studio_track_off.body.radius = LV_RADIUS_CIRCLE;
    studio_track_off.body.main_color = studio_track_off.body.grad_color = LV_COLOR_HEX(0x343644);
    lv_style_copy(&studio_track_on, &studio_track_off);
    studio_track_on.body.main_color = studio_track_on.body.grad_color = LV_COLOR_HEX(0x8F8BDD);
    studio_track_on.body.border.color = LV_COLOR_HEX(0xBDBBFF);
    lv_style_copy(&studio_knob, &studio_track_off);
    studio_knob.body.main_color = studio_knob.body.grad_color = LV_COLOR_HEX(0xECECF8);
    studio_knob.body.border.width = 0;
}

lv_obj_t *nyx_studio_icon(lv_obj_t *parent, const char *symbol, lv_coord_t x, lv_coord_t y, bool selected)
{
    lv_obj_t *frame = lv_cont_create(parent, NULL);
    lv_cont_set_style(frame, selected ? &studio_icon_selected : &studio_icon_frame);
    lv_cont_set_fit(frame, false, false); lv_cont_set_layout(frame, LV_LAYOUT_OFF);
    lv_obj_set_size(frame, 36, 36); lv_obj_set_pos(frame, x, y); lv_obj_set_click(frame, false);
    lv_obj_t *icon = lv_label_create(frame, NULL);
    lv_label_set_style(icon, &studio_symbol); lv_label_set_text(icon, symbol);
    lv_obj_align(icon, frame, LV_ALIGN_CENTER, 0, 0); lv_obj_set_click(icon, false);
    return frame;
}

const char *nyx_studio_action_title(const char *title, char symbol[5])
{
    const char *translated = nyx_tr(title);
    unsigned length = 0;
    /* Existing Hekate symbols occupy the UTF-8 private-use area. Translate
     * the whole catalog key first, then split its icon from the caption. */
    if ((u8)translated[0] == 0xEF && translated[1] && translated[2]) length = 3;
    memcpy(symbol, translated, length); symbol[length] = 0;
    translated += length;
    while (*translated == ' ') ++translated;
    return translated;
}

void nyx_studio_icon_select(lv_obj_t *icon, bool selected)
{
    lv_cont_set_style(icon, selected ? &studio_icon_selected : &studio_icon_frame);
}

static bool studio_label_fits(lv_obj_t *label, lv_coord_t max_height)
{
    if (lv_obj_get_height(label) > max_height) return false;
    const lv_style_t *style = lv_label_get_style(label);
    const char *start = lv_label_get_text(label), *end = start;
    for (;;) {
        if (!*end || *end == ' ' || *end == '\n') {
            if (lv_txt_get_width(start, end - start, style->text.font, style->text.letter_space,
                LV_TXT_FLAG_NONE) > lv_obj_get_width(label)) return false;
            if (!*end) break;
            start = end + 1;
        }
        ++end;
    }
    return true;
}

void nyx_studio_fit_label(lv_obj_t *label, lv_coord_t max_height)
{
    if (studio_label_fits(label, max_height)) return;
    lv_label_set_style(label, &studio_caption);
    if (!studio_label_fits(label, max_height)) {
        lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL);
        lv_obj_set_height(label, lv_font_get_height(studio_caption.text.font));
    }
}

void nyx_studio_navigation_select(u32 tab)
{
    if (!studio_heading || tab >= 6) return;
    lv_label_set_text(studio_heading, studio_names[tab]);
    for (u32 i = 1; i < 6; ++i) {
        if (!studio_buttons[i]) continue;
        lv_btn_set_style(studio_buttons[i], LV_BTN_STYLE_REL, i == tab ? &studio_selected : &studio_nav);
        lv_cont_set_style(studio_icons[i], i == tab ? &studio_icon_selected : &studio_icon_frame);
    }
}

static lv_res_t studio_navigate(lv_obj_t *button)
{
    u32 tab = lv_obj_get_free_num(button);
    if (studio_tabs && tab < 6) {
        lv_tabview_set_tab_act(studio_tabs, tab, false);
        nyx_studio_navigation_select(tab);
    }
    return LV_RES_OK;
}

lv_obj_t *nyx_studio_tabview(lv_obj_t *screen)
{
    lv_obj_t *tabs = lv_tabview_create(screen, NULL);
    lv_tabview_set_sliding(tabs, false);
    lv_tabview_set_btns_hidden(tabs, true);
    lv_obj_set_size(tabs, LV_HOR_RES - NYX_STUDIO_SIDEBAR, NYX_STUDIO_BOTTOM - NYX_STUDIO_HEADER);
    lv_obj_set_pos(tabs, NYX_STUDIO_SIDEBAR, NYX_STUDIO_HEADER);
    return tabs;
}

void nyx_studio_navigation(lv_obj_t *screen, lv_obj_t *tabs, const char *version, lv_action_t settings)
{
    studio_tabs = tabs;
    lv_obj_t *rail = lv_cont_create(screen, NULL);
    lv_cont_set_style(rail, &studio_panel); lv_cont_set_fit(rail, false, false);
    lv_obj_set_size(rail, NYX_STUDIO_SIDEBAR, NYX_STUDIO_BOTTOM);
    lv_obj_set_pos(rail, 0, 0); lv_obj_set_click(rail, false);
    lv_obj_t *brand = lv_label_create(rail, NULL);
    lv_label_set_style(brand, &studio_caption); lv_label_set_text(brand, "HEKATE"); lv_obj_set_pos(brand, 28, 24);
    lv_obj_t *edition = lv_label_create(rail, NULL);
    lv_label_set_style(edition, &studio_symbol); lv_label_set_text(edition, SYMBOL_CIRCUIT); lv_obj_set_pos(edition, 174, 23);
    lv_obj_t *subtitle = lv_label_create(rail, NULL);
    lv_label_set_style(subtitle, &studio_caption); lv_label_set_text(subtitle, "Studio"); lv_obj_set_pos(subtitle, 28, 66);
    const u32 order[] = {1, 2, 3, 5, 4};
    for (u32 row = 0; row < 5; ++row) {
        u32 tab = order[row];
        lv_obj_t *button = lv_btn_create(rail, NULL);
        studio_buttons[tab] = button;
        lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
        lv_btn_set_style(button, LV_BTN_STYLE_REL, &studio_nav);
        lv_btn_set_style(button, LV_BTN_STYLE_PR, &studio_selected);
        lv_obj_set_size(button, NYX_STUDIO_SIDEBAR - 24, 78); lv_obj_set_pos(button, 12, 116 + row * 82);
        lv_obj_set_free_num(button, tab); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, studio_navigate);
        studio_icons[tab] = nyx_studio_icon(button, studio_symbols[tab], 20, 21, false);
        lv_obj_t *label = lv_label_create(button, NULL);
        lv_label_set_style(label, &studio_caption); lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
        lv_obj_set_width(label, 128); lv_label_set_text(label, studio_names[tab]);
        if (lv_obj_get_height(label) > 64) {
            lv_label_set_long_mode(label, LV_LABEL_LONG_ROLL); lv_obj_set_height(label, lv_font_get_height(studio_caption.text.font));
        }
        lv_obj_set_pos(label, 66, (78 - lv_obj_get_height(label)) / 2);
    }
    lv_obj_t *settings_btn = lv_btn_create(rail, NULL);
    lv_btn_set_fit(settings_btn, false, false); lv_btn_set_layout(settings_btn, LV_LAYOUT_OFF);
    lv_btn_set_style(settings_btn, LV_BTN_STYLE_REL, &studio_nav);
    lv_obj_set_size(settings_btn, NYX_STUDIO_SIDEBAR - 24, 60); lv_obj_set_pos(settings_btn, 12, 532);
    lv_btn_set_action(settings_btn, LV_BTN_ACTION_CLICK, settings);
    lv_obj_t *label = lv_label_create(settings_btn, NULL);
    lv_label_set_style(label, &studio_caption); lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, NYX_STUDIO_SIDEBAR - 48); lv_label_set_text(label, "Reglages Nyx");
    nyx_studio_fit_label(label, 52);
    lv_obj_align(label, settings_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *about = lv_btn_create(rail, NULL);
    lv_btn_set_fit(about, false, false); lv_btn_set_layout(about, LV_LAYOUT_OFF);
    lv_btn_set_style(about, LV_BTN_STYLE_REL, &studio_nav);
    lv_obj_set_size(about, NYX_STUDIO_SIDEBAR - 24, 56); lv_obj_set_pos(about, 12, 596);
    lv_obj_set_free_num(about, 0); lv_btn_set_action(about, LV_BTN_ACTION_CLICK, studio_navigate);
    label = lv_label_create(about, NULL); lv_label_set_style(label, &studio_caption);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK); lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_width(label, NYX_STUDIO_SIDEBAR - 48);
    (lv_label_set_text)(label, version); nyx_studio_fit_label(label, 52);
    lv_obj_align(label, about, LV_ALIGN_CENTER, 0, 0);
    studio_heading = lv_label_create(screen, NULL);
    lv_label_set_long_mode(studio_heading, LV_LABEL_LONG_ROLL);
    lv_obj_set_width(studio_heading, LV_HOR_RES - NYX_STUDIO_SIDEBAR - 64);
    lv_obj_set_pos(studio_heading, NYX_STUDIO_SIDEBAR + 32, (NYX_STUDIO_HEADER - lv_font_get_height(nyx_font_regular())) / 2);
    nyx_studio_navigation_select(1);
}

typedef struct {
    lv_btn_ext_t button;
    lv_obj_t *track, *knob, *state;
} studio_toggle_ext;
static lv_signal_func_t studio_button_signal;
static lv_res_t studio_toggle_signal(lv_obj_t *object, lv_signal_t signal, void *param)
{
    return studio_button_signal(object, signal, param);
}

bool nyx_studio_toggle_refresh(lv_obj_t *button)
{
    if (lv_obj_get_signal_func(button) != studio_toggle_signal) return false;
    studio_toggle_ext *ext = lv_obj_get_ext_attr(button);
    bool enabled = (lv_btn_get_state(button) & LV_BTN_STATE_TGL_REL) != 0;
    lv_cont_set_style(ext->track, enabled ? &studio_track_on : &studio_track_off);
    lv_obj_set_x(ext->knob, enabled ? 34 : 4);
    lv_label_set_text(ext->state, enabled ? "ON" : "OFF");
    return true;
}

lv_obj_t *nyx_studio_toggle(lv_obj_t *parent, const char *title, lv_action_t action,
    bool enabled, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *button = lv_btn_create(parent, NULL);
    if (!studio_button_signal) studio_button_signal = lv_obj_get_signal_func(button);
    studio_toggle_ext *ext = lv_obj_allocate_ext_attr(button, sizeof(*ext));
    if (!ext) { lv_obj_del(button); return NULL; }
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_obj_set_size(button, width, height); lv_btn_set_toggle(button, true);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *label = lv_label_create(button, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK); lv_obj_set_width(label, width - 24);
    lv_label_set_text(label, title); lv_obj_set_pos(label, 12, 10);
    if (lv_obj_get_height(label) > height - 62) lv_label_set_style(label, &studio_caption);
    ext->track = lv_cont_create(button, NULL);
    lv_cont_set_style(ext->track, &studio_track_off); lv_cont_set_fit(ext->track, false, false);
    lv_obj_set_size(ext->track, 62, 32); lv_obj_set_pos(ext->track, 14, height - 46); lv_obj_set_click(ext->track, false);
    ext->knob = lv_cont_create(ext->track, NULL);
    lv_cont_set_style(ext->knob, &studio_knob); lv_cont_set_fit(ext->knob, false, false);
    lv_obj_set_size(ext->knob, 24, 24); lv_obj_set_pos(ext->knob, 4, 4); lv_obj_set_click(ext->knob, false);
    ext->state = lv_label_create(button, NULL);
    lv_label_set_style(ext->state, &studio_caption); lv_label_set_long_mode(ext->state, LV_LABEL_LONG_ROLL);
    lv_obj_set_width(ext->state, width - 104); lv_obj_set_pos(ext->state, 90, height - 46 + (32 - lv_font_get_height(studio_caption.text.font)) / 2);
    lv_obj_set_signal_func(button, studio_toggle_signal);
    lv_btn_set_state(button, enabled ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_studio_toggle_refresh(button);
    return button;
}
