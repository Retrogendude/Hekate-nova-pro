"""Freeze final v21 sources and verify matching ARM binaries and evidence."""
from pathlib import Path
import subprocess,hashlib,json,re,sys
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v21-bt';baseline=w/'hekate-nova-pro-v20-dock'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources():
    return {f.relative_to(p).as_posix():sha(f) for f in p.rglob('*') if f.is_file()
        and not any(v in ('build','output','.git','__pycache__') for v in f.relative_to(p).parts)
        and f.relative_to(p).as_posix() not in ('loader/payload_00.h','loader/payload_01.h')}
before=sources()
reports={n:json.loads((w/n).read_text()) for n in ('nova-bt-check.json','nova-bt-radio-check.json','nova-bt-input-check.json','native-ui-nova-bt.json','nova-bt-tv-lifecycle-check.json')}
assert reports['native-ui-nova-bt.json']['native_scenes']==462 and not reports['native-ui-nova-bt.json']['failures']
for name in ('nova-bt-check.json','nova-bt-radio-check.json','nova-bt-input-check.json'):assert reports[name]['passed']
for name,h in reports['nova-bt-check.json']['source_sha256'].items():assert sha(p/'nyx/nyx_gui/bluetooth'/name)==h
assert sha(p/'nyx/nyx_gui/bluetooth/nova_bt_radio.c')==reports['nova-bt-radio-check.json']['source_sha256']
assert sha(p/'nyx/nyx_gui/frontend/gui_bluetooth.c')==reports['nova-bt-input-check.json']['source_sha256']
assert sha(p/'nyx/nyx_gui/frontend/gui_tv.c')==reports['nova-bt-tv-lifecycle-check.json']['source_sha256']
for f,h in reports['native-ui-nova-bt.json']['source_sha256'].items():assert before[f]==h
for f in (p/'nyx/nyx_gui/libs/btstack').rglob('*'):
    if f.suffix not in ('.c','.h'):continue
    rel=f.relative_to(p/'nyx/nyx_gui/libs/btstack');assert sha(f)==sha(w/'btstack-upstream/src'/rel),rel
expected={'ETUDE-JOYCONS-SANS-FIL.md','nyx/Makefile','nyx/nyx_gui/nova_version.h',
    'nyx/nyx_gui/localization_catalog.inc','nyx/nyx_gui/localization_catalog.json',
    'nyx/nyx_gui/frontend/gui.c','nyx/nyx_gui/frontend/gui_tools.c','nyx/nyx_gui/frontend/gui_options.c',
    'nyx/nyx_gui/frontend/gui_tv.c','nyx/nyx_gui/frontend/gui_nova.c','nyx/nyx_gui/frontend/nova_assets/nova_icons.inc'}
changed={f for f,h in before.items() if (baseline/f).exists() and sha(baseline/f)!=h}
assert changed==expected,changed^expected
for f in ('bdk/display/tv/dpout.c','bdk/display/tv/usbpd.c','nyx/nyx_gui/diagnostics/ram_handoff.h'):
    assert sha(p/f)==sha(baseline/f)
print('Final sources frozen; compiling all ARM targets.',flush=True)
r=subprocess.run([sys.executable,str(w/'run_build_nova_bt.py'),'-B','-r'],capture_output=True,text=True)
print('\n'.join(r.stdout.splitlines()[-8:]),flush=True)
if r.returncode:print(r.stdout+r.stderr);raise SystemExit(r.returncode)
assert before==sources(),'Sources changed during the build'
arm=Path('C:/devkitPro/devkitARM/bin');out=p/'output'
for elf,filename in (('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')):
    target=w/('linked-nova-bt-'+filename)
    subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(target)],check=True)
    assert target.read_bytes()+(b'ICTC49' if filename=='hekate.bin' else b'')==(out/filename).read_bytes()
assert (out/'hekate_unc.bin').stat().st_size<=140288
assert (out/'hekate.bin').stat().st_size<=126296
assert (out/'nyx.bin').stat().st_size<=16*1024*1024
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
for symbol in ('gui_tv_open','hw_display_external_end','gui_tv_auto_configure','nova_bt_start','nova_bt_stop','nova_bt_poll','hci_init','hid_host_connect','gui_bluetooth_open','gui_bluetooth_merge','gui_bluetooth_shutdown'):
    assert re.search(r'\bT '+symbol+r'$',nm,re.M),symbol
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+name+r'$',nm,re.M) for name in probes)
translations=json.loads((p/'nyx/nyx_gui/localization_bt_v21.json').read_text())
assert len(translations)==27 and all(len(v)==7 for v in translations.values())
info={'product':'Hekate Nova Pro','version':'21-BT','base':'6.5.4 / Nyx 1.9.4 / Nova20',
    'build_exit_code':0,'physical_bluetooth_validation':False,'physical_tv_validation_of_this_integration':False,
    'bluetooth_default':'off; manual start only; ROM startup experimental; existing Joy-Con pairing imported into RAM',
    'btstack_revision':(w/'btstack-revision.txt').read_text().strip(),'license_note':'personal prototype; BTstack has separate noncommercial terms; full notice included',
    'source_sha256':before,'changed_existing_sources':sorted(changed),
    'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)} for f in out.iterdir() if f.is_file()},
    'tests':reports,'native_probes':42,'tv_protocol_regressions':18,
    'video_and_pd_drivers_unchanged_from_v20':True}
(w/'verification-nova-bt.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: final ARM binaries match ELF; Bluetooth and cleanup linked; 42 probes retained; upstream and source hashes verified.')
