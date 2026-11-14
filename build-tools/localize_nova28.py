from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v28/nyx/nyx_gui'
phrases={}
def add(en,fr,es,it,ru,de,pt):phrases[en]=[en,fr,es,it,ru,de,pt]
add('v28 · Input responsiveness\nShort Bluetooth polls while drawing menus.\nPairing import and button reception diagnostics.',
 'v28 · Réactivité des commandes\nLectures Bluetooth courtes pendant le dessin.\nDiagnostic des appairages et des boutons reçus.',
 'v28 · Respuesta de los mandos\nLecturas Bluetooth cortas al dibujar menús.\nDiagnóstico de enlaces y botones recibidos.',
 'v28 · Reattività dei comandi\nLetture Bluetooth brevi durante il disegno.\nDiagnostica di associazioni e pulsanti ricevuti.',
 'v28 · Отклик управления\nКороткий опрос Bluetooth при отрисовке меню.\nДиагностика сопряжения и приёма кнопок.',
 'v28 · Reaktionszeit der Steuerung\nKurze Bluetooth-Abfragen beim Zeichnen.\nDiagnose von Kopplung und Tastenempfang.',
 'v28 · Resposta dos comandos\nLeituras Bluetooth curtas ao desenhar menus.\nDiagnóstico de emparelhamento e botões.')
add('Rail connection not ready. Attach both Joy-Con and retry.',
 'Connexion aux rails indisponible. Fixez les deux Joy-Con et réessayez.',
 'Conexión a los raíles no disponible. Acopla ambos Joy-Con y reintenta.',
 'Collegamento alle guide non pronto. Collega entrambi i Joy-Con e riprova.',
 'Соединение с направляющими не готово. Подключите оба Joy-Con и повторите.',
 'Schienenverbindung nicht bereit. Beide Joy-Con anstecken und erneut versuchen.',
 'Ligação aos trilhos indisponível. Encaixe ambos os Joy-Con e tente novamente.')
add('Pairing data could not be read from the rails.',
 'Impossible de lire les données d’appairage sur les rails.',
 'No se pudieron leer los datos de enlace desde los raíles.',
 'Impossibile leggere i dati di associazione dalle guide.',
 'Не удалось прочитать данные сопряжения через направляющие.',
 'Kopplungsdaten konnten nicht über die Schienen gelesen werden.',
 'Não foi possível ler os dados de emparelhamento pelos trilhos.')
add('Nintendo pairing data not recognized by the reader.',
 'Le lecteur ne reconnaît pas les données d’appairage Nintendo.',
 'El lector no reconoce los datos de enlace de Nintendo.',
 'Il lettore non riconosce i dati di associazione Nintendo.',
 'Считыватель не распознал данные сопряжения Nintendo.',
 'Nintendo-Kopplungsdaten werden vom Leser nicht erkannt.',
 'O leitor não reconhece os dados de emparelhamento Nintendo.')
add('Pairing records did not pass compatibility checks.',
 'Les données d’appairage n’ont pas passé les contrôles de compatibilité.',
 'Los datos de enlace no superaron las comprobaciones de compatibilidad.',
 'I dati di associazione non hanno superato i controlli di compatibilità.',
 'Данные сопряжения не прошли проверку совместимости.',
 'Kopplungsdaten haben die Kompatibilitätsprüfung nicht bestanden.',
 'Os dados de emparelhamento não passaram as verificações de compatibilidade.')
# Preserve the existing catalog and regenerate its pattern indices after sorting.
catalog=root/'localization_catalog.inc';lines={}
for line in catalog.read_text(encoding='utf-8').splitlines():
    if line.startswith('    {"'):lines[json.JSONDecoder().raw_decode(line.strip()[1:])[0]]=line
for key,values in phrases.items():
    assert len(values)==7
    lines[key]='    {'+json.dumps(key,ensure_ascii=False)+', {'+', '.join(json.dumps(v,ensure_ascii=False)for v in values)+'}},'
keys=sorted(lines);patterns=[i for i,k in enumerate(keys)if '%'in k and len(re.findall(r'[A-Za-z]',k))>=5]
first=[keys[i].encode('utf-8')[0]for i in patterns]
tail='static const uint16_t locale_pattern_rows[] = {'+', '.join(map(str,patterns))+'};\n'
tail+='static const uint16_t locale_pattern_start[257] = {'+', '.join(str(sum(ch<code for ch in first))for code in range(257))+'};\n'
catalog.write_text('/* Nova catalog: en fr es it ru de pt. */\nstatic const locale_entry locale_catalog[] = {\n'+'\n'.join(lines[k]for k in keys)+'\n};\n'+tail,encoding='utf-8')
data=json.loads((root/'localization_catalog.json').read_text(encoding='utf-8'));data.update(phrases)
(root/'localization_catalog.json').write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
(root/'localization_nova28.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print(f'PASS: {len(phrases)} Bluetooth/changelog phrases in seven languages.')
