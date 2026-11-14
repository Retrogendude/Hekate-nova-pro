"""Build seven bounded UI packs staged outside the RAM test's writable regions."""
from pathlib import Path
import argparse
import hashlib
import json
import struct
from PIL import Image, ImageDraw, ImageFont
from audit_localization_coverage import literals, compose, technical

WORK = Path(__file__).resolve().parent
PROJECT = WORK / 'hekate-6.5.4'
SOURCE = WORK / 'HWTest-RCM-0.15/ramtest/source/ramtest_main.c'
CODES = ['en', 'fr', 'es', 'it', 'ru', 'de', 'pt']
MAGIC = 0x4955524E

def fnv(data):
    value = 2166136261
    for byte in data: value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
    return value

def glyph(font, codepoint):
    # A fixed 8x8 cell, doubled by the service to its existing 16px grid.
    char = chr(codepoint)
    left, top, right, bottom = font.getbbox(char, anchor='ls')
    width = max(1, right - left)
    canvas = Image.new('L', (width, 16))
    ImageDraw.Draw(canvas).text((-left, 12), char, font=font, anchor='ls', fill=255)
    target_width = min(8, width)
    canvas = canvas.resize((target_width, 16), Image.Resampling.LANCZOS)
    cell = Image.new('L', (8, 8)); x_offset = (8 - target_width) // 2
    # Six cap-height rows plus separate accent/descender rows. Using a
    # larger source face fills the 8px cell instead of spacing tiny glyphs.
    cell.paste(canvas.crop((0, 3, target_width, 12)).resize((target_width, 6), Image.Resampling.LANCZOS), (x_offset, 1))
    for x in range(target_width):
        cell.putpixel((x + x_offset, 0), max(canvas.getpixel((x, y)) for y in range(3)))
        cell.putpixel((x + x_offset, 7), max(canvas.getpixel((x, y)) for y in range(12, 16)))
    rows = [0] * 8
    for y in range(8):
        for x in range(8):
            if cell.getpixel((x, y)) >= 80: rows[y] |= 1 << x
    return bytes(rows)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--allow-missing', action='store_true')
    args = parser.parse_args()
    catalog = json.loads((PROJECT / 'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
    keys = sorted({text for text, line in literals(SOURCE) if text and '\0' not in text})
    assert 0 < len(keys) <= 512
    translations, missing, preserved = {}, [], []
    for key in keys:
        values = compose(key, catalog)
        if values is None:
            if technical(key): values = [key] * 7; preserved.append(key)
            else: missing.append(key); values = [key] * 7
        translations[key] = values
    directory = WORK / 'ram-ui'
    directory.mkdir(exist_ok=True)
    (directory / 'missing.json').write_text(json.dumps(missing, indent=2, ensure_ascii=False), encoding='utf-8')
    if missing and not args.allow_missing:
        raise SystemExit(f'{len(missing)} RAM UI texts still need translations; see work/ram-ui/missing.json')
    font = ImageFont.truetype(str(PROJECT / 'nyx/nyx_gui/fonts/source/NotoSans-Regular.ttf'), 12)
    metadata = {'base': '0x80000000', 'limit_bytes': 0x300000, 'entry_count': len(keys),
                'missing_translations': missing, 'preserved_identifiers': preserved, 'files': {}}
    for language, code in enumerate(CODES):
        characters = sorted(set(range(32, 127)) | {ord(c) for k in keys for s in (k, translations[k][language]) for c in s if ord(c) >= 32})
        assert len(characters) <= 1024
        font_data = b''.join(struct.pack('<I', cp) + glyph(font, cp) for cp in characters)
        entries_offset, font_offset = 32, 32 + len(keys) * 8
        strings_offset = font_offset + len(font_data)
        strings, entries, offsets = bytearray(), bytearray(), {}
        def string(value):
            encoded = value.encode('utf-8')
            assert len(encoded) <= 2048
            if value not in offsets:
                offsets[value] = strings_offset + len(strings)
                strings.extend(encoded + b'\0')
            return offsets[value]
        for key in keys: entries.extend(struct.pack('<II', string(key), string(translations[key][language])))
        data = bytes(entries) + font_data + bytes(strings)
        size = 32 + len(data)
        assert size <= 0x300000
        header = struct.pack('<8I', MAGIC, size, fnv(data), len(keys), entries_offset, strings_offset, len(characters), font_offset)
        blob = header + data
        path = directory / f'ram-ui-{code}.bin'
        path.write_bytes(blob)
        # Decode every entry again: the firmware consumes these absolute offsets.
        for index, key in enumerate(keys):
            source, target = struct.unpack_from('<II', blob, entries_offset + index * 8)
            assert strings_offset <= source < size and strings_offset <= target < size
            assert blob[source:blob.index(0, source)].decode('utf-8') == key
            assert blob[target:blob.index(0, target)].decode('utf-8') == translations[key][language]
        metadata['files'][path.name] = {'bytes': size, 'glyphs': len(characters), 'sha256': hashlib.sha256(blob).hexdigest()}
    (directory / 'manifest.json').write_text(json.dumps(metadata, indent=2, ensure_ascii=False), encoding='utf-8')
    print(f'RAM UI packs: {len(keys)} entries × 7 languages; {len(missing)} missing translations.')
    for name, meta in metadata['files'].items(): print(name, meta['bytes'], 'bytes;', meta['glyphs'], 'glyphs')

if __name__ == '__main__': main()
