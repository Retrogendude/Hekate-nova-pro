/*
 * Copyright (c) 2018-2026 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <bdk.h>

#include "gui.h"
#include "gui_options.h"
#include "gui_info.h"
#include "gui_appearance.h"
#include "gui_choices.h"
#include "../config.h"
#include "../localization.h"
#include <libs/lvgl/lv_themes/lv_theme_hekate.h>
#include <libs/lvgl/lvgl.h>

#define CLOCK_MIN_YEAR 2025
#define CLOCK_MAX_YEAR (CLOCK_MIN_YEAR + 10)
#define CLOCK_YEARLIST "2025\n2026\n2027\n2028\n2029\n2030\n2031\n2032\n2033\n2034\n2035"

static lv_obj_t *autoboot_btn;
static bool autoboot_first_time = true;

static bool ini_changes_made = false;
static bool nyx_changes_made = false;

void nyx_options_clear_ini_changes_made()
{
	ini_changes_made = false;
}

bool nyx_options_get_ini_changes_made()
{
	return ini_changes_made;
}

static lv_res_t auto_hos_poweroff_toggle(lv_obj_t *btn)
{
	h_cfg.autohosoff = !h_cfg.autohosoff;
	ini_changes_made = true;

	if (!h_cfg.autohosoff)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static lv_res_t auto_nogc_toggle(lv_obj_t *btn)
{
	h_cfg.autonogc = !h_cfg.autonogc;
	ini_changes_made = true;

	if (!h_cfg.autonogc)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static lv_res_t _update_r2p_action(lv_obj_t *btn)
{
	h_cfg.updater2p = !h_cfg.updater2p;
	ini_changes_made = true;

	if (!h_cfg.updater2p)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static lv_res_t _win_autoboot_close_action(lv_obj_t * btn)
{
	if (!h_cfg.autoboot)
		lv_btn_set_state(autoboot_btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(autoboot_btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(autoboot_btn);

	lv_obj_t * win = lv_win_get_from_btn(btn);

	lv_obj_del(win);

	close_btn = NULL;

	return LV_RES_INV;
}

lv_obj_t *create_window_autoboot(const char *win_title)
{
	static lv_style_t win_bg_style;

	lv_style_copy(&win_bg_style, &lv_style_plain);
	win_bg_style.body.main_color = lv_theme_get_current()->bg->body.main_color;
	win_bg_style.body.grad_color = win_bg_style.body.main_color;

	lv_obj_t *win = lv_win_create(lv_scr_act(), NULL);
	lv_win_set_title(win, win_title);
	lv_win_set_style(win, LV_WIN_STYLE_BG, &win_bg_style);
	nyx_nova_window(win);

	close_btn = lv_win_add_btn(win, NULL, SYMBOL_CLOSE" Close", _win_autoboot_close_action);

	return win;
}

static lv_res_t _autoboot_disable_action(lv_obj_t *btn)
{
	h_cfg.autoboot = 0;
	h_cfg.autoboot_list = 0;
	ini_changes_made = true;

	lv_btn_set_state(autoboot_btn, LV_BTN_STATE_REL);
	nyx_generic_onoff_toggle(autoboot_btn);

	lv_obj_t * win = lv_win_get_from_btn(btn);

	lv_obj_del(win);

	close_btn = NULL;

	return LV_RES_OK;
}

lv_obj_t *auto_main_list;
lv_obj_t *auto_more_list;
static lv_res_t _autoboot_enable_main_action(lv_obj_t *btn)
{
	h_cfg.autoboot = lv_list_get_btn_index(auto_main_list, btn) + 1;
	h_cfg.autoboot_list = 0;
	ini_changes_made = true;

	lv_btn_set_state(autoboot_btn, LV_BTN_STATE_TGL_REL);
	nyx_generic_onoff_toggle(autoboot_btn);

	lv_obj_t *obj = lv_obj_get_parent(btn);
	for (int i = 0; i < 5; i++)
		obj = lv_obj_get_parent(obj);
	lv_obj_del(obj);

	close_btn = NULL;

	return LV_RES_INV;
}

static lv_res_t _autoboot_enable_more_action(lv_obj_t *btn)
{
	h_cfg.autoboot = lv_list_get_btn_index(auto_more_list, btn) + 1;
	h_cfg.autoboot_list = 1;
	ini_changes_made = true;

	lv_btn_set_state(autoboot_btn, LV_BTN_STATE_TGL_REL);
	nyx_generic_onoff_toggle(autoboot_btn);

	lv_obj_t *obj = lv_obj_get_parent(btn);
	for (int i = 0; i < 5; i++)
		obj = lv_obj_get_parent(obj);
	lv_obj_del(obj);

	close_btn = NULL;

	return LV_RES_INV;
}

static void _create_autoboot_window()
{
	lv_obj_t *win = create_window_autoboot(SYMBOL_GPS" Auto Boot");
	lv_win_add_btn(win, NULL, SYMBOL_POWER" Disable", _autoboot_disable_action);

	static lv_style_t h_style;
	lv_style_copy(&h_style, lv_theme_get_current()->panel);
	h_style.body.padding.inner = 0;
	h_style.body.padding.hor = LV_DPI - (LV_DPI / 4);
	h_style.body.padding.ver = LV_DPI / 6;

	// Main configurations container.
	lv_obj_t *h1 = lv_cont_create(win, NULL);
	lv_cont_set_style(h1, &h_style);
	lv_cont_set_fit(h1, false, true);
	lv_obj_set_width(h1, (LV_HOR_RES / 9) * 4);
	lv_obj_set_click(h1, false);
	lv_cont_set_layout(h1, LV_LAYOUT_OFF);

	lv_obj_t *label_sep = lv_label_create(h1, NULL);
	lv_label_set_static_text(label_sep, "");

	lv_obj_t *label_txt = lv_label_create(h1, NULL);
	lv_label_set_static_text(label_txt, "Main configurations");
	lv_obj_set_style(label_txt, lv_theme_get_current()->label.prim);
	lv_obj_align(label_txt, label_sep, LV_ALIGN_OUT_BOTTOM_LEFT, LV_DPI / 4, -(LV_DPI / 4));

	lv_obj_t *line_sep = lv_line_create(h1, NULL);
	static const lv_point_t line_pp[] = { {0, 0}, { LV_HOR_RES - (LV_DPI - (LV_DPI / 4)) * 2, 0} };
	lv_line_set_points(line_sep, line_pp, 2);
	lv_line_set_style(line_sep, lv_theme_get_current()->line.decor);
	lv_obj_align(line_sep, label_txt, LV_ALIGN_OUT_BOTTOM_LEFT, -(LV_DPI / 4), LV_DPI / 8);

	// Create list and populate it with Main boot entries.
	lv_obj_t *list_main = lv_list_create(h1, NULL);
	auto_main_list = list_main;
	lv_obj_align(list_main, line_sep, LV_ALIGN_OUT_BOTTOM_LEFT, LV_DPI / 4, LV_DPI / 4);

	lv_obj_set_size(list_main, LV_HOR_RES * 4 / 10, LV_VER_RES * 4 / 7);
	lv_list_set_single_mode(list_main, true);

	sd_mount();

	// Parse hekate main configuration.
	LIST_INIT(ini_sections);
	if (!ini_parse(&ini_sections, "bootloader/hekate_ipl.ini", false))
	{
		LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, &ini_sections, link)
		{
			if (!strcmp(ini_sec->name, "config") || (ini_sec->type != INI_CHOICE))
				continue;

			lv_list_add(list_main, NULL, ini_sec->name, _autoboot_enable_main_action);
		}

		ini_free(&ini_sections);
	}

	// More configuration container.
	lv_obj_t *h2 = lv_cont_create(win, NULL);
	lv_cont_set_style(h2, &h_style);
	lv_cont_set_fit(h2, false, true);
	lv_obj_set_width(h2, (LV_HOR_RES / 9) * 4);
	lv_obj_set_click(h2, false);
	lv_cont_set_layout(h2, LV_LAYOUT_OFF);
	lv_obj_align(h2, h1, LV_ALIGN_OUT_RIGHT_TOP, LV_DPI * 17 / 29, 0);

	label_sep = lv_label_create(h2, NULL);
	lv_label_set_static_text(label_sep, "");

	lv_obj_t *label_txt3 = lv_label_create(h2, NULL);
	lv_label_set_static_text(label_txt3, "Ini folder configurations");
	lv_obj_set_style(label_txt3, lv_theme_get_current()->label.prim);
	lv_obj_align(label_txt3, label_sep, LV_ALIGN_OUT_BOTTOM_LEFT, LV_DPI / 4, -LV_DPI / 11);

	line_sep = lv_line_create(h2, line_sep);
	lv_obj_align(line_sep, label_txt3, LV_ALIGN_OUT_BOTTOM_LEFT, -(LV_DPI / 2), LV_DPI / 8);
	lv_line_set_style(line_sep, lv_theme_get_current()->line.decor);

	// Create list and populate it with more cfg boot entries.
	lv_obj_t *list_more_cfg = lv_list_create(h2, NULL);
	auto_more_list = list_more_cfg;
	lv_obj_align(list_more_cfg, line_sep, LV_ALIGN_OUT_BOTTOM_LEFT, LV_DPI / 2, LV_DPI / 4);

	lv_obj_set_size(list_more_cfg, LV_HOR_RES * 4 / 10, LV_VER_RES * 4 / 7);
	lv_list_set_single_mode(list_more_cfg, true);

	// Parse all .ini files in ini folder.
	LIST_INIT(ini_list_sections);
	if (!ini_parse(&ini_list_sections, "bootloader/ini", true))
	{
		LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, &ini_list_sections, link)
		{
			if (!strcmp(ini_sec->name, "config") || (ini_sec->type != INI_CHOICE))
				continue;

			lv_list_add(list_more_cfg, NULL, ini_sec->name, _autoboot_enable_more_action);
		}

		ini_free(&ini_list_sections);
	}

	sd_unmount();
}

static lv_res_t _autoboot_hide_delay_action(lv_obj_t *btn)
{
	if (!autoboot_first_time)
		_create_autoboot_window();

	if (!h_cfg.autoboot && autoboot_first_time)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);
	autoboot_first_time = false;

	nyx_generic_onoff_toggle(btn);

	return LV_RES_INV;
}

static lv_res_t _autoboot_delay_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (h_cfg.bootwait != new_selection)
	{
		h_cfg.bootwait = new_selection;
		ini_changes_made = true;
	}

	return LV_RES_OK;
}

static lv_res_t _slider_brightness_action(lv_obj_t * slider)
{
	display_backlight_brightness(lv_slider_get_value(slider) - 20, 0);
	h_cfg.backlight = lv_slider_get_value(slider);
	ini_changes_made = true;

	return LV_RES_OK;
}

static lv_res_t _data_verification_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (n_cfg.verification != new_selection)
	{
		n_cfg.verification = new_selection;
		nyx_changes_made = true;
	}

	return LV_RES_OK;
}

static lv_res_t _entries_columns_action(lv_obj_t *btn)
{
	n_cfg.entries_5_col = !n_cfg.entries_5_col;
	nyx_changes_made = true;

	if (!n_cfg.entries_5_col)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static lv_res_t _save_nyx_options_action(lv_obj_t *btn)
{
	static const char * mbox_btn_map[] = {"\251", "\222OK!", "\251", ""};
	lv_obj_t * mbox = lv_mbox_create(lv_scr_act(), NULL);
	lv_mbox_set_recolor_text(mbox, true);

	int res = !create_nyx_config_entry(true);

	nyx_changes_made = false;

	if (res)
		lv_mbox_set_text(mbox, "#FF8000 Nyx Configuration#\n\n#96FF00 The configuration was saved to sd card!#");
	else
		lv_mbox_set_text(mbox, "#FF8000 Nyx Configuration#\n\n#FFDD00 Failed to save the configuration#\n#FFDD00 to sd card!#");
	lv_mbox_add_btns(mbox, mbox_btn_map, NULL);
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	return LV_RES_OK;
}

static const u16 theme_colors[18] = {
	0, 4, 13, 23, 33, 43, 54, 66, 89, 124, 167, 187, 200, 208, 231, 261, 291, 341
};

void create_flat_button(lv_obj_t *btn, int color_idx, lv_action_t action)
{
	/* Palette swatches represent a color, so they must keep their fill even
	 * when ordinary menu buttons use a transparent body. Reuse the fixed
	 * palette's styles instead of leaking two allocations on every opening. */
	static lv_style_t palette_rel[ARRAY_SIZE(theme_colors)];
	static lv_style_t palette_pr[ARRAY_SIZE(theme_colors)];
	u32 index = 0;
	for (u32 i = 0; i < ARRAY_SIZE(theme_colors); ++i)
		if (theme_colors[i] == color_idx) { index = i; break; }
	lv_color_t color = color_idx ? lv_color_hsv_to_rgb(color_idx, 100, 100) : nyx_nova_color(NOVA_COLOR_REFLECTION);
	lv_style_t *btn_onoff_rel_hos_style = &palette_rel[index];
	lv_style_t *btn_onoff_pr_hos_style  = &palette_pr[index];
	lv_style_copy(btn_onoff_rel_hos_style, lv_theme_get_current()->btn.rel);
	btn_onoff_rel_hos_style->body.empty       = 0;
	btn_onoff_rel_hos_style->body.opa         = LV_OPA_COVER;
	btn_onoff_rel_hos_style->body.shadow.width = 0;
	btn_onoff_rel_hos_style->body.main_color  = color;
	btn_onoff_rel_hos_style->body.grad_color  = btn_onoff_rel_hos_style->body.main_color;
	btn_onoff_rel_hos_style->body.padding.hor = 0;
	btn_onoff_rel_hos_style->body.radius      = 28;

	if (color_idx == 167)
	{
		btn_onoff_rel_hos_style->body.border.color = LV_COLOR_HEX(0x000000);
		btn_onoff_rel_hos_style->body.border.opa = LV_OPA_20;
		btn_onoff_rel_hos_style->body.border.width = 3;
	}

	lv_style_copy(btn_onoff_pr_hos_style, lv_theme_get_current()->btn.pr);
	btn_onoff_pr_hos_style->body.empty        = 0;
	btn_onoff_pr_hos_style->body.opa          = LV_OPA_COVER;
	btn_onoff_pr_hos_style->body.shadow.width = 0;
	btn_onoff_pr_hos_style->body.main_color   = color;
	btn_onoff_pr_hos_style->body.grad_color   = btn_onoff_pr_hos_style->body.main_color;
	btn_onoff_pr_hos_style->body.padding.hor  = 0;
	btn_onoff_pr_hos_style->body.border.color = LV_COLOR_HEX(0xFFFFFF);
	btn_onoff_pr_hos_style->body.border.opa   = LV_OPA_50;
	btn_onoff_pr_hos_style->body.border.width = 4;
	btn_onoff_pr_hos_style->body.radius       = 28;

	lv_btn_set_style(btn, LV_BTN_STYLE_REL,     btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_PR,      btn_onoff_pr_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_REL, btn_onoff_rel_hos_style);
	lv_btn_set_style(btn, LV_BTN_STYLE_TGL_PR,  btn_onoff_pr_hos_style);

	lv_btn_set_fit(btn, false, false);
	lv_obj_set_size(btn, LV_DPI * 7 / 11, LV_DPI * 7 / 11);
	lv_btn_set_toggle(btn, true);

	if (action)
		lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);
}

