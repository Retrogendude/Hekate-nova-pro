from pathlib import Path
import ctypes as C,subprocess,io,zlib,struct,json
from PIL import Image
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4';folder=work/'png-check-v18';folder.mkdir(exist_ok=True)
(folder/'free.c').write_text('#include <stdlib.h>\nvoid png_test_free(void *p){free(p);}\n')
library=folder/'png.dll'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-shared','-std=c11','-O2','-Wall','-Wextra','-I'+str(root/'bdk'),str(root/'nyx/nyx_gui/frontend/nova_image.c'),str(root/'nyx/nyx_gui/libs/png/lodepng_nyx.c'),str(folder/'free.c'),'-o',str(library)],check=True)
lib=C.CDLL(str(library));decode=lib.nova_image_png;decode.argtypes=[C.c_void_p,C.c_uint,C.c_uint,C.c_uint,C.c_bool,C.POINTER(C.c_void_p)];decode.restype=C.c_bool
lib.png_test_free.argtypes=[C.c_void_p];lib.lodepng_malloc.argtypes=[C.c_size_t];lib.lodepng_malloc.restype=C.c_void_p
lib.lodepng_realloc.argtypes=[C.c_void_p,C.c_size_t];lib.lodepng_realloc.restype=C.c_void_p;lib.lodepng_free.argtypes=[C.c_void_p]
checks=0
def check(ok):
 global checks
 checks+=1;assert ok,checks
def decoded(data,w,h,cover):
 source=C.create_string_buffer(data);out=C.c_void_p()
 okay=decode(source,len(data),w,h,cover,C.byref(out))
 if not okay:check(not out.value);return None
 pixels=list(C.cast(out,C.POINTER(C.c_uint32))[:w*h]);lib.png_test_free(out);return pixels
image=Image.new('RGBA',(23,17));image.putdata([((x*11)%256,(y*17)%256,170,(0,64,128,255)[(x+y)%4])for y in range(17)for x in range(23)])
def png(im):
 b=io.BytesIO();im.save(b,format='PNG');return b.getvalue()
fixtures=[image,image.convert('RGB'),image.convert('LA'),image.convert('L'),image.quantize(colors=16)]
def chunk(kind,payload):return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',zlib.crc32(kind+payload)&0xFFFFFFFF)
def interlaced(im):
 data=b'';w,h=im.size
 for x0,y0,dx,dy in [(0,0,8,8),(4,0,8,8),(0,4,4,8),(2,0,4,4),(0,2,2,4),(1,0,2,2),(0,1,1,2)]:
  for y in range(y0,h,dy):
   xs=range(x0,w,dx)
   if xs:data+=b'\0'+b''.join(bytes(im.getpixel((x,y)))for x in xs)
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,6,0,0,1))+chunk(b'IDAT',zlib.compress(data))+chunk(b'IEND',b'')
encoded=[(png(im),im.convert('RGBA'))for im in fixtures]+[(interlaced(image),image)]
for data,reference in encoded:
 for w,h in [(23,17),(80,80),(4,9),(1280,720),(1,1)]:
  for cover in (False,True):
   actual=decoded(data,w,h,cover);check(actual is not None)
   sw,sh=reference.size;cw,ch,dw,dh=sw,sh,w,h
   if cover:
    ch=sw*h//w
    if ch>sh:ch=sh;cw=sh*w//h
   else:
    dh=w*sh//sw
    if dh>h:dh=h;dw=h*sw//sh
   cw,ch,dw,dh=[max(1,x)for x in (cw,ch,dw,dh)]
   sx,sy,ox,oy=(sw-cw)//2,(sh-ch)//2,(w-dw)//2,(h-dh)//2
   for y in range(h):
    for x in range(w):
     expected=0
     if ox<=x<ox+dw and oy<=y<oy+dh:
      r,g,b,a=reference.getpixel((sx+(x-ox)*cw//dw,sy+(y-oy)*ch//dh));expected=(a<<24)|(r<<16)|(g<<8)|b
     check(actual[y*w+x]==expected)
data=png(image)
for length in range(len(data)):check(decoded(data[:length],48,48,True)is None)
corrupt=bytearray(data);corrupt[-5]^=1;check(decoded(bytes(corrupt),48,48,True)is None)
large=data[:8]+chunk(b'IHDR',struct.pack('>IIBBBBB',1921,1080,8,6,0,0,0))+data[33:];check(decoded(large,48,48,True)is None)
for w,h in [(0,20),(20,0),(1281,1),(1,721),(0xFFFFFFFF,20)]:check(decoded(data,w,h,False)is None)
p=lib.lodepng_malloc(16);check(bool(p));C.memset(p,0x75,16)
check(not lib.lodepng_realloc(p,21*1024*1024));check(C.string_at(p,16)==b'\x75'*16)
q=lib.lodepng_realloc(p,64);check(bool(q)and C.string_at(q,16)==b'\x75'*16);lib.lodepng_free(q)
icons=work/'test-sd-v18/icone';icons.mkdir(parents=True,exist_ok=True)
for name in ['launch.png','configs.png','payloads.png','emummc.png','diagnostic.png']:image.save(icons/name)
(work/'png-check-v18.json').write_text(json.dumps({'assertions':checks,'formats':['RGBA','RGB','LA','L','palette+tRNS','Adam7 RGBA'],'alpha':'exact BGRA including zero, partial and opaque alpha','bounds':'source 1920x1080; target 1280x720; file 8 MiB; per-allocation 20 MiB','crc_truncation_dimensions_and_failed_growth':'checked'},indent=2))
print(f'PASS: {checks} PNG checks; exact alpha/scaling, palette/grayscale/Adam7, corrupt files, bounds and BDK-compatible realloc.')
