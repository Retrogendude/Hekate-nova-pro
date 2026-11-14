from pathlib import Path
import subprocess,json
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4'
folder=work/'ram-modes-v17';folder.mkdir(exist_ok=True)
header=project/'nyx/nyx_gui/diagnostics/ram_handoff.h'
assert header.read_bytes()==(work/'HWTest-RCM-0.15/ramtest/source/nyx_ram_handoff.h').read_bytes()
code=r'''
#include <stdio.h>
#include <string.h>
#include "ram_handoff.h"
static unsigned checks;
#define CHECK(c) do{++checks;if(!(c)){printf("FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void){
 for(u32 mode=0;mode<4;++mode)for(u32 mask=0;mask<256;++mask){
  bool expected=mode==0?mask==1:mode==1?mask==127:mode==2?mask>0&&mask<128:false;
  CHECK(nyx_ram_selection_valid(mode,mask)==expected);
  nyx_ram_handoff_t h={0};h.magic=NYX_RAM_HANDOFF_MAGIC;h.version=NYX_RAM_VERSION;h.payload_size=109864;h.payload_hash=123;h.auto_run=1;h.mode=mode;h.selected_mask=mask;h.request_hash=nyx_ram_request_hash(&h);
  CHECK(nyx_ram_request_valid(&h)==expected);
  if(!expected)continue;
  nyx_ram_handoff_t good=h;
  u32 *words=(u32 *)&h;
  for(u32 i=0;i<sizeof(h)/4;++i){if(i>4&&i!=14&&i!=15&&i!=19)continue;h=good;words[i]^=0x80000000u;CHECK(!nyx_ram_request_valid(&h));}
  h=good;h.result_magic=NYX_RAM_RESULT_MAGIC;h.result=NYX_RAM_RESULT_PASS;h.completed_mask=mask;h.patterns_completed=h.patterns_total=nyx_ram_count(mask);h.tested_mib=h.verified_mib=4092;h.dram_mib=4096;h.result_hash=nyx_ram_hash(&h.result,NYX_RAM_RESULT_WORDS*4);
  CHECK(nyx_ram_result_valid(&h));good=h;
  for(u32 i=0;i<NYX_RAM_RESULT_WORDS;++i){h=good;((u32 *)&h.result)[i]^=1u;CHECK(!nyx_ram_result_valid(&h));}
  h=good;h.result=NYX_RAM_RESULT_ABORT;h.completed_mask=0;h.patterns_completed=0;h.result_hash=nyx_ram_hash(&h.result,NYX_RAM_RESULT_WORDS*4);CHECK(nyx_ram_result_valid(&h));
  h.result=NYX_RAM_RESULT_PASS;h.result_hash=nyx_ram_hash(&h.result,NYX_RAM_RESULT_WORDS*4);CHECK(!nyx_ram_result_valid(&h));
 }
 CHECK(sizeof(nyx_ram_handoff_t)<4096);
 CHECK(NYX_RAM_UI_ADDR+NYX_RAM_UI_MAX<=NYX_RAM_HANDOFF_ADDR);
 CHECK(NYX_RAM_RETURN_ADDR+NYX_RAM_RETURN_MAX<=0x80400000u);
 printf("PASS: %u actual-C RAM mode/contract assertions; invalid selections, tampering, partial results and protected ranges.\n",checks);return 0;
}
'''
(folder/'check.c').write_text(code,encoding='utf-8')
exe=folder/'check.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-Wall','-Wextra','-Werror','-I'+str(header.parent),'-I'+str(project/'bdk'),str(folder/'check.c'),'-o',str(exe)],check=True)
result=subprocess.run([str(exe)],capture_output=True,text=True);print(result.stdout,end='');assert not result.returncode
(work/'ram-modes-v17.json').write_text(json.dumps({'summary':result.stdout.strip(),'hardware_io':'not executed','handoff_version':2,'reserved_first_mib':4},indent=2),encoding='utf-8')
