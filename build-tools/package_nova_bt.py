"""Package the verified personal prototype, sources, licenses and test evidence."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v21-bt';dest=w.parent/'outputs'
info=json.loads((w/'verification-nova-bt.json').read_text(encoding='utf-8'))
sha=lambda b:hashlib.sha256(b).hexdigest()
assert info['version']=='21-BT' and info['build_exit_code']==0
for f,h in info['source_sha256'].items():assert sha((p/f).read_bytes())==h,f
for f,r in info['binaries'].items():assert sha((p/'output'/f).read_bytes())==r['sha256'],f
with ZipFile(dest/'hekate-nova-pro-v20-dock.zip')as z:
    assert z.testzip() is None
    files={n:z.read(n)for n in z.namelist()if not n.endswith('/') and not n.startswith(('documentation/','hekate_nova_pro_'))
        and n not in ('LIRE-MOI.txt','BUILD-INFO.json','BUNDLE-MANIFEST.json')}
for f in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+f]=(p/'output'/f).read_bytes()
files['bootloader/update.bin']=(p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.4_v21_bt.bin']=(p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt']=(p/'LIRE-MOI-V21-BT.md').read_bytes()
files['documentation/TV-SOURCES.md']=(p/'TV-SOURCES.md').read_bytes()
files['documentation/BTSTACK-LICENSE.txt']=(p/'nyx/nyx_gui/libs/btstack/LICENSE').read_bytes()
files['documentation/BTSTACK-UPSTREAM.txt']=(p/'nyx/nyx_gui/libs/btstack/UPSTREAM.txt').read_bytes()
files['documentation/VERIFICATION-V21-BT.json']=(w/'verification-nova-bt.json').read_bytes()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode('utf-8')
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini'))for n in files)
assert files['bootloader/sys/hwtest_ram.bin']==(w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin').read_bytes()
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
bundle=dest/'hekate-nova-pro-v21-bt.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
    for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
    assert z.testzip() is None and len(z.namelist())==len(set(z.namelist()))
    for n,b in files.items():assert z.read(n)==b
tools=('run_build_nova_bt.py','verify_nova_bt.py','package_nova_bt.py','check_nova_bt.py',
       'check_nova_bt_radio.py','check_nova_bt_input.py','check_nova_bt_tv_lifecycle.py',
       'check_tv_protocol.py','render_lvgl_preview_nova_bt.py','check_nova_bt_native_ui.py','localize_nova_bt.py','build_nova_bt_icons.cjs')
reports=('verification-nova-bt.json','nova-bt-build.log','nova-bt-check.json','nova-bt-radio-check.json',
         'nova-bt-input-check.json','nova-bt-tv-lifecycle-check.json','native-ui-nova-bt.json')
source=dest/'hekate-nova-pro-v21-bt-sources.zip'
with ZipFile(source,'w',ZIP_DEFLATED)as z:
    for rel in sorted(info['source_sha256']):z.write(p/rel,'hekate-nova-pro-v21-bt/'+rel)
    for name in tools:z.write(w/name,'build-tools/'+name)
    for name in reports:z.write(w/name,'verification/'+name)
    for f in (w/'nova-bt-tv-tests').rglob('*'):
        if f.is_file()and f.suffix in ('.c','.h','.inc'):z.write(f,'build-tools/nova-bt-tv-tests/'+f.relative_to(w/'nova-bt-tv-tests').as_posix())
    for name in ('dpout.c','dpout.h','usbpd.c','usbpd.h','README.md','docs/dock.md','commit.txt'):
        z.write(w/'tv-upstream'/name,'reference/HWTest-RCM/'+name)
    with ZipFile(dest/'hekate-nova-pro-v18-sources.zip')as previous:
        for name in previous.namelist():
            if not name.endswith('/')and name.startswith(('HWTest-RCM-0.15/','resources/')):z.writestr(name,previous.read(name))
    z.writestr('BUILDING.txt',
        'Prototype personnel Hekate Nova Pro 21-BT. Lire LIRE-MOI-V21-BT.md.\n'
        'devkitARM, GNU make et Python : make -j4 -r dans les sources.\n'
        'BDK et pile BTstack Classic inclus, cette derniere avec sa licence propre.\n'
        'Aucun micrologiciel Bluetooth proprietaire fourni. RF non valide sur console.\n'
        'La suite Windows documente les chemins du workspace (work/hekate-nova-pro-v21-bt,\n'
        'work/toolchains/w64devkit, outils devkitPro, archive officielle pour le rendu).\n'
        'Les fixtures font fonctionner les vrais corps C avec entrees materielles substituees.\n'
        'Les tests de comparaison demandent aussi le projet v20 et les references amont.\n'
        'Le compagnon RAM et les ressources de la v18 sont conserves.\n')
with ZipFile(source)as z:
    assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
    for rel,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v21-bt/'+rel))==h
for f in (bundle,source):print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v21-bt-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n'for f in (bundle,source)))
(dest/'LIRE-MOI-V21-BT.md').write_bytes((p/'LIRE-MOI-V21-BT.md').read_bytes())
