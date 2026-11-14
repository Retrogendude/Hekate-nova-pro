/* SD font loading and bounded bitmap scaling. SPDX-License-Identifier: GPL-2.0-or-later */
#include "font_manager.h"
#include "../config.h"
#include "../localization.h"
#include <bdk.h>
#include <string.h>
#include <stdlib.h>

#define FONT_LIMIT 16
#define SECTION_LIMIT 24
#define MAX_FILE_SIZE (16u * 1024u * 1024u)
#define MAX_SECTION_SIZE (2u * 1024u * 1024u)
#define MAX_GLYPHS 1024u

typedef struct {
    lv_font_t font;
    lv_font_glyph_dsc_t *glyphs;
    uint32_t *codepoints;
    u8 *bitmap;
} managed_font;
static managed_font regular, small, mono, regular_icons, small_icons;
static const lv_font_t *regular_font = &interui_30, *small_font = &interui_20, *mono_font = &ubuntu_mono;
static char font_files[FONT_LIMIT + 1][64];
static char font_options[(FONT_LIMIT + 1) * 65];
static u32 font_count, selected_size = 30;
static const char *status = "Default font";

static u16 read16(const u8 *p) { return p[0] | (u16)p[1] << 8; }
static u32 read32(const u8 *p) { return p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24; }
static u32 hash_bytes(const u8 *p, u32 length)
{
    u32 hash = 2166136261u;
    while (length--) { hash ^= *p++; hash *= 16777619u; }
    return hash;
}
bool nyx_font_filename_valid(const char *filename)
{
    if (!filename || !filename[0]) return false;
    u32 length = 0;
    while (length < 64 && filename[length]) ++length;
    if (length < 6 || length >= 64) return false;
    static const char extension[] = ".nyxf";
    for (u32 i = 0; i < 5; ++i) if ((filename[length - 5 + i] | 0x20) != extension[i]) return false;
    if (filename[0] == '.' || strstr(filename, "..")) return false;
    for (u32 i = 0; i < length; ++i) {
        u8 c = filename[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == ' '))
            return false;
    }
    return true;
}
bool nyx_font_validate_header(const u8 header[32], u32 file_size)
{
    return header && file_size >= 64 && file_size <= MAX_FILE_SIZE &&
        !memcmp(header, "NYXF", 4) && read16(header + 4) == 1 && read16(header + 6) == 32 &&
        read32(header + 8) == file_size && read32(header + 12) > 0 && read32(header + 12) <= SECTION_LIMIT &&
        read32(header + 16) == 32 && read32(header + 20) == 0 &&
        32 + 32 * read32(header + 12) <= file_size && read32(header + 28) == hash_bytes(header, 28);
}
static bool read_exact(FIL *file, u32 offset, void *buffer, u32 length)
{
    UINT read = 0;
    return f_lseek(file, offset) == FR_OK && f_read(file, buffer, length, &read) == FR_OK && read == length;
}
static bool open_font(const char *filename, FIL *file, u8 header[32])
{
    if (!nyx_font_filename_valid(filename)) return false;
    char path[72]; strcpy(path, "fonts/"); strcat(path, filename);
    if (f_open(file, path, FA_READ) != FR_OK) return false;
    if (f_size(file) > MAX_FILE_SIZE || !read_exact(file, 0, header, 32) || !nyx_font_validate_header(header, f_size(file))) {
        f_close(file); return false;
    }
    return true;
}
bool nyx_font_available(const char *filename)
{
    u32 size = n_cfg.font_size >= 20 && n_cfg.font_size <= 36 && !(n_cfg.font_size & 1) ? n_cfg.font_size : 30;
    return nyx_font_available_at_size(filename, size);
}

