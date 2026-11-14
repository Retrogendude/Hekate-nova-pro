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
#include <string.h>

#include <bdk.h>

#include "gui.h"
#include "gui_emmc_tools.h"
#include "gui_tools.h"
#include "fe_emmc_tools.h"
#include "../config.h"
#include "../hos/pkg1.h"
#include "../hos/pkg2.h"
#include "../hos/hos.h"
#include <libs/fatfs/ff.h>

extern char *emmcsn_path_impl(char *path, char *sub_dir, char *filename, sdmmc_storage_t *storage);

typedef struct _emmc_backup_buttons_t
{
	lv_obj_t *emmc_boot;
	lv_obj_t *emmc_raw_gpp;
	lv_obj_t *emmc_sys;
	lv_obj_t *emmc_usr;
	bool raw_emummc;
	bool restore;
} emmc_backup_buttons_t;

static emmc_backup_buttons_t emmc_btn_ctxt;

static void _create_window_backup_restore(emmcPartType_t type, const char* win_label)
{
	emmc_tool_gui_t emmc_tool_gui_ctxt;

	emmc_tool_gui_ctxt.raw_emummc = emmc_btn_ctxt.raw_emummc;

	char win_label_full[80];

	s_printf(win_label_full,nyx_tr(emmc_btn_ctxt.restore?"Restore %s":"Backup %s"),nyx_nova_caption(win_label));

	lv_obj_t *win = nyx_create_standard_window(win_label_full, NULL);

	//Disable buttons.
	nyx_window_toggle_buttons(win, true);

    nyx_nova_transfer(win,&emmc_tool_gui_ctxt);

	if (!emmc_btn_ctxt.restore)
		dump_emmc_selected(type, &emmc_tool_gui_ctxt);
	else
		restore_emmc_selected(type, &emmc_tool_gui_ctxt);

	nyx_window_toggle_buttons(win, false);

	// Refresh AutoRCM button.
	if (emmc_btn_ctxt.restore && (type == PART_BOOT) && !emmc_btn_ctxt.raw_emummc)
	{
		if (get_set_autorcm_status(false))
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_TGL_REL);
		else
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_REL);
		nyx_generic_onoff_toggle(autorcm_btn);

		if (h_cfg.rcm_patched)
		{
			lv_obj_set_click(autorcm_btn, false);
			lv_btn_set_state(autorcm_btn, LV_BTN_STATE_INA);
		}
	}
}

static lv_res_t _emmc_backup_buttons_decider(lv_obj_t *btn)
{
	if (!nyx_emmc_check_battery_enough())
		return LV_RES_OK;

	char *win_label = lv_label_get_text(lv_obj_get_child(btn, NULL));

	if (emmc_btn_ctxt.emmc_boot == btn)
		_create_window_backup_restore(PART_BOOT, win_label);
	else if (emmc_btn_ctxt.emmc_raw_gpp == btn)
		_create_window_backup_restore(PART_RAW, win_label);
	else if (emmc_btn_ctxt.emmc_sys == btn)
	{
		if (!emmc_btn_ctxt.restore)
			_create_window_backup_restore(PART_SYSTEM, win_label);
		else
			_create_window_backup_restore(PART_GP_ALL, win_label);
	}
	else if (!emmc_btn_ctxt.restore && emmc_btn_ctxt.emmc_usr == btn)
		_create_window_backup_restore(PART_USER, win_label);

	return LV_RES_OK;
}

