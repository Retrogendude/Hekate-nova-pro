"""Check compiled probe-catalog coverage and immutable diagnostic values.

These checks detect changed format fields, numeric notation, paths and
regression of reviewed electrical explanations. They do not assess linguistic
quality or simulate the hardware measurements.
"""
from pathlib import Path
import collections
import json
import re
from audit_localization_coverage import FORMATS, literals

ROOT = Path(__file__).resolve().parent / 'hekate-6.5.4/nyx/nyx_gui'
catalog = json.loads((ROOT / 'localization_catalog.json').read_text(encoding='utf-8'))
extra = json.loads((ROOT / 'localization_probe_extra.json').read_text(encoding='utf-8'))
review = json.loads((ROOT / 'localization_probe_review.json').read_text(encoding='utf-8'))
numeric = re.compile(r'0x(?=%)|(?<=%08X)\.\.(?=%08X)|(?<=%d)\.(?=%02d)|(?<![A-Za-z])(?:[0-9]+(?:\.[0-9]+)?)(?![A-Za-z])')
paths = re.compile(r'(?<![\w.])(?:sd:)?[/\w.-]*\.(?:bin|ini|keys|txt|nyxf|elf|c|h)(?!\w)|(?:contents|bootloader)/')
checks = 0
source_keys = set()
for path in (ROOT / 'diagnostics/vendor').glob('*.c'):
    for source, line in literals(path):
        if re.search(r'[A-Za-z]{3}', source):
            assert source in catalog, (path.name, line, source)
            source_keys.add(source)
            checks += 1
for source in extra:
    values = catalog[source]
    for value in values:
        assert FORMATS.findall(value) == FORMATS.findall(source), source
        assert collections.Counter(numeric.findall(value)) == collections.Counter(numeric.findall(source)), (source, value)
        assert not any(symbol in value for symbol in '√■□♪'), (source, value)
        checks += 3
for key, values in catalog.items():
    for value in values:
        assert collections.Counter(paths.findall(value)) == collections.Counter(paths.findall(values[0])), (key, value)
        checks += 1
for key, values in review.items():
    assert catalog[key] == values, key
    checks += 1
assert all('>5 ms' in value for value in catalog['Result : ADMAIF FIFO stayed full for >5 ms'])
assert 'convertisseur de niveau' in catalog['Verdict : line stuck LOW - shifter unpowered or shorted to GND'][1]
print(f'PASS: {checks} probe translation checks; {len(source_keys)} source literals covered, seven languages, exact numeric notation and paths, {len(review)} reviewed overrides. Linguistic and hardware review remain separate.')
