"""Compile the driver's real pure functions, with no MMIO or simulated verdicts."""
from pathlib import Path
import hashlib, json, re, subprocess
work = Path(__file__).resolve().parent
tests = work / 'nova-tv-tests'
root = work / 'hekate-nova-pro-v19-tv/bdk/display/tv'
a = (root / 'dpout.c').read_text()
b = (root / 'usbpd.c').read_text()

def function(source, name):
    match = re.search(r'^static [^\n]*\b' + re.escape(name) + r'\([^\n]*\)\n\{', source, re.M)
    assert match, name
    opening = source.index('{', match.start())
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == '{': depth += 1
        elif source[i] == '}':
            depth -= 1
            if depth == 0: return source[match.start():i + 1]
    raise AssertionError('Unclosed function: ' + name)

def typedef(source, name):
    match = re.search(r'typedef struct \{[^}]*\} ' + name + r';', source)
    assert match, name
    return match[0]

code = ('#include <stdint.h>\n#include <stdbool.h>\n#include <stdio.h>\n'
        '#include <assert.h>\n#include <string.h>\n#include <display/tv/usbpd.h>\n'
        'typedef uint64_t u64;\n')
code += '\n'.join(typedef(a, t) for t in ('dpout_mode_t', 'dpout_link_t')) + '\n'
code += '\n'.join(function(a, t) for t in ('_link_calc', '_text')) + '\n'
code += '\n'.join(re.findall(r'^#define DOCK_\w+[^\n]*', b, re.M)) + '\n'
code += '\n'.join(function(b, t) for t in ('_pdo_decode', '_choose', '_op_ma')) + '\n'
code += (tests / 'policy_cases.inc').read_text()
(tests / 'policy.c').write_text(code)
subprocess.run([str(work / 'toolchains/w64devkit/bin/gcc.exe'), str(tests / 'policy.c'),
                '-I' + (tests / 'stubs').as_posix(), '-I' + (root.parent.parent).as_posix(),
                '-o', str(tests / 'policy.exe'), '-Wall', '-Wextra', '-Werror'], check=True)
result = subprocess.run([str(tests / 'policy.exe')], check=True, text=True, capture_output=True)
print(result.stdout.strip())
(work / 'nova-tv-policy-check.json').write_text(json.dumps({
    'result': result.stdout.strip(), 'hardware_validation': False,
    'source_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest()
                      for name in ('dpout.c', 'usbpd.c')},
}, indent=2))
