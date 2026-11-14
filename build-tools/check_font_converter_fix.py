"""Reproduce Caviar's accent failure, then check fitting and real NYXF loading."""
from pathlib import Path
from PIL import ImageFont,Image,ImageDraw
import importlib.util,subprocess,sys,struct,json
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui/fonts';folder=work/'font-converter-fix'
caviar=Path('C:/Users/Retrogen/Desktop/hekate-nova-pro-v11/outils-polices/Caviar.ttf')
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,path);loaded=importlib.util.module_from_spec(spec);spec.loader.exec_module(loaded);return loaded
old=module('previous_generator',folder/'original/generate_unicode_fonts.py');new=module('fixed_converter',root/'tools/convert_font.py')
font=ImageFont.truetype(str(caviar),20)
try:old.render_glyph(font,'A',20)
except AssertionError:pass
else:raise AssertionError('The reported Caviar failure was not reproduced')
checks=0
for height in new.HEIGHTS:
 selected=ImageFont.truetype(str(caviar),height);fitted=new.unicode_generator.fit_font_to_cell(selected,height)
 assert 0<-fitted.getbbox('H',anchor='ls')[1]<round(height*0.8)
 fallback=ImageFont.truetype(str(root/'source/NotoSans-Regular.ttf'),height)
 for style in ('regular','strong','fixed'):
  for cp in map(ord,' AagjpQÀÂÄÇÉÊÑÖÙŸœЖЙЁя'):
   image=new.glyph(fitted,fallback,cp,height,style)
   assert image.height==height and 0<image.width<=128
   if cp!=32:assert image.getbbox(),(height,style,cp)
   packed=new.unicode_generator.pack_4bpp(image);decoded=new.unicode_generator.unpack_4bpp(packed,image.width,height)
   assert len(packed)==((image.width+1)//2)*height
   assert all(abs(a-b)<=8 for a,b in zip(image.tobytes(),decoded.tobytes()))
   checks+=1
regenerated=folder/'Lecture.nyxf'
subprocess.run([sys.executable,str(root/'tools/convert_font.py'),str(root/'source/NotoSans-Regular.ttf'),'-o',str(regenerated)],check=True)
assert regenerated.read_bytes()==(root/'sd/Lecture.nyxf').read_bytes(),'Existing Noto conversion changed'
subprocess.run([sys.executable,str(root/'tools/convert_font.py'),str(caviar),'-o',str(work.parent/'outputs/Caviar.nyxf')],check=True)
data=(work.parent/'outputs/Caviar.nyxf').read_bytes();assert data[:4]==b'NYXF'
assert struct.unpack_from('<I',data,8)[0]==len(data)
count,table_offset=struct.unpack_from('<II',data,12);assert count==12 and table_offset==32
assert new.fnv(data[:28])==struct.unpack_from('<I',data,28)[0]
assert new.fnv(data[32:32+32*count])==struct.unpack_from('<I',data,24)[0]
heights=[]
for i in range(count):
 height,glyphs,start,bitmap,byte_count,end,checksum,reserved=struct.unpack_from('<8I',data,32+i*32)
 assert glyphs==len(new.CODEPOINTS)and not reserved and bitmap-start==glyphs*12
 assert end-bitmap==byte_count and end<=len(data)and new.fnv(data[start:end])==checksum
 expected_offset=0;points=[]
 for j in range(glyphs):
  cp,width,offset=struct.unpack_from('<III',data,start+j*12);assert 0<width<=128 and offset==expected_offset
  expected_offset+=((width+1)//2)*height;points.append(cp)
 assert expected_offset==byte_count and points==list(new.CODEPOINTS);heights.append(height)
assert tuple(heights)==new.HEIGHTS
bad=subprocess.run([sys.executable,str(root/'tools/convert_font.py'),str(folder/'absent.ttf'),'-o',str(folder/'absent.nyxf')],capture_output=True,text=True)
assert bad.returncode==1 and 'Conversion failed:'in bad.stderr and 'Traceback'not in bad.stderr and not(folder/'absent.nyxf').exists()
subprocess.run([sys.executable,str(work/'check_font_manager.py')],check=True)
loader=subprocess.check_output([str(work/'font-manager-check/test.exe'),str(work.parent/'outputs/Caviar.nyxf')],text=True).strip()
assert loader.startswith('PASS:');print(loader)
# A decoded bitmap preview uses exactly the exported 4bpp data.
image=Image.new('RGB',(1120,500),'#10151D');draw=ImageDraw.Draw(image);heading=ImageFont.truetype(str(root/'source/NotoSans-Regular.ttf'),22)
draw.text((26,16),'Caviar — fichier NYXF décodé, accents et secours cyrillique',font=heading,fill='#F2F5FA')
sample='Caviar 0123456789 · ÀÉÊÇ œ ç é · Français Español · Русский'
for index,height in enumerate((20,24,30,36)):
 entry=next(struct.unpack_from('<8I',data,32+i*32)for i in range(count)if struct.unpack_from('<I',data,32+i*32)[0]==height)
 _,glyphs,start,bitmap,*_=entry;lookup={struct.unpack_from('<I',data,start+j*12)[0]:struct.unpack_from('<III',data,start+j*12)[1:]for j in range(glyphs)}
 y=76+index*100;draw.text((26,y),f'{height}px',font=heading,fill='#55C9F1');x=112
 for char in sample:
  width,offset=lookup[ord(char)];packed=data[bitmap+offset:bitmap+offset+((width+1)//2)*height]
  mask=new.unicode_generator.unpack_4bpp(packed,width,height);image.paste('#F2F5FA',(x,y),mask);x+=width+1
image.save(work.parent/'outputs/apercu-Caviar-police-corrigee.png')
summary={'reported_failure_reproduced':True,'font':'Caviar.ttf','glyphs':len(new.CODEPOINTS),'sizes':heights,'representative_glyph_style_checks':checks,'existing_noto_output_byte_identical':True,'nyxf_checksums_and_layout_valid':True,'real_c_loader':loader,'firmware_code_changed':False}
(folder/'verification.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(f'PASS: converter fix; {checks} accent/Cyrillic/style checks, 12 valid Caviar sizes, unchanged Noto output, friendly error and actual-C loader verified.')
