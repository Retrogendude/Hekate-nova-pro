/* Native implementation of the selected Nova reference. */
/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_nova.h"
#include "gui.h"
#include "gui_choices.h"
#include "gui_options.h"
#include "../fonts/font_manager.h"
#include "../config.h"
#include <string.h>
#include <stdlib.h>

typedef struct { const char *id, *title, *description, *icon; } nova_item;
#include "nova_assets/nova_reference.inc"
#include "nova_assets/nova_icons.inc"

static lv_style_t nova_panel, nova_selected, nova_hero, nova_clear, nova_primary;
static lv_style_t nova_text[5], nova_dim, nova_white, nova_accent_text;
static lv_style_t nova_frame, nova_frame_active, nova_frame_halo;
static lv_style_t nova_logo_image, nova_ink_image, nova_image, nova_image_white, nova_image_dim;
static lv_style_t nova_dot, nova_dot_active, nova_dot_on, nova_separator, nova_tag;
static lv_style_t nova_track_off, nova_track_on, nova_thumb_off, nova_thumb_on;
static lv_obj_t *nova_tabs, *nova_nav_buttons[6], *nova_nav_labels[6], *nova_nav_icons[6], *nova_underline;
static lv_obj_t *nova_battery_icon;
static lv_action_t nova_quick_actions[4];
static lv_obj_t *nova_quick_buttons[4];
static lv_obj_t *nova_brand_button;
static u32 nova_navigation_locks;
static lv_obj_t *nova_grid_pages[3], *nova_home_icons[5], *nova_home_cards[5];
static u32 nova_home_selected;
static const char *const nova_navigation_names[] = {"hekate", "Accueil", "Outils", "Infos", "Réglages", "Diagnostic"};
static const char *const nova_navigation_icons[] = {"info", "layout-grid", "wrench", "info", "settings-2", "activity"};

void nyx_nova_init(lv_theme_t *theme)
{
    nyx_nova_reflection_reset();
    memset(nova_grid_pages, 0, sizeof(nova_grid_pages));
    memset(nova_nav_buttons, 0, sizeof(nova_nav_buttons));
    memset(nova_quick_buttons, 0, sizeof(nova_quick_buttons));
    nova_navigation_locks=0;nova_brand_button=NULL;nova_underline=NULL;
    lv_color_t bg = nyx_nova_color(NOVA_COLOR_BACKGROUND);
    lv_color_t text = nyx_nova_color(NOVA_COLOR_TEXT);
    lv_color_t border = nyx_nova_color(NOVA_COLOR_BORDER);
    lv_color_t accent = nyx_nova_color(NOVA_COLOR_ICONS);
    lv_color_t panel = n_cfg.theme_bg == 0x141A19 ? LV_COLOR_HEX(0x19212D) : lv_color_mix(LV_COLOR_WHITE, bg, 14);
    lv_color_t dim = n_cfg.nova_text_color == 0xF2F5FA ? LV_COLOR_HEX(0x9BAABD) : lv_color_mix(text, bg, 166);
    memset(nova_home_icons, 0, sizeof(nova_home_icons));
    memset(nova_home_cards,0,sizeof(nova_home_cards));nova_home_selected=0;
    lv_style_copy(&nova_panel, theme->panel);
    nova_panel.body.empty = 0;
    nova_panel.body.opa = n_cfg.nova_menu_opacity * 255 / 100;
    nova_panel.body.main_color = nova_panel.body.grad_color = panel;
    nova_panel.body.radius = 8;
    nova_panel.body.border.width = 1;
    nova_panel.body.border.color = border;
    nova_panel.body.border.opa = LV_OPA_COVER;
    nova_panel.body.shadow.width = 0;
    nova_panel.body.padding.hor = nova_panel.body.padding.ver = nova_panel.body.padding.inner = 0;
    lv_style_copy(&nova_selected, &nova_panel);
    nova_selected.body.main_color = nova_selected.body.grad_color = nyx_nova_color(NOVA_COLOR_SELECTION);
    nova_selected.body.border.color = n_cfg.nova_border_color==0x334050?lv_color_mix(accent,border,160):border;
    lv_style_copy(&nova_hero, &nova_panel);
    nova_hero.body.main_color = LV_COLOR_HEX(0x1E303F);
    nova_hero.body.grad_color = LV_COLOR_HEX(0x19212D);
    nova_hero.body.border.color = LV_COLOR_HEX(0x428398);
    lv_style_copy(&nova_clear, &lv_style_transp_tight);
    nova_clear.body.border.width = nova_clear.body.shadow.width = 0;
    nova_clear.body.radius = 0;
    const u32 sizes[] = {14,16,18,20,36};
    for (u32 i = 0; i < 5; ++i) {
        lv_style_copy(&nova_text[i], theme->label.prim);
        nova_text[i].text.font = nyx_font_ui(sizes[i]);
        nova_text[i].text.letter_space = 0;
        nova_text[i].text.line_space = 3;
        nova_text[i].text.color = text;
    }
    nova_text[NOVA_KICKER].text.letter_space = 2;
    nova_text[NOVA_KICKER].text.color = dim;
    lv_style_copy(&nova_dim, &nova_text[NOVA_SMALL]);
    nova_dim.text.color = dim;
    lv_style_copy(&nova_white, &nova_text[NOVA_BODY]);
    lv_style_copy(&nova_accent_text, &nova_text[NOVA_SMALL]);
    nova_accent_text.text.color = accent;
    lv_style_copy(&nova_frame, &nova_panel);
    nova_frame.body.radius = 8;
    nova_frame.body.opa = n_cfg.nova_icon_opacity * 255 / 100;
    nova_frame.body.main_color = nova_frame.body.grad_color = n_cfg.theme_bg == 0x141A19 ? LV_COLOR_HEX(0x1E303F) : lv_color_mix(accent, panel, 22);
    nova_frame.body.border.color = n_cfg.nova_border_color==0x334050?lv_color_mix(accent,border,38):border;
    lv_style_copy(&nova_frame_active, &nova_frame);
    nova_frame_active.body.border.color = n_cfg.nova_border_color==0x334050?accent:border;
    lv_style_copy(&nova_frame_halo, &nova_frame_active);
    nova_frame_halo.body.shadow.width = 0; /* The moving reflection replaces the expensive blurred halo. */
    nova_frame_halo.body.shadow.type = LV_SHADOW_FULL;
    nova_frame_halo.body.shadow.color = LV_COLOR_HEX(0x29546A);
    lv_style_copy(&nova_image, &lv_style_plain);
    nova_image.image.color = accent;
    nova_image.image.opa = LV_OPA_COVER;
    lv_style_copy(&nova_logo_image, &nova_image); nova_logo_image.image.color = nyx_nova_color(NOVA_COLOR_LOGO);
    lv_style_copy(&nova_image_dim, &nova_image); nova_image_dim.image.color = dim;
    lv_style_copy(&nova_image_white, &nova_image); nova_image_white.image.color = text;
    lv_style_copy(&nova_ink_image, &nova_image); nova_ink_image.image.color = LV_COLOR_HEX(0x0C1721);
    lv_style_copy(&nova_primary, &nova_panel);
    nova_primary.body.main_color = nova_primary.body.grad_color = accent;
    nova_primary.body.opa = LV_OPA_COVER;
    nova_primary.body.border.width = 0; nova_primary.body.radius = 6;
    nova_primary.text.color = LV_COLOR_HEX(0x0C1721);
    nova_primary.text.font = nyx_font_ui(18);
    lv_style_copy(&nova_dot, &nova_clear);
    nova_dot.body.empty = 0; nova_dot.body.opa = LV_OPA_COVER;
    nova_dot.body.radius = LV_RADIUS_CIRCLE;
    nova_dot.body.main_color = nova_dot.body.grad_color = border;
    lv_style_copy(&nova_dot_active, &nova_dot);
    nova_dot_active.body.main_color = nova_dot_active.body.grad_color = accent;
    lv_style_copy(&nova_dot_on, &nova_dot_active);
    nova_dot_on.body.main_color = nova_dot_on.body.grad_color = LV_COLOR_HEX(0x8AE6B6);
    lv_style_copy(&nova_separator, &nova_dot); nova_separator.body.radius = 0;
    lv_style_copy(&nova_tag, &nova_frame); nova_tag.body.border.width = 0; nova_tag.body.radius = 4;
    nova_tag.body.main_color = nova_tag.body.grad_color = LV_COLOR_HEX(0x162530);
    lv_style_copy(&nova_track_off, &nova_dot);
    nova_track_off.body.main_color = nova_track_off.body.grad_color = panel;
    nova_track_off.body.radius = 8; nova_track_off.body.border.width = 1;
    nova_track_off.body.border.color = border; nova_track_off.body.border.opa = LV_OPA_COVER;
    lv_style_copy(&nova_track_on, &nova_track_off);
    nova_track_on.body.main_color = nova_track_on.body.grad_color = nyx_nova_color(NOVA_COLOR_SELECTION);
    nova_track_on.body.border.color = accent;
    lv_style_copy(&nova_thumb_off, &nova_track_off);
    nova_thumb_off.body.main_color = nova_thumb_off.body.grad_color = dim;
    nova_thumb_off.body.radius = 5; nova_thumb_off.body.border.width = 0;
    lv_style_copy(&nova_thumb_on, &nova_thumb_off);
    nova_thumb_on.body.main_color = nova_thumb_on.body.grad_color = accent;
}