static lv_res_t _create_window_nyx_colors(lv_obj_t *button)
{
    return nyx_nova_theme_window(button);
}

typedef struct _time_edit_obj_t
{
	lv_obj_t *year;
	lv_obj_t *month;
	lv_obj_t *day;
	lv_obj_t *hour;
	lv_obj_t *min;
} time_edit_obj_t;

time_edit_obj_t clock_ctxt;

static lv_res_t _action_clock_edit(lv_obj_t *btns, const char * txt)
{
	int btn_idx = lv_btnm_get_pressed(btns);

	if (btn_idx == 1)
	{
		rtc_time_t time;
		max77620_rtc_get_time(&time);
		u32 epoch = max77620_rtc_date_to_epoch(&time);

		u32 year  = lv_roller_get_selected(clock_ctxt.year) + CLOCK_MIN_YEAR;
		u32 month = lv_roller_get_selected(clock_ctxt.month) + 1;
		u32 day   = lv_roller_get_selected(clock_ctxt.day) + 1;
		u32 hour  = lv_roller_get_selected(clock_ctxt.hour);
		u32 min   = lv_roller_get_selected(clock_ctxt.min);

		switch (month)
		{
		case 2:
			if (!(year % 4) && day > 29)
				day = 29;
			else if (day > 28)
				day = 28;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			if (day > 30)
				day = 30;
			break;
		}

		time.year  = year;
		time.month = month;
		time.day   = day;
		time.hour  = hour;
		time.min   = min;

		u32 new_epoch = max77620_rtc_date_to_epoch(&time);

		// Stored in u32 and allow overflow for integer offset casting.
		n_cfg.timeoffset = new_epoch - epoch;

		// If canceled set 1 for invalidating first boot clock edit.
		if (!n_cfg.timeoffset)
			n_cfg.timeoffset = 1;
		else
		{
			// Adjust for DST between 28 march and 28 october.
			// Good enough to cover all years as week info is not valid.
			u16 md = (time.month << 8) | time.day;
			if (n_cfg.timedst && md >= 0x31C && md < 0xA1C)
				n_cfg.timeoffset -= 3600; // Store time in non DST.
			max77620_rtc_set_epoch_offset((int)n_cfg.timeoffset);
		}

		nyx_changes_made = true;
	}

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

static lv_res_t _action_clock_edit_save(lv_obj_t *btns, const char * txt)
{
	_action_clock_edit(btns, txt);

	// Save if changes were made.
	if (nyx_changes_made)
		_save_nyx_options_action(NULL);

	return LV_RES_INV;
}

static lv_res_t _action_auto_dst_toggle(lv_obj_t *btn)
{
	n_cfg.timedst = !n_cfg.timedst;
	max77620_rtc_set_auto_dst(n_cfg.timedst);

	if (!n_cfg.timedst)
		lv_btn_set_state(btn, LV_BTN_STATE_REL);
	else
		lv_btn_set_state(btn, LV_BTN_STATE_TGL_REL);

	nyx_generic_onoff_toggle(btn);

	return LV_RES_OK;
}

static const u32 month_days[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static lv_res_t _action_date_validation(lv_obj_t *roller)
{
	u32 year  = lv_roller_get_selected(clock_ctxt.year) + CLOCK_MIN_YEAR;
	u32 month = lv_roller_get_selected(clock_ctxt.month) + 1;
	u32 day   = lv_roller_get_selected(clock_ctxt.day) + 1;

	// Adjust max day based on year and month.
	u32 max_mon_day = month_days[month - 1];
	u32 max_feb_day = !(year % 4) ? 29 : 28;
	if (month == 2 && day > max_feb_day)
		lv_roller_set_selected(clock_ctxt.day, max_feb_day - 1, false);
	else if (day > max_mon_day)
		lv_roller_set_selected(clock_ctxt.day, max_mon_day - 1, false);

	return LV_RES_OK;
}

static lv_res_t _create_mbox_clock_edit(lv_obj_t *btn)
{
	static lv_style_t mbox_style;
	lv_theme_t *th = lv_theme_get_current();
	lv_style_copy(&mbox_style, th->mbox.bg);
	mbox_style.body.padding.inner = LV_DPI / 10;

	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\251", "\222Done", "\222Cancel", "\251", "" };
	lv_obj_t *mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_style(mbox, LV_MBOX_STYLE_BG, &mbox_style);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	lv_mbox_set_text(mbox, "Enter #C7EA46 Date# and #C7EA46 Time# for Nyx\n"
						   "Used in all file operations and menu.\n"
						   "This doesn't alter the actual HW clock!");

	lv_obj_t *padding = lv_cont_create(mbox, NULL);
	lv_cont_set_fit(padding, true, false);
	lv_cont_set_style(padding, &lv_style_transp);
	lv_obj_set_height(padding, LV_DPI / 10);

	// Get current time.
	rtc_time_t time;
	max77620_rtc_get_time_adjusted(&time);

	// Normalize year if out of range.
	if (time.year < CLOCK_MIN_YEAR)
		time.year = CLOCK_MIN_YEAR;
	else if (time.year > CLOCK_MAX_YEAR)
		time.year = CLOCK_MAX_YEAR;

	time.year -= CLOCK_MIN_YEAR;

	lv_obj_t *h1 = lv_cont_create(mbox, NULL);
	lv_cont_set_fit(h1, true, true);

	// Create year roller.
	lv_obj_t *roller_year = lv_roller_create(h1, NULL);
	lv_roller_set_options(roller_year, CLOCK_YEARLIST);
	lv_roller_set_selected(roller_year, time.year, false);
	lv_roller_set_visible_row_count(roller_year, 3);
	lv_roller_set_action(roller_year, _action_date_validation);
	clock_ctxt.year = roller_year;

	// Create month roller.
	lv_obj_t *roller_month = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_month,
		"January\n"
		"February\n"
		"March\n"
		"April\n"
		"May\n"
		"June\n"
		"July\n"
		"August\n"
		"September\n"
		"October\n"
		"November\n"
		"December");
	lv_roller_set_selected(roller_month, time.month - 1, false);
	lv_obj_align(roller_month, roller_year, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	lv_roller_set_action(roller_month, _action_date_validation);
	clock_ctxt.month = roller_month;

	// Create day roller.
	static char days[256];
	days[0] = 0;
	for (u32 i = 1; i < 32; i++)
		s_printf(days + strlen(days), " %d \n", i);
	days[strlen(days) - 1] = 0;
	lv_obj_t *roller_day = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_day, days);
	lv_roller_set_selected(roller_day, time.day - 1, false);
	lv_roller_set_action(roller_day, _action_date_validation);
	lv_obj_align(roller_day, roller_month, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	clock_ctxt.day = roller_day;

	// Create hours roller.
	static char hours[256];
	hours[0] = 0;
	for (u32 i = 0; i < 24; i++)
		s_printf(hours + strlen(hours), " %d \n", i);
	hours[strlen(hours) - 1] = 0;
	lv_obj_t *roller_hour = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_hour, hours);
	lv_roller_set_selected(roller_hour, time.hour, false);
	lv_obj_align(roller_hour, roller_day, LV_ALIGN_OUT_RIGHT_MID, LV_DPI / 2, 0);
	clock_ctxt.hour = roller_hour;

	// Create minutes roller.
	static char minutes[512];
	minutes[0] = 0;
	for (u32 i = 0; i < 60; i++)
		s_printf(minutes + strlen(minutes), " %02d \n", i);
	minutes[strlen(minutes) - 1] = 0;
	lv_obj_t *roller_minute = lv_roller_create(h1, roller_year);
	lv_roller_set_options(roller_minute, minutes);
	lv_roller_set_selected(roller_minute, time.min, false);
	lv_obj_align(roller_minute, roller_hour, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
	clock_ctxt.min = roller_minute;

	// Add DST option.
	lv_obj_t *btn_dst = lv_btn_create(mbox, NULL);
	nyx_create_onoff_button(th, h1, btn_dst, SYMBOL_BRIGHTNESS" Auto Daylight Saving Time", _action_auto_dst_toggle, true);
	if (n_cfg.timedst)
		lv_btn_set_state(btn_dst, LV_BTN_STATE_TGL_REL);
	nyx_generic_onoff_toggle(btn_dst);

	// If btn is empty, save options also because it was launched from boot.
	lv_mbox_add_btns(mbox, mbox_btn_map, btn ? _action_clock_edit : _action_clock_edit_save);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	return LV_RES_OK;
}

void first_time_clock_edit(void *param)
{
	_create_mbox_clock_edit(NULL);
}

static lv_res_t _joycon_info_dump_action(lv_obj_t * btn)
{
	FIL fp;
	int error = 0;
	int cal_error = 0;
	bool is_l_hos = false;
	bool is_r_hos = false;
	u32 joycon_found = 0;
	bool nx_hoag = fuse_read_hw_type() == FUSE_NX_HW_TYPE_HOAG;
	jc_gamepad_rpt_t *jc_pad = jc_get_bt_pairing_info(&is_l_hos, &is_r_hos);

	char *data = (char *)malloc(SZ_16K);
	char *txt_buf = (char *)malloc(SZ_4K);

	if (!nx_hoag && !jc_pad)
		error = 255;

	// Try 2 times to get factory calibration data.
	for (u32 i = 0; i < 2; i++)
	{
		if (!error)
			cal_error = hos_dump_cal0();
		if (!cal_error)
			break;
	}

	if (cal_error && nx_hoag)
		error = cal_error;

	if (error)
		goto disabled_or_cal0_issue;

	if (nx_hoag)
		goto save_data;

	// Count valid joycon.
	joycon_found = jc_pad->bt_conn_l.type ? 1 : 0;
	if (jc_pad->bt_conn_r.type)
		joycon_found++;

	// Reset PC based for dumping.
	jc_pad->bt_conn_l.type = is_l_hos ? jc_pad->bt_conn_l.type : 0;
	jc_pad->bt_conn_r.type = is_r_hos ? jc_pad->bt_conn_r.type : 0;

save_data:
	error = sd_mount() ? 5 : 0;

	if (!error)
	{
		nx_emmc_cal0_t *cal0 = (nx_emmc_cal0_t *)cal0_buf;

		f_mkdir("switchroot");

		if (!nx_hoag)
		{
			// Save binary dump.
			memcpy(data, &jc_pad->bt_conn_l, sizeof(jc_bt_conn_t));
			memcpy(data + sizeof(jc_bt_conn_t), &jc_pad->bt_conn_r, sizeof(jc_bt_conn_t));

			error = sd_save_to_file((u8 *)data, sizeof(jc_bt_conn_t) * 2, "switchroot/joycon_mac.bin") ? 4 : 0;

			// Save readable dump.
			data[0] = 0;
			for (u32 i = 0; i < 2; i++)
			{
				jc_bt_conn_t *bt = !i ? &jc_pad->bt_conn_l : &jc_pad->bt_conn_r;
				s_printf(data + strlen(data),
					"[joycon_0%d]\ntype=%d\nmac=%02X:%02X:%02X:%02X:%02X:%02X\n"
					"host=%02X:%02X:%02X:%02X:%02X:%02X\n"
					"ltk=%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n\n",
					i, bt->type, bt->mac[0], bt->mac[1], bt->mac[2], bt->mac[3], bt->mac[4], bt->mac[5],
					bt->host_mac[0], bt->host_mac[1], bt->host_mac[2], bt->host_mac[3], bt->host_mac[4], bt->host_mac[5],
					bt->ltk[0], bt->ltk[1], bt->ltk[2], bt->ltk[3], bt->ltk[4], bt->ltk[5], bt->ltk[6], bt->ltk[7],
					bt->ltk[8], bt->ltk[9], bt->ltk[10], bt->ltk[11], bt->ltk[12], bt->ltk[13], bt->ltk[14], bt->ltk[15]);
			}

			if (!error)
				error = f_open(&fp, "switchroot/joycon_mac.ini", FA_WRITE | FA_CREATE_ALWAYS) ? 4 : 0;
			if (!error)
			{
				f_puts(data, &fp);
				f_close(&fp);
			}

			// Save IMU Calibration data.
			if (!error && !cal_error)
			{
				s_printf(data,
					"imu_type=%d\n\n"
					"acc_cal_off_x=0x%X\n"
					"acc_cal_off_y=0x%X\n"
					"acc_cal_off_z=0x%X\n"
					"acc_cal_scl_x=0x%X\n"
					"acc_cal_scl_y=0x%X\n"
					"acc_cal_scl_z=0x%X\n\n"

					"gyr_cal_off_x=0x%X\n"
					"gyr_cal_off_y=0x%X\n"
					"gyr_cal_off_z=0x%X\n"
					"gyr_cal_scl_x=0x%X\n"
					"gyr_cal_scl_y=0x%X\n"
					"gyr_cal_scl_z=0x%X\n\n"

					"device_bt_mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
					cal0->console_6axis_sensor_type,
					cal0->acc_offset[0],  cal0->acc_offset[1],  cal0->acc_offset[2],
					cal0->acc_scale[0],   cal0->acc_scale[1],   cal0->acc_scale[2],
					cal0->gyro_offset[0], cal0->gyro_offset[1], cal0->gyro_offset[2],
					cal0->gyro_scale[0],  cal0->gyro_scale[1],  cal0->gyro_scale[2],
					cal0->bd_mac[0], cal0->bd_mac[1], cal0->bd_mac[2], cal0->bd_mac[3], cal0->bd_mac[4], cal0->bd_mac[5]);

				error = f_open(&fp, "switchroot/switch.cal", FA_WRITE | FA_CREATE_ALWAYS) ? 4 : 0;
				if (!error)
				{
					f_puts(data, &fp);
					f_close(&fp);
				}
			}
		}
		else
		{
			jc_calib_t *stick_cal_l = (jc_calib_t *)cal0->analog_stick_cal_l;
			jc_calib_t *stick_cal_r = (jc_calib_t *)cal0->analog_stick_cal_r;

			// Save Lite Gamepad and IMU Calibration data.
			// Actual max/min are right/left and up/down offsets.
			// Sticks: 0x23: H1 (Hosiden), 0x25: H5 (Hosiden), 0x41: F1 (FIT), ?add missing?
			s_printf(data,
				"lite_cal_l_type=0x%X\n"
				"lite_cal_lx_lof=0x%X\n"
				"lite_cal_lx_cnt=0x%X\n"
				"lite_cal_lx_rof=0x%X\n"
				"lite_cal_ly_dof=0x%X\n"
				"lite_cal_ly_cnt=0x%X\n"
				"lite_cal_ly_uof=0x%X\n\n"

				"lite_cal_r_type=0x%X\n"
				"lite_cal_rx_lof=0x%X\n"
				"lite_cal_rx_cnt=0x%X\n"
				"lite_cal_rx_rof=0x%X\n"
				"lite_cal_ry_dof=0x%X\n"
				"lite_cal_ry_cnt=0x%X\n"
				"lite_cal_ry_uof=0x%X\n\n"

				"imu_type=%d\n\n"
				"acc_cal_off_x=0x%X\n"
				"acc_cal_off_y=0x%X\n"
				"acc_cal_off_z=0x%X\n"
				"acc_cal_scl_x=0x%X\n"
				"acc_cal_scl_y=0x%X\n"
				"acc_cal_scl_z=0x%X\n\n"

				"gyr_cal_off_x=0x%X\n"
				"gyr_cal_off_y=0x%X\n"
				"gyr_cal_off_z=0x%X\n"
				"gyr_cal_scl_x=0x%X\n"
				"gyr_cal_scl_y=0x%X\n"
				"gyr_cal_scl_z=0x%X\n\n"

				"device_bt_mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
				cal0->analog_stick_type_l,
				stick_cal_l->x_min, stick_cal_l->x_center, stick_cal_l->x_max,
				stick_cal_l->y_min, stick_cal_l->y_center, stick_cal_l->y_max,
				cal0->analog_stick_type_r,
				stick_cal_r->x_min, stick_cal_r->x_center, stick_cal_r->x_max,
				stick_cal_r->y_min, stick_cal_r->y_center, stick_cal_r->y_max,
				cal0->console_6axis_sensor_type,
				cal0->acc_offset[0],  cal0->acc_offset[1],  cal0->acc_offset[2],
				cal0->acc_scale[0],   cal0->acc_scale[1],   cal0->acc_scale[2],
				cal0->gyro_offset[0], cal0->gyro_offset[1], cal0->gyro_offset[2],
				cal0->gyro_scale[0],  cal0->gyro_scale[1],  cal0->gyro_scale[2],
				cal0->bd_mac[0], cal0->bd_mac[1], cal0->bd_mac[2], cal0->bd_mac[3], cal0->bd_mac[4], cal0->bd_mac[5]);
			if (!error)
				error = f_open(&fp, "switchroot/switch.cal", FA_WRITE | FA_CREATE_ALWAYS) ? 4 : 0;
			if (!error)
			{
				f_puts(data, &fp);
				f_close(&fp);
			}
		}

		sd_unmount();
	}

disabled_or_cal0_issue:;
	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\251", "\222OK", "\251", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 5);

	if (!error)
	{
		if (!nx_hoag)
		{
			s_printf(txt_buf,
				"Dumping to SD card finished!\n"
				"Saved to: #C7EA46 switchroot/joycon_mac.[bin/ini]#\n\n");

			bool success = true;

			// Check if pairing info was found.
			if (joycon_found == 2)
				strcat(txt_buf, "#C7EA46 Success!#\n#C7EA46 Found 2 out of 2 Joy-Con pairing data!#\n");
			else
			{
				s_printf(txt_buf + strlen(txt_buf), "#FF8000 Failed!#\n#FF8000 Warning:# Found #FFDD00 %d out of 2# pairing data!\n", joycon_found);
				success = false;
			}

			// Check if pairing was done in HOS.
			if (is_l_hos && is_r_hos)
				strcat(txt_buf, "#C7EA46 Both pairing data are HOS based!#");
			else if (!is_l_hos && is_r_hos)
			{
				strcat(txt_buf, "#FF8000 Warning:# #FFDD00 Left# pairing data is not HOS based!");
				success = false;
			}
			else if (is_l_hos && !is_r_hos)
			{
				strcat(txt_buf, "#FF8000 Warning:# #FFDD00 Right# pairing data is not HOS based!");
				success = false;
			}
			else
			{
				strcat(txt_buf, "#FF8000 Warning:# #FFDD00 No# pairing data is HOS based!");
				success = false;
			}

			if (!success)
				strcat(txt_buf,
					"\n\n#FFDD00 Make sure that both Joy-Con are connected,#\n"
					"#FFDD00 and that you paired them in HOS!#");

			if (cal_error)
				s_printf(txt_buf + strlen(txt_buf), "\n\n#FF8000 Warning: Failed (%d) to get IMU calibration!#", cal_error);
		}
		else
		{
			s_printf(txt_buf,
				"Dumping to SD card finished!\n"
				"Saved to: #C7EA46 switchroot/switch.cal#\n\n");
			strcat(txt_buf, "#C7EA46 Success!#\n#C7EA46 Found Lite Gamepad data!#\n");
		}
	}
	else
	{
		if (!nx_hoag)
			s_printf(txt_buf, "#FFDD00 Failed to dump Joy-Con pairing info!#\n#FFDD00 Error: %d#", error);
		else
			s_printf(txt_buf, "#FFDD00 Failed to get Lite Gamepad info!#\n#FFDD00 Error: %d#", error);
	}

	lv_mbox_set_text(mbox, txt_buf);

	lv_mbox_add_btns(mbox, mbox_btn_map, nyx_mbox_action); // Important. After set_text.

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	free(txt_buf);
	free(data);

	return LV_RES_OK;
}

/* Pending until Save & Reload: closing this window leaves n_cfg unchanged. */
static u32 home_layout_pending;

static void _home_layout_notice(const char *text)
{
	static const char *buttons[] = { "\251", "\222OK", "\251", "" };
	lv_obj_t *mbox = lv_mbox_create(lv_scr_act(), NULL);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 5);
	lv_mbox_set_text(mbox, text);
	lv_mbox_add_btns(mbox, buttons, NULL);
	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);
}

