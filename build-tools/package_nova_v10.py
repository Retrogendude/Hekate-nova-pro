from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
import hashlib
import json
import re

workspace = Path(__file__).resolve().parent.parent
work = workspace/'work'
project = work/'hekate-6.5.4'
companion = work/'HWTest-RCM-0.15'
output = project/'output'
deliveries = workspace/'outputs'
source_zip = deliveries/'hekate-6.5.4-nova-v10-sources.zip'
bundle_zip = deliveries/'hekate-6.5.4-nova-v10.zip'

def sha(data): return hashlib.sha256(data).hexdigest()

def source_file(p, root):
    rel = p.relative_to(root)
    if any(part in ('build', 'build-ram', 'build-gui', 'output', '.git', '__pycache__') for part in rel.parts): return False
    if p.suffix.lower() in ('.exe', '.o', '.d', '.elf', '.dll', '.pyc'): return False
    if rel.as_posix() in ('loader/payload_00.h', 'loader/payload_01.h'): return False
    return True

with ZipFile(source_zip,'w',ZIP_DEFLATED) as z:
    for root in (project, companion):
        for p in sorted(root.rglob('*')):
            if p.is_file() and source_file(p, root):
                z.write(p, root.name+'/'+p.relative_to(root).as_posix())
    for name in ('run_build.py', 'build_ram.py', 'verify_full_build.py', 'check_payload_reader.py',
                 'check_nyx_config_writer.py', 'check_ram_handoff.py', 'check_ram_payload.py',
                 'test_native_backend_host.py', 'test_native_verdict_host.py',
                 'check_language_config.py', 'check_language_actions.py',
                 'check_localization.py', 'check_unicode_fonts.py',
                 'check_appearance_config.py', 'check_appearance_actions.py',
                 'check_font_manager.py', 'check_gui_readability.py', 'check_nova_reference.py', 'check_nova_save.py', 'check_nova_guards.py',
                 'check_display_present.py', 'check_background_performance.py',
                 'render_lvgl_preview_v10.py', 'check_alternate_fonts.py', 'package_nova_v10.py', 'export_nova_previews.py', 'add_nova_catalog.py', 'build_nova_icons.cjs', 'build_nova_reference.cjs',
                 'check_diag_format.py', 'check_probe_translations.py', 'check_ram_locale.py', 'check_report_paging.py',
                 'create_localization_catalog.py', 'extend_localization_catalog.py',
                 'build_legacy_extra.py', 'audit_localization_coverage.py',
                 'build_ram_translations.py', 'build_ram_ui_packs.py',
                 'complete_probe_translations.py', 'polish_probe_translations.py',
                 'sanitize_probe_translations.py', 'build_probe_review.py'):
        z.write(work/name,'build-tools/'+name)
    z.write(work/'nova-reference.json','design/nova-reference.json')
    design=json.loads((work/'nova-reference.json').read_text(encoding='utf-8'))
    if Path(design['source']).is_file():z.write(Path(design['source']),'design/hekate-directions-pro.html')
    for path in sorted((work/'ram-ui').glob('ram-ui-*.bin')):
        z.write(path,'resources/bootloader/sys/'+path.name)
    z.write(work/'ram-ui/manifest.json','resources/RAM-UI.json')
    z.writestr('BUILDING.txt',
        'Sources Hekate/Nyx et compagnon HWTest-RCM avec dependances RAM incluses.\n'
        'Construire Hekate a sa racine avec make -j4 (devkitARM, devkitA64, Python3, GCC natif).\n'
        'Construire ensuite HWTest-RCM-0.15/ramtest avec make -j4 (devkitARM, Python3, Git).\n'
        'Les scripts build-tools sont les references de construction et verification\n'
        'du workspace Windows utilise ici ; ils attendent sa disposition work/.\n'
        'Les icones Nova sont deja compilees dans nova_icons.inc : Node.js est\n'
        'uniquement utile pour les regenerer, avec Lucide 1.8.0 et Sharp.\n'
        'Ne pas utiliser les sorties intermediaires *_unc.bin comme payloads.\n')

