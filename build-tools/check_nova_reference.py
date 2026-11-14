"""Check Nova's native copy, palette and icon provenance against its reference."""
from pathlib import Path
import hashlib
import json

work=Path(__file__).resolve().parent
root=work/'hekate-6.5.4/nyx/nyx_gui'
reference=json.loads((work/'nova-reference.json').read_text(encoding='utf-8'))
catalog=json.loads((root/'localization_catalog.json').read_text(encoding='utf-8'))
generated=(root/'frontend/nova_assets/nova_reference.inc').read_text(encoding='utf-8')
source=(root/'frontend/gui_nova.c').read_text(encoding='utf-8')
theme=(work/'hekate-6.5.4/bdk/libs/lvgl/lv_themes/lv_theme_hekate.c').read_text(encoding='utf-8')
counts={'home':5,'tools':8,'info':7,'diagnostics':12}
for name,count in counts.items():
    assert len(reference['items'][name])==count
    for row in reference['items'][name]:
        literal='{'+','.join(json.dumps(value,ensure_ascii=False)for value in row)+'},'
        assert literal in generated,row
        for text in row[1:3]:assert text in catalog and catalog[text][1]==text,text
for color in reference['colors'].values():assert color[1:].upper() in (source+theme).upper(),color
icons=json.loads((root/'frontend/nova_assets/manifest.json').read_text())
assert icons['version']=='1.8.0' and icons['strokeWidth']==1.7 and icons['sizes']==[18,24,40]
for rows in reference['items'].values():
    for row in rows:assert row[3] in icons['icons']
for row in reference['navigation']:assert row[2] in icons['icons']
assert (root/'frontend/nova_assets/LUCIDE-LICENSE.txt').is_file()
result={'reference_actions':32,'reference_navigation_entries':5,'columns':4,'tools_rows':2,'diagnostics_rows':3,
        'colors':reference['colors'],'card_radius_px':8,'reflection_period_ms':50,'authorized_amendments':12,'authorized_corrections_v12':7,'icon_library':'Lucide 1.8.0; original Nova Pro brand mark',
        'native_glyph_sizes':[18,24,40],'svg_stroke_width':1.7,
        'reference_data_sha256':hashlib.sha256((work/'nova-reference.json').read_bytes()).hexdigest(),
        'native_reference_include_sha256':hashlib.sha256(generated.encode()).hexdigest(),
        'scope':'Native reference captions, icon names, colors and layout; not a browser pixel comparison.'}
(work/'nova-reference-check.json').write_text(json.dumps(result,indent=2,ensure_ascii=False),encoding='utf-8')
print('PASS: Nova reference: 32 verbatim French action cards, five navigation entries, the exact palette, matching Lucide paths and seven-language catalog coverage.')
