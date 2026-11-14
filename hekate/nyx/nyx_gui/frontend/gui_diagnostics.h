#ifndef GUI_DIAGNOSTICS_H
#define GUI_DIAGNOSTICS_H
#include "gui.h"
void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent);
/* Success transfers control and does not return. Errors: 1 SD, 2 RAM image,
 * 3 return image, 4 size/format, 5 diagnostic busy, 6 handoff returned. */
int nyx_launch_ram_service(void);
#endif
