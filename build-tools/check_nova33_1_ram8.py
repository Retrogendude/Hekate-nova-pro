"""Compile the real bridge, handoff and SMMU mapping for host-only 4/8GB checks."""
from pathlib import Path
import re, subprocess, json, hashlib
W=Path(__file__).resolve().parent
S=W/'HWTest-RCM-nova33_1/ramtest/source'
B=W/'HWTest-RCM-0.15/third_party/hekate/bdk'
D=W/'nova33_1-8gb-check';D.mkdir(exist_ok=True)
gcc=W/'toolchains/w64devkit/bin/gcc.exe';gpp=gcc.with_name('g++.exe')
def extract(text,name):
 start=re.search(r'^(?:static )?[\w *]+\b'+name+r'\(',text,re.M).start()
 opening=text.index('{',start);end=opening+1;depth=1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
(D/'exosphere.hpp').write_text('#include <stdint.h>\nusing u8=uint8_t;using u32=uint32_t;\n')
cpp=r'''
#include <cstdio>
#include <cstdlib>
#include "exosphere.hpp"
#include "ramtest_mtc.h"
static int checks,called,index_seen,periodic,restored,raw_reads;
static u32 effective_id;
#define C(x) do{++checks;if(!(x)){std::fprintf(stderr,"bridge line%d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
extern "C" u32 fuse_read_dramid(int raw){raw_reads+=raw;return effective_id;}
namespace ams::nxboot {
void DoMemoryTrainingErista(int index,void*){called=1;index_seen=index;}
void DoMemoryTrainingMariko(bool*out,int index,void*){called=2;index_seen=index;*out=true;}
void DoPeriodicTrainingErista(void*){periodic=1;}void DoPeriodicTrainingMariko(void*){periodic=2;}
void RestoreMemoryClockRateErista(void*){restored=1;}void RestoreMemoryClockRateMariko(void*){restored=2;}
u32 GetMemoryTrainingErrorsErista(){return 0;}u32 GetMemoryTrainingErrorsMariko(){return 0;}
u32 GetMemoryTrainingFailedMaskErista(){return 0;}u32 GetMemoryTrainingFailedMaskMariko(){return 0;}
bool DidMemoryTrainingErista(){return true;}
}
static void good(u32 platform,u32 mb,u32 id,int index){
 called=periodic=restored=0;effective_id=id;ram_mtc_select_platform(platform);
 C(ram_mtc_set_capacity(mb)==0);C(ram_mtc_train()==0);C(ram_mtc_is_trained());
 C(called==(int)platform);C(index_seen==index);C(ram_mtc_dram_id()==id);C(!raw_reads);
 C(ram_mtc_check_capacity(mb)==0);ram_mtc_periodic();C(periodic==(int)platform);
 ram_mtc_restore();C(restored==(int)platform);C(!ram_mtc_is_trained());
}
int main(){
 good(1,4096,0,0);good(1,4096,1,2);good(1,4096,2,3);good(1,6144,4,1);
 good(1,8192,7,1);good(1,8192,4,1);
 const int ids[]={9,13,18,23,28},idx[]={6,6,13,13,13};
 for(int i=0;i<5;++i)good(2,8192,ids[i],idx[i]);
 good(2,4096,3,16);good(2,4096,8,5);good(2,4096,24,12);good(2,4096,34,20);
 for(u32 platform=1;platform<=2;++platform){
  ram_mtc_select_platform(platform);C(ram_mtc_set_capacity(2048)<0);C(ram_mtc_errors()&RAM_MTC_ERR_CAPACITY);
  ram_mtc_select_platform(platform);C(ram_mtc_set_capacity(4096)==0);effective_id=platform==1?7:28;
  called=0;C(ram_mtc_train()<0);C(!called);C(ram_mtc_errors()&RAM_MTC_ERR_UNSUPPORTED);
  good(platform,8192,platform==1?7:28,platform==1?1:13);
  ram_mtc_select_platform(platform);ram_mtc_set_capacity(8192);ram_mtc_train();
  C(ram_mtc_check_capacity(4096)<0);C(!ram_mtc_is_trained());C(ram_mtc_errors()&RAM_MTC_ERR_CAPACITY);
  periodic=restored=0;ram_mtc_periodic();ram_mtc_restore();C(!periodic&&!restored);
  ram_mtc_select_platform(platform);ram_mtc_set_capacity(8192);effective_id=999;called=0;
  C(ram_mtc_train()<0);C(!called);
 }
 ram_mtc_select_platform(0);C(ram_mtc_train()<0);
 std::printf("PASS bridge: %d checks\n",checks);
}
'''
(D/'bridge.cpp').write_text(cpp)
subprocess.run([str(gpp),'-std=c++17','-Wall','-Wextra','-Werror','-I'+str(D),'-I'+str(S),str(D/'bridge.cpp'),str(S/'ramtest_mtc_universal.cpp'),'-o',str(D/'bridge.exe')],check=True)
bridge=subprocess.check_output([str(D/'bridge.exe')],text=True);print(bridge.strip())
main=(S/'ramtest_main.c').read_text();contract=(S/'nyx_ram_handoff.h').read_text().replace('#include <utils/types.h>','')
smmu=(B/'mem/smmu.c').read_text();header=(B/'mem/smmu.h').read_text()
pde=header[header.index('typedef struct _pde_t'):header.index('typedef struct _pte_t')]
maps=(B/'memory_map.h').read_text()
constants='\n'.join(re.search(r'^#define\s+'+n+r'\s+.+',maps,re.M)[0]for n in ['DRAM_START','SMMU_HEAP_ADDR','IPL_FB_ADDRESS','IPL_FB_SZ'])
c=r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef uint8_t u8;typedef uint32_t u32;typedef uint64_t u64;
#define SZ_4M 0x400000u
#define RAM_WINDOW_BYTES SZ_4M
#define SMMU_PDN_SHIFT 22u
#define SMMU_ADDR_TO_PDN(addr) ((addr)>>SMMU_PDN_SHIFT)
#define SMMU_4MB_REGION 0
static void smmu_flush_all(void){}
'''+constants+'\n'+pde+contract+r'''
static nyx_ram_handoff_t record;static u8 image[NYX_RAM_RETURN_MAX];
#undef NYX_RAM_HANDOFF_ADDR
#define NYX_RAM_HANDOFF_ADDR ((uintptr_t)&record)
#undef NYX_RAM_RETURN_ADDR
#define NYX_RAM_RETURN_ADDR ((uintptr_t)image)
static bool g_abort_user,g_run_failed;
static u32 g_errors,g_passes_completed,g_pass_total,g_regions_testable,g_ram_mb;
static u32 g_mode,g_pattern_mask,g_completed_mask,g_verified_regions;
static u32 ram_mtc_errors(void){return 0;}
#define BPMP_MMU_MAINT_CLEAN_WAY 0
static void bpmp_mmu_maintenance(u32 x,bool y){(void)x;(void)y;}
'''+ '\n'.join(extract(main,n)for n in ['nyx_return_valid','nyx_requested_8gb','nyx_record_result','overlaps','protected_region'])+'\n'+extract(smmu,'smmu_map_huge')+'\n'+re.search(r'typedef enum \{\s*PAT_ADDR.*?pattern_t;',main,re.S)[0]+r'''
static u64 last_error_addr;
static void record_error(u64 addr,u32 expected,u32 got,pattern_t p){(void)expected;(void)got;(void)p;++g_errors;last_error_addr=addr;}
'''+ '\n'.join(extract(main,n)for n in ['pattern_seed','prbs_step','fill_region','verify_region'])+r'''
static int checks;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"memory line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void request(bool eight){
 memset(&record,0,sizeof record);memset(image,0,sizeof image);memcpy(image+0x118,"ICTC655",7);
 image[0x122]=eight?1:0;record.magic=NYX_RAM_HANDOFF_MAGIC;record.version=NYX_RAM_VERSION;
 record.auto_run=1;record.payload_size=4096;record.mode=NYX_RAM_MODE_ADVANCED;
 record.selected_mask=NYX_RAM_PATTERN_ALL;record.payload_hash=nyx_ram_hash(image,record.payload_size);
 record.request_hash=nyx_ram_request_hash(&record);
}
int main(void){
 request(true);C(nyx_requested_8gb());request(false);C(nyx_return_valid()&&!nyx_requested_8gb());
 request(true);image[0x122]^=1;C(!nyx_return_valid()&&!nyx_requested_8gb());
 request(true);record.magic^=1;C(!nyx_requested_8gb());request(true);record.request_hash^=1;C(!nyx_requested_8gb());
 request(true);record.payload_size=0x122;record.payload_hash=nyx_ram_hash(image,record.payload_size);record.request_hash=nyx_ram_request_hash(&record);C(!nyx_requested_8gb());
 request(true);image[0x118]='X';record.payload_hash=nyx_ram_hash(image,record.payload_size);record.request_hash=nyx_ram_request_hash(&record);C(!nyx_requested_8gb());
 request(true);record.payload_size=NYX_RAM_RETURN_MAX+1;record.request_hash=nyx_ram_request_hash(&record);C(!nyx_requested_8gb());
 pde_t pd[1024]={0};u32 protected_count=0,above4=0;
 for(u32 mb=4096;mb<=8192;mb+=4096){
  g_regions_testable=0;protected_count=above4=0;g_ram_mb=mb;
  const u64 end=(u64)DRAM_START+((u64)mb<<20);u64 last=0;
  for(u64 phys=DRAM_START;phys<end;phys+=RAM_WINDOW_BYTES){
   if(protected_region(phys)){++protected_count;continue;}
   ++g_regions_testable;if(phys>=0x100000000ULL)++above4;
   smmu_map_huge(pd,0xC0000000u,phys,1,7);
   C(((u64)pd[0x300].huge.page<<22)==phys);C(pd[0x300].huge.attr==7);last=phys;
  }
  C(protected_count==4);C(last+SZ_4M==end);C(g_regions_testable*4==mb-16);
  C(above4==(mb==8192?1536:512));
  request(mb==8192);g_mode=record.mode;g_pattern_mask=g_completed_mask=record.selected_mask;
  g_pass_total=g_passes_completed=7;g_verified_regions=g_regions_testable;nyx_record_result(123,false);
  C(nyx_ram_result_valid(&record));C(record.dram_mib==mb);C(record.tested_mib==mb-16);
  C(record.verified_mib==record.tested_mib);C(record.result==NYX_RAM_RESULT_PASS);
  g_verified_regions--;nyx_record_result(124,false);C(!nyx_ram_result_valid(&record));
  nyx_record_result(124,true);C(record.result==NYX_RAM_RESULT_ERROR&&nyx_ram_result_valid(&record));
 }
 u32 *window=malloc(SZ_4M);C(window);const u64 addresses[]={0x80400000ULL,0x100400000ULL,0x200400000ULL,0x27FC00000ULL};
 for(unsigned a=0;a<4;++a)for(pattern_t p=PAT_ADDR;p<PAT_COUNT;++p){
  g_errors=0;fill_region(window,addresses[a],p,0x1234);verify_region(window,addresses[a],p,0x1234);C(!g_errors);
  window[123]^=1;verify_region(window,addresses[a],p,0x1234);C(g_errors==1&&last_error_addr==addresses[a]+123*4);
 }
 fill_region(window,0x80400000ULL,PAT_ADDR,0);u32 lower=window[0];fill_region(window,0x180400000ULL,PAT_ADDR,0);C(lower!=window[0]);free(window);
 printf("PASS handoff/mapping/report: %d checks; 8GB end=0x280000000, testable=8176 MiB\n",checks);
}
'''
(D/'memory.c').write_text(c)
subprocess.run([str(gcc),'-std=c11','-Wall','-Wextra','-Werror',str(D/'memory.c'),'-o',str(D/'memory.exe')],check=True)
memory=subprocess.check_output([str(D/'memory.exe')],text=True);print(memory.strip())
assert main.index('if(nyx_requested_8gb())fuse_force_8gb_dramid();')<main.index('    hw_init();')
assert main.index('ram_mtc_check_capacity(MC(MC_EMEM_CFG)')<main.index('        ram_locale_init(g_from_nyx)')
report={'passed':True,'host_only':True,'hardware_validated':False,'bridge':bridge.strip(),'mapping':memory.strip(),'8gb_testable_mib':8176,'protected_mib':16,'sources':{str(p.relative_to(W)):hashlib.sha256(p.read_bytes()).hexdigest()for p in [S/'ramtest_main.c',S/'ramtest_mtc_universal.cpp',S/'ramtest_mtc.h']}}
(W/'nova33_1-8gb-check.json').write_text(json.dumps(report,indent=2))