const lv_style_t *nyx_nova_text_style(u32 role) { return role < 5 ? &nova_text[role] : &nova_dim; }

lv_obj_t *nyx_nova_text(lv_obj_t *parent, const char *text, int x, int y, int width, u32 role)
{
    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_style(label, (lv_style_t *)nyx_nova_text_style(role));
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, width); lv_label_set_text(label, text); lv_obj_set_pos(label, x, y);
    return label;
}

void nyx_nova_fit_text(lv_obj_t *label, int height)
{
    if (lv_obj_get_height(label) > height) lv_label_set_style(label, &nova_text[NOVA_SMALL]);
    if (lv_obj_get_height(label) > height) lv_label_set_style(label, &nova_text[NOVA_KICKER]);
    if (lv_obj_get_height(label) > height) {
        /* Fixed text boxes use ellipsis rather than perpetual marquees. The
         * selected card's complete help is also available in the detail bar. */
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_height(label, height);
    }
}

lv_obj_t *nyx_nova_panel(lv_obj_t *parent, int x, int y, int width, int height)
{
    lv_obj_t *panel = lv_cont_create(parent, NULL);
    lv_cont_set_style(panel, &nova_panel); lv_cont_set_fit(panel, false, false);
    lv_cont_set_layout(panel, LV_LAYOUT_OFF); lv_obj_set_size(panel, width, height);
    lv_obj_set_pos(panel, x, y); lv_obj_set_click(panel, false); return panel;
}

lv_obj_t *nyx_nova_glyph(lv_obj_t *parent, const char *name, int x, int y, u32 pixels)
{
    const lv_img_dsc_t *asset = nova_icon_assets[0].normal;
    for (u32 i = 0; i < ARRAY_SIZE(nova_icon_assets); ++i) if (!strcmp(name, nova_icon_assets[i].name)) {
        asset = pixels <= 18 ? nova_icon_assets[i].small : pixels >= 40 ? nova_icon_assets[i].large : nova_icon_assets[i].normal;
        break;
    }
    lv_obj_t *image = lv_img_create(parent, NULL); lv_img_set_src(image, asset);
    lv_img_set_style(image, !strcmp(name,"brand-mark") ? &nova_logo_image : &nova_image); lv_obj_set_pos(image, x, y); lv_obj_set_click(image, false);
    return image;
}

lv_obj_t *nyx_nova_icon_box(lv_obj_t *parent, const char *name, int x, int y, int size, bool selected)
{
    lv_obj_t *frame = nyx_nova_panel(parent, x, y, size, size);
    lv_cont_set_style(frame, selected ? (n_cfg.nova_halo ? &nova_frame_halo : &nova_frame_active) : &nova_frame);
    u32 pixels = size >= 70 ? 40 : 24;
    nyx_nova_glyph(frame, name, (size - pixels) / 2, (size - pixels) / 2, pixels);
    /* Static icon frame; the selected tile owns the diagonal sheen. */
    return frame;
}

void nyx_nova_icon_select(lv_obj_t *icon, bool selected)
{
    lv_cont_set_style(icon, selected ? (n_cfg.nova_halo ? &nova_frame_halo : &nova_frame_active) : &nova_frame);

}

static lv_res_t nova_navigate(lv_obj_t *button)
{
    if(nova_navigation_locks)return LV_RES_OK;
    u32 tab = lv_obj_get_free_num(button);
    if (nova_tabs && tab < 6) { lv_tabview_set_tab_act(nova_tabs, tab, false); nyx_nova_navigation_select(tab); }
    return LV_RES_OK;
}

lv_obj_t *nyx_nova_tabview(lv_obj_t *screen)
{
    lv_obj_t *tabs = lv_tabview_create(screen, NULL);
    lv_tabview_set_sliding(tabs, false); lv_tabview_set_btns_hidden(tabs, true);
    lv_obj_set_size(tabs, LV_HOR_RES, NYX_NOVA_BOTTOM - NYX_NOVA_HEADER);
    lv_obj_set_pos(tabs, 0, NYX_NOVA_HEADER); return tabs;
}

void nyx_nova_navigation_select(u32 tab)
{
    for (u32 i = 1; i < 6; ++i) if (nova_nav_buttons[i]) {
        lv_label_set_style(nova_nav_labels[i], i == tab&&!nova_navigation_locks ? &nova_white : &nova_dim);
        lv_img_set_style(nova_nav_icons[i], i == tab&&!nova_navigation_locks ? &nova_image_white : &nova_image_dim);
        /* Navigation borders remain static; only selected card glyphs shimmer. */
    }
    if (!nova_underline) return;
    lv_obj_set_hidden(nova_underline, !tab || tab >= 6);
    if (tab && tab < 6 && nova_nav_buttons[tab]) {
        lv_obj_set_x(nova_underline, lv_obj_get_x(nova_nav_buttons[tab]));
        lv_obj_set_width(nova_underline, lv_obj_get_width(nova_nav_buttons[tab]));
    }
}

