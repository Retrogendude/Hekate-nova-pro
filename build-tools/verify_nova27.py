"""Bind the v27 test evidence, final ARM binaries and restricted source diff."""
from pathlib import Path
import subprocess,json,hashlib,re,difflib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v27';base=w/'hekate-nova-pro-v26'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources(root):
 return {f.relative_to(root).as_posix():sha(f) for f in root.rglob('*') if f.is_file()
  and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(root).parts)
  and f.relative_to(root).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
current,prior=sources(p),sources(base)
changed={n for n in current if n in prior and current[n]!=prior[n]}
expected={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_bluetooth.c','nova_version.h','localization_catalog.inc','localization_catalog.json','bluetooth/nova_bt.h','bluetooth/nova_bt_stack.c','bluetooth/nova_bt_radio.c']}
assert changed==expected,changed^expected
assert set(prior)<=set(current),'Prior source removed'
old_catalog=json.loads((base/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
catalog=json.loads((p/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(catalog[k]==v for k,v in old_catalog.items())
assert len(catalog)==len(old_catalog)+1
names=['nova27-pairing-check.json','nova27-bt-check.json','nova27-negotiation-check.json','nova27-report-check.json','nova27-input-check.json','nova27-header-check.json','native-ui-nova27.json','nova27-navigation-check.json']
tests={n:json.loads((w/n).read_text(encoding='utf-8')) for n in names}
assert tests[names[0]]['old_double_reversal_rejected']
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
ui=tests[names[6]];assert ui['native_scenes']==462 and not ui['failures']
for n,h in ui['source_sha256'].items():assert current[n]==h,n
nav=tests[names[7]];assert nav['passed_scenes']==28
for n,h in nav['source_sha256'].items():assert current['nyx/nyx_gui/'+n]==h,n
for n,h in tests[names[0]]['source_sha256'].items():assert current[n]==h,n
radio=json.loads((w/'nova27-radio-check.json').read_text());assert radio['passed']
assert radio['source_sha256']==current['nyx/nyx_gui/bluetooth/nova_bt_radio.c']
tests['nova27-radio-check.json']=radio
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
 tmp=w/('linked-nova27-'+name)
 subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+f+r'$',nm,re.M)for f in probes)
reference={f.name:sha(f)for f in (w/'nova24-reference').iterdir()if f.is_file()}
info={'product':'Hekate Nova Pro','version':'27','base':'Hekate 6.5.5 / Nyx 1.9.5','build_exit_code':0,
 'source_sha256':current,'changed_existing_sources':sorted(changed),'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},
 'tests':tests,'native_probes':42,'physical_validation':False,'reference_snapshot_sha256':reference,'hwtest_reference':audit,
 'change':'Adapt HWTest-RCM GUI radio profile: BR, sniff recovery, QoS, bounded UART bursts, 8-byte watermark, supplies/reference clock and decoded multi-signal trace',
 'limits':'HWTest documents bench results; PC models check EDR input loss, UART restart timing and register restoration. Actual v26 code fails new regressions. Physical Nova27 input remains unconfirmed',
 'unchanged':'BDK/TV/diagnostics/input decoder/key import and vendor BTstack unchanged versus v26; loader and libsys binaries identical',
 'license_note':'Existing notices retained; HWTest GUI host referenced with commit/hashes, protocol settings adapted through BTstack APIs'}
(w/'verification-nova27.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
patch=''.join(''.join(difflib.unified_diff((base/n).read_text(encoding='utf-8').splitlines(True),(p/n).read_text(encoding='utf-8').splitlines(True),fromfile='v26/'+n,tofile='v27/'+n))for n in sorted(changed))
(w/'nova27-radio-profile.patch').write_text(patch,encoding='utf-8')
print('PASS: v27 ARM binaries, fixed header, restricted changes and current test evidence verified; physical link not claimed.')
