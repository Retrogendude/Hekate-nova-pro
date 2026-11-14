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

#include <string.h>
#include <stdlib.h>

#include <bdk.h>

#include "config.h"
#include "localization.h"
#include <libs/fatfs/ff.h>

void set_default_configuration()
{
	h_cfg.t210b01 = hw_get_chip_id() == GP_HIDREV_MAJOR_T210B01;
	h_cfg.devmode = fuse_read_hw_state();

	h_cfg.autoboot      = 0;
	h_cfg.autoboot_list = 0;
	h_cfg.bootwait      = 3;
	h_cfg.noticker      = 0; //! TODO: Add GUI option.
	h_cfg.backlight     = 100;
	h_cfg.autohosoff    = h_cfg.t210b01 ? 1 : 0;
	h_cfg.autonogc      = 1;
	h_cfg.updater2p     = 0;
	h_cfg.bootprotect   = 0;

	h_cfg.errors = 0;
	h_cfg.eks = NULL;
	h_cfg.rcm_patched = fuse_check_patched_rcm();
	h_cfg.autorcm_enabled = false;
	h_cfg.emummc_force_disable = false;

	sd_power_cycle_time_start = 0;
}

void set_nyx_default_configuration()
{
	n_cfg.theme_bg       = 0x10151D;
	n_cfg.theme_color    = 0;
	n_cfg.entries_5_col  = 0;
	n_cfg.timeoffset     = 0;
	n_cfg.timedst        = 1;
	n_cfg.home_screen    = 0;
	n_cfg.home_layout    = HOME_LAYOUT_TILES;
	n_cfg.language       = I18N_EN;
	n_cfg.font_file[0]   = 0;
	n_cfg.font_size      = 30;
	n_cfg.background_effect = 0;
	n_cfg.background_speed = 2;
	n_cfg.background_opacity = 20;
	n_cfg.nova_halo = 1;
	n_cfg.nova_text_color = 0xF2F5FA;
	n_cfg.nova_border_color = 0x334050;
	n_cfg.nova_reflection_color = 0x55C9F1;
	n_cfg.nova_menu_opacity = 25;
	n_cfg.nova_icon_opacity = 100;
	strcpy(n_cfg.wallpaper_file, "Nova-Carbon.bmp");
	n_cfg.nova_logo_color = 0x55C9F1;
	n_cfg.nova_icon_color = 0x55C9F1;
	n_cfg.nova_selection_color = 0x1C2937;
	n_cfg.launch_image[0] = 0;
	n_cfg.boot_logo_color = n_cfg.boot_bar_color = 0x55C9F1;
	n_cfg.verification   = 1;
	n_cfg.ums_emmc_rw    = 0;
	n_cfg.jc_disable     = 0;
	n_cfg.jc_force_right = 0;
	n_cfg.bpmp_clock     = 0;
}

static bool valid_font_basename(const char *value)
{
	u32 length = strlen(value);
	if (length >= sizeof(n_cfg.font_file) || value[0] == '.' || strstr(value, "..")) return false;
	for (u32 i = 0; i < length; ++i) {
		char c = value[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == ' ')) return false;
	}
	return !length || (length > 5 && value[length - 5] == '.' &&
		(value[length - 4] | 32) == 'n' && (value[length - 3] | 32) == 'y' &&
		(value[length - 2] | 32) == 'x' && (value[length - 1] | 32) == 'f');
}

static bool valid_wallpaper_basename(const char *value)
{
	u32 length = strlen(value);
	if (!length) return true;
	if (length >= sizeof(n_cfg.wallpaper_file) || length < 5 || value[0] == '.' || strstr(value, "..")) return false;
	for (u32 i = 0; i < length; ++i) {
		char c = value[i];
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == ' ')) return false;
	}
	return value[length - 4] == '.' && (value[length - 3] | 32) == 'b' &&
		(value[length - 2] | 32) == 'm' && (value[length - 1] | 32) == 'p';
}

