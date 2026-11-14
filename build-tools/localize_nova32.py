from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v32/nyx/nyx_gui'
phrases={}
def add(*v):assert len(v)==7;phrases[v[0]]=list(v)
add('Saved pairing loaded. Bluetooth starts automatically.',
 'Appairages enregistrés chargés. Démarrage automatique du Bluetooth.',
 'Enlaces guardados cargados. Bluetooth se inicia automáticamente.',
 'Associazioni salvate caricate. Bluetooth si avvia automaticamente.',
 'Сохранённые сопряжения загружены. Bluetooth запускается автоматически.',
 'Gespeicherte Kopplungen geladen. Bluetooth startet automatisch.',
 'Emparelhamentos guardados carregados. Bluetooth inicia automaticamente.')
add('v32 · Background and responsiveness\nAutomatic wireless start without attached Joy-Con.\nBounded redraws and brief button press retention.',
 'v32 · Arrière-plan et réactivité\nBluetooth automatique sans Joy-Con sur les rails.\nRedessin limité et prise en compte des appuis brefs.',
 'v32 · Segundo plano y respuesta\nBluetooth automático sin Joy-Con acoplados.\nRedibujado limitado y recepción de pulsaciones breves.',
 'v32 · Attività in background e risposta\nBluetooth automatico senza Joy-Con collegati.\nRidisegno limitato e gestione delle pressioni brevi.',
 'v32 · Фоновая работа и отклик\nАвтозапуск Bluetooth без Joy-Con на направляющих.\nОграничение перерисовки и учёт коротких нажатий.',
 'v32 · Hintergrund und Reaktionszeit\nBluetooth-Autostart ohne angesteckte Joy-Con.\nBegrenzte Neuzeichnung und Erfassung kurzer Tastendrücke.',
 'v32 · Segundo plano e resposta\nBluetooth automático sem Joy-Con encaixados.\nRedesenho limitado e receção de toques breves.')
