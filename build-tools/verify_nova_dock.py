"""Verify the personal Nova TV integration and record matching binaries/sources."""
from pathlib import Path
from zipfile import ZipFile
import hashlib, json, re, subprocess, sys
w = Path(__file__).resolve().parent
p = w / 'hekate-nova-pro-v20-dock'
arm = Path('C:/devkitPro/devkitARM/bin')
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
def sources():
    result = {}
    for f in p.rglob('*'):
        if not f.is_file(): continue
        rel = f.relative_to(p)
        if any(x in ('build','output','.git','__pycache__') for x in rel.parts): continue
        if rel.as_posix() in ('loader/payload_00.h','loader/payload_01.h'): continue
        result[rel.as_posix()] = sha(f)
    return result
for script in ('check_nova_dock_lifecycle.py', 'check_nova_dock_sd_contract.py', 'check_nova_dock_policy.py', 'check_nova_dock_config.py', 'check_nova_dock_passive_probe.py'):
    subprocess.run([sys.executable, str(w / script)], check=True)
subprocess.run([sys.executable, str(w / 'check_tv_protocol.py'), p.name], check=True)
before = sources()
result = subprocess.run([sys.executable, str(w / 'run_build_nova_dock.py'), '-B', '-r'], capture_output=True, text=True)
print('\n'.join(result.stdout.splitlines()[-8:]))
if result.returncode:
    print(result.stderr)
    raise SystemExit(result.returncode)
assert sources() == before, 'Source changed during final build'
out = p / 'output'
for elf, filename in (('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')):
    target = w / ('linked-nova-tv-' + filename)
    subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'), '-S','-O','binary',str(p/elf),str(target)], check=True)
    expected = target.read_bytes() + (b'ICTC49' if filename == 'hekate.bin' else b'')
    assert expected == (out/filename).read_bytes()
assert (out/'hekate_unc.bin').stat().st_size <= 140288
assert (out/'hekate.bin').stat().st_size <= 126296
assert (out/'nyx.bin').stat().st_size <= 16*1024*1024
nm = subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
for symbol in ('gui_tv_open','gui_tv_dirty','gui_tv_shutdown','hw_display_external_end','dpout_video_start','dock_step','pd_dock_attached','gui_tv_auto_configure','gui_tv_resume_detection','gui_tv_screen_configure'):
    assert re.search(r'\bT ' + symbol + r'$', nm, re.M), symbol
objdump = subprocess.check_output([str(arm/'arm-none-eabi-objdump.exe'),'-dr',str(p/'build/nyx/hw_init.o')],text=True)
assert 'R_ARM_CALL\thw_display_external_end' in objdump or re.search(r'R_ARM_CALL\s+hw_display_external_end',objdump)
names = re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(names) == 42 and all(re.search(r'\bT '+name+r'$',nm,re.M) for name in names)
baseline = w/'hekate-nova-pro-v19-tv'
assert (p/'nyx/nyx_gui/diagnostics/ram_handoff.h').read_bytes() == (baseline/'nyx/nyx_gui/diagnostics/ram_handoff.h').read_bytes()
assert (p/'bdk/display/tv/dpout.c').read_bytes() == (w/'hekate-tv-6.5.5/bdk/display/tv/dpout.c').read_bytes()
changed = [rel for rel in before if (baseline/rel).exists() and sha(baseline/rel) != before[rel]]
expected_changes = {'TV-SOURCES.md','nyx/nyx_gui/config.c','nyx/nyx_gui/config.h',
    'nyx/nyx_gui/nyx.c','nyx/nyx_gui/nova_version.h','nyx/nyx_gui/localization_catalog.inc',
    'nyx/nyx_gui/localization_catalog.json','nyx/nyx_gui/frontend/gui.c',
    'nyx/nyx_gui/frontend/gui_options.c','nyx/nyx_gui/frontend/gui_tools.c',
    'nyx/nyx_gui/frontend/gui_tv.c','nyx/nyx_gui/frontend/gui_tv.h',
    'bdk/display/tv/usbpd.c','bdk/display/tv/usbpd.h'}
assert set(changed) == expected_changes, changed
ui = json.loads((w/'native-ui-nova-dock.json').read_text())
assert ui['native_scenes'] == 616 and not ui['failures']
life = json.loads((w/'nova-dock-lifecycle-check.json').read_text())
policy = json.loads((w/'nova-dock-policy-check.json').read_text())
protocol = json.loads((w/'tv-protocol-check.json').read_text())
assert life['source_sha256'] == before['nyx/nyx_gui/frontend/gui_tv.c']
assert protocol['source_sha256'] == before['bdk/display/tv/usbpd.c']
for name, digest in policy['source_sha256'].items(): assert digest == before['bdk/display/tv/'+name]
translations = json.loads((p/'nyx/nyx_gui/localization_dock_v20.json').read_text())
for key, values in translations.items():
    spec = re.findall(r'%(?:0?\d*)[sdX]',key)
    assert len(values) == 7 and all(re.findall(r'%(?:0?\d*)[sdX]',v) == spec for v in values)
info = {'product':'Hekate Nova Pro', 'version':'20-Dock', 'base':'6.5.4 / Nyx 1.9.4',
    'build_exit_code':0, 'hardware_validation_of_this_integration':False,
    'user_reported_reference':'Prototype 01: OLED directly in official dock; stable picture and VOL- stop. Cable-mediated failures were a different setup.',
    'source_sha256':before, 'changed_existing_sources':changed,
    'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)} for f in out.iterdir() if f.is_file()},
    'tests':{'lifecycle':life,'policy':policy,'protocol':protocol,'native_ui':ui,'sd_contract':'real sd_mount tested with I/O substituted; zero means success'},
    'native_probes':42,'dock_config_test_cases':84,'passive_probe_cases':15,'auto_default':False,'console_screen_default':'TV + console','translated_new_tv_phrases':len(translations),
    'video_tuning_identical_to_prototype_01':True}
(w/'verification-nova-dock.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: Nova TV binaries match ELF, cleanup hook linked, 42 probes retained, known source changes only.')
