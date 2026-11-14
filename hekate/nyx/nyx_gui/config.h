/*
 * Copyright (c) 2018-2025 CTCaer
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <bdk.h>

#include "hos/hos.h"

typedef struct _hekate_config
{
	// Non-volatile config.
	u32 autoboot;
	u32 autoboot_list;
	u32 bootwait;
	u32 noticker;
	u32 backlight;
	u32 autohosoff;
	u32 autonogc;
	u32 updater2p;
	u32 bootprotect;
	// Global temporary config.
	bool t210b01;
	bool devmode;
	bool emummc_force_disable;
	bool rcm_patched;
	bool autorcm_enabled;
	u32  errors;
	hos_eks_mbr_t *eks;
} hekate_config;

#define HOME_LAYOUT_TILES    0
#define HOME_LAYOUT_CAROUSEL 1

typedef struct _nyx_config
{
	u32 theme_bg; // COLOR_BG_BASE_MIN - COLOR_BG_BASE_MAX.
	u32 theme_color;
	u32 entries_5_col;
	u32 timeoffset;
	u32 timedst;
	u32 home_screen;
	u32 verification;
	u32 ums_emmc_rw;
	u32 jc_disable;
	u32 jc_force_right;
	u32 bpmp_clock;
	u32 home_layout; // Appended: keep all previous field offsets unchanged.
	u32 language; // Appended: UI locale, persisted independently of home layout.
	char font_file[64]; // Empty selects the embedded font; basename under /fonts.
	u32 font_size; // 20..36 px, even values.
	u32 background_effect; // 0 none, 1 waves, 2 particles, 3 aurora.
	u32 background_speed; // 1 slow, 2 normal, 3 fast.
	u32 background_opacity; // 10..50 percent.
} nyx_config;

extern hekate_config h_cfg;
extern nyx_config n_cfg;

void set_default_configuration();
void set_nyx_default_configuration();
int create_config_entry();
int create_nyx_config_entry(bool force_unmount);
bool nyx_appearance_key(const char *key, const char *value);
void nyx_sanitize_appearance(void);

#endif /* _CONFIG_H_ */
