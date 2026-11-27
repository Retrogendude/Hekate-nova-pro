"""Actual C installation plan tests; no storage I/O."""
from pathlib import Path
import subprocess,json
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha3';D=W/'nova34a3-plan-check';D.mkdir(exist_ok=True)
c=r'''
#include <stdio.h>
#include <stdlib.h>
#include "gui_partition_plan.h"
static unsigned checks;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
int main(void){
 const u32 cards[]={4,8,16,32,64,128,256,512,1024},models[]={0,2,3};
 for(u32 actual=0;actual<4;++actual)for(u32 m=0;m<3;++m)for(u32 systems=0;systems<8;++systems)for(u32 c=0;c<9;++c){
  nova_partition_plan p={0};p.total_sectors=cards[c]*2097152u;p.fat_min_mib=2048;
  u32 emmc=actual==3?59640:29820;bool same=actual==models[m]||(actual<2&&models[m]==0);
  nova_plan_result result=nova_partition_installation(&p,actual,models[m],emmc,systems);
  C(!systems?result==NOVA_PLAN_INVALID:!same?result==NOVA_PLAN_MODEL:result==NOVA_PLAN_OK||result==NOVA_PLAN_CAPACITY);
  if(result!=NOVA_PLAN_OK)continue;
  C(p.emu_mib==((systems&1)?(actual==3?59680u:29856u):0u));C(!p.emu_double);
  C((p.linux_mib!=0)==((systems&2)!=0));C((p.android_mib!=0)==((systems&4)!=0));
  C(!p.linux_mib||p.linux_mib==16384||p.linux_mib==32768);C(!p.android_mib||p.android_mib==16384||p.android_mib==32768);
  if(systems&1){
   u32 index=p.linux_mib&&!p.android_mib?2:1;
   u32 start=0x8000u+(p.fat_mib+p.linux_mib+p.android_mib)*2048u;
   u32 size=p.emu_mib*2048u-0x800;
   u32 gpp=emmc*2048u;
   C(nova_install_target_valid(&p,p.total_sectors,gpp,index,0xE0,start,size));
   C(!nova_install_target_valid(&p,p.total_sectors,gpp,index,0x0B,start,size));
   C(!nova_install_target_valid(&p,p.total_sectors,gpp,index,0xE0,start+1,size));
   C(!nova_install_target_valid(&p,p.total_sectors,gpp,index,0xE0,start,size-1));
   C(!nova_install_target_valid(&p,p.total_sectors,gpp,index==1?2:1,0xE0,start,size));
   C(!nova_install_target_valid(&p,p.total_sectors,0,index,0xE0,start,size));
   C(!nova_install_target_valid(&p,p.total_sectors,size,index,0xE0,start,size));
   C(!nova_install_target_valid(&p,p.total_sectors-0x10000,gpp,index,0xE0,start,size));
   C(!nova_install_target_valid(NULL,p.total_sectors,gpp,index,0xE0,start,size));
   p.emu_double=true;C(!nova_install_target_valid(&p,p.total_sectors,gpp,index,0xE0,start,size));p.emu_double=false;
  }
  C(p.fat_mib>=2048);C((u64)p.fat_mib+p.emu_mib+p.linux_mib+p.android_mib+16==cards[c]*1024u);
 }
 nova_partition_plan p={0};p.total_sectors=128*2097152u;p.fat_min_mib=2048;
 C(nova_partition_installation(&p,0,0,0,2)==NOVA_PLAN_OK);C(!p.emu_mib&&p.linux_mib==32768);
 C(nova_partition_installation(&p,0,0,0,1)==NOVA_PLAN_EMMC_UNAVAILABLE);
 C(nova_partition_installation(&p,0,0,60000,1)==NOVA_PLAN_EMMC_UPGRADED);
 C(nova_partition_installation(&p,0,0,29820,8)==NOVA_PLAN_INVALID);
 C(nova_partition_installation(&p,4,0,29820,1)==NOVA_PLAN_INVALID);
 C(nova_partition_installation(&p,0,1,29820,1)==NOVA_PLAN_INVALID);
 C(nova_partition_installation(NULL,0,0,29820,1)==NOVA_PLAN_INVALID);
 p.total_sectors=32*2097152u;C(nova_partition_installation(&p,0,0,29820,2)==NOVA_PLAN_OK);C(p.linux_mib==16384);
 p.total_sectors=125000000u;C(nova_partition_installation(&p,0,0,29820,7)==NOVA_PLAN_CAPACITY);
 p.total_sectors=128*2097152u;C(nova_partition_installation(&p,0,0,29820,7)==NOVA_PLAN_OK);C(p.linux_mib==32768&&p.android_mib==32768);
 for(u32 hw=0;hw<4;++hw)for(u32 card=32;card<=128;card*=2)for(u32 mask=1;mask<8;++mask)for(u32 l=16;l<=32;l+=16)for(u32 a=16;a<=32;a+=16){
  nova_partition_plan q={0};q.total_sectors=card*2097152u;q.fat_min_mib=2048;
  nova_plan_result r=nova_partition_installation_sizes(&q,hw,hw<2?0:hw,hw==3?59640:29820,mask,l*1024,a*1024);
  u32 emu=(mask&1)?(hw==3?59680:29856):0,linux_size=(mask&2)?l*1024:0,android_size=(mask&4)?a*1024:0;
  bool fits=(u64)emu+linux_size+android_size+16+2048<=card*1024;
  C(r==(fits?NOVA_PLAN_OK:NOVA_PLAN_CAPACITY));C(q.linux_mib==linux_size&&q.android_mib==android_size&&q.emu_mib==emu);
 }
 p.total_sectors=32*2097152u;C(nova_partition_installation_sizes(&p,0,0,29820,2,32768,32768)==NOVA_PLAN_CAPACITY);
 C(p.linux_mib==32768);C(nova_partition_installation_sizes(&p,0,0,29820,2,16384,32768)==NOVA_PLAN_OK);
 C(nova_partition_installation_sizes(&p,0,0,29820,2,8192,32768)==NOVA_PLAN_INVALID);
 C(nova_partition_installation_sizes(&p,0,0,29820,4,16384,4096)==NOVA_PLAN_INVALID);
 C(nova_partition_installation_sizes(NULL,0,0,29820,1,16384,16384)==NOVA_PLAN_INVALID);
 printf("PASS: %u actual-C installation-plan assertions\n",checks);
}
'''
(D/'test.c').write_text(c)
subprocess.run([str(W/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-I'+str(P/'bdk'),'-I'+str(P/'nyx/nyx_gui/frontend'),str(D/'test.c'),str(P/'nyx/nyx_gui/frontend/gui_partition_plan.c'),'-o',str(D/'test.exe')],check=True)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
(W/'nova34a3-plan-check.json').write_text(json.dumps({'passed':True,'actual_c':True,'result':r.stdout.strip(),'physical_storage_test':False},indent=2))
