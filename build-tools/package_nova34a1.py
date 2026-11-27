"""Local first installation-assistant draft; never writes to a real SD card."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha1';OLD=W/'hekate-nova-pro-v33.3';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
reports=['nova34a1-plan-check.json','nova34a1-wizard-check.json','nova34a1-ui-check.json','nova34a1-header-check.json','nova34a1-chinese-font.json']
for n in reports[:-1]:assert json.loads((W/n).read_text())['passed'],n
wizard=json.loads((W/'nova34a1-wizard-check.json').read_text())
for n,h in wizard['source_sha256'].items():assert sha((P/'nyx/nyx_gui/frontend'/n).read_bytes())==h,n
ui=json.loads((W/'nova34a1-ui-check.json').read_text())
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
catalog=json.loads((P/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
coverage=set(json.loads((W/'nova34a1-chinese-font.json').read_text())['coverage'])
assert {ord(c)for row in catalog.values()for c in row[7]if ord(c)>=32}<=coverage
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for k,v in catalog.items():assert len(v)==8 and all(fmt.findall(t)==fmt.findall(k)for t in v),k
readme='''Hekate Nova Pro v34 alpha 1 — premier assistant d’installation

Depuis Accueil, choisir Assistant d’installation. Le bouton existe dans les dispositions tuiles et carrousel.

1. Lire la présentation du processus, puis Suivant.
2. Choisir uniquement le modèle (Switch V1/V2, Lite ou OLED) et les éléments souhaités : emuMMC/emuNAND, Linux, Android. Plusieurs choix sont possibles.
3. Suivant affiche un récapitulatif calculé avec la capacité réelle de la carte SD. Précédent conserve les choix. Sans système choisi, sans carte, avec un modèle incorrect ou une capacité insuffisante, la poursuite est bloquée.

La disposition réserve une emuMMC complète quand elle est sélectionnée : environ 29 Go pour V1/V2/Lite, 58 Go pour OLED. Linux et Android reçoivent automatiquement les tailles existantes recommandées de 32 Go, ou les préréglages de 16 Go si la capacité ne suffit pas. Les réserves techniques et au moins 2 Go de FAT32 sont conservés. Le choix Linux ou Android seul n’impose pas de partition emuMMC.

PORTÉE DE CETTE ÉBAUCHE
L’introduction, les choix et le récapitulatif n’effacent rien. Le bouton Suivant du récapitulatif ouvre cependant le VRAI partitionneur Hekate avec les tailles préparées. Il garde ses confirmations avant effacement. Pour tester seulement l’interface, revenir ou fermer avant de confirmer l’effacement. Sauvegarder les fichiers personnels avant tout partitionnement réel.

La copie automatique de la mémoire interne vers l’emuMMC n’est PAS encore raccordée. Les partitions Linux/Android ne constituent pas une installation de ces systèmes : leurs fichiers d’installation restent nécessaires. Cette alpha ne prétend donc pas achever une installation complète. Le chaînage et la reprise du processus feront l’objet de la prochaine étape après validation du parcours.

INSTALLATION DU PACK
Conserver vos fichiers nyx.ini, hekate_ipl.ini, vos appairages et vos images personnels. Fusionner le dossier bootloader fourni avec celui de la SD et utiliser le payload correspondant. Aucun fichier de configuration personnel n’est livré. Le pack standard est destiné aux consoles normales ; le pack 8GB reste exclusivement destiné aux consoles physiquement modifiées et expérimental, comme la v33.3.

CONTRÔLES
Compilation ARM et en-têtes Hekate/Nyx vérifiés. 456 cas de parcours, accueil et transmission protégée au partitionneur ; 683 scènes de régression ; 2061 assertions du planificateur C. Les huit langues ont des libellés pour l’assistant. Aucun test d’effacement réel ou de copie automatique n’a été exécuté sur console. Les aperçus sont des rendus du code sur PC.
'''
(O/'LIRE-MOI-NOVA34-ALPHA1.txt').write_text(readme,encoding='utf-8')
(P/'ASSISTANT-V34-ALPHA1.txt').write_text(readme,encoding='utf-8')
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b}
allowed={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_nova.h','frontend/gui_tools_partition_manager.c','frontend/gui_partition_plan.c','frontend/gui_partition_plan.h','nova_version.h','localization_catalog.json','localization_catalog.inc','fonts/sd/Nova-Chinese-Simplified.nyxf']}
assert changed<=allowed,changed-allowed
assert 'nyx/nyx_gui/frontend/gui_install_wizard.inc'in source
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
info={'version':'34-alpha1','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','wizard_cases':456,'regression_scenes':683,'planner_assertions':2061,'automatic_emummc_copy':False,'linux_android_installation':False,'partitioning_requires_existing_confirmations':True,'hardware_validated':False,'changed_existing_sources':sorted(changed),'source_sha256':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova34a1.json').write_text(json.dumps(info,indent=2))
notes={
'fr':'Premier assistant sur l’accueil : présentation, modèle et choix indépendants emuMMC/Linux/Android, récapitulatif et passage protégé vers le partitionneur. Tailles automatiques avec validation de capacité et de modèle. Copie emuMMC automatique et installation des OS non raccordées dans cette alpha.',
'en':'First assistant on Home: introduction, model and independent emuMMC/Linux/Android choices, review and guarded handoff to the partition tool. Automatic sizes with model/capacity validation. Automatic emuMMC copying and OS installation are not connected in this alpha.',
'es':'Primer asistente en Inicio: presentación, modelo y opciones emuMMC/Linux/Android independientes, resumen y acceso protegido al particionador. Tamaños automáticos y validación. Copia automática emuMMC e instalación de sistemas aún no conectadas.',
'it':'Primo assistente in Home: introduzione, modello e scelte emuMMC/Linux/Android indipendenti, riepilogo e passaggio protetto al partizionamento. Dimensioni automatiche e verifica. Copia automatica emuMMC e installazione dei sistemi non ancora collegate.',
'ru':'Первый мастер на главном экране: введение, модель, независимый выбор emuMMC/Linux/Android, обзор и защищённый переход к разметке. Автоматический расчёт и проверки. Автокопирование emuMMC и установка систем пока не подключены.',
'de':'Erster Assistent auf der Startseite: Einführung, Modell und unabhängige emuMMC/Linux/Android-Auswahl, Übersicht und geschützter Übergang zum Partitionierer. Automatische Größen und Prüfung. Automatisches emuMMC-Kopieren und OS-Installation noch nicht verbunden.',
'pt':'Primeiro assistente no Início: apresentação, modelo e escolhas emuMMC/Linux/Android independentes, resumo e acesso protegido ao particionador. Tamanhos automáticos e validação. Cópia automática emuMMC e instalação dos sistemas ainda não ligadas.',
'zh-CN':'首版安装向导：主页入口、过程介绍、机型及独立的 emuMMC/Linux/Android 选择、方案检查及受确认保护的分区工具。自动计算并检查容量和机型。本初版尚未连接 emuMMC 自动复制与系统安装。'}
for code,text in notes.items():(O/('CHANGELOG-NOVA34-ALPHA1-'+code+'.md')).write_text('# Hekate Nova Pro v34 alpha 1\n\n'+text+'\n\nPC checks passed; physical console validation pending.\n',encoding='utf-8')
summary={}
def archive(path,files):
 assert not path.exists(),path
 with ZipFile(path,'w',ZIP_DEFLATED,compresslevel=6)as z:
  for n,b in sorted(files.items()):z.writestr(n,b)
 with ZipFile(path)as z:
  assert z.testzip()is None and len(z.namelist())==len(set(z.namelist()))
  for n,b in files.items():assert z.read(n)==b,n
 summary[path.name]={'bytes':path.stat().st_size,'sha256':sha(path.read_bytes())}
for suffix in ('','-8GB'):
 with ZipFile(O/('hekate-nova-pro-v33.3-beta'+suffix+'.zip'))as z:
  files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate_nova_pro_')and n not in ('BUILD-INFO.json','BUNDLE-MANIFEST.json')}
 for n in ('nyx.bin','libsys_lp0.bso','libsys_minerva.bso'):files['bootloader/sys/'+n]=(P/'output'/n).read_bytes()
 files['bootloader/sys/Nova-Chinese-Simplified.nyxf']=(P/'nyx/nyx_gui/fonts/sd/Nova-Chinese-Simplified.nyxf').read_bytes()
 payload=bytearray((P/'output/hekate.bin').read_bytes());assert not payload[0x122]&1
 if suffix:payload[0x122]|=1
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v34_alpha1'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/ASSISTANT-V34-ALPHA1.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA34-ALPHA1-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in reports+['verification-nova34a1.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name.lower()in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','prod.keys','title.keys','last-session.txt')for n in files)
 archive(O/('hekate-nova-pro-v34-alpha1'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.3-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.3/')}
files.update({'hekate-nova-pro-v34-alpha1/'+n:b for n,b in source.items()})
for n in ['run_build_nova34a1.py','render_lvgl_preview_nova34a1.py','check_nova34a1_plan.py','check_nova34a1_wizard.py','check_nova34a1_ui.py','check_nova34a1_header.py','localize_nova34a1.py','build_nova34a1_chinese_font.py','nova34a1-zh-characters.json','package_nova34a1.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in reports+['verification-nova34a1.json','nova34a1-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V34-ALPHA1.txt']=b'Restore hekate-nova-pro-v34-alpha1 under work beside unchanged HWTest-RCM-nova33_1 and HWTest-RCM-0.15 dependencies. Use run_build_nova34a1.py with devkitARM and GNU make. The assistant include is part of gui_nova.c; partition planning uses gui_partition_plan.c. UI host checks use the official res.pak fixture and test-sd-nova34a1 resources. No SD is written by host checks. See ASSISTANT-V34-ALPHA1.txt for prototype scope. The automatic copy/install chain is intentionally not implemented in this alpha. The standalone RAM service is unchanged; the 8GB payload uses the official software flag at 0x122.\n'
archive(O/'hekate-nova-pro-v34-alpha1-sources.zip',files)
(O/'hekate-nova-pro-v34-alpha1-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova34a1-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False,'source_changes_confined_to_assistant_ui_planning_and_locales':True},indent=2))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