static bool valid_launch_basename(const char *value)
{
    if(valid_wallpaper_basename(value))return true;
    u32 n=strlen(value);if(n<5||n>=64)return false;
    char bmp[64];strcpy(bmp,value);bmp[n-3]='b';bmp[n-2]='m';bmp[n-1]='p';
    return (value[n-3]|32)=='p'&&(value[n-2]|32)=='n'&&(value[n-1]|32)=='g'&&valid_wallpaper_basename(bmp);
}
void nyx_sanitize_appearance(void)
{
	if(n_cfg.theme_bg>0xFFFFFF)n_cfg.theme_bg=0x141A19;
	if(n_cfg.theme_color>360)n_cfg.theme_color=0;
	/* Ensure corrupted configuration cannot leave an unterminated path. */
	n_cfg.font_file[sizeof(n_cfg.font_file) - 1] = 0;
	if (!valid_font_basename(n_cfg.font_file)) n_cfg.font_file[0] = 0;
	if (n_cfg.font_size < 20 || n_cfg.font_size > 36 || (n_cfg.font_size & 1)) n_cfg.font_size = 30;
	if (n_cfg.background_effect > 3) n_cfg.background_effect = 0;
	if (n_cfg.background_speed < 1 || n_cfg.background_speed > 3) n_cfg.background_speed = 2;
	if (n_cfg.background_opacity < 10 || n_cfg.background_opacity > 50) n_cfg.background_opacity = 20;
	if (n_cfg.nova_halo > 1) n_cfg.nova_halo = 1;
	if (n_cfg.nova_logo_color > 0xFFFFFF) n_cfg.nova_logo_color = 0x55C9F1;
	if (n_cfg.nova_icon_color > 0xFFFFFF) n_cfg.nova_icon_color = 0x55C9F1;
	if (n_cfg.nova_selection_color > 0xFFFFFF) n_cfg.nova_selection_color = 0x1C2937;
	if (n_cfg.nova_text_color > 0xFFFFFF) n_cfg.nova_text_color = 0xF2F5FA;
	if (n_cfg.nova_border_color > 0xFFFFFF) n_cfg.nova_border_color = 0x334050;
	if (n_cfg.nova_reflection_color > 0xFFFFFF) n_cfg.nova_reflection_color = 0x55C9F1;
	if (n_cfg.nova_menu_opacity > 100) n_cfg.nova_menu_opacity = 100;
	if (n_cfg.nova_icon_opacity > 100) n_cfg.nova_icon_opacity = 100;
	n_cfg.wallpaper_file[sizeof(n_cfg.wallpaper_file) - 1] = 0;
	if (!valid_wallpaper_basename(n_cfg.wallpaper_file)) n_cfg.wallpaper_file[0] = 0;
	n_cfg.launch_image[sizeof(n_cfg.launch_image) - 1] = 0;
	if (!valid_launch_basename(n_cfg.launch_image)) n_cfg.launch_image[0] = 0;
	if (n_cfg.boot_logo_color > 0xFFFFFF) n_cfg.boot_logo_color = 0x55C9F1;
	if (n_cfg.boot_bar_color > 0xFFFFFF) n_cfg.boot_bar_color = 0x55C9F1;
}

static u32 appearance_number(const char *value, u32 fallback)
{
	u32 result = 0, length = 0;
	while (*value) {
		if (*value < '0' || *value > '9' || ++length > 2) return fallback;
		result = result * 10 + *value++ - '0';
	}
	return length ? result : fallback;
}

static u32 appearance_rgb(const char *value, u32 fallback)
{
	u32 color = 0;
	if (strlen(value) != 6) return fallback;
	for (u32 i = 0; i < 6; ++i) {
		char c = value[i] | 32;
		if (c >= '0' && c <= '9') color = (color << 4) | (c - '0');
		else if (c >= 'a' && c <= 'f') color = (color << 4) | (c - 'a' + 10);
		else return fallback;
	}
	return color;
}

