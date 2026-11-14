#!/usr/bin/env python3
"""Generate Nyx's compact Unicode fallback fonts using Pillow.

The embedded glyph artwork is derived from Noto Sans, licensed under SIL OFL
1.1; see ../source/OFL.txt. This generator is GPL-2.0-or-later.

LVGL 5 uses a fixed line height and no individual baseline/glyph offsets. Keep
Noto's cap-height at the requested pixel size, fitting only the accent and
descender bands into the remaining line space. Each 4-bit row is byte-aligned,
as required by lv_draw_rbasic/lv_draw_vbasic.
"""
from __future__ import annotations

import argparse
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "source" / "NotoSans-Regular.ttf"
EXTRAS = (
    0x2010, 0x2011, 0x2013, 0x2014, 0x2018, 0x2019, 0x201A, 0x201C,
    0x201D, 0x201E, 0x2022, 0x2026, 0x2030, 0x2039, 0x203A, 0x20AC,
    0x2116, 0x2122, 0xFFFD,
)
CODEPOINTS = tuple(sorted(set(range(0xA0, 0x250)) |
                          set(range(0x400, 0x500)) | set(EXTRAS)))


def render_glyph(font: ImageFont.FreeTypeFont, char: str, height: int) -> Image.Image:
    """Rasterize to an LVGL fixed-height cell with no clipped diacritics."""
    left, top, right, bottom = font.getbbox(char, anchor="ls")
    origin_x = -min(0, left)
    width = max(1, math.ceil(font.getlength(char)) + origin_x, right + origin_x)
    cap_height = -font.getbbox("H", anchor="ls")[1]
    baseline = round(height * 0.8)
    accent_height = baseline - cap_height
    assert accent_height > 0
    raw_top = min(top, -cap_height)
    raw_bottom = max(bottom, 0)
    raw = Image.new("L", (width, raw_bottom - raw_top))
    ImageDraw.Draw(raw).text((origin_x, -raw_top), char, font=font, fill=255, anchor="ls")
    result = Image.new("L", (width, height))

    # Keep the body at the native font size, preserving ASCII/fallback alignment.
    body = raw.crop((0, -cap_height - raw_top, width, -raw_top))
    result.paste(body, (0, accent_height))
    if top < -cap_height:
        band = raw.crop((0, top - raw_top, width, -cap_height - raw_top))
        fitted_height = min(accent_height, band.height)
        band = band.resize((width, fitted_height), Image.Resampling.LANCZOS)
        result.paste(band, (0, accent_height - fitted_height))
    if bottom > 0:
        band = raw.crop((0, -raw_top, width, bottom - raw_top))
        fitted_height = min(height - baseline, band.height)
        band = band.resize((width, fitted_height), Image.Resampling.LANCZOS)
        result.paste(band, (0, baseline))
    return result


def pack_4bpp(image: Image.Image) -> bytes:
    """High nibble first, padding each odd-width scanline independently."""
    output = bytearray()
    for y in range(image.height):
        for x in range(0, image.width, 2):
            high = (image.getpixel((x, y)) + 8) // 17
            low = (image.getpixel((x + 1, y)) + 8) // 17 if x + 1 < image.width else 0
            output.append((min(high, 15) << 4) | min(low, 15))
    return bytes(output)


