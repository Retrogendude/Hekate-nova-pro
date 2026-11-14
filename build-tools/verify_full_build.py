from pathlib import Path
import json
import re
import subprocess
import sys

work = Path(__file__).resolve().parent
project = work / 'hekate-6.5.4'
arm = Path('C:/devkitPro/devkitARM/bin')
out = project / 'output'
logs = []
scripts = ['check_nyx_config_writer.py', 'check_payload_reader.py',
           'test_native_backend_host.py', 'test_native_verdict_host.py',
           'check_ram_handoff.py', 'check_ram_payload.py']
for script in scripts:
    result = subprocess.run([sys.executable, str(work / script)], capture_output=True, text=True)
    logs.append(script + '\n' + result.stdout + result.stderr)
    if result.returncode:
        print(logs[-1])
        raise SystemExit(result.returncode)

for name, elf in [('loader', project/'build/loader/loader.elf'), ('nyx', project/'build/nyx/nyx.elf')]:
    raw = work / ('final-verify-' + name + '.bin')
    subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'), '-S', '-O', 'binary', str(elf), str(raw)], check=True)
    actual = (out/('hekate.bin' if name == 'loader' else 'nyx.bin')).read_bytes()
    assert actual == raw.read_bytes() + (b'ICTC49' if name == 'loader' else b'')
assert (out/'hekate.bin').stat().st_size <= 126296
assert (out/'hekate_unc.bin').stat().st_size <= 140288
ram = work/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin'
assert ram.stat().st_size <= 130392
assert (project/'nyx/nyx_gui/diagnostics/ram_handoff.h').read_bytes() == (work/'HWTest-RCM-0.15/ramtest/source/nyx_ram_handoff.h').read_bytes()
pages = (project/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text()
probes = re.findall(r'\{\s*(probe_\w+)\s*,', pages)
original = re.findall(r'\{\s*(probe_\w+)\s*,', (work/'HWTest-RCM-0.15/core.c').read_text())
assert probes == original and len(probes) == 42
symbols = subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'), str(project/'build/nyx/nyx.elf')], text=True)
assert all(re.search(r'\bT '+probe+r'$', symbols, re.M) for probe in probes)
stub_symbol = re.search(r'^(\w+) [DR] cpu_stub_bin$', symbols, re.M)
assert stub_symbol
stub = (project/'build/nyx/cpu_stub.bin').read_bytes()
assert len(stub) == 8128
stub_offset = int(stub_symbol.group(1), 16) - 0x81000000
assert (out/'nyx.bin').read_bytes()[stub_offset:stub_offset+len(stub)] == stub

summary = {
    'native_probes_compiled_and_linked': probes,
    'native_probe_entries': len(probes),
    'wifi_a64_stub_bytes': 8128,
    'hekate_payload_bytes': (out/'hekate.bin').stat().st_size,
    'hekate_uncompressed_bytes': (out/'hekate_unc.bin').stat().st_size,
    'nyx_bytes': (out/'nyx.bin').stat().st_size,
    'ram_service_bytes': ram.stat().st_size,
    'hardware_validation': 'not performed',
    'tests': {'config_writer_cases': 69, 'payload_reader_assertions': 108,
              'report_export_cases': 10, 'verdict_cases': 6, 'ram_contract_checks': 28,
              'ram_decoder_and_binary': 'passed'},
}
(work/'verification-full.json').write_text(json.dumps(summary, indent=2), encoding='utf-8')
logs.append('PASS: all 42 source probe entries are linked; required A64 stub present; ELF/bin comparisons and all payload limits pass.')
(work/'verification-full.txt').write_text('\n\n'.join(logs), encoding='utf-8')
print(logs[-1])
print('All actual-C regression checks passed. Hekate/Nyx/RAM:',summary['hekate_payload_bytes'],summary['nyx_bytes'],summary['ram_service_bytes'])
