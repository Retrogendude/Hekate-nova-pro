"""Run the exact BMP decoder against valid, truncated and corrupted files."""
from pathlib import Path
import re,struct,subprocess
from PIL import Image
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4'
source=(root/'nyx/nyx_gui/frontend/gui_nova_theme.c').read_text()
def function(name):
    m=re.search(r'^static (?:u32|void|lv_img_dsc_t \*)\s*'+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert m,name
    end=re.search(r'^\}',source[m.end():],re.M);return source[m.start():m.end()+end.end()]
# A tiny one-line endian helper precedes the larger checked decoder.
u32=re.search(r'static u32 wallpaper_u32[^\n]*\n\{[^\n]*\}',source).group()
free=re.search(r'static void wallpaper_free[^\n]*\n\{[^\n]*\}',source).group()
directory=work/'nova-wallpaper-check';directory.mkdir(exist_ok=True)
for depth in (24,32):
    image=Image.new('RGB' if depth==24 else 'RGBA',(7,5),(200,30,50) if depth==24 else (200,30,50,0))
    image.save(directory/f'red{depth}.bmp')
image=Image.new('RGB',(7,5));image.paste((255,0,0),(0,0,7,2));image.paste((0,0,255),(0,2,7,5));image.save(directory/'orientation.bmp')
raw=(directory/'orientation.bmp').read_bytes();offset=struct.unpack_from('<I',raw,10)[0];stride=(7*3+3)&~3
top=bytearray(raw);struct.pack_into('<i',top,22,-5);top[offset:]=b''.join(raw[offset+y*stride:offset+(y+1)*stride]for y in range(4,-1,-1));(directory/'top-down.bmp').write_bytes(top)
bad=[]
def bad_file(name,data):
    (directory/name).write_bytes(data);bad.append(name)
base=(directory/'red24.bmp').read_bytes()
for cut in (0,1,2,10,26,53,54,len(base)-1):bad_file(f'truncated-{cut}.bmp',base[:cut])
for field,fmt,values in [(0,'2s',[b'XX']),(14,'I',[0,12,39]),(18,'I',[0,1921,0xFFFFFFFF]),(22,'i',[0,1081,-1081,-2147483648]),(26,'H',[0,2]),(28,'H',[1,8,16]),(30,'I',[1,2,3]),(10,'I',[0,53,len(base)+1]),(2,'I',[0,54,len(base)+1])]:
    for value in values:
        b=bytearray(base);struct.pack_into('<'+fmt,b,field,value);bad_file(f'bad-{field}-{len(bad)}.bmp',b)
preamble=r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8;typedef uint32_t u32;typedef unsigned UINT;
typedef struct{struct{u32 cf,w,h;}header;u32 data_size;const u8*data;}lv_img_dsc_t;
typedef struct{FILE*fp;u32 size;}FIL;
#define FR_OK 0
#define FA_READ 1
#define LV_IMG_CF_TRUE_COLOR 4
#define s_printf sprintf
#define f_size(f) ((f)->size)
static int files_open,reads,seeks,fail_read,fail_seek,fail_alloc,allocations;
static char fixture_root[1024];
static int f_open(FIL*f,const char*path,int flags){(void)flags;char full[1200];snprintf(full,sizeof(full),"%s/%s",fixture_root,strrchr(path,'/')+1);f->fp=fopen(full,"rb");if(!f->fp)return 1;fseek(f->fp,0,SEEK_END);f->size=ftell(f->fp);rewind(f->fp);files_open++;return 0;}
static int f_close(FIL*f){files_open--;return fclose(f->fp);}
static int f_read(FIL*f,void*p,u32 bytes,UINT*actual){if(++reads==fail_read){*actual=0;return 1;}*actual=fread(p,1,bytes,f->fp);return ferror(f->fp)?1:0;}
static int f_lseek(FIL*f,u32 offset){if(++seeks==fail_seek)return 1;return fseek(f->fp,offset,SEEK_SET);}
static void*test_malloc(size_t n){if(++allocations==fail_alloc)return NULL;return malloc(n);}
static void*test_calloc(size_t n,size_t size){if(++allocations==fail_alloc)return NULL;return calloc(n,size);}
#define malloc test_malloc
#define calloc test_calloc
'''
cases='\nstatic const char*bad_files[]={'+','.join('"'+x+'"'for x in bad)+'};\n'
tests=r'''
static unsigned checks;
#define CHECK(x)do{checks++;assert(x);}while(0)
static lv_img_dsc_t*decode(const char*file,int w,int h){reads=seeks=allocations=0;return wallpaper_read(file,w,h);}
int main(int argc,char**argv){assert(argc==2);strcpy(fixture_root,argv[1]);
 for(int depth=0;depth<2;++depth)for(int size=0;size<3;++size){char file[32];sprintf(file,"red%d.bmp",depth?32:24);u32 w=size==0?32:size==1?516:1280,h=size==0?18:size==1?405:720;lv_img_dsc_t*i=decode(file,w,h);CHECK(i&&i->header.w==w&&i->header.h==h&&i->data_size==w*h*4);const u32*p=(const u32*)i->data;for(u32 n=0;n<w*h;++n)assert(p[n]==0xFFC81E32);CHECK(!files_open&&allocations==3);wallpaper_free(i);}
 lv_img_dsc_t*bottom=decode("orientation.bmp",7,5),*top=decode("top-down.bmp",7,5);CHECK(bottom&&top&&!memcmp(bottom->data,top->data,140));CHECK(((u32*)bottom->data)[0]==0xFFFF0000&&((u32*)bottom->data)[34]==0xFF0000FF);wallpaper_free(bottom);wallpaper_free(top);
 for(u32 n=0;n<sizeof(bad_files)/sizeof(*bad_files);++n){CHECK(!decode(bad_files[n],32,18));CHECK(!files_open);}
 for(int n=1;n<=3;++n){fail_alloc=n;CHECK(!decode("red24.bmp",32,18));CHECK(!files_open);}fail_alloc=0;
 lv_img_dsc_t*valid=decode("red24.bmp",32,18);assert(valid);int read_count=reads,seek_count=seeks;wallpaper_free(valid);
 for(int n=1;n<=read_count;++n){fail_read=n;CHECK(!decode("red24.bmp",32,18));CHECK(!files_open);}fail_read=0;
 for(int n=1;n<=seek_count;++n){fail_seek=n;CHECK(!decode("red24.bmp",32,18));CHECK(!files_open);}fail_seek=0;
 const char*names[]={"","../red24.bmp","a/red24.bmp","a\\red24.bmp","missing.bmp"};for(u32 i=0;i<5;++i){CHECK(!decode(names[i],32,18));CHECK(!files_open);}
 printf("PASS: %u actual-C wallpaper checks; 24/32-bit BMP, portrait/top-down orientation, crop/scale, opaque output, bad headers/truncation, allocation/read/seek failures and handle cleanup.\n",checks);return 0;
}
'''
program=directory/'check.c';program.write_text(preamble+u32+'\n'+free+'\n'+function('wallpaper_read')+cases+tests)
cc=work/'toolchains/w64devkit/bin/gcc.exe';exe=directory/'check.exe'
subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2',str(program),'-o',str(exe)],check=True)
subprocess.run([str(exe),str(directory)],check=True)
