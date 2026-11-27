"""Package the local automated assistant alpha; packaging never writes to a real SD."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import json,hashlib,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha3';OLD=W/'hekate-nova-pro-v33.3';O=W.parent/'outputs'
sha=lambda b:hashlib.sha256(b).hexdigest()
reports=['nova34a3-plan-check.json','nova34a3-wizard-check.json','nova34a3-ui-check.json','nova34a3-header-check.json','nova34a3-copy-check.json','nova34a3-transfer-check.json','nova34a3-partition-check.json','nova34a3-help-check.json','nova34a3-chinese-font.json']
for n in reports[:-1]:assert json.loads((W/n).read_text())['passed'],n
wizard=json.loads((W/'nova34a3-wizard-check.json').read_text())
for n,h in wizard['source_sha256'].items():assert sha((P/'nyx/nyx_gui/frontend'/n).read_bytes())==h,n
ui=json.loads((W/'nova34a3-ui-check.json').read_text())
for n,h in ui['source_sha256'].items():assert sha((P/n).read_bytes())==h,n
for report,file in [('nova34a3-copy-check.json','fe_emummc_tools.c'),('nova34a3-transfer-check.json','fe_emummc_tools.c'),('nova34a3-partition-check.json','gui_tools_partition_manager.c')]:
 assert json.loads((W/report).read_text())['source_sha256']==sha((P/'nyx/nyx_gui/frontend'/file).read_bytes()),report
catalog=json.loads((P/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
coverage=set(json.loads((W/'nova34a3-chinese-font.json').read_text())['coverage'])
assert {ord(c)for row in catalog.values()for c in row[7]if ord(c)>=32}<=coverage
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for k,v in catalog.items():assert len(v)==8 and all(fmt.findall(t)==fmt.findall(k)for t in v),k
readme=(O/'LIRE-MOI-NOVA34-ALPHA3.txt').read_text(encoding='utf-8')
(O/'LIRE-MOI-NOVA34-ALPHA3.txt').write_text(readme,encoding='utf-8')
(P/'ASSISTANT-V34-ALPHA3.txt').write_text(readme,encoding='utf-8')
source={p.relative_to(P).as_posix():p.read_bytes()for p in P.rglob('*')if p.is_file()and not any(x in ('build','output','.git','__pycache__')for x in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed={n for n,b in source.items()if(OLD/n).is_file()and(OLD/n).read_bytes()!=b}
allowed={'nyx/nyx_gui/'+n for n in ['frontend/gui_nova.c','frontend/gui_nova.h','frontend/gui_tools_partition_manager.c','frontend/fe_emummc_tools.c','frontend/fe_emummc_tools.h','frontend/gui_emummc_tools.c','frontend/gui_partition_plan.c','frontend/gui_partition_plan.h','frontend/gui_partition_nova.inc','nova_version.h','localization_catalog.json','localization_catalog.inc','fonts/sd/Nova-Chinese-Simplified.nyxf']}
assert changed<=allowed,changed-allowed
assert 'nyx/nyx_gui/frontend/gui_install_wizard.inc'in source
for n in ('nyx.bin','hekate.bin','hekate_unc.bin'):
 b=(P/'output'/n).read_bytes();at,magic=(0x98,b'GCTC195')if n=='nyx.bin'else(0x118,b'ICTC655');assert b[at:at+7]==magic
assert (P/'output/hekate.bin').stat().st_size<=130392
info={'version':'34-alpha3','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','wizard_cases':696,'help_fake_card_scenes':48,'regression_scenes':683,'planner_assertions':3539,'transaction_assertions':3664,'transfer_assertions':76,'copy_activation_assertions':254,'automatic_emummc_copy':True,'linux_android_installation':False,'partitioning_requires_existing_confirmations':True,'hardware_validated':False,'changed_existing_sources':sorted(changed),'source_sha256':{n:sha(b)for n,b in source.items()},'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p.read_bytes())}for p in (P/'output').glob('*')if p.is_file()}}
(W/'verification-nova34a3.json').write_text(json.dumps(info,indent=2))
notes={
'fr':'Choix Linux et Android indépendants : 16 Go minimum / 32 Go recommandé, sans réduction automatique. Aide : emuMMC 29 Go V1/V2/Lite ou 58 Go OLED et renvoi vers le mode expert. Accord explicite « J’accepte / Je n’accepte pas ». Suivant en bas à droite uniquement dans l’assistant. Erreurs de stockage détaillées ; blocage sur l’indicateur SD suspecte de Hekate (fabricant 0x00). Cet indicateur ne détecte pas toutes les fausses capacités. Copie emuMMC vérifiée conservée. Linux/Android : partitions seulement.',
'en':'Independent Linux and Android choices: 16 GB minimum / 32 GB recommended, without automatic reduction. Help explains 29 GB V1/V2/Lite or 58 GB OLED emuMMC and links conceptually to Expert mode. Explicit agree/disagree consent. Bottom-right Next only inside the assistant. Detailed storage errors; blocking on Hekate’s suspect-SD indicator (manufacturer 0x00). It cannot detect every false capacity. Verified emuMMC copying retained. Linux/Android: partitions only.',
'es':'Tamaños Linux y Android independientes: 16 GB mínimo / 32 GB recomendado, sin reducción automática. Ayuda: emuMMC de 29 GB V1/V2/Lite o 58 GB OLED y modo experto para tamaños propios. Consentimiento Acepto/No acepto. Siguiente abajo a la derecha solo en el asistente. Errores detallados y bloqueo del indicador SD sospechosa de Hekate (fabricante 0x00); no detecta toda capacidad falsa. Copia emuMMC verificada conservada. Linux/Android: solo particiones.',
'it':'Scelte Linux e Android indipendenti: 16 GB minimo / 32 GB consigliato, senza riduzione automatica. Aiuto: emuMMC 29 GB V1/V2/Lite o 58 GB OLED e modalità esperta per dimensioni personalizzate. Consenso Accetto/Non accetto. Avanti in basso a destra solo nell’assistente. Errori dettagliati e blocco per SD sospetta secondo Hekate (produttore 0x00); non rileva tutte le capacità false. Copia emuMMC verificata mantenuta. Linux/Android: solo partizioni.',
'ru':'Отдельные размеры Linux и Android: 16 ГБ минимум / 32 ГБ рекомендуется, без автоматического уменьшения. Справка: emuMMC 29 ГБ V1/V2/Lite или 58 ГБ OLED и экспертный режим для своего размера. Явное согласие/отказ. Далее внизу справа только в мастере. Подробные ошибки и блокировка по признаку подозрительной SD Hekate (код производителя 0x00); не выявляет все ложные ёмкости. Проверенная копия emuMMC сохранена. Linux/Android: только разделы.',
'de':'Linux- und Android-Größen getrennt: 16 GB Minimum / 32 GB empfohlen, keine automatische Verringerung. Hilfe erklärt emuMMC mit 29 GB V1/V2/Lite oder 58 GB OLED und Expertenmodus für eigene Größen. Explizite Zustimmung/Ablehnung. Weiter nur im Assistenten unten rechts. Detaillierte Speicherfehler; Sperre bei Hekates verdächtiger SD-Kennung (Hersteller 0x00), keine vollständige Fälschungserkennung. Geprüfte emuMMC-Kopie bleibt erhalten. Linux/Android: nur Partitionen.',
'pt':'Tamanhos Linux e Android independentes: 16 GB mínimo / 32 GB recomendado, sem redução automática. Ajuda explica emuMMC de 29 GB V1/V2/Lite ou 58 GB OLED e modo avançado para tamanhos próprios. Consentimento Aceito/Não aceito. Seguinte no canto inferior direito apenas no assistente. Erros detalhados; bloqueio do indicador SD suspeita do Hekate (fabricante 0x00), sem detetar todas as capacidades falsas. Cópia emuMMC verificada mantida. Linux/Android: apenas partições.',
'zh-CN':'Linux 和 Android 分别选择最低 16 GB 或推荐 32 GB，不自动缩小。帮助说明 V1/V2/Lite 的 29 GB、OLED 的 58 GB 完整 emuMMC，以及专家模式自定义大小。明确同意或拒绝后继续。仅向导中的下一步位于右下方。显示详细存储错误；按 Hekate 的可疑 SD 标记（厂商 0x00）阻止安装，此标记不能发现所有虚假容量。保留回读验证的 emuMMC 自动复制。Linux/Android 仅创建分区。'}
for code,text in notes.items():(O/('CHANGELOG-NOVA34-ALPHA3-'+code+'.md')).write_text('# Hekate Nova Pro v34 alpha 3\n\n'+text+'\n\nPC checks passed; physical console validation pending.\n',encoding='utf-8')
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
 files['bootloader/update.bin']=bytes(payload);files['hekate_nova_pro_6.5.5_v34_alpha3'+('_8GB_EXPERIMENTAL'if suffix else'')+'.bin']=bytes(payload)
 files['LIRE-MOI.txt']=readme.encode();files['documentation/ASSISTANT-V34-ALPHA3.txt']=readme.encode()
 for p in O.glob('CHANGELOG-NOVA34-ALPHA3-*.md'):files['documentation/changelogs/'+p.name]=p.read_bytes()
 for n in reports+['verification-nova34a3.json']:files['verification/'+n]=(W/n).read_bytes()
 files['BUILD-INFO.json']=json.dumps({k:v for k,v in info.items()if k!='source_sha256'}|{'variant':'8GB-experimental'if suffix else'standard'},indent=2).encode()
 files['BUNDLE-MANIFEST.json']=json.dumps({n:sha(b)for n,b in files.items()},indent=2).encode()
 assert not any(Path(n).name.lower()in ('nyx.ini','hekate_ipl.ini','joycon-pairing.bin','joycon-pairing.bak','prod.keys','title.keys','last-session.txt')for n in files)
 archive(O/('hekate-nova-pro-v34-alpha3'+suffix+'.zip'),files)
with ZipFile(O/'hekate-nova-pro-v33.3-beta-sources.zip')as z:files={n:z.read(n)for n in z.namelist()if not n.endswith('/')and not n.startswith('hekate-nova-pro-v33.3/')}
files.update({'hekate-nova-pro-v34-alpha3/'+n:b for n,b in source.items()})
for n in ['run_build_nova34a3.py','render_lvgl_preview_nova34a3.py','check_nova34a3_plan.py','check_nova34a3_wizard.py','check_nova34a3_ui.py','check_nova34a3_header.py','check_nova34a3_copy.py','check_nova34a3_transfer.py','check_nova34a3_partition.py','check_nova34a3_help.py','localize_nova34a3.py','build_nova34a3_chinese_font.py','nova34a3-zh-characters.json','package_nova34a3.py']:files['build-tools/'+n]=(W/n).read_bytes()
for n in reports+['verification-nova34a3.json','nova34a3-build.log']:files['verification/'+n]=(W/n).read_bytes()
files['BUILDING-V34-ALPHA3.txt']=b'Restore hekate-nova-pro-v34-alpha3 under work beside unchanged HWTest-RCM-nova33_1 and HWTest-RCM-0.15 dependencies. Use run_build_nova34a3.py with devkitARM and GNU make. The assistant include is part of gui_nova.c; partition planning uses gui_partition_plan.c. UI host checks use the official res.pak fixture and test-sd-nova34a3 resources. No SD is written by host checks. See ASSISTANT-V34-ALPHA3.txt for prototype scope. Partitioning chains automatically into checked full-size emuMMC copying. Linux/Android installation is not implemented. Physical testing is pending. The standalone RAM service is unchanged; the 8GB payload uses the official software flag at 0x122.\n'
archive(O/'hekate-nova-pro-v34-alpha3-sources.zip',files)
(O/'hekate-nova-pro-v34-alpha3-SHA256.txt').write_text(''.join(v['sha256']+'  '+n+'\n'for n,v in summary.items()))
(W/'nova34a3-package-check.json').write_text(json.dumps({'passed':True,'archives':summary,'published':False,'changes_include_guarded_partition_copy_transaction':True},indent=2))
for n,v in summary.items():print(n,v['bytes'],v['sha256'])