void nyx_nova_navigation(lv_obj_t *screen, lv_obj_t *tabs, const char *version, lv_action_t brand_action)
{
    (void)version; nova_tabs = tabs;
    lv_obj_t *brand = lv_btn_create(screen, NULL);
    nova_brand_button=brand;
    lv_btn_set_fit(brand, false, false); lv_btn_set_layout(brand, LV_LAYOUT_OFF);
    lv_btn_set_style(brand, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(brand, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(brand, 178, 60); lv_obj_set_pos(brand, 26, 8);
    (void)brand_action; lv_btn_set_action(brand, LV_BTN_ACTION_CLICK, nova_navigate);
    lv_btn_set_action(brand, LV_BTN_ACTION_LONG_PR, nova_navigate); lv_obj_set_free_num(brand, 0);
    nyx_nova_glyph(brand, "brand-mark", 0, 10, 40);
    lv_obj_t *name = nyx_nova_text(brand, "HEKATE", 50, 8, 122, NOVA_BODY);
    static lv_style_t brand_text; lv_style_copy(&brand_text, &nova_text[NOVA_BODY]); brand_text.text.letter_space = 2;
    lv_label_set_style(name, &brand_text);
    nyx_nova_text(brand, "NOVA PRO", 50, 34, 122, NOVA_KICKER);
    const u32 order[] = {1,2,3,5,4};
    int widths[5], total = 0, gap = 18, available = 778;
    for (u32 row = 0; row < 5; ++row) {
        const char *text = nyx_tr(nova_navigation_names[order[row]]);
        widths[row] = lv_txt_get_width(text, strlen(text), nova_white.text.font, 0, LV_TXT_FLAG_NONE) + 32;
        total += widths[row];
    }
    if (total + 4 * gap > available) gap = 6;
    int x = 218;
    for (u32 row = 0; row < 5; ++row) {
        u32 tab = order[row];
        int width = total + 4 * gap > available ? widths[row] * (available - 4 * gap) / total : widths[row];
        lv_obj_t *button = lv_btn_create(screen, NULL); nova_nav_buttons[tab] = button;
        lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
        lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_clear);
        lv_obj_set_size(button, width, 52); lv_obj_set_pos(button, x, 12);
        lv_obj_set_free_num(button, tab); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_navigate);
        nova_nav_icons[tab] = nyx_nova_glyph(button, nova_navigation_icons[tab], 0, 17, 18);
        nova_nav_labels[tab] = nyx_nova_text(button, nova_navigation_names[tab], 26, 14, width - 26, NOVA_BODY);
        nyx_nova_fit_text(nova_nav_labels[tab], 26);
        x += width + gap;
    }
    const char *quick_icons[] = {"settings-2", "refresh-cw", "rotate-ccw", "power"};
    for (u32 i = 0; i < 4; ++i) {
        lv_obj_t *button = lv_btn_create(screen, NULL); nova_quick_buttons[i] = button;
        lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
        lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_frame); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_selected);
        lv_obj_set_size(button, 44, 44); lv_obj_set_pos(button, 1034 + i * 56, 16);
        nyx_nova_glyph(button, quick_icons[i], 10, 10, 24);
        lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_quick_actions[i]);
    }
    lv_obj_t *separator = lv_cont_create(screen, NULL); lv_cont_set_style(separator, &nova_separator);
    lv_cont_set_fit(separator, false, false); lv_obj_set_size(separator, LV_HOR_RES, 1);
    lv_obj_set_pos(separator, 0, NYX_NOVA_HEADER - 1); lv_obj_set_click(separator, false);
    nova_underline = lv_cont_create(screen, NULL); lv_cont_set_style(nova_underline, &nova_dot_active);
    lv_cont_set_fit(nova_underline, false, false); lv_obj_set_size(nova_underline, 80, 2);
    lv_obj_set_y(nova_underline, NYX_NOVA_HEADER - 2); lv_obj_set_click(nova_underline, false);
    nyx_nova_navigation_select(1);
}

void nyx_nova_footer(lv_obj_t *layer, lv_action_t save)
{
    lv_obj_t *bar = lv_cont_create(layer, NULL); lv_cont_set_style(bar, &nova_clear); lv_cont_set_fit(bar, false, false);
    lv_obj_set_size(bar, LV_HOR_RES, LV_VER_RES - NYX_NOVA_BOTTOM); lv_obj_set_pos(bar, 0, NYX_NOVA_BOTTOM);
    status_bar.bar_bg = bar;
    lv_obj_t *separator = lv_cont_create(bar, NULL); lv_cont_set_style(separator, &nova_separator);
    lv_cont_set_fit(separator, false, false); lv_obj_set_size(separator, LV_HOR_RES, 1); lv_obj_set_pos(separator, 0, 0); lv_obj_set_click(separator, false);
    lv_obj_t *clock = nyx_nova_glyph(bar, "clock-3", 34, 23, 18); lv_img_set_style(clock, &nova_image_dim);
    status_bar.time_temp = nyx_nova_text(bar, "00:00 · 00.0 °C", 64, 22, 450, NOVA_SMALL); lv_label_set_style(status_bar.time_temp, &nova_dim);
    static lv_style_t compact; lv_style_copy(&compact, &nova_dim); compact.text.font = nyx_font_ui(14);
    lv_obj_t *version = nyx_nova_text(bar, "hekate 6.5.4", 972, 24, 152, NOVA_SMALL); lv_label_set_style(version, &compact);
    nova_battery_icon = nyx_nova_glyph(bar, "battery-full", 1138, 20, 24); lv_img_set_style(nova_battery_icon, &nova_image_dim);
    status_bar.battery = nyx_nova_text(bar, "00.0%", 1174, 24, 72, NOVA_SMALL); lv_label_set_style(status_bar.battery, &compact);
    lv_label_set_align(status_bar.battery, LV_LABEL_ALIGN_LEFT); lv_label_set_recolor(status_bar.battery, true);
    lv_label_set_long_mode(status_bar.battery, LV_LABEL_LONG_CROP);
    lv_obj_set_size(status_bar.battery, 72, 26); lv_obj_set_pos(status_bar.battery, 1174, 24);
    status_bar.battery_more = nyx_nova_text(bar, "", 34, 42, 500, NOVA_SMALL); lv_obj_set_hidden(status_bar.battery_more, true);
    status_bar.mid = lv_btn_create(bar, NULL); lv_btn_set_fit(status_bar.mid, false, false); lv_btn_set_layout(status_bar.mid, LV_LAYOUT_OFF);
    lv_obj_set_size(status_bar.mid, 248, 44); lv_obj_set_pos(status_bar.mid, 516, 10);
    lv_obj_t *label = nyx_nova_text(status_bar.mid, "Save Options", 12, 12, 224, NOVA_SMALL); lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    nyx_nova_fit_text(label, 28); lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, save);
    lv_obj_set_opa_scale_enable(status_bar.mid, true); lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP); lv_obj_set_click(status_bar.mid, false);
}

void nyx_nova_battery(u32 percent, bool charging)
{
    if (!nova_battery_icon) return;
    const char *name = charging ? "battery-charging" : "battery-full";
    for (u32 i = 0; i < ARRAY_SIZE(nova_icon_assets); ++i) if (!strcmp(name, nova_icon_assets[i].name)) {
        lv_img_set_src(nova_battery_icon, nova_icon_assets[i].normal); break;
    }
    static lv_style_t low; lv_style_copy(&low, &nova_image_dim); low.image.color = LV_COLOR_HEX(0xFF6B6B);
    lv_img_set_style(nova_battery_icon, percent <= 15 && !charging ? &low : &nova_image_dim);
}

