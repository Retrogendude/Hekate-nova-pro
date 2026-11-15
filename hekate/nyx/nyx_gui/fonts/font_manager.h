/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_FONT_MANAGER_H
#define NYX_FONT_MANAGER_H
#include <libs/lvgl/lv_misc/lv_font.h>
#include <utils/types.h>

/* Initialize after resource loading, lv_init and nyx_unicode_fonts_init. */
void nyx_font_manager_init(void);
const lv_font_t *nyx_font_regular(void);
const lv_font_t *nyx_font_small(void);
const lv_font_t *nyx_font_mono(void);
/* Cached typographic roles for Nova. Requested pixels are based on 30px UI
 * preference, scaled once at screen creation, never during a paint. */
const lv_font_t *nyx_font_ui(u32 pixels);
const char *nyx_font_options(void);
const char *nyx_font_filename(u32 index);
u32 nyx_font_count(void);
u32 nyx_font_selected_index(const char *filename);
bool nyx_font_available(const char *filename);
bool nyx_font_available_at_size(const char *filename, u32 size);
const char *nyx_font_status(void);
u32 nyx_font_size(void);
/* The saved personal face is never overwritten by a language-specific face. */
bool nyx_font_language_locked(u32 language);
const char *nyx_font_effective_filename(u32 language, const char *personal);
const char *nyx_font_display_name(u32 language, const char *personal);
#define NYX_FONT_ORIGINAL "Hekate-Original.nyxf"
#define NYX_FONT_CHINESE "Nova-Chinese-Simplified.nyxf"
/* Standalone validator, also used by the actual-C host regression checks. */
bool nyx_font_validate_header(const u8 header[32], u32 file_size);
bool nyx_font_filename_valid(const char *filename);
#endif
