"""Package the local automated assistant alpha; packaging never writes to a real SD."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha2';OLD=W/'hekate-nova-pro-v33.3';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
reports=['nova34a2-plan-check.json','nova34a2-wizard-check.json','nova34a2-ui-check.json','nova34a2-header-check.json','nova34a2-copy-check.json','nova34a2-transfer-check.json','nova34a2-partition-check.json','nova34a2-chinese-font.json']
for n in reports[:-1]:assert json.loads((W/n).read_text())['passed'],n
wizard=json.loads((W/'nova34a2-wizard-check.json').read_text())
for n,h in wizard['source_sha256'].items():assert sha((P/'nyx/nyx_gui/frontend'/n).read_bytes())==h,n
ui=json.loads((W/'nova34a2-ui-check.json').read_text())
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
for report,file in [('nova34a2-copy-check.json','fe_emummc_tools.c'),('nova34a2-transfer-check.json','fe_emummc_tools.c'),('nova34a2-partition-check.json','gui_tools_partition_manager.c')]:
 assert json.loads((W/report).read_text())['source_sha256']==sha((P/'nyx/nyx_gui/frontend'/file).read_bytes()),report
catalog=json.loads((P/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
coverage=set(json.loads((W/'nova34a2-chinese-font.json').read_text())['coverage'])
assert {ord(c)for row in catalog.values()for c in row[7]if ord(c)>=32}<=coverage
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for k,v in catalog.items():assert len(v)==8 and all(fmt.findall(t)==fmt.findall(k)for t in v),k
readme=(O/'LIRE-MOI-NOVA34-ALPHA2.txt').read_text(encoding='utf-8')
(O/'LIRE-MOI-NOVA34-ALPHA2.txt').write_text(readme,encoding='utf-8')
(P/'ASSISTANT-V34-ALPHA2.txt').write_text(readme,encoding='utf-8')
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b}
allowed={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_nova.h','frontend/gui_tools_partition_manager.c','frontend/fe_emummc_tools.c','frontend/fe_emummc_tools.h','frontend/gui_emummc_tools.c','frontend/gui_partition_plan.c','frontend/gui_partition_plan.h','frontend/gui_partition_nova.inc','nova_version.h','localization_catalog.json','localization_catalog.inc','fonts/sd/Nova-Chinese-Simplified.nyxf']}
assert changed<=allowed,changed-allowed
assert 'nyx/nyx_gui/frontend/gui_install_wizard.inc'in source
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
info={'version':'34-alpha2','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','wizard_cases':480,'regression_scenes':683,'planner_assertions':2861,'transaction_assertions':3651,'transfer_assertions':76,'copy_activation_assertions':254,'automatic_emummc_copy':True,'linux_android_installation':False,'partitioning_requires_existing_confirmations':True,'hardware_validated':False,'changed_existing_sources':sorted(changed),'source_sha256':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova34a2.json').write_text(json.dumps(info,indent=2))
notes={
'fr':'Bouton Assistant d’installation automatisé agrandi avec icône ; confirmation obligatoire de sauvegarde et avertissement d’effacement ; après un partitionnement réussi, copie complète emuMMC avec relecture avant activation. Vérifications de capacité, modèle, identité SD et cible ; arrêt sur erreur. Linux/Android : partitions seulement. Tests PC réussis ; validation physique à effectuer.',
'en':'Larger Automated installation assistant button with icon; required backup acknowledgement and erasure warning; successful partitioning automatically starts full emuMMC copying with readback before activation. Capacity, model, SD identity and target checks; stop on error. Linux/Android: partitions only. PC checks passed; hardware validation pending.',
'es':'Botón del asistente de instalación automatizada ampliado con icono; confirmación de copia de seguridad y aviso de borrado; tras particionar correctamente, copia completa de emuMMC y verificación por lectura antes de activarla. Controles de capacidad, modelo, identidad SD y destino; detención ante errores. Linux/Android: solo particiones. Pruebas en PC superadas; falta validación en consola.',
'it':'Pulsante assistente di installazione automatizzata più grande con icona; conferma del backup e avviso di cancellazione; dopo il partizionamento riuscito, copia completa emuMMC verificata in lettura prima dell’attivazione. Controlli di capacità, modello, identità SD e destinazione; arresto in caso di errore. Linux/Android: solo partizioni. Test PC superati; verifica su console da eseguire.',
'ru':'Увеличена кнопка автоматического мастера установки и добавлен значок. Обязательное подтверждение резервной копии и предупреждение об удалении. После успешной разметки выполняется полное копирование emuMMC с проверкой чтением до активации. Проверяются ёмкость, модель, идентичность SD и целевой раздел; при ошибке процесс останавливается. Linux/Android: только разделы. Тесты ПК пройдены; проверка на консоли ожидается.',
'de':'Größere Schaltfläche für den automatischen Installationsassistenten mit Symbol; verpflichtende Backup-Bestätigung und Löschwarnung; nach erfolgreicher Partitionierung vollständige emuMMC-Kopie mit Rückleseprüfung vor der Aktivierung. Prüfung von Kapazität, Modell, SD-Identität und Ziel; Stopp bei Fehlern. Linux/Android: nur Partitionen. PC-Tests bestanden; Hardwareprüfung ausstehend.',
'pt':'Botão maior do assistente de instalação automatizada com ícone; confirmação obrigatória de backup e aviso de apagamento; após particionamento bem-sucedido, cópia completa da emuMMC com releitura antes da ativação. Verifica capacidade, modelo, identidade SD e destino; para em caso de erro. Linux/Android: apenas partições. Testes no PC concluídos; validação na consola pendente.',
'zh-CN':'主页自动安装向导按钮增大并添加图标。必须确认已备份，并显示清除数据警告。分区成功后自动完整复制 emuMMC，回读验证后才启用。检查容量、机型、SD 卡身份和目标分区；遇错停止。Linux/Android 仅创建分区。电脑测试已通过，仍需实体主机验证。'}
for code,text in notes.items():(O/('CHANGELOG-NOVA34-ALPHA2-'+code+'.md')).write_text('# Hekate Nova Pro v34 alpha 2\n\n'+text+'\n\nPC checks passed; physical console validation pending.\n',encoding='utf-8')
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
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v34_alpha2'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/ASSISTANT-V34-ALPHA2.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA34-ALPHA2-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in reports+['verification-nova34a2.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name.lower()in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','prod.keys','title.keys','last-session.txt')for n in files)
 archive(O/('hekate-nova-pro-v34-alpha2'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.3-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.3/')}
files.update({'hekate-nova-pro-v34-alpha2/'+n:b for n,b in source.items()})
for n in ['run_build_nova34a2.py','render_lvgl_preview_nova34a2.py','check_nova34a2_plan.py','check_nova34a2_wizard.py','check_nova34a2_ui.py','check_nova34a2_header.py','check_nova34a2_copy.py','check_nova34a2_transfer.py','check_nova34a2_partition.py','localize_nova34a2.py','build_nova34a2_chinese_font.py','nova34a2-zh-characters.json','package_nova34a2.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in reports+['verification-nova34a2.json','nova34a2-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V34-ALPHA2.txt']=b'Restore hekate-nova-pro-v34-alpha2 under work beside unchanged HWTest-RCM-nova33_1 and HWTest-RCM-0.15 dependencies. Use run_build_nova34a2.py with devkitARM and GNU make. The assistant include is part of gui_nova.c; partition planning uses gui_partition_plan.c. UI host checks use the official res.pak fixture and test-sd-nova34a2 resources. No SD is written by host checks. See ASSISTANT-V34-ALPHA2.txt for prototype scope. Partitioning chains automatically into checked full-size emuMMC copying. Linux/Android installation is not implemented. Physical testing is pending. The standalone RAM service is unchanged; the 8GB payload uses the official software flag at 0x122.\n'
archive(O/'hekate-nova-pro-v34-alpha2-sources.zip',files)
(O/'hekate-nova-pro-v34-alpha2-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova34a2-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False,'changes_include_guarded_partition_copy_transaction':True},indent=2))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