static lv_res_t _emmc_backup_buttons_raw_toggle(lv_obj_t *btn)
{
	nyx_generic_onoff_toggle(btn);

	lv_obj_set_click(emmc_btn_ctxt.emmc_boot, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_boot, LV_BTN_STATE_REL);
	lv_obj_set_click(emmc_btn_ctxt.emmc_raw_gpp, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_raw_gpp, LV_BTN_STATE_REL);

	lv_obj_set_click(emmc_btn_ctxt.emmc_sys, true);
	lv_btn_set_state(emmc_btn_ctxt.emmc_sys, LV_BTN_STATE_REL);

	// Backup/Restore from and to eMMC.
	if (!(lv_btn_get_state(btn) & LV_BTN_STATE_TGL_REL))
	{
		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_UPLOAD"  eMMC BOOT0 & BOOT1");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_DOWNLOAD"  eMMC BOOT0 & BOOT1");
		lv_obj_realign(emmc_btn_ctxt.emmc_boot);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_UPLOAD"  eMMC RAW GPP");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_DOWNLOAD"  eMMC RAW GPP");
		lv_obj_realign(emmc_btn_ctxt.emmc_raw_gpp);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_sys, NULL), SYMBOL_MODULES"  eMMC SYS");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_sys, NULL), SYMBOL_MODULES"  eMMC ALL");
		lv_obj_realign(emmc_btn_ctxt.emmc_sys);

		if (!emmc_btn_ctxt.restore)
		{
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_usr, NULL), SYMBOL_MODULES_ALT"  eMMC USER");
			lv_obj_realign(emmc_btn_ctxt.emmc_usr);

			lv_obj_set_click(emmc_btn_ctxt.emmc_usr, true);
			lv_btn_set_state(emmc_btn_ctxt.emmc_usr, LV_BTN_STATE_REL);
		}

		emmc_btn_ctxt.raw_emummc = false;
	}
	else // Backup/Restore from and to emuMMC.
	{
		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_UPLOAD"  SD emuMMC BOOT0 & BOOT1");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_boot, NULL), SYMBOL_DOWNLOAD"  SD emuMMC BOOT0 & BOOT1");
		lv_obj_realign(emmc_btn_ctxt.emmc_boot);

		if (!emmc_btn_ctxt.restore)
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_UPLOAD"  SD emuMMC RAW GPP");
		else
			lv_label_set_static_text(lv_obj_get_child(emmc_btn_ctxt.emmc_raw_gpp, NULL), SYMBOL_DOWNLOAD"  SD emuMMC RAW GPP");
		lv_obj_realign(emmc_btn_ctxt.emmc_raw_gpp);

		lv_obj_set_click(emmc_btn_ctxt.emmc_sys, false);
		lv_btn_set_state(emmc_btn_ctxt.emmc_sys, LV_BTN_STATE_INA);

		if (!emmc_btn_ctxt.restore)
		{
			lv_obj_set_click(emmc_btn_ctxt.emmc_usr, false);
			lv_btn_set_state(emmc_btn_ctxt.emmc_usr, LV_BTN_STATE_INA);
		}

		emmc_btn_ctxt.raw_emummc = true;
	}
	/* Keep the new card titles and disabled state consistent when the target
	 * changes. The single-label button contract is preserved for the decider. */
	lv_obj_t *cards[]={emmc_btn_ctxt.emmc_boot,emmc_btn_ctxt.emmc_raw_gpp,emmc_btn_ctxt.emmc_sys,emmc_btn_ctxt.emmc_usr};
	for(u32 i=0;i<4;++i)if(cards[i]){
		lv_obj_t *label=lv_obj_get_child(cards[i],NULL);
		const char *caption=nyx_nova_caption(lv_label_get_text(label));
		char title[160];u32 length=strlen(caption);if(length>=sizeof(title))length=sizeof(title)-1;
		memcpy(title,caption,length);title[length]=0;lv_label_set_text(label,title);
		lv_label_set_style(label,(lv_style_t *)nyx_nova_text_style(lv_obj_get_click(cards[i])?NOVA_BODY:NOVA_MUTED));
		nyx_nova_fit_text(label,58);
	}

	return LV_RES_OK;
}

lv_res_t create_window_backup_restore_tool(lv_obj_t *btn)
{
    emmc_btn_ctxt.restore=lv_obj_get_free_num(btn)==1;
    lv_obj_t *win=nyx_create_standard_window(emmc_btn_ctxt.restore?SYMBOL_SD" Restore":SYMBOL_SD" Backup",NULL);
    const char *descriptions[]={
        "Partitions BOOT0/BOOT1, à conserver avec RAW GPP.",
        "Stockage complet : CAL0, SYSTEM, USER et fichiers de démarrage.",
        "Partitions système de RAW GPP.",
        "Partition USER. Cette copie seule est incomplète."
    };
    int height=(lv_obj_get_height(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page)-132)/2;
    emmc_btn_ctxt.emmc_boot=nyx_nova_action(win,"eMMC BOOT0 & BOOT1",descriptions[0],"hard-drive-download",_emmc_backup_buttons_decider,0,34,18,594,height);
    emmc_btn_ctxt.emmc_raw_gpp=nyx_nova_action(win,"eMMC RAW GPP",descriptions[1],"hard-drive",_emmc_backup_buttons_decider,0,652,18,594,height);
    emmc_btn_ctxt.emmc_sys=nyx_nova_action(win,emmc_btn_ctxt.restore?"eMMC ALL":"eMMC SYS",descriptions[2],"boxes",_emmc_backup_buttons_decider,0,34,34+height,594,height);
    emmc_btn_ctxt.emmc_usr=emmc_btn_ctxt.restore?NULL:nyx_nova_action(win,"eMMC USER",descriptions[3],"folder-open",_emmc_backup_buttons_decider,0,652,34+height,594,height);
    lv_obj_t *panel=nyx_nova_panel(win,34,50+height*2,1212,64);
    nyx_nova_switch_row(panel,"SD emuMMC Raw Partition","Utiliser emuMMC sur la carte SD à la place de l’eMMC physique.","database",_emmc_backup_buttons_raw_toggle,false,18,0,1176,64);
    emmc_btn_ctxt.raw_emummc=false;
    return LV_RES_OK;
}
