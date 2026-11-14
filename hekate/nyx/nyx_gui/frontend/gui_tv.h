#ifndef GUI_TV_H
#define GUI_TV_H
#include "gui.h"
lv_res_t gui_tv_open(lv_obj_t *btn);
void gui_tv_dirty(int y1, int y2);
void gui_tv_shutdown(bool save_report);
#endif
