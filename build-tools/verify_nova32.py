"""Bind v32's actual source, tests, ARM executable and limited source changes."""
from pathlib import Path
import subprocess,json,hashlib,re,difflib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32';base=w/'hekate-nova-pro-v31'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources(root):
 return {f.relative_to(root).as_posix():sha(f)for f in root.rglob('*')if f.is_file()
  and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(root).parts)
  and f.relative_to(root).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
cur,prior=sources(p),sources(base);changed={n for n in cur if n in prior and cur[n]!=prior[n]}
expected={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_partition_nova.inc','frontend/gui_bluetooth.c','bluetooth/nova_bt.h','bluetooth/nova_bt_radio.c','bluetooth/nova_bt_stack.c','nova_version.h','localization_catalog.inc','localization_catalog.json']}
assert changed==expected,changed^expected
assert set(prior)<=set(cur)
old=json.loads((base/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'));new=json.loads((p/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(new[k]==v for k,v in old.items())and len(new)==len(old)+11
names=['nova32-'+n+'-check.json'for n in ['pairing','reader','pairing-file','pairing-sd','bt','negotiation','report','input','radio','header','navigation','autostart','wireless']]+['native-ui-nova32.json','nova30-core-io-check.json','nova30-reflection-check.json','nova32-rf-check.json','nova32-display-check.json','nova31-raster-check.json','nova32-partition-check.json','nova32-recovery-check.json','nova32-recovery-hci-check.json']
tests={n:json.loads((w/n).read_text(encoding='utf-8'))for n in names}
for name,t in tests.items():
 if name not in ['native-ui-nova32.json','nova32-navigation-check.json']:assert t.get('passed',False),name
def root_hashes(name):
 for n,h in tests[name]['source_sha256'].items():assert cur[n]==h,(name,n)
for name in ['nova32-pairing-check.json','nova32-reader-check.json','nova32-header-check.json','native-ui-nova32.json','nova32-autostart-check.json','nova32-display-check.json','nova31-raster-check.json','nova32-partition-check.json']:root_hashes(name)
for name in ['nova32-bt-check.json','nova32-pairing-file-check.json','nova32-wireless-check.json','nova32-rf-check.json','nova32-recovery-check.json','nova32-recovery-hci-check.json']:
 for n,h in tests[name]['source_sha256'].items():assert cur['nyx/nyx_gui/bluetooth/'+n]==h,(name,n)
for name in ['nova32-negotiation-check.json','nova32-navigation-check.json']:
 for n,h in tests[name]['source_sha256'].items():assert cur['nyx/nyx_gui/'+n]==h,(name,n)
for name in ['nova32-pairing-sd-check.json','nova32-report-check.json','nova32-input-check.json']:
 assert tests[name]['source_sha256']==cur['nyx/nyx_gui/frontend/gui_bluetooth.c'],name
assert tests['nova32-radio-check.json']['source_sha256']==cur['nyx/nyx_gui/bluetooth/nova_bt_radio.c']
bt=tests['nova32-bt-check.json'];assert bt['old_v27_display_wait_rejected']and bt['fast_display_budget']and bt['old_v26_burst_failure']
neg=tests['nova32-negotiation-check.json'];assert len(neg['logs'])==18 and neg['radio_health_through_hci'] and neg['late_wake_v28_regression']and neg['hwtest_rom_regression']
assert neg['hid_host_sha256']==cur['nyx/nyx_gui/libs/btstack/classic/hid_host.c']
assert tests['nova32-reader-check.json']['old_v28_standard_profile_rejected']
assert tests['nova32-pairing-file-check.json']['corruption_cases']==576
assert tests['nova32-pairing-sd-check.json']['rollback_and_backup']
assert tests['nova32-report-check.json']['cases']==11
assert tests['nova32-navigation-check.json']['passed_scenes']==28
assert tests['native-ui-nova32.json']['native_scenes']==882 and not tests['native-ui-nova32.json']['failures']
core=tests['nova30-core-io-check.json'];assert core['source_sha256']==cur['nyx/nyx_gui/nyx.c']
assert len(core['results']['v30']['runs'])==20 and all(r['exit']==0 for r in core['results']['v30']['runs'])
assert len(core['results']['v29']['runs'])==8 and all(r['exit']!=0 for r in core['results']['v29']['runs'])
ref=tests['nova30-reflection-check.json'];assert ref['production_source_sha256']==cur['nyx/nyx_gui/frontend/gui_nova_reflection.c']
assert len(ref['results'])==18
for r in ref['results']:
 if r['variant']=='fixed':
  assert r['overflow']==r['full_frames']==0 and r['peak']<=31
  before=next(b for b in ref['results'] if b['variant']=='before' and b['scene']==r['scene'] and b['refresh']==r['refresh'])
  assert before['pixels']==r['pixels'] and before['overflow']>0
assert tests['nova32-partition-check.json']['native_scenes']==720 and not tests['nova32-partition-check.json']['failures']
assert len(tests['nova31-raster-check.json']['pixel_exact_frames'])==6
assert 'watch_disconnect[1]==1' in tests['nova32-recovery-hci-check.json']['v31_regression']
assert tests['nova32-radio-check.json']['v31_regression']
assert len(tests['nova32-wireless-check.json']['evidence'])==4
assert tests['nova32-autostart-check.json']['boot_without_window'] and tests['nova32-autostart-check.json']['once_only']
out=p/'output'
for n in ('hekate.bin','hekate_unc.bin','libsys_minerva.bso','libsys_lp0.bso'):assert sha(out/n)==sha(base/'output'/n),n
assert (out/'hekate_unc.bin').stat().st_size<=140288 and (out/'hekate.bin').stat().st_size<=126296
assert (out/'nyx.bin').stat().st_size<16*1024*1024
assert (out/'nyx.bin').read_bytes()[0x98:0x9f]==b'GCTC195'
assert b'Hekate Nova Pro 32 Bluetooth' in (out/'nyx.bin').read_bytes()
assert (out/'hekate.bin').read_bytes()[0x118:0x11f]==b'ICTC655'
arm=Path('C:/devkitPro/devkitARM/bin')
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
 tmp=w/('linked-nova32-'+name);subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
for name in ['jc_get_bt_pairing_info_ex','jc_get_bt_pairing_info','nova_bt_pairing_encode','nova_bt_pairing_decode','gui_bluetooth_autostart','jc_get_attached_mask','nova_bt_take_buttons','nova_bt_draw_poll','gui_bluetooth_draw_poll','lv_draw_set_service_cb','vic_wait_idle_poll','vic_compose_poll']:assert re.search(r'\bT '+name+r'$',nm,re.M),name
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text());assert len(probes)==42
assert all(re.search(r'\bT '+name+r'$',nm,re.M)for name in probes)
gui=(p/'nyx/nyx_gui/frontend/gui.c').read_text();assert gui.count('gui_bluetooth_poll_fast();')==2 and gui.count('gui_bluetooth_poll();')==2
info={'product':'Hekate Nova Pro','version':'32','base':'Hekate 6.5.5 / Nyx 1.9.5','build_exit_code':0,'source_sha256':cur,'changed_existing_sources':sorted(changed),
 'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},'tests':tests,'native_probes':42,'physical_validation':False,
 'change':'Single-row partition recap, spaced controls and separate right-side integer sizes; per-peer silent HID stream report request/reconnection with backlog guard; HWTest WL_REG_ON-high radio-session policy, exact prior state restored on Stop',
 'limits':'No physical v32 range test or guarantee of 2-3 m. V31 user telemetry shows up to 2.8 s report gaps vs 22 ms host poll gap and negative relative RSSI. Hardware is substituted in tests. The unchanged nyx.c, reflection.c and rasterizer retain v30/v31 evidence checked against identical hashes; current UI has fresh native geometry tests.',
 'unchanged':'Storage action callbacks; pairing/autostart and SD key format; BR/sniff/QoS profile, RF power limits, UART/EDR and ROM firmware; vendor BTstack; TV and theme assets; metadata offsets and loader/libsys binaries remain compatible with v31',
 'private_files':'No user pairing data is in this distribution. Auto load is read-only and uses the existing validated private file. A rail present at boot prevents SD and radio autostart',
 'hwtest_reference':json.loads((w/'nova27-reference/audit.json').read_text())}
(w/'verification-nova32.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
patch=''.join(''.join(difflib.unified_diff((base/n).read_text(encoding='utf-8').splitlines(True),(p/n).read_text(encoding='utf-8').splitlines(True),fromfile='v31/'+n,tofile='v32/'+n))for n in sorted(changed))
(w/'nova32-layout-recovery.patch').write_text(patch,encoding='utf-8')
print('PASS: v32 ARM binaries, fixed headers, nine source changes, all current test evidence and unchanged loader/libsys verified; physical fixes not claimed.')
