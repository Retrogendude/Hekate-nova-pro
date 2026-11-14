"""Exercise the actual RAM UI decoder, formatter and framebuffer renderer."""
from pathlib import Path
import subprocess
from PIL import Image
WORK=Path(__file__).resolve().parent
HW=WORK/'HWTest-RCM-0.15'
OUT=WORK/'ram-locale-check';OUT.mkdir(exist_ok=True)
header=(HW/'third_party/hekate/bootloader/gfx/gfx.h').read_text()
header=header.replace('#include <bdk.h>', '#include <stdint.h>\n#include <utils/types.h>')
(OUT/'gfx.h').write_text(header)
code=r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ram_locale.h"
#include "gfx.h"
static unsigned cases;
#define CHECK(x) do { ++cases;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);} }while(0)
static u32 read32(const u8*p){return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static void put32(u8*p,u32 v){p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static void rehash(u8*p){u32 h=2166136261u;for(u32 i=32;i<read32(p+4);i++)h=(h^p[i])*16777619u;put32(p+8,h);}
static void utf8(const char*p){while(*p){u8 b=*p++;unsigned n=b<128?0:b<224?1:b<240?2:3;CHECK((b&0xC0)!=0x80);while(n--){CHECK(*p&&((u8)*p&0xC0)==0x80);p++;}}}
int main(int argc,char**argv){
 CHECK(argc==2);const char*codes[]={"en","fr","es","it","ru","de","pt"};
 const char*expected[]={"Advanced test","Test avancé","Prueba avanzada","Test avanzato","Расширенный тест","Erweiterter Test","Teste avançado"};
 static u32 pixels[1280*720+32];gfx_init_ctxt(pixels+16,1280,720,720);gfx_con_init();
 for(unsigned language=0;language<7;language++){
  char path[1024];snprintf(path,sizeof path,"%s/ram-ui-%s.bin",argv[1],codes[language]);
  FILE*f=fopen(path,"rb");CHECK(f!=NULL);fseek(f,0,SEEK_END);u32 size=ftell(f);rewind(f);
  u8*data=malloc(size),*copy=malloc(size);CHECK(data&&copy);CHECK(fread(data,1,size,f)==size);fclose(f);memcpy(copy,data,size);
  CHECK(ram_locale_bind(data,size));CHECK(!strcmp(ram_tr("Advanced test"),expected[language]));
  CHECK(!strcmp(ram_tr("unlisted hardware ID"),"unlisted hardware ID"));
  for(u32 cp=32;cp<127;cp++)CHECK(ram_ui_glyph(cp)!=NULL);
  CHECK(!ram_ui_glyph(0x110000));CHECK(!ram_ui_glyph(0xD800));
  for(u32 pos=32;pos<size&&pos<200;pos++){data[pos]^=1;CHECK(!ram_locale_bind(data,size));data[pos]^=1;}
  const u32 bad[][2]={{0,0},{4,31},{4,0x300001},{12,513},{16,0},{20,0},{24,1025},{28,1}};
  for(unsigned i=0;i<sizeof bad/sizeof*bad;i++){put32(data+bad[i][0],bad[i][1]);CHECK(!ram_locale_bind(data,size));memcpy(data,copy,size);}
  CHECK(!ram_locale_bind(data,size-1));CHECK(!ram_locale_bind(NULL,size));
  u32 entries=read32(data+16),fonts=read32(data+28);
  put32(data+entries,UINT32_MAX);rehash(data);CHECK(!ram_locale_bind(data,size));memcpy(data,copy,size);
  put32(data+fonts,0xD800);rehash(data);CHECK(!ram_locale_bind(data,size));memcpy(data,copy,size);
  CHECK(ram_locale_bind(data,size));put32(data+entries,UINT32_MAX);
  CHECK(ram_tr("Advanced test")!=NULL);memcpy(data,copy,size);CHECK(ram_locale_bind(data,size));
  char output[256];ram_ui_snprintf(output,sizeof output,"%s", "Advanced test");CHECK(!strcmp(output,expected[language]));
  for(unsigned capacity=0;capacity<70;capacity++){
   u8 guarded[128];memset(guarded,0xA5,sizeof guarded);
   ram_ui_snprintf((char*)guarded+8,capacity,"%s", "Advanced test");
   for(unsigned i=0;i<8;i++)CHECK(guarded[i]==0xA5);
   for(unsigned i=8+capacity;i<sizeof guarded;i++)CHECK(guarded[i]==0xA5);
   if(capacity){CHECK(memchr(guarded+8,0,capacity)!=NULL);utf8((char*)guarded+8);}
  }
  for(unsigned i=0;i<16;i++)pixels[i]=pixels[1280*720+16+i]=0xAABBCCDD;
  gfx_clear_color(0xFF141A19);gfx_con_setcol(0xFFF2F7F3,0,0);gfx_con_setpos(32,32);
  ram_ui_printf("[hekate] Diagnostic RAM");gfx_con_setpos(32,80);ram_ui_printf("Advanced test");
  gfx_con_setpos(32,128);ram_ui_printf("Run all seven RAM patterns.");gfx_con_setpos(32,176);
  ram_ui_printf("DRAM size     : %d MiB\n",4096u);gfx_con_setpos(32,224);ram_ui_printf("Training      : PASS\n");
  gfx_puts("\xE0\x80\x80\xED\xA0\x80\xF4\x90\x80\x80\x80");
  for(unsigned i=0;i<16;i++){CHECK(pixels[i]==0xAABBCCDD);CHECK(pixels[1280*720+16+i]==0xAABBCCDD);}
  snprintf(path,sizeof path,"ram-locale-%s.ppm",codes[language]);f=fopen(path,"wb");CHECK(f!=NULL);fprintf(f,"P6\n1280 720\n255\n");
  for(unsigned y=0;y<720;y++)for(unsigned x=0;x<1280;x++){u32 c=pixels[16+y+(1279-x)*720];u8 rgb[]={c>>16,c>>8,c};fwrite(rgb,1,3,f);}fclose(f);
  free(data);free(copy);ram_locale_init(false);CHECK(!strcmp(ram_tr("Advanced test"),"Advanced test"));
 }
 printf("PASS: %u actual-C RAM locale assertions; seven packs, bounds/checksum faults, mutated offsets, bounded UTF-8 formatting and real framebuffer canaries.\n",cases);return 0;
}
'''
(OUT/'check.c').write_text(code,encoding='utf-8')
gcc=WORK/'toolchains/w64devkit/bin/gcc.exe'
exe=OUT/'check.exe'
subprocess.run([str(gcc),'-std=gnu11','-O2','-DNYX_RAM_LOCALE','-I'+str(OUT),'-I'+str(HW/'ramtest/source'),'-I'+str(HW/'third_party/hekate/bdk'),str(OUT/'check.c'),str(HW/'ramtest/source/ram_locale.c'),str(HW/'gfx/gfx.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe),str(WORK/'ram-ui')],cwd=OUT,check=True)
for p in OUT.glob('*.ppm'): Image.open(p).save(p.with_suffix('.png'))
