from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v31';dest=w.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((w/'verification-nova31.json').read_text(encoding='utf-8'))
assert info['version']=='31'
for n,h in info['source_sha256'].items():assert sha((p/n).read_bytes())==h,n
for n,v in info['binaries'].items():assert sha((p/'output'/n).read_bytes())==v['sha256'],n
with ZipFile(dest/'hekate-nova-pro-v30.zip')as old:
 assert old.testzip()is None
 files={n:old.read(n)for n in old.namelist()if not n.endswith('/')and not n.startswith(('documentation/','hekate_nova_pro_'))
  and n not in ('LIRE-MOI.txt','BUILD-INFO.json','BUNDLE-MANIFEST.json')}
for n in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+n]=(p/'output'/n).read_bytes()
files['bootloader/update.bin']=(p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.5_v31.bin']=(p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt']=(p/'LIRE-MOI-V31.md').read_bytes()
files['documentation/HWTEST-BLUETOOTH-SOURCES.md']=(p/'HWTEST-BLUETOOTH-SOURCES.md').read_bytes()
files['documentation/HWTEST-BLUETOOTH-AUDIT.json']=(w/'nova27-reference/audit.json').read_bytes()
files['documentation/VERIFICATION-V31.json']=(w/'verification-nova31.json').read_bytes()
files['documentation/NOVA31-EXPERT-WIRELESS.diff']=(w/'nova31-expert-wireless.patch').read_bytes()
for name,src in [('BTSTACK-LICENSE.txt','nyx/nyx_gui/libs/btstack/LICENSE'),('BTSTACK-UPSTREAM.txt','nyx/nyx_gui/libs/btstack/UPSTREAM.txt'),('TV-SOURCES.md','TV-SOURCES.md')]:
 files['documentation/'+name]=(p/src).read_bytes()
