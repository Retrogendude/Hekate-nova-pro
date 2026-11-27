"""Package the local automated assistant alpha; packaging never writes to a real SD."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha4';OLD=W/'hekate-nova-pro-v33.3';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
reports=['nova34a4-plan-check.json','nova34a4-wizard-check.json','nova34a4-ui-check.json','nova34a4-header-check.json','nova34a4-copy-check.json','nova34a4-transfer-check.json','nova34a4-partition-check.json','nova34a4-help-check.json','nova34a4-fatfs-check.json','nova34a4-chinese-font.json']
for n in reports[:-1]:assert json.loads((W/n).read_text())['passed'],n
wizard=json.loads((W/'nova34a4-wizard-check.json').read_text())
for n,h in wizard['source_sha256'].items():assert sha((P/'nyx/nyx_gui/frontend'/n).read_bytes())==h,n
ui=json.loads((W/'nova34a4-ui-check.json').read_text())
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
for report,file in [('nova34a4-copy-check.json','fe_emummc_tools.c'),('nova34a4-transfer-check.json','fe_emummc_tools.c'),('nova34a4-partition-check.json','gui_tools_partition_manager.c')]:
 assert json.loads((W/report).read_text())['source_sha256']==sha((P/'nyx/nyx_gui/frontend'/file).read_bytes()),report
for name,h in json.loads((W/'nova34a4-fatfs-check.json').read_text())['source_sha256'].items():
 assert sha((P/name).read_bytes())==h,name
catalog=json.loads((P/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
coverage=set(json.loads((W/'nova34a4-chinese-font.json').read_text())['coverage'])
assert {ord(c)for row in catalog.values()for c in row[7]if ord(c)>=32}<=coverage
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for k,v in catalog.items():assert len(v)==8 and all(fmt.findall(t)==fmt.findall(k)for t in v),k
readme=(O/'LIRE-MOI-NOVA34-ALPHA4.txt').read_text(encoding='utf-8')
(O/'LIRE-MOI-NOVA34-ALPHA4.txt').write_text(readme,encoding='utf-8')
(P/'ASSISTANT-V34-ALPHA4.txt').write_text(readme,encoding='utf-8')
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b}
allowed={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_nova.h','frontend/gui_tools_partition_manager.c','frontend/fe_emummc_tools.c','frontend/fe_emummc_tools.h','frontend/gui_emummc_tools.c','frontend/gui_partition_plan.c','frontend/gui_partition_plan.h','frontend/gui_partition_nova.inc','nova_version.h','localization_catalog.json','localization_catalog.inc','fonts/sd/Nova-Chinese-Simplified.nyxf']}
assert changed<=allowed,changed-allowed
assert 'nyx/nyx_gui/frontend/gui_install_wizard.inc'in source
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
info={'version':'34-alpha4','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','wizard_cases':696,'help_fake_card_scenes':48,'regression_scenes':683,'planner_assertions':3539,'transaction_assertions':3664,'transfer_assertions':76,'copy_activation_assertions':246,'real_nyx_fatfs_assertions':147,'gui_plan_checks_inherited_from':'34-alpha3','automatic_emummc_copy':True,'linux_android_installation':False,'partitioning_requires_existing_confirmations':True,'hardware_validated':False,'changed_existing_sources':sorted(changed),'source_sha256':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova34a4.json').write_text(json.dumps(info,indent=2))
notes={
'fr':'Correction du faux échec d’enregistrement emuMMC/emummc.ini avant le partitionnement : écriture en octets exacts, sans conversion automatique des fins de ligne. Synchronisation et relecture conservées. Défaut reproduit et correctif testé avec le vrai FatFs Nyx sur FAT32/exFAT virtuels. Écritures courtes et protection en écriture toujours refusées. Interface et fonctions de l’alpha 3 conservées ; essai réel sur console à effectuer.',
'en':'Fix false emuMMC/emummc.ini save failure before partitioning: write exact bytes without automatic newline conversion. Sync and readback remain enforced. Reproduced the defect and tested the fix with actual Nyx FatFs on virtual FAT32/exFAT. Short writes and write protection still rejected. Alpha 3 UI and features retained; console validation pending.',
'es':'Corregido el falso fallo al guardar emuMMC/emummc.ini antes de particionar: escritura exacta sin convertir saltos de línea. Se conservan sincronización y verificación. Defecto reproducido y solución probada con FatFs real de Nyx en FAT32/exFAT virtuales. Se rechazan escrituras cortas y protección de escritura. Interfaz alpha 3 conservada; falta ensayo en consola.',
'it':'Corretto il falso errore di salvataggio emuMMC/emummc.ini prima del partizionamento: scrittura esatta senza conversione delle righe. Sincronizzazione e rilettura mantenute. Difetto riprodotto e correzione verificata con FatFs reale Nyx su FAT32/exFAT virtuali. Scritture incomplete e protezione scrittura ancora rifiutate. Interfaccia alpha 3 mantenuta; prova su console da eseguire.',
'ru':'Исправлен ложный отказ сохранения emuMMC/emummc.ini до разметки: точная запись байтов без преобразования переводов строк. Синхронизация и проверка чтением сохранены. Ошибка воспроизведена, исправление проверено реальным FatFs Nyx на виртуальных FAT32/exFAT. Неполная запись и защита записи по-прежнему блокируют операцию. Интерфейс alpha 3 сохранён; нужна проверка на консоли.',
'de':'Falschen Speicherfehler von emuMMC/emummc.ini vor der Partitionierung behoben: exakte Byte-Ausgabe ohne Zeilenumbruchkonvertierung. Synchronisierung und Rückleseprüfung bleiben aktiv. Fehler mit echtem Nyx-FatFs auf virtuellen FAT32/exFAT reproduziert und Korrektur geprüft. Unvollständige Ausgabe und Schreibschutz werden weiterhin abgewiesen. Alpha-3-Oberfläche bleibt erhalten; Konsolentest ausstehend.',
'pt':'Corrigida a falsa falha ao guardar emuMMC/emummc.ini antes de particionar: escrita exata sem conversão de linhas. Sincronização e releitura mantidas. Defeito reproduzido e correção testada com FatFs real Nyx em FAT32/exFAT virtuais. Escritas incompletas e proteção de escrita continuam bloqueadas. Interface alpha 3 mantida; falta teste na consola.',
'zh-CN':'修复分区前保存 emuMMC/emummc.ini 的误报：按原始字节写入，不自动转换换行。保留同步和回读验证。已用真实 Nyx FatFs 在虚拟 FAT32/exFAT 上重现故障并验证修复。不完整写入和写保护仍会阻止操作。保留 alpha 3 界面与功能，仍需实体主机验证。'}
for code,text in notes.items():(O/('CHANGELOG-NOVA34-ALPHA4-'+code+'.md')).write_text('# Hekate Nova Pro v34 alpha 4\n\n'+text+'\n\nPC checks passed; physical console validation pending.\n',encoding='utf-8')
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
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v34_alpha4'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/ASSISTANT-V34-ALPHA4.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA34-ALPHA4-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in reports+['verification-nova34a4.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name.lower()in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','prod.keys','title.keys','last-session.txt')for n in files)
 archive(O/('hekate-nova-pro-v34-alpha4'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.3-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.3/')}
files.update({'hekate-nova-pro-v34-alpha4/'+n:b for n,b in source.items()})
for n in ['run_build_nova34a4.py','render_lvgl_preview_nova34a4.py','check_nova34a4_plan.py','check_nova34a4_wizard.py','check_nova34a4_ui.py','check_nova34a4_header.py','check_nova34a4_copy.py','check_nova34a4_transfer.py','check_nova34a4_partition.py','check_nova34a4_help.py','check_nova34a4_fatfs.py','localize_nova34a4.py','build_nova34a4_chinese_font.py','nova34a4-zh-characters.json','package_nova34a4.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in reports+['verification-nova34a4.json','nova34a4-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V34-ALPHA4.txt']=b'Restore hekate-nova-pro-v34-alpha4 under work beside unchanged HWTest-RCM-nova33_1 and HWTest-RCM-0.15 dependencies. Use run_build_nova34a4.py with devkitARM and GNU make. The assistant include is part of gui_nova.c; partition planning uses gui_partition_plan.c. UI host checks use the official res.pak fixture and test-sd-nova34a4 resources. No SD is written by host checks. See ASSISTANT-V34-ALPHA4.txt for prototype scope. Partitioning chains automatically into checked full-size emuMMC copying. Linux/Android installation is not implemented. Physical testing is pending. The standalone RAM service is unchanged; the 8GB payload uses the official software flag at 0x122.\n'
archive(O/'hekate-nova-pro-v34-alpha4-sources.zip',files)
(O/'hekate-nova-pro-v34-alpha4-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova34a4-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False,'changes_include_guarded_partition_copy_transaction':True},indent=2))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
