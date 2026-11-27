from pathlib import Path
from zipfile import ZipFile
import json,struct,hashlib,types,re
from PIL import Image,ImageFont,ImageDraw
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33/nyx/nyx_gui'
source=(W/'audit_localization_coverage.py').read_text(encoding='utf-8')
source=re.sub(r'\b7\b','8',source)
helpers=types.ModuleType('nova33_locale_composition');helpers.__file__=str(W/'audit_localization_coverage.py');exec(compile(source,helpers.__file__,'exec'),helpers.__dict__)
catalog=json.loads((P/'localization_catalog.json').read_text(encoding='utf-8'))
directory=W/'nova33-ram-ui';directory.mkdir(exist_ok=True)
with ZipFile(W.parent/'outputs/hekate-nova-pro-v32.zip')as z:
 for code in ('en','fr','es','it','ru','de','pt'):
  name='ram-ui-'+code+'.bin';(directory/name).write_bytes(z.read('bootloader/sys/'+name))
base=(directory/'ram-ui-en.bin').read_bytes();_,size,_,count,entries,strings,_,_=struct.unpack_from('<8I',base)
def string(offset):return base[offset:base.index(0,offset)].decode('utf-8')
keys=[string(struct.unpack_from('<I',base,entries+i*8)[0])for i in range(count)]
translated={};missing=[]
for key in keys:
 value=helpers.compose(key,catalog)
 if value is None:
  if not helpers.technical(key):missing.append(key)
  value=[key]*8
 translated[key]=value[7]
assert not missing,missing
points=sorted(set(range(32,127))|{ord(c)for key,value in translated.items()for s in (key,value)for c in s if ord(c)>=32})
assert len(points)<=2048
font=ImageFont.truetype(str(W/'nova33-language-resources/NotoSansCJKsc-Regular.otf'),14)
font_bytes=bytearray()
for cp in points:
 char=chr(cp);left,top,right,bottom=font.getbbox(char,anchor='ls')
 image=Image.new('L',(16,16));x=(16-round(font.getlength(char)))//2
 ImageDraw.Draw(image).text((x,13),char,font=font,anchor='ls',fill=255)
 if char.strip():assert image.getbbox(),hex(cp)
 rows=[sum((1<<x)for x in range(16)if image.getpixel((x,y))>=88)for y in range(16)]
 font_bytes+=struct.pack('<I16H',cp,*rows)
entries_offset=32;font_offset=32+len(keys)*8;strings_offset=font_offset+len(font_bytes)
string_bytes=bytearray();offsets={};table=bytearray()
def put(s):
 assert len(s.encode())<=2048
 if s not in offsets:offsets[s]=strings_offset+len(string_bytes);string_bytes.extend(s.encode()+b'\0')
 return offsets[s]
for key in keys:table+=struct.pack('<II',put(key),put(translated[key]))
body=table+font_bytes+string_bytes;h=2166136261
for b in body:h=((h^b)*16777619)&0xffffffff
blob=struct.pack('<8I',0x3255524e,32+len(body),h,len(keys),entries_offset,strings_offset,len(points),font_offset)+body
assert len(blob)<=0x300000
(directory/'ram-ui-zh.bin').write_bytes(blob)
(directory/'translations-zh.json').write_text(json.dumps(translated,ensure_ascii=False,indent=2),encoding='utf-8')
(W/'nova33-ram-ui.json').write_text(json.dumps({'entries':len(keys),'glyphs':len(points),'glyph_pixels':16,'bytes':len(blob),'sha256':hashlib.sha256(blob).hexdigest(),'missing_translations':missing},indent=2))
print('Chinese RAM UI:',len(keys),'entries;',len(points),'glyphs;',len(blob),'bytes; high-resolution 16x16')