static void nova_uppercase_label(lv_obj_t *label, const char *text)
{
    const char *translated = nyx_tr(text); char upper[256]; uint32_t at = 0; u32 used = 0;
    while (translated[at] && used < sizeof(upper) - 5) {
        u32 cp = lv_txt_encoded_next(translated, &at);
        if ((cp >= 'a' && cp <= 'z') || (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) || (cp >= 0x430 && cp <= 0x44F)) cp -= 32;
        if (cp == 0x451) cp = 0x401;
        u32 encoded = lv_txt_unicode_to_encoded(cp);
        u32 length = lv_txt_encoded_size((const char *)&encoded);
        if (!length) break;
        memcpy(upper + used, &encoded, length); used += length;
    }
    upper[used] = 0; (lv_label_set_text)(label, upper);
}

void nyx_nova_page(lv_obj_t *page, u32 tab)
{
    static const char *const kickers[] = {"hekate", "Bienvenue dans Hekate", "Outils", "Infos", "Réglages", "Diagnostic"};
    static const char *const headings[] = {"Hekate Nova Pro", "Votre console, simplement.", "Outils et maintenance.", "Informations console.", "Apparence et options.", "Diagnostic matériel."};
    static const char *const subtitles[] = {"", "Démarrer, gérer et vérifier.", "Stockage, maintenance et connexions.", "Matériel, stockage et alimentation.", "Lecture, options et effets de sélection.", "42 contrôles natifs et un service RAM isolé."};

    lv_page_set_style(page, LV_PAGE_STYLE_BG, &lv_style_transp_tight);
    lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &lv_style_transp_tight);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF); lv_page_set_scrl_fit(page, false, false);
    lv_page_set_scrl_width(page, lv_obj_get_width(page)); lv_page_set_scrl_height(page, lv_obj_get_height(page));
    lv_page_set_sb_mode(page, LV_SB_MODE_OFF); lv_obj_set_pos(lv_page_get_scrl(page), 0, 0); lv_obj_set_drag(lv_page_get_scrl(page), false);
    if (tab >= 6) return;
    lv_obj_t *kicker = nyx_nova_text(page, kickers[tab], 34, 22, 940, NOVA_KICKER); nova_uppercase_label(kicker, kickers[tab]);
    lv_obj_t *heading = nyx_nova_text(page, headings[tab], 34, 46, 940, NOVA_HEADING);
    nyx_nova_fit_text(heading, 44);
    lv_obj_t *subtitle = nyx_nova_text(page, subtitles[tab], 34, 98, 1040, NOVA_BODY);
    lv_label_set_style(subtitle, &nova_dim); nyx_nova_fit_text(subtitle, 32);

}

typedef struct {
    lv_page_ext_t page;
    u32 kind, count, selected;
    bool diagnostics_opened, armed;
    lv_obj_t *buttons[12], *cards[12], *icons[12], *dots[12], *descriptions[12];
    const char *help[12];
    lv_action_t actions[12], holds[12];
    lv_obj_t *detail_title, *detail_text, *open;
} nova_grid_ext;
typedef struct { lv_btn_ext_t button; lv_obj_t *page; u32 index; } nova_card_ext;
static lv_signal_func_t nova_page_signal_base, nova_button_signal_base, nova_toggle_signal_base;
static lv_res_t nova_page_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_page_signal_base(object, signal, parameter); }
static lv_res_t nova_card_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_button_signal_base(object, signal, parameter); }
static const nova_item *nova_grid_items(u32 kind)
{ return kind == NOVA_TOOLS ? nova_reference_tools : kind == NOVA_INFO ? nova_reference_info : nova_reference_diagnostics; }

static void nova_grid_select(lv_obj_t *page, u32 index)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    if (index >= ext->count || !lv_obj_get_click(ext->buttons[index])) return;
    ext->selected = index;
    const nova_item *items = nova_grid_items(ext->kind);
    for (u32 i = 0; i < ext->count; ++i) {
        lv_cont_set_style(ext->cards[i], i == index ? &nova_selected : &nova_panel);
        nyx_nova_reflect(ext->cards[i],i==index);

        nyx_nova_icon_select(ext->icons[i], i == index);
        lv_cont_set_style(ext->dots[i], i == index ? &nova_dot_active : &nova_dot);
        if (ext->kind == NOVA_TOOLS && i == 6)
            lv_cont_set_style(ext->dots[i], lv_btn_get_state(ext->buttons[i]) & LV_BTN_STATE_TGL_REL ? &nova_dot_on : &nova_dot);
    }
    lv_label_set_text(ext->detail_title, items[index].title);
    if (ext->kind == NOVA_DIAGNOSTICS && !ext->diagnostics_opened) {
        char description[384];
        s_printf(description, "%s %s", nyx_tr(items[index].description), nyx_tr("Aucun test lancé."));
        lv_label_set_text(ext->detail_text, description);
    } else lv_label_set_text(ext->detail_text, items[index].description);
    nyx_nova_fit_text(ext->detail_text, 26);
}

static lv_res_t nova_card_select(lv_obj_t *button)
{
    nova_card_ext *card = lv_obj_get_ext_attr(button);
    nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
    if (!lv_obj_get_click(button)) return LV_RES_OK;
    if (ext->selected == card->index && ext->armed) {
        ext->armed = false;
        if (ext->kind == NOVA_DIAGNOSTICS) ext->diagnostics_opened = true;
        return ext->actions[card->index] ? ext->actions[card->index](button) : LV_RES_OK;
    }
    nova_grid_select(card->page, card->index); ext->armed = true;
    return LV_RES_OK;
}
static lv_res_t nova_card_hold(lv_obj_t *button)
{
    nova_card_ext *card = lv_obj_get_ext_attr(button); nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
    return ext->holds[card->index] ? ext->holds[card->index](button) : LV_RES_OK;
}
static lv_res_t nova_grid_help(lv_obj_t *button)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(lv_obj_get_free_ptr(button));
    if (ext->selected < ext->count) nyx_show_help(ext->help[ext->selected]);
    return LV_RES_OK;
}
lv_res_t nyx_nova_grid_open(lv_obj_t *button)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(lv_obj_get_free_ptr(button));
    u32 index = ext->selected;
    if (index >= ext->count || !lv_obj_get_click(ext->buttons[index]) || !ext->actions[index]) return LV_RES_OK;
    if (ext->kind == NOVA_DIAGNOSTICS) ext->diagnostics_opened = true;
    /* Retained for callers outside the tile view; visible tiles use two taps. */
    return ext->actions[index](ext->buttons[index]);
}

