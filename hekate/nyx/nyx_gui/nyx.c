/*
 * Copyright (c) 2018 naehrwert
 *
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

#include <string.h>
#include <stdlib.h>

#include <bdk.h>

#include "config.h"
#include <display/nova_splash.h>
#include "localization.h"
#include "hos/hos.h"
#include <ianos/ianos.h>
#include <libs/compr/blz.h>
#include <libs/fatfs/ff.h>

#include "frontend/fe_emmc_tools.h"
#include "frontend/gui.h"
#include "frontend/gui_diagnostics.h"
#include "diagnostics/nyx_diagnostics.h"
#include "diagnostics/ram_handoff.h"

nyx_config n_cfg;
hekate_config h_cfg;

const volatile ipl_ver_meta_t __attribute__((section ("._ipl_version"))) ipl_ver = {
	.magic = NYX_MAGIC,
	.version = (NYX_VER_MJ + '0') | ((NYX_VER_MN + '0') << 8) | ((NYX_VER_HF + '0') << 16) | ((NYX_VER_RL) << 24),
};

volatile nyx_storage_t *nyx_str = (nyx_storage_t *)NYX_STORAGE_ADDR;
volatile boot_cfg_t *b_cfg;

char *emmcsn_path_impl(char *path, char *sub_dir, char *filename, sdmmc_storage_t *storage)
{
	static char emmc_sn[9] = {0};

	// Check if eMMC S/N storage has valid data and skip parsing in that case.
	if (emmc_sn[0] && strcmp(emmc_sn, "00000000"))
		goto create_dir;

	// Get actual eMMC S/N.
	if (!storage)
	{
		if (emmc_initialize(false))
			strcpy(emmc_sn, "00000000");
		else
		{
			itoa(emmc_storage.cid.serial, emmc_sn, 16);
			emmc_end();
		}
	}
	else
		itoa(storage->cid.serial, emmc_sn, 16);

create_dir:
	// Check if only eMMC S/N was requested.
	if (!path)
		return emmc_sn;

	// Create main folder.
	strcpy(path, "backup");
	f_mkdir(path);

	// Create eMMC S/N folder.
	strcat(path, "/");
	strcat(path, emmc_sn);
	f_mkdir(path);

	// Create sub folder if defined. Dir slash must be included.
	strcat(path, sub_dir);  // Can be a null-terminator.
	if (strlen(sub_dir))
		f_mkdir(path);

	// Add filename.
	strcat(path, "/");
	strcat(path, filename); // Can be a null-terminator.

	return emmc_sn;
}

// This is a safe and unused DRAM region for our payloads.
#define RELOC_META_OFF      0x7C
#define PATCHED_RELOC_SZ    0x94
#define PATCHED_RELOC_STACK 0x40007000
#define PATCHED_RELOC_ENTRY 0x40010000
#define EXT_PAYLOAD_ADDR    0xC0000000
#define RCM_PAYLOAD_ADDR    (EXT_PAYLOAD_ADDR + ALIGN(PATCHED_RELOC_SZ, 0x10))

static void _reloc_append(u32 payload_dst, u32 payload_src, u32 payload_size)
{
	memcpy((u8 *)payload_src, (u8 *)nyx_str->hekate, PATCHED_RELOC_SZ);

	volatile reloc_meta_t *relocator = (reloc_meta_t *)(payload_src + RELOC_META_OFF);

	relocator->start = payload_dst - ALIGN(PATCHED_RELOC_SZ, 0x10);
	relocator->stack = PATCHED_RELOC_STACK;
	relocator->end   = payload_dst + payload_size;
	relocator->ep    = payload_dst;
}

lv_res_t launch_payload(lv_obj_t *list)
{
	const char *filename = lv_list_get_btn_text(list);

	if (!filename || !filename[0])
		goto out;

	char path[128];

	strcpy(path,"bootloader/payloads/");
	strcat(path, filename);

	if (sd_mount())
		goto out;

	// Read payload.
	u32 size = 0;
	void *buf = sd_file_read(path, &size);
	if (!buf)
	{
		EPRINTFARGS("Payload file is missing!\n(%s)", path);

		goto out;
	}

	// Check if it safely fits IRAM.
	if (size > 0x30000)
	{
		EPRINTF("Payload is too big!");

		goto out;
	}

	sd_end();

	// Copy the payload to our chosen address.
	memcpy((void *)RCM_PAYLOAD_ADDR, buf, size);

	// Append relocator.
	_reloc_append(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(size, 0x10));

	hw_deinit(false);

	// Launch our payload.
	void (*payload_ptr)() = (void *)EXT_PAYLOAD_ADDR;
	(*payload_ptr)();

out:
	sd_unmount();

	return LV_RES_OK;
}

/* Validate size before allocating, and reject short/failed reads. */
static u8 *_read_service_payload(const char *path, u32 limit, u32 *size)
{
	FIL file;
	if (f_open(&file, path, FA_READ) != FR_OK)
		return NULL;
	u32 length = f_size(&file);
	if (length < 0x100 || length > limit)
	{
		f_close(&file);
		return NULL;
	}
	u8 *buffer = malloc(length);
	if (!buffer)
	{
		f_close(&file);
		return NULL;
	}
	UINT received = 0;
	FRESULT read_result = f_read(&file, buffer, length, &received);
	FRESULT close_result = f_close(&file);
	if (read_result != FR_OK || close_result != FR_OK || received != length)
	{
		free(buffer);
		return NULL;
	}
	*size = length;
	return buffer;
}

