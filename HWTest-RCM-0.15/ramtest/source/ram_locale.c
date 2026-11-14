/* A read-only UI pack lives in the first 4 MiB already excluded by the test.
 * The service keeps its native English display if validation fails.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "ram_locale.h"
#include <gfx.h>
#include <string.h>
#include <stdint.h>
static const u8 *pack;
static u32 pack_size, strings_start, entries_start, entry_count, fonts_start, font_count;
static u32 word(const u8 *p) { return p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24; }
static const char *string_at(u32 offset)
{
    if (!pack || offset < strings_start || offset >= pack_size) return NULL;
    u32 limit = pack_size - offset;
    if (limit > 2049) limit = 2049;
    return memchr(pack + offset, 0, limit) ? (const char *)pack + offset : NULL;
}
bool ram_locale_bind(const void *data, u32 available)
{
    pack = NULL;
    if (!data || available < 32) return false;
    const u8 *p = data;
    u32 size = word(p + 4), count = word(p + 12), entries = word(p + 16);
    u32 strings = word(p + 20), glyphs = word(p + 24), fonts = word(p + 28);
    if (word(p) != RAM_UI_MAGIC || size < 32 || size > available || size > RAM_UI_LIMIT ||
        !count || count > 512 || !glyphs || glyphs > 1024 || entries < 32 ||
        entries > size || count * 8 > size - entries || fonts < entries + count * 8 ||
        fonts > size || glyphs * 12 > size - fonts || strings < fonts + glyphs * 12 || strings >= size) return false;
    u32 hash = 2166136261u;
    for (u32 i = 32; i < size; ++i) hash = (hash ^ p[i]) * 16777619u;
    if (hash != word(p + 8)) return false;
    pack = p; pack_size = size; strings_start = strings; entries_start = entries;
    entry_count = count; fonts_start = fonts; font_count = glyphs;
    for (u32 i = 0; i < count; ++i) {
        const u8 *entry = p + entries + i * 8;
        if (!string_at(word(entry)) || !string_at(word(entry + 4))) { pack = NULL; return false; }
    }
    u32 previous = 0;
    for (u32 i = 0; i < glyphs; ++i) {
        u32 cp = word(p + fonts + i * 12);
        if (cp < 32 || cp <= previous || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) { pack = NULL; return false; }
        previous = cp;
    }
    return true;
}
void ram_locale_init(bool from_nyx)
{
    pack = NULL;
    if (from_nyx) ram_locale_bind((const void *)RAM_UI_BASE, RAM_UI_LIMIT);
}
const char *ram_tr(const char *source)
{
    if (!pack || !source) return source;
    for (u32 i = 0; i < entry_count; ++i) {
        const u8 *entry = pack + entries_start + i * 8;
        const char *key = string_at(word(entry));
        const char *value = string_at(word(entry + 4));
        if (key && value) {
            u32 limit = pack_size - (u32)((const u8 *)key - pack);
            if (limit > 2049) limit = 2049;
            for (u32 c = 0; c < limit; ++c) {
                if (key[c] != source[c]) break;
                if (!key[c]) return value;
            }
        }
    }
    return source;
}
const u8 *ram_ui_glyph(u32 codepoint)
{
    if (!pack) return NULL;
    u32 low = 0, high = font_count;
    while (low < high) {
        u32 index = low + (high - low) / 2;
        const u8 *glyph = pack + fonts_start + index * 12;
        u32 cp = word(glyph);
        if (cp == codepoint) return glyph + 4;
        if (cp < codepoint) low = index + 1; else high = index;
    }
    return NULL;
}
typedef struct { char *out; u32 size, used; } output_t;
static void emit(output_t *out, char c) { if (out->used + 1 < out->size) out->out[out->used++] = c; }
static u32 text_limit(const char *text)
{
    uintptr_t address = (uintptr_t)text, base = (uintptr_t)pack;
    u32 limit = 2048;
    if (pack && address >= base && address < base + pack_size && base + pack_size - address < limit)
        limit = base + pack_size - address;
    return limit;
}
static void emit_text(output_t *out, const char *text)
{
    if (!text) return;
    u32 limit = text_limit(text);
    while (limit-- && *text) emit(out, *text++);
}
void ram_ui_vsnprintf(char *out, u32 size, const char *format, va_list arguments)
{
    if (!out || !size) return;
    output_t buffer = {out, size, 0};
    /* Snapshot the validated format. Training faults cannot turn a changed
     * terminator in DRAM into an unbounded format-string walk. */
    char snapshot[2049]; const char *translated = ram_tr(format);
    u32 n = 0, limit = text_limit(translated);
    while (translated && n < limit && translated[n]) { snapshot[n] = translated[n]; ++n; }
    snapshot[n] = 0; const char *cursor = snapshot;
    while (*cursor) {
        if (*cursor++ != '%') { emit(&buffer, cursor[-1]); continue; }
        int width = 0; char fill = 0;
        if ((*cursor >= '0' && *cursor <= '9') || *cursor == ' ' || *cursor == '.') {
            char first = *cursor++;
            if (*cursor >= '0' && *cursor <= '9') {
                fill = first;
                do { width = width * 10 + *cursor++ - '0'; } while (*cursor >= '0' && *cursor <= '9' && width < 1000);
            } else { width = first - '0'; fill = ' '; }
        }
        if (width > 1024) width = 1024;
        char type = *cursor;
        if (!type) break;
        ++cursor;
        if (type == '%') { emit(&buffer, '%'); continue; }
        if (type == 'c') { char c = (char)va_arg(arguments, u32); if (c) emit(&buffer, c); continue; }
        if (type == 's') {
            const char *value = ram_tr(va_arg(arguments, const char *));
            if (!value) value = "";
            u32 length = 0, value_limit = text_limit(value);
            while (length < value_limit && value[length]) ++length;
            int padding = width - (int)length;
            if (fill != '.') while (padding-- > 0) emit(&buffer, ' ');
            emit_text(&buffer, value);
            if (fill == '.') while (padding-- > 0) emit(&buffer, ' ');
            continue;
        }
        if (type == 'd' || type == 'u' || type == 'x' || type == 'X' || type == 'p' || type == 'P') {
            u32 value = va_arg(arguments, u32), base = type == 'd' || type == 'u' ? 10 : 16;
            bool negative = type == 'd' && (value & 0x80000000u);
            if (negative) value = 0u - value;
            char digits[34]; u32 count = 0;
            do { digits[count++] = "0123456789ABCDEF"[value % base]; value /= base; } while (value);
            int padding = width - (int)count - negative;
            while (padding-- > 0) emit(&buffer, fill ? fill : ' ');
            if (negative) emit(&buffer, '-');
            while (count) emit(&buffer, digits[--count]);
        } else { emit(&buffer, '%'); emit(&buffer, type); }
    }
    /* Avoid ending a bounded line halfway through a UTF-8 character. */
    if (buffer.used && buffer.used + 1 == size) {
        u32 start = buffer.used - 1;
        while (start && ((u8)out[start] & 0xC0) == 0x80) --start;
        u8 lead = out[start]; u32 bytes = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        if (start + bytes > buffer.used) buffer.used = start;
    }
    out[buffer.used] = 0;
}
void ram_ui_snprintf(char *out, u32 size, const char *format, ...)
{
    va_list arguments; va_start(arguments, format); ram_ui_vsnprintf(out, size, format, arguments); va_end(arguments);
}
void ram_ui_printf(const char *format, ...)
{
    char output[1024]; va_list arguments; va_start(arguments, format);
    ram_ui_vsnprintf(output, sizeof(output), format, arguments); va_end(arguments); gfx_puts(output);
}
