"""Exercise the real bounded RAM/return-image reader with failing SD stubs."""
from pathlib import Path
import re
import subprocess

work = Path(__file__).resolve().parent
source = (work/'hekate-6.5.4/nyx/nyx_gui/nyx.c').read_text(encoding='utf-8')
match = re.search(r'static u8 \*_read_service_payload\([^\n]*\)\s*\n\{', source)
start = match.start()
end = source.index('\n}', match.end()) + 2
function = source[start:end]
harness = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;
typedef uint8_t u8;
typedef unsigned int UINT;
typedef int FRESULT;
typedef struct { u32 length; } FIL;
#define FR_OK 0
#define FA_READ 1
#define f_size(f) ((f)->length)
static u32 length;
static int open_error, alloc_error, read_error, close_error, short_read;
static int allocations, releases, opens, closes, reads, checks;
static int f_open(FIL *f,const char *p,int mode) {
 (void)p; (void)mode; opens++; f->length=length; return open_error;
}
static int f_close(FIL *f) { (void)f;closes++;return close_error; }
static int f_read(FIL *f,void *buffer,u32 size,UINT *received) {
 (void)f; reads++; memset(buffer,0x5a,size); *received=short_read?size-1:size;return read_error;
}
static void *allocate(size_t size) { allocations++; return alloc_error?NULL:malloc(size); }
static void release(void *p) { releases++;free(p); }
#define malloc allocate
#define free release
''' + function + r'''
#undef malloc
#undef free
#define CHECK(v) do {checks++;if(!(v)){fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#v);return 1;}}while(0)
static void reset(void) {
 length=256;open_error=alloc_error=read_error=close_error=short_read=0;
 allocations=releases=opens=closes=reads=0;
}
int main(void) {
 u32 limits[]={130392,131072};
 for(int i=0;i<2;i++) {
  u32 size;
  for(int error=0;error<8;error++) {
   reset();size=12345;
   if(error==0)open_error=1;
   if(error==1)length=255;
   if(error==2)length=limits[i]+1;
   if(error==3)length=0xffffffffu;
   if(error==4)alloc_error=1;
   if(error==5)read_error=1;
   if(error==6)short_read=1;
   if(error==7)close_error=1;
   u8 *p=_read_service_payload("test.bin",limits[i],&size);
   CHECK(!p);CHECK(size==12345);CHECK(opens==1);CHECK(closes==(error?1:0));
   if(error>=5) {CHECK(releases==1);CHECK(reads==1);}
   if(error>=1&&error<=3){CHECK(allocations==0);CHECK(reads==0);}
  }
  u32 sizes[]={256,limits[i]};
  for(int j=0;j<2;j++) {
   reset();length=sizes[j];size=0;
   u8 *p=_read_service_payload("test.bin",limits[i],&size);
   CHECK(p);CHECK(size==length);CHECK(p[0]==0x5a);CHECK(p[size-1]==0x5a);CHECK(closes==1);
   free(p);
  }
 }
 printf("PASS: %d actual-C bounded payload reader assertions, including both limits and every failure path.\n",checks);
 return 0;
}
'''
c = work/'payload_reader_generated.c'
exe = work/'payload_reader_generated.exe'
c.write_text(harness,encoding='utf-8')
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