phrases['%d Go']=['%d GB', '%d Go', '%d GB', '%d GB', '%d ГБ', '%d GB', '%d GB']
phrases['%d Go Full']=['%d GB Full', '%d Go Full', '%d GB Full', '%d GB Full', '%d ГБ Full', '%d GB Full', '%d GB Full']
phrases['2 x %d Go']=['2 x %d GB', '2 x %d Go', '2 x %d GB', '2 x %d GB', '2 x %d ГБ', '2 x %d GB', '2 x %d GB']
phrases['2 x %d Go Full']=['2 x %d GB Full', '2 x %d Go Full', '2 x %d GB Full', '2 x %d GB Full', '2 x %d ГБ Full', '2 x %d GB Full', '2 x %d GB Full']
phrases['Capacité : %d Go']=['Capacity: %d GB', 'Capacité : %d Go', 'Capacidad: %d GB', 'Capacità: %d GB', 'Объём: %d ГБ', 'Kapazität: %d GB', 'Capacidade: %d GB']
phrases['%s : %d Go  |  emuMMC : %s  |  Linux : %d Go  |  Android : %d Go']=['%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s : %d Go  |  emuMMC : %s  |  Linux : %d Go  |  Android : %d Go', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d ГБ  |  emuMMC: %s  |  Linux: %d ГБ  |  Android: %d ГБ', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB', '%s: %d GB  |  emuMMC: %s  |  Linux: %d GB  |  Android: %d GB']
phrases['v32 · Expert partitions and wireless\nFull-width controls and integer GB / Full sizes.\nBluetooth servicing during drawing and radio metrics.']=['v32 · Expert partitions and wireless\nFull-width controls and integer GB / Full sizes.\nBluetooth servicing during drawing and radio metrics.', 'v32 · Partitions expertes et sans-fil\nRéglages pleine largeur et tailles entières en Go / Full.\nBluetooth pendant le dessin et mesures radio.', 'v32 · Particiones expertas y conexión inalámbrica\nControles amplios y tamaños enteros en GB / Full.\nBluetooth durante el dibujo y medidas de radio.', 'v32 · Partizioni esperte e wireless\nControlli ampi e dimensioni intere in GB / Full.\nBluetooth durante il disegno e misure radio.', 'v32 · Разделы и беспроводное управление\nШирокие регуляторы, целые ГБ / Full.\nОбслуживание Bluetooth при отрисовке и метрики радио.', 'v32 · Expertenpartitionen und Funk\nBreite Regler und ganze GB / Full.\nBluetooth bei der Darstellung und Funkmesswerte.', 'v32 · Partições avançadas e sem fios\nControlos largos e tamanhos inteiros em GB / Full.\nBluetooth durante o desenho e medidas de rádio.']
phrases["Vérifier la disposition\n\nFAT32 / USER : %d Go\nemuMMC : %s\nLinux : %d Go\nAndroid : %d Go\nRéserves et alignements inclus.\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante."]=['Check layout\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReserves and alignment included.\n\nThe existing erase confirmation is shown next.', "Vérifier la disposition\n\nFAT32 / USER : %d Go\nemuMMC : %s\nLinux : %d Go\nAndroid : %d Go\nRéserves et alignements inclus.\n\nLa confirmation d'effacement existante sera affichée à l'étape suivante.", 'Verificar distribución\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReservas y alineación incluidas.\n\nLa confirmación de borrado se mostrará a continuación.', 'Verifica disposizione\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nRiserve e allineamento inclusi.\n\nLa conferma di cancellazione apparirà al passaggio successivo.', 'Проверка разметки\n\nFAT32 / USER: %d ГБ\nemuMMC: %s\nLinux: %d ГБ\nAndroid: %d ГБ\nРезервы и выравнивание учтены.\n\nПодтверждение удаления появится на следующем шаге.', 'Aufteilung prüfen\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReserven und Ausrichtung berücksichtigt.\n\nDie Löschbestätigung folgt im nächsten Schritt.', 'Verificar disposição\n\nFAT32 / USER: %d GB\nemuMMC: %s\nLinux: %d GB\nAndroid: %d GB\nReservas e alinhamento incluídos.\n\nA confirmação de apagamento será apresentada a seguir.']
phrases['Valide · réserves incluses']=['Valid · reserves included', 'Valide · réserves incluses', 'Válido · reservas incluidas', 'Valido · riserve incluse', 'Верно · резервы учтены', 'Gültig · Reserven enthalten', 'Válido · reservas incluídas']
phrases['Espace insuffisant']=['Insufficient space', 'Espace insuffisant', 'Espacio insuficiente', 'Spazio insufficiente', 'Недостаточно места', 'Nicht genug Speicher', 'Espaço insuficiente']
phrases['Modèle incompatible']=['Incompatible model', 'Modèle incompatible', 'Modelo incompatible', 'Modello incompatibile', 'Несовместимая модель', 'Modell inkompatibel', 'Modelo incompatível']
phrases['eMMC indisponible']=['eMMC unavailable', 'eMMC indisponible', 'eMMC no disponible', 'eMMC non disponibile', 'eMMC недоступна', 'eMMC nicht verfügbar', 'eMMC indisponível']
phrases['Mode expert requis']=['Expert mode required', 'Mode expert requis', 'Requiere modo experto', 'Richiede modalità esperta', 'Нужен экспертный режим', 'Expertenmodus nötig', 'Requer modo avançado']
phrases['Linux trop petit']=['Linux too small', 'Linux trop petit', 'Linux demasiado pequeño', 'Linux troppo piccolo', 'Linux слишком мал', 'Linux zu klein', 'Linux demasiado pequeno']
phrases['Android incomplet']=['Android incomplete', 'Android incomplet', 'Android incompleto', 'Android incompleto', 'Android неполный', 'Android unvollständig', 'Android incompleto']
phrases['Tailles incompatibles']=['Incompatible sizes', 'Tailles incompatibles', 'Tamaños incompatibles', 'Dimensioni incompatibili', 'Несовместимые размеры', 'Größen inkompatibel', 'Tamanhos incompatíveis']
phrases['v32 · Layout and wireless recovery\nAligned partition values and a single-line summary.\nAutomatic per-Joy-Con stream recovery.']=['v32 · Layout and wireless recovery\nAligned partition values and a single-line summary.\nAutomatic per-Joy-Con stream recovery.', 'v32 · Disposition et reprise sans fil\nTailles alignées et récapitulatif sur une ligne.\nReprise automatique du flux de chaque Joy-Con.', 'v32 · Distribución y recuperación inalámbrica\nTamaños alineados y resumen en una línea.\nRecuperación automática de cada Joy-Con.', 'v32 · Disposizione e ripristino wireless\nDimensioni allineate e riepilogo su una riga.\nRipristino automatico del flusso di ogni Joy-Con.', 'v32 · Разметка и восстановление связи\nРазмеры справа, сводка в одну строку.\nАвтовосстановление потока каждого Joy-Con.', 'v32 · Anordnung und Funkwiederherstellung\nAusgerichtete Größen, Übersicht in einer Zeile.\nAutomatische Wiederherstellung je Joy-Con.', 'v32 · Disposição e recuperação sem fios\nTamanhos alinhados e resumo numa linha.\nRecuperação automática de cada Joy-Con.']
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
(root/'localization_nova32.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: new boot/changelog texts in seven languages; prior catalog retained.')
