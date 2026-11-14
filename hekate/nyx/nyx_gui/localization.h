/* Native Nyx language support; UI text only, never paths or boot names. */
#ifndef NYX_LOCALIZATION_H
#define NYX_LOCALIZATION_H
#include <bdk.h>
#include <libs/lvgl/lvgl.h>
#define I18N_EN 0
#define I18N_FR 1
#define I18N_ES 2
#define I18N_IT 3
#define I18N_RU 4
#define I18N_DE 5
#define I18N_PT 6
#define I18N_LANGUAGE_COUNT 7
const char *nyx_tr(const char *text);
void nyx_localization_init(void);
void nyx_label_set_text(lv_obj_t *label, const char *text);
void nyx_label_set_static_text(lv_obj_t *label, const char *text);
void nyx_mbox_set_text(lv_obj_t *mbox, const char *text);
void nyx_ddlist_set_options(lv_obj_t *list, const char *options);
void nyx_roller_set_options(lv_obj_t *roller, const char *options);
void nyx_mbox_add_btns(lv_obj_t *mbox, const char **map, lv_btnm_action_t action);
void nyx_btnm_set_map(lv_obj_t *buttons, const char **map);
void nyx_win_set_title(lv_obj_t *win, const char *title);
lv_obj_t *nyx_win_add_btn(lv_obj_t *win, const void *image, const char *label, lv_action_t action);
lv_obj_t *nyx_tabview_add_tab(lv_obj_t *tabs, const char *title);
#endif
