"""Assemble a test bundle and reproducible sources; do not publish or touch SD."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33';OUT=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((W/'verification-nova33.json').read_text(encoding='utf-8'));assert info['passed']and info['version']=='33-beta'
for name,h in info['source_sha256'].items():assert sha((P/name).read_bytes())==h,name
for name,v in info['binaries'].items():assert sha((P/'output'/name).read_bytes())==v['sha256'],name
assert sha((W/'HWTest-RCM-nova33/build-ram/hwtest_ram.bin').read_bytes())==info['ram_payload']['sha256']
with ZipFile(OUT/'hekate-nova-pro-v32.zip')as z:
 assert z.testzip()is None
 files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and n not in ('BUILD-INFO.json','BUNDLE-MANIFEST.json','LIRE-MOI.txt')and not n.startswith('hekate_nova_pro_')}
for name in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+name]=(P/'output'/name).read_bytes()
files['bootloader/update.bin']=(P/'output/hekate.bin').read_bytes()
files['hekate_nova_pro_6.5.5_v33_beta.bin']=(P/'output/hekate.bin').read_bytes()
files['bootloader/sys/hwtest_ram.bin']=(W/'HWTest-RCM-nova33/build-ram/hwtest_ram.bin').read_bytes()
files['bootloader/sys/Nova-Chinese-Simplified.nyxf']=(P/'nyx/nyx_gui/fonts/sd/Nova-Chinese-Simplified.nyxf').read_bytes()
for p in (W/'nova33-ram-ui').glob('ram-ui-*.bin'):files['bootloader/sys/'+p.name]=p.read_bytes()
for folder in ('wallpapers','icone'):
 for p in (OUT/'nova-pack-visuel-01'/folder).iterdir():
  if p.is_file():files[folder+'/'+p.name]=p.read_bytes()
files['icone/Launcher.png']=(W/'test-sd-nova33/icone/Launcher.png').read_bytes()
files['LIRE-MOI.txt']=(P/'LIRE-MOI-V33.md').read_bytes()
files['documentation/LIRE-MOI-V33.md']=(P/'LIRE-MOI-V33.md').read_bytes()
notes=json.loads((W/'nova33_release_notes.json').read_text(encoding='utf-8'));assert set(notes)=={'en','fr','es','it','ru','de','pt','zh-CN'}
for code,value in notes.items():
 assert len(value['items'])==9
 text='# '+value['title']+'\n\n'+'\n'.join('- '+s for s in value['items'])+'\n\n'+value['validation']+'\n'
 files['documentation/changelogs/CHANGELOG-V33-'+code+'.md']=text.encode('utf-8')
 (OUT/('CHANGELOG-NOVA33-'+code+'.md')).write_text(text,encoding='utf-8')
for file in ['verification-nova33.json','nova33-ui-check.json','nova33-font-check.json','nova33-header-check.json','nova33-chinese-font.json','nova33-ram-ui.json']:
 files['verification/'+file]=(W/file).read_bytes()
files['documentation/Noto-CJK-OFL.txt']=(W/'nova33-language-resources/OFL.txt').read_bytes()
files['documentation/LUCIDE-LICENSE.txt']=(OUT/'nova-pack-visuel-01/documentation/LUCIDE-LICENSE.txt').read_bytes()
files['documentation/PACK-VISUEL-LIRE-MOI.md']=(OUT/'nova-pack-visuel-01/LIRE-MOI.md').read_bytes()
files['documentation/LANGUES-V33.json']=json.dumps({'languages':['en','fr','es','it','ru','de','pt','zh-CN'],'chinese_font_source':'https://github.com/notofonts/noto-cjk','translation_model':'https://data.argosopentech.com/argospm/v1/translate-en_zh-1_9.argosmodel','method':'Offline machine translation with authored primary labels and RAM UI; extended diagnostic prose needs linguistic review','chinese_glyphs':info['chinese_font_glyphs'],'custom_font_policy':'Chinese-only font lock; saved personal font preserved','hardware_validation':False,'github_publication':'Awaiting user console test'},ensure_ascii=False,indent=2).encode()
for p in (P/'nyx/nyx_gui/fonts/sd').glob('Lecture*.nyxf'):files['fonts/'+p.name]=p.read_bytes()
files['fonts/LIRE-MOI-V33.txt']='Police Hekate d’origine : choix intégré au menu, sans fichier supplémentaire.\nLe choix Par défaut utilise Lecture pour les langues latines, la famille intégrée pour le russe et la ressource dédiée pour le chinois.\nLa police chinoise doit rester dans bootloader/sys ; elle ne se personnalise pas.\n'.encode()
files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'},ensure_ascii=False,indent=2).encode()
assert not any(n.endswith(('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','joycon-pairing.tmp'))for n in files)
assert len([n for n in files if n.startswith('bootloader/sys/ram-ui-')and n.endswith('.bin')])==8
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert files['bootloader/update.bin'][0x118:0x11f]==b'ICTC655'
files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in sorted(files.items())},indent=2).encode()
bundle=OUT/'hekate-nova-pro-v33-beta.zip';assert not bundle.exists()
with ZipFile(bundle,'w',ZIP_DEFLATED,compresslevel=6)as z:
 for n,b in sorted(files.items()):z.writestr(n,b)
with ZipFile(bundle)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,b in files.items():assert z.read(n)==b,n

source=OUT/'hekate-nova-pro-v33-beta-sources.zip';assert not source.exists()
tools=['run_build_nova33.py','prepare_nova33_test_sd.py','render_lvgl_preview_nova33.py','check_nova33_ui.py','check_nova33_fonts.py','check_nova33_ram_locale.py','check_nova33_header.py','verify_nova33.py','package_nova33.py','finalize_nova33_catalog.py','build_nova33_chinese_font.py','build_nova33_ram_ui.py','build_ram_nova33.py','translate_nova33_zh.py','nova33_ram_zh.json','nova33_release_notes.json','check_font_manager.py','audit_localization_coverage.py','nova33_boot_selector.c.part']
with ZipFile(source,'w',ZIP_DEFLATED,compresslevel=6)as z:
 for n in sorted(info['source_sha256']):z.write(P/n,'hekate-nova-pro-v33/'+n)
 for n in tools:z.write(W/n,'build-tools/'+n)
 for n in ['verification-nova33.json','nova33-ui-check.json','nova33-font-check.json','nova33-header-check.json','nova33-chinese-font.json','nova33-ram-ui.json','nova33-build.log','nova33-ram-build.log']:
  z.write(W/n,'verification/'+n)
 for name in ('NotoSansCJKsc-Regular.otf','OFL.txt'):
  z.write(W/'nova33-language-resources'/name,'build-tools/nova33-language-resources/'+name)
 for p in (W/'HWTest-RCM-nova33').rglob('*'):
  if p.is_file()and not any(c in ('build-ram','.git','__pycache__')for c in p.relative_to(W/'HWTest-RCM-nova33').parts):z.write(p,'HWTest-RCM-nova33/'+p.relative_to(W/'HWTest-RCM-nova33').as_posix())
 with ZipFile(OUT/'hekate-nova-pro-v32-sources.zip')as old:
  for n in old.namelist():
   if not n.endswith('/')and n.startswith(('HWTest-RCM-0.15/','reference/','resources/')):z.writestr(n,old.read(n))
 for n in files:
  if n.startswith(('icone/','wallpapers/')):z.writestr('assets/'+n,files[n])
 z.writestr('BUILDING-V33.txt',
  'Hekate Nova Pro v33 beta: devkitARM, GNU make and Python/Pillow.\n'
  'Restore hekate-nova-pro-v33, HWTest-RCM-nova33, HWTest-RCM-0.15, assets and build-tools contents under a common work directory.\n'
  'The Windows scripts expect their own directory to be work/. Adjust the local toolchain paths for another computer.\n'
  'Generated Chinese translations, reviewed overrides and NYXF files are included; no machine-translation runtime is needed to compile.\n'
  'To rebuild the CJK font, restore build-tools/nova33-language-resources next to build_nova33_chinese_font.py, then run finalize_nova33_catalog.py and build_nova33_chinese_font.py.\n'
  'The font builder requires Pillow. Regenerating draft MT additionally needs Argos English-Chinese 1.9, CTranslate2 and SentencePiece; these are build-time tools, not console dependencies.\n'
  'run_build_nova33.py builds Hekate/Nyx; build_ram_nova33.py builds the separate RAM service without modifying its training/test logic.\n'
  'The RAM recipe uses the previous HWTest third_party dependencies supplied in HWTest-RCM-0.15.\n'
  'Run prepare_nova33_test_sd.py to prepare isolated host-test assets. Native UI checks need the public official 6.5.4 release ZIP for res.pak and the native GCC described in the renderer.\n'
  'check_nova33_fonts.py and check_nova33_ram_locale.py compile and exercise the real C readers.\n'
  'verify_nova33.py compares against the preceding public v32 source tree; the header negative test uses the v21 nyx.bin fixture available in the v32 source archive.\n'
  'The package script also expects the preceding v32 bundle, source archive and visual collection. Read LIRE-MOI-V33.md and all licenses.\n'
  'PC tests do not establish console hardware operation or translation quality. Publication is held for user console testing.\n')
with ZipFile(source)as z:
 assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
 for n,h in info['source_sha256'].items():assert sha(z.read('hekate-nova-pro-v33/'+n))==h,n
summary={}
for p in (bundle,source):summary[p.name]={'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())};print(p.name,p.stat().st_size,summary[p.name]['sha256'])
(OUT/'hekate-nova-pro-v33-beta-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()),encoding='utf-8')
(W/'nova33-package-check.json').write_text(json.dumps({'passed':True,'files':len(files),'archives':summary,'not_published':True,'configurations_not_overwritten':True},indent=2))
