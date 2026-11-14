"""Bind the v28 test evidence, final ARM binaries and restricted source diff."""
from pathlib import Path
import subprocess,json,hashlib,re,difflib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v28';base=w/'hekate-nova-pro-v27'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources(root):
 return {f.relative_to(root).as_posix():sha(f) for f in root.rglob('*') if f.is_file()
  and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(root).parts)
  and f.relative_to(root).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
current,prior=sources(p),sources(base)
changed={n for n in current if n in prior and current[n]!=prior[n]}
expected={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui.c','frontend/gui_bluetooth.c','frontend/gui_bluetooth.h','nova_version.h','localization_catalog.inc','localization_catalog.json','bluetooth/nova_bt.h','bluetooth/nova_bt_stack.c','bluetooth/nova_bt_protocol.c']}
assert changed==expected,changed^expected
assert set(prior)<=set(current),'Prior source removed'
old_catalog=json.loads((base/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
catalog=json.loads((p/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(catalog[k]==v for k,v in old_catalog.items())
assert len(catalog)==len(old_catalog)+5
names=['nova28-pairing-check.json','nova28-bt-check.json','nova28-negotiation-check.json','nova28-report-check.json','nova28-input-check.json','nova28-header-check.json','native-ui-nova28.json','nova28-navigation-check.json']
tests={n:json.loads((w/n).read_text(encoding='utf-8')) for n in names}
assert tests[names[0]]['old_double_reversal_rejected']
assert tests[names[1]]['fast_display_budget'] and tests[names[1]]['old_v27_display_wait_rejected']
assert tests[names[1]]['passed'] and tests[names[1]]['timed_uart_restart'] and tests[names[1]]['old_v26_burst_failure'] and tests[names[1]]['metadata_multi_command']
neg=tests[names[2]];assert neg['passed']and len(neg['logs'])==16 and len(neg['old_policy_failures'])==2
for n,h in neg['source_sha256'].items():assert current['nyx/nyx_gui/'+n]==h,n
assert neg['hwtest_rom_regression']
assert neg['v25_hid_regression']['actual_l2cap_refusal']==4
assert neg['hid_host_sha256']==current['nyx/nyx_gui/libs/btstack/classic/hid_host.c']
report=tests[names[3]];assert report['passed']and report['cases']==11
assert report['source_sha256']==current['nyx/nyx_gui/frontend/gui_bluetooth.c']
assert tests[names[4]]['source_sha256']==current['nyx/nyx_gui/frontend/gui_bluetooth.c']
assert tests[names[4]]['passed']and tests[names[5]]['nyx_metadata_offset']=='0x98'
ui=tests[names[6]];assert ui['native_scenes']==630 and not ui['failures']
for n,h in ui['source_sha256'].items():assert current[n]==h,n
nav=tests[names[7]];assert nav['passed_scenes']==28
for n,h in nav['source_sha256'].items():assert current['nyx/nyx_gui/'+n]==h,n
for n,h in tests[names[0]]['source_sha256'].items():assert current[n]==h,n
radio=json.loads((w/'nova28-radio-check.json').read_text());assert radio['passed']
assert radio['source_sha256']==current['nyx/nyx_gui/bluetooth/nova_bt_radio.c']
tests['nova28-radio-check.json']=radio
for n,h in tests[names[1]]['source_sha256'].items():assert current['nyx/nyx_gui/bluetooth/'+n]==h,n
audit=json.loads((w/'nova27-reference/audit.json').read_text())
assert all(audit['probes_equal_after_normalizing_line_endings'].values())
out=p/'output'
for n in ('hekate.bin','hekate_unc.bin','libsys_minerva.bso','libsys_lp0.bso'):
 assert sha(out/n)==sha(base/'output'/n),n
assert (out/'nyx.bin').read_bytes()[0x98:0x9f]==b'GCTC195'
assert (out/'hekate.bin').read_bytes()[0x118:0x11f]==b'ICTC655'
arm=Path('C:/devkitPro/devkitARM/bin')
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
 tmp=w/('linked-nova28-'+name)
 subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
gui=(p/'nyx/nyx_gui/frontend/gui.c').read_text()
assert gui.count('gui_bluetooth_poll_fast();')==1 and gui.count('gui_bluetooth_poll();')==2
assert 'gui_bluetooth_poll_fast();' in gui[gui.index('gfx_set_rect_pitch('):gui.index('lv_flush_ready();')]
assert '(void)unused;nova_bt_poll_fast();' in (p/'nyx/nyx_gui/frontend/gui_bluetooth.c').read_text()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+f+r'$',nm,re.M)for f in probes)
reference={f.name:sha(f)for f in (w/'nova24-reference').iterdir()if f.is_file()}
info={'product':'Hekate Nova Pro','version':'28','base':'Hekate 6.5.5 / Nyx 1.9.5','build_exit_code':0,
 'source_sha256':current,'changed_existing_sources':sorted(changed),'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},
 'tests':tests,'native_probes':42,'physical_validation':False,'reference_snapshot_sha256':reference,'hwtest_reference':audit,
 'change':'Short Bluetooth RX while drawing and on the GUI timer; foreground burst drainage retained. Import failure categories and buttons-seen/report-rate metadata',
 'limits':'PC models validate receive budgets and metadata, not console FPS. Physical v27 L input was reported with severe latency and no R A action; current import remains 0/2. V28 does not yet identify or repair the physical import failure',
 'unchanged':'BDK, TV, radio profile, validation policy and vendor BTstack unchanged versus v27; input mapping unchanged, loader and libsys identical',
 'license_note':'Existing notices retained; HWTest GUI host referenced with commit/hashes, protocol settings adapted through BTstack APIs'}
(w/'verification-nova28.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
patch=''.join(''.join(difflib.unified_diff((base/n).read_text(encoding='utf-8').splitlines(True),(p/n).read_text(encoding='utf-8').splitlines(True),fromfile='v27/'+n,tofile='v28/'+n))for n in sorted(changed))
(w/'nova28-responsiveness.patch').write_text(patch,encoding='utf-8')
print('PASS: v28 ARM binaries, fixed header, restricted changes and current test evidence verified; physical link not claimed.')