static int glyph_index(const lv_font_t *font, u32 codepoint)
{
    if (codepoint < font->unicode_first || codepoint > font->unicode_last) return -1;
    /* Every accepted text face includes the complete leading ASCII block.
     * Confirm its index for compatibility with icon/custom faces, then avoid
     * a binary search for each of the repeated width/bitmap/bpp queries. */
    if (codepoint >= 32 && codepoint <= 126) {
        u32 index = codepoint - 32;
        if (index < font->glyph_cnt && font->unicode_list[index] == codepoint) return index;
    }
    u32 low = 0, high = font->glyph_cnt;
    while (low < high) {
        u32 mid = low + (high - low) / 2;
        u32 current = font->unicode_list[mid];
        if (current == codepoint) return mid;
        if (current < codepoint) low = mid + 1; else high = mid;
    }
    return -1;
}
static const u8 *get_bitmap(const lv_font_t *font, uint32_t cp)
{
    int index = glyph_index(font, cp);
    return index < 0 ? NULL : font->glyph_bitmap + font->glyph_dsc[index].glyph_index;
}
static int16_t get_width(const lv_font_t *font, uint32_t cp)
{
    int index = glyph_index(font, cp);
    return index < 0 ? -1 : font->glyph_dsc[index].w_px;
}
static void release_font(managed_font *font)
{
    free(font->glyphs); free(font->codepoints); free(font->bitmap); memset(font, 0, sizeof(*font));
}
static bool allocate_font(managed_font *font, u32 count, u32 bytes, u32 height)
{
    memset(font, 0, sizeof(*font));
    if (!count || count > MAX_GLYPHS || !bytes || bytes > MAX_SECTION_SIZE || height < 8 || height > 48) return false;
    font->glyphs = calloc(count, sizeof(*font->glyphs));
    font->codepoints = calloc(count + 1, sizeof(*font->codepoints));
    font->bitmap = malloc(bytes);
    if (!font->glyphs || !font->codepoints || !font->bitmap) { release_font(font); return false; }
    font->font.glyph_bitmap = font->bitmap; font->font.glyph_dsc = font->glyphs;
    font->font.unicode_list = font->codepoints; font->font.glyph_cnt = count;
    font->font.h_px = height; font->font.bpp = 4;
    font->font.get_bitmap = get_bitmap; font->font.get_width = get_width;
    return true;
}
static bool load_face(FIL *file, const u8 *entry, managed_font *font)
{
    u32 height = read32(entry), count = read32(entry + 4), start = read32(entry + 8);
    u32 bitmap_start = read32(entry + 12), bytes = read32(entry + 16), end = read32(entry + 20);
    if (!count || count > MAX_GLYPHS || height < 8 || height > 48 || read32(entry + 28) != 0 ||
        count * 12 > MAX_SECTION_SIZE || bitmap_start < start || bitmap_start - start != count * 12 ||
        bytes > MAX_SECTION_SIZE || bytes > end || bitmap_start > end || bytes != end - bitmap_start || end > f_size(file)) return false;
    u32 section_size = end - start;
    if (section_size > MAX_SECTION_SIZE || !allocate_font(font, count, bytes, height)) return false;
    u8 *section = malloc(section_size);
    if (!section) { release_font(font); return false; }
    bool valid = read_exact(file, start, section, section_size) && hash_bytes(section, section_size) == read32(entry + 24);
    u32 previous_cp = 0, previous_end = 0;
    for (u32 i = 0; valid && i < count; ++i) {
        const u8 *glyph = section + i * 12;
        u32 cp = read32(glyph), width = read32(glyph + 4), offset = read32(glyph + 8);
        u32 glyph_bytes = ((width + 1) / 2) * height;
        if (cp < 32 || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF) || cp <= previous_cp ||
            !width || width > 128 || offset != previous_end || offset > bytes || glyph_bytes > bytes - offset) valid = false;
        else {
            font->codepoints[i] = cp; font->glyphs[i].w_px = width; font->glyphs[i].glyph_index = offset;
            previous_cp = cp; previous_end = offset + glyph_bytes;
        }
    }
    if (valid && previous_end != bytes) valid = false;
    if (valid) {
        font->font.unicode_first = font->codepoints[0]; font->font.unicode_last = previous_cp;
        /* Missing ASCII or Cyrillic must never render a language unreadable. */
        for (u32 cp = 32; cp <= 126 && valid; ++cp) valid = glyph_index(&font->font, cp) >= 0;
        for (u32 cp = 0x400; cp <= 0x45F && valid; ++cp) valid = glyph_index(&font->font, cp) >= 0;
        if (valid) memcpy(font->bitmap, section + count * 12, bytes);
    }
    free(section);
    if (!valid) release_font(font);
    return valid;
}
static bool load_selected_into(const char *filename, u32 normal_height, u32 small_height,
    managed_font *normal_font, managed_font *hint_font)
{
    FIL file; u8 header[32], table[SECTION_LIMIT * 32];
    if (!open_font(filename, &file, header)) return false;
    u32 count = read32(header + 12), previous_end = 32 + count * 32;
    bool valid = read_exact(&file, 32, table, count * 32) && hash_bytes(table, count * 32) == read32(header + 24);
    const u8 *normal = NULL, *hint = NULL;
    u32 seen_heights = 0;
    for (u32 i = 0; valid && i < count; ++i) {
        const u8 *entry = table + i * 32;
        u32 height = read32(entry), start = read32(entry + 8), end = read32(entry + 20);
        if (height < 8 || height > 39 || (seen_heights & (1u << (height - 8))) || start != previous_end || end < start || end > f_size(&file)) valid = false;
        else {
            seen_heights |= 1u << (height - 8); previous_end = end;
            if (height == normal_height) normal = entry;
            if (height == small_height) hint = entry;
        }
    }
    valid = valid && previous_end == f_size(&file) && normal && hint;
    if (valid) valid = load_face(&file, normal, normal_font) && load_face(&file, hint, hint_font);
    if (f_close(&file) != FR_OK) valid = false;
    if (!valid) { release_font(normal_font); release_font(hint_font); }
    return valid;
}
static bool load_selected(const char *filename, u32 normal_height, u32 small_height)
{
    return load_selected_into(filename, normal_height, small_height, &regular, &small);
}
bool nyx_font_available_at_size(const char *filename, u32 size)
{
    if (size < 20 || size > 36 || size & 1) return false;
    if (!filename || !filename[0]) return true;
    if (!nyx_font_filename_valid(filename)) return false;
    bool was_mounted = sd_get_card_mounted();
    if (sd_mount()) return false;
    u32 small_size = ((size * 2 / 3) + 1) & ~1u;
    if (small_size < 14) small_size = 14;
    managed_font temporary_normal = {0}, temporary_small = {0};
    bool valid = load_selected_into(filename, size, small_size, &temporary_normal, &temporary_small);
    release_font(&temporary_normal); release_font(&temporary_small);
    if (!was_mounted) sd_unmount();
    return valid;
}

