/* Rectangular hue/tint picker, matching the supplied reference. GPL-2.0-or-later. */
#ifndef NYX_COLOR_PICKER_H
#define NYX_COLOR_PICKER_H
#include <libs/lvgl/lvgl.h>
typedef void (*nyx_color_changed_t)(lv_obj_t *picker,u32 rgb);
lv_obj_t *nyx_color_picker_create(lv_obj_t *parent,int x,int y,int width,int height,nyx_color_changed_t changed);
void nyx_color_picker_set(lv_obj_t *picker,u32 rgb);
u32 nyx_color_picker_rgb(lv_obj_t *picker);
#endif