meta = json.loads((work/'official-release.json').read_text(encoding='utf-8'))
official = work/'hekate_ctcaer_6.5.4_Nyx_1.9.4.zip'
asset = next(a for a in meta['assets'] if a['name']==official.name)
assert 'sha256:'+sha(official.read_bytes())==asset['digest']
replacements = {'bootloader/sys/nyx.bin':'nyx.bin',
                'bootloader/sys/libsys_lp0.bso':'libsys_lp0.bso',
                'bootloader/sys/libsys_minerva.bso':'libsys_minerva.bso',
                'bootloader/update.bin':'hekate.bin'}
files = {}
with ZipFile(official) as z:
    assert z.testzip() is None
    for entry in z.infolist():
        if not entry.is_dir() and entry.filename.startswith('bootloader/'):
            files[entry.filename]=(output/replacements[entry.filename]).read_bytes() if entry.filename in replacements else z.read(entry.filename)
payload_name='hekate_ctcaer_6.5.4_custom.bin'
files[payload_name]=(output/'hekate.bin').read_bytes()
files['bootloader/sys/hwtest_ram.bin']=(companion/'build-ram/hwtest_ram.bin').read_bytes()
for path in sorted((work/'ram-ui').glob('ram-ui-*.bin')):
    files['bootloader/sys/'+path.name]=path.read_bytes()
for path in sorted((project/'nyx/nyx_gui/fonts/sd').glob('*.nyxf')):
    files['fonts/'+path.name]=path.read_bytes()
font_root=project/'nyx/nyx_gui/fonts'
for relative in ('tools/convert_font.py','tools/generate_unicode_fonts.py','source/NotoSans-Regular.ttf','source/OFL.txt'):
    files['outils-polices/'+relative]=(font_root/relative).read_bytes()
files['outils-polices/LIRE-MOI.md']=(font_root/'FONTS.md').read_bytes()
files['outils-polices/convertir-police.cmd']='''@echo off
if "%~1"=="" (
  echo Glisser un fichier TTF ou OTF sur ce programme.
  pause
  exit /b 1
)
python -c "import PIL" 2>nul
if errorlevel 1 (
  echo Python 3 et Pillow sont necessaires.
  echo Installer Pillow avec : python -m pip install Pillow
  pause
  exit /b 1
)
python "%~dp0tools\\convert_font.py" "%~1" -o "%~dpn1.nyxf"
if errorlevel 1 (
  echo Conversion impossible. Consulter le message ci-dessus.
  pause
  exit /b 1
)
echo Copier le fichier .nyxf dans fonts a la racine de la carte SD.
pause
'''.replace('\n','\r\n').encode('utf-8')
files['LICENSE']=(project/'LICENSE').read_bytes()
files['licenses/NotoSans-OFL.txt']=(project/'nyx/nyx_gui/fonts/source/OFL.txt').read_bytes()
files['licenses/Lucide-ISC.txt']=(project/'nyx/nyx_gui/frontend/nova_assets/LUCIDE-LICENSE.txt').read_bytes()
files['sources/'+source_zip.name]=source_zip.read_bytes()
for name, destination in [('hekate-build.log','verification/compilation-hekate-nyx.log'),
                          ('hwtest-ram-build.log','verification/compilation-ram.log'),
                          ('verification-full.txt','verification/tests.txt')]:
    content=(work/name).read_text(encoding='utf-8',errors='replace')
    files[destination]=re.sub(r'\x1b\[[0-9;]*[A-Za-z]','',content).encode('utf-8')
for name in ('INTERFACE-ACCUEIL.md','DIAGNOSTIC-INTEGRATION.md','BUILD-NOTES.md','PERSONNALISATION.md','TRADUCTIONS.md'):
    files['documentation/'+name]=(project/name).read_bytes()
