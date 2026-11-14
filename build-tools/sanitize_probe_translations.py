"""Normalize offline MT typography before applying the reviewed overrides.

Run after polish_probe_translations.py and before build_probe_review.py and
create_localization_catalog.py. No translation engine or network is needed.
"""
from pathlib import Path
import html
import json
import re

ROOT = Path(__file__).resolve().parent / 'hekate-6.5.4/nyx/nyx_gui'
path = ROOT / 'localization_probe_extra.json'
extra = json.loads(path.read_text(encoding='utf-8'))
for source, values in extra.items():
    for language in range(1, 7):
        value = re.sub(r'0x\s+(?=%)', '0x', values[language])
        if '%d.%02d' in source:
            value = re.sub(r'%d\s*%02d', '%d.%02d', value)
        value = html.unescape(re.sub(r'&\s*#\s*(\d+)\s*;', r'&#\1;', value))
        values[language] = value.replace('\xa0', ' ')
path.write_text(json.dumps(extra, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print('Normalized report typography; reviewed translations take precedence in the catalog builder.')
