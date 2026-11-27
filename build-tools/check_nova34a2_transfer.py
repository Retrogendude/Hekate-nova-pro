"""Actual full-size raw transfer loop on fake SD/eMMC; resize-only tail excluded."""
from pathlib import Path
import re,subprocess,json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha2';D=W/'nova34a2-transfer-check';D.mkdir(exist_ok=True)
s=(P/'nyx/nyx_gui/frontend/fe_emummc_tools.c').read_text()
def fn(n):
 m=re.search(r'^static (?:int|bool) '+n+r'\(',s,re.M);a=s.index('{',m.start());i=a+1;d=1
 while d:d+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[m.start():i]
raw=fn('_dump_emummc_raw_part');pos=[m.start()for m in re.finditer(r'\n\tif \(resized_count\)\n',raw)];assert len(pos)==2
raw=raw[:pos[1]]+'\n return 0;\n}'
mbr=(P/'bdk/storage/mbr_gpt.h').read_text();mbr=mbr[mbr.index('typedef struct _mbr_chs_t'):mbr.index('typedef struct _gpt_header_t')]
c=r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef uint64_t u64;
'''+mbr+r'''
typedef struct{int dummy;}sdmmc_storage_t;static sdmmc_storage_t sd_storage,emmc_storage;
typedef struct{u32 lba_start,lba_end;}emmc_part_t;
typedef struct{void*label_log,*label_info,*label_pct,*bar;char*txt_buf,*base_path;}emmc_tool_gui_t;
#define NUM_SECTORS_PER_ITER 8192
#define MIXD_BUF_ALIGNED ((uintptr_t)mix)
#define SDMMC_ALT_DMA_BUFFER ((uintptr_t)alt)
#define LV_LABEL_POS_LAST 65535
#define LV_OPA_COVER 255
#define BTN_VOL_UP 1
#define BTN_VOL_DOWN 2
#define SYMBOL_DOT "."
#define MIN(a,b) ((a)<(b)?(a):(b))
#define s_printf sprintf
#define LIST_INIT(n) int n=0
static u8 mix[8192*512],alt[8192*512],sd_bytes[32*1024*1024];static mbr_t table;
static unsigned checks,writes,reads,e_reads,mb_writes,retry_e,retry_w,fail_verify,corrupt,cancel;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"line%d %s\n",__LINE__,#x);exit(1);}}while(0)
static void emmc_gpt_parse(void*p){(void)p;}
static emmc_part_t*emmc_part_find(void*p,const char*n){(void)p;(void)n;return NULL;}
static void emmc_gpt_free(void*p){(void)p;}
static void manual_system_maintenance(bool b){(void)b;}
static void msleep(u32 ms){(void)ms;}
static u32 btn_read_vol(void){return cancel?3:0;}
static void lv_label_ins_text(void*p,int a,const char*s){(void)p;(void)a;(void)s;}
static void lv_label_set_text(void*p,const char*s){(void)p;(void)s;}
static void lv_bar_set_value(void*p,u32 x){(void)p;(void)x;}
static void lv_obj_set_opa_scale(void*p,u32 x){(void)p;(void)x;}
static int sdmmc_storage_read(sdmmc_storage_t*d,u32 l,u32 n,void*p){
 if(d==&emmc_storage){++e_reads;if(retry_e){--retry_e;return 1;}for(u32 i=0;i<n*512;++i)((u8*)p)[i]=(u8)((l*512u+i)*31u+17u+(l>>5));return 0;}
 if(!l&&n==1){memcpy(p,&table,512);return 0;}
 ++reads;if(fail_verify)return 1;C(l>=0x10000&&(l-0x10000+n)*512<=sizeof sd_bytes);memcpy(p,sd_bytes+(l-0x10000)*512,n*512);if(corrupt)((u8*)p)[7]^=1;return 0;
}
static int sdmmc_storage_write(sdmmc_storage_t*d,u32 l,u32 n,void*p){
 C(d==&sd_storage);if(!l&&n==1){++mb_writes;memcpy(&table,p,512);return 0;}
 ++writes;if(retry_w){--retry_w;return 1;}C(l>=0x10000&&(l-0x10000+n)*512<=sizeof sd_bytes);memcpy(sd_bytes+(l-0x10000)*512,p,n*512);return 0;
}
'''+fn('emummc_verify_raw_chunk')+'\n'+raw+r'''
static void reset(void){writes=reads=e_reads=mb_writes=retry_e=retry_w=fail_verify=corrupt=cancel=0;memset(sd_bytes,0xA5,sizeof sd_bytes);memset(&table,0,sizeof table);}
static int copy(int active,u32 sectors,bool verify){char text[16384];emmc_tool_gui_t g={0};g.txt_buf=text;g.base_path="emuMMC/RAW1/";emmc_part_t p={0,sectors-1};return _dump_emummc_raw_part(&g,active,1,0x10000,&p,0,verify);}
int main(void){
 for(int side=0;side<3;++side)for(int verify=0;verify<2;++verify){reset();C(copy(side,side==2?20000:8192,verify)==0);C(writes==(side==2?3u:1u));C(reads==(verify?writes:0));C(mb_writes==(side==2));if(side==2)C(table.partitions[1].type==0xE0);}
 reset();retry_e=2;C(copy(0,8192,true)==0&&e_reads==3&&writes==1&&reads==1);
 reset();retry_w=2;C(copy(0,8192,true)==0&&writes==3&&reads==1);
 reset();retry_e=3;C(copy(0,8192,true)!=0&&!writes&&!reads);
 reset();retry_w=3;C(copy(0,8192,true)!=0&&writes==3&&!reads);
 reset();fail_verify=1;C(copy(0,8192,true)!=0&&writes==1&&reads==1&&!mb_writes);
 reset();corrupt=1;C(copy(0,8192,true)!=0&&writes==1&&reads==1&&!mb_writes);
 reset();cancel=1;C(copy(0,8192,true)!=0&&!writes&&!e_reads);
 printf("PASS: %u actual-C full-size transfer assertions: write-before-readback, all 3 areas, multi-chunk, retries, cancel, corrupt/unreadable SD\n",checks);
}
'''
(D/'test.c').write_text(c)
subprocess.run([str(W/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-Wall',str(D/'test.c'),'-o',str(D/'test.exe')],check=True)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
(W/'nova34a2-transfer-check.json').write_text(json.dumps({'passed':True,'actual_full_size_transfer_loop':True,'resize_only_tail_excluded':True,'storage_mocked':True,'hardware_test':False,'result':r.stdout.strip(),'source_sha256':hashlib.sha256((P/'nyx/nyx_gui/frontend/fe_emummc_tools.c').read_bytes()).hexdigest()},indent=2))

