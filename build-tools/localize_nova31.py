from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v31/nyx/nyx_gui'
phrases={}
def add(*v):assert len(v)==7;phrases[v[0]]=list(v)
add('Saved pairing loaded. Bluetooth starts automatically.',
 'Appairages enregistrés chargés. Démarrage automatique du Bluetooth.',
 'Enlaces guardados cargados. Bluetooth se inicia automáticamente.',
 'Associazioni salvate caricate. Bluetooth si avvia automaticamente.',
 'Сохранённые сопряжения загружены. Bluetooth запускается автоматически.',
 'Gespeicherte Kopplungen geladen. Bluetooth startet automatisch.',
 'Emparelhamentos guardados carregados. Bluetooth inicia automaticamente.')
add('v31 · Background and responsiveness\nAutomatic wireless start without attached Joy-Con.\nBounded redraws and brief button press retention.',
 'v31 · Arrière-plan et réactivité\nBluetooth automatique sans Joy-Con sur les rails.\nRedessin limité et prise en compte des appuis brefs.',
 'v31 · Segundo plano y respuesta\nBluetooth automático sin Joy-Con acoplados.\nRedibujado limitado y recepción de pulsaciones breves.',
 'v31 · Attività in background e risposta\nBluetooth automatico senza Joy-Con collegati.\nRidisegno limitato e gestione delle pressioni brevi.',
 'v31 · Фоновая работа и отклик\nАвтозапуск Bluetooth без Joy-Con на направляющих.\nОграничение перерисовки и учёт коротких нажатий.',
 'v31 · Hintergrund und Reaktionszeit\nBluetooth-Autostart ohne angesteckte Joy-Con.\nBegrenzte Neuzeichnung und Erfassung kurzer Tastendrücke.',
 'v31 · Segundo plano e resposta\nBluetooth automático sem Joy-Con encaixados.\nRedesenho limitado e receção de toques breves.')
phrases['%d Go']=['%d GB', '%d Go', '%d GB', '%d GB', '%d ГБ', '%d GB', '%d GB']
phrases['%d Go Full']=['%d GB Full', '%d Go Full', '%d GB Full', '%d GB Full', '%d ГБ Full', '%d GB Full', '%d GB Full']
phrases['2 x %d Go']=['2 x %d GB', '2 x %d Go', '2 x %d GB', '2 x %d GB', '2 x %d ГБ', '2 x %d GB', '2 x %d GB']
phrases['2 x %d Go Full']=['2 x %d GB Full', '2 x %d Go Full', '2 x %d GB Full', '2 x %d GB Full', '2 x %d ГБ Full', '2 x %d GB Full', '2 x %d GB Full']
phrases['Capacité : %d Go']=['Capacity: %d GB', 'Capacité : %d Go', 'Capacidad: %d GB', 'Capacità: %d GB', 'Объём: %d ГБ', 'Kapazität: %d GB', 'Capacidade: %d GB']
phrases['%s : %d Go  |  emuMMC : %s  |  Linux : %d Go  |  Android : %d Go']=['%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s : %d Go  |  emuMMC : %s  |  Linux : %d Go  |  Android : %d Go', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d ГБ  |  emuMMC: %s  |  Linux: %d ГБ  |  Android: %d ГБ', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB']
phrases['v31 · Expert partitions and wireless\nFull-width controls and integer GB / Full sizes.\nBluetooth servicing during drawing and radio metrics.']=['v31 · Expert partitions and wireless\nFull-width controls and integer GB / Full sizes.\nBluetooth servicing during drawing and radio metrics.', 'v31 · Partitions expertes et sans-fil\nRéglages pleine largeur et tailles entières en Go / Full.\nBluetooth pendant le dessin et mesures radio.', 'v31 · Particiones expertas y conexión inalámbrica\nControles amplios y tamaños enteros en GB / Full.\nBluetooth durante el dibujo y medidas de radio.', 'v31 · Partizioni esperte e wireless\nControlli ampi e dimensioni intere in GB / Full.\nBluetooth durante il disegno e misure radio.', 'v31 · Разделы и беспроводное управление\nШирокие регуляторы, целые ГБ / Full.\nОбслуживание Bluetooth при отрисовке и метрики радио.', 'v31 · Expertenpartitionen und Funk\nBreite Regler und ganze GB / Full.\nBluetooth bei der Darstellung und Funkmesswerte.', 'v31 · Partições avançadas e sem fios\nControlos largos e tamanhos inteiros em GB / Full.\nBluetooth durante o desenho e medidas de rádio.']
phrases["Vérifier la disposition\n\nFAT32 / USER : %d Go\nemuMMC : %s\nLinux : %d Go\nAndroid : %d Go\nRéserves et alignements inclus.\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante."]=['Check layout\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReserves and alignment included.\n\nThe existing erase confirmation is shown next.', "Vérifier la disposition\n\nFAT32 / USER : %d Go\nemuMMC : %s\nLinux : %d Go\nAndroid : %d Go\nRéserves et alignements inclus.\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante.", 'Verificar distribución\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReservas y alineación incluidas.\n\nLa confirmación de borrado se mostrará a continuación.', 'Verifica disposizione\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nRiserve e allineamento inclusi.\n\nLa conferma di cancellazione apparirà al passaggio successivo.', 'Проверка разметки\n\nFAT32 / USER: %d ГБ\nemuMMC: %s\nLinux: %d ГБ\nAndroid: %d ГБ\nРезервы и выравнивание учтены.\n\nПодтверждение удаления появится на следующем шаге.', 'Aufteilung prüfen\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReserven und Ausrichtung berücksichtigt.\n\nDie Löschbestätigung folgt im nächsten Schritt.', 'Verificar disposição\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReservas e alinhamento incluídos.\n\nA confirmação de apagamento será apresentada a seguir.']
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
(root/'localization_nova31.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: new boot/changelog texts in seven languages; prior catalog retained.')
