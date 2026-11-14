/* Bounded localized report formatter, with BDK's integer/padding syntax.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "diag_format.h"
#include "../localization.h"
#include <string.h>
#include <stdlib.h>
typedef struct { char *out; u32 capacity, used; bool truncated; } sink;
static void put(sink *buffer, char value)
{
    if (buffer->used + 1 < buffer->capacity) buffer->out[buffer->used++] = value;
    else buffer->truncated = true;
}
static void text(sink *buffer, const char *value)
{
    while (*value) put(buffer, *value++);
}
static void spaces(sink *buffer, int count) { while (count-- > 0) put(buffer, ' '); }
void nyx_diag_vsnprintf(char *out, u32 capacity, const char *format, va_list arguments)
{
    if (!out || !capacity) return;
    sink buffer = {out, capacity, 0, false};
    char *translated = nyx_translate_text(format);
    const char *cursor = translated ? translated : format;
    while (cursor && *cursor) {
        if (*cursor++ != '%') { put(&buffer, cursor[-1]); continue; }
        int width = 0; char fill = 0;
        if ((*cursor >= '0' && *cursor <= '9') || *cursor == ' ' || *cursor == '.') {
            char first = *cursor++;
            if (*cursor >= '0' && *cursor <= '9') {
                fill = first;
                do { width = width * 10 + *cursor++ - '0'; } while (*cursor >= '0' && *cursor <= '9' && width < 10000);
            } else { width = first - '0'; fill = ' '; }
        }
        if (width > 4096) width = 4096;
        char type = *cursor;
        if (!type) break;
        ++cursor;
        if (type == '%') { put(&buffer, '%'); continue; }
        if (type == 'c') { char c = (char)va_arg(arguments, unsigned int); if (c) put(&buffer, c); continue; }
        if (type == 's') {
            const char *original = va_arg(arguments, const char *);
            if (!original) original = "";
            char *localized = nyx_translate_text(original);
            const char *value = localized ? localized : original;
            int padding = width - (int)strlen(value);
            if (fill != '.') spaces(&buffer, padding);
            text(&buffer, value);
            if (fill == '.') spaces(&buffer, padding);
            free(localized); continue;
        }
        if (type == 'd' || type == 'u' || type == 'x' || type == 'X' || type == 'p' || type == 'P') {
            u32 number = va_arg(arguments, u32);
            u32 base = type == 'd' || type == 'u' ? 10 : 16;
            bool negative = type == 'd' && (number & 0x80000000u);
            if (negative) number = 0u - number;
            char digits[34]; u32 used = 0;
            do { digits[used++] = "0123456789ABCDEF"[number % base]; number /= base; } while (number);
            int padding = width - (int)used - negative;
            while (padding-- > 0) put(&buffer, fill ? fill : ' ');
            if (negative) put(&buffer, '-');
            while (used) put(&buffer, digits[--used]);
            continue;
        }
        put(&buffer, '%'); put(&buffer, type);
    }
    free(translated);
    /* Retain only complete UTF-8 characters on a bounded report field. */
    if (buffer.truncated && buffer.used) {
        u32 start = buffer.used - 1;
        while (start && ((u8)out[start] & 0xC0) == 0x80) --start;
        u8 lead = out[start];
        u32 bytes = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
        if (start + bytes > buffer.used) buffer.used = start;
    }
    out[buffer.used] = 0;
}
void nyx_diag_snprintf(char *out, u32 capacity, const char *format, ...)
{
    va_list arguments; va_start(arguments, format);
    nyx_diag_vsnprintf(out, capacity, format, arguments); va_end(arguments);
}
