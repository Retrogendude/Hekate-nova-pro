/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_GUI_READABILITY_H
#define NYX_GUI_READABILITY_H
#include <libs/lvgl/lvgl.h>

/* Adapt after positions/sizes are final. Does not alter application text,
 * button IDs, font choice, card heights or absolute layout positions. */
void nyx_label_adapt(lv_obj_t *label);
void nyx_readability_refresh(lv_obj_t *root);
/* UI setters can request a coalesced pass after the creating callback ends. */
void nyx_readability_request(void);
#endif