def unpack_4bpp(data: bytes, width: int, height: int) -> Image.Image:
    image = Image.new("L", (width, height))
    stride = (width + 1) // 2
    for y in range(height):
        for x in range(width):
            byte = data[y * stride + x // 2]
            image.putpixel((x, y), ((byte >> 4) if x % 2 == 0 else (byte & 15)) * 17)
    return image


def make_font(height: int):
    font = ImageFont.truetype(str(SOURCE), height)
    missing = bytes(font.getmask(chr(0x10FFFF)))
    glyphs = {}
    for codepoint in CODEPOINTS:
        char = chr(codepoint)
        mask = bytes(font.getmask(char))
        if mask == missing and codepoint != 0xFFFD:
            raise ValueError(f"Source font is missing U+{codepoint:04X}")
        image = render_glyph(font, char, height)
        packed = pack_4bpp(image)
        decoded = unpack_4bpp(packed, image.width, height)
        assert len(packed) == ((image.width + 1) // 2) * height
        assert image.width < 256
        glyphs[codepoint] = (decoded, packed)
    return font, glyphs


def write_font(height: int, glyphs) -> str:
    prefix = f"unicode_{height}"
    bitmap = bytearray()
    descriptions = []
    for codepoint, (image, packed) in glyphs.items():
        descriptions.append(f"\t{{ .w_px = {image.width}, .glyph_index = {len(bitmap)} }}, /* U+{codepoint:04X} */")
        bitmap.extend(packed)
    assert len(bitmap) < (1 << 24)
    lines = [f"static const uint8_t {prefix}_bitmap[] = {{"]
    for offset in range(0, len(bitmap), 24):
        lines.append("\t" + ",".join(f"0x{byte:02X}" for byte in bitmap[offset:offset+24]) + ",")
    lines += ["};", "", f"static const lv_font_glyph_dsc_t {prefix}_glyphs[] = {{",
              *descriptions, "};", "", f"static lv_font_t {prefix} = {{",
              f"\t.unicode_first = 0x{min(glyphs):X},",
              f"\t.unicode_last = 0x{max(glyphs):X},",
              f"\t.glyph_bitmap = {prefix}_bitmap,",
              f"\t.glyph_dsc = {prefix}_glyphs,",
              "\t.unicode_list = unicode_codepoints,",
              "\t.get_bitmap = lv_font_get_bitmap_sparse,",
              "\t.get_width = lv_font_get_width_sparse,",
              "\t.next_page = NULL,", f"\t.h_px = {height},", "\t.bpp = 4,",
              "\t.monospace = 0,", f"\t.glyph_cnt = {len(glyphs)},", "};", ""]
    print(f"{height}px: {len(glyphs)} glyphs, {len(bitmap):,} bitmap bytes")
    return "\n".join(lines)


def make_preview(path: Path, fonts):
    image = Image.new("RGB", (1280, 740), "#15191D")
    draw = ImageDraw.Draw(image)
    heading = ImageFont.truetype(str(SOURCE), 20)
    draw.text((28, 14), "Nyx Unicode — decoded 4-bit glyphs, 20 / 30 px", font=heading, fill="#93E1AE")
    samples = (
        "Français · Écran, réglages, mémoire, démarrer, à bientôt !",
        "Español · Configuración, botón, información, ¿qué tal?",
        "Italiano / Português · più, città, ecrã, definições, não",
        "Deutsch · Zurück, Größe, auswählen, überprüfen",
        "Русский · Настройки, язык, память, проверка, вернуться",
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ",
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюя · № € — … « »",
        "ÀÂÄÇÈÉÊËÎÏÔÖÙÛÜŸ àâäçèéêëîïôöùûüÿ Æœ Œñ ß",
    )
    y = 55
    for height, (font, glyphs) in fonts.items():
        draw.text((28, y), f"{height} px", font=heading, fill="#93E1AE")
        y += 32
        for text in samples:
            x = 28
            for char in text:
                mask = glyphs.get(ord(char), (None,))[0]
                if mask is None:
                    mask = unpack_4bpp(pack_4bpp(render_glyph(font, char, height)),
                                       render_glyph(font, char, height).width, height)
                image.paste("#F2F7F3", (x, y), mask)
                x += mask.width + 1
            y += height + 7
        y += 14
    path.parent.mkdir(parents=True, exist_ok=True)
    image.crop((0, 0, image.width, min(image.height, y + 15))).save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preview", type=Path)
    args = parser.parse_args()
    fonts = {height: make_font(height) for height in (20, 30)}
    sections = [
        "/* Generated by tools/generate_unicode_fonts.py. Do not hand-edit.\n"
        " * Glyph artwork: Copyright 2018 The Noto Project Authors.\n"
        " * Font data: SIL Open Font License 1.1, see source/OFL.txt.\n"
        " * Integration code: SPDX-License-Identifier: GPL-2.0-or-later\n */",
        '#include "unicode_fonts.h"',
        "#include <libs/lvgl/lv_misc/lv_font.h>", "",
        "static const uint32_t unicode_codepoints[] = {",
    ]
    for offset in range(0, len(CODEPOINTS), 12):
        sections.append("\t" + ",".join(f"0x{c:04X}" for c in CODEPOINTS[offset:offset+12]) + ",")
    sections += ["\t0", "};", ""]
    sections += [write_font(height, font[1]) for height, font in fonts.items()]
    sections.append("""static void add_fallback_once(lv_font_t *parent, lv_font_t *fallback)
{
    for (lv_font_t *page = parent; page; page = page->next_page)
        if (page == fallback)
            return;
    lv_font_add(fallback, parent);
}

void nyx_unicode_fonts_init(void)
{
    // The 20px symbol page is shared by Inter UI and Ubuntu Mono. Append once
    // through Inter UI so both chains reach this fallback without creating a loop.
    add_fallback_once(&interui_20, &unicode_20);
    add_fallback_once(&interui_30, &unicode_30);
}
""")
    (ROOT / "unicode_fonts.c").write_text("\n".join(sections), encoding="utf-8", newline="\n")
    if args.preview:
        make_preview(args.preview.resolve(), fonts)
        print(f"Preview: {args.preview.resolve()}")


if __name__ == "__main__":
    main()