void nyx_nova_grid_begin(lv_obj_t *page, u32 kind)
{
    if (!nova_page_signal_base) nova_page_signal_base = lv_obj_get_signal_func(page);
    nova_grid_ext *ext = lv_obj_allocate_ext_attr(page, sizeof(*ext));
    if (!ext) return;
    memset((u8 *)ext + sizeof(lv_page_ext_t), 0, sizeof(*ext) - sizeof(lv_page_ext_t));
    ext->kind = kind; lv_obj_set_signal_func(page, nova_page_signal); nova_grid_pages[kind] = page;
    nyx_nova_page(page, kind == NOVA_TOOLS ? 2 : kind == NOVA_INFO ? 3 : 5);
    int y = lv_obj_get_height(page) - 104;
    lv_obj_t *detail = nyx_nova_panel(page, 34, y, LV_HOR_RES - 68, 76);
    lv_obj_t *edge = lv_cont_create(detail, NULL); lv_cont_set_style(edge, &nova_dot_active);
    lv_cont_set_fit(edge, false, false); lv_obj_set_size(edge, 3, 76); lv_obj_set_pos(edge, 0, 0); lv_obj_set_click(edge, false);
    ext->detail_title = nyx_nova_text(detail, "", 24, 13, 1020, NOVA_BODY);
    ext->detail_text = nyx_nova_text(detail, "", 24, 42, 1020, NOVA_SMALL); lv_label_set_style(ext->detail_text, &nova_dim);
    lv_obj_t *help = lv_btn_create(detail, NULL); lv_btn_set_fit(help, false, false); lv_btn_set_layout(help, LV_LAYOUT_OFF);
    lv_btn_set_style(help, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(help, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(help, 1080, 76); lv_obj_set_pos(help, 4, 0); lv_obj_set_free_ptr(help, page);
    lv_btn_set_action(help, LV_BTN_ACTION_CLICK, nova_grid_help);

}

lv_obj_t *nyx_nova_grid_add(lv_obj_t *page, const char *help, lv_action_t action, lv_action_t hold, u32 id)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    u32 index = ext->count, total = ext->kind == NOVA_TOOLS ? 8 : ext->kind == NOVA_INFO ? 7 : 12;
    if (index >= total) return NULL;
    const nova_item *item = &nova_grid_items(ext->kind)[index];
    int gap = ext->kind == NOVA_DIAGNOSTICS ? 12 : 16;
    int rows = ext->kind == NOVA_DIAGNOSTICS ? 3 : 2;
    int width = (LV_HOR_RES - 68 - 3 * gap) / 4;
    int height = (lv_obj_get_height(page) - 104 - 20 - 140 - (rows - 1) * gap) / rows;
    lv_obj_t *card = nyx_nova_panel(page, 34 + index % 4 * (width + gap), 140 + index / 4 * (height + gap), width, height);
    ext->cards[index] = card;
    lv_obj_t *button = lv_btn_create(card, NULL);
    if (!nova_button_signal_base) nova_button_signal_base = lv_obj_get_signal_func(button);
    nova_card_ext *card_ext = lv_obj_allocate_ext_attr(button, sizeof(*card_ext));
    if (!card_ext) { lv_obj_del(card); return NULL; }
    card_ext->page = page; card_ext->index = index; lv_obj_set_signal_func(button, nova_card_signal);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    for (u32 state = LV_BTN_STYLE_REL; state <= LV_BTN_STYLE_INA; ++state) lv_btn_set_style(button, state, &nova_clear);
    lv_obj_set_size(button, width, height); lv_obj_set_pos(button, 0, 0); lv_obj_set_free_num(button, id);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, nova_card_select);
    if (hold) lv_btn_set_action(button, LV_BTN_ACTION_LONG_PR, nova_card_hold);
    ext->buttons[index] = button; ext->actions[index] = action; ext->holds[index] = hold; ext->help[index] = help;
    int text_x = 72, text_width = width - text_x - 18;
    lv_obj_t *title = nyx_nova_text(button, item->title, text_x, 18, text_width, NOVA_BODY);
    nyx_nova_fit_text(title, ext->kind == NOVA_DIAGNOSTICS ? height - 24 : 52);
    ext->descriptions[index] = nyx_nova_text(card, item->description, text_x, 20 + lv_obj_get_height(title), text_width, NOVA_SMALL);
    lv_label_set_style(ext->descriptions[index], &nova_dim);
    if (ext->kind == NOVA_DIAGNOSTICS) lv_obj_set_hidden(ext->descriptions[index], true);
    else nyx_nova_fit_text(ext->descriptions[index], height - lv_obj_get_y(ext->descriptions[index]) - 18);
    int group_height = lv_obj_get_height(title) + (ext->kind == NOVA_DIAGNOSTICS ? 0 : lv_obj_get_height(ext->descriptions[index]) + 3);
    int title_y = (height - group_height) / 2;
    if (title_y < 12) title_y = 12;
    lv_obj_set_y(title, title_y); lv_obj_set_y(ext->descriptions[index], title_y + lv_obj_get_height(title) + 3);
    ext->icons[index] = nyx_nova_icon_box(card, item->icon, 18, (height - 42) / 2, 42, false);
    ext->dots[index] = lv_cont_create(card, NULL); lv_cont_set_style(ext->dots[index], &nova_dot);
    lv_cont_set_fit(ext->dots[index], false, false); lv_obj_set_size(ext->dots[index], 5, 5);
    lv_obj_set_pos(ext->dots[index], width - 20, 16); lv_obj_set_click(ext->dots[index], false);
    ++ext->count; return button;
}

void nyx_nova_grid_finish(lv_obj_t *page)
{
    nova_grid_ext *ext = lv_obj_get_ext_attr(page);
    for (u32 i = 0; i < ext->count; ++i) if (!lv_obj_get_click(ext->buttons[i])) {
        lv_obj_t *title = lv_obj_get_child(ext->buttons[i], NULL); lv_label_set_style(title, &nova_dim);
        lv_obj_t *glyph = lv_obj_get_child(ext->icons[i], NULL); lv_img_set_style(glyph, &nova_image_dim);
        lv_label_set_text(ext->descriptions[i], "Unavailable");
    }
    nova_grid_select(page, 0); ext->armed = false;
}

void nyx_nova_diagnostic_result(void)
{
    lv_obj_t *page = nova_grid_pages[NOVA_DIAGNOSTICS];
    if (!page) return;
    nova_grid_ext *ext = lv_obj_get_ext_attr(page); ext->diagnostics_opened = true;
    nova_grid_select(page, ext->selected);
}

void nyx_nova_refresh_halos(void)
{
    for (u32 i = 0; i < 3; ++i) if (nova_grid_pages[i]) {
        nova_grid_ext *ext = lv_obj_get_ext_attr(nova_grid_pages[i]); nova_grid_select(nova_grid_pages[i], ext->selected);
    }
    for (u32 i = 0; i < 5; ++i) {
        if(nova_home_icons[i])nyx_nova_icon_select(nova_home_icons[i],i==nova_home_selected);
        if(nova_home_cards[i])nyx_nova_reflect(nova_home_cards[i],i==nova_home_selected);
    }
}