int nyx_launch_ram_service(void)
{
	if (nyx_diag_running())
		return 5;
	/* A failed preparation must not leave an older automatic-run request. */
	((volatile nyx_ram_handoff_t *)NYX_RAM_HANDOFF_ADDR)->magic = 0;
	/* The optional localized service UI occupies the same protected first
	 * 4 MiB as the return image. A missing/corrupt pack falls back to English. */
	*(volatile u32 *)NYX_RAM_UI_ADDR = 0;
	if (sd_mount())
		return 1;
	u32 ram_size = 0, return_size = 0;
	u8 *ram_image = _read_service_payload("bootloader/sys/hwtest_ram.bin", 130392, &ram_size);
	if (!ram_image)
	{
		sd_unmount();
		return 2;
	}
	u8 *return_image = _read_service_payload("bootloader/update.bin", NYX_RAM_RETURN_MAX, &return_size);
	if (!return_image)
	{
		free(ram_image);
		sd_unmount();
		return 3;
	}
	/* The return image must be a Hekate payload, not an arbitrary .bin. */
	if (!ram_size || ram_size > 130392 || return_size < 0x100 ||
		return_size > NYX_RAM_RETURN_MAX ||
		memcmp(return_image + return_size - 6, "ICTC", 4))
	{
		free(return_image);
		free(ram_image);
		sd_unmount();
		return 4;
	}
	/* Force Hekate's menu on return, even when normal autoboot is configured. */
	static const char *ram_languages[] = {"en", "fr", "es", "it", "ru", "de", "pt"};
	_Static_assert(ARRAY_SIZE(ram_languages) == I18N_LANGUAGE_COUNT, "RAM language codes must match Nyx locales");
	_Static_assert(NYX_RAM_UI_ADDR + NYX_RAM_UI_MAX <= NYX_RAM_HANDOFF_ADDR, "RAM UI must stay below the return handoff");
	char ui_path[64];
	s_printf(ui_path, "bootloader/sys/ram-ui-%s.bin", ram_languages[n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN]);
	u32 ui_size = 0;
	u8 *ui = _read_service_payload(ui_path, NYX_RAM_UI_MAX, &ui_size);
	if (ui && *(u32 *)ui == NYX_RAM_UI_MAGIC && ((u32 *)ui)[1] == ui_size &&
		((u32 *)ui)[2] == nyx_ram_hash(ui + 32, ui_size - 32))
		memcpy((void *)NYX_RAM_UI_ADDR, ui, ui_size);
	free(ui);
	return_image[0x94] = BOOT_CFG_AUTOBOOT_EN;
	return_image[0x95] = 0;
	return_image[0x96] = 0;
	return_image[0x97] = 0;
	memcpy((void *)NYX_RAM_RETURN_ADDR, return_image, return_size);
	nyx_ram_handoff_t request = {0};
	request.version = NYX_RAM_VERSION;
	request.payload_size = return_size;
	request.payload_hash = nyx_ram_hash(return_image, return_size);
	request.auto_run = 1;
	request.magic = NYX_RAM_HANDOFF_MAGIC;
	memcpy((void *)NYX_RAM_HANDOFF_ADDR, &request, sizeof(request));
	memcpy((void *)RCM_PAYLOAD_ADDR, ram_image, ram_size);
	memset((void *)(RCM_PAYLOAD_ADDR + ram_size), 0, ALIGN(ram_size, 0x10) - ram_size);
	_reloc_append(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(ram_size, 0x10));
	free(return_image);
	free(ram_image);
	sd_end();
	bpmp_mmu_maintenance(BPMP_MMU_MAINT_CLEAN_WAY, false);
	hw_deinit(false);
	void (*payload)(void) = (void *)EXT_PAYLOAD_ADDR;
	payload();
	((volatile nyx_ram_handoff_t *)NYX_RAM_HANDOFF_ADDR)->magic = 0;
	return 6;
}

