/* Nyx UI translation boundary. GPL-2.0. */
#include "localization.h"
#include "config.h"
#include "fonts/unicode_fonts.h"
#include "frontend/gui_readability.h"
#include <stdlib.h>
#include <string.h>

typedef struct { const char *source; const char *text[I18N_LANGUAGE_COUNT]; } locale_entry;
#include "localization_catalog.inc"

const char *nyx_tr(const char *text)
{
    if (!text) return NULL;
    u32 lang = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
    u32 low = 0, high = ARRAY_SIZE(locale_catalog);
    while (low < high) {
        u32 middle = low + (high - low) / 2;
        int comparison = strcmp(text, locale_catalog[middle].source);
        if (!comparison) return locale_catalog[middle].text[lang];
        if (comparison < 0) high = middle; else low = middle + 1;
    }
    return text;
}

static const locale_entry *lookup_span(const char *text, u32 length)
{
    u32 low = 0, high = ARRAY_SIZE(locale_catalog);
    while (low < high) {
        u32 middle = low + (high - low) / 2;
        const char *source = locale_catalog[middle].source;
        int comparison = strncmp(text, source, length);
        if (!comparison && source[length]) comparison = -1;
        if (!comparison) return &locale_catalog[middle];
        if (comparison < 0) high = middle; else low = middle + 1;
    }
    return NULL;
}

void nyx_localization_init(void) { nyx_unicode_fonts_init(); }

/* Translate whole phrases first; otherwise preserve icons, matrix control
 * bytes, whitespace and LVGL color commands around translated fragments.
 * Every returned buffer is owned by the caller. No UI buffer is overwritten. */
typedef struct { char *data; u32 size, length; bool changed, failed; } text_builder;
static void append(text_builder *out, const char *text, u32 count)
{
    if (out->failed) return;
    if (out->length + count + 1 > out->size) {
        u32 capacity = (out->length + count + 1) * 2;
        char *next = malloc(capacity);
        if (!next) { out->failed = true; return; }
        if (out->length) memcpy(next, out->data, out->length);
        free(out->data);
        out->data = next; out->size = capacity;
    }
    memcpy(out->data + out->length, text, count);
    out->length += count; out->data[out->length] = 0;
}

/* Match formatted display text without ever writing a larger translation into
 * the caller's original (often small) printf buffer. Values and paths are
 * captured verbatim, and the catalog retains exactly the same argument order. */
