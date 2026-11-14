#ifndef GUI_DIAGNOSTICS_H
#define GUI_DIAGNOSTICS_H
#include "gui.h"
void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent);
const char *nyx_diag_probe_caption(u32 index);
void nyx_components_window(lv_action_t report,lv_action_t export_action);
/* Success transfers control and does not return. Errors: 1 SD, 2 RAM image,
 * 3 return image, 4 size/format, 5 diagnostic busy, 6 handoff returned. */
int nyx_launch_ram_service(u32 mode,u32 mask);
#endif