static lv_res_t _home_layout_action(lv_obj_t *ddlist)
{
	home_layout_pending = lv_ddlist_get_selected(ddlist) == HOME_LAYOUT_CAROUSEL
		? HOME_LAYOUT_CAROUSEL : HOME_LAYOUT_TILES;
	return LV_RES_OK;
}

static lv_res_t _save_home_layout(lv_obj_t *btn)
{
	(void)btn;
	bool was_mounted = sd_get_card_mounted();
	if (sd_mount())
	{
		_home_layout_notice("Carte SD indisponible. Aucun changement enregistre.");
		return LV_RES_OK;
	}
	if (f_stat("bootloader/sys/nyx.bin", NULL) != FR_OK)
	{
		if (!was_mounted)
			sd_unmount();
		_home_layout_notice("Impossible de recharger Nyx : bootloader/sys/nyx.bin est introuvable.\n\nAucun changement enregistre.");
		return LV_RES_OK;
	}

	u32 previous_layout = n_cfg.home_layout;
	n_cfg.home_layout = home_layout_pending;
	if (create_nyx_config_entry(true))
	{
		n_cfg.home_layout = previous_layout;
		_home_layout_notice("Echec de l'enregistrement. Verifiez la carte SD puis reessayez.");
		return LV_RES_OK;
	}

	nyx_changes_made = false;
	reload_nyx(NULL, false);
	/* Reload can return if the SD card or Nyx file disappears after saving. */
	_home_layout_notice("Disposition enregistree. Nyx n'a pas pu etre recharge.\n\nLe choix sera applique au prochain demarrage de Nyx.");
	return LV_RES_OK;
}