/* Scale existing, trusted font pixels for the built-in family and icon pages.
 * No TrueType parser or executable font code is run on the console. */
static u32 make_codepoints(u32 *list, bool icons)
{
    u32 count = 0;
    if (icons) { for (u32 cp = LV_SYMBOL_GLYPH_FIRST; cp <= LV_SYMBOL_GLYPH_LAST; ++cp) list[count++] = cp; }
    else {
        for (u32 cp = 32; cp <= 126; ++cp) list[count++] = cp;
        for (u32 cp = 0xA0; cp < 0x250; ++cp) list[count++] = cp;
        for (u32 cp = 0x400; cp < 0x500; ++cp) list[count++] = cp;
        static const u32 extra[] = {0x2010,0x2011,0x2013,0x2014,0x2018,0x2019,0x201A,0x201C,0x201D,0x201E,0x2022,0x2026,0x2030,0x2039,0x203A,0x20AC,0x2116,0x2122,0xFFFD};
        for (u32 i = 0; i < ARRAY_SIZE(extra); ++i) list[count++] = extra[i];
    }
    return count;
}
static bool scale_font(const lv_font_t *source, u32 height, managed_font *target, bool icons, bool fixed)
{
    u32 codepoints[MAX_GLYPHS], count = make_codepoints(codepoints, icons), total = 0;
    u32 widths[MAX_GLYPHS], source_height = source->h_px;
    u32 fixed_width = (10u * height + source_height / 2) / source_height;
    for (u32 i = 0; i < count; ++i) {
        u32 source_width = lv_font_get_width(source, codepoints[i]);
        widths[i] = fixed ? fixed_width : (source_width * height + source_height / 2) / source_height;
        if (!widths[i]) widths[i] = height / 2;
        if (widths[i] > 128) return false;
        total += ((widths[i] + 1) / 2) * height;
    }
    if (!allocate_font(target, count, total, height)) return false;
    memset(target->bitmap, 0, total);
    u32 offset = 0;
    for (u32 i = 0; i < count; ++i) {
        u32 cp = codepoints[i], width = widths[i];
        const u8 *source_bitmap = lv_font_get_bitmap(source, cp);
        u32 source_width = lv_font_get_width(source, cp), source_bpp = lv_font_get_bpp(source, cp);
        u32 source_stride = source_bpp == 8 ? source_width : (source_width + 1) / 2;
        target->codepoints[i] = cp; target->glyphs[i].w_px = width; target->glyphs[i].glyph_index = offset;
        if (source_bitmap && source_width && (source_bpp == 4 || source_bpp == 8)) {
            for (u32 y = 0; y < height; ++y) {
                u32 source_y = y * source_height / height;
                for (u32 x = 0; x < width; ++x) {
                    u32 source_x = x * source_width / width;
                    u8 byte = source_bitmap[source_y * source_stride + (source_bpp == 8 ? source_x : source_x / 2)];
                    u8 pixel = source_bpp == 8 ? (byte + 8) / 17 : (source_x & 1 ? byte & 15 : byte >> 4);
                    target->bitmap[offset + y * ((width + 1) / 2) + x / 2] |= pixel << (x & 1 ? 0 : 4);
                }
            }
        }
        offset += ((width + 1) / 2) * height;
    }
    target->font.unicode_first = target->codepoints[0]; target->font.unicode_last = target->codepoints[count - 1];
    if (fixed) target->font.monospace = fixed_width;
    return true;
}
static void scan_fonts(void)
{
    font_count = 1; font_files[0][0] = 0;
    strcpy(font_options, nyx_tr("Default"));
    DIR directory; FILINFO entry;
    if (f_opendir(&directory, "fonts") != FR_OK) return;
    while (font_count <= FONT_LIMIT && f_readdir(&directory, &entry) == FR_OK && entry.fname[0]) {
        if ((entry.fattrib & (AM_DIR | AM_HID)) || !nyx_font_filename_valid(entry.fname) || !nyx_font_available(entry.fname)) continue;
        strcpy(font_files[font_count++], entry.fname);
    }
    f_closedir(&directory);
    for (u32 i = 1; i < font_count; ++i)
        for (u32 j = i + 1; j < font_count; ++j)
            if (strcmp(font_files[i], font_files[j]) > 0) {
                char temp[64]; strcpy(temp, font_files[i]); strcpy(font_files[i], font_files[j]); strcpy(font_files[j], temp);
            }
    for (u32 i = 1; i < font_count; ++i) { strcat(font_options, "\n"); strcat(font_options, font_files[i]); }
}
void nyx_font_manager_init(void)
{
    /* Only call while building a fresh screen; existing objects retain pointers. */
    release_font(&regular); release_font(&small); release_font(&mono); release_font(&regular_icons); release_font(&small_icons);
    regular_font = &interui_30; small_font = &interui_20; mono_font = &ubuntu_mono;
    selected_size = n_cfg.font_size >= 20 && n_cfg.font_size <= 36 && !(n_cfg.font_size & 1) ? n_cfg.font_size : 30;
    u32 small_size = ((selected_size * 2 / 3) + 1) & ~1u;
    if (small_size < 14) small_size = 14;
    bool was_mounted = sd_get_card_mounted();
    bool mounted = !sd_mount();
    if (mounted) scan_fonts();
    else { font_count = 1; font_files[0][0] = 0; strcpy(font_options, nyx_tr("Default")); }
    bool custom = mounted && n_cfg.font_file[0] && load_selected(n_cfg.font_file, selected_size, small_size);
    if (mounted && !was_mounted) sd_unmount();
    if (n_cfg.font_file[0]) status = custom ? "SD font loaded" : "Font unavailable or invalid. Default font restored.";
    else status = "Default font";
    if (custom) { regular_font = &regular.font; small_font = &small.font; }
    else if (selected_size != 30) {
        if (scale_font(&interui_30, selected_size, &regular, false, false) && scale_font(&interui_20, small_size, &small, false, false)) {
            regular_font = &regular.font; small_font = &small.font;
        } else { release_font(&regular); release_font(&small); selected_size = 30; small_size = 20; status = "Not enough memory. Default font restored."; }
    }
    bool icons_ok = true;
    if (regular_font == &regular.font) {
        icons_ok = scale_font(&hekate_symbol_30, selected_size, &regular_icons, true, false);
        if (icons_ok) regular.font.next_page = &regular_icons.font;
    }
    if (icons_ok && small_font == &small.font) {
        icons_ok = scale_font(&hekate_symbol_20, small_size, &small_icons, true, false);
        if (icons_ok) small.font.next_page = &small_icons.font;
    }
    if (!icons_ok) {
        release_font(&regular); release_font(&small); release_font(&regular_icons); release_font(&small_icons);
        regular_font = &interui_30; small_font = &interui_20; selected_size = 30; small_size = 20;
        status = "Not enough memory. Default font restored.";
    }
    if (small_size != 20 && scale_font(&ubuntu_mono, small_size, &mono, false, true)) mono_font = &mono.font;
}
const lv_font_t *nyx_font_regular(void) { return regular_font; }
const lv_font_t *nyx_font_small(void) { return small_font; }
const lv_font_t *nyx_font_mono(void) { return mono_font; }
u32 nyx_font_size(void) { return selected_size; }
const char *nyx_font_status(void) { return status; }
const char *nyx_font_options(void) { return font_options; }
const char *nyx_font_filename(u32 index) { return index < font_count ? font_files[index] : ""; }
u32 nyx_font_count(void) { return font_count; }
u32 nyx_font_selected_index(const char *filename)
{
    if (filename) for (u32 i = 0; i < font_count; ++i) if (!strcmp(font_files[i], filename)) return i;
    return 0;
}