static const char *format_end(const char *format, char *type)
{
    const char *cursor = format + 1;
    while (*cursor && ((*cursor >= '0' && *cursor <= '9') || *cursor == ' ' ||
        *cursor == '.' || *cursor == '-' || *cursor == '+' || *cursor == '#')) ++cursor;
    if (!strchr("dsuoxXpc", *cursor) || !*cursor) return NULL;
    *type = *cursor; return cursor + 1;
}
static void translate_piece(text_builder *out, const char *text, u32 length, u32 depth);
static bool translate_pattern(text_builder *out, const char *text, u32 length, u32 depth)
{
    u32 lang = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
    u32 first[2] = { '%', (u8)text[0] };
    if (first[1] < first[0]) { u32 swap = first[0]; first[0] = first[1]; first[1] = swap; }
    u32 buckets = first[0] == first[1] ? 1 : 2;
    for (u32 bucket = 0; bucket < buckets; ++bucket)
    for (u32 index = locale_pattern_start[first[bucket]]; index < locale_pattern_start[first[bucket] + 1]; ++index) {
        u32 row = locale_pattern_rows[index];
        const char *source = locale_catalog[row].source;
        const char *replacement = locale_catalog[row].text[lang];
        if (!strcmp(source, replacement)) continue;
        const char *starts[12]; u32 sizes[12], count = 0, at = 0;
        const char *format = source;
        bool match = true;
        while (*format && match) {
            if (*format != '%') {
                if (at >= length || text[at++] != *format++) match = false;
                continue;
            }
            if (format[1] == '%') {
                if (at >= length || text[at++] != '%') match = false;
                format += 2; continue;
            }
            char type;
            const char *end = format_end(format, &type);
            if (!end || count == ARRAY_SIZE(starts)) { match = false; break; }
            u32 group = 1;
            u32 start = at;
            if (type == 's') {
                /* Adjacent text fields retain their original concatenation.
                 * Their internal boundary is immaterial to the translated
                 * surrounding phrase, so preserve them as one captured span. */
                while (*end == '%') {
                    char next_type; const char *next = format_end(end, &next_type);
                    if (!next || next_type != 's') break;
                    ++group; end = next;
                }
                if (count + group > ARRAY_SIZE(starts)) { match = false; break; }
                if (!*end) at = length;
                else {
                    if (*end == '%') { match = false; break; }
                    const char *next = strchr(end, '%');
                    u32 literal = next ? (u32)(next - end) : strlen(end);
                    while (at <= length && (literal > length - at || memcmp(text + at, end, literal))) ++at;
                    if (at > length) { match = false; break; }
                }
            } else if (type == 'c') {
                if (at == length) { match = false; break; }
                ++at;
            } else {
                while (at < length && text[at] == ' ') ++at;
                if (at < length && (text[at] == '-' || text[at] == '+')) ++at;
                u32 digits = at;
                while (at < length && ((text[at] >= '0' && text[at] <= '9') ||
                    ((type == 'x' || type == 'X' || type == 'p') &&
                    ((text[at] >= 'A' && text[at] <= 'F') || (text[at] >= 'a' && text[at] <= 'f'))))) ++at;
                if (at == digits) { match = false; break; }
            }
            starts[count] = text + start; sizes[count++] = at - start;
            while (--group) { starts[count] = text + at; sizes[count++] = 0; }
            format = end;
        }
        if (!match || *format || at != length || !count) continue;
        u32 arg = 0; format = replacement;
        while (*format) {
            if (*format != '%') { append(out, format++, 1); continue; }
            if (format[1] == '%') { append(out, "%", 1); format += 2; continue; }
            char type; const char *end = format_end(format, &type);
            if (!end || arg == count) break; // Generator rejects this case.
            if (type == 's' && depth < 8 && !memchr(starts[arg], '/', sizes[arg]) &&
                !memchr(starts[arg], '\\', sizes[arg]) && !memchr(starts[arg], '.', sizes[arg]))
                translate_piece(out, starts[arg], sizes[arg], depth + 1);
            else append(out, starts[arg], sizes[arg]);
            ++arg; format = end;
        }
        out->changed = true; return true;
    }
    return false;
}

