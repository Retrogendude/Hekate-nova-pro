from pathlib import Path
import json,re,struct,hashlib
from PIL import Image,ImageDraw,ImageFont
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.1/nyx/nyx_gui'
key='v33.1 · Interface and 8GB RAM\nIndependent Launch icon and background; refined settings and emuMMC frames.\nSeparate 8GB payload and capacity-aware RAM diagnostic (hardware validation pending).'
line=' - memory capacity mismatch; test stopped\n'
rows={key:[key,
'v33.1 · Interface et RAM 8 Go\nIcône et fond Launch indépendants ; cadres réglages et emuMMC corrigés.\nPayload 8 Go séparé et diagnostic adapté (validation matérielle à faire).',
'v33.1 · Interfaz y RAM de 8 GB\nIcono y fondo Launch independientes; marcos de ajustes y emuMMC corregidos.\nPayload de 8 GB separado y diagnóstico adaptado (pendiente de validar en consola).',
'v33.1 · Interfaccia e RAM da 8 GB\nIcona e sfondo Launch indipendenti; cornici impostazioni ed emuMMC corrette.\nPayload 8 GB separato e diagnostica adattata (da verificare su console).',
'v33.1 · Интерфейс и 8 ГБ ОЗУ\nОтдельные значок и фон Launch; исправлены рамки настроек и emuMMC.\nОтдельный загрузчик 8 ГБ и диагностика памяти (требуется проверка на консоли).',
'v33.1 · Oberfläche und 8 GB RAM\nUnabhängiges Launch-Symbol und Hintergrund; korrigierte Einstellungs- und emuMMC-Rahmen.\nSeparater 8-GB-Payload und angepasster Speichertest (Konsolentest ausstehend).',
'v33.1 · Interface e RAM de 8 GB\nÍcone e fundo Launch independentes; molduras de opções e emuMMC corrigidas.\nPayload de 8 GB separado e diagnóstico adaptado (validação na consola pendente).',
'v33.1 · 界面与 8GB 内存\n启动图标与背景独立设置；修正设置及 emuMMC 边框。\n独立的 8GB 引导文件及内存检测，尚待实机验证。'],
line:[line,' - capacité mémoire incohérente ; test arrêté\n',' - capacidad de memoria incompatible; prueba detenida\n',' - capacità memoria non coerente; test interrotto\n',' - несоответствие объёма памяти; тест остановлен\n',' - Speicherkapazität stimmt nicht überein; Test gestoppt\n',' - capacidade de memória incompatível; teste interrompido\n',' - 内存容量不匹配，测试已停止\n']}
catalog=json.loads((P/'localization_catalog.json').read_text(encoding='utf-8'));catalog.update(rows)
(P/'localization_catalog.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
quote=lambda s:json.dumps(s,ensure_ascii=False)
keys=sorted(catalog);lines=['/* Nova v33.1: en fr es it ru de pt zh-CN. */','static const locale_entry locale_catalog[] = {']
for k in keys:lines.append('    {'+quote(k)+', {'+', '.join(map(quote,catalog[k]))+'}},')
lines.append('};');patterns=[i for i,k in enumerate(keys)if '%'in k and len(re.findall('[A-Za-z]',k))>=5]
lines.append('static const uint16_t locale_pattern_rows[] = {'+', '.join(map(str,patterns))+'};')
first=[keys[i].encode()[0]for i in patterns];starts=[sum(ch<code for ch in first)for code in range(257)]
lines.append('static const uint16_t locale_pattern_start[257] = {'+', '.join(map(str,starts))+'};')
(P/'localization_catalog.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
(W/'nova33_1-zh-characters.json').write_text(json.dumps(sorted({ord(c)for v in catalog.values()for c in v[7]if ord(c)>=32})))
gui=P/'frontend/gui_nova.c';s=gui.read_text(encoding='utf-8')
marker='                "v33 · Languages and defaults\\nSimplified Chinese with its required font.\\nBundled wallpapers, icons and clearer boot options.",'
if quote(key) not in s:
 assert marker in s;s=s.replace(marker,'                '+quote(key)+',\n'+marker);gui.write_text(s,encoding='utf-8')
D=W/'nova33_1-ram-ui';D.mkdir(exist_ok=True)
def fnv(data):
 h=2166136261
 for b in data:h=((h^b)*16777619)&0xffffffff
 return h
for index,code in enumerate(('en','fr','es','it','ru','de','pt','zh')):
 data=(W/'nova33-ram-ui'/('ram-ui-'+code+'.bin')).read_bytes()
 magic,_,_,count,entries,strings,glyphs,fonts=struct.unpack_from('<8I',data)
 def string(off):return data[off:data.index(0,off)].decode()
 table={string(struct.unpack_from('<I',data,entries+i*8)[0]):string(struct.unpack_from('<I',data,entries+i*8+4)[0])for i in range(count)}
 table[line]=rows[line][index];stride=36 if code=='zh'else 12
 if code=='zh':
  points=sorted(set(range(32,127))|{ord(c)for kv in table.items()for t in kv for c in t if ord(c)>=32})
  font=ImageFont.truetype(str(W/'nova33-language-resources/NotoSansCJKsc-Regular.otf'),14);font_bytes=bytearray()
  for cp in points:
   im=Image.new('L',(16,16));ch=chr(cp);ImageDraw.Draw(im).text(((16-round(font.getlength(ch)))//2,13),ch,font=font,anchor='ls',fill=255)
   if ch.strip():assert im.getbbox()
   font_bytes+=struct.pack('<I16H',cp,*[sum(1<<x for x in range(16)if im.getpixel((x,y))>=88)for y in range(16)])
  glyphs=len(points)
 else:
  font_bytes=data[fonts:fonts+glyphs*stride]
  points={struct.unpack_from('<I',font_bytes,i*stride)[0]for i in range(glyphs)}
  assert {ord(c)for v in table.values()for c in v if ord(c)>=32}<=points,code
 count=len(table);fonts=32+count*8;strings=fonts+len(font_bytes);texts=bytearray();offsets={};entries_data=bytearray()
 def put(t):
  if t not in offsets:offsets[t]=strings+len(texts);texts.extend(t.encode()+b'\0')
  return offsets[t]
 for k,v in table.items():entries_data+=struct.pack('<II',put(k),put(v))
 body=entries_data+font_bytes+texts
 blob=struct.pack('<8I',magic,32+len(body),fnv(body),count,32,strings,glyphs,fonts)+body
 (D/('ram-ui-'+code+'.bin')).write_bytes(blob)
print('Updated catalog',len(catalog),'and all 8 RAM UI packs')
