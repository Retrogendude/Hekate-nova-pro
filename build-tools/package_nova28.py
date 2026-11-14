from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v28';dest=w.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((w/'verification-nova28.json').read_text(encoding='utf-8'))
assert info['version']=='28'
for n,h in info['source_sha256'].items():assert sha((p/n).read_bytes())==h,n
for n,v in info['binaries'].items():assert sha((p/'output'/n).read_bytes())==v['sha256'],n
with ZipFile(dest/'hekate-nova-pro-v27.zip')as old:
 assert old.testzip()is None
 files={n:old.read(n)for n in old.namelist()if not n.endswith('/')and not n.startswith(('documentation/','hekate_nova_pro_'))
  and n not in ('LIRE-MOI.txt','BUILD-INFO.json','BUNDLE-MANIFEST.json')}
for n in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+n]=(p/'output'/n).read_bytes()
files['bootloader/update.bin']=(p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.5_v28.bin']=(p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt']=(p/'LIRE-MOI-V28.md').read_bytes()
files['documentation/HWTEST-BLUETOOTH-SOURCES.md']=(p/'HWTEST-BLUETOOTH-SOURCES.md').read_bytes()
files['documentation/HWTEST-BLUETOOTH-AUDIT.json']=(w/'nova27-reference/audit.json').read_bytes()
files['documentation/VERIFICATION-V28.json']=(w/'verification-nova28.json').read_bytes()
files['documentation/NOVA27-RADIO-PROFILE.diff']=(w/'nova28-responsiveness.patch').read_bytes()
for name,src in [('BTSTACK-LICENSE.txt','nyx/nyx_gui/libs/btstack/LICENSE'),('BTSTACK-UPSTREAM.txt','nyx/nyx_gui/libs/btstack/UPSTREAM.txt'),('TV-SOURCES.md','TV-SOURCES.md')]:
 files['documentation/'+name]=(p/src).read_bytes()
files['documentation/BTSTACK-NOVA-PATCH.diff']=(w/'nova26-hid-host.patch').read_bytes()
files['documentation/UPSTREAM-AUDIT.json']=(w/'upstream-audit-6.5.5/audit.json').read_bytes()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode('utf-8')
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini'))for n in files)
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert files['bootloader/update.bin'][0x118:0x11f]==b'ICTC655'
assert files['bootloader/sys/hwtest_ram.bin']==(w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin').read_bytes()
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
bundle=dest/'hekate-nova-pro-v28.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
 for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,b in files.items():assert z.read(n)==b,n
tools=('run_build_nova28.py','verify_nova28.py','package_nova28.py','check_nova28_pairing.py','check_nova28_bt.py','check_nova28_negotiation.py',
 'check_nova28_report.py','check_nova28_radio.py','check_nova28_input.py','check_nova28_header.py','render_lvgl_preview_nova28.py','check_nova28_native_ui.py','check_nova28_navigation.py','localize_nova28.py')
source=dest/'hekate-nova-pro-v28-sources.zip'
with ZipFile(source,'w',ZIP_DEFLATED)as z:
 for n in sorted(info['source_sha256']):z.write(p/n,'hekate-nova-pro-v28/'+n)
 for n in tools:z.write(w/n,'build-tools/'+n)
 for n in (*info['tests'],'verification-nova28.json','nova28-build.log','nova28-responsiveness.patch','nova26-hid-host.patch'):
  z.write(w/n,'verification/'+n)
 # Preserve previously supplied upstream provenance and the standalone RAM tool.
 with ZipFile(dest/'hekate-nova-pro-v27-sources.zip')as old:
  for n in old.namelist():
   if not n.endswith('/')and n.startswith(('reference/','HWTest-RCM-0.15/','resources/')):z.writestr(n,old.read(n))
 # Minimal older fixtures used by negative regressions; no console data.
 for name,src in [('v27-nova_bt_stack.c','hekate-nova-pro-v27/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v26-nova_bt_stack.c','hekate-nova-pro-v26/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v25-hid_host.c','hekate-nova-pro-v25/nyx/nyx_gui/libs/btstack/classic/hid_host.c'),('v24-nova_bt_stack.c','hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v23-gui_bluetooth.c','hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c'),('v21-gui.c','hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c'),('v21-nyx.bin','hekate-nova-pro-v21-bt/output/nyx.bin')]:
  z.write(w/src,'build-tools/regression-reference/'+name)
 z.writestr('BUILDING.txt',
  'Hekate Nova Pro28: build with devkitARM, GNU make -j4 -r and Python.\n'
  'Read LIRE-MOI-V28.md, HWTEST-BLUETOOTH-SOURCES.md and all licenses.\n'
  'The Windows scripts retain the development workspace layout: scripts in work/, project in work/hekate-nova-pro-v28.\n'
  'Adjust the portable tools paths when using another machine; the project Makefiles also support the usual devkitPro environment.\n'
  'Native UI tests additionally use the prior official res.pak archive and font fixtures described in the renderer.\n'
  'For negative regressions, restore build-tools/regression-reference/v23-gui_bluetooth.c as work/hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c,\n'
  'v21-gui.c as work/hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c, and v21-nyx.bin as work/hekate-nova-pro-v21-bt/output/nyx.bin.\n'
  'Restore v24-nova_bt_stack.c as work/hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the old-policy regressions.\n'
  'Restore v25-hid_host.c as work/hekate-nova-pro-v25/nyx/nyx_gui/libs/btstack/classic/hid_host.c for the actual refusal regression.\n'
  'Restore v26-nova_bt_stack.c as work/hekate-nova-pro-v26/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the ROM and timed-UART regressions.\n'
  'Restore v27-nova_bt_stack.c as work/hekate-nova-pro-v27/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the display-poll regression.\n'
  'Run pairing before negotiation (it builds the SPI/import fixture object); build renderer and run native UI before navigation.\n'
  'Run verify after the tests and final ARM build. Do not remove the Nyx metadata linker assertion.\n'
  'PC tests substitute the radio and do not establish successful wireless operation on real hardware.\n'
  'The separate BTstack personal/noncommercial license remains applicable; vendor HID changes are documented in the supplied patch.\n')
with ZipFile(source)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v28/'+n))==h,n
for f in (bundle,source):print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v28-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n'for f in (bundle,source)))
(dest/'LIRE-MOI-V28.md').write_bytes((p/'LIRE-MOI-V28.md').read_bytes())
