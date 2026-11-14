from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v30/nyx/nyx_gui'
phrases={}
def add(*v):assert len(v)==7;phrases[v[0]]=list(v)
add('Saved pairing loaded. Bluetooth starts automatically.',
 'Appairages enregistrés chargés. Démarrage automatique du Bluetooth.',
 'Enlaces guardados cargados. Bluetooth se inicia automáticamente.',
 'Associazioni salvate caricate. Bluetooth si avvia automaticamente.',
 'Сохранённые сопряжения загружены. Bluetooth запускается автоматически.',
 'Gespeicherte Kopplungen geladen. Bluetooth startet automatisch.',
 'Emparelhamentos guardados carregados. Bluetooth inicia automaticamente.')
add('v30 · Background and responsiveness\nAutomatic wireless start without attached Joy-Con.\nBounded redraws and brief button press retention.',
 'v30 · Arrière-plan et réactivité\nBluetooth automatique sans Joy-Con sur les rails.\nRedessin limité et prise en compte des appuis brefs.',
 'v30 · Segundo plano y respuesta\nBluetooth automático sin Joy-Con acoplados.\nRedibujado limitado y recepción de pulsaciones breves.',
 'v30 · Attività in background e risposta\nBluetooth automatico senza Joy-Con collegati.\nRidisegno limitato e gestione delle pressioni brevi.',
 'v30 · Фоновая работа и отклик\nАвтозапуск Bluetooth без Joy-Con на направляющих.\nОграничение перерисовки и учёт коротких нажатий.',
 'v30 · Hintergrund und Reaktionszeit\nBluetooth-Autostart ohne angesteckte Joy-Con.\nBegrenzte Neuzeichnung und Erfassung kurzer Tastendrücke.',
 'v30 · Segundo plano e resposta\nBluetooth automático sem Joy-Con encaixados.\nRedesenho limitado e receção de toques breves.')
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
(root/'localization_nova30.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: new boot/changelog texts in seven languages; prior catalog retained.')
