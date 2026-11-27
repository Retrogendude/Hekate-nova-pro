"""Build the complete translated UI character subset, not a full CJK font."""
from pathlib import Path
import struct,json,hashlib,importlib.util
from PIL import Image,ImageDraw,ImageFont
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha1/nyx/nyx_gui/fonts'
spec=importlib.util.spec_from_file_location('converter',P/'tools/convert_font.py');c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
base=(P/'sd/Lecture.nyxf').read_bytes()
extra=json.loads((W/'nova34a1-zh-characters.json').read_text())
points=sorted(set(c.CODEPOINTS)|set(extra))
assert len(points)<=2048
source=W/'nova33-language-resources/NotoSansCJKsc-Regular.otf'
sections=[];report=[]
for height in c.HEIGHTS:
 entry=next(struct.unpack_from('<8I',base,32+i*32)for i in range(struct.unpack_from('<I',base,12)[0])if struct.unpack_from('<I',base,32+i*32)[0]==height)
 _,count,start,bitmap,*_=entry
 old={struct.unpack_from('<III',base,start+j*12)[0]:struct.unpack_from('<III',base,start+j*12)[1:]for j in range(count)}
 wanted=[cp for cp in points if cp not in old]
 font=ImageFont.truetype(str(source),height)
 baseline=round(height*.8)
 while True:
  boxes=[font.getbbox(chr(cp),anchor='ls')for cp in wanted]
  if min(b[1]for b in boxes)>=-baseline and max(b[3]for b in boxes)<=height-baseline:break
  font=font.font_variant(size=font.size-1)
  assert font.size>0
 missing=bytes(font.getmask(chr(0x10ffff)))
 descriptions=bytearray();pixels=bytearray()
 for cp in points:
  if cp in old:
   width,offset=old[cp];packed=base[bitmap+offset:bitmap+offset+((width+1)//2)*height]
  else:
   char=chr(cp);assert bytes(font.getmask(char))!=missing,hex(cp)
   left,top,right,bottom=font.getbbox(char,anchor='ls');x=-min(left,0)
   width=max(1,round(font.getlength(char))+x,right+x)
   image=Image.new('L',(width,height));ImageDraw.Draw(image).text((x,baseline),char,font=font,fill=255,anchor='ls')
   packed=c.unicode_generator.pack_4bpp(image)
   if char.strip():assert image.getbbox(),hex(cp)
  descriptions+=struct.pack('<III',cp,width,len(pixels));pixels+=packed
 data=descriptions+pixels;sections.append((height,len(descriptions),len(pixels),data))
 report.append({'height':height,'glyphs':len(points),'bitmap_bytes':len(pixels),'section_bytes':len(data),'cjk_font_pixels':font.size})
offset=32+32*len(sections);table=bytearray();payload=bytearray()
for height,desc_bytes,pixel_bytes,data in sections:
 end=offset+len(data);table+=struct.pack('<8I',height,len(points),offset,offset+desc_bytes,pixel_bytes,end,c.fnv(data),0);payload+=data;offset=end
header=struct.pack('<4sHH5I',b'NYXF',1,32,offset,len(sections),32,0,c.fnv(table));header+=struct.pack('<I',c.fnv(header))
blob=header+table+payload
assert len(blob)<=16*1024*1024 and max(x['section_bytes']for x in report)<=2*1024*1024
destination=P/'sd/Nova-Chinese-Simplified.nyxf';destination.write_bytes(blob)
(W/'nova34a1-chinese-font.json').write_text(json.dumps({'file':str(destination),'bytes':len(blob),'glyphs':len(points),'coverage':points,'sections':report,'sha256':hashlib.sha256(blob).hexdigest()},indent=2))
print(destination.name,len(blob),'bytes;',len(points),'glyphs;',len(sections),'sizes')
