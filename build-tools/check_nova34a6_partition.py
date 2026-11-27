"""Actual partition transaction/table writers with fake I/O and injected failures."""
from pathlib import Path
import re,subprocess,json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha6';D=W/'nova34a6-partition-check';D.mkdir(exist_ok=True)
s=(P/'nyx/nyx_gui/frontend/gui_tools_partition_manager.c').read_text()
def fn(n):
 m=re.search(r'^static (?:int|bool|void|lv_res_t) '+n+r'\(',s,re.M);assert m,n
 a=s.index('{',m.start());i=a+1;d=1
 while d:d+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[m.start():i]
job=s[s.index('/* A one-shot, card-bound'):s.index('static bool nova_install_job_valid')]
c=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gui_partition_plan.h"
#include <storage/mbr_gpt.h>
typedef struct{u32 sec_cnt;u8 raw_cid[16];struct{u8 manfid;}cid;}sdmmc_storage_t;
typedef int lv_res_t;typedef int lv_obj_t;typedef int FATFS;
#define AU_ALIGN_SECTORS 0x8000u
#define AU_ALIGN_BYTES (AU_ALIGN_SECTORS*512u)
#define RAM_DISK_SZ 1
#define LV_HOR_RES 1280
#define LV_VER_RES 720
#define LV_DPI 100
#define LV_ALIGN_CENTER 0
#define LV_LABEL_LONG_DOT 0
#define LV_LABEL_ALIGN_CENTER 0
#define LV_BTN_STATE_REL 0
#define LV_BTN_STATE_INA 1
#define LV_RES_OK 0
#define BTN_POWER 1
#define EMMC_GPP 0
#define FR_OK 0
#define FM_FAT32 1
#define DRIVE_SD 0
#define SET_SECTOR_COUNT 0
#define SE_RNG_BLOCK_SIZE 16
#define s_printf sprintf
#define nyx_tr(x) (x)
static sdmmc_storage_t sd_storage,emmc_storage;
static struct{bool emmc,emu_double,and_dynamic,backup_possible;sdmmc_storage_t*storage;u32 total_sct,alignment,emu_size,l4t_size,and_size;int hos_size;lv_obj_t*partition_button;mbr_t mbr_old;}part_info;
static u8 alt[64u*1024*1024],cache[32u*1024*1024];static mbr_t disk_mbr;
#define SDMMC_ALT_DMA_BUFFER ((uintptr_t)alt)
static lv_obj_t mbox_darken,*btn_flash_l4t,*btn_flash_android;static FATFS sd_fs;
static unsigned checks,model,button,write_no,read_no,last_lba,last_count,copy_calls,format_calls,backup_calls,restore_calls,disable_calls;
static unsigned fail_write,flip_read,fail_read,fail_mount,fail_emmc,fail_ram,fail_backup,fail_restore,fail_format,fail_remount,fail_disable,swap_power,swap_final,bad_target,preflight_reject;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"line %d %s\n",__LINE__,#x);exit(1);}}while(0)
static int sd_mount(void){return fail_mount;}
static void sd_unmount(void){}
static void sd_end(void){}
static int emmc_initialize(bool b){(void)b;return fail_emmc;}
static void emmc_set_partition(int p){(void)p;}
static void emmc_end(void){}
static u32 nyx_nova_installation_model(void){return model;}
static int sdmmc_storage_write(sdmmc_storage_t*d,u32 l,u32 n,void*p){
 C(d==&sd_storage&&(u64)l+n<=sd_storage.sec_cnt&&n*512<=sizeof cache);++write_no;
 if(write_no==fail_write)return 1;memcpy(cache,p,n*512);last_lba=l;last_count=n;
 if(!l&&n==1){memcpy(&disk_mbr,p,512);if(swap_final)sd_storage.raw_cid[0]++;}return 0;
}
static int sdmmc_storage_read(sdmmc_storage_t*d,u32 l,u32 n,void*p){
 C(d==&sd_storage);++read_no;if(read_no==fail_read)return 1;
 if(!l&&n==1){memcpy(p,&disk_mbr,512);if(bad_target&&write_no)((mbr_t*)p)->partitions[1].type=0x83;}
 else{C(l==last_lba&&n==last_count);memcpy(p,cache,n*512);}if(read_no==flip_read)((u8*)p)[0]^=1;return 0;
}
static void se_rng_pseudo(void*p,u32 n){memset(p,0x42,n);}
static u32 crc32_calc(u32 n,const u8*p,u32 z){(void)n;(void)p;(void)z;return 0;}
static void*zalloc(size_t n){return calloc(1,n);}
#define lv_obj_create(...) ((lv_obj_t*)1)
#define lv_mbox_create(...) ((lv_obj_t*)2)
#define lv_label_create(...) ((lv_obj_t*)3)
#define lv_obj_set_style(...) ((void)0)
#define lv_obj_set_size(...) ((void)0)
#define lv_mbox_set_recolor_text(...) ((void)0)
#define lv_obj_set_width(...) ((void)0)
static char last_message[2048];
static void lv_mbox_set_text(lv_obj_t*p,const char*text){(void)p;snprintf(last_message,sizeof last_message,"%s",text);}
#define lv_obj_align(...) ((void)0)
#define lv_obj_set_top(...) ((void)0)
#define lv_label_set_recolor(...) ((void)0)
#define lv_label_set_text(...) ((void)0)
#define lv_label_set_long_mode(...) ((void)0)
#define lv_cont_set_fit(...) ((void)0)
#define lv_label_set_align(...) ((void)0)
#define lv_obj_set_click(...) ((void)0)
#define lv_btn_set_state(...) ((void)0)
#define lv_mbox_add_btns(...) ((void)0)
#define lv_obj_del(...) ((void)0)
static void manual_system_maintenance(bool b){(void)b;}
static void msleep(u32 n){(void)n;}
static u32 btn_wait(void){if(swap_power)sd_storage.raw_cid[0]++;return button;}
static int save_emummc_cfg_checked(u32 a,u32 b,const char*p){C(!a&&!b&&!p);++disable_calls;return fail_disable;}
static int ram_disk_init(FATFS*f,u32 n){(void)f;(void)n;return fail_ram;}
static int _backup_and_restore_files(bool backup,lv_obj_t**p){(void)p;if(backup){++backup_calls;return fail_backup;}++restore_calls;return fail_restore;}
static int f_unmount(const char*p){(void)p;return 0;}
static void disk_set_info(int a,int b,u32*c){(void)a;(void)b;(void)c;}
static int f_mkfs(const char*a,int b,u32 c,void*d,u32 e){(void)a;(void)b;(void)c;(void)d;(void)e;++format_calls;return fail_format;}
static int f_mount(FATFS*f,const char*p,int n){(void)f;(void)p;(void)n;return fail_remount;}
static void f_chdrive(const char*p){(void)p;}
static void f_setlabel(const char*p){(void)p;}
static bool nyx_nova_installation_copy(u32 i,u32 start,u32 size,u32 sectors,u32 gpp,const u8*cid){
 C(i==1||i==2);C(sectors==sd_storage.sec_cnt&&gpp==emmc_storage.sec_cnt&&!memcmp(cid,sd_storage.raw_cid,16));C(disk_mbr.partitions[i].start_sct==start&&disk_mbr.partitions[i].size_sct==size);++copy_calls;return true;
}
'''+job+fn('nova_install_job_valid')+'\n'+fn('nova_table_write')+r'''
static bool nova_partition_preflight(void){return !preflight_reject&&nova_install_job_valid();}
'''+fn('_create_gpt_partition')+'\n'+fn('_sd_prepare_and_flash_mbr_gpt')+'\n'+fn('_sd_create_mbox_start_partitioning')+r'''
static void reset(u32 hw,u32 systems){
 memset(&install_job,0,sizeof install_job);memset(&part_info,0,sizeof part_info);memset(&disk_mbr,0,sizeof disk_mbr);
 write_no=read_no=last_lba=last_count=copy_calls=format_calls=backup_calls=restore_calls=disable_calls=0;
 fail_write=flip_read=fail_read=fail_mount=fail_emmc=fail_ram=fail_backup=fail_restore=fail_format=fail_remount=fail_disable=swap_power=swap_final=bad_target=preflight_reject=0;
 model=hw;button=BTN_POWER;sd_storage.cid.manfid=3;sd_storage.sec_cnt=128*2097152u;memset(sd_storage.raw_cid,42,16);emmc_storage.sec_cnt=(hw==3?59640u:29820u)*2048u;
 nova_partition_plan p={0};p.total_sectors=sd_storage.sec_cnt;p.fat_min_mib=2048;C(nova_partition_installation(&p,hw,hw<2?0:hw,emmc_storage.sec_cnt/2048,systems)==NOVA_PLAN_OK);
 install_job.armed=true;install_job.plan=p;install_job.systems=systems;install_job.sectors=sd_storage.sec_cnt;install_job.gpp=emmc_storage.sec_cnt;install_job.model=hw;memcpy(install_job.cid,sd_storage.raw_cid,16);
 part_info.storage=&sd_storage;part_info.total_sct=sd_storage.sec_cnt;part_info.hos_size=p.fat_mib;part_info.emu_size=p.emu_mib;part_info.l4t_size=p.linux_mib;part_info.and_size=p.android_mib;
 disk_mbr.boot_signature=0xAA55;disk_mbr.partitions[0].type=0x0B;disk_mbr.partitions[0].start_sct=0x8000;disk_mbr.partitions[0].size_sct=p.fat_mib*2048;
}
int main(void){
 for(u32 hw=0;hw<4;++hw)for(u32 mask=1;mask<8;++mask)for(u32 dynamic=0;dynamic<2;++dynamic){reset(hw,mask);part_info.and_dynamic=dynamic;C(_sd_create_mbox_start_partitioning()==0);C(copy_calls==!!(mask&1));C(disable_calls==1&&format_calls==1&&backup_calls==1&&restore_calls==1&&!install_job.running&&!install_job.armed);}
 reset(3,7);_sd_create_mbox_start_partitioning();u32 writes=write_no,reads=read_no;
 for(u32 failure=1;failure<=writes;++failure){reset(3,7);fail_write=failure;_sd_create_mbox_start_partitioning();C(!copy_calls&&!install_job.running&&!install_job.armed);}
 for(u32 failure=1;failure<=reads;++failure){reset(3,7);fail_read=failure;_sd_create_mbox_start_partitioning();C(!copy_calls);}
 for(u32 failure=3;failure<reads;++failure){reset(3,7);flip_read=failure;_sd_create_mbox_start_partitioning();C(!copy_calls);}
 reset(0,1);button=0;_sd_create_mbox_start_partitioning();C(!copy_calls&&!write_no&&!format_calls&&!disable_calls);
 unsigned*errors[]={&preflight_reject,&fail_mount,&fail_emmc,&fail_ram,&fail_backup,&fail_restore,&fail_format,&fail_remount,&fail_disable,&swap_power,&swap_final,&bad_target};
 for(u32 i=0;i<sizeof errors/sizeof*errors;++i){reset(0,1);*errors[i]=1;_sd_create_mbox_start_partitioning();C(!copy_calls&&!install_job.running&&!install_job.armed);}
 reset(0,1);sd_storage.cid.manfid=0;_sd_create_mbox_start_partitioning();C(!copy_calls&&!format_calls&&strstr(install_storage_error,"counterfeit"));
 reset(0,1);fail_disable=1;_sd_create_mbox_start_partitioning();C(!format_calls&&!copy_calls&&strstr(last_message,"emummc.ini")&&strstr(last_message,"No partitioning"));
 reset(0,1);swap_power=1;_sd_create_mbox_start_partitioning();C(!format_calls&&!copy_calls&&strstr(last_message,"different SD card")&&strstr(last_message,"No partitioning"));
 reset(0,1);emmc_storage.sec_cnt--;C(!nova_install_job_valid()&&strstr(install_storage_error,"Internal memory capacity"));
 reset(0,1);sd_storage.sec_cnt--;C(!nova_install_job_valid()&&strstr(install_storage_error,"SD card capacity"));
 reset(0,1);part_info.hos_size--;C(!nova_install_job_valid()&&strstr(install_storage_error,"Partition sizes"));
 reset(0,1);part_info.emmc=true;_sd_create_mbox_start_partitioning();C(!copy_calls&&!write_no&&!format_calls);
 reset(0,1);install_job.armed=false;_sd_create_mbox_start_partitioning();C(!copy_calls&&!disable_calls);
 reset(0,1);install_job.running=true;unsigned before=write_no;C(nova_table_write(&sd_storage,sd_storage.sec_cnt,1,cache)!=0&&write_no==before);C(nova_table_write(&emmc_storage,0,1,cache)!=0&&write_no==before);C(nova_table_write(&sd_storage,0,0,cache)!=0&&write_no==before);
 printf("PASS: %u actual-C transaction/table assertions: 56 model/OS/layout cases, cancel, all %u write failures, %u read failures, readback corruption, card change and manual exclusion\n",checks,writes,reads);
}
'''
(D/'test.c').write_text(c)
subprocess.run([str(W/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-O2','-I'+str(P/'bdk'),'-I'+str(P/'nyx/nyx_gui/frontend'),str(D/'test.c'),str(P/'nyx/nyx_gui/frontend/gui_partition_plan.c'),'-o',str(D/'test.exe')],check=True)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
(W/'nova34a6-partition-check.json').write_text(json.dumps({'passed':True,'actual_transaction_and_table_writers':True,'storage_and_ui_mocked':True,'hardware_test':False,'result':r.stdout.strip(),'source_sha256':hashlib.sha256((P/'nyx/nyx_gui/frontend/gui_tools_partition_manager.c').read_bytes()).hexdigest()},indent=2))
