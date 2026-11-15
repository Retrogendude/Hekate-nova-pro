"""Build distinct standard/8GB beta bundles and a source archive, without touching SD."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.1';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
info=json.loads((W/'verification-nova33_1.json').read_text(encoding='utf-8'));assert info['passed']and info['version']=='33.1-beta'
for n,h in info['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
for n,v in info['binaries'].items():assert sha((P/'output'/n).read_bytes())==v['sha256'],n
ram=W/'HWTest-RCM-nova33_1/build-ram/hwtest_ram.bin';assert sha(ram.read_bytes())==info['ram_payload']['sha256']
with ZipFile(O/'hekate-nova-pro-v33-beta.zip')as z:
 assert z.testzip()is None
 files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and n not in ('BUILD-INFO.json','BUNDLE-MANIFEST.json','LIRE-MOI.txt')and not n.startswith(('hekate_nova_pro_','verification/'))}
for n in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+n]=(P/'output'/n).read_bytes()
files['bootloader/sys/hwtest_ram.bin']=ram.read_bytes()
files['bootloader/sys/Nova-Chinese-Simplified.nyxf']=(P/'nyx/nyx_gui/fonts/sd/Nova-Chinese-Simplified.nyxf').read_bytes()
for p in (W/'nova33_1-ram-ui').glob('*.bin'):files['bootloader/sys/'+p.name]=p.read_bytes()
files['LIRE-MOI.txt']=(P/'LIRE-MOI-V33.1.md').read_bytes()
for name in ('LIRE-MOI-V33.1.md','RAM-8GO-V33.1.md'):files['documentation/'+name]=(P/name).read_bytes()
files['documentation/LIRE-MOI-V33.md']=(P/'LIRE-MOI-V33.md').read_bytes()
for p in O.glob('CHANGELOG-NOVA33.1-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
lang=json.loads(files['documentation/LANGUES-V33.json']);lang['github_publication']='Authorized v33.1 beta publication';lang['chinese_glyphs']=info['chinese_font_glyphs'];files['documentation/LANGUES-V33.json']=json.dumps(lang,ensure_ascii=False,indent=2).encode()
evidence=['verification-nova33_1.json','nova33_1-ui-check.json','nova33_1-font-check.json','nova33_1-header-check.json','nova33_1-chinese-font.json','nova33_1-ram-ui.json','nova33_1-8gb-check.json']
for name in evidence:files['verification/'+name]=(W/name).read_bytes()
standard=(P/'output/hekate.bin').read_bytes();assert standard[0x118:0x11f]==b'ICTC655'and not standard[0x122]&1
eight=bytearray(standard);eight[0x122]|=1;eight=bytes(eight)
assert [i for i,(a,b)in enumerate(zip(standard,eight))if a!=b]==[0x122]
assert files['bootloader/sys/nyx.bin'][0x98:0x9f]==b'GCTC195'
assert not any(Path(n).name.lower()in {'nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','joycon-pairing.tmp','last-session.txt','prod.keys','title.keys'}for n in files)
summary={}
def archive(path,values):
 assert not path.exists(),str(path)
 with ZipFile(path,'w',ZIP_DEFLATED,compresslevel=6)as z:
  for n,b in sorted(values.items()):z.writestr(n,b)
 with ZipFile(path)as z:
  assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
  for n,b in values.items():assert z.read(n)==b,n
 summary[path.name]={'bytes':path.stat().st_size,'sha256':sha(path.read_bytes())}
for variant,payload in [('standard',standard),('8GB-experimental',eight)]:
 values=dict(files);suffix='_8GB_EXPERIMENTAL'if variant!='standard'else''
 name='hekate_nova_pro_6.5.5_v33.1_beta'+suffix+'.bin'
 values[name]=values['bootloader/update.bin']=payload
 values['BUILD-INFO.json']=json.dumps({**{k:v for k,v in info.items()if k!='source_sha256'},'variant':variant,'payload_sha256':sha(payload),'dram_override_8gb':variant!='standard','hardware_validated_8gb':False},ensure_ascii=False,indent=2).encode()
 if variant!='standard':values['LIRE-MOI.txt']=(P/'RAM-8GO-V33.1.md').read_bytes()+b'\n\n'+values['LIRE-MOI.txt']
 values['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in sorted(values.items())},indent=2).encode()
 archive(O/('hekate-nova-pro-v33.1-beta'+('-8GB'if variant!='standard'else'')+'.zip'),values)
sources={'hekate-nova-pro-v33.1/'+n:(P/n).read_bytes()for n in info['source_sha256']}
for p in (W/'HWTest-RCM-nova33_1').rglob('*'):
 if p.is_file()and not any(c in ('build-ram','.git','__pycache__')for c in p.relative_to(W/'HWTest-RCM-nova33_1').parts):sources['HWTest-RCM-nova33_1/'+p.relative_to(W/'HWTest-RCM-nova33_1').as_posix()]=p.read_bytes()
with ZipFile(O/'hekate-nova-pro-v33-beta-sources.zip')as old:
 for n in old.namelist():
  if not n.endswith('/')and n.startswith(('HWTest-RCM-0.15/','reference/','resources/','assets/')):sources[n]=old.read(n)
tools=['run_build_nova33_1.py','build_ram_nova33_1.py','render_lvgl_preview_nova33_1.py','render_nova33_1_previews.py','check_nova33_1_ui.py','check_nova33_1_fonts.py','check_nova33_1_ram_locale.py','check_nova33_1_ram8.py','check_nova33_1_ram_payload.py','check_nova33_1_header.py','verify_nova33_1.py','package_nova33_1.py','prepare_nova33_1_docs.py','finalize_nova33_1_catalog.py','build_nova33_1_chinese_font.py','nova33_1-zh-characters.json','nova33_1_release_notes.json','check_font_manager.py','audit_localization_coverage.py']
for n in tools:sources['build-tools/'+n]=(W/n).read_bytes()
for name in ['NotoSansCJKsc-Regular.otf','OFL.txt']:sources['build-tools/nova33-language-resources/'+name]=(W/'nova33-language-resources'/name).read_bytes()
for name in evidence+['nova33_1-build.log','nova33_1-ram-build.log']:sources['verification/'+name]=(W/name).read_bytes()
sources['BUILDING-V33.1.txt']='''Build sources: restore the Hekate project, HWTest-RCM-nova33_1 and HWTest-RCM-0.15 under one work directory. Put build-tools contents in work. Requirements: devkitARM, GNU make, Python/Pillow, native GCC for PC checks. Windows paths are recorded in the scripts; adjust for your installation. No translation runtime is needed to compile.
run_build_nova33_1.py builds Hekate/Nyx. build_ram_nova33_1.py builds the separate RAM service using the public dependencies in HWTest-RCM-0.15. The 8GB payload differs solely by rcfg bit 0 at offset 0x122, as in official Hekate; keep standard bit clear.
The catalog and Chinese NYXF are generated and included. To rebuild the CJK font run build_nova33_1_chinese_font.py with Pillow and build-tools/nova33-language-resources restored next to it. finalize_nova33_1_catalog.py updates v33.1 messages and all RAM UI packs; it expects the prior v33 RAM packs and release-note JSON from the previous source/bundle generation. These final generated packs are available in both installation ZIPs.
Native UI checks require the official Hekate 6.5.4 resource ZIP and host GCC. Prepare test-sd-nova33_1/nyx/nyx_gui/fonts/sd from project fonts/sd, copy assets/icone and assets/wallpapers to test-sd-nova33_1, and put the Chinese font under its bootloader/sys. The renderer checks the actual C menu code while substituting hardware I/O; it does not establish console operation. The header negative test uses a v21 fixture available in the earlier source history. verify_nova33_1.py compares protected code with v32 from the repository history. Package assembly expects the previous v33 beta ZIP, outputs and PC evidence; those are release-tool prerequisites, not console build dependencies.
The standalone RAM binary and expanded memory mode require real hardware tests. Read RAM-8GO-V33.1.md; do not assert universal chip compatibility.
'''.encode()
archive(O/'hekate-nova-pro-v33.1-beta-sources.zip',sources)
checks=''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items());(O/'hekate-nova-pro-v33.1-beta-SHA256.txt').write_text(checks,encoding='utf-8')
(W/'nova33_1-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'configs_preserved':True,'8gb_only_difference':'payload byte 0x122 bit 0','not_yet_published':True},indent=2))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