static lv_obj_t *nova_home_action(lv_obj_t *parent, u32 index, lv_action_t action, int x, int y, int width, int height, bool hero)
{
    const nova_item *item = &nova_reference_home[index];
    lv_obj_t *card = nyx_nova_panel(parent, x, y, width, height);
    lv_cont_set_style(card, hero ? &nova_selected : &nova_panel);
    nova_home_cards[index]=card;

    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_clear);
    lv_obj_set_size(button, width, height); lv_obj_set_pos(button, 0, 0); lv_obj_set_free_num(button, index);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    nova_home_icons[index] = nyx_nova_icon_box(card, item->icon, hero ? 32 : 22, hero ? 32 : 22, hero ? 82 : 44, hero);
    int title_y = hero ? height - 184 : 84;
    lv_obj_t *title = nyx_nova_text(card, item->title, hero ? 32 : 22, title_y, width - (hero ? 64 : 44), hero ? NOVA_HEADING : NOVA_TITLE);
    nyx_nova_fit_text(title, hero ? 48 : 56);
    int description_width = hero ? 14 * lv_font_get_height(nova_text[NOVA_BODY].text.font) : width - 44;
    if (description_width > width - 64 && hero) description_width = width - 64;
    lv_obj_t *description = nyx_nova_text(card, item->description, hero ? 32 : 22, title_y + lv_obj_get_height(title) + 10, description_width, hero ? NOVA_BODY : NOVA_SMALL);
    lv_label_set_style(description, &nova_dim);
    nyx_nova_fit_text(description, hero ? 50 : height - lv_obj_get_y(description) - 20);
    if (hero) {
        lv_obj_t *launch = lv_btn_create(card, NULL); lv_btn_set_fit(launch, false, false); lv_btn_set_layout(launch, LV_LAYOUT_OFF);
        lv_btn_set_style(launch, LV_BTN_STYLE_REL, &nova_primary); lv_btn_set_style(launch, LV_BTN_STYLE_PR, &nova_primary);
        const char *caption = nyx_tr("Choisir un système");
        int launch_width = lv_txt_get_width(caption, strlen(caption), nova_text[NOVA_BODY].text.font, 0, LV_TXT_FLAG_NONE) + 72;
        if (launch_width > width - 64) launch_width = width - 64;
        lv_obj_set_size(launch, launch_width, 48); lv_obj_set_pos(launch, 32, height - 72);
        lv_obj_set_free_num(launch, index); lv_btn_set_action(launch, LV_BTN_ACTION_CLICK, action);
        lv_obj_t *label = nyx_nova_text(launch, "Choisir un système", 16, 13, launch_width - 64, NOVA_BODY);
        static lv_style_t ink_text; lv_style_copy(&ink_text, &nova_text[NOVA_BODY]); ink_text.text.color = LV_COLOR_HEX(0x0C1721);
        lv_label_set_style(label, &ink_text); nyx_nova_fit_text(label, 26);
        lv_obj_t *arrow = nyx_nova_glyph(launch, "arrow-up-right", launch_width - 40, 13, 24); lv_img_set_style(arrow, &nova_ink_image);
    } else {
        lv_obj_t *dot = lv_cont_create(card, NULL); lv_cont_set_style(dot, &nova_dot); lv_cont_set_fit(dot, false, false);
        lv_obj_set_size(dot, 5, 5); lv_obj_set_pos(dot, width - 22, 18); lv_obj_set_click(dot, false);
    }
    nyx_nova_reflect(card,hero);
    return button;
}

void nyx_nova_home(lv_obj_t *page, lv_action_t activate)
{
    nyx_nova_page(page, 1);
    int height = lv_obj_get_height(page) - 140 - 28;
    int width = LV_HOR_RES - 68, hero_width = (width - 22) * 11 / 21;
    nova_home_action(page, 0, activate, 34, 140, hero_width, height, true);
    int tile_width = (width - hero_width - 22 - 16) / 2, tile_height = (height - 16) / 2;
    for (u32 i = 1; i < 5; ++i) nova_home_action(page, i, activate,
        34 + hero_width + 22 + (i - 1) % 2 * (tile_width + 16), 140 + (i - 1) / 2 * (tile_height + 16), tile_width, tile_height, false);
}

static struct {
    u32 selected;
    lv_obj_t *track, *cards[5], *icons[5], *title, *detail, *open, *counter;
} home_carousel;

static void nova_carousel_update(void)
{
    u32 selected = home_carousel.selected;nova_home_selected=selected;
    lv_obj_set_x(home_carousel.track, 372 - selected * 344);
    for (u32 i = 0; i < 5; ++i) {
        lv_btn_set_style(home_carousel.cards[i], LV_BTN_STYLE_REL, i == selected ? &nova_selected : &nova_panel);
        nyx_nova_icon_select(home_carousel.icons[i], i == selected);
        nyx_nova_reflect(home_carousel.cards[i],i==selected);

    }
    lv_label_set_text(home_carousel.title, nova_reference_home[selected].title);
    lv_label_set_text(home_carousel.detail, nova_reference_home[selected].description);
    nyx_nova_fit_text(home_carousel.title, 44); nyx_nova_fit_text(home_carousel.detail, 58);
    lv_obj_set_free_num(home_carousel.open, selected);
    char position[16]; s_printf(position, "%d / 5", selected + 1); lv_label_set_text(home_carousel.counter, position);
}

static lv_res_t nova_carousel_select(lv_obj_t *button)
{
    u32 index = lv_obj_get_free_num(button);
    if (index < 5) { home_carousel.selected = index; nova_carousel_update(); }
    return LV_RES_OK;
}
static lv_res_t nova_carousel_step(lv_obj_t *button)
{
    home_carousel.selected = (home_carousel.selected + (lv_obj_get_free_num(button) ? 1 : 4)) % 5;
    nova_carousel_update(); return LV_RES_OK;
}

void nyx_nova_carousel(lv_obj_t *page, lv_action_t activate)
{
    nyx_nova_page(page, 1); home_carousel.selected = 0;
    for (u32 i = 0; i < 2; ++i) {
        lv_obj_t *arrow = lv_btn_create(page, NULL); lv_btn_set_fit(arrow, false, false); lv_btn_set_layout(arrow, LV_LAYOUT_OFF);
        lv_obj_set_size(arrow, 50, 60); lv_obj_set_pos(arrow, i ? 1196 : 34, 226);
        lv_obj_set_free_num(arrow, i); lv_btn_set_action(arrow, LV_BTN_ACTION_CLICK, nova_carousel_step);
        lv_obj_t *label = nyx_nova_text(arrow, i ? ">" : "<", 16, 16, 22, NOVA_TITLE); (void)label;
    }
    lv_obj_t *viewport = nyx_nova_panel(page, 108, 140, 1064, 244); lv_cont_set_style(viewport, &nova_clear);
    home_carousel.track = nyx_nova_panel(viewport, 0, 0, 5 * 344 - 24, 244); lv_cont_set_style(home_carousel.track, &nova_clear);
    for (u32 i = 0; i < 5; ++i) {
        lv_obj_t *card = lv_btn_create(home_carousel.track, NULL); home_carousel.cards[i] = card;nova_home_cards[i]=card;
        lv_btn_set_fit(card, false, false); lv_btn_set_layout(card, LV_LAYOUT_OFF);
        lv_btn_set_style(card, LV_BTN_STYLE_REL, &nova_panel); lv_btn_set_style(card, LV_BTN_STYLE_PR, &nova_selected);
        lv_obj_set_size(card, 320, 244); lv_obj_set_pos(card, i * 344, 0);
        lv_obj_set_free_num(card, i); lv_btn_set_action(card, LV_BTN_ACTION_CLICK, nova_carousel_select);
        home_carousel.icons[i] = nyx_nova_icon_box(card, nova_reference_home[i].icon, 124, 26, 72, i == 0);
        lv_obj_t *title = nyx_nova_text(card, nova_reference_home[i].title, 22, 128, 276, NOVA_TITLE); nyx_nova_fit_text(title, 54);
        lv_obj_t *description = nyx_nova_text(card, nova_reference_home[i].description, 22, 186, 276, NOVA_SMALL);
        lv_label_set_style(description, &nova_dim); nyx_nova_fit_text(description, 44);
    }
    lv_obj_t *detail = nyx_nova_panel(page, 108, 406, 1064, 146);
    home_carousel.title = nyx_nova_text(detail, "", 24, 20, 730, NOVA_TITLE);
    home_carousel.detail = nyx_nova_text(detail, "", 24, 66, 730, NOVA_BODY); lv_label_set_style(home_carousel.detail, &nova_dim);
    home_carousel.open = lv_btn_create(detail, NULL); lv_btn_set_fit(home_carousel.open, false, false); lv_btn_set_layout(home_carousel.open, LV_LAYOUT_OFF);
    lv_btn_set_style(home_carousel.open, LV_BTN_STYLE_REL, &nova_primary); lv_btn_set_style(home_carousel.open, LV_BTN_STYLE_PR, &nova_primary);
    lv_obj_set_size(home_carousel.open, 220, 54); lv_obj_set_pos(home_carousel.open, 820, 46);
    lv_btn_set_action(home_carousel.open, LV_BTN_ACTION_CLICK, activate);
    lv_obj_t *label = nyx_nova_text(home_carousel.open, "Open", 18, 14, 180, NOVA_BODY);
    static lv_style_t carousel_ink; lv_style_copy(&carousel_ink, &nova_text[NOVA_BODY]); carousel_ink.text.color = LV_COLOR_HEX(0x0C1721);
    lv_label_set_style(label, &carousel_ink);
    home_carousel.counter = nyx_nova_text(page, "", 34, 406, 62, NOVA_SMALL);
    nova_carousel_update();
}

