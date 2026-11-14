from pathlib import Path
import re,subprocess
work=Path(__file__).resolve().parent
source=(work/'hekate-6.5.4/nyx/nyx_gui/frontend/gui_nova_reflection.c').read_text(encoding='utf-8')
defines='\n'.join(re.findall(r'^#define REFLECTION_(?:PERIOD|SWEEP|FIRST|CYCLE)[^\n]*',source,re.M))
a=source.index('static bool reflection_sweeping(');b=source.index('\n}',a)+2
folder=work/'reflection-timing-v17';folder.mkdir(exist_ok=True)
code='#include <stdbool.h>\n#include <stdint.h>\n#include <assert.h>\n#include <stdio.h>\ntypedef uint32_t u32;\n'+defines+'\n'+source[a:b]+r'''
int main(void){
 for(u32 cycle=0;cycle<100;++cycle){u32 offset=cycle*REFLECTION_CYCLE;
  for(u32 i=0;i<REFLECTION_CYCLE;++i){bool expected=i<12||(i>=162&&i<174);assert(reflection_sweeping(offset+i)==expected);}
 }assert(REFLECTION_PERIOD*REFLECTION_SWEEP==240);
 assert((REFLECTION_FIRST-REFLECTION_SWEEP)*REFLECTION_PERIOD==3000);
 assert((REFLECTION_CYCLE-REFLECTION_FIRST-REFLECTION_SWEEP)*REFLECTION_PERIOD==5000);
 puts("PASS: 42400 exact-C animation phases; 240ms sweep, alternating 3000/5000ms pauses.");return 0;
}
'''
(folder/'check.c').write_text(code,encoding='utf-8');exe=folder/'check.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(folder/'check.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
