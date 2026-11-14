from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v25';dest=w.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((w/'verification-nova25.json').read_text(encoding='utf-8'))
assert info['version']=='25'
for n,h in info['source_sha256'].items():assert sha((p/n).read_bytes())==h,n
for n,v in info['binaries'].items():assert sha((p/'output'/n).read_bytes())==v['sha256'],n
with ZipFile(dest/'hekate-nova-pro-v24.zip')as old:
 assert old.testzip()is None
 files={n:old.read(n)for n in old.namelist()if not n.endswith('/')and not n.startswith(('documentation/','hekate_nova_pro_'))
  and n not in ('LIRE-MOI.txt','BUILD-INFO.json','BUNDLE-MANIFEST.json')}
for n in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+n]=(p/'output'/n).read_bytes()
files['bootloader/update.bin']=(p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.5_v25.bin']=(p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt']=(p/'LIRE-MOI-V25.md').read_bytes()
files['documentation/AUTH-SOURCES-V25.md']=(p/'AUTH-SOURCES-V25.md').read_bytes()
files['documentation/VERIFICATION-V25.json']=(w/'verification-nova25.json').read_bytes()
files['documentation/NOVA25-AUTHENTICATION.diff']=(w/'nova25-authentication.patch').read_bytes()
for name,src in [('BTSTACK-LICENSE.txt','nyx/nyx_gui/libs/btstack/LICENSE'),('BTSTACK-UPSTREAM.txt','nyx/nyx_gui/libs/btstack/UPSTREAM.txt'),('TV-SOURCES.md','TV-SOURCES.md')]:
 files['documentation/'+name]=(p/src).read_bytes()
files['documentation/BTSTACK-NOVA-PATCH.diff']=(w/'nova23-hid-host.patch').read_bytes()
files['documentation/UPSTREAM-AUDIT.json']=(w/'upstream-audit-6.5.5/audit.json').read_bytes()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode('utf-8')
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini'))for n in files)
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert files['bootloader/update.bin'][0x118:0x11f]==b'ICTC655'
assert files['bootloader/sys/hwtest_ram.bin']==(w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin').read_bytes()
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
bundle=dest/'hekate-nova-pro-v25.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
 for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,b in files.items():assert z.read(n)==b,n
tools=('run_build_nova25.py','verify_nova25.py','package_nova25.py','check_nova25_pairing.py','check_nova25_bt.py','check_nova25_negotiation.py',
 'check_nova25_report.py','check_nova25_input.py','check_nova25_header.py','render_lvgl_preview_nova25.py','check_nova25_native_ui.py','check_nova25_navigation.py','localize_nova25.py')
source=dest/'hekate-nova-pro-v25-sources.zip'
with ZipFile(source,'w',ZIP_DEFLATED)as z:
 for n in sorted(info['source_sha256']):z.write(p/n,'hekate-nova-pro-v25/'+n)
 for n in tools:z.write(w/n,'build-tools/'+n)
 for n in (*info['tests'],'verification-nova25.json','nova25-build.log','nova25-authentication.patch','nova23-hid-host.patch'):
  z.write(w/n,'verification/'+n)
 # Preserve previously supplied upstream provenance and the standalone RAM tool.
 with ZipFile(dest/'hekate-nova-pro-v24-sources.zip')as old:
  for n in old.namelist():
   if not n.endswith('/')and n.startswith(('reference/','HWTest-RCM-0.15/','resources/')):z.writestr(n,old.read(n))
 # Minimal older fixtures used by negative regressions; no console data.
 for name,src in [('v24-nova_bt_stack.c','hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v23-gui_bluetooth.c','hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c'),('v21-gui.c','hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c'),('v21-nyx.bin','hekate-nova-pro-v21-bt/output/nyx.bin')]:
  z.write(w/src,'build-tools/regression-reference/'+name)
 z.writestr('BUILDING.txt',
  'Hekate Nova Pro25: build with devkitARM, GNU make -j4 -r and Python.\n'
  'Read LIRE-MOI-V25.md, AUTH-SOURCES-V25.md and all licenses.\n'
  'The Windows scripts retain the development workspace layout: scripts in work/, project in work/hekate-nova-pro-v25.\n'
  'Adjust the portable tools paths when using another machine; the project Makefiles also support the usual devkitPro environment.\n'
  'Native UI tests additionally use the prior official res.pak archive and font fixtures described in the renderer.\n'
  'For negative regressions, restore build-tools/regression-reference/v23-gui_bluetooth.c as work/hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c,\n'
  'v21-gui.c as work/hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c, and v21-nyx.bin as work/hekate-nova-pro-v21-bt/output/nyx.bin.\n'
  'Restore v24-nova_bt_stack.c as work/hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the old-policy regressions.\n'
  'Run pairing before negotiation (it builds the SPI/import fixture object); build renderer and run native UI before navigation.\n'
  'Run verify after the tests and final ARM build. Do not remove the Nyx metadata linker assertion.\n'
  'PC tests substitute the radio and do not establish successful wireless operation on real hardware.\n'
  'The separate BTstack personal/noncommercial license remains applicable; vendor HID changes are documented in the supplied patch.\n')
with ZipFile(source)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v25/'+n))==h,n
for f in (bundle,source):print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v25-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n'for f in (bundle,source)))
(dest/'LIRE-MOI-V25.md').write_bytes((p/'LIRE-MOI-V25.md').read_bytes())