static void _load_saved_configuration()
{
	LIST_INIT(ini_sections);
	LIST_INIT(ini_nyx_sections);

	if (ini_parse(&ini_sections, "bootloader/hekate_ipl.ini", false))
	{
		create_config_entry();
		goto skip_main_cfg_parse;
	}

	// Load hekate configuration.
	LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, &ini_sections, link)
	{
		// Only parse config section.
		if (ini_sec->type == INI_CHOICE && !strcmp(ini_sec->name, "config"))
		{
			LIST_FOREACH_ENTRY(ini_kv_t, kv, &ini_sec->kvs, link)
			{
				if      (!strcmp("autoboot",      kv->key))
					h_cfg.autoboot      = atoi(kv->val);
				else if (!strcmp("autoboot_list", kv->key))
					h_cfg.autoboot_list = atoi(kv->val);
				else if (!strcmp("bootwait",      kv->key))
					h_cfg.bootwait      = atoi(kv->val);
				else if (!strcmp("backlight",     kv->key))
				{
					h_cfg.backlight = atoi(kv->val);
					if (h_cfg.backlight <= 20)
						h_cfg.backlight = 30;
				}
				else if (!strcmp("noticker",    kv->key))
					h_cfg.noticker    = atoi(kv->val);
				else if (!strcmp("autohosoff",  kv->key))
					h_cfg.autohosoff  = atoi(kv->val);
				else if (!strcmp("autonogc",    kv->key))
					h_cfg.autonogc    = atoi(kv->val);
				else if (!strcmp("updater2p",   kv->key))
					h_cfg.updater2p   = atoi(kv->val);
				else if (!strcmp("bootprotect", kv->key))
					h_cfg.bootprotect = atoi(kv->val);
			}

			break;
		}
	}

	ini_free(&ini_sections);

skip_main_cfg_parse:
	if (ini_parse(&ini_nyx_sections, "bootloader/nyx.ini", false))
		return;

	// Load Nyx configuration.
	LIST_FOREACH_ENTRY(ini_sec_t, ini_sec, &ini_nyx_sections, link)
	{
		// Only parse config section.
		if (ini_sec->type == INI_CHOICE && !strcmp(ini_sec->name, "config"))
		{
			bool time_old_raw = false;
			LIST_FOREACH_ENTRY(ini_kv_t, kv, &ini_sec->kvs, link)
			{
				if      (!strcmp("themebg",      kv->key))
					n_cfg.theme_bg       = strtol(kv->val, NULL, 16);
				else if (!strcmp("themecolor",   kv->key))
					n_cfg.theme_color    = atoi(kv->val);
				else if (!strcmp("entries5col",  kv->key))
					n_cfg.entries_5_col  = atoi(kv->val) == 1;
				else if (!strcmp("timeoffset",   kv->key))
				{
					n_cfg.timeoffset = strtol(kv->val, NULL, 16);
					if (n_cfg.timeoffset != 1)
						max77620_rtc_set_epoch_offset((int)n_cfg.timeoffset);
				}
				else if (!strcmp("timeoff",      kv->key))
				{
					if (strtol(kv->val, NULL, 16) == 1)
						time_old_raw = true;
				}
				else if (!strcmp("timedst",      kv->key))
					n_cfg.timedst        = atoi(kv->val);
				else if (!strcmp("homescreen",   kv->key))
					n_cfg.home_screen    = atoi(kv->val);
				else if (!strcmp("home_layout",  kv->key))
					n_cfg.home_layout = !strcmp(kv->val, "1") ? HOME_LAYOUT_CAROUSEL : HOME_LAYOUT_TILES;
				else if (!strcmp("language",     kv->key))
					n_cfg.language = kv->val[0] >= '0' && kv->val[0] < '0' + I18N_LANGUAGE_COUNT && !kv->val[1]
						? (u32)(kv->val[0] - '0') : I18N_EN;
				else if (!strcmp("verification", kv->key))
					n_cfg.verification   = atoi(kv->val);
				else if (!strcmp("umsemmcrw",    kv->key))
					n_cfg.ums_emmc_rw    = atoi(kv->val) == 1;
				else if (!strcmp("jcdisable",    kv->key))
					n_cfg.jc_disable     = atoi(kv->val) == 1;
				else if (!strcmp("jcforceright", kv->key))
					n_cfg.jc_force_right = atoi(kv->val) == 1;
				else if (!strcmp("bpmpclock",    kv->key))
					n_cfg.bpmp_clock     = atoi(kv->val);
				else nyx_appearance_key(kv->key, kv->val);
			}

			// Check if user canceled time setting before.
			if (time_old_raw && !n_cfg.timeoffset)
				n_cfg.timeoffset = 1;

			break;
		}
	}

	// Set auto DST here in case it's missing.
	max77620_rtc_set_auto_dst(n_cfg.timedst);

	ini_free(&ini_nyx_sections);
}