static lv_res_t _create_window_home_layout(lv_obj_t *btn)
{
	(void)btn;
	lv_theme_t *th = lv_theme_get_current();
	home_layout_pending = n_cfg.home_layout == HOME_LAYOUT_CAROUSEL
		? HOME_LAYOUT_CAROUSEL : HOME_LAYOUT_TILES;
	lv_obj_t *win = nyx_create_standard_window("Disposition de l'accueil", NULL);
	lv_win_add_btn(win, NULL, SYMBOL_SAVE" Enregistrer & recharger", _save_home_layout);

	lv_obj_t *title = lv_label_create(win, NULL);
	lv_label_set_style(title, th->label.prim);
	lv_label_set_text(title, "Choisir la disposition");
	lv_obj_set_pos(title, 50, 30);

	lv_obj_t *ddlist = lv_ddlist_create(win, NULL);
	lv_ddlist_set_draw_arrow(ddlist, true);
	lv_ddlist_set_options(ddlist, "Tuiles          \nCarrousel");
	lv_ddlist_set_selected(ddlist, home_layout_pending);
	lv_ddlist_set_action(ddlist, _home_layout_action);
	lv_obj_set_pos(ddlist, 50, 95);

	lv_obj_t *help = lv_label_create(win, NULL);
	lv_label_set_style(help, &hint_small_style);
	lv_label_set_long_mode(help, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(help, 1100);
	lv_label_set_text(help,
		"Tuiles : les fonctions sont affichees dans une grille.\n"
		"Carrousel : les fonctions sont presentees une par une.\n\n"
		"Enregistrer & recharger conserve les reglages Nyx sur la carte SD,\n"
		"puis recharge l'interface pour appliquer la disposition.\n"
		"Fermer cette fenetre annule le choix sans modifier les reglages.");
	lv_obj_set_pos(help, 50, 225);
	lv_obj_set_top(ddlist, true);
	return LV_RES_OK;
}
/* Locale choice is pending until the explicit save/reload action. */
static u32 language_pending;
static lv_obj_t *language_parent_close_btn;

static lv_res_t _language_action(lv_obj_t *ddlist)
{
	u32 selected = lv_ddlist_get_selected(ddlist);
	language_pending = selected < I18N_LANGUAGE_COUNT ? selected : I18N_EN;
	return LV_RES_OK;
}

static lv_res_t _close_language_window(lv_obj_t *btn)
{
	language_pending = n_cfg.language;
	lv_res_t result = nyx_win_close_action(btn);
	close_btn = language_parent_close_btn;
	language_parent_close_btn = NULL;
	/* Restore the parent Nyx Settings save action. */
	lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, _save_nyx_options_action);
	lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER);
	lv_obj_set_click(status_bar.mid, true);
	return result;
}

