"""Package the verified experiment without changing the user's installed SD files."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import difflib, hashlib, json

work = Path(__file__).resolve().parent
project = work / 'hekate-tv-02'
dest = work.parent / 'outputs'
reference = work / 'tv-upstream'
info = json.loads((work / 'verification-tv-02.json').read_text())
sha = lambda data: hashlib.sha256(data).hexdigest()
assert info['build_exit_code'] == 0 and info['hardware_validation'] is False
for relative, digest in info['source_sha256'].items():
    assert sha((project / relative).read_bytes()) == digest, relative
for filename, record in info['binaries'].items():
    assert sha((project / 'output' / filename).read_bytes()) == record['sha256'], filename

official = reference / 'hekate_ctcaer_6.5.5_Nyx_1.9.5.zip'
assert sha(official.read_bytes()) == 'e3c6613fc9e099d00683063855812a6059247da6e9d75f2caa203f9e78e5df56'
with ZipFile(official) as z:
    assert z.testzip() is None
    files = {name: z.read(name) for name in z.namelist()
             if name.startswith('bootloader/') and not name.endswith('/')
             and name != 'bootloader/update.bin'}
for name in ('nyx.bin', 'libsys_lp0.bso', 'libsys_minerva.bso'):
    files['bootloader/sys/' + name] = (project / 'output' / name).read_bytes()
files['hekate_tv_experiment_02_6.5.5.bin'] = (project / 'output/hekate.bin').read_bytes()
files['LIRE-MOI-TV-02.md'] = (project / 'LIRE-MOI-TV-02.md').read_bytes()
files['documentation/TV-SOURCES.md'] = (project / 'TV-SOURCES.md').read_bytes()
files['documentation/VERIFICATION-TV-02.json'] = (work / 'verification-tv-02.json').read_bytes()
files['documentation/LICENSE-HEKATE'] = (project / 'LICENSE').read_bytes()
files['BUNDLE-MANIFEST.json'] = json.dumps({name: sha(data) for name, data in files.items()}, indent=2).encode()
assert not any(name.endswith(('hekate_ipl.ini', 'nyx.ini', 'update.bin')) for name in files)
bundle = dest / 'hekate-tv-experiment-02.zip'
with ZipFile(bundle, 'w', ZIP_DEFLATED) as z:
    for name, data in sorted(files.items()): z.writestr(name, data)
with ZipFile(bundle) as z:
    assert z.testzip() is None
    assert len(z.namelist()) == len(set(z.namelist()))
    for name, data in files.items(): assert z.read(name) == data

changed = ['nyx/Makefile', 'nyx/nyx_gui/frontend/gui.c', 'nyx/nyx_gui/frontend/gui_info.c']
added = ['bdk/display/tv/' + n for n in ('dpout.c', 'dpout.h', 'usbpd.c', 'usbpd.h')]
added += ['nyx/nyx_gui/frontend/gui_tv.c', 'nyx/nyx_gui/frontend/gui_tv.h']
diff = ''
for rel in changed + added:
    original = Path('C:/Users/Retrogen/Desktop/nd5') / rel
    old = original.read_text().splitlines(keepends=True) if original.exists() else []
    new = (project / rel).read_text().splitlines(keepends=True)
    diff += ''.join(difflib.unified_diff(old, new, fromfile='a/' + rel if old else '/dev/null', tofile='b/' + rel))

sources = dest / 'hekate-tv-experiment-02-sources.zip'
with ZipFile(sources, 'w', ZIP_DEFLATED) as z:
    for rel in sorted(info['source_sha256']): z.write(project / rel, 'hekate-tv-02/' + rel)
    for name in ('run_build_tv02.py', 'verify_tv_experiment_02.py', 'package_tv_experiment_02.py',
                 'check_tv_lifecycle_02.py', 'check_tv_policy_02.py', 'check_tv_protocol.py'):
        z.write(work / name, 'build-tools/' + name)
    for p in (work / 'tv02-tests').rglob('*'):
        if p.is_file() and p.suffix in ('.h', '.c', '.inc'):
            z.write(p, 'build-tools/tv02-tests/' + p.relative_to(work / 'tv02-tests').as_posix())
    for name in ('dpout.c', 'dpout.h', 'usbpd.c', 'usbpd.h', 'README.md', 'docs/dock.md', 'commit.txt'):
        z.write(reference / name, 'reference/HWTest-RCM/' + name)
    z.writestr('patches/nd5-to-tv-02.patch', diff)
    z.write(work / 'verification-tv-02.json', 'VERIFICATION-TV-02.json')
    z.write(work / 'tv-protocol-baseline.json', 'verification/protocol-baseline.json')
    z.write(work / 'hekate-tv-02-build.log', 'verification/full-build.log')
    z.writestr('BUILDING.txt',
        'Sources du prototype TV personnel 02, distinct de Nova Pro v18.\n'
        'Compilation standard : make -j4 avec devkitARM, Python3, un GCC natif et les outils Unix de devkitPro.\n'
        'Les scripts de verification Windows attendent work/hekate-tv-02,\n'
        'work/tv02-tests et work/toolchains/w64devkit dans le workspace.\n'
        'Les tests PC ne valident pas le fonctionnement du dock sur console.\n'
        'Lire TV-SOURCES.md avant toute redistribution publique.\n')
with ZipFile(sources) as z:
    assert z.testzip() is None and len(z.namelist()) == len(set(z.namelist()))
    for rel, digest in info['source_sha256'].items(): assert sha(z.read('hekate-tv-02/' + rel)) == digest
for p in (bundle, sources): print(p.name, p.stat().st_size, sha(p.read_bytes()))
(dest / 'hekate-tv-experiment-02-SHA256.txt').write_text(
    ''.join(sha(p.read_bytes()) + '  ' + p.name + '\n' for p in (bundle, sources)))
(dest / 'LIRE-MOI-TV-02.md').write_bytes((project / 'LIRE-MOI-TV-02.md').read_bytes())
