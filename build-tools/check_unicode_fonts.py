#!/usr/bin/env python3
"""Compile and run Nyx Unicode font checks, then validate catalog coverage.

Run: python work/check_unicode_fonts.py
Optional: --source PATH --cc PATH. A native GCC/Clang compiler is needed.
The check uses the real lv_font.c and unicode_fonts.c implementation. Only
the existing ASCII/symbol pages are stubbed: their bitmap data live on the SD
card, while this test exercises the newly embedded fallback and shared chains.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import unicodedata

WORK = Path(__file__).resolve().parent

HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include <libs/lvgl/lv_misc/lv_font.h>
#include "unicode_fonts.h"

lv_font_t interui_20, interui_30, hekate_symbol_20, hekate_symbol_30, ubuntu_mono;
void lv_font_builtin_init(void) {}
static const uint8_t *empty_bitmap(const lv_font_t *font, uint32_t codepoint)
{ (void)font; (void)codepoint; return NULL; }
static int16_t empty_width(const lv_font_t *font, uint32_t codepoint)
{ (void)font; (void)codepoint; return -1; }
static const uint32_t catalog_codepoints[] = { @CATALOG@ };

static int check_chain(lv_font_t *font)
{
    int count = 0;
    lv_font_t *base = font;
    while (font) {
        assert(++count < 6); /* Detect cycles, including the shared symbol page. */
        if (font->unicode_list) {
            assert(font->glyph_cnt == @COUNT@);
            assert(font->bpp == 4);
            for (int i = 0; i < font->glyph_cnt; ++i) {
                assert(font->unicode_list[i]);
                assert(font->glyph_dsc[i].w_px > 0);
                if (i + 1 < font->glyph_cnt) {
                    assert(font->unicode_list[i] < font->unicode_list[i+1]);
                    assert(font->glyph_dsc[i+1].glyph_index - font->glyph_dsc[i].glyph_index ==
                        ((font->glyph_dsc[i].w_px + 1) / 2) * font->h_px);
                }
                assert(lv_font_get_bitmap(font, font->unicode_list[i]) ==
                    font->glyph_bitmap + font->glyph_dsc[i].glyph_index);
            }
            assert(font->unicode_list[font->glyph_cnt] == 0);
        }
        font = font->next_page;
    }
    for (unsigned i = 0; i < sizeof(catalog_codepoints) / sizeof(catalog_codepoints[0]); ++i) {
        assert(lv_font_get_width(base, catalog_codepoints[i]) > 0);
        assert(lv_font_get_bitmap(base, catalog_codepoints[i]) != NULL);
        assert(lv_font_get_bpp(base, catalog_codepoints[i]) == 4);
    }
    return count;
}

int main(void)
{
    lv_font_t *stubs[] = { &interui_20, &interui_30, &ubuntu_mono,
                          &hekate_symbol_20, &hekate_symbol_30 };
    for (unsigned i = 0; i < sizeof(stubs) / sizeof(stubs[0]); ++i) {
        stubs[i]->get_width = empty_width;
        stubs[i]->get_bitmap = empty_bitmap;
    }
    interui_20.next_page = &hekate_symbol_20;
    ubuntu_mono.next_page = &hekate_symbol_20;
    interui_30.next_page = &hekate_symbol_30;
    for (int i = 0; i < 10; ++i) nyx_unicode_fonts_init();
    assert(check_chain(&interui_20) == 3);
    assert(check_chain(&ubuntu_mono) == 3);
    assert(check_chain(&interui_30) == 3);
    assert(hekate_symbol_20.next_page->h_px == 20);
    assert(hekate_symbol_30.next_page->h_px == 30);
    puts("PASS: native lookup, 4bpp offsets, catalog characters and repeat/shared-chain initialization");
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=WORK / "hekate-6.5.4")
    parser.add_argument("--cc", type=Path)
    args = parser.parse_args()
    source = args.source.resolve()
    fonts = source / "nyx/nyx_gui/fonts"
    font_source = (fonts / "unicode_fonts.c").read_text(encoding="utf-8")
    codepoint_text = font_source.split("unicode_codepoints[] = {", 1)[1].split("};", 1)[0]
    font_codepoints = {int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]+)", codepoint_text)}
    catalog = json.loads((source / "nyx/nyx_gui/localization_catalog.json").read_text(encoding="utf-8"))
    assert all(len(row) == 7 for row in catalog.values()), "Each catalog entry must have seven languages"
    catalog_characters = {ord(char) for text in [*catalog, *(text for row in catalog.values() for text in row)]
                          for char in text if ord(char) > 127}
    # Private-use Hekate icons are provided by the preceding symbol-font page.
    icon_characters = {value for value in catalog_characters if 0xF800 <= value <= 0xF831}
    fallback_characters = catalog_characters - icon_characters
    missing = fallback_characters - font_codepoints
    assert not missing, "Missing catalog glyphs: " + ", ".join(
        f"U+{value:04X} {unicodedata.name(chr(value), '?')}" for value in sorted(missing))

    for height in (20, 30):
        bitmap = font_source.split(f"unicode_{height}_bitmap[] = {{", 1)[1].split("};", 1)[0]
        bitmap_size = len(re.findall(r"0x[0-9A-Fa-f]{2}", bitmap))
        glyph_table = font_source.split(f"unicode_{height}_glyphs[] = {{", 1)[1].split("};", 1)[0]
        glyphs = [(int(w), int(index)) for w, index in
                  re.findall(r"\.w_px = (\d+), \.glyph_index = (\d+)", glyph_table)]
        assert len(glyphs) == len(font_codepoints)
        assert glyphs[-1][1] + ((glyphs[-1][0] + 1) // 2) * height == bitmap_size
        print(f"{height}px: {len(glyphs)} glyphs; {bitmap_size:,} bitmap bytes")

    compiler = args.cc
    if not compiler:
        bundled = WORK / "toolchains/w64devkit/bin/gcc.exe"
        compiler = bundled if bundled.is_file() else shutil.which("gcc") or shutil.which("clang")
    if not compiler:
        raise SystemExit("A native GCC/Clang compiler is required; pass --cc PATH")
    harness = HARNESS.replace("@COUNT@", str(len(font_codepoints))).replace(
        "@CATALOG@", ",".join(f"0x{value:X}" for value in sorted(fallback_characters)))
    # A stable work directory also preserves the exact harness for inspection.
    directory = WORK / "unicode-font-check-build"
    directory.mkdir(parents=True, exist_ok=True)
    test_source = directory / "check.c"
    executable = directory / "check.exe"
    test_source.write_text(harness, encoding="utf-8")
    subprocess.run([str(compiler), "-std=gnu11", "-Wall", "-Wextra",
                    "-I" + str(source / "bdk"), "-I" + str(fonts),
                    str(test_source), str(fonts / "unicode_fonts.c"),
                    str(source / "bdk/libs/lvgl/lv_misc/lv_font.c"),
                    "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
    print(f"PASS: all {len(catalog_characters)} non-ASCII catalog characters covered "
          f"({len(catalog)} entries, seven languages; {len(icon_characters)} icons use symbol fonts)")


if __name__ == "__main__":
    main()