files['documentation/BTSTACK-NOVA-PATCH.diff']=(w/'nova26-hid-host.patch').read_bytes()
files['documentation/UPSTREAM-AUDIT.json']=(w/'upstream-audit-6.5.5/audit.json').read_bytes()
for name in ('AUDIT-NOVA30.md','AUDIT-NOVA30-GUI.md','AUDIT-NOVA30-WIRELESS.md','AUDIT-NOVA30-CORE.md'):files['documentation/'+name]=(p/name).read_bytes()
files['documentation/INVESTIGATION-V31.md']=(w/'TASK-NOVA31-RANGE.md').read_bytes()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode('utf-8')
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','joycon-pairing.tmp'))for n in files)
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert files['bootloader/update.bin'][0x118:0x11f]==b'ICTC655'
assert files['bootloader/sys/hwtest_ram.bin']==(w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin').read_bytes()
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
bundle=dest/'hekate-nova-pro-v31.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
 for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,b in files.items():assert z.read(n)==b,n
tools=('run_build_nova31.py','verify_nova31.py','package_nova31.py','check_nova31_pairing.py','check_nova31_bt.py','check_nova31_negotiation.py',
 'check_nova31_report.py','check_nova31_radio.py','check_nova31_input.py','check_nova31_header.py','render_lvgl_preview_nova31.py','check_nova31_native_ui.py','check_nova31_navigation.py','localize_nova31.py','check_nova31_reader.py','check_nova31_pairing_file.py','check_nova31_pairing_sd.py','check_nova31_autostart.py','check_nova31_wireless.py','check_nova30_reflection_timing.py','check_nova30_core_io.py','check_nova31_rf.py','check_nova31_display.py','check_nova31_raster.py','check_nova31_partition.py','check_nova29_bt.py')
source=dest/'hekate-nova-pro-v31-sources.zip'
with ZipFile(source,'w',ZIP_DEFLATED)as z:
 for n in sorted(info['source_sha256']):z.write(p/n,'hekate-nova-pro-v31/'+n)
 for n in tools:z.write(w/n,'build-tools/'+n)
 for n in (*info['tests'],'verification-nova31.json','nova31-build.log','nova31-expert-wireless.patch','nova26-hid-host.patch'):
  z.write(w/n,'verification/'+n)
 # Preserve previously supplied upstream provenance and the standalone RAM tool.
 with ZipFile(dest/'hekate-nova-pro-v30-sources.zip')as old:
  for n in old.namelist():
   if not n.endswith('/')and n.startswith(('reference/','HWTest-RCM-0.15/','resources/')):z.writestr(n,old.read(n))
 # Minimal older fixtures used by negative regressions; no console data.
 for name,src in [('v28-nova_bt_stack.c','hekate-nova-pro-v28/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v28-gui_bluetooth.c','hekate-nova-pro-v28/nyx/nyx_gui/frontend/gui_bluetooth.c'),('v27-nova_bt_stack.c','hekate-nova-pro-v27/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v26-nova_bt_stack.c','hekate-nova-pro-v26/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v25-hid_host.c','hekate-nova-pro-v25/nyx/nyx_gui/libs/btstack/classic/hid_host.c'),('v24-nova_bt_stack.c','hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c'),('v23-gui_bluetooth.c','hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c'),('v21-gui.c','hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c'),('v21-nyx.bin','hekate-nova-pro-v21-bt/output/nyx.bin')]:
  z.write(w/src,'build-tools/regression-reference/'+name)
 # Current project plus these snapshots reproduce the v29 before-fix cases.
 for name in ['bluetooth/nova_bt.h','bluetooth/nova_bt_protocol.c','bluetooth/nova_bt_stack.c','frontend/gui_nova.c','frontend/gui_nova_reflection.c','nyx.c','nova_version.h']:
  z.write(w/'hekate-nova-pro-v29/nyx/nyx_gui'/name,'build-tools/regression-reference/v29/'+name)
 for name in ('preview.c','lv_conf.h','res.pak'):
  if 'resources/preview-v29/'+name not in z.namelist():z.write(w/'lvgl-preview-nova29'/name,'resources/preview-v29/'+name)
 for n in ['bdk/libs/lvgl/lv_draw/lv_draw_vbasic.c','bdk/libs/lvgl/lv_draw/lv_draw_vbasic.h']:
  z.write(w/'hekate-nova-pro-v30'/n,'build-tools/regression-reference/v30/'+n)
 z.writestr('BUILDING.txt',
  'Hekate Nova Pro31: build with devkitARM, GNU make -j4 -r and Python.\n'
  'Read LIRE-MOI-V31.md, HWTEST-BLUETOOTH-SOURCES.md and all licenses.\n'
  'The Windows scripts retain the development workspace layout: scripts in work/, project in work/hekate-nova-pro-v31.\n'
  'Adjust the portable tools paths when using another machine; the project Makefiles also support the usual devkitPro environment.\n'
  'Native UI tests additionally use the prior official res.pak archive and font fixtures described in the renderer.\n'
  'For negative regressions, restore build-tools/regression-reference/v23-gui_bluetooth.c as work/hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c,\n'
  'v21-gui.c as work/hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c, and v21-nyx.bin as work/hekate-nova-pro-v21-bt/output/nyx.bin.\n'
  'Restore v24-nova_bt_stack.c as work/hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the old-policy regressions.\n'
  'Restore v25-hid_host.c as work/hekate-nova-pro-v25/nyx/nyx_gui/libs/btstack/classic/hid_host.c for the actual refusal regression.\n'
  'Restore v26-nova_bt_stack.c as work/hekate-nova-pro-v26/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the ROM and timed-UART regressions.\n'
  'Restore v27-nova_bt_stack.c as work/hekate-nova-pro-v27/nyx/nyx_gui/bluetooth/nova_bt_stack.c for the display-poll regression.\n'
  'Restore v28-nova_bt_stack.c and v28-gui_bluetooth.c under work/hekate-nova-pro-v28/nyx/nyx_gui/bluetooth/ and frontend/ for the v28 regressions.\n'
  'For new v29 regressions, copy current project into work/hekate-nova-pro-v29 and overwrite nyx/nyx_gui with build-tools/regression-reference/v29 contents.\n'
  'Restore resources/preview-v29 to work/lvgl-preview-nova29 for the faithful reflection timing test.\n'
  'Run pairing SD before autostart; run display/raster/RF/partition checks before verify. The two v30 audit JSON files describe unchanged sources and are checked against current hashes.\n'
  'Run pairing before negotiation (it builds the SPI/import fixture object); build renderer and run native UI before navigation.\n'
  'Run verify after the tests and final ARM build. Do not remove the Nyx metadata linker assertion.\n'
  'PC tests substitute hardware and do not establish wireless range or storage safety on real hardware. No physical partitioning has been executed.\n'
  'The separate BTstack personal/noncommercial license remains applicable; vendor HID changes are documented in the supplied patch.\n')
with ZipFile(source)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v31/'+n))==h,n
for f in (bundle,source):print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v31-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n'for f in (bundle,source)))
(dest/'LIRE-MOI-V31.md').write_bytes((p/'LIRE-MOI-V31.md').read_bytes())

for name in ('AUDIT-NOVA30.md','AUDIT-NOVA30-GUI.md','AUDIT-NOVA30-WIRELESS.md','AUDIT-NOVA30-CORE.md'):(dest/name).write_bytes((p/name).read_bytes())
