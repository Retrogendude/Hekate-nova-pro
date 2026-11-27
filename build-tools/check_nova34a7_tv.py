"""Exercise the real TV integration's lifecycle with simulated UI/hardware APIs.

This verifies software cleanup and error paths, never physical HDMI operation.
Only the host pointer alignment cast is widened; function bodies are unchanged.
"""
from pathlib import Path
import subprocess, hashlib, json

work = Path(__file__).resolve().parent
tests = work / 'nova34a7-tv-tests'
project = work / 'hekate-nova-pro-v34-alpha7'
source = project / 'nyx/nyx_gui/frontend/gui_tv.c'
code = '#define NYX_NOVA_VERSION "34-alpha7"\nstatic void gui_bluetooth_shutdown(void){}\n' + source.read_text()
for header in ('gui_tv.h', 'gui_bluetooth.h', 'gui_nova.h', '../localization.h', '../config.h', '../nova_version.h'):
    code = code.replace('#include \"' + header + '\"', '')
assert code.count('(u32)tv.allocation') == 1
code = code.replace('(u32)tv.allocation', '(uintptr_t)tv.allocation')
(tests / 'gui_tv_under_test.c').write_text(code)
command = [str(work / 'toolchains/w64devkit/bin/gcc.exe'),
           str(tests / 'lifecycle.c'), str(project / 'bdk/utils/sprintf.c'),
           '-I' + (tests / 'stubs').as_posix(),
           '-I' + (project / 'bdk').as_posix(), '-o', str(tests / 'lifecycle.exe'),
           '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
           '-Wno-unused-function', '-Wno-pointer-to-int-cast']
subprocess.run(command, check=True)
result = subprocess.run([str(tests / 'lifecycle.exe')], check=True, text=True, capture_output=True)
print(result.stdout.strip())
(work / 'nova34a7-tv-lifecycle-check.json').write_text(json.dumps({
    'result': result.stdout.strip(),
    'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
    'hardware_validation': False,
    'real_bdk_formatter': True,
    'pointer_alignment_cast_widened_for_64_bit_host': True,
}, indent=2))