typedef struct { lv_btn_ext_t button; lv_obj_t *track, *knob, *power, *state; } nova_toggle_ext;
static lv_res_t nova_toggle_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{ return nova_toggle_signal_base(object, signal, parameter); }

bool nyx_nova_toggle_refresh(lv_obj_t *button)
{
    if (lv_obj_get_signal_func(button) == nova_card_signal) {
        nova_card_ext *card = lv_obj_get_ext_attr(button); nova_grid_ext *ext = lv_obj_get_ext_attr(card->page);
        if (ext->kind == NOVA_TOOLS && card->index == 6) {
            lv_cont_set_style(ext->dots[card->index], lv_btn_get_state(button) & LV_BTN_STATE_TGL_REL ? &nova_dot_on : &nova_dot);
            return true;
        }
        return false;
    }
    if (lv_obj_get_signal_func(button) != nova_toggle_signal) return false;
    nova_toggle_ext *ext = lv_obj_get_ext_attr(button); bool enabled = (lv_btn_get_state(button) & LV_BTN_STATE_TGL_REL) != 0;
    lv_cont_set_style(ext->track, enabled ? &nova_track_on : &nova_track_off);
    lv_cont_set_style(ext->knob, enabled ? &nova_thumb_on : &nova_thumb_off);
    lv_obj_set_x(ext->knob, enabled ? 35 : 5);
    lv_label_set_style(ext->state, enabled ? &nova_accent_text : &nova_dim);
    lv_label_set_text(ext->state, enabled ? "Actif" : "Inactif");
    if (status_bar.mid && nyx_options_has_changes()) {
        lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    }
    return true;
}

static lv_obj_t *nova_toggle_base(lv_obj_t *parent, lv_action_t action, bool enabled, int width, int height, int track_x, int track_y, int state_width)
{
    lv_obj_t *button = lv_btn_create(parent, NULL);
    if (!nova_toggle_signal_base) nova_toggle_signal_base = lv_obj_get_signal_func(button);
    nova_toggle_ext *ext = lv_obj_allocate_ext_attr(button, sizeof(*ext));
    if (!ext) { lv_obj_del(button); return NULL; }
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_PR, &nova_clear);
    lv_btn_set_style(button, LV_BTN_STYLE_TGL_REL, &nova_clear); lv_btn_set_style(button, LV_BTN_STYLE_TGL_PR, &nova_clear);
    lv_obj_set_size(button, width, height); lv_btn_set_toggle(button, true); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    ext->track = lv_cont_create(button, NULL); lv_cont_set_style(ext->track, &nova_track_off); lv_cont_set_fit(ext->track, false, false);
    lv_obj_set_size(ext->track, 64, 34); lv_obj_set_pos(ext->track, track_x, track_y); lv_obj_set_click(ext->track, false);
    ext->knob = lv_cont_create(ext->track, NULL); lv_cont_set_style(ext->knob, &nova_thumb_off); lv_cont_set_fit(ext->knob, false, false);
    lv_obj_set_size(ext->knob, 24, 24); lv_obj_set_pos(ext->knob, 5, 5); lv_obj_set_click(ext->knob, false);
    ext->power = nyx_nova_glyph(ext->knob, "power", 3, 3, 18); lv_img_set_style(ext->power, &nova_ink_image);
    ext->state = nyx_nova_text(button, "", track_x + 72, track_y + 6, state_width, NOVA_SMALL); lv_label_set_style(ext->state, &nova_dim);
    lv_obj_set_signal_func(button, nova_toggle_signal); lv_btn_set_state(button, enabled ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_nova_toggle_refresh(button); return button;
}

lv_obj_t *nyx_nova_toggle(lv_obj_t *parent, const char *title, lv_action_t action, bool enabled, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *button = nova_toggle_base(parent, action, enabled, width, height, 14, height - 48, width - 100);
    if (!button) return NULL;
    lv_obj_t *label = nyx_nova_text(button, title, 12, 10, width - 24, NOVA_TITLE); nyx_nova_fit_text(label, height - 66);
    return button;
}

lv_obj_t *nyx_nova_switch_row(lv_obj_t *parent, const char *title, const char *help, const char *icon, lv_action_t action, bool enabled, int x, int y, int width, int height)
{
    int track_x = width - 158;
    lv_obj_t *button = nova_toggle_base(parent, action, enabled, width, height, track_x, (height - 34) / 2, 82);
    if (!button) return NULL;
    lv_obj_set_pos(button, x, y); nyx_nova_glyph(button, icon, 0, 18, 18);
    lv_obj_t *label = nyx_nova_text(button, title, 30, 12, track_x - 42, NOVA_BODY); nyx_nova_fit_text(label, 26);
    if (!strcmp(title, "Auto NoGC")) (lv_label_set_text)(label, title);
    lv_obj_t *description = nyx_nova_text(button, help, 30, 42, track_x - 42, NOVA_SMALL);
    lv_label_set_style(description, &nova_dim); nyx_nova_fit_text(description, height - 46);
    lv_obj_t *line = lv_cont_create(parent, NULL); lv_cont_set_style(line, &nova_separator); lv_cont_set_fit(line, false, false);
    lv_obj_set_size(line, width, 1); lv_obj_set_pos(line, x, y + height - 1); lv_obj_set_click(line, false);
    return button;
}