static int nyx_load_resources()
{
	FIL fp;
	int res;

	res = f_open(&fp, "bootloader/sys/res.pak", FA_READ);
	if (res)
		return res;

	res = f_read(&fp, (void *)NYX_RES_ADDR, f_size(&fp), NULL);
	f_close(&fp);

	return res;
}

static void nyx_load_bg_icons()
{
	// If no custom switch icon exists, load normal.
	if (!f_stat("bootloader/res/icon_switch_custom.bmp", NULL))
		icon_switch = bmp_to_lvimg_obj("bootloader/res/icon_switch_custom.bmp");
	else
		icon_switch = bmp_to_lvimg_obj("bootloader/res/icon_switch.bmp");

	// If no custom payload icon exists, load normal.
	if (!f_stat("bootloader/res/icon_payload_custom.bmp", NULL))
		icon_payload = bmp_to_lvimg_obj("bootloader/res/icon_payload_custom.bmp");
	else
		icon_payload = bmp_to_lvimg_obj("bootloader/res/icon_payload.bmp");

	// Load background resource if any.
	hekate_bg = bmp_to_lvimg_obj("bootloader/res/background.bmp");
	nyx_nova_wallpaper_load();
}

#define EXCP_EN_ADDR   0x4003FF1C
#define  EXCP_MAGIC 0x30505645      // EVP0
#define EXCP_TYPE_ADDR 0x4003FF18
#define  EXCP_TYPE_RESET 0x545352   // RST
#define  EXCP_TYPE_UNDEF 0x464455   // UDF
#define  EXCP_TYPE_PABRT 0x54424150 // PABT
#define  EXCP_TYPE_DABRT 0x54424144 // DABT
#define EXCP_LR_ADDR   0x4003FF14

enum {
	SD_NO_ERROR    = 0,
	SD_MOUNT_ERROR = 1,
	SD_FILE_ERROR  = 2
};

static void _show_errors(int sd_error)
{
	u32 *excp_enabled = (u32 *)EXCP_EN_ADDR;
	u32 *excp_type = (u32 *)EXCP_TYPE_ADDR;
	u32 *excp_lr = (u32 *)EXCP_LR_ADDR;

	if (*excp_enabled == EXCP_MAGIC || sd_error)
	{
		gfx_clear_grey(0);
		gfx_con_setpos(0, 0, 0);
		display_backlight_brightness(150, 1000);
		display_init_window_d_console();
		display_window_d_console_enable();
	}

	switch (sd_error)
	{
	case SD_MOUNT_ERROR:
		WPRINTF("Failed to init or mount SD!\n");
		goto error_occured;
	case SD_FILE_ERROR:
		WPRINTF("Failed to load GUI resources!\nres.pak not found or corrupted.\n");
		goto error_occured;
	case SD_NO_ERROR:
	default:
		break;
	}

	if (*excp_enabled == EXCP_MAGIC)
	{
		WPRINTFARGS("Nyx exception occurred (LR %08X):\n", *excp_lr);
		switch (*excp_type)
		{
		case EXCP_TYPE_RESET:
			WPRINTF("RESET");
			break;
		case EXCP_TYPE_UNDEF:
			WPRINTF("UNDEF");
			break;
		case EXCP_TYPE_PABRT:
			WPRINTF("PABRT");
			break;
		case EXCP_TYPE_DABRT:
			WPRINTF("DABRT");
			break;
		}
		gfx_puts("\n");

		// Clear the exception.
		*excp_lr = 0;
		*excp_type = 0;
		*excp_enabled = 0;

error_occured:
		WPRINTF("Press any key to reload Nyx...");

		msleep(1000);
		btn_wait();

		reload_nyx(NULL, true);
	}
}

