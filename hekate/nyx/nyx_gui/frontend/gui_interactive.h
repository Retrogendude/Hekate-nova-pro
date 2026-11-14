#ifndef NYX_INTERACTIVE_H
#define NYX_INTERACTIVE_H
#include "gui.h"
#include <input/joycon.h>
#include <input/touch.h>
bool nyx_interactive_joycon_active(void);
bool nyx_interactive_joycon(const jc_gamepad_rpt_t *pad);
bool nyx_interactive_touch(const touch_event_t *event);
lv_res_t nyx_joycon_test(lv_obj_t *button);
lv_res_t nyx_touch_test(lv_obj_t *button);
lv_res_t nyx_screen_patterns(lv_obj_t *button);
#endif