void nyx_nova_quick_actions(const lv_action_t actions[4])
{
    for (u32 i = 0; i < 4; ++i) {
        nova_quick_actions[i] = actions[i];
        if (nova_quick_buttons[i]) lv_btn_set_action(nova_quick_buttons[i], LV_BTN_ACTION_CLICK, actions[i]);
    }
}

void nyx_nova_navigation_lock(bool lock)
{
    if(lock)++nova_navigation_locks;else if(nova_navigation_locks)--nova_navigation_locks;
    bool enabled=!nova_navigation_locks;
    if(nova_brand_button)lv_obj_set_click(nova_brand_button,enabled);
    for(u32 i=1;i<6;++i)if(nova_nav_buttons[i])lv_obj_set_click(nova_nav_buttons[i],enabled);
    for(u32 i=0;i<4;++i)if(nova_quick_buttons[i]){
        lv_obj_set_click(nova_quick_buttons[i],enabled);
        lv_obj_t *glyph=lv_obj_get_child(nova_quick_buttons[i],NULL);if(glyph)lv_img_set_style(glyph,enabled?&nova_image:&nova_image_dim);
    }
    if(nova_tabs)nyx_nova_navigation_select(lv_tabview_get_tab_act(nova_tabs));
}

lv_color_t nyx_nova_color(u32 role)
{
    if (role == NOVA_COLOR_LOGO) return LV_COLOR_HEX(n_cfg.nova_logo_color);
    if (role == NOVA_COLOR_ICONS) return LV_COLOR_HEX(n_cfg.nova_icon_color);
    if (role == NOVA_COLOR_SELECTION) return LV_COLOR_HEX(n_cfg.nova_selection_color);
    if (role == NOVA_COLOR_TEXT) return LV_COLOR_HEX(n_cfg.nova_text_color);
    if (role == NOVA_COLOR_BORDER) return LV_COLOR_HEX(n_cfg.nova_border_color);
    if (role == NOVA_COLOR_REFLECTION) return LV_COLOR_HEX(n_cfg.nova_reflection_color);
    return LV_COLOR_HEX(n_cfg.theme_bg == 0x141A19 ? 0x10151D : n_cfg.theme_bg);
}

/* Visual bands only: no changes to fan policy or hardware protection. */
u32 nyx_nova_temperature_rgb(u32 degrees)
{ return degrees < 45 ? 0x8AE6B6 : degrees < 55 ? 0xF3D36A : degrees < 65 ? 0xF4A261 : 0xFF6B6B; }

void nyx_nova_about(lv_obj_t *page)
{
    nyx_nova_page(page,0);
    static const char *titles[]={"Créateurs","Remerciements","Logiciels libres"};
    static const char *icons[]={"brand-mark","badge-check","layers-2"};
    static const char *credits[]={
        "hekate\n(c) 2018, naehrwert, st4rk\n(c) 2018-2026, CTCaer\n\nNyx\n(c) 2019-2026, CTCaer\n\nHekate 6.5.4 / Nyx 1.9.4",
        "derrek, nedwill, plutoo\nshuffle2, smea, thexyz\nyellows8\n\nfincs, hexkyz, SciresM\nShiny Quagsire, WinterMute\nswitchbrew",
        "Littlev Graphics Library\n(c) 2016-2018, Gabor Kiss-Vamosi\nFatFs R0.13c\n(c) 2006-2018, ChaN\n(c) 2018-2022, CTCaer\nbcl-1.2.0\n(c) 2003-2006, Marcus Geelnard\nblz (c) 2018, SciresM\nelfload (c) 2014, Owen Shepherd\n(c) 2018, M4xw"
    };
    for(u32 i=0;i<3;++i){
        lv_obj_t *card=nyx_nova_panel(page,34+i*410,140,392,350);
        nyx_nova_glyph(card,icons[i],24,24,24);
        lv_obj_t *title=nyx_nova_text(card,titles[i],64,24,304,NOVA_TITLE);nyx_nova_fit_text(title,40);
        lv_obj_t *text=nyx_nova_text(card,credits[i],24,86,344,NOVA_KICKER);
        static lv_style_t credits_style;lv_style_copy(&credits_style,&nova_text[NOVA_KICKER]);credits_style.text.letter_space=0;credits_style.text.line_space=2;credits_style.text.color=nova_text[NOVA_BODY].text.color;
        lv_label_set_style(text,&credits_style);nyx_nova_fit_text(text,250);
    }
    lv_obj_t *resources=nyx_nova_text(page,"HWTest-RCM 0.15 · Noto Sans (OFL) · Lucide (ISC) · Nova Pro (GPL)",34,520,1212,NOVA_SMALL);nyx_nova_fit_text(resources,44);
}

const char *nyx_nova_caption(const char *text)
{
    if(!text)return "";
    uint32_t at=0;u32 cp=lv_txt_encoded_next(text,&at);
    if(cp>=0xE000&&cp<=0xF8FF){text+=at;while(*text==' ')++text;}
    return text;
}
static lv_res_t nova_action_help(lv_obj_t *button)
{
    const char *help=lv_obj_get_free_ptr(lv_obj_get_parent(button));if(help&&*help)nyx_show_help(help);return LV_RES_OK;
}
typedef struct {lv_cont_ext_t container;char *help;} nova_action_ext;
static lv_signal_func_t nova_action_base_signal;
static lv_res_t nova_action_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    if(signal==LV_SIGNAL_CLEANUP){nova_action_ext *ext=lv_obj_get_ext_attr(object);free(ext->help);ext->help=NULL;}
    return nova_action_base_signal(object,signal,parameter);
}
lv_obj_t *nyx_nova_action(lv_obj_t *parent,const char *title,const char *description,const char *icon,
    lv_action_t action,u32 id,int x,int y,int width,int height)
{
    lv_obj_t *card=nyx_nova_panel(parent,x,y,width,height);
    if(!nova_action_base_signal)nova_action_base_signal=lv_obj_get_signal_func(card);
    nova_action_ext *ext=lv_obj_allocate_ext_attr(card,sizeof(*ext));
    if(ext){ext->help=NULL;if(description&&*description){ext->help=malloc(strlen(description)+1);if(ext->help)strcpy(ext->help,description);}lv_obj_set_free_ptr(card,ext->help);lv_obj_set_signal_func(card,nova_action_signal);}
    lv_obj_t *button=lv_btn_create(card,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    for(u32 state=LV_BTN_STYLE_REL;state<=LV_BTN_STYLE_INA;++state)lv_btn_set_style(button,state,&nova_clear);
    lv_obj_set_size(button,width,height);lv_obj_set_pos(button,0,0);lv_obj_set_free_num(button,id);
    lv_btn_set_action(button,LV_BTN_ACTION_CLICK,action);lv_btn_set_action(button,LV_BTN_ACTION_LONG_PR,nova_action_help);
    lv_obj_t *label=nyx_nova_text(button,nyx_nova_caption(title),76,18,width-94,NOVA_BODY);nyx_nova_fit_text(label,height<120?44:58);
    nyx_nova_icon_box(card,icon,18,20,42,false);
    if(description&&*description){int top=22+lv_obj_get_height(label);lv_obj_t *detail=nyx_nova_text(card,description,76,top,width-94,NOVA_SMALL);lv_label_set_style(detail,&nova_dim);nyx_nova_fit_text(detail,height-top-16);}
    return button;
}
