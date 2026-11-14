"""Bind v29's actual source, tests, ARM executable and limited source changes."""
from pathlib import Path
import subprocess,json,hashlib,re,difflib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v29';base=w/'hekate-nova-pro-v28'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources(root):
 return {f.relative_to(root).as_posix():sha(f)for f in root.rglob('*')if f.is_file()
  and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(root).parts)
  and f.relative_to(root).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
cur,prior=sources(p),sources(base);changed={n for n in cur if n in prior and cur[n]!=prior[n]}
expected={'bdk/input/joycon.c','bdk/input/joycon.h'}|{'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_bluetooth.c','bluetooth/nova_bt.h','bluetooth/nova_bt_protocol.c','bluetooth/nova_bt_stack.c','nova_version.h','localization_catalog.inc','localization_catalog.json']}
assert changed==expected,changed^expected
assert set(prior)<=set(cur)
old=json.loads((base/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'));new=json.loads((p/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(new[k]==v for k,v in old.items())and len(new)==len(old)+9
names=['nova29-'+n+'-check.json'for n in ['pairing','reader','pairing-file','pairing-sd','bt','negotiation','report','input','radio','header','navigation']]+['native-ui-nova29.json']
tests={n:json.loads((w/n).read_text(encoding='utf-8'))for n in names}
for name,t in tests.items():
 if name not in ['native-ui-nova29.json','nova29-navigation-check.json']:assert t.get('passed',False),name
def root_hashes(name):
 for n,h in tests[name]['source_sha256'].items():assert cur[n]==h,(name,n)
for name in ['nova29-pairing-check.json','nova29-reader-check.json','nova29-header-check.json','native-ui-nova29.json']:root_hashes(name)
for name in ['nova29-bt-check.json','nova29-pairing-file-check.json']:
 for n,h in tests[name]['source_sha256'].items():assert cur['nyx/nyx_gui/bluetooth/'+n]==h,(name,n)
for name in ['nova29-negotiation-check.json','nova29-navigation-check.json']:
 for n,h in tests[name]['source_sha256'].items():assert cur['nyx/nyx_gui/'+n]==h,(name,n)
for name in ['nova29-pairing-sd-check.json','nova29-report-check.json','nova29-input-check.json']:
 assert tests[name]['source_sha256']==cur['nyx/nyx_gui/frontend/gui_bluetooth.c'],name
assert tests['nova29-radio-check.json']['source_sha256']==cur['nyx/nyx_gui/bluetooth/nova_bt_radio.c']
bt=tests['nova29-bt-check.json'];assert bt['old_v27_display_wait_rejected']and bt['fast_display_budget']and bt['old_v26_burst_failure']
neg=tests['nova29-negotiation-check.json'];assert len(neg['logs'])==17 and neg['late_wake_v28_regression']and neg['hwtest_rom_regression']
assert neg['hid_host_sha256']==cur['nyx/nyx_gui/libs/btstack/classic/hid_host.c']
assert tests['nova29-reader-check.json']['old_v28_standard_profile_rejected']
assert tests['nova29-pairing-file-check.json']['corruption_cases']==576
assert tests['nova29-pairing-sd-check.json']['rollback_and_backup']
assert tests['nova29-report-check.json']['cases']==11
assert tests['nova29-navigation-check.json']['passed_scenes']==28
assert tests['native-ui-nova29.json']['native_scenes']==840 and not tests['native-ui-nova29.json']['failures']
out=p/'output'
for n in ('hekate.bin','hekate_unc.bin','libsys_minerva.bso','libsys_lp0.bso'):assert sha(out/n)==sha(base/'output'/n),n
assert (out/'nyx.bin').read_bytes()[0x98:0x9f]==b'GCTC195'
assert b'Hekate Nova Pro 29 Bluetooth' in (out/'nyx.bin').read_bytes()
assert (out/'hekate.bin').read_bytes()[0x118:0x11f]==b'ICTC655'
arm=Path('C:/devkitPro/devkitARM/bin')
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
 tmp=w/('linked-nova29-'+name);subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
for name in ['jc_get_bt_pairing_info_ex','jc_get_bt_pairing_info','nova_bt_pairing_encode','nova_bt_pairing_decode']:assert re.search(r'\bT '+name+r'$',nm,re.M),name
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text());assert len(probes)==42
assert all(re.search(r'\bT '+name+r'$',nm,re.M)for name in probes)
gui=(p/'nyx/nyx_gui/frontend/gui.c').read_text();assert gui.count('gui_bluetooth_poll_fast();')==1 and gui.count('gui_bluetooth_poll();')==2
info={'product':'Hekate Nova Pro','version':'29','base':'Hekate 6.5.5 / Nyx 1.9.5','build_exit_code':0,'source_sha256':cur,'changed_existing_sources':sorted(changed),
 'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},'tests':tests,'native_probes':42,'physical_validation':False,
 'change':'Validated standard and Nintendo record imports, SPI length guards, explicit CRC32-protected private SD save/load with backup, shorter serialized pages and persistent reconnect attempts',
 'limits':'V29 physical results pending. V28 two-peer input confirmed on a second OLED; the first OLED right peer dropped with supervision timeout08. Return to 0/2 observed on both OLEDs after a successful test; Nintendo synchronization restores 2/2. Standard-profile classification is a tested software cause, not yet confirmed from physical recognized flags. No claim that underlying RF timeout is fixed',
 'unchanged':'Input mapping, foreground/display RX budgets, radio profile, TV and vendor BTstack unchanged versus v28. Legacy Nintendo export filter preserved. Loader and libsys binaries identical',
 'private_files':'Only explicit Save pairing writes joycon-pairing.bin/.bak. These files contain plain keys and are excluded from logs and distribution. Test keys are synthetic',
 'hwtest_reference':json.loads((w/'nova27-reference/audit.json').read_text())}
(w/'verification-nova29.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
patch=''.join(''.join(difflib.unified_diff((base/n).read_text(encoding='utf-8').splitlines(True),(p/n).read_text(encoding='utf-8').splitlines(True),fromfile='v28/'+n,tofile='v29/'+n))for n in sorted(changed))
(w/'nova29-pairing-reconnect.patch').write_text(patch,encoding='utf-8')
print('PASS: v29 ARM binaries, fixed headers, ten limited source changes, all current test evidence and unchanged loader/libsys verified; physical fixes not claimed.')
