/*
 * Copyright (c) 2019-2026 CTCaer
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
#include "gui_nova.h"
#include "gui_choices.h"
#include "fe_emummc_tools.h"
#include "gui_tools_partition_manager.h"
#include <libs/fatfs/ff.h>

extern char *emmcsn_path_impl(char *path, char *sub_dir, char *filename, sdmmc_storage_t *storage);

typedef struct _mbr_ctxt_t
{
	u32 available;
	u32 sector[3];
	u32 resized_cnt[3];

	int part_idx;
	u32 sector_start;
} mbr_ctxt_t;

static bool emummc_backup;
static mbr_ctxt_t mbr_ctx;
static lv_obj_t *emummc_manage_window;
static lv_res_t (*emummc_tools)(lv_obj_t *btn);

bool nyx_nova_installation_copy(u32 index,u32 start,u32 size,u32 sectors,u32 gpp,const u8 *cid)
{
    if(!cid||index<1||index>3||start<=0x8000||(u64)start+size>sectors||0xC000ULL+gpp>size||sd_mount()){
        nyx_show_help("Copy target validation failed. Automatic installation stopped.");return false;
    }
    mbr_t table;bool valid=sd_storage.sec_cnt==sectors&&!memcmp(sd_storage.raw_cid,cid,16)&&
        !sdmmc_storage_read(&sd_storage,0,1,&table);
    if(valid)valid=table.boot_signature==0xAA55&&table.partitions[index].type==0xE0&&
        table.partitions[index].start_sct==start&&table.partitions[index].size_sct==size;
    if(emmc_initialize(false))valid=false;
    else{emmc_set_partition(EMMC_GPP);valid=valid&&emmc_storage.sec_cnt==gpp;emmc_end();}
    sd_unmount();
    if(!valid){nyx_show_help("Copy target validation failed. Automatic installation stopped.");return false;}
    lv_obj_t *win=nyx_create_standard_window("Automated installation",NULL);
    emmc_tool_gui_t context={0};nyx_nova_transfer(win,&context);nyx_window_toggle_buttons(win,true);
    lv_label_set_text(context.label_finish,"Copying and verifying emuMMC. Do not remove the SD card.");
    int result=dump_emummc_raw_checked(&context,index,start+0x8000,cid,gpp);
    lv_label_set_text(context.label_finish,result?
        "Installation stopped. emuMMC was not activated. Check the log.":
        "emuMMC copied, verified and activated. Linux/Android installation files are still required.");
    nyx_window_toggle_buttons(win,false);return result==0;
}

static lv_res_t _action_emummc_window_close(lv_obj_t *btn)
{
	nyx_win_close_action(btn);

	// Delete and relaunch main emuMMC window.
	lv_obj_del(emummc_manage_window);
	(*emummc_tools)(NULL);

	return LV_RES_INV;
}

static void _create_window_emummc()
{
	emmc_tool_gui_t emmc_tool_gui_ctxt;

	lv_obj_t *win;
	if (!mbr_ctx.part_idx)
		win = nyx_create_standard_window(SYMBOL_DRIVE"  Create SD File emuMMC", _action_emummc_window_close);
	else
		win = nyx_create_standard_window(SYMBOL_DRIVE"  Create SD Partition emuMMC", _action_emummc_window_close);

	//Disable buttons.
	nyx_window_toggle_buttons(win, true);

	// Create important info container.
	lv_obj_t *h1 = lv_cont_create(win, NULL);
	lv_cont_set_fit(h1, false, true);
	lv_obj_set_width(h1, (LV_HOR_RES / 9) * 5);
	lv_obj_set_click(h1, false);
	lv_cont_set_layout(h1, LV_LAYOUT_OFF);

	static lv_style_t h_style;
	lv_style_copy(&h_style, lv_cont_get_style(h1));
	h_style.body.main_color = lv_theme_get_current()->bg->body.main_color;
	h_style.body.grad_color = h_style.body.main_color;
	h_style.body.opa = LV_OPA_COVER;

	// Chreate log container.
	lv_obj_t *h2 = lv_cont_create(win, h1);
	lv_cont_set_style(h2, &h_style);
	lv_cont_set_fit(h2, false, false);
	lv_obj_set_size(h2, (LV_HOR_RES / 11) * 4, LV_DPI * 5);
	lv_cont_set_layout(h2, LV_LAYOUT_OFF);
	lv_obj_align(h2, h1, LV_ALIGN_OUT_RIGHT_TOP, 0, LV_DPI / 5);

	lv_obj_t *label_log = lv_label_create(h2, NULL);
	lv_label_set_recolor(label_log, true);
	lv_obj_set_style(label_log, &monospace_text);
	lv_label_set_long_mode(label_log, LV_LABEL_LONG_BREAK);
	lv_label_set_static_text(label_log, "");
	lv_obj_set_width(label_log, lv_obj_get_width(h2));
	lv_obj_align(label_log, h2, LV_ALIGN_IN_TOP_LEFT, LV_DPI / 10, LV_DPI / 10);
	emmc_tool_gui_ctxt.label_log = label_log;

	// Create elements for info container.
	lv_obj_t *label_sep = lv_label_create(h1, NULL);
	lv_label_set_static_text(label_sep, "");

	lv_obj_t *label_info = lv_label_create(h1, NULL);
	lv_label_set_recolor(label_info, true);
	lv_obj_set_width(label_info, lv_obj_get_width(h1));
	lv_label_set_static_text(label_info, "\n\n\n\n\n\n\n\n\n");
	lv_obj_set_style(label_info, lv_theme_get_current()->label.prim);
	lv_obj_align(label_info, label_sep, LV_ALIGN_OUT_BOTTOM_LEFT, LV_DPI / 4, LV_DPI / 10);
	emmc_tool_gui_ctxt.label_info = label_info;

	lv_obj_t *bar = lv_bar_create(h1, NULL);
	lv_obj_set_size(bar, LV_DPI * 38 / 10, LV_DPI / 5);
	lv_bar_set_range(bar, 0, 100);
	lv_bar_set_value(bar, 0);
	lv_obj_align(bar, label_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, LV_DPI / 8);
	lv_obj_set_opa_scale(bar, LV_OPA_0);
	lv_obj_set_opa_scale_enable(bar, true);
	emmc_tool_gui_ctxt.bar = bar;

	lv_obj_t *label_pct= lv_label_create(h1, NULL);
	lv_label_set_recolor(label_pct, true);
	lv_label_set_static_text(label_pct, " "SYMBOL_DOT" 0%");
	lv_obj_set_style(label_pct, lv_theme_get_current()->label.prim);
	lv_obj_align(label_pct, bar, LV_ALIGN_OUT_RIGHT_MID, LV_DPI / 20, 0);
	lv_obj_set_opa_scale(label_pct, LV_OPA_0);
	lv_obj_set_opa_scale_enable(label_pct, true);
	emmc_tool_gui_ctxt.label_pct = label_pct;

	lv_obj_t *label_finish = lv_label_create(h1, NULL);
	lv_label_set_recolor(label_finish, true);
	lv_label_set_static_text(label_finish, "");
	lv_obj_set_style(label_finish, lv_theme_get_current()->label.prim);
	lv_obj_align(label_finish, bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, LV_DPI * 9 / 20);
	emmc_tool_gui_ctxt.label_finish = label_finish;

	if (!mbr_ctx.part_idx)
		dump_emummc_file(&emmc_tool_gui_ctxt);
	else
		dump_emummc_raw(&emmc_tool_gui_ctxt, mbr_ctx.part_idx, mbr_ctx.sector_start, mbr_ctx.resized_cnt[mbr_ctx.part_idx - 1]);

	nyx_window_toggle_buttons(win, false);
}

static lv_res_t _create_emummc_raw_format(lv_obj_t * btns, const char * txt)
{
	int btn_idx = lv_btnm_get_pressed(btns);

	// Delete parent mbox.
	nyx_mbox_action(btns, txt);

	// Create partition window.
	if (!btn_idx)
		create_window_sd_partition_manager(btns);

	mbr_ctx.part_idx = 0;
	mbr_ctx.sector_start = 0;

	return LV_RES_INV;
}

static lv_res_t _create_emummc_raw_action(lv_obj_t * btns, const char * txt)
{
	int btn_idx = lv_btnm_get_pressed(btns);
	lv_obj_t *bg = lv_obj_get_parent(lv_obj_get_parent(btns));

	mbr_ctx.sector_start = 0x8000; // Protective offset.

	switch (btn_idx)
	{
	case 0:
		mbr_ctx.part_idx = 1;
		mbr_ctx.sector_start += mbr_ctx.sector[0];
		break;
	case 1:
		mbr_ctx.part_idx = 2;
		mbr_ctx.sector_start += mbr_ctx.sector[1];
		break;
	case 2:
		mbr_ctx.part_idx = 3;
		mbr_ctx.sector_start += mbr_ctx.sector[2];
		break;
	default:
		break;
	}

	if (btn_idx < 3)
	{
		lv_obj_set_style(bg, &lv_style_transp);
		_create_window_emummc();
	}

	mbr_ctx.part_idx = 0;
	mbr_ctx.sector_start = 0;

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

static void _create_mbox_emummc_raw()
{
	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_format[] = { "\222Continue", "\222Cancel", "" };
	static char *mbox_btn_parts[] = { "\262Part 1", "\262Part 2", "\262Part 3", "\222Cancel", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	char *txt_buf = (char *)malloc(SZ_16K);
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));

	memset(&mbr_ctx, 0, sizeof(mbr_ctxt_t));

	sd_mount();
	sdmmc_storage_read(&sd_storage, 0, 1, mbr);
	sd_unmount();

	emmc_initialize(false);

	u32 emmc_size_safe = emmc_storage.sec_cnt + 0xC000; // eMMC GPP size + BOOT0/1.

	emmc_end();

	for (int i = 1; i < 4; i++)
	{
		u32 part_size = mbr->partitions[i].size_sct;
		u32 part_start = mbr->partitions[i].start_sct;
		u8  part_type = mbr->partitions[i].type;

		// Skip Linux, GPT (Android) and SFD partitions.
		bool valid_part = (part_type != 0x83) && (part_type != 0xEE) && (part_type != 0xFF);

		// Check if at least 4GB and start above 16MB.
		if ((part_size >= 0x80F000) && part_start > 0x8000 && valid_part)
		{
			mbr_ctx.available |= BIT(i - 1);
			mbr_ctx.sector[i - 1] = part_start;

			// Only allow up to 28GB resized emuMMC.
			if (part_size <= 0x3810000)
				mbr_ctx.resized_cnt[i - 1] = part_size - 0xC000; // Save sectors count without protective size and BOOT0/1.
			else if (part_size >= emmc_size_safe)
				mbr_ctx.resized_cnt[i - 1] = 0;
			else
			{
				mbr_ctx.available &= ~BIT(i - 1);
				mbr_ctx.sector[i - 1] = 0;
			}
		}
	}

	if (mbr_ctx.available)
	{
		s_printf(txt_buf,
			"#C7EA46 Found applicable partition(s)!#\n"
			"#FF8000 Choose a partition to continue:#\n\n");
	}
	else
		s_printf(txt_buf, "#FFDD00 Failed to find applicable partition!#\n\n");

	s_printf(txt_buf + strlen(txt_buf),
		"Partition table:\n"
		"#C0C0C0 Part 0: Type: %02x, Start: %08x, Size: %08x#\n"
		"#%s Part 1: Type: %02x, Start: %08x, Size: %08x#\n"
		"#%s Part 2: Type: %02x, Start: %08x, Size: %08x#\n"
		"#%s Part 3: Type: %02x, Start: %08x, Size: %08x#",
		mbr->partitions[0].type, mbr->partitions[0].start_sct, mbr->partitions[0].size_sct,
		(mbr_ctx.available & BIT(0)) ? (mbr_ctx.resized_cnt[0] ? "FFDD00" : "C7EA46") : "C0C0C0",
		 mbr->partitions[1].type, mbr->partitions[1].start_sct, mbr->partitions[1].size_sct,
		(mbr_ctx.available & BIT(1)) ? (mbr_ctx.resized_cnt[1] ? "FFDD00" : "C7EA46") : "C0C0C0",
		 mbr->partitions[2].type, mbr->partitions[2].start_sct, mbr->partitions[2].size_sct,
		(mbr_ctx.available & BIT(2)) ? (mbr_ctx.resized_cnt[2] ? "FFDD00" : "C7EA46") : "C0C0C0",
		 mbr->partitions[3].type, mbr->partitions[3].start_sct, mbr->partitions[3].size_sct);

	if (mbr_ctx.resized_cnt[0] || mbr_ctx.resized_cnt[1] || mbr_ctx.resized_cnt[2])
		strcat(txt_buf, "\n\n#FFDD00 Note:# Yellow entries have USER partition resized.");

	if (!mbr_ctx.available)
		strcat(txt_buf, "\n#FF8000 Do you want to partition the SD card?#\n"
						  "#FF8000 (You will be asked on how to proceed)#");

	lv_mbox_set_text(mbox, txt_buf);
	free(txt_buf);
	free(mbr);

	if (mbr_ctx.available)
	{
		// Check available partitions and enable the corresponding buttons.
		if (mbr_ctx.available & 1)
			mbox_btn_parts[0][0] = '\222';
		else
			mbox_btn_parts[0][0] = '\262';
		if (mbr_ctx.available & 2)
			mbox_btn_parts[1][0] = '\222';
		else
			mbox_btn_parts[1][0] = '\262';
		if (mbr_ctx.available & 4)
			mbox_btn_parts[2][0] = '\222';
		else
			mbox_btn_parts[2][0] = '\262';

		lv_mbox_add_btns(mbox, (const char **)mbox_btn_parts, _create_emummc_raw_action);
	}
	else
		lv_mbox_add_btns(mbox, mbox_btn_format, _create_emummc_raw_format);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);
}

static lv_res_t _create_emummc_action(lv_obj_t * btns, const char * txt)
{
	int btn_idx = lv_btnm_get_pressed(btns);
	lv_obj_t *bg = lv_obj_get_parent(lv_obj_get_parent(btns));

	mbr_ctx.part_idx = 0;
	mbr_ctx.sector_start = 0;

	switch (btn_idx)
	{
	case 0:
		lv_obj_set_style(bg, &lv_style_transp);
		_create_window_emummc();
		break;
	case 1:
		_create_mbox_emummc_raw();
		break;
	}

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

static lv_res_t _create_mbox_emummc_create(lv_obj_t *btn)
{
	if (!nyx_emmc_check_battery_enough())
		return LV_RES_OK;

	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char * mbox_btn_map[] = { "\222SD File", "\222SD Partition", "\222Cancel", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	lv_mbox_set_text(mbox,
		"Welcome to #C7EA46 emuMMC# creation tool!\n\n"
		"Please choose what type of emuMMC you want to create.\n"
		"#FF8000 SD File# is saved as files in the FAT partition.\n"
		"#FF8000 SD Partition# is saved as raw image in an available partition.");

	lv_mbox_add_btns(mbox, mbox_btn_map, _create_emummc_action);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	return LV_RES_OK;
}

static void _change_raw_emummc_part_type()
{
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	sdmmc_storage_read(&sd_storage, 0, 1, mbr);
	mbr->partitions[mbr_ctx.part_idx].type = 0xE0;
	sdmmc_storage_write(&sd_storage, 0, 1, mbr);
	free(mbr);
}

static lv_res_t _save_emummc_cfg_mig_mbox_action(lv_obj_t *btns, const char *txt)
{
	// Delete main emuMMC and popup windows and relaunch main emuMMC window.
	lv_obj_del(emummc_manage_window);
	nyx_mbox_action(btns, txt);

	(*emummc_tools)(NULL);

	return LV_RES_INV;
}

static void _create_emummc_migrated_mbox()
{
	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\251", "OK", "\251", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 4);

	lv_mbox_set_text(mbox,
		"#FF8000 emuMMC Configuration#\n\n"
		"#96FF00 The emuMMC configuration#\n#96FF00 was saved to sd card!#");

	lv_mbox_add_btns(mbox, mbox_btn_map, _save_emummc_cfg_mig_mbox_action);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);
}

static void _migrate_sd_raw_emummc_based()
{
	char *tmp = (char *)malloc(0x80);
	s_printf(tmp, "emuMMC/RAW%d", mbr_ctx.part_idx);

	sd_mount();
	f_mkdir("emuMMC");
	f_mkdir(tmp);
	strcat(tmp, "/raw_based");

	FIL fp;
	if (!f_open(&fp, tmp, FA_CREATE_ALWAYS | FA_WRITE))
	{
		f_write(&fp, &mbr_ctx.sector_start, 4, NULL);
		f_close(&fp);
	}

	s_printf(tmp, "emuMMC/RAW%d", mbr_ctx.part_idx);

	_change_raw_emummc_part_type();

	save_emummc_cfg(mbr_ctx.part_idx, mbr_ctx.sector_start, tmp);
	_create_emummc_migrated_mbox();
	free(tmp);

	sd_unmount();
}

static void _migrate_sd_backup_file_based()
{
	char *emu_path = (char *)malloc(128);
	char *parts_path = (char *)malloc(128);
	char *backup_path = (char *)malloc(128);
	char *backup_file_path = (char *)malloc(128);

	sd_mount();
	f_mkdir("emuMMC");

	strcpy(emu_path, "emuMMC/BK");
	u32 base_len = strlen(emu_path);

	for (int j = 0; j < 100; j++)
	{
		update_emummc_base_folder(emu_path, base_len, j);
		if (f_stat(emu_path, NULL) == FR_NO_FILE)
			break;
	}
	base_len = strlen(emu_path);

	f_mkdir(emu_path);
	strcat(emu_path, "/eMMC");
	f_mkdir(emu_path);

	FIL fp;
	// Create file based flag.
	strcpy(emu_path + base_len, "/file_based");
	f_open(&fp, "emuMMC/BK00/file_based", FA_CREATE_ALWAYS | FA_WRITE);
	f_close(&fp);

	if (!emummc_backup)
		emmcsn_path_impl(backup_path, "", "", NULL);
	else
		emmcsn_path_impl(backup_path, "/emummc", "", NULL);

	// Move BOOT0.
	s_printf(backup_file_path, "%s/BOOT0", backup_path);
	strcpy(emu_path + base_len, "/eMMC/BOOT0");
	f_rename(backup_file_path, emu_path);

	// Move BOOT1.
	s_printf(backup_file_path, "%s/BOOT1", backup_path);
	strcpy(emu_path + base_len, "/eMMC/BOOT1");
	f_rename(backup_file_path, emu_path);

	// Move raw GPP.
	bool multipart = false;
	s_printf(backup_file_path, "%s/rawnand.bin", backup_path);

	if (f_stat(backup_file_path, NULL))
		multipart = true;

	if (!multipart)
	{
		strcpy(emu_path + base_len, "/eMMC/00");
		f_rename(backup_file_path, emu_path);
	}
	else
	{
		emu_path[base_len] = 0;
		for (int i = 0; i < 32; i++)
		{
			s_printf(backup_file_path, "%s/rawnand.bin.%02d", backup_path, i);
			s_printf(parts_path, "%s/eMMC/%02d", emu_path, i);
			if (f_rename(backup_file_path, parts_path))
				break;
		}
	}

	free(emu_path);
	free(parts_path);
	free(backup_path);
	free(backup_file_path);

	save_emummc_cfg(0, 0, "emuMMC/BK00");
	_create_emummc_migrated_mbox();
	sd_unmount();
}

static lv_res_t _create_emummc_mig0_action(lv_obj_t * btns, const char * txt)
{
	switch (lv_btnm_get_pressed(btns))
	{
	case 0:
		_migrate_sd_raw_emummc_based();
		break;
	}

	mbr_ctx.part_idx = 0;
	mbr_ctx.sector_start = 0;

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

static lv_res_t _create_emummc_mig1_action(lv_obj_t * btns, const char * txt)
{
	switch (lv_btnm_get_pressed(btns))
	{
	case 0:
		_migrate_sd_backup_file_based();
		break;
	}

	mbr_ctx.part_idx = 0;
	mbr_ctx.sector_start = 0;

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

static lv_res_t _create_emummc_migrate_action(lv_obj_t * btns, const char * txt)
{
	bool backup = false;
	bool emummc = false;

	switch (lv_btnm_get_pressed(btns))
	{
	case 0:
		backup = true;
		break;
	case 1:
		emummc = true;
		break;
	case 2:
		nyx_mbox_action(btns, txt);
		return LV_RES_INV;
	}

	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\222Continue", "\222Cancel", "" };
	static const char *mbox_btn_map1[] = { "\251", "OK", "\251", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	char *txt_buf = (char *)malloc(SZ_16K);

	if (backup)
	{
		if (!emummc_backup)
			s_printf(txt_buf,
				"#C7EA46 Found suitable eMMC backup!#\n\n"
				"#FF8000 Do you want to migrate it?#\n");
		else
			s_printf(txt_buf,
				"#C7EA46 Found suitable emuMMC backup!#\n\n"
				"#FF8000 Do you want to migrate it?#\n");
		lv_mbox_add_btns(mbox, mbox_btn_map, _create_emummc_mig1_action);
	}
	else if (emummc)
	{
		s_printf(txt_buf,
			"#C7EA46 Found SD Partition based emuMMC!#\n\n"
			"#FF8000 Do you want to repair the config and partition type for it?#\n");
		lv_mbox_add_btns(mbox, mbox_btn_map, _create_emummc_mig0_action);
	}
	else
	{
		s_printf(txt_buf, "No emuMMC found!\n");
		lv_mbox_add_btns(mbox, mbox_btn_map1, nyx_mbox_action);
	}

	lv_mbox_set_text(mbox, txt_buf);
	free(txt_buf);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	nyx_mbox_action(btns, txt);

	return LV_RES_INV;
}

typedef struct _emummc_images_t
{
	dirlist_t *dirlist;
	u32 part_sector[3];
	u32 part_type[3];
	u32 part_end[3];
	char part_path[3 * 128];
	lv_obj_t *win;
    lv_obj_t *file_items,*file_counter;
    u32 file_count,file_page;
} emummc_images_t;

static lv_res_t _create_mbox_emummc_migrate(lv_obj_t *btn)
{
	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static char *mbox_btn_map[] = { "\262Backup", "\262Fix RAW", "\222Cancel", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 6);

	lv_mbox_set_text(mbox,
		"Welcome to #C7EA46 emuMMC# migration tool!\n\n"
		"Please choose what type of migration you want to do.\n"
		"Anything that was not found will have the button disabled.");

	char *path_buf = (char *)malloc(0x512);
	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	u8 *efi_part = (u8 *)malloc(0x200);

	sd_mount();
	sdmmc_storage_read(&sd_storage, 0, 1, mbr);

	emmc_initialize(false);

	bool backup = false;
	bool emummc = false;
	bool rawnand_backup = false;

	mbr_ctx.sector_start = 0;
	mbr_ctx.part_idx = 0;

	// Try to find a partition based emuMMC.
	for (int i = 1; i < 4; i++)
	{
		mbr_ctx.sector_start = mbr->partitions[i].start_sct;

		if (!mbr_ctx.sector_start)
			continue;

		sdmmc_storage_read(&sd_storage, mbr_ctx.sector_start + 0xC001, 1, efi_part);
		if (!memcmp(efi_part, "EFI PART", 8))
		{
			mbr_ctx.sector_start += 0x8000;
			emummc = true;
			mbr_ctx.part_idx = i;
			break;
		}
		else
		{
			sdmmc_storage_read(&sd_storage, mbr_ctx.sector_start + 0x4001, 1, efi_part);
			if (!memcmp(efi_part, "EFI PART", 8))
			{
				emummc = true;
				mbr_ctx.part_idx = i;
				break;
			}
		}
	}

	emummc_backup = false;

	emmcsn_path_impl(path_buf, "", "BOOT0", &emmc_storage);
	if (!f_stat(path_buf, NULL))
		backup = true;

	emmcsn_path_impl(path_buf, "", "rawnand.bin", &emmc_storage);
	if (!f_stat(path_buf, NULL))
		rawnand_backup = true;

	emmcsn_path_impl(path_buf, "", "rawnand.bin.00", &emmc_storage);
	if (!f_stat(path_buf, NULL))
		rawnand_backup = true;

	backup = backup && rawnand_backup;

	if (!backup)
	{
		rawnand_backup = false;
		emummc_backup = true;

		emmcsn_path_impl(path_buf, "/emummc", "BOOT0", &emmc_storage);
		if (!f_stat(path_buf, NULL))
			backup = true;

		emmcsn_path_impl(path_buf, "/emummc", "rawnand.bin", &emmc_storage);
		if (!f_stat(path_buf, NULL))
			rawnand_backup = true;

		emmcsn_path_impl(path_buf, "/emummc", "rawnand.bin.00", &emmc_storage);
		if (!f_stat(path_buf, NULL))
			rawnand_backup = true;

		backup = backup && rawnand_backup;
	}

	sd_unmount();
	emmc_end();

	// Check available types and enable the corresponding buttons.
	if (backup)
		mbox_btn_map[0][0] = '\222';
	else
		mbox_btn_map[0][0] = '\262';
	if (emummc)
		mbox_btn_map[1][0] = '\222';
	else
		mbox_btn_map[1][0] = '\262';

	free(path_buf);
	free(mbr);
	free(efi_part);

	lv_mbox_add_btns(mbox, (const char **)mbox_btn_map, _create_emummc_migrate_action);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);

	return LV_RES_OK;
}

static emummc_images_t *emummc_img;

static lv_res_t _save_emummc_cfg_mbox_action(lv_obj_t *btns, const char *txt)
{
	// Free components, delete main emuMMC and popup windows and relaunch main emuMMC window.
	lv_obj_del(emummc_img->win);
	lv_obj_del(emummc_manage_window);
	free(emummc_img->dirlist);
	free(emummc_img);

	nyx_mbox_action(btns, txt);

	(*emummc_tools)(NULL);

	return LV_RES_INV;
}

static void _create_emummc_saved_mbox()
{
	lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_set_style(dark_bg, &mbox_darken);
	lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

	static const char *mbox_btn_map[] = { "\251", "OK", "\251", "" };
	lv_obj_t * mbox = lv_mbox_create(dark_bg, NULL);
	lv_mbox_set_recolor_text(mbox, true);
	lv_obj_set_width(mbox, LV_HOR_RES / 9 * 4);

	lv_mbox_set_text(mbox,
		"#FF8000 emuMMC Configuration#\n\n"
		"#96FF00 The emuMMC configuration#\n#96FF00 was saved to sd card!#");

	lv_mbox_add_btns(mbox, mbox_btn_map, _save_emummc_cfg_mbox_action);

	lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_top(mbox, true);
}

static lv_res_t _save_raw_emummc_cfg_action(lv_obj_t * btn)
{
	lv_btn_ext_t *ext = lv_obj_get_ext_attr(btn);
	switch (ext->idx)
	{
	case 0:
		save_emummc_cfg(1, emummc_img->part_sector[0], &emummc_img->part_path[0]);
		break;
	case 1:
		save_emummc_cfg(2, emummc_img->part_sector[1], &emummc_img->part_path[128]);
		break;
	case 2:
		save_emummc_cfg(3, emummc_img->part_sector[2], &emummc_img->part_path[256]);
		break;
	}

	_create_emummc_saved_mbox();
	sd_unmount();

	return LV_RES_INV;
}

static lv_res_t _save_disable_emummc_cfg_action(lv_obj_t * btn)
{
	save_emummc_cfg(0, 0, NULL);
	_create_emummc_saved_mbox();
	sd_unmount();

	return LV_RES_INV;
}

static lv_res_t _save_file_emummc_cfg_action(lv_obj_t *btn)
{
	u32 index=lv_obj_get_free_num(btn);
    if(!emummc_img->dirlist||index>=emummc_img->file_count)return LV_RES_OK;
    char path[512];s_printf(path,"emuMMC/%s",emummc_img->dirlist->name[index]);
    save_emummc_cfg(0,0,path);
	_create_emummc_saved_mbox();
	sd_unmount();

	return LV_RES_INV;
}


static void _fill_emummc_files(void)
{
    lv_obj_t *holder=emummc_img->file_items;lv_obj_clean(holder);
    int width=lv_obj_get_width(holder),height=lv_obj_get_height(holder),row_height=(height-36)/4;
    for(u32 row=0;row<4;++row){
        u32 index=emummc_img->file_page*4+row;if(index>=emummc_img->file_count)break;
        lv_obj_t *button=lv_btn_create(holder,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
        lv_obj_set_pos(button,0,row*(row_height+12));lv_obj_set_size(button,width,row_height);lv_obj_set_free_num(button,index);
        lv_btn_set_action(button,LV_BTN_ACTION_CLICK,_save_file_emummc_cfg_action);
        lv_obj_t *label=nyx_nova_text(button,"",16,(row_height-26)/2,width-32,NOVA_BODY);
        (lv_label_set_text)(label,emummc_img->dirlist->name[index]);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,30);
    }
    if(!emummc_img->file_count){lv_obj_t *label=nyx_nova_text(holder,"No emuMMC found.",0,height/2-30,width,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,60);}
    char position[32];s_printf(position,"%d / %d",emummc_img->file_page+1,(emummc_img->file_count+3)/4?(emummc_img->file_count+3)/4:1);(lv_label_set_text)(emummc_img->file_counter,position);
}
static lv_res_t _step_emummc_files(lv_obj_t *button)
{
    u32 pages=(emummc_img->file_count+3)/4;if(!pages)pages=1;
    emummc_img->file_page=(emummc_img->file_page+(lv_obj_get_free_num(button)?1:pages-1))%pages;_fill_emummc_files();return LV_RES_OK;
}
static void _create_change_emummc_ui(lv_obj_t *win,u32 file_count)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;
    emummc_img->file_count=file_count;emummc_img->file_page=0;
    lv_obj_t *left=nyx_nova_panel(page,34,18,594,height),*right=nyx_nova_panel(page,652,18,594,height);
    lv_obj_t *title=nyx_nova_text(left,"SD Raw Partitions",24,22,546,NOVA_TITLE);lv_label_set_align(title,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(title,38);
    title=nyx_nova_text(right,"SD File Based",24,22,546,NOVA_TITLE);lv_label_set_align(title,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(title,38);
    int row_height=(height-112)/3;
    for(u32 i=0;i<3;++i){
        char name[32],details[256];s_printf(name,"SD RAW %d",i+1);
        s_printf(details,nyx_tr("Sector start: 0x%08X\nFolder: %s"),emummc_img->part_sector[i],&emummc_img->part_path[i*128]);
        lv_obj_t *button=nyx_nova_action(left,name,details,"database",_save_raw_emummc_cfg_action,i,24,78+i*(row_height+12),546,row_height);
        ((lv_btn_ext_t *)lv_obj_get_ext_attr(button))->idx=i;
        if(!emummc_img->part_sector[i]||emummc_img->part_type[i]==0x83||!emummc_img->part_path[i*128]){lv_obj_set_click(button,false);lv_btn_set_state(button,LV_BTN_STATE_INA);}
    }
    emummc_img->file_items=nyx_nova_panel(right,24,78,546,height-164);lv_cont_set_style(emummc_img->file_items,&lv_style_transp_tight);
    for(u32 i=0;i<2;++i){
        lv_obj_t *button=lv_btn_create(right,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);lv_obj_set_size(button,170,42);lv_obj_set_pos(button,i?400:24,height-60);lv_obj_set_free_num(button,i);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,_step_emummc_files);
        lv_obj_t *label=nyx_nova_text(button,i?"Suivant":"Précédent",8,10,154,NOVA_SMALL);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);
    }
    emummc_img->file_counter=nyx_nova_text(right,"",210,height-48,170,NOVA_SMALL);lv_label_set_align(emummc_img->file_counter,LV_LABEL_ALIGN_CENTER);_fill_emummc_files();
}

static lv_res_t _action_win_change_emummc_close(lv_obj_t *btn)
{
	free(emummc_img->dirlist);
	free(emummc_img);

	return nyx_win_close_action(btn);
}

static lv_res_t _create_change_emummc_window(lv_obj_t *btn_caller)
{
	lv_obj_t *win = nyx_create_standard_window(SYMBOL_SETTINGS"  Change emuMMC", _action_win_change_emummc_close);
	lv_win_add_btn(win, NULL, SYMBOL_POWER"  Disable", _save_disable_emummc_cfg_action);

	sd_mount();

	emummc_img = malloc(sizeof(emummc_images_t));
	emummc_img->win = win;

	mbr_t *mbr = (mbr_t *)malloc(sizeof(mbr_t));
	char *path = malloc(512);

	sdmmc_storage_read(&sd_storage, 0, 1, mbr);

	memset(emummc_img->part_path, 0, 3 * 128);

	for (int i = 1; i < 4; i++)
	{
		emummc_img->part_sector[i - 1] = mbr->partitions[i].start_sct;
		emummc_img->part_end[i - 1]    = emummc_img->part_sector[i - 1] + mbr->partitions[i].size_sct - 1;
		emummc_img->part_type[i - 1]   = mbr->partitions[i].type;
	}
	free(mbr);

	emummc_img->dirlist = dirlist("emuMMC", NULL, DIR_SHOW_DIRS);

	u32 file_based_idx = 0;
	if (!emummc_img->dirlist)
		goto out0;

	u32 emummc_idx = 0;

	FIL fp;

	// Check for sd raw partitions, based on the folders in /emuMMC.
	while (emummc_img->dirlist->name[emummc_idx])
	{
		s_printf(path, "emuMMC/%s/raw_based", emummc_img->dirlist->name[emummc_idx]);

		if (!f_stat(path, NULL))
		{
			f_open(&fp, path, FA_READ);
			u32 curr_list_sector = 0;
			f_read(&fp, &curr_list_sector, 4, NULL);
			f_close(&fp);

			// Check if there's a HOS image there.
			if ((curr_list_sector == 2) || (emummc_img->part_sector[0] && curr_list_sector >= emummc_img->part_sector[0] &&
				curr_list_sector < emummc_img->part_end[0] && emummc_img->part_type[0] != 0x83))
			{
				s_printf(&emummc_img->part_path[0], "emuMMC/%s", emummc_img->dirlist->name[emummc_idx]);
				emummc_img->part_sector[0] = curr_list_sector;
				emummc_img->part_end[0]    = 0;
			}
			else if (emummc_img->part_sector[1] && curr_list_sector >= emummc_img->part_sector[1] &&
				curr_list_sector < emummc_img->part_end[1] && emummc_img->part_type[1] != 0x83)
			{
				s_printf(&emummc_img->part_path[1 * 128], "emuMMC/%s", emummc_img->dirlist->name[emummc_idx]);
				emummc_img->part_sector[1] = curr_list_sector;
				emummc_img->part_end[1]    = 0;
			}
			else if (emummc_img->part_sector[2] && curr_list_sector >= emummc_img->part_sector[2] &&
				curr_list_sector < emummc_img->part_end[2] && emummc_img->part_type[2] != 0x83)
			{
				s_printf(&emummc_img->part_path[2 * 128], "emuMMC/%s", emummc_img->dirlist->name[emummc_idx]);
				emummc_img->part_sector[2] = curr_list_sector;
				emummc_img->part_end[2]    = 0;
			}
		}
		emummc_idx++;
	}

	emummc_idx = 0;

	// Sanitize the directory list with sd file based ones.
	while (emummc_img->dirlist->name[emummc_idx])
	{
		s_printf(path, "emuMMC/%s/file_based", emummc_img->dirlist->name[emummc_idx]);

		if (!f_stat(path, NULL))
		{
			char *tmp = emummc_img->dirlist->name[emummc_idx];
			memcpy(emummc_img->dirlist->name[file_based_idx], tmp, strlen(tmp) + 1);
			file_based_idx++;
		}
		emummc_idx++;
	}
	emummc_img->dirlist->name[file_based_idx] = NULL;

out0:;
    _create_change_emummc_ui(win,emummc_img->dirlist?file_based_idx:0);

	free(path);
	sd_unmount();

	return LV_RES_OK;
}

lv_res_t create_win_emummc_tools(lv_obj_t *btn)
{
    (void)btn;
    lv_obj_t *win=nyx_create_standard_window(SYMBOL_EDIT" emuMMC Manage",NULL);
    emummc_manage_window=win;emummc_tools=(void *)create_win_emummc_tools;
    sd_mount();emummc_cfg_t info;load_emummc_cfg(&info);sd_unmount();
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    int height=lv_obj_get_height(page)-36;
    lv_obj_t *panel=nyx_nova_panel(win,34,18,594,height);
    nyx_nova_icon_box(panel,"database",24,24,48,false);
    lv_obj_t *state=nyx_nova_text(panel,info.enabled?"Enabled":"Disabled",90,38,456,NOVA_TITLE);
    nyx_nova_fit_text(state,36);
    char details[1024],paths[2][256];
    const char *sources[]={info.path?info.path:"",info.nintendo_path?info.nintendo_path:""};
    for(u32 i=0;i<2;++i){u32 length=strlen(sources[i]),copy=length>252?252:length;while(copy&&((u8)sources[i][copy]&0xC0)==0x80)--copy;memcpy(paths[i],sources[i],copy);paths[i][copy]=0;if(copy<length)strcat(paths[i],"…");}
    if(info.enabled&&info.sector)s_printf(details,nyx_tr("Type : partition SD\nSecteur : 0x%08X\nDossier Nintendo : %s"),info.sector,paths[1]);
    else if(info.enabled)s_printf(details,nyx_tr("Type : fichiers SD\nDossier : %s\nDossier Nintendo : %s"),paths[0],paths[1]);
    else strcpy(details,nyx_tr("emuMMC is disabled and eMMC will be used for boot."));
    lv_obj_t *text=nyx_nova_text(panel,details,24,112,546,NOVA_BODY);nyx_nova_fit_text(text,height-140);
    free(info.path);free(info.nintendo_path);
    int tile_height=(height-32)/3;
    nyx_nova_action(win,"Change emuMMC","Choisir une image emuMMC sur la carte SD.","settings-2",_create_change_emummc_window,0,652,18,594,tile_height);
    nyx_nova_action(win,"Create emuMMC","Créer une copie sous forme de fichiers ou dans une partition SD.","database",_create_mbox_emummc_create,0,652,34+tile_height,594,tile_height);
    nyx_nova_action(win,"Migrate emuMMC","Migrer une sauvegarde ou réparer une partition emuMMC existante.","refresh-cw",_create_mbox_emummc_migrate,0,652,50+tile_height*2,594,tile_height);
    return LV_RES_OK;
}