static lv_res_t _save_language(lv_obj_t *btn)
{
	(void)btn;
	bool was_mounted = sd_get_card_mounted();
	if (sd_mount())
	{
		_home_layout_notice(nyx_tr("SD card unavailable. The language was not changed."));
		return LV_RES_OK;
	}
	if (f_stat("bootloader/sys/nyx.bin", NULL) != FR_OK)
	{
		if (!was_mounted)
			sd_unmount();
		_home_layout_notice(nyx_tr("Nyx cannot be reloaded: bootloader/sys/nyx.bin is missing. The language was not changed."));
		return LV_RES_OK;
	}

	u32 previous = n_cfg.language;
	n_cfg.language = language_pending < I18N_LANGUAGE_COUNT ? language_pending : I18N_EN;
	if (create_nyx_config_entry(true))
	{
		n_cfg.language = previous;
		_home_layout_notice(nyx_tr("Could not save the language. Check the SD card and try again."));
		return LV_RES_OK;
	}
	nyx_changes_made = false;
	reload_nyx(NULL, false);
	_home_layout_notice(nyx_tr("Language saved. Nyx could not be reloaded. The choice will apply the next time Nyx starts."));
	return LV_RES_OK;
}

static lv_res_t _create_window_language(lv_obj_t *btn)
{
	(void)btn;
	language_pending = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
	language_parent_close_btn = close_btn;
	lv_obj_t *win = nyx_create_standard_window(nyx_tr("Language"), _close_language_window);
	lv_win_add_btn(win, NULL, nyx_tr("Save & reload"), _save_language);

	lv_obj_t *title = lv_label_create(win, NULL);
	lv_label_set_text(title, nyx_tr("Choose the interface language"));
	lv_obj_set_pos(title, 50, 30);
	lv_obj_t *ddlist = lv_ddlist_create(win, NULL);
	lv_ddlist_set_draw_arrow(ddlist, true);
	/* Stable persisted order; endonyms stay recognizable in every locale. */
	(lv_ddlist_set_options)(ddlist,
		"English\nFrançais\nEspañol\nItaliano\nРусский\nDeutsch\nPortuguês");
	lv_ddlist_set_selected(ddlist, language_pending);
	lv_ddlist_set_action(ddlist, _language_action);
	lv_obj_set_pos(ddlist, 50, 95);

	lv_obj_t *help = lv_label_create(win, NULL);
	lv_label_set_style(help, &hint_small_style);
	lv_label_set_long_mode(help, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(help, 1100);
	lv_label_set_text(help, nyx_tr(
		"Save & reload stores all Nyx settings on the SD card, then reloads the interface.\n"
		"Closing this window cancels the pending language choice."));
	lv_obj_set_pos(help, 50, 225);
	lv_obj_set_top(ddlist, true);
	/* A generic Save button must not misleadingly save the old locale. */
	lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP);
	lv_obj_set_click(status_bar.mid, false);
	return LV_RES_OK;
}