void nyx_init_load_res()
{
	bpmp_mmu_enable();
	bpmp_clk_rate_get();

	// Set a modest clock for init. It will be restored later if possible.
	bpmp_clk_rate_set(BPMP_CLK_LOWEST_BOOST);

	// Set bootloader's default configuration.
	set_default_configuration();
	set_nyx_default_configuration();

	// Reset new info if magic not correct.
	if (nyx_str->info.magic != NYX_NEW_INFO)
	{
		nyx_str->info.sd_init = 0;
		for (u32 i = 0; i < 3; i++)
			nyx_str->info.sd_errors[i] = 0;
	}

	// Reset new extended info if magic not correct.
	if (nyx_str->info_ex.magic != NYX_NEW_INFO)
		nyx_str->info_ex.rsvd_flags = 0;

	// Clear info magic.
	nyx_str->info.magic    = 0;
	nyx_str->info_ex.magic = 0;

	// Override DRAM ID if needed.
	if (nyx_str->info_ex.rsvd_flags & RSVD_FLAG_DRAM_8GB)
		fuse_force_8gb_dramid();

	// Set display id from previous initialization.
	display_set_decoded_panel_id(nyx_str->info.panel_id);

	// Initialize gfx console.
	gfx_init_ctxt((u32 *)LOG_FB_ADDRESS, 1280, 656, 656);
	gfx_con_init();

	// Show exception errors if any.
	_show_errors(SD_NO_ERROR);

	// Try 2 times to mount SD card.
	if (sd_mount())
	{
		// Restore speed to SDR104.
		sd_end();

		// Retry.
		if (sd_mount())
			_show_errors(SD_MOUNT_ERROR); // Fatal.
	}

	// Train DRAM and switch to max frequency.
	minerva_init((minerva_str_t *)&nyx_str->minerva);
	minerva_change_freq(FREQ_1600);

	// Load hekate/Nyx configuration.
	_load_saved_configuration();

	nova_splash_progress((u32 *)IPL_FB_ADDRESS,20,65);
	// Load Nyx resources.
	if (nyx_load_resources())
	{
		// Try again.
		if (nyx_load_resources())
			_show_errors(SD_FILE_ERROR); // Fatal since resources are mandatory.
	}

	// Initialize nyx cfg to lower clock on first boot.
	// In case of lower binned SoC, this can help with hangs.
	if (!n_cfg.bpmp_clock)
	{
		// Set lower clock and save it.
		n_cfg.bpmp_clock = 2;
		create_nyx_config_entry(false);

		// Start at max clock and test it.
		n_cfg.bpmp_clock = 0;
	}

	// Set selected clock.
	switch (n_cfg.bpmp_clock)
	{
	case 0:
	case 1:
		bpmp_clk_rate_set(BPMP_CLK_BIN0_BOOST);
		break;
	case 2:
		bpmp_clk_rate_set(BPMP_CLK_BIN1_BOOST);
		break;
	case 3:
		bpmp_clk_rate_set(BPMP_CLK_BIN2_BOOST);
		break;
	case 4:
		bpmp_clk_rate_set(BPMP_CLK_BIN3_BOOST);
		break;
	case 5:
	default:
		bpmp_clk_rate_set(BPMP_CLK_NORMAL);
		break;
	}

	nova_splash_progress((u32 *)IPL_FB_ADDRESS,28,80);
	// Load default launch icons and background if it exists.
	nyx_load_bg_icons();

	// Unmount FAT partition.
	sd_unmount();
}

void ipl_main()
{
	// Set heap address.
	heap_init((void *)IPL_HEAP_START);

	b_cfg = (boot_cfg_t *)(nyx_str->hekate + 0x94);

#ifdef DEBUG_UART_PORT
	// Enable the selected uart debug port.
	#if   (DEBUG_UART_PORT == UART_B)
		gpio_config(GPIO_PORT_G, GPIO_PIN_0, GPIO_MODE_SPIO);
	#elif (DEBUG_UART_PORT == UART_C)
		gpio_config(GPIO_PORT_D, GPIO_PIN_1, GPIO_MODE_SPIO);
	#endif
	pinmux_config_uart(DEBUG_UART_PORT);
	clock_enable_uart(DEBUG_UART_PORT);
	uart_init(DEBUG_UART_PORT, DEBUG_UART_BAUDRATE, UART_AO_TX_AO_RX);
	uart_invert(DEBUG_UART_PORT, DEBUG_UART_INVERT, UART_INVERT_TXD);

	uart_send(DEBUG_UART_PORT, (u8 *)"hekate-NYX: Hello!\r\n", 20);
	uart_wait_xfer(DEBUG_UART_PORT, UART_TX_IDLE);
#endif

	// Initialize the rest of hw and load Nyx resources.
	nyx_init_load_res();

	// Initialize Nyx GUI and show it.
	nyx_load_and_run();

	// Halt BPMP if we managed to get out of execution.
	while (true)
		bpmp_halt();
}
