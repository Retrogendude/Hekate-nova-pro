from pathlib import Path
import json,re
root=Path(__file__).resolve().parent/'hekate-nova-pro-v25/nyx/nyx_gui'
phrases={}
def add(en,fr,es,it,ru,de,pt):phrases[en]=[en,fr,es,it,ru,de,pt]
add('v25 · Joy-Con reconnection\nCentral role requested on incoming links.\nConnection attempts serialized; reports retained.',
 'v25 · Reconnexion Joy-Con\nRôle principal demandé à la reconnexion.\nConnexions successives ; rapports conservés.',
 'v25 · Reconexión Joy-Con\nRol central solicitado al reconectar.\nConexiones sucesivas; informes conservados.',
 'v25 · Riconnessione Joy-Con\nRuolo centrale richiesto in ingresso.\nConnessioni successive; rapporti conservati.',
 'v25 · Переподключение Joy-Con\nЗапрос центральной роли при подключении.\nПодключения по очереди; отчёты сохранены.',
 'v25 · Joy-Con-Wiederverbindung\nZentralrolle bei eingehenden Verbindungen.\nVerbindungen nacheinander; Berichte erhalten.',
 'v25 · Reconexão Joy-Con\nPapel central pedido ao reconectar.\nLigações sucessivas; relatórios preservados.')
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
(root/'localization_nova25.json').write_text(json.dumps(phrases,ensure_ascii=False,indent=2),encoding='utf-8')
print(f'PASS: {len(phrases)} Bluetooth/changelog phrases in seven languages.')
