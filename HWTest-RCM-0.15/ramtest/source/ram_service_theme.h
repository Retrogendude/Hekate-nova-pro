/* Match the modern Nyx palette without pulling LVGL into the IRAM service. */
#ifndef RAM_SERVICE_THEME_H
#define RAM_SERVICE_THEME_H
#include "ui_theme.h"
#undef COL_HEADER
#undef COL_DEFAULT
#undef COL_OK
#undef COL_BG
#define COL_HEADER  0xFF93E1AEu
#define COL_DEFAULT 0xFFF2F7F3u
#define COL_OK      0xFF93E1AEu
#define COL_BG      0xFF141A19u
#define COL_CARD    0xFF202B27u
#define COL_MUTED   0xFFC3D4C8u
#endif
