from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v22';dest=w.parent/'outputs';audit=w/'upstream-audit-6.5.5'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((w/'verification-nova22.json').read_text(encoding='utf-8'))
for f,h in info['source_sha256'].items():assert sha((p/f).read_bytes())==h,f
for f,v in info['binaries'].items():assert sha((p/'output'/f).read_bytes())==v['sha256'],f
for n in info['tests']:info['tests'][n]=json.loads((w/n).read_text(encoding='utf-8'))
info['tests']['tv-protocol-check.json']=json.loads((w/'tv-protocol-check.json').read_text())
info['native_ui_stress']='large report counters, imported pairing note, all seven languages and font sizes 20/36'
info['native_renderer_sha256']=sha((w/'render_lvgl_preview_nova22.py').read_bytes())
with ZipFile(dest/'hekate-nova-pro-v21-bt.zip')as z:
    assert z.testzip()is None
    files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith(('documentation/','hekate_nova_pro_'))
        and n not in ('LIRE-MOI.txt','BUILD-INFO.json','BUNDLE-MANIFEST.json')}
with ZipFile(audit/'official-6.5.5.zip')as official:
    for n in ('bootloader/sys/emummc.kipm','bootloader/sys/l4t/sc7exit_b01.bin'):files[n]=official.read(n)
info['official_companions']={n:sha(files[n])for n in ('bootloader/sys/emummc.kipm','bootloader/sys/l4t/sc7exit_b01.bin')}
(w/'verification-nova22.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
for n in ('nyx.bin','libsys_minerva.bso','libsys_lp0.bso'):files['bootloader/sys/'+n]=(p/'output'/n).read_bytes()
files['bootloader/update.bin']=(p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.5_v22.bin']=(p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt']=(p/'LIRE-MOI-V22.md').read_bytes()
files['documentation/VERIFICATION-V22.json']=(w/'verification-nova22.json').read_bytes()
files['documentation/TV-SOURCES.md']=(p/'TV-SOURCES.md').read_bytes()
files['documentation/BTSTACK-LICENSE.txt']=(p/'nyx/nyx_gui/libs/btstack/LICENSE').read_bytes()
files['documentation/BTSTACK-UPSTREAM.txt']=(p/'nyx/nyx_gui/libs/btstack/UPSTREAM.txt').read_bytes()
files['documentation/UPSTREAM-AUDIT.json']=(audit/'audit.json').read_bytes()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode('utf-8')
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini'))for n in files)
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert files['bootloader/update.bin'][0x118:0x11f]==b'ICTC655'
assert files['bootloader/sys/hwtest_ram.bin']==(w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin').read_bytes()
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
bundle=dest/'hekate-nova-pro-v22.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
    for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
    assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
    for n,b in files.items():assert z.read(n)==b
tools=('run_build_nova22.py','verify_nova22.py','package_nova22.py','check_nova22_bt.py','check_nova22_radio.py',
    'check_nova22_input.py','check_nova22_navigation.py','check_nova22_header.py','check_nova22_tv.py','check_tv_protocol.py',
    'render_lvgl_preview_nova22.py','check_nova22_native_ui.py','check_nova22_visual.py','localize_nova22.py')
source=dest/'hekate-nova-pro-v22-sources.zip'
with ZipFile(source,'w',ZIP_DEFLATED)as z:
    for n in sorted(info['source_sha256']):z.write(p/n,'hekate-nova-pro-v22/'+n)
    for n in tools:z.write(w/n,'build-tools/'+n)
    for n in (*info['tests'],'verification-nova22.json','nova22-build.log'):z.write(w/n,'verification/'+n)
    for folder in ('nova22-tv-tests','tv-tests'):
        for f in (w/folder).rglob('*'):
            if f.is_file()and f.suffix in ('.c','.h','.inc'):z.write(f,'build-tools/'+folder+'/'+f.relative_to(w/folder).as_posix())
    for f in audit.rglob('*'):
        if f.is_file():z.write(f,'reference/upstream-6.5.5/'+f.relative_to(audit).as_posix())
    for name in ('dpout.c','dpout.h','usbpd.c','usbpd.h','README.md','docs/dock.md','commit.txt'):
        z.write(w/'tv-upstream'/name,'reference/HWTest-RCM/'+name)
    with ZipFile(dest/'hekate-nova-pro-v18-sources.zip')as old:
        for n in old.namelist():
            if not n.endswith('/')and n.startswith(('HWTest-RCM-0.15/','resources/')):z.writestr(n,old.read(n))
    z.writestr('BUILDING.txt',
        'Hekate Nova Pro22 : make -j4 -r avec devkitARM, GNU make et Python.\n'
        'Les outils Windows documentent les chemins du workspace de developpement.\n'
        'Lire LIRE-MOI-V22.md et les licences, dont la licence propre de BTstack.\n'
        'Le lien Nyx refuse un en-tete deplace ; ne pas retirer cette assertion.\n'
        'Les tests PC substituent la radio et ne prouvent pas le fonctionnement RF.\n'
        'La suite de regression demande la v21, l archive officielle 6.5.4 pour\n'
        'son res.pak identique a6.5.5 et les polices de test decrites dans les scripts.\n'
        'Le coeur porte les correctifs officiels jusqu a8e1392df4569ac501a115867056b13c361429e9e.\n'
        'Le compagnon RAM et ses ressources sources sont inclus comme dans la v18.\n')
with ZipFile(source)as z:
    assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
    for n,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v22/'+n))==h
for f in (bundle,source):print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v22-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n'for f in (bundle,source)))
(dest/'LIRE-MOI-V22.md').write_bytes((p/'LIRE-MOI-V22.md').read_bytes())
