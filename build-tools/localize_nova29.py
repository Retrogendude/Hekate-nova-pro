from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v29/nyx/nyx_gui'
phrases={}
def add(*values):assert len(values)==7;phrases[values[0]]=list(values)
add('v29 · Pairing and reconnection\nValidated standard host profiles accepted.\nSD pairing backup and persistent reconnect attempts.',
 'v29 · Appairages et reconnexion\nProfils standard validés acceptés.\nSauvegarde SD et tentatives de reconnexion prolongées.',
 'v29 · Enlace y reconexión\nSe aceptan perfiles estándar válidos.\nCopia en SD y reintentos de conexión prolongados.',
 'v29 · Associazione e riconnessione\nProfili standard validati accettati.\nBackup su SD e tentativi di riconnessione prolungati.',
 'v29 · Сопряжение и переподключение\nПриём проверенных стандартных профилей.\nКопия на SD и продолжение попыток подключения.',
 'v29 · Kopplung und Wiederverbindung\nGeprüfte Standardprofile werden akzeptiert.\nSD-Sicherung und weitere Verbindungsversuche.',
 'v29 · Emparelhamento e reconexão\nPerfis padrão validados aceites.\nCópia no SD e novas tentativas de ligação.')
add('Pairing checksum or host profile not recognized.',
 'Somme de contrôle ou profil d’appairage non reconnu.',
 'Suma de comprobación o perfil de enlace no reconocido.',
 'Checksum o profilo di associazione non riconosciuto.',
 'Контрольная сумма или профиль сопряжения не распознаны.',
 'Prüfsumme oder Kopplungsprofil nicht erkannt.',
 'Soma de verificação ou perfil de emparelhamento não reconhecido.')
add('Save pairing','Sauvegarder les appairages','Guardar enlaces','Salva associazioni','Сохранить сопряжения','Kopplungen speichern','Guardar emparelhamentos')
add('Load pairing','Charger les appairages','Cargar enlaces','Carica associazioni','Загрузить сопряжения','Kopplungen laden','Carregar emparelhamentos')
add('Stop Bluetooth before saving or loading pairing.',
 'Arrêtez le Bluetooth avant de sauvegarder ou charger les appairages.',
 'Detén Bluetooth antes de guardar o cargar enlaces.',
 'Arresta Bluetooth prima di salvare o caricare le associazioni.',
 'Остановите Bluetooth перед сохранением или загрузкой сопряжений.',
 'Bluetooth vor dem Speichern oder Laden von Kopplungen stoppen.',
 'Pare o Bluetooth antes de guardar ou carregar emparelhamentos.')
add('Pairing saved to SD. Keep this file private.',
 'Appairages sauvegardés sur SD. Gardez ce fichier privé.',
 'Enlaces guardados en SD. Mantén este archivo privado.',
 'Associazioni salvate su SD. Mantieni privato questo file.',
 'Сопряжения сохранены на SD. Не передавайте этот файл.',
 'Kopplungen auf SD gespeichert. Diese Datei privat halten.',
 'Emparelhamentos guardados no SD. Mantenha este ficheiro privado.')
add('Pairing could not be saved. Read valid pairing first.',
 'Sauvegarde impossible. Lisez d’abord des appairages valides.',
 'No se pudo guardar. Lee primero enlaces válidos.',
 'Salvataggio impossibile. Leggi prima associazioni valide.',
 'Сохранение не удалось. Сначала считайте действительные сопряжения.',
 'Speichern fehlgeschlagen. Zuerst gültige Kopplungen lesen.',
 'Não foi possível guardar. Leia primeiro emparelhamentos válidos.')
add('Pairing loaded from SD.',
 'Appairages chargés depuis la SD.',
 'Enlaces cargados desde la SD.',
 'Associazioni caricate dalla SD.',
 'Сопряжения загружены с SD.',
 'Kopplungen von SD geladen.',
 'Emparelhamentos carregados do SD.')
add('No valid pairing file found. Current session keys kept.',
 'Aucun fichier d’appairage valide. Les clés de la session sont conservées.',
 'No hay archivo de enlace válido. Se conservan las claves de la sesión.',
 'Nessun file di associazione valido. Chiavi della sessione conservate.',
 'Действительный файл сопряжения не найден. Ключи сеанса сохранены.',
 'Keine gültige Kopplungsdatei gefunden. Sitzungsschlüssel beibehalten.',
 'Nenhum ficheiro de emparelhamento válido. Chaves da sessão mantidas.')
catalog=root/'localization_catalog.inc';lines={}
for line in catalog.read_text(encoding='utf-8').splitlines():
 if line.startswith('    {"'):lines[json.JSONDecoder().raw_decode(line.strip()[1:])[0]]=line
for key,values in phrases.items():lines[key]='    {'+json.dumps(key,ensure_ascii=False)+', {'+', '.join(json.dumps(v,ensure_ascii=False)for v in values)+'}},'
keys=sorted(lines);patterns=[i for i,k in enumerate(keys)if '%'in k and len(re.findall(r'[A-Za-z]',k))>=5];first=[keys[i].encode('utf-8')[0]for i in patterns]
tail='static const uint16_t locale_pattern_rows[] = {'+', '.join(map(str,patterns))+'};\n'
tail+='static const uint16_t locale_pattern_start[257] = {'+', '.join(str(sum(ch<code for ch in first))for code in range(257))+'};\n'
catalog.write_text('/* Nova catalog: en fr es it ru de pt. */\nstatic const locale_entry locale_catalog[] = {\n'+'\n'.join(lines[k]for k in keys)+'\n};\n'+tail,encoding='utf-8')
data=json.loads((root/'localization_catalog.json').read_text(encoding='utf-8'));data.update(phrases)
(root/'localization_catalog.json').write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
(root/'localization_nova29.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print(f'PASS: {len(phrases)} new phrases in seven languages.')
