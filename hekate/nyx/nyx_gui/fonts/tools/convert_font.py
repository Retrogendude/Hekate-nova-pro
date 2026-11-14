#!/usr/bin/env python3
"""Convert a licensed TTF/OTF into a bounded Nyx SD font (NYXF v1).

    python convert_font.py MyFont.ttf -o Lecture.nyxf

Requires Python 3 and Pillow. Fonts without Cyrillic use the bundled Noto
Sans as a fallback for missing glyphs. Copy the result into /fonts on the SD.
"""
from __future__ import annotations
import argparse
import importlib.util
import math
import re
from pathlib import Path
import struct
from PIL import Image, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("unicode_generator", ROOT / "tools/generate_unicode_fonts.py")
unicode_generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(unicode_generator)
CODEPOINTS = tuple(range(32, 127)) + unicode_generator.CODEPOINTS
HEIGHTS = tuple(range(14, 38, 2))

def fnv(data: bytes) -> int:
    value = 2166136261
    for byte in data:
        value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
    return value

def glyph(font, fallback, codepoint, height, style):
    char = chr(codepoint)
    missing = bytes(font.getmask(chr(0x10FFFF)))
    actual = font if bytes(font.getmask(char)) != missing or codepoint == 0xFFFD else fallback
    image = unicode_generator.render_glyph(actual, char, height)
    if style == "strong":
        # Keep the original baseline/cell; a one-pixel stroke improves contrast.
        grown = Image.new("L", (image.width + 2, height))
        grown.paste(image, (1, 0))
        image = grown.filter(ImageFilter.MaxFilter(3))
    return image

def build(source: Path, output: Path, style="regular"):
    sections = []
    for height in HEIGHTS:
        font = ImageFont.truetype(str(source), height)
        fallback = ImageFont.truetype(str(ROOT / "source/NotoSans-Regular.ttf"), height)
        images = [glyph(font, fallback, cp, height, style) for cp in CODEPOINTS]
        if style == "fixed":
            # A true fixed cell for every glyph, including Cyrillic and accents.
            width = round(height * 0.67)
            fixed_images = []
            for image in images:
                if image.width > width:
                    image = image.resize((width, height), Image.Resampling.LANCZOS)
                fixed = Image.new("L", (width, height))
                fixed.paste(image, ((width - image.width) // 2, 0))
                fixed_images.append(fixed)
            images = fixed_images
        descriptions, bitmap = bytearray(), bytearray()
        for cp, image in zip(CODEPOINTS, images):
            if not 0 < image.width <= 128:
                raise ValueError(f"Glyph U+{cp:04X} has unsupported width {image.width}")
            descriptions += struct.pack("<III", cp, image.width, len(bitmap))
            bitmap += unicode_generator.pack_4bpp(image)
        section = descriptions + bitmap
        assert len(section) <= 2 * 1024 * 1024
        sections.append((height, bytes(descriptions), bytes(bitmap), fnv(section)))
    table, payload = bytearray(), bytearray()
    start = 32 + 32 * len(sections)
    for height, descriptions, bitmap, checksum in sections:
        table += struct.pack("<8I", height, len(CODEPOINTS), start,
                             start + len(descriptions), len(bitmap),
                             start + len(descriptions) + len(bitmap), checksum, 0)
        payload += descriptions + bitmap
        start += len(descriptions) + len(bitmap)
    header = struct.pack("<4sHH5I", b"NYXF", 1, 32, start, len(sections), 32, 0, fnv(table))
    header += struct.pack("<I", fnv(header))
    assert len(header) == 32
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(header + table + payload)
    print(f"{output.name}: {start:,} bytes, {len(CODEPOINTS)} glyphs, {len(HEIGHTS)} sizes")
    return header + table + payload

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("font", type=Path)
    parser.add_argument("-o", "--output", type=Path)
    parser.add_argument("--style", choices=("regular", "strong", "fixed"), default="regular")
    args = parser.parse_args()
    output = args.output or args.font.with_suffix(".nyxf")
    if output.name.startswith(".") or ".." in output.name or len(output.name) >= 64 or not re.fullmatch(r"[A-Za-z0-9_. -]+", output.name):
        parser.error("Choose a short ASCII file name, e.g. Lecture.nyxf")
    if output.suffix.lower() != ".nyxf":
        parser.error("The output extension must be .nyxf")
    build(args.font, output, args.style)

if __name__ == "__main__":
    main()
