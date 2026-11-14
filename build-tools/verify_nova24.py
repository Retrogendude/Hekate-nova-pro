"""Bind the v24 test evidence, final ARM binaries and restricted source diff."""
from pathlib import Path
import subprocess,json,hashlib,re,difflib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v24';base=w/'hekate-nova-pro-v23'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources(root):
 return {f.relative_to(root).as_posix():sha(f) for f in root.rglob('*') if f.is_file()
  and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(root).parts)
  and f.relative_to(root).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
current,prior=sources(p),sources(base)
changed={n for n in current if n in prior and current[n]!=prior[n]}
expected={'nyx/nyx_gui/'+n for n in ['bluetooth/nova_bt.h','bluetooth/nova_bt_stack.c','frontend/gui_bluetooth.c','frontend/gui_nova.c','nova_version.h','localization_catalog.inc','localization_catalog.json']}
assert changed==expected,changed^expected
assert set(prior)<=set(current),'Prior source removed'
old_catalog=json.loads((base/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
catalog=json.loads((p/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(catalog[k]==v for k,v in old_catalog.items())
assert len(catalog)==len(old_catalog)+1
names=['nova24-pairing-check.json','nova24-bt-check.json','nova24-negotiation-check.json','nova24-report-check.json','nova24-input-check.json','nova24-header-check.json','native-ui-nova24.json','nova24-navigation-check.json']
tests={n:json.loads((w/n).read_text(encoding='utf-8')) for n in names}
assert tests[names[0]]['old_double_reversal_rejected']
assert tests[names[1]]['passed']
neg=tests[names[2]];assert neg['passed']and len(neg['logs'])==7
for n,h in neg['source_sha256'].items():assert current['nyx/nyx_gui/'+n]==h,n
assert neg['hid_host_sha256']==current['nyx/nyx_gui/libs/btstack/classic/hid_host.c']
report=tests[names[3]];assert report['passed']and report['cases']==11
assert report['source_sha256']==current['nyx/nyx_gui/frontend/gui_bluetooth.c']
assert tests[names[4]]['passed']and tests[names[5]]['nyx_metadata_offset']=='0x98'
ui=tests[names[6]];assert ui['native_scenes']==462 and not ui['failures']
for n,h in ui['source_sha256'].items():assert current[n]==h,n
nav=tests[names[7]];assert nav['passed_scenes']==28
for n,h in nav['source_sha256'].items():assert current['nyx/nyx_gui/'+n]==h,n
for n,h in tests[names[0]]['source_sha256'].items():assert current[n]==h,n
out=p/'output'
for n in ('hekate.bin','hekate_unc.bin','libsys_minerva.bso','libsys_lp0.bso'):
 assert sha(out/n)==sha(base/'output'/n),n
assert (out/'nyx.bin').read_bytes()[0x98:0x9f]==b'GCTC195'
assert (out/'hekate.bin').read_bytes()[0x118:0x11f]==b'ICTC655'
arm=Path('C:/devkitPro/devkitARM/bin')
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
 tmp=w/('linked-nova24-'+name)
 subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+f+r'$',nm,re.M)for f in probes)
reference={f.name:sha(f)for f in (w/'nova24-reference').iterdir()if f.is_file()}
info={'product':'Hekate Nova Pro','version':'24','base':'Hekate 6.5.5 / Nyx 1.9.5','build_exit_code':0,
 'source_sha256':current,'changed_existing_sources':sorted(changed),'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},
 'tests':tests,'native_probes':42,'physical_validation':False,'reference_snapshot_sha256':reference,
 'change':'Copy the BDK cached pairing key without a second reversal; verify controller host identity; separate authentication status in report',
 'limits':'Virtual controller tests include imported keys and SSP/SC support; physical Joy-Con operation still requires confirmation',
 'unchanged':'All BDK, TV, diagnostics, input decoding, HID channels and vendor BTstack code identical to v23; loader and libsys binaries identical',
 'license_note':'Separate BTstack personal/noncommercial license retained'}
(w/'verification-nova24.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
patch=''.join(''.join(difflib.unified_diff((base/n).read_text(encoding='utf-8').splitlines(True),(p/n).read_text(encoding='utf-8').splitlines(True),fromfile='v23/'+n,tofile='v24/'+n))for n in sorted(changed))
(w/'nova24-authentication.patch').write_text(patch,encoding='utf-8')
print('PASS: v24 ARM binaries, fixed header, restricted changes and current test evidence verified; physical link not claimed.')
