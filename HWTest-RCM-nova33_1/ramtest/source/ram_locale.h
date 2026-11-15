/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RAM_LOCALE_H
#define RAM_LOCALE_H
#include <utils/types.h>
#include <stdarg.h>
#include "nyx_ram_handoff.h"
#define RAM_UI_BASE NYX_RAM_UI_ADDR
#define RAM_UI_LIMIT NYX_RAM_UI_MAX
#define RAM_UI_MAGIC NYX_RAM_UI_MAGIC
bool ram_locale_bind(const void *data, u32 available);
void ram_locale_init(bool from_nyx);
const char *ram_tr(const char *source);
const u8 *ram_ui_glyph(u32 codepoint);
u32 ram_ui_glyph_size(void);
void ram_ui_vsnprintf(char *out, u32 size, const char *format, va_list arguments);
void ram_ui_snprintf(char *out, u32 size, const char *format, ...);
void ram_ui_printf(const char *format, ...);
#endif
