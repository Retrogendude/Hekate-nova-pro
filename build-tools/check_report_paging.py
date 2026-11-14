"""Exercise the actual UTF-8 report pager at every multibyte boundary."""
from pathlib import Path
import re,subprocess
WORK=Path(__file__).resolve().parent
source=(WORK/'hekate-6.5.4/nyx/nyx_gui/frontend/gui_diagnostics.c').read_text(encoding='utf-8')
start=source.index('static void render_report_page(void)')
end=source.index('\n}',start)+2
function=source[start:end]
code=r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;typedef uint8_t u8;typedef int lv_obj_t;
#define REPORT_PAGE 12000u
static u32 report_page,report_length,report_start;
static lv_obj_t first,second;static lv_obj_t *report_label=&first,*report_position=&second;
static char text[REPORT_PAGE+8],position[80];static const char *data;
static const char *nyx_diag_report(void){return data;}
static const char *nyx_tr(const char *s){return s;}
static void lv_label_set_text(lv_obj_t*o,const char*s){if(o==report_label){assert(strlen(s)<sizeof text);strcpy(text,s);}else strcpy(position,s);}
#define s_printf sprintf
'''+function+r'''
int main(void){unsigned cases=0;
 for(unsigned shift=0;shift<8;shift++){
  const char *sample="Яé中";u32 n=REPORT_PAGE*4+shift+32;char *storage=malloc(n+40);char*joined=malloc(n+40);report_start=17;
  memset(storage,'Z',17);memset(storage+17,'A',shift);u32 used=shift;
  while(used+strlen(sample)<n){memcpy(storage+17+used,sample,strlen(sample));used+=strlen(sample);}
  storage[17+used]=0;data=storage;report_length=used;u32 total=0;
  for(report_page=0;report_page<(used+REPORT_PAGE-1)/REPORT_PAGE;report_page++){
   render_report_page();u32 count=strlen(text);assert(count<=REPORT_PAGE+3);memcpy(joined+total,text,count);total+=count;cases++;
  }
  assert(total==used&&!memcmp(joined,storage+17,used));cases++;free(storage);free(joined);
 }
 data=NULL;report_length=report_page=report_start=0;render_report_page();assert(text[0]);cases++;
 printf("PASS: %u actual-C report paging cases; adjacent UTF-8 pages reproduce every source byte exactly.\n",cases);return 0;
}
'''
directory=WORK/'report-paging-check';directory.mkdir(exist_ok=True)
p=directory/'check.c';p.write_text(code,encoding='utf-8');exe=directory/'check.exe'
subprocess.run([str(WORK/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-Wall','-Wextra','-Werror',str(p),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