static lv_res_t _home_screen_action(lv_obj_t *ddlist)
{
	u32 new_selection = lv_obj_get_free_num(ddlist);
	if (n_cfg.home_screen != new_selection)
	{
		n_cfg.home_screen = new_selection;
		nyx_changes_made = true;
	}

	return LV_RES_OK;
}

static lv_res_t _action_nyx_options_save(lv_obj_t *btns, const char * txt)
{
	int btn_idx = lv_btnm_get_pressed(btns);

	nyx_mbox_action(btns, txt);

	if (!btn_idx)
		_save_nyx_options_action(NULL);

	return LV_RES_INV;
}

static void _check_nyx_changes()
{
	if (nyx_changes_made)
	{
		lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
		lv_obj_set_style(dark_bg, &mbox_darken);
		lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

		static const char * mbox_btn_map[] = { "\222Save", "\222Cancel", "" };
		lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
		lv_mbox_set_recolor_text(mbox, true);

		lv_mbox_set_text(mbox,
			"#FF8000 Nyx configuration#\n\n"
			"You changed the configuration!\n\n"
			"Do you want to save it?");

		lv_mbox_add_btns(mbox, mbox_btn_map, _action_nyx_options_save);
		lv_obj_set_width(mbox, LV_HOR_RES / 9 * 5);
		lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_top(mbox, true);

		nyx_changes_made = false;
	}
}

static lv_res_t _action_win_nyx_options_close(lv_obj_t *btn)
{
	// Hide status bar options save button.
	lv_obj_set_opa_scale(status_bar.mid, LV_OPA_0);
	lv_obj_set_click(status_bar.mid, false);

	lv_res_t res = nyx_win_close_action(btn);

	_check_nyx_changes();

	return res;
}

void nyx_options_appearance_active(bool active, bool saved)
{
	if (saved)
		nyx_changes_made = false;
	if (active) {
		lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP);
		lv_obj_set_click(status_bar.mid, false);
	} else {
		lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, nyx_options_save_all);
		bool visible = close_btn != NULL || nyx_options_has_changes();
		lv_obj_set_opa_scale(status_bar.mid, visible ? LV_OPA_COVER : LV_OPA_TRANSP);
		lv_obj_set_click(status_bar.mid, visible);
	}
}


/* Compact pages keep controls above the persistent status bar. */
static void settings_page(lv_obj_t *page)
{
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

static lv_obj_t *_settings_card(lv_obj_t *parent, u32 index, u32 rows)
{
    lv_coord_t width = (lv_obj_get_width(parent) - 80) / 2;
    lv_coord_t height = (lv_obj_get_height(parent) - 32 - (rows - 1) * 12) / rows;
    lv_obj_t *card = lv_cont_create(parent, NULL);
    lv_cont_set_style(card, lv_theme_get_current()->panel);
    lv_cont_set_fit(card, false, false); lv_cont_set_layout(card, LV_LAYOUT_OFF);
    lv_obj_set_size(card, width, height);
    lv_obj_set_pos(card, 32 + index % 2 * (width + 16), 16 + index / 2 * (height + 12));
    return card;
}

static lv_res_t settings_help_pressed(lv_obj_t *button)
{
    nyx_show_help(lv_obj_get_free_ptr(button));
    return LV_RES_OK;
}

static void _settings_help(lv_obj_t *card, const char *help)
{
    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false);
    lv_obj_set_size(button, 36, 36);
    lv_obj_set_pos(button, lv_obj_get_width(card) - 46, 10);
    lv_obj_set_free_ptr(button, (void *)help);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, settings_help_pressed);
    lv_obj_t *mark = lv_label_create(button, NULL); lv_label_set_style(mark, &hint_small_style);
    lv_label_set_text(mark, "?");
}

static lv_obj_t *_settings_heading(lv_obj_t *card, const char *title)
{
    lv_obj_t *label = nyx_nova_text(card,nyx_nova_caption(title),16,12,lv_obj_get_width(card)-80,NOVA_TITLE);
    nyx_nova_fit_text(label,48);
    return label;
}

static lv_obj_t *_settings_choices(lv_obj_t *card, const char *title,
    const char *options, u32 selected, u32 columns, lv_action_t action, bool raw)
{
    lv_obj_t *heading = _settings_heading(card, title);
    int y = 20 + lv_obj_get_height(heading);
    int rows = 1; for (const char *p = options; *p; ++p) if (*p == '\n') ++rows;
    rows = (rows + columns - 1) / columns;
    int height = (lv_obj_get_height(card) - y - 14 - (rows - 1) * 8) / rows;
    lv_obj_t *choices = nyx_choices_create(card, options, selected, columns,
        lv_obj_get_width(card) - 32, height, action, 0, raw);
    if (choices) lv_obj_set_pos(choices, 16, y);
    return choices;
}

static lv_obj_t *_settings_button(lv_obj_t *card, const char *title, lv_action_t action)
{
    lv_obj_t *button = lv_btn_create(card, NULL);
    lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    static lv_style_t clear;lv_style_copy(&clear,&lv_style_transp_tight);
    lv_btn_set_style(button,LV_BTN_STYLE_REL,&clear);lv_btn_set_style(button,LV_BTN_STYLE_PR,&clear);
    lv_obj_set_size(button, lv_obj_get_width(card) - 74, lv_obj_get_height(card) - 28);
    lv_obj_set_pos(button, 14, 14); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    const char *icon=(strstr(title,"Color")||strstr(title,"couleurs"))?"paintbrush":strstr(title,"Clock")?"clock-3":strstr(title,"Joy-Con")?"gamepad-2":"settings-2";
    nyx_nova_icon_box(button,icon,10,(lv_obj_get_height(button)-44)/2,44,false);
    lv_obj_t *label=nyx_nova_text(button,nyx_nova_caption(title),76,0,lv_obj_get_width(button)-92,NOVA_TITLE);
    nyx_nova_fit_text(label,lv_obj_get_height(button)-24);lv_obj_set_y(label,(lv_obj_get_height(button)-lv_obj_get_height(label))/2);
    return button;
}

