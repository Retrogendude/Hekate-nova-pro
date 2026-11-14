"""Package verified binaries, existing resources, and matching editable sources."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,re

work=Path(__file__).resolve().parent;workspace=work.parent;project=work/'hekate-6.5.4';out=project/'output'
companion=work/'HWTest-RCM-0.15';deliveries=workspace/'outputs'
verification=json.loads((work/'verification-v18.json').read_text(encoding='utf-8'))
version=re.search(r'#define NYX_NOVA_VERSION "([^"]+)"',(project/'nyx/nyx_gui/nova_version.h').read_text())[1]
assert version==verification['version']=='18' and verification['build_exit_code']==0
def sha(data):return hashlib.sha256(data).hexdigest()
for filename,info in verification['binaries'].items():
    path=companion/'build-ram'/filename if filename=='hwtest_ram.bin' else out/filename
    assert sha(path.read_bytes())==info['sha256'],filename
for relative,expected in verification['source_sha256'].items():
    assert sha((project/relative).read_bytes())==expected,relative
for relative,expected in verification['ram_source_sha256'].items():
    assert sha((companion/relative).read_bytes())==expected,relative

# Keep the previously delivered, verified support resources. Replace every
# compiled component from this build, and remove old versioned previews/docs.
with ZipFile(deliveries/'hekate-nova-pro-v17.zip')as previous:
    assert previous.testzip()is None
    files={name:previous.read(name)for name in previous.namelist()if not name.endswith('/')
        and not name.startswith('documentation/')and not name.startswith('hekate_nova_pro_')
        and name not in('BUILD-INFO.json','LIRE-MOI.txt')}
replacements={'bootloader/sys/nyx.bin':'nyx.bin','bootloader/sys/libsys_lp0.bso':'libsys_lp0.bso',
              'bootloader/sys/libsys_minerva.bso':'libsys_minerva.bso','bootloader/update.bin':'hekate.bin'}
for target,source in replacements.items():files[target]=(out/source).read_bytes()
payload='hekate_nova_pro_6.5.4_v18.bin';files[payload]=(out/'hekate.bin').read_bytes()
files['bootloader/sys/hwtest_ram.bin']=(companion/'build-ram/hwtest_ram.bin').read_bytes()
files['LIRE-MOI.txt']=(project/'LIRE-MOI-V18.txt').read_bytes()
files['documentation/SUIVI-CAHIER-CHARGES-V18.md']=(deliveries/'SUIVI-CAHIER-CHARGES-V18.md').read_bytes()
files['documentation/VERIFICATION-V18.json']=(work/'verification-v18.json').read_bytes()
files['documentation/VERIFICATION-V18.txt']=(work/'verification-v18.txt').read_bytes()
files['documentation/ANIMATIONS-V18.json']=(work/'animation-v18.json').read_bytes()
files['documentation/NAVIGATION-V18.json']=(work/'navigation-v18.json').read_bytes()
files['icone/LIRE-MOI.txt']=(project/'icone/LIRE-MOI.txt').read_bytes()
files['documentation/DEVELOPPEMENT-TV-DOCK.md']=(deliveries/'DEVELOPPEMENT-TV-DOCK.md').read_bytes()
for name in ('png-check-v18.json','changes-ui-v18.json'):
    files['documentation/'+name]=(work/name).read_bytes()
for name in ('native-ui-v18.json','hands-check-v17.json','components-check-v17.json','components-ui-v17.json',
             'picker-check-v17.json','boot-launch-ui-v17.json','ram-modes-v17.json','boot-check-v17.json'):
    files['documentation/'+name]=(work/name).read_bytes()

assert files['wallpapers/Nova-Carbon.bmp']==(project/'wallpapers/Nova-Carbon.bmp').read_bytes()
assert files[payload]==files['bootloader/update.bin']
assert not any(name.endswith(('nyx.ini','hekate_ipl.ini'))for name in files)
for p in (project/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'):assert files['fonts/'+p.name]==p.read_bytes()
for p in (work/'ram-ui').glob('ram-ui-*.bin'):assert files['bootloader/sys/'+p.name]==p.read_bytes()
manifest={'project':'Hekate Nova Pro','build_version':version,'base':'Hekate 6.5.4 / Nyx 1.9.4',
 'build_exit_code':0,'hardware_validation':'not performed','verification':verification,
 'animation':json.loads((work/'animation-v18.json').read_text()),
 'files':{name:{'bytes':len(data),'sha256':sha(data)}for name,data in sorted(files.items())}}
files['BUILD-INFO.json']=json.dumps(manifest,indent=2,ensure_ascii=False).encode('utf-8')
bundle=deliveries/'hekate-nova-pro-v18.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED)as z:
    for name,data in sorted(files.items()):z.writestr(name,data)

def include_source(p,root):
    relative=p.relative_to(root)
    if any(part in('build','build-ram','build-gui','output','.git','__pycache__')for part in relative.parts):return False
    if p.suffix.lower()in('.exe','.o','.d','.elf','.dll','.pyc'):return False
    if relative.as_posix()in('loader/payload_00.h','loader/payload_01.h'):return False
    return True
sources=deliveries/'hekate-nova-pro-v18-sources.zip'
with ZipFile(sources,'w',ZIP_DEFLATED)as z:
    for root in(project,companion):
        for p in sorted(root.rglob('*')):
            if p.is_file()and include_source(p,root):z.write(p,root.name+'/'+p.relative_to(root).as_posix())
    for p in sorted(work.glob('*.py')):
        if p.name.startswith(('check_','test_native_'))or p.name in(
            'run_build.py','build_ram.py','render_lvgl_preview_v18.py','verify_full_build_v18.py','package_nova_pro_v18.py',
            'create_localization_catalog.py','audit_localization_coverage.py','localize_nova_v17.py','localize_nova_v18.py'):
            z.write(p,'build-tools/'+p.name)
    for p in sorted((work/'ram-ui').glob('ram-ui-*.bin')):z.write(p,'resources/bootloader/sys/'+p.name)
    z.write(work/'ram-ui/manifest.json','resources/RAM-UI.json')
    z.write(work/'missing-v17.json','build-tools/missing-v17.json')
    z.write(deliveries/'Caviar.nyxf','resources/fonts/Caviar.nyxf')
    z.write(work/'native-ui-v18.json','NATIVE-UI-V18.json')
    z.write(work/'png-check-v18.json','verification/png-check-v18.json')
    z.write(work/'changes-ui-v18.json','verification/changes-ui-v18.json')
    z.write(deliveries/'DEVELOPPEMENT-TV-DOCK.md','DEVELOPPEMENT-TV-DOCK.md')
    for name in ('hands-check-v17.json','components-check-v17.json','components-ui-v17.json','picker-check-v17.json',
                 'boot-launch-ui-v17.json','ram-modes-v17.json','boot-check-v17.json','animation-v18.json','navigation-v18.json'):
        z.write(work/name,'verification/'+name)

    z.write(deliveries/'SUIVI-CAHIER-CHARGES-V18.md','SUIVI-CAHIER-CHARGES-V18.md')
    z.write(work/'verification-v18.json','VERIFICATION-V18.json')
    z.writestr('BUILDING.txt',
        'Hekate Nova Pro 18 : sources Hekate/Nyx et compagnon HWTest-RCM.\n'
        'Construire hekate-6.5.4 avec make -j4 : devkitARM, devkitA64, Python3\n'
        'et un compilateur GCC natif sont necessaires. Les icones sont deja generees.\n'
        'Le service RAM se construit dans HWTest-RCM-0.15/ramtest.\n'
        'Les scripts build-tools documentent la construction et les tests du\n'
        'workspace Windows : ils attendent work/hekate-6.5.4, work/HWTest-RCM-0.15,\n'
        'work/toolchains/w64devkit et les ressources de test de ce workspace.\n'
        'Le dossier resources contient les sept packs de langue du service RAM\n'
        'et le fichier Caviar utilise pour les verifications graphiques.\n'
        'La suite actuelle est verify_full_build_v18.py ; des tests historiques\n'
        'conservent le nom des versions precedentes.\n'
        'Aucun test PC ne remplace une validation sur console.\n')
for archive in(bundle,sources):
    with ZipFile(archive)as z:
        assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
        if archive==bundle:
            for name,data in files.items():assert z.read(name)==data
        else:
            assert z.read('hekate-6.5.4/nyx/nyx_gui/frontend/gui_partition_nova.inc')==(project/'nyx/nyx_gui/frontend/gui_partition_nova.inc').read_bytes()
            assert 'HWTest-RCM-0.15/third_party/Atmosphere/LICENSE'in z.namelist()
    print(f'PASS: {archive.name}, {archive.stat().st_size} bytes, SHA-256 {sha(archive.read_bytes())}')
(deliveries/'hekate-nova-pro-v18-SHA256.txt').write_text(''.join(f'{sha(p.read_bytes())}  {p.name}\n'for p in(bundle,sources)),encoding='ascii')
