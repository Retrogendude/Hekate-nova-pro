"""Compile actual checked copier/activation code against fault-injected fake I/O."""
from pathlib import Path
import re,subprocess,json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha6';D=W/'nova34a6-copy-check';D.mkdir(exist_ok=True)
source=(P/'nyx/nyx_gui/frontend/fe_emummc_tools.c').read_text()
def fn(name):
 m=re.search(r'^(?:static )?(?:int|bool|void) '+name+r'\(',source,re.M);assert m,name
 a=source.index('{',m.start());i=a+1;depth=1
 while depth:depth+=(source[i]=='{')-(source[i]=='}');i+=1
 return source[m.start():i]
mbr=(P/'bdk/storage/mbr_gpt.h').read_text();mbr=mbr[mbr.index('typedef struct _mbr_chs_t'):mbr.index('typedef struct _gpt_header_t')]
code=r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef uint64_t u64;typedef unsigned UINT;typedef int FRESULT;
'''+mbr+r'''
#define SZ_16M (16u*1024*1024)
#define SZ_16K 16384u
#define OUT_FILENAME_SZ 128
#define EMMC_GPP 0
#define FR_OK 0
#define FR_EXIST 8
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 8
#define LV_LABEL_POS_LAST 65535
#define s_printf sprintf
typedef struct{u32 sec_cnt;u8 raw_cid[16];}sdmmc_storage_t;
typedef struct{u32 lba_start,lba_end;char name[36];}emmc_part_t;
typedef struct{void*label_log,*label_info,*label_finish,*bar,*label_pct;char*txt_buf,*base_path;}emmc_tool_gui_t;
static sdmmc_storage_t sd_storage,emmc_storage;static mbr_t table;
static u8 mix[SZ_16M],alt[SZ_16M],chunk[1024];
#define MIXD_BUF_ALIGNED ((uintptr_t)mix)
#define SDMMC_ALT_DMA_BUFFER ((uintptr_t)alt)
static int cases,fail_at,operations,fail_stage,stages,verified,raw_writes,read_error,read_flip,clear_error,mount_error,emmc_error,short_write;
#define C(x) do{++cases;if(!(x)){fprintf(stderr,"FAIL line%d: %s (fault=%d op=%d)\n",__LINE__,#x,fail_at,operations);exit(1);}}while(0)
static bool fault(void){return ++operations==fail_at;}
typedef struct{char name[160],bytes[1024];unsigned size;bool exists;}file_t;
typedef struct{int index;unsigned pos;bool open;}FIL;
static file_t files[16];
static int find(const char*n){for(int i=0;i<16;i++)if(files[i].exists&&!strcmp(files[i].name,n))return i;return -1;}
static int slot(const char*n){int i=find(n);if(i>=0)return i;for(i=0;i<16;i++)if(!files[i].exists){memset(&files[i],0,sizeof files[i]);strcpy(files[i].name,n);files[i].exists=true;return i;}return -1;}
static int sd_mount(void){return mount_error;}
static void sd_unmount(void){}
static int emmc_initialize(bool b){(void)b;return emmc_error;}
static void emmc_set_partition(int p){(void)p;}
static void emmc_end(void){}
static u32 get_tmr_s(void){return 10;}
static void manual_system_maintenance(bool b){(void)b;}
static void lv_label_set_text(void*p,const char*s){(void)p;(void)s;}
static void lv_label_ins_text(void*p,int pos,const char*s){(void)p;(void)pos;(void)s;}
static int f_mkdir(const char*n){(void)n;return fault()?1:0;}
static int f_open(FIL*f,const char*n,int flags){if(fault())return 1;int i=find(n);if(flags&FA_CREATE_ALWAYS){i=slot(n);if(i>=0)files[i].size=0;}if(i<0)return 1;f->index=i;f->pos=0;f->open=true;return 0;}
static int f_close(FIL*f){C(f->open);f->open=false;return fault()?1:0;}
static int f_sync(FIL*f){C(f->open);return fault()?1:0;}
static int f_write(FIL*f,const void*p,UINT n,UINT*out){if(fault()){if(out)*out=0;return 1;}C(f->open&&f->pos+n<1024);if(short_write&&n)--n;memcpy(files[f->index].bytes+f->pos,p,n);f->pos+=n;files[f->index].size=f->pos;files[f->index].bytes[f->pos]=0;if(out)*out=n;return 0;}
static int f_read(FIL*f,void*p,UINT n,UINT*out){if(fault()){*out=0;return 1;}C(f->open);if(n>files[f->index].size-f->pos)n=files[f->index].size-f->pos;memcpy(p,files[f->index].bytes+f->pos,n);f->pos+=n;*out=n;return 0;}
static int f_stat(const char*n,void*p){(void)p;if(fault())return 1;return find(n)<0?4:0;}
static int f_unlink(const char*n){if(fault())return 1;int i=find(n);if(i<0)return 4;files[i].exists=false;return 0;}
static int f_rename(const char*a,const char*b){if(fault())return 1;int i=find(a);if(i<0||find(b)>=0)return 1;strcpy(files[i].name,b);return 0;}
static int sdmmc_storage_read(sdmmc_storage_t*d,u32 l,u32 count,void*p){(void)d;if(read_error)return 1;if(!l&&count==1)memcpy(p,&table,512);else {C(count*512<=sizeof chunk);memcpy(p,chunk,count*512);if(read_flip)((u8*)p)[1]^=1;}return 0;}
static int sdmmc_storage_write(sdmmc_storage_t*d,u32 l,u32 n,const void*p){(void)d;(void)l;(void)n;(void)p;++raw_writes;return clear_error;}
static int emummc_raw_derive_bis_keys(void){return 1;}
static int _dump_emummc_raw_part(emmc_tool_gui_t*g,int active,int index,u32 start,emmc_part_t*p,u32 resized,bool verify){(void)g;(void)index;(void)start;(void)p;C(!resized&&active==stages);++stages;if(verify)++verified;return stages==fail_stage?1:0;}
'''+fn('save_emummc_cfg_checked')+'\n'+fn('emummc_verify_raw_chunk')+'\n'+fn('dump_emummc_raw_impl')+r'''
static void reset(void){
 memset(files,0,sizeof files);memset(&table,0,sizeof table);operations=fail_at=fail_stage=stages=verified=raw_writes=read_error=read_flip=clear_error=mount_error=emmc_error=short_write=0;
 sd_storage.sec_cnt=0x100000;memset(sd_storage.raw_cid,42,16);emmc_storage.sec_cnt=1000;
 table.boot_signature=0xAA55;table.partitions[1].type=0xE0;table.partitions[1].start_sct=0x8000;table.partitions[1].size_sct=0x20000;
 int i=slot("emuMMC/emummc.ini");strcpy(files[i].bytes,"[emummc]\nenabled=0\n");files[i].size=strlen(files[i].bytes);
}
static int copy(void){emmc_tool_gui_t gui={0};return dump_emummc_raw_impl(&gui,1,0x10000,0,true,sd_storage.raw_cid,1000);}
static bool enabled(void){int i=find("emuMMC/emummc.ini");return i>=0&&strstr(files[i].bytes,"enabled=1\n");}
int main(void){
 reset();C(copy()==0);C(stages==3&&verified==3&&enabled());int total=operations;
 C(find("emuMMC/RAW1/raw_based")>=0);C(strstr(files[find("emuMMC/emummc.ini")].bytes,"sector=0x10000"));
 for(int fail=1;fail<=total;++fail){reset();fail_at=fail;int rc=copy();if(rc)C(!enabled());else C(enabled()&&stages==3);}
 for(int stage=1;stage<=3;++stage){reset();fail_stage=stage;C(copy()!=0);C(stages==stage&&!enabled()&&find("emuMMC/RAW1/raw_based")<0);}
 reset();short_write=1;C(save_emummc_cfg_checked(1,0x10000,"emuMMC/RAW1")!=0&&!enabled());
 reset();clear_error=1;C(copy()!=0&&!enabled()&&!stages);
 reset();mount_error=1;C(copy()!=0&&!raw_writes&&!enabled());
 reset();emmc_error=1;C(copy()!=0&&!raw_writes&&!enabled());
 reset();table.partitions[1].size_sct=1;C(copy()!=0&&!raw_writes&&!enabled());
 reset();table.boot_signature=0;C(copy()!=0&&!raw_writes&&!enabled());
 reset();table.partitions[1].type=0x83;C(copy()!=0&&!raw_writes&&!enabled());
 reset();read_error=1;C(copy()!=0&&!raw_writes&&!enabled());
 reset();u8 wrong[16]={0};emmc_tool_gui_t g={0};C(dump_emummc_raw_impl(&g,1,0x10000,0,true,wrong,1000)!=0&&!raw_writes);
 reset();memset(chunk,0x5A,sizeof chunk);C(emummc_verify_raw_chunk(100,2,chunk));read_error=1;C(!emummc_verify_raw_chunk(100,2,chunk));read_error=0;read_flip=1;C(!emummc_verify_raw_chunk(100,2,chunk));
 printf("PASS: %d actual-C copier/activation assertions; %d metadata failure positions, all copy-stage failures, CID/size/MBR rejection and readback corruption\n",cases,total);
}
'''
(D/'test.c').write_text(code)
subprocess.run([str(W/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-Wall','-Wextra',str(D/'test.c'),'-o',str(D/'test.exe')],check=True)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
(W/'nova34a6-copy-check.json').write_text(json.dumps({'passed':True,'result':r.stdout.strip(),'actual_c':True,'raw_data_transfer_mocked':True,'hardware_test':False,'source_sha256':hashlib.sha256((P/'nyx/nyx_gui/frontend/fe_emummc_tools.c').read_bytes()).hexdigest()},indent=2))