static lv_obj_t *_settings_toggle(lv_obj_t *card, const char *title, lv_action_t action, bool enabled)
{
    lv_obj_t *button = nyx_nova_toggle(card, nyx_nova_caption(title), action, enabled,
        lv_obj_get_width(card) - 74, lv_obj_get_height(card) - 28);
    if (button) lv_obj_set_pos(button, 14, 14);
    return button;
}

lv_res_t create_win_nyx_options(lv_obj_t *parent_button)
{
    (void)parent_button;
    lv_obj_t *win = nyx_create_standard_window(SYMBOL_HOME" Nyx Settings", _action_win_nyx_options_close);
    lv_win_add_btn(win, NULL, nyx_tr("Appearance"), nyx_create_appearance_window);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win);
    settings_page(ext->page);
    lv_obj_t *tabs = lv_tabview_create(ext->page, NULL);
    lv_obj_set_size(tabs, lv_obj_get_width(ext->page), lv_obj_get_height(ext->page));
    lv_obj_set_pos(tabs, 0, 0); lv_tabview_set_sliding(tabs, false);
    lv_obj_t *interface_page = lv_tabview_add_tab(tabs, "Interface"); settings_page(interface_page);
    lv_obj_t *system = lv_tabview_add_tab(tabs, "System"); settings_page(system);
    lv_obj_t *card = _settings_card(interface_page, 0, 2);
    _settings_button(card, SYMBOL_EDIT" Appearance & language", nyx_create_appearance_window);
    _settings_help(card, "Choose your language, reading comfort and background in one place.");
    card = _settings_card(interface_page, 1, 2);
    _settings_button(card, SYMBOL_COPY" Thèmes et couleurs", _create_window_nyx_colors);
    _settings_help(card, "Couleurs indépendantes, fond d’écran et transparence.");
    card = _settings_card(interface_page, 2, 2);
    _settings_choices(card, SYMBOL_HOME" Home Screen", "Main menu\nAll Configs\nLaunch\nMore Configs", n_cfg.home_screen, 2, _home_screen_action, false);
    _settings_help(card, "Select what screen to show on Nyx boot.\nAll Configs combines extra configurations into the launch screen.");
    card = _settings_card(interface_page, 3, 2);
    _settings_toggle(card, SYMBOL_GPS" Extended Boot Entries", _entries_columns_action, n_cfg.entries_5_col);
    _settings_help(card, "Increase the number of boot entries shown on each row.");
    card = _settings_card(system, 0, 2);
    _settings_button(card, SYMBOL_DOWNLOAD" Dump Joy-Con BT", _joycon_info_dump_action);
    _settings_help(card, "Save Switch and Joy-Con MAC addresses and pairing keys for Android and Linux.");
    card = _settings_card(system, 1, 2);
    _settings_choices(card, SYMBOL_MODULES_ALT" Data Verification", "Off\nSparse\nFull\nHashes", n_cfg.verification, 2, _data_verification_action, false);
    _settings_help(card, "Set the type of data verification done for backup and restore.\nCan be canceled without losing the backup/restore.");
    card = _settings_card(system, 2, 2);
    _settings_button(card, SYMBOL_CLOCK" Clock (Offset)", _create_mbox_clock_edit);
    _settings_help(card, "Change the clock offset manually. The date and time are also used for SD card operations.");
    lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, _save_nyx_options_action);
    lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    return LV_RES_OK;
}

void create_tab_boot_options(lv_theme_t *theme, lv_obj_t *parent)
{
    (void)theme; settings_page(parent);
    lv_obj_t *card = _settings_card(parent, 0, 3);
    autoboot_btn = _settings_toggle(card, SYMBOL_GPS" Auto Boot", _autoboot_hide_delay_action, h_cfg.autoboot);
    autoboot_first_time = false;
    _settings_help(card, "Choose which boot entry or payload to automatically boot when injecting.");
    card = _settings_card(parent, 1, 3);
    _settings_choices(card, SYMBOL_CLOCK" Boot Time Delay", "0\n1\n2\n3\n4\n5\n6", h_cfg.bootwait, 7, _autoboot_delay_action, true);
    _settings_help(card, "Set how long to show the boot logo when automatic boot is enabled.\nPress VOL- during that time to enter the Hekate menu.");
    card = _settings_card(parent, 2, 3);
    _settings_toggle(card, SYMBOL_CHIP" Auto NoGC", auto_nogc_toggle, h_cfg.autonogc);
    _settings_help(card, "Check the fuses and automatically apply the game-card protection patch when needed.");
    card = _settings_card(parent, 3, 3);
    _settings_toggle(card, SYMBOL_POWER" Auto HOS Power Off", auto_hos_poweroff_toggle, h_cfg.autohosoff);
    _settings_help(card, "Automatically power off on the next payload injection after shutting down from HOS.");
    card = _settings_card(parent, 4, 3);
    lv_obj_t *heading = _settings_heading(card, SYMBOL_BRIGHTNESS" Backlight");
    lv_obj_t *slider = lv_slider_create(card, NULL);
    lv_obj_set_size(slider, lv_obj_get_width(card) - 48, 40);
    lv_obj_set_pos(slider, 24, lv_obj_get_height(heading) + 34);
    lv_bar_set_range(slider, 30, 220); lv_bar_set_value(slider, h_cfg.backlight);
    lv_slider_set_action(slider, _slider_brightness_action);
    _settings_help(card, "Set backlight brightness.");
    card = _settings_card(parent, 5, 3);
    _settings_toggle(card, SYMBOL_REFRESH" Update Reboot 2 Payload", _update_r2p_action, h_cfg.updater2p);
    _settings_help(card, "When FSS0 is used, check and update the reboot-to-payload image to Hekate.");
}

bool nyx_options_has_changes(void) { return ini_changes_made || nyx_changes_made; }

bool nyx_options_commit_boot(void)
{
    if (!ini_changes_made) return true;
    if (create_config_entry()) return false;
    ini_changes_made = false; return true;
}

lv_res_t nyx_options_save_all(lv_obj_t *button)
{
    (void)button;
    if (sd_mount()) { nyx_show_help("SD card unavailable. Nothing was saved."); return LV_RES_OK; }
    bool boot_saved = !create_config_entry();
    if (boot_saved) ini_changes_made = false;
    bool nyx_saved = boot_saved && !create_nyx_config_entry(true);
    if (!boot_saved) sd_unmount();
    if (boot_saved && nyx_saved) {
        nyx_changes_made = false;
        if (status_bar.mid) { lv_obj_set_opa_scale(status_bar.mid, LV_OPA_TRANSP); lv_obj_set_click(status_bar.mid, false); }
        nyx_show_help("Settings saved.");
    } else nyx_show_help(boot_saved ? "Boot options were saved. Nyx settings could not be saved. Check the SD card and retry." : "Could not save settings. Check the SD card and retry.");
    return LV_RES_OK;
}

