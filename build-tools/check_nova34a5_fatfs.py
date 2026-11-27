"""Run both broken and fixed config writers against actual Nyx FatFs on virtual media."""
from pathlib import Path
import re,subprocess,json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha5';D=W/'nova34a5-fatfs-check';D.mkdir(exist_ok=True)
def fn(path):
 s=path.read_text();a=s.index('int save_emummc_cfg_checked(');b=s.index('\nvoid save_emummc_cfg(',a);return s[a:b]
fixed=fn(P/'nyx/nyx_gui/frontend/fe_emummc_tools.c');broken=fn(W/'hekate-nova-pro-v34-alpha3/nyx/nyx_gui/frontend/fe_emummc_tools.c').replace('save_emummc_cfg_checked','save_emummc_cfg_broken')
(D/'host.h').write_text('void gfx_printf(const char *,...);\n')
c=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libs/fatfs/ff.h>
#include <libs/fatfs/diskio.h>
#define s_printf sprintf
#define SECTORS 262144u
static BYTE *media;static FATFS volume;static unsigned checks;static int protected;
#define C(x) do{++checks;if(!(x)){fprintf(stderr,"line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
DSTATUS disk_initialize(BYTE p){return p?STA_NODISK:protected?STA_PROTECT:0;}
DSTATUS disk_status(BYTE p){return disk_initialize(p);}
DRESULT disk_read(BYTE p,BYTE*b,DWORD l,UINT n){if(p||(u64)l+n>SECTORS)return RES_PARERR;memcpy(b,media+l*512u,n*512u);return RES_OK;}
DRESULT disk_write(BYTE p,const BYTE*b,DWORD l,UINT n){if(protected)return RES_WRPRT;if(p||(u64)l+n>SECTORS)return RES_PARERR;memcpy(media+l*512u,b,n*512u);return RES_OK;}
DRESULT disk_ioctl(BYTE p,BYTE cmd,void*b){if(p)return RES_PARERR;switch(cmd){case CTRL_SYNC:case CTRL_TRIM:return RES_OK;case GET_SECTOR_COUNT:*(DWORD*)b=SECTORS;return RES_OK;case GET_SECTOR_SIZE:*(WORD*)b=512;return RES_OK;case GET_BLOCK_SIZE:*(DWORD*)b=2048;return RES_OK;default:return RES_PARERR;}}
void*ff_memalloc(UINT n){return malloc(n);}
void ff_memfree(void*p){free(p);}
DWORD get_fattime(void){return (46u<<25)|(11u<<21)|(25u<<16);}
void gfx_printf(const char*f,...){(void)f;}
static int sd_mount(void){return f_mount(&volume,"sd:",1);}
'''+broken+'\n'+fixed+r'''
static void content(const char*path,char*out,UINT*size){FIL f;C(f_open(&f,path,FA_READ)==FR_OK);C(f_read(&f,out,511,size)==FR_OK);out[*size]=0;C(f_close(&f)==FR_OK);}
static void expect(const char*fragment){char b[512];UINT n;content("emuMMC/emummc.ini",b,&n);C(strstr(b,fragment)!=NULL);C(!strchr(b,'\r'));C(n==strlen(b));}
int main(void){
 C(sizeof(DWORD)==4&&sizeof(u32)==4);C(FF_USE_STRFUNC==2&&FF_USE_LFN==3&&FF_FS_EXFAT==1);
 media=calloc(SECTORS,512);C(media);u8 work[4096];
 for(int format=0;format<2;++format){
  f_unmount("sd:");memset(media,0,SECTORS*512u);FRESULT formatted=f_mkfs("sd:",format?FM_EXFAT:FM_FAT32,format?65536:512,work,sizeof work);fprintf(stderr,"format=%d status=%d\n",format,formatted);C(formatted==FR_OK);C(sd_mount()==0);C(volume.fs_type==(format?FS_EXFAT:FS_FAT32));
  C(save_emummc_cfg_broken(0,0,NULL)!=0);char old[512];UINT n;content("emuMMC/emummc.ini.nova.tmp",old,&n);C(strstr(old,"\r\n")!=NULL);C(f_stat("emuMMC/emummc.ini",NULL)==FR_NO_FILE);
  C(save_emummc_cfg_checked(0,0,NULL)==0);expect("enabled=0\n");
  C(save_emummc_cfg_checked(2,0x123456,"emuMMC/RAW2")==0);expect("enabled=2\nsector=0x123456\npath=emuMMC/RAW2\n");expect("nintendo_path=emuMMC/RAW2/Nintendo\n");
  C(save_emummc_cfg_checked(0,0,NULL)==0);expect("enabled=0\n");C(save_emummc_cfg_checked(0,0,NULL)==0);expect("enabled=0\n");
  C(save_emummc_cfg_checked(0,0,"emuMMC/BK00")==0);expect("enabled=1\n");
  /* An old Nintendo/Hekate-style CRLF config is replaced and backed up intact. */
  FIL file;C(f_open(&file,"emuMMC/emummc.ini",FA_WRITE|FA_CREATE_ALWAYS)==FR_OK);C(f_puts("[emummc]\nenabled=1\n",&file)>19);C(f_close(&file)==FR_OK);
  C(save_emummc_cfg_checked(0,0,NULL)==0);expect("enabled=0\n");content("emuMMC/emummc.ini.nova.bak",old,&n);C(strstr(old,"enabled=1\r\n")!=NULL);
  C(save_emummc_cfg_checked(1,0x10000,"emuMMC/RAW1")==0);protected=1;C(save_emummc_cfg_checked(0,0,NULL)!=0);protected=0;expect("enabled=1\n");
 }
 printf("PASS: %u actual Nyx FatFs assertions; v34-alpha3 CRLF false failure reproduced on FAT32/exFAT; fixed writer, first/repeated disable, raw/file activation, old CRLF backup and real write-protect refusal\n",checks);free(media);
}
'''
(D/'test.c').write_text(c)
cc=str(W/'toolchains/w64devkit/bin/gcc.exe')
args=[cc,'-std=gnu11','-O2','-include',str(D/'host.h'),'-I'+str(P/'bdk'),'-DFFCFG_INC="'+str((P/'nyx/nyx_gui/libs/fatfs/ffconf.h').resolve()).replace('\\','/')+'"',str(D/'test.c'),str(P/'bdk/libs/fatfs/ff.c'),str(P/'bdk/libs/fatfs/ffunicode.c'),'-o',str(D/'test.exe')]
r=subprocess.run(args,capture_output=True,text=True)
if r.returncode:print(r.stderr);raise SystemExit(r.returncode)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
report={'passed':True,'real_nyx_fatfs_c':True,'real_nyx_ffconf':True,'virtual_disk_only':True,'physical_sd_written':False,'result':r.stdout.strip(),'source_sha256':{n:hashlib.sha256((P/n).read_bytes()).hexdigest()for n in ['nyx/nyx_gui/frontend/fe_emummc_tools.c','nyx/nyx_gui/libs/fatfs/ffconf.h','bdk/libs/fatfs/ff.c','bdk/libs/fatfs/ffunicode.c']}}
(W/'nova34a5-fatfs-check.json').write_text(json.dumps(report,indent=2))