static u32 appearance_percent(const char *value)
{
	return !strcmp(value,"100") ? 100 : appearance_number(value,100);
}

bool nyx_appearance_key(const char *key, const char *value)
{
	if (!strcmp(key, "font_file")) {
		n_cfg.font_file[0] = 0;
		if (valid_font_basename(value)) strcpy(n_cfg.font_file, value);
	} else if (!strcmp(key, "font_size")) n_cfg.font_size = appearance_number(value, 30);
	else if (!strcmp(key, "background_effect")) n_cfg.background_effect = appearance_number(value, 0);
	else if (!strcmp(key, "background_speed")) n_cfg.background_speed = appearance_number(value, 2);
	else if (!strcmp(key, "background_opacity")) n_cfg.background_opacity = appearance_number(value, 20);
	else if (!strcmp(key, "nova_halo")) n_cfg.nova_halo = appearance_number(value, 1);
	else if (!strcmp(key, "nova_logo_color")) n_cfg.nova_logo_color = appearance_rgb(value, 0x55C9F1);
	else if (!strcmp(key, "nova_icon_color")) n_cfg.nova_icon_color = appearance_rgb(value, 0x55C9F1);
	else if (!strcmp(key, "nova_selection_color")) n_cfg.nova_selection_color = appearance_rgb(value, 0x1C2937);
	else if (!strcmp(key, "nova_text_color")) n_cfg.nova_text_color = appearance_rgb(value, 0xF2F5FA);
	else if (!strcmp(key, "nova_border_color")) n_cfg.nova_border_color = appearance_rgb(value, 0x334050);
	else if (!strcmp(key, "nova_reflection_color")) n_cfg.nova_reflection_color = appearance_rgb(value, 0x55C9F1);
	else if (!strcmp(key, "nova_menu_opacity")) n_cfg.nova_menu_opacity = appearance_percent(value);
	else if (!strcmp(key, "nova_icon_opacity")) n_cfg.nova_icon_opacity = appearance_percent(value);
	else if (!strcmp(key, "boot_logo_color")) n_cfg.boot_logo_color = appearance_rgb(value, 0x55C9F1);
	else if (!strcmp(key, "boot_bar_color")) n_cfg.boot_bar_color = appearance_rgb(value, 0x55C9F1);
	else if (!strcmp(key, "launch_image")) {
		n_cfg.launch_image[0] = 0;
		if (valid_launch_basename(value)) strcpy(n_cfg.launch_image, value);
	}
	else if (!strcmp(key, "wallpaper_file")) {
		n_cfg.wallpaper_file[0] = 0;
		if (valid_wallpaper_basename(value)) strcpy(n_cfg.wallpaper_file, value);
	}
	else return false;
	nyx_sanitize_appearance();
	return true;
}

