/* Match the modern Nyx palette without pulling LVGL into the IRAM service. */
#ifndef RAM_SERVICE_THEME_H
#define RAM_SERVICE_THEME_H
#include "ui_theme.h"
#undef COL_HEADER
#undef COL_DEFAULT
#undef COL_OK
#undef COL_BG
#define COL_HEADER  0xFF55C9F1u
#define COL_DEFAULT 0xFFF2F5FAu
#define COL_OK      0xFF93E1AEu
#define COL_BG      0xFF10151Du
#define COL_CARD    0xFF1C2937u
#define COL_MUTED   0xFF93A8BAu
#endif