files['documentation/RETOUR-RAM.md']=(companion/'ramtest/NYX-RETURN.md').read_bytes()
files['documentation/POLICES.md']=(font_root/'FONTS.md').read_bytes()
files['verification/couverture-traductions.json']=(work/'localization-coverage.json').read_bytes()
files['verification/provenance-traductions-techniques.json']=(project/'nyx/nyx_gui/localization_probe_provenance.json').read_bytes()
files['verification/performance-fonds.json']=(work/'background-performance-check/results.json').read_bytes()
files['verification/performance-halo-nova.json']=(work/'lvgl-preview-v10/nova-performance.json').read_bytes()
files['verification/reference-nova.json']=(work/'nova-reference-check.json').read_bytes()
files['verification/ressources-ram.json']=(work/'ram-ui/manifest.json').read_bytes()
for path in sorted(work.glob('preview-v10-*.png')):
    files['documentation/apercus/'+path.name]=path.read_bytes()

files['LIRE-MOI.txt']=(project/'LIRE-MOI-V10.txt').read_bytes()
verification=json.loads((work/'verification-full.json').read_text(encoding='utf-8'))
manifest={
    'project':'Hekate 6.5.4 / Nyx 1.9.4 custom Nova prototype 10',
    'build_date_local':'2026-11-06 Europe/Paris',
    'build_exit_code':0,
    'toolchain_arm':'devkitARM GCC 16.1.0',
    'toolchain_a64':'devkitA64 GCC 16.1.0',
    'toolchain_host':'w64devkit 2.11.0 / GCC 16.2.0 / GNU Make 4.4.1',
    'hardware_validation':'not performed',
    'translation_scope':'Seven-language GUI catalog and RAM UI; all 1268 native probe source literals cataloged. Additional report prose was translated offline and needs linguistic review. Early boot/recovery retains English.',
    'unicode_font':{'name':'Noto Sans Regular','license':'SIL Open Font License 1.1',
                    'source':'https://github.com/notofonts/noto-fonts',
                    'ttf_sha256':sha((project/'nyx/nyx_gui/fonts/source/NotoSans-Regular.ttf').read_bytes())},
    'verification':verification,
    'official_resources':{'release':meta['url'],'archive':official.name,'sha256':sha(official.read_bytes())},
    'ram_dependencies':json.loads((companion/'ramtest/DEPENDENCIES.json').read_text(encoding='utf-8')),
    'compiled_replacements':list(replacements)+[payload_name,'bootloader/sys/hwtest_ram.bin'],
    'files':{name:{'bytes':len(data),'sha256':sha(data)} for name,data in sorted(files.items())}
}
files['BUILD-INFO.json']=json.dumps(manifest,indent=2,ensure_ascii=False).encode('utf-8')
assert files['bootloader/update.bin']==files[payload_name]
assert len(files[payload_name])<=126296
assert len(files['bootloader/sys/hwtest_ram.bin'])<=130392
assert len(files['bootloader/sys/nyx.bin'])<=16*1024*1024
assert (output/'hekate_unc.bin').stat().st_size<=140288
assert len([p for p in files if p.startswith('fonts/') and p.endswith('.nyxf')])==3
assert len([p for p in files if p.startswith('bootloader/sys/ram-ui-') and p.endswith('.bin')])==7
ram_ui=json.loads((work/'ram-ui/manifest.json').read_text(encoding='utf-8'))
assert not ram_ui['missing_translations']
for name, data in ram_ui['files'].items():
    assert sha(files['bootloader/sys/'+name])==data['sha256']
with ZipFile(bundle_zip,'w',ZIP_DEFLATED) as z:
    for name,data in sorted(files.items()): z.writestr(name,data)
with ZipFile(bundle_zip) as z:
    assert z.testzip() is None
    assert len(z.namelist())==len(set(z.namelist()))
    assert not any(name.endswith(('nyx.ini','hekate_ipl.ini')) for name in z.namelist())
    for name,data in files.items(): assert z.read(name)==data
with ZipFile(source_zip) as z:
    assert z.testzip() is None
    assert 'HWTest-RCM-0.15/third_party/Atmosphere/LICENSE' in z.namelist()
    assert 'HWTest-RCM-0.15/ramtest/source/nyx_ram_handoff.h' in z.namelist()
    assert not any('/build-ram/' in p or '/build/' in p for p in z.namelist())
print('PASS: compiled package and corresponding sources verified.')
print(bundle_zip, bundle_zip.stat().st_size,'bytes')
print(source_zip, source_zip.stat().st_size,'bytes')