static void translate_piece(text_builder *out, const char *text, u32 length, u32 depth)
{
    if (!length || out->failed) return;
    const locale_entry *entry = lookup_span(text, length);
    if (entry) {
        u32 language = n_cfg.language < I18N_LANGUAGE_COUNT ? n_cfg.language : I18N_EN;
        const char *translated = entry->text[language];
        u32 translated_length = strlen(translated);
        append(out, translated, translated_length);
        if (length != translated_length || memcmp(text, translated, length)) out->changed = true;
        return;
    }
    if (depth < 8 && translate_pattern(out, text, length, depth)) return;
    if (depth > 32) { append(out, text, length); return; }
    /* Matrix width/state control byte, independent of UTF-8 text. */
    if (((u8)text[0] & 0xC0) == 0x80) {
        append(out, text, 1); translate_piece(out, text + 1, length - 1, depth + 1); return;
    }
    /* Private-use Hekate symbol U+F800..F831; preserve all three bytes. */
    if (length >= 3 && (u8)text[0] == 0xEF && (u8)text[1] == 0xA0) {
        append(out, text, 3); translate_piece(out, text + 3, length - 3, depth + 1); return;
    }
    u32 left = 0, right = length;
    while (left < right && (text[left] == ' ' || text[left] == '\t')) ++left;
    while (right > left && (text[right - 1] == ' ' || text[right - 1] == '\t')) --right;
    if (left || right != length) {
        append(out, text, left); translate_piece(out, text + left, right - left, depth + 1);
        append(out, text + right, length - right); return;
    }
    /* Lines are siblings, so even long option lists have bounded depth. */
    u32 line_start = 0;
    for (u32 i = 0; i < length; ++i) {
        if (text[i] != '\n') continue;
        translate_piece(out, text + line_start, i - line_start, depth + 1);
        append(out, "\n", 1); line_start = i + 1;
    }
    if (line_start) {
        translate_piece(out, text + line_start, length - line_start, depth + 1); return;
    }
    /* Technical tables often assemble a translated field name and value
     * separately. Only split at a recognized label, preserving arbitrary
     * user text and path-like values when the prefix is unknown. */
    for (u32 i = 0; i + 1 < length; ++i) {
        if (text[i] != ':') continue;
        text_builder prefix = {0};
        translate_piece(&prefix, text, i + 1, depth + 1);
        if (prefix.changed && !prefix.failed) {
            append(out, prefix.data, prefix.length); free(prefix.data);
            out->changed = true;
            translate_piece(out, text + i + 1, length - i - 1, depth + 1); return;
        }
        free(prefix.data);
    }
    /* Split only at presentation boundaries, never inside words or paths. */
    for (u32 i = 0; i < length; ++i) {
        if (i + 2 < length && (u8)text[i] == 0xEF && (u8)text[i + 1] == 0xA0) {
            translate_piece(out, text, i, depth + 1); append(out, text + i, 3);
            translate_piece(out, text + i + 3, length - i - 3, depth + 1); return;
        }
        if (text[i] == '#') {
            u32 color_end = i + 1;
            while (color_end < length && color_end < i + 7 &&
                ((text[color_end] >= '0' && text[color_end] <= '9') ||
                 (text[color_end] >= 'A' && text[color_end] <= 'F') ||
                 (text[color_end] >= 'a' && text[color_end] <= 'f'))) ++color_end;
            if (color_end == i + 7 && color_end < length && text[color_end] == ' ') {
                u32 end = color_end + 1;
                while (end < length && text[end] != '#') ++end;
                if (end < length) {
                    translate_piece(out, text, i, depth + 1);
                    append(out, text + i, color_end + 1 - i);
                    translate_piece(out, text + color_end + 1, end - color_end - 1, depth + 1);
                    append(out, "#", 1);
                    translate_piece(out, text + end + 1, length - end - 1, depth + 1); return;
                }
            }
        }
    }
    append(out, text, length);
}

char *nyx_translate_text(const char *text)
{
    if (!text) return NULL;
    const char *direct = nyx_tr(text);
    if (direct != text) {
        if (!strcmp(direct, text)) return NULL;
        u32 length = strlen(direct) + 1;
        char *copy = malloc(length); if (copy) memcpy(copy, direct, length);
        return copy;
    }
    bool letters = false;
    for (const char *at = text; *at; ++at)
        if ((*at >= 'A' && *at <= 'Z') || (*at >= 'a' && *at <= 'z')) { letters = true; break; }
    if (!letters) return NULL;
    text_builder out = {0};
    translate_piece(&out, text, strlen(text), 0);
    if (!out.changed || out.failed) { free(out.data); return NULL; }
    return out.data;
}
#define translated_copy nyx_translate_text