static lv_obj_t *nova_contour_value;
static lv_res_t _nova_verification_toggle(lv_obj_t *button)
{
    n_cfg.verification = n_cfg.verification ? 0 : 2; nyx_changes_made = true;
    lv_btn_set_state(button, n_cfg.verification ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(button); return LV_RES_OK;
}
static lv_res_t _nova_halo_toggle(lv_obj_t *button)
{
    n_cfg.nova_halo = !n_cfg.nova_halo; nyx_changes_made = true;
    lv_btn_set_state(button, n_cfg.nova_halo ? LV_BTN_STATE_TGL_REL : LV_BTN_STATE_REL);
    nyx_generic_onoff_toggle(button); nyx_nova_refresh_halos();
    if (nova_contour_value) lv_label_set_text(nova_contour_value, n_cfg.nova_halo ? "Reflet animé" : "Contour fixe");
    return LV_RES_OK;
}
static lv_res_t _nova_language_action(lv_obj_t *button)
{ nyx_appearance_open_language(lv_obj_get_free_num(button)); return LV_RES_OK; }
static lv_res_t _nova_size_action(lv_obj_t *button)
{ nyx_appearance_open_size(lv_obj_get_free_num(button)); return LV_RES_OK; }
static lv_res_t _nova_font_action(lv_obj_t *button)
{ (void)button; nyx_appearance_open_font(); return LV_RES_OK; }
static lv_res_t _nova_language_menu(lv_obj_t *button)
{ (void)button; nyx_appearance_open_language(n_cfg.language); return LV_RES_OK; }
static lv_res_t _nova_boot_options(lv_obj_t *button)
{
    (void)button; lv_obj_t *win = nyx_create_standard_window("Options système", NULL);
    lv_win_ext_t *ext = lv_obj_get_ext_attr(win); create_tab_boot_options(lv_theme_get_current(), ext->page);
    if (status_bar.mid) {
        lv_btn_set_action(status_bar.mid, LV_BTN_ACTION_CLICK, nyx_options_save_all);
        lv_obj_set_opa_scale(status_bar.mid, LV_OPA_COVER); lv_obj_set_click(status_bar.mid, true);
    }
    return LV_RES_OK;
}

static lv_obj_t *_nova_heading_button(lv_obj_t *panel, const char *title, const char *icon, lv_action_t action)
{
    static lv_style_t transparent; lv_style_copy(&transparent, &lv_style_transp_tight);
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &transparent); lv_btn_set_style(button, LV_BTN_STYLE_PR, &transparent);
    lv_obj_set_size(button, lv_obj_get_width(panel) - 48, 42); lv_obj_set_pos(button, 24, 16);
    lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action); nyx_nova_glyph(button, icon, 0, 8, 24);
    lv_obj_t *label = nyx_nova_text(button, title, 38, 8, lv_obj_get_width(button) - 40, NOVA_TITLE); nyx_nova_fit_text(label, 30);
    return button;
}

static lv_obj_t *_nova_setting_line(lv_obj_t *panel, const char *title, const char *value, int y, lv_action_t action)
{
    static lv_style_t transparent, accent, line;
    lv_style_copy(&transparent, &lv_style_transp_tight);
    lv_style_copy(&accent, nyx_nova_text_style(NOVA_SMALL)); accent.text.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_style_copy(&line, &lv_style_plain); line.body.main_color = line.body.grad_color = nyx_nova_color(NOVA_COLOR_BORDER); line.body.border.width = 0;
    int width = lv_obj_get_width(panel) - 48;
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &transparent); lv_btn_set_style(button, LV_BTN_STYLE_PR, &transparent);
    lv_obj_set_size(button, width, 44); lv_obj_set_pos(button, 24, y); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *title_label = nyx_nova_text(button, title, 0, 12, width / 2 - 10, NOVA_BODY); nyx_nova_fit_text(title_label, 28);
    lv_obj_t *value_label = nyx_nova_text(button, value, width / 2, 12, width / 2, NOVA_SMALL);
    lv_label_set_style(value_label, &accent); lv_label_set_align(value_label, LV_LABEL_ALIGN_RIGHT); nyx_nova_fit_text(value_label, 28);
    lv_obj_t *separator = lv_cont_create(panel, NULL); lv_cont_set_style(separator, &line); lv_cont_set_fit(separator, false, false);
    lv_obj_set_size(separator, width, 1); lv_obj_set_pos(separator, 24, y + 43); lv_obj_set_click(separator, false);
    return value_label;
}

static lv_obj_t *_nova_chip(lv_obj_t *panel, const char *text, int x, int y, int width, bool selected, lv_action_t action, u32 id)
{
    static lv_style_t normal, active, label_active;
    lv_style_copy(&normal, lv_theme_get_current()->btn.rel);
    normal.body.empty = 0; normal.body.main_color = normal.body.grad_color = nyx_nova_color(NOVA_COLOR_BACKGROUND);
    normal.body.radius = 5; normal.body.border.width = 1; normal.body.border.color = nyx_nova_color(NOVA_COLOR_BORDER);
    normal.body.padding.hor = normal.body.padding.ver = 0;
    lv_style_copy(&active, &normal); active.body.border.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_style_copy(&label_active, nyx_nova_text_style(NOVA_SMALL)); label_active.text.color = nyx_nova_color(NOVA_COLOR_REFLECTION);
    lv_obj_t *button = lv_btn_create(panel, NULL); lv_btn_set_fit(button, false, false); lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, selected ? &active : &normal); lv_btn_set_style(button, LV_BTN_STYLE_PR, &active);
    lv_obj_set_size(button, width, 36); lv_obj_set_pos(button, x, y); lv_obj_set_free_num(button, id); lv_btn_set_action(button, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *label = nyx_nova_text(button, text, 4, 9, width - 8, NOVA_SMALL);
    if (selected) lv_label_set_style(label, &label_active);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER); nyx_nova_fit_text(label, 24); return button;
}

void create_tab_options(lv_theme_t *theme, lv_obj_t *parent)
{
    (void)theme; nyx_nova_page(parent, 4);
    int height = lv_obj_get_height(parent) - 168;
    int left_width = (LV_HOR_RES - 68 - 22) * 113 / 213;
    int right_width = LV_HOR_RES - 68 - 22 - left_width;
    lv_obj_t *system = nyx_nova_panel(parent, 34, 140, left_width, height);
    lv_obj_t *reading = nyx_nova_panel(parent, 34 + left_width + 22, 140, right_width, height);
    _nova_heading_button(system, "Options système", "toggle-right", _nova_boot_options);
    _nova_heading_button(reading, "Apparence et lecture", "paintbrush", create_win_nyx_options);
    int row_height = (height - 64 - 16) / 4;
    nyx_nova_switch_row(system, "Auto NoGC", "Protection du lecteur de cartes.", "shield-check", auto_nogc_toggle, h_cfg.autonogc, 24, 64, left_width - 48, row_height);
    nyx_nova_switch_row(system, "Extinction HOS", "Arrêt automatique après HOS.", "power", auto_hos_poweroff_toggle, h_cfg.autohosoff, 24, 64 + row_height, left_width - 48, row_height);
    nyx_nova_switch_row(system, "Vérification des sauvegardes", "Contrôler les données écrites.", "badge-check", _nova_verification_toggle, n_cfg.verification, 24, 64 + 2 * row_height, left_width - 48, row_height);
    nyx_nova_switch_row(system, "Effets de sélection", "Reflet léger sur les contours actifs.", "sparkles", _nova_halo_toggle, n_cfg.nova_halo, 24, 64 + 3 * row_height, left_width - 48, row_height);
    const char *language_names[] = {"English", "Français", "Español", "Italiano", "Русский", "Deutsch", "Português"};
    _nova_setting_line(reading, "Language", language_names[n_cfg.language < 7 ? n_cfg.language : 0], 64, _nova_language_menu);
    const u32 order[] = {1,0,2,3,4,5,6}; const char *codes[] = {"EN","FR","ES","IT","RU","DE","PT"};
    for (u32 i = 0; i < 7; ++i) _nova_chip(reading, codes[order[i]], 24 + i * 50, 118, 42, n_cfg.language == order[i], _nova_language_action, order[i]);
    char face[64]; strcpy(face, n_cfg.font_file[0] ? n_cfg.font_file : "Inter"); char *extension = strstr(face, ".nyxf"); if (extension) *extension = 0;
    lv_obj_t *font_name = _nova_setting_line(reading, "Font", face, 170, _nova_font_action); (lv_label_set_text)(font_name, face);
    char size[20]; s_printf(size, "%d px", n_cfg.font_size);
    _nova_setting_line(reading, "Text size", size, 226, _nova_font_action);
    const u32 sizes[] = {24,28,32};
    for (u32 i = 0; i < 3; ++i) { char number[12]; s_printf(number, "%d", sizes[i]); _nova_chip(reading, number, 24 + i * 66, 280, 58, n_cfg.font_size == sizes[i], _nova_size_action, sizes[i]); }
    nova_contour_value = _nova_setting_line(reading, "Sélection", n_cfg.nova_halo ? "Reflet animé" : "Contour fixe", 334, NULL);
}
