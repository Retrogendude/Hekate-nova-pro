"""Package the seven outline fixes as a local console-test beta."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,os,subprocess
from PIL import Image,ImageDraw
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.2';OLD=W/'hekate-nova-pro-v33.1';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
selection=json.loads((W/'nova33_2-selection-check.json').read_text());ui=json.loads((W/'nova33_2-ui-check.json').read_text());header=json.loads((W/'nova33_2-header-check.json').read_text())
assert selection['passed']and ui['passed']and header['passed']
for n,h in selection['source_sha256'].items():assert sha((P/'nyx/nyx_gui'/n).read_bytes())==h,n
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
allowed={'nyx/nyx_gui/frontend/'+n for n in ['gui_nova.c','gui_nova_reflection.c','gui_nova.h','gui_tools.c','gui_emmc_tools.c','gui.c']}|{'nyx/nyx_gui/nova_version.h'}
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b};assert changed==allowed,changed
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
notes={
'fr':('Contours lumineux de sélection sur toutes les tuiles de l’accueil, les trois actions Gestion emuMMC, les quatre actions Sauvegarde, les trois actions Restauration et les huit actions Outils USB.','Le contour suit le curseur Joy-Con ou la sélection tactile ; les boutons conservent leurs actions et les cases désactivées perdent leur contour.','Contour extérieur complet pour Partition SD emuMMC brute et Lecture seule lorsqu’ils sont activés, y compris le bord inférieur auparavant incomplet. Les contours d’état restent visibles sans animations.'),
'en':('Glowing selection outlines on all home tiles, three emuMMC management actions, four Backup actions, three Restore actions and eight USB Tools actions.','Outlines follow the Joy-Con pointer or touch selection; button actions are preserved and disabled cards lose their outline.','Complete outer outlines for enabled raw SD emuMMC and Read-only options, including the previously incomplete bottom edge. State outlines remain visible without animations.'),
'es':('Contornos luminosos en todas las baldosas de inicio, tres acciones de gestión emuMMC, cuatro de copia, tres de restauración y ocho de herramientas USB.','El contorno sigue el cursor Joy-Con o la selección táctil; se conservan las acciones y se retira el contorno de las casillas desactivadas.','Contorno exterior completo al activar la partición SD emuMMC o Solo lectura, incluido el borde inferior antes incompleto. Visible incluso sin animaciones.'),
'it':('Contorni luminosi su tutti i riquadri iniziali, tre azioni di gestione emuMMC, quattro di backup, tre di ripristino e otto degli strumenti USB.','Il contorno segue il puntatore Joy-Con o la selezione tattile; azioni preservate e contorno rimosso dai riquadri disabilitati.','Contorno esterno completo per partizione SD emuMMC e Sola lettura attive, incluso il bordo inferiore prima incompleto. Visibile anche senza animazioni.'),
'ru':('Светящиеся рамки всех плиток главного экрана, трёх действий управления emuMMC, четырёх действий копирования, трёх восстановления и восьми инструментов USB.','Рамка следует за указателем Joy-Con или сенсорным выбором; действия кнопок сохранены, рамка отключённых плиток убирается.','Полный внешний контур активных параметров раздела SD emuMMC и Только чтение, включая ранее неполный нижний край. Контуры состояния видны без анимации.'),
'de':('Leuchtende Auswahlrahmen für alle Startkacheln, drei emuMMC-Verwaltungsaktionen, vier Sicherungsaktionen, drei Wiederherstellungsaktionen und acht USB-Werkzeuge.','Rahmen folgen dem Joy-Con-Zeiger oder der Touch-Auswahl; Tastenaktionen bleiben erhalten, deaktivierte Kacheln verlieren den Rahmen.','Vollständiger Außenrahmen für aktivierte rohe SD-emuMMC-Partition und Schreibschutz, einschließlich der zuvor unvollständigen Unterkante. Statusrahmen bleiben ohne Animation sichtbar.'),
'pt':('Contornos luminosos em todos os blocos iniciais, três ações de gestão emuMMC, quatro de cópia, três de restauro e oito de ferramentas USB.','O contorno acompanha o cursor Joy-Con ou a seleção tátil; ações preservadas e contorno removido dos blocos desativados.','Contorno exterior completo para partição SD emuMMC e Apenas leitura ativas, incluindo a margem inferior antes incompleta. Visível mesmo sem animações.'),
'zh-CN':('为所有主页磁贴、三项 emuMMC 管理操作、四项备份操作、三项恢复操作和八项 USB 工具添加发光选择边框。','边框跟随 Joy-Con 指针或触摸选择；保留按钮原有操作，禁用的磁贴不再显示选择边框。','启用 SD emuMMC 原始分区或只读选项时显示完整外边框，修复底边发光部分缺失；关闭动画后状态边框仍可见。')}
for code,items in notes.items():
 text='# Hekate Nova Pro v33.2 beta\n\n'+'\n'.join('- '+x for x in items)+'\n\nPC: 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.1.\n'
 (O/('CHANGELOG-NOVA33.2-'+code+'.md')).write_text(text,encoding='utf-8')
readme='''Hekate Nova Pro v33.2 — bêta de test

Cette version corrige les sept points visuels signalés sur la v33.1 : contours de sélection dans Accueil, Gestion emuMMC, Sauvegarde, Restauration et Outils USB ; contours complets des options actives ; bord lumineux inférieur du bloc SD emuMMC brute.

Installer ensemble le nouveau payload et le dossier bootloader fournis, en conservant vos fichiers nyx.ini, hekate_ipl.ini et vos appairages personnels. Les packs ne contiennent pas ces fichiers personnels. Les autres ressources et les huit langues restent celles de v33.1.

Le pack standard convient aux consoles 4 Go. Le pack 8GB est uniquement destiné aux consoles physiquement modifiées et reste expérimental ; consulter documentation/RAM-8GO-V33.1.md. Le test mémoire est inchangé.

Essayer le déplacement du curseur Joy-Con et le tactile sur chacune des cases, puis activer et désactiver les options SD emuMMC brute et Lecture seule. Vérifier que le contour suit la sélection et reste complet sur les quatre côtés. Les cases désactivées ne doivent pas être illuminées. Les contours d’état restent visibles lorsque les effets animés sont désactivés.

Compilation et contrôles PC réussis : 192 cas de sélection et contours, 683 scènes de régression, protections des en-têtes binaires. Le rendu et la réactivité restent à confirmer sur console. Voir les aperçus issus du code réel dans preview-nova33.2.
'''
(O/'LIRE-MOI-NOVA33.2.txt').write_text(readme,encoding='utf-8')
info={'version':'33.2-beta','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','changes':sorted(changed),'selection_cases':192,'regression_scenes':683,'hardware_test':False,'ram_payload_unchanged':True,'languages_fonts_and_bluetooth_unchanged':True,'sources':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova33_2.json').write_text(json.dumps(info,indent=2))
summary={}
def archive(path,files):
 assert not path.exists(),path
 with ZipFile(path,'w',ZIP_DEFLATED,compresslevel=6)as z:
  for n,b in sorted(files.items()):z.writestr(n,b)
 with ZipFile(path)as z:
  assert not z.testzip()and len(z.namelist())==len(set(z.namelist()))
  for n,b in files.items():assert z.read(n)==b
 summary[path.name]={'bytes':path.stat().st_size,'sha256':sha(path.read_bytes())}
for suffix in ('','-8GB'):
 with ZipFile(O/('hekate-nova-pro-v33.1-beta'+suffix+'.zip'))as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate_nova_pro_')and n not in ('BUNDLE-MANIFEST.json','BUILD-INFO.json')}
 for n in ['nyx.bin','libsys_lp0.bso','libsys_minerva.bso']:files['bootloader/sys/'+n]=(P/'output'/n).read_bytes()
 payload=bytearray((P/'output/hekate.bin').read_bytes());assert not payload[0x122]&1
 if suffix:payload[0x122]|=1
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v33.2_beta'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/LIRE-MOI-NOVA33.2.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA33.2-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in ['verification-nova33_2.json','nova33_2-selection-check.json','nova33_2-ui-check.json','nova33_2-header-check.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='sources'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak')for n in files)
 archive(O/('hekate-nova-pro-v33.2-beta'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.1-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.1/')}
files.update({'hekate-nova-pro-v33.2/'+n:b for n,b in source.items()})
for n in ['run_build_nova33_2.py','render_lvgl_preview_nova33_2.py','check_nova33_2_selection.py','check_nova33_2_ui.py','check_nova33_2_header.py','package_nova33_2.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in ['verification-nova33_2.json','nova33_2-selection-check.json','nova33_2-ui-check.json','nova33_2-header-check.json','nova33_2-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V33.2.txt']=b'Use the v33.1 build requirements and unchanged HWTest-RCM-nova33_1 dependency. Restore hekate-nova-pro-v33.2 under work and use run_build_nova33_2.py. RAM, languages, fonts, Bluetooth and storage code are unchanged. The 8GB variant differs by bit 0 at payload offset 0x122. Native UI validation needs the official res.pak ZIP and test-sd-nova33_2 fixture, as described in the preceding BUILDING guide.\n'
archive(O/'hekate-nova-pro-v33.2-beta-sources.zip',files)
(O/'hekate-nova-pro-v33.2-beta-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova33_2-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False},indent=2))
# Real-code previews: final selected card plus active option, without hardware actions.
D=W/'lvgl-preview-nova33_2';F=W/'test-sd-nova33_2/nyx/nyx_gui/fonts/sd';preview=O/'preview-nova33.2';preview.mkdir(exist_ok=True)
env=os.environ.copy();env.update(NYX_TEST_SELECTION='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp');env.pop('NYX_PREVIEW_NO_DUMP',None)
for scene in ['home','emummc','backup','restore','usb']:
 r=subprocess.run([str(D/'preview.exe'),str(F),str(D/'res.pak'),scene,'1','30','1'],cwd=D,env=env,capture_output=True,text=True,timeout=40);assert r.returncode==0,r.stderr
 Image.open(D/'preview.ppm').save(preview/(scene+'.png'))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