void nyx_label_set_text(lv_obj_t *label, const char *text)
{
    char *tr = translated_copy(text); lv_label_set_text(label, tr ? tr : text); free(tr);
    nyx_readability_request();
}
void nyx_label_set_static_text(lv_obj_t *label, const char *text)
{
    char *tr = translated_copy(text);
    if (tr) lv_label_set_text(label, tr);
    else lv_label_set_static_text(label, text);
    free(tr);
    nyx_readability_request();
}
void nyx_mbox_set_text(lv_obj_t *mbox, const char *text)
{
    char *tr = translated_copy(text); lv_mbox_set_text(mbox, tr ? tr : text); free(tr);
}
void nyx_ddlist_set_options(lv_obj_t *list, const char *options)
{
    char *tr = translated_copy(options); lv_ddlist_set_options(list, tr ? tr : options); free(tr);
}
void nyx_roller_set_options(lv_obj_t *roller, const char *options)
{
    char *tr = translated_copy(options); lv_roller_set_options(roller, tr ? tr : options); free(tr);
}
void nyx_win_set_title(lv_obj_t *win, const char *title)
{
    char *tr = translated_copy(title); lv_win_set_title(win, tr ? tr : title); free(tr);
    nyx_readability_request();
}
lv_obj_t *nyx_win_add_btn(lv_obj_t *win, const void *image, const char *label, lv_action_t action)
{
    char *tr = translated_copy(label);
    lv_obj_t *button = lv_win_add_btn(win, image, tr ? tr : label, action); free(tr);
    nyx_readability_request(); return button;
}
lv_obj_t *nyx_tabview_add_tab(lv_obj_t *tabs, const char *title)
{
    char *tr = translated_copy(title);
    lv_obj_t *tab = lv_tabview_add_tab(tabs, tr ? tr : title); free(tr);
    nyx_readability_request(); return tab;
}

/* Button matrices retain the map pointers, unlike labels. Keep translated
 * copies on the object lifetime; never mutate a caller's static/dynamic map. */
typedef struct owned_map {
    struct owned_map *next; lv_obj_t *object;
    lv_signal_func_t original_signal; const char **map; u32 count;
} owned_map;
static owned_map *maps;
static void free_map(owned_map *record)
{
    for (u32 i = 0; i < record->count; ++i) free((void *)record->map[i]);
    free(record->map); record->map = NULL; record->count = 0;
}
static lv_res_t map_signal(lv_obj_t *obj, lv_signal_t signal, void *param)
{
    owned_map **link = &maps;
    while (*link && (*link)->object != obj) link = &(*link)->next;
    if (!*link) return LV_RES_OK;
    owned_map *record = *link;
    lv_res_t result = record->original_signal(obj, signal, param);
    if (signal == LV_SIGNAL_CLEANUP) { *link = record->next; free_map(record); free(record); }
    return result;
}
void nyx_btnm_set_map(lv_obj_t *buttons, const char **map)
{
    if (!map) return;
    u32 count = 0;
    while (map[count][0]) ++count;
    const char **copy = calloc(count + 1, sizeof(char *));
    if (!copy) { lv_btnm_set_map(buttons, map); return; }
    bool changed = false;
    for (u32 i = 0; i < count; ++i) {
        char *tr = translated_copy(map[i]);
        if (tr) changed = true;
        else { tr = malloc(strlen(map[i]) + 1); if (tr) strcpy(tr, map[i]); }
        copy[i] = tr;
        if (!tr) {
            for (u32 j = 0; j < i; ++j) free((void *)copy[j]);
            free(copy); lv_btnm_set_map(buttons, map); return;
        }
    }
    copy[count] = "";
    owned_map *record = maps;
    while (record && record->object != buttons) record = record->next;
    if (!changed) {
        for (u32 i = 0; i < count; ++i) free((void *)copy[i]);
        free(copy);
        lv_btnm_set_map(buttons, map);
        if (record) free_map(record);
        return;
    }
    if (!record) {
        record = calloc(1, sizeof(*record));
        if (!record) {
            for (u32 i = 0; i < count; ++i) free((void *)copy[i]);
            free(copy);
            lv_btnm_set_map(buttons, map); return;
        }
        record->object = buttons; record->original_signal = lv_obj_get_signal_func(buttons);
        record->next = maps; maps = record; lv_obj_set_signal_func(buttons, map_signal);
    }
    lv_btnm_set_map(buttons, copy); free_map(record);
    record->map = copy; record->count = count;
}
void nyx_mbox_add_btns(lv_obj_t *mbox, const char **map, lv_btnm_action_t action)
{
    lv_mbox_add_btns(mbox, map, action);
    lv_mbox_ext_t *ext = lv_obj_get_ext_attr(mbox);
    nyx_btnm_set_map(ext->btnm, map);
}
