"""Package personal Nova 19-TV, preserving the previous releases."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib, json
w = Path(__file__).resolve().parent
p = w/'hekate-nova-pro-v19-tv'
dest = w.parent/'outputs'
info = json.loads((w/'verification-nova-tv.json').read_text(encoding='utf-8'))
sha = lambda b: hashlib.sha256(b).hexdigest()
assert info['build_exit_code'] == 0 and info['version'] == '19-TV'
for rel, digest in info['source_sha256'].items(): assert sha((p/rel).read_bytes()) == digest,rel
for name, record in info['binaries'].items(): assert sha((p/'output'/name).read_bytes()) == record['sha256'],name
with ZipFile(dest/'hekate-nova-pro-v18.zip') as z:
    assert z.testzip() is None
    files = {n:z.read(n) for n in z.namelist() if not n.endswith('/')
             and not n.startswith(('documentation/','hekate_nova_pro_'))
             and n not in ('LIRE-MOI.txt','BUILD-INFO.json')}
for name in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):
    files['bootloader/sys/'+name] = (p/'output'/name).read_bytes()
files['bootloader/update.bin'] = (p/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.4_v19_tv.bin'] = (p/'output/hekate.bin').read_bytes()
files['LIRE-MOI.txt'] = (p/'LIRE-MOI-V19-TV.md').read_bytes()
files['documentation/TV-SOURCES.md'] = (p/'TV-SOURCES.md').read_bytes()
files['documentation/VERIFICATION-V19-TV.json'] = (w/'verification-nova-tv.json').read_bytes()
files['documentation/BASELINE-V18.md'] = (dest/'SUIVI-CAHIER-CHARGES-V18.md').read_bytes()
files['documentation/NATIVE-UI-V19-TV.json'] = (w/'native-ui-nova-tv.json').read_bytes()
files['BUILD-INFO.json'] = json.dumps({k:v for k,v in info.items() if k!='source_sha256'},ensure_ascii=False,indent=2).encode()
assert not any(n.endswith(('hekate_ipl.ini','nyx.ini')) for n in files)
ram = w/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin'
assert files['bootloader/sys/hwtest_ram.bin'] == ram.read_bytes()
files['BUNDLE-MANIFEST.json'] = json.dumps({n:sha(b) for n,b in files.items()},indent=2).encode()
bundle = dest/'hekate-nova-pro-v19-tv.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED) as z:
    for name,data in sorted(files.items()): z.writestr(name,data)
with ZipFile(bundle) as z:
    assert z.testzip() is None and len(z.namelist()) == len(set(z.namelist()))
    for name,data in files.items(): assert z.read(name) == data
sources = dest/'hekate-nova-pro-v19-tv-sources.zip'
with ZipFile(sources,'w',ZIP_DEFLATED) as z:
    for rel in sorted(info['source_sha256']): z.write(p/rel,'hekate-nova-pro-v19-tv/'+rel)
    for name in ('run_build_nova_tv.py','verify_nova_tv.py','package_nova_tv.py',
                 'check_nova_tv_lifecycle.py','check_nova_tv_sd_contract.py','check_nova_tv_policy.py',
                 'check_tv_protocol.py','localize_nova_tv.py','render_lvgl_preview_nova_tv.py','check_nova_tv_native_ui.py'):
        z.write(w/name,'build-tools/'+name)
    for f in (w/'nova-tv-tests').rglob('*'):
        if f.is_file() and f.suffix in ('.c','.h','.inc'):
            z.write(f,'build-tools/nova-tv-tests/'+f.relative_to(w/'nova-tv-tests').as_posix())
    z.write(w/'tv-tests/stubs/utils/types.h','build-tools/tv-tests/stubs/utils/types.h')
    for name in ('dpout.c','dpout.h','usbpd.c','usbpd.h','README.md','docs/dock.md','commit.txt'):
        z.write(w/'tv-upstream'/name,'reference/HWTest-RCM/'+name)
    for name in ('verification-nova-tv.json','native-ui-nova-tv.json','nova-tv-build.log'):
        z.write(w/name,'verification/'+name)
    with ZipFile(dest/'hekate-nova-pro-v18-sources.zip') as previous:
        for name in previous.namelist():
            if not name.endswith('/') and name.startswith(('HWTest-RCM-0.15/','resources/')):
                z.writestr(name,previous.read(name))
    z.writestr('BUILDING.txt',
        'Hekate Nova Pro 19-TV : make -j4 avec devkitARM et les outils du projet.\n'
        'Base 6.5.4 / Nyx 1.9.4 ; les ressources Nova sont reprises de v18.\n'
        'HWTest-RCM et ses ressources RAM restent identiques a v18.\n'
        'Les scripts Windows attendent les sources dans work/hekate-nova-pro-v19-tv,\n'
        'les fixtures dans work/nova-tv-tests et le compilateur natif dans\n'
        'work/toolchains/w64devkit. Ils documentent la configuration de ce workspace.\n'
        'Les rendus natifs attendent aussi l archive officielle 6.5.4/1.9.4 et les\n'
        'polices de test documentees dans les scripts. Ils ne simulent pas HDMI.\n'
        'Lire LIRE-MOI-V19-TV.md et TV-SOURCES.md avant essai ou redistribution.\n')
with ZipFile(sources) as z:
    assert z.testzip() is None and len(z.namelist()) == len(set(z.namelist()))
    for rel,digest in info['source_sha256'].items(): assert sha(z.read('hekate-nova-pro-v19-tv/'+rel)) == digest
for f in (bundle,sources): print(f.name,f.stat().st_size,sha(f.read_bytes()))
(dest/'hekate-nova-pro-v19-tv-SHA256.txt').write_text(''.join(sha(f.read_bytes())+'  '+f.name+'\n' for f in (bundle,sources)))
(dest/'LIRE-MOI-V19-TV.md').write_bytes((p/'LIRE-MOI-V19-TV.md').read_bytes())
