"""Package the two-color Halo settings as a local console-test beta."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,os,subprocess
from PIL import Image,ImageDraw
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.3';OLD=W/'hekate-nova-pro-v33.2';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
selection=json.loads((W/'nova33_3-selection-check.json').read_text());ui=json.loads((W/'nova33_3-ui-check.json').read_text());header=json.loads((W/'nova33_3-header-check.json').read_text())
assert selection['passed']and ui['passed']and header['passed']
for n,h in selection['source_sha256'].items():assert sha((P/'nyx/nyx_gui'/n).read_bytes())==h,n
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
allowed={'nyx/nyx_gui/'+n for n in ['config.c','config.h','frontend/gui_nova_theme.c','frontend/gui_nova_reflection.c','frontend/gui_nova.h','nova_version.h','localization_catalog.json','localization_catalog.inc']}
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b};assert changed==allowed,changed
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
notes={'fr': ['Le choix Fond est remplacé par Halo dans Interface > Thèmes et couleurs > Couleurs.', 'Couleur 1 et Couleur 2 permettent de personnaliser indépendamment les deux couleurs du contour sélectionné. L’aperçu affiche le dégradé sans appliquer les changements avant sauvegarde.', 'Les deux valeurs sont conservées entre les onglets, enregistrées dans nyx.ini et utilisées après rechargement. Bleu/violet reste le réglage par défaut ; le fond existant est conservé.'], 'en': ['The Background color choice is replaced by Halo under Interface > Themes and colors > Colors.', 'Color 1 and Color 2 independently customize the selected outline gradient. The preview shows pending colors without applying them before saving.', 'Both values survive tab changes and are stored in nyx.ini for reload. Blue/violet remains the default; the existing background is preserved.'], 'es': ['La opción de color Fondo se sustituye por Halo en Interfaz > Temas y colores > Colores.', 'Color 1 y Color 2 personalizan por separado el degradado del contorno seleccionado, con vista previa antes de guardar.', 'Ambos valores se conservan entre pestañas y se guardan en nyx.ini. Azul/violeta por defecto; se conserva el fondo existente.'], 'it': ['La scelta colore Sfondo è sostituita da Alone in Interfaccia > Temi e colori > Colori.', 'Colore 1 e Colore 2 personalizzano indipendentemente il gradiente del contorno selezionato, con anteprima prima del salvataggio.', 'Entrambi i valori restano cambiando scheda e vengono salvati in nyx.ini. Blu/viola predefiniti; sfondo esistente preservato.'], 'ru': ['Выбор цвета фона заменён на Ореол в разделе Интерфейс > Темы и цвета > Цвета.', 'Цвет 1 и Цвет 2 независимо задают градиент рамки выбора с предпросмотром до сохранения.', 'Оба цвета сохраняются при смене вкладок и записываются в nyx.ini. По умолчанию синий/фиолетовый; существующий фон сохранён.'], 'de': ['Die Hintergrundfarbauswahl wird unter Oberfläche > Themen und Farben > Farben durch Halo ersetzt.', 'Farbe 1 und Farbe 2 passen den Verlauf des Auswahlrahmens unabhängig an; Vorschau vor dem Speichern.', 'Beide Werte bleiben beim Tabwechsel erhalten und werden in nyx.ini gespeichert. Standard blau/violett; vorhandener Hintergrund bleibt erhalten.'], 'pt': ['A escolha da cor Fundo é substituída por Halo em Interface > Temas e cores > Cores.', 'Cor 1 e Cor 2 personalizam separadamente o degradê do contorno selecionado, com pré-visualização antes de guardar.', 'Ambos os valores mantêm-se entre separadores e são guardados em nyx.ini. Azul/violeta por predefinição; fundo existente preservado.'], 'zh-CN': ['在界面 > 主题与颜色 > 颜色中，将背景颜色选项替换为选中边框。', '颜色 1 与颜色 2 可独立设置选择边框渐变，并在保存前预览效果。', '切换标签页时保留两种颜色，并保存到 nyx.ini。默认仍为蓝色与紫色，现有背景保持不变。']}
for code,items in notes.items():
 text='# Hekate Nova Pro v33.3 beta\n\n'+'\n'.join('- '+x for x in items)+'\n\nPC: 98 configuration checks + 24 Halo scenes + 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.2.\n'
 (O/('CHANGELOG-NOVA33.3-'+code+'.md')).write_text(text,encoding='utf-8')
readme='''Hekate Nova Pro v33.3 — bêta de test

Halo à deux couleurs : Interface > Thèmes et couleurs > Couleurs > Halo remplace l’ancien choix Fond. Choisir Couleur 1 puis régler la palette ; choisir Couleur 2 et régler la seconde couleur. L’aperçu montre immédiatement le dégradé. Les deux choix restent en attente lors du changement d’onglet. Enregistrer et recharger applique et conserve le résultat ; fermer sans sauvegarder abandonne les modifications.

Le bleu/violet d’origine reste le défaut. Les nouveaux paramètres nova_halo_color1 et nova_halo_color2 sont enregistrés dans nyx.ini ; les anciennes configurations sans ces clés restent compatibles. Le fond existant reste conservé. Réinitialiser rétablit les couleurs Nova par défaut.

Installer ensemble le payload et le dossier bootloader fournis. Conserver vos fichiers de configuration et appairages personnels. Utiliser le pack standard sur une console normale ; le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental, comme précédemment. Les corrections de contours de la v33.2 sont incluses.

Contrôles PC : 98 cas de configuration réelle en C, 24 scènes Halo dans les huit langues, 192 scènes de sélection et 683 scènes de régression. Compilation ARM et en-têtes vérifiés. Essai réel sur console encore nécessaire.
'''
(O/'LIRE-MOI-NOVA33.3.txt').write_text(readme,encoding='utf-8')
info={'version':'33.3-beta','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','changes':sorted(changed),'selection_cases':192,'regression_scenes':683,'hardware_test':False,'ram_payload_unchanged':True,'fonts_and_bluetooth_unchanged':True,'new_halo_labels_localized':True,'halo_cases':24,'config_cases':98,'sources':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova33_3.json').write_text(json.dumps(info,indent=2))
summary={}
def archive(path,files):
 assert path.parent==O and path.name.startswith('hekate-nova-pro-v33.3-beta'),path
 with ZipFile(path,'w',ZIP_DEFLATED,compresslevel=6)as z:
  for n,b in sorted(files.items()):z.writestr(n,b)
 with ZipFile(path)as z:
  assert not z.testzip()and len(z.namelist())==len(set(z.namelist()))
  for n,b in files.items():assert z.read(n)==b
 summary[path.name]={'bytes':path.stat().st_size,'sha256':sha(path.read_bytes())}
for suffix in ('','-8GB'):
 with ZipFile(O/('hekate-nova-pro-v33.2-beta'+suffix+'.zip'))as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate_nova_pro_')and n not in ('BUNDLE-MANIFEST.json','BUILD-INFO.json')}
 for n in ['nyx.bin','libsys_lp0.bso','libsys_minerva.bso']:files['bootloader/sys/'+n]=(P/'output'/n).read_bytes()
 payload=bytearray((P/'output/hekate.bin').read_bytes());assert not payload[0x122]&1
 if suffix:payload[0x122]|=1
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v33.3_beta'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/LIRE-MOI-NOVA33.3.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA33.3-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in ['verification-nova33_3.json','nova33_3-selection-check.json','nova33_3-ui-check.json','nova33_3-header-check.json','nova33_3-halo-check.json','nova33_3-config-check.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='sources'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak')for n in files)
 archive(O/('hekate-nova-pro-v33.3-beta'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.2-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.2/')}
files.update({'hekate-nova-pro-v33.3/'+n:b for n,b in source.items()})
for n in ['run_build_nova33_3.py','render_lvgl_preview_nova33_3.py','check_nova33_3_selection.py','check_nova33_3_ui.py','check_nova33_3_header.py','package_nova33_3.py','check_nova33_3_config.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in ['verification-nova33_3.json','nova33_3-selection-check.json','nova33_3-ui-check.json','nova33_3-header-check.json','nova33_3-halo-check.json','nova33_3-config-check.json','nova33_3-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V33.3.txt']=b'Use the v33.2 build requirements and unchanged HWTest-RCM-nova33_1 dependency. Restore hekate-nova-pro-v33.3 under work and use run_build_nova33_3.py. RAM, fonts, Bluetooth and storage code are unchanged. The Halo controls add two RGB settings and localized labels. The 8GB variant differs by bit 0 at payload offset 0x122. Native UI validation needs the official res.pak ZIP and test-sd-nova33_3 fixture, as described in the preceding BUILDING guide.\n'
archive(O/'hekate-nova-pro-v33.3-beta-sources.zip',files)
(O/'hekate-nova-pro-v33.3-beta-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova33_3-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False},indent=2))
# Real-code previews: final selected card plus active option, without hardware actions.
D=W/'lvgl-preview-nova33_3';F=W/'test-sd-nova33_3/nyx/nyx_gui/fonts/sd';preview=O/'preview-nova33.3';preview.mkdir(exist_ok=True)
env=os.environ.copy();env.update(NYX_TEST_SELECTION='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp');env.pop('NYX_PREVIEW_NO_DUMP',None)
for scene in ['home','emummc','backup','restore','usb','colors']:
 if scene=='colors':env['NYX_TEST_HALO']='1';env.pop('NYX_TEST_SELECTION',None)
 r=subprocess.run([str(D/'preview.exe'),str(F),str(D/'res.pak'),scene,'1','30','1'],cwd=D,env=env,capture_output=True,text=True,timeout=40);assert r.returncode==0,r.stderr
 Image.open(D/'preview.ppm').save(preview/(scene+'.png'))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