int create_config_entry()
{
	char lbuf[64];
	FIL fp;
	bool ini_found = false;

	LIST_INIT(ini_sections);

	if (!ini_parse(&ini_sections, "bootloader/hekate_ipl.ini", false))
		ini_found = true;
	else
	{
		u8 res = f_open(&fp, "bootloader/hekate_ipl.ini", FA_READ);
		if (res == FR_NO_FILE || res == FR_NO_PATH)
		{
			f_mkdir("bootloader");
			f_mkdir("bootloader/ini");
			f_mkdir("bootloader/payloads");
			f_mkdir("bootloader/sys");
		}
		else
		{
			if (!res)
				f_close(&fp);
			return 1;
		}
	}

	if (f_open(&fp, "bootloader/hekate_ipl.ini", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
		return 1;

	// Add config entry.
	f_puts("[config]\nautoboot=", &fp);
	itoa(h_cfg.autoboot, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nautoboot_list=", &fp);
	itoa(h_cfg.autoboot_list, lbuf, 10);
	f_puts(lbuf, &fp);
	/*
	 * Clamp value to default if it exceeds 20s to protect against corruption.
	 * Allow up to 20s though for use in cases where user needs lots of time.
	 * For example dock-only use and r2p with enough time to reach dock and cancel it.
	*/
	if (h_cfg.bootwait > 20)
		h_cfg.bootwait = 3;
	f_puts("\nbootwait=", &fp);
	itoa(h_cfg.bootwait, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nbacklight=", &fp);
	itoa(h_cfg.backlight, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nnoticker=", &fp);
	itoa(h_cfg.noticker, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nautohosoff=", &fp);
	itoa(h_cfg.autohosoff, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nautonogc=", &fp);
	itoa(h_cfg.autonogc, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nupdater2p=", &fp);
	itoa(h_cfg.updater2p, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\nbootprotect=", &fp);
	itoa(h_cfg.bootprotect, lbuf, 10);
	f_puts(lbuf, &fp);

	f_puts("\n", &fp);

	if (ini_found)
	{
		// Re-construct existing entries.
		LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, &ini_sections, link)
		{
			if (!strcmp(ini_sec->name, "config"))
				continue;

			switch (ini_sec->type)
			{
			case INI_CHOICE: // Re-construct Boot entry [ ].
				f_puts("[", &fp);
				f_puts(ini_sec->name, &fp);
				f_puts("]\n", &fp);
				// Re-construct boot entry's config.
				LIST_FOREACH_ENTRY(ini_kv_t, kv, &ini_sec->kvs, link)
				{
					f_puts(kv->key, &fp);
					f_puts("=", &fp);
					f_puts(kv->val, &fp);
					f_puts("\n", &fp);
				}
				break;
			case INI_CAPTION: // Re-construct caption entry { }.
				f_puts("{", &fp);
				f_puts(ini_sec->name, &fp);
				f_puts("}\n", &fp);
				break;
			case INI_NEWLINE: // Re-construct cosmetic newline \n.
				f_puts("\n", &fp);
				break;
			case INI_COMMENT: // Re-construct comment entry #.
				f_puts("#", &fp);
				f_puts(ini_sec->name, &fp);
				f_puts("\n", &fp);
				break;
			}
		}

		ini_free(&ini_sections);
	}

	f_close(&fp);

	return 0;
}

int create_nyx_config_entry(bool force_unmount)
{
	bool sd_mounted = sd_get_card_mounted();

	if (sd_mount())
		return 1;

	char lbuf[64];
	FIL fp;
	bool file_open = false;
	int result = 1;

	// Make sure that bootloader folder exists.
	f_mkdir("bootloader");

	if (f_open(&fp, "bootloader/nyx.ini.nova.tmp", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
		goto out;
	file_open = true;

	// Add config entry.
	if (f_puts("[config]\nthemebg=", &fp) < 0)
		goto out;
	itoa(n_cfg.theme_bg, lbuf, 16);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\nthemecolor=", &fp) < 0)
		goto out;
	itoa(n_cfg.theme_color, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\nentries5col=", &fp) < 0)
		goto out;
	itoa(n_cfg.entries_5_col, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\ntimeoffset=", &fp) < 0)
		goto out;
	itoa(n_cfg.timeoffset, lbuf, 16);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\ntimedst=", &fp) < 0)
		goto out;
	itoa(n_cfg.timedst, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\nhomescreen=", &fp) < 0)
		goto out;
	itoa(n_cfg.home_screen, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	// Invalid or corrupted values always fall back to the tiles layout.
	if (n_cfg.home_layout != HOME_LAYOUT_CAROUSEL)
		n_cfg.home_layout = HOME_LAYOUT_TILES;
	if (f_puts("\nhome_layout=", &fp) < 0)
		goto out;
	itoa(n_cfg.home_layout, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;
	if (f_puts("\nverification=", &fp) < 0)
		goto out;
	itoa(n_cfg.verification, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\numsemmcrw=", &fp) < 0)
		goto out;
	itoa(n_cfg.ums_emmc_rw, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\njcdisable=", &fp) < 0)
		goto out;
	itoa(n_cfg.jc_disable, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\njcforceright=", &fp) < 0)
		goto out;
	itoa(n_cfg.jc_force_right, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (f_puts("\nbpmpclock=", &fp) < 0)
		goto out;
	itoa(n_cfg.bpmp_clock, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	if (n_cfg.language >= I18N_LANGUAGE_COUNT)
		n_cfg.language = I18N_EN;
	if (f_puts("\nlanguage=", &fp) < 0)
		goto out;
	itoa(n_cfg.language, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0)
		goto out;

	nyx_sanitize_appearance();
	if (f_puts("\nfont_file=", &fp) < 0 || f_puts(n_cfg.font_file, &fp) < 0) goto out;
	if (f_puts("\nfont_size=", &fp) < 0) goto out;
	itoa(n_cfg.font_size, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nbackground_effect=", &fp) < 0) goto out;
	itoa(n_cfg.background_effect, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nbackground_speed=", &fp) < 0) goto out;
	itoa(n_cfg.background_speed, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nbackground_opacity=", &fp) < 0) goto out;
	itoa(n_cfg.background_opacity, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_halo=", &fp) < 0) goto out;
	itoa(n_cfg.nova_halo, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_text_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_text_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_border_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_border_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_reflection_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_reflection_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_menu_opacity=", &fp) < 0) goto out;
	itoa(n_cfg.nova_menu_opacity, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_icon_opacity=", &fp) < 0) goto out;
	itoa(n_cfg.nova_icon_opacity, lbuf, 10);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_logo_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_logo_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_icon_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_icon_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nnova_selection_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.nova_selection_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nwallpaper_file=", &fp) < 0 || f_puts(n_cfg.wallpaper_file, &fp) < 0) goto out;
	if (f_puts("\nlaunch_image=", &fp) < 0 || f_puts(n_cfg.launch_image, &fp) < 0) goto out;
	if (f_puts("\nboot_logo_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.boot_logo_color);
	if (f_puts(lbuf, &fp) < 0) goto out;
	if (f_puts("\nboot_bar_color=", &fp) < 0) goto out;
	s_printf(lbuf, "%06X", n_cfg.boot_bar_color);
	if (f_puts(lbuf, &fp) < 0) goto out;

	if (f_puts("\n", &fp) < 0)
		goto out;

	result = 0;

out:
	if (file_open && f_close(&fp) != FR_OK)
		result = 1;

    /* Commit only a completely written AND closed file. Keep a recoverable
     * previous copy across the two FAT renames, including power loss. */
    if (!result) {
        FRESULT existing=f_stat("bootloader/nyx.ini",NULL);
        bool previous=existing==FR_OK;
        if (!previous && existing!=FR_NO_FILE && existing!=FR_NO_PATH) result=1;
        if (!result && previous) {
            FRESULT removed=f_unlink("bootloader/nyx.ini.nova.bak");
            if (removed!=FR_OK && removed!=FR_NO_FILE && removed!=FR_NO_PATH) result=1;
            if (!result && f_rename("bootloader/nyx.ini","bootloader/nyx.ini.nova.bak")!=FR_OK) result=1;
        }
        if (!result && f_rename("bootloader/nyx.ini.nova.tmp","bootloader/nyx.ini")!=FR_OK) {
            result=1;
            if(previous) f_rename("bootloader/nyx.ini.nova.bak","bootloader/nyx.ini");
        }
        if (!result && previous) f_unlink("bootloader/nyx.ini.nova.bak");
    }
    if(result && file_open) f_unlink("bootloader/nyx.ini.nova.tmp");

	if (force_unmount || !sd_mounted)
		sd_unmount();

	return result;
}
