from pathlib import Path
import json,hashlib,subprocess,sys,re
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v22';base=w/'hekate-nova-pro-v21-bt';arm=Path('C:/devkitPro/devkitARM/bin')
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources():
    return {f.relative_to(p).as_posix():sha(f)for f in p.rglob('*')if f.is_file()
        and not any(x in ('build','output','.git','__pycache__')for x in f.relative_to(p).parts)
        and f.relative_to(p).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
before=sources()
expected={'Versions.inc','bootloader/main.c','bootloader/hos/pkg1.c','bootloader/hos/pkg2_patches.inl',
    'nyx/nyx_gui/link.ld','nyx/nyx_gui/start.S','nyx/nyx_gui/localization_catalog.inc','nyx/nyx_gui/localization_catalog.json',
    'nyx/nyx_gui/nova_version.h','nyx/nyx_gui/hos/pkg1.c','nyx/nyx_gui/bluetooth/nova_bt.h',
    'nyx/nyx_gui/bluetooth/nova_bt_protocol.c','nyx/nyx_gui/bluetooth/nova_bt_stack.c','nyx/nyx_gui/frontend/gui.c',
    'nyx/nyx_gui/frontend/gui_bluetooth.c','nyx/nyx_gui/frontend/gui_info.c','nyx/nyx_gui/frontend/gui_nova.c'}
changed={f for f,h in before.items()if(base/f).exists()and sha(base/f)!=h}
assert changed==expected,changed^expected
audit=w/'upstream-audit-6.5.5';pin='8e1392df4569ac501a115867056b13c361429e9e'
for f in ('Versions.inc','bootloader/hos/pkg1.c','nyx/nyx_gui/hos/pkg1.c'):assert sha(p/f)==sha(audit/'v6.5.5'/f)
assert sha(p/'bootloader/hos/pkg2_patches.inl')==sha(audit/pin/'bootloader/hos/pkg2_patches.inl')
patches=(p/'bootloader/hos/pkg2_patches.inl').read_text().split('_fs_nogc_2350[] = {')[1].split('};')[0]
assert '0x1B35CD'in patches and '0x1BE7CD'not in patches
for f in ('bdk/display/tv/dpout.c','bdk/display/tv/usbpd.c','nyx/nyx_gui/diagnostics/ram_handoff.h'):assert sha(p/f)==sha(base/f)
reports={n:json.loads((w/n).read_text(encoding='utf-8'))for n in ('nova22-bt-check.json','nova22-radio-check.json','nova22-input-check.json',
    'nova22-navigation-check.json','nova22-tv-lifecycle-check.json','native-ui-nova22.json','nova22-visual-check.json')}
assert reports['native-ui-nova22.json']['native_scenes']==462 and not reports['native-ui-nova22.json']['failures']
assert reports['nova22-navigation-check.json']['passed_scenes']==28
assert reports['nova22-visual-check.json']['identical_views']==49
for f,h in reports['nova22-bt-check.json']['source_sha256'].items():assert sha(p/'nyx/nyx_gui/bluetooth'/f)==h
assert sha(p/'nyx/nyx_gui/bluetooth/nova_bt_radio.c')==reports['nova22-radio-check.json']['source_sha256']
assert sha(p/'nyx/nyx_gui/frontend/gui_bluetooth.c')==reports['nova22-input-check.json']['source_sha256']
for f,h in reports['nova22-navigation-check.json']['source_sha256'].items():assert sha(p/'nyx/nyx_gui'/f)==h
for f,h in reports['native-ui-nova22.json']['source_sha256'].items():assert before[f]==h
print('Sources and upstream fixes verified. Final full ARM build.',flush=True)
r=subprocess.run([sys.executable,str(w/'run_build_nova22.py'),'-B','-r'],capture_output=True,text=True)
print('\n'.join(r.stdout.splitlines()[-8:]),flush=True)
if r.returncode:print(r.stdout+r.stderr);raise SystemExit(r.returncode)
assert before==sources(),'Source mutation during final build'
subprocess.run([sys.executable,str(w/'check_nova22_header.py')],check=True)
reports['nova22-header-check.json']=json.loads((w/'nova22-header-check.json').read_text())
out=p/'output'
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
    tmp=w/('linked-nova22-'+name);subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
    assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
assert (out/'hekate_unc.bin').stat().st_size<=140288
assert (out/'hekate.bin').stat().st_size<=126296
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
for symbol in ('gui_bluetooth_merge','nova_bt_decode','gui_tv_auto_configure','hw_display_external_end','hci_init','hid_host_connect'):
    assert re.search(r'\bT '+symbol+r'$',nm,re.M)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+f+r'$',nm,re.M)for f in probes)
info={'product':'Hekate Nova Pro','version':'22','base':'Hekate6.5.5 / Nyx1.9.5','build_exit_code':0,
    'upstream_fix':pin,'source_sha256':before,'changed_existing_sources':sorted(changed),
    'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},'tests':reports,
    'native_probes':42,'physical_validation':False,'bluetooth':'experimental; decoded input routing tested on host; RF test needed',
    'display_style':'Nova preserved; 49 unaffected views compared pixel for pixel excluding version footer',
    'license_note':'BTstack retains separate personal/noncommercial license'}
(w/'verification-nova22.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: Nova22 ARM build, binary headers, official fixes, source hashes and test evidence match.')
