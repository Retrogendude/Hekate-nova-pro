"""Execute the real original-clock actions and transactional Nyx INI writer."""
from pathlib import Path
import ast,calendar,datetime,re,subprocess,json
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui'
directory=work/'nova-v12-check';directory.mkdir(exist_ok=True)
def function(source,name):
 m=re.search(r'^(?:static )?(?:u32\s*\*|u32|lv_res_t|void|int|bool)\s+'+name+r'\([^;]*?\)\s*\n\{',source,re.M);assert m,name
 depth=1;at=m.end()
 while depth:
  depth+=(source[at]=='{')-(source[at]=='}');at+=1
 return source[m.start():at]+'\n'
header=(root/'config.h').read_text();types=re.search(r'typedef struct _nyx_config\s*\{.*?} nyx_config;',header,re.S).group()
opts=(root/'frontend/gui_options.c').read_text();gui=(root/'frontend/gui.c').read_text();loader=(root/'nyx.c').read_text();config=(root/'config.c').read_text()
assert 'if (!n_cfg.timeoffset)' in gui
assert len(re.findall(r'lv_task_create\(first_time_clock_edit,',gui))==1
assert '_create_mbox_clock_edit(NULL)' in function(opts,'first_time_clock_edit')
assert 'gui_nova_setup' not in (root.parents[0]/'Makefile').read_text()
rtc=(root.parents[1]/'bdk/rtc/max77620-rtc.c').read_text()
constants='\n'.join(re.findall(r'^#define CLOCK_[^\n]*',opts,re.M))
pre=r'''
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;
typedef struct{u8 weekday,sec,min,hour,day,month;u16 year;}rtc_time_t;
typedef struct{u32 selected,id;}lv_obj_t;typedef int lv_res_t;
#define LV_RES_OK 1
#define LV_RES_INV 0
#define FR_OK 0
#define I18N_LANGUAGE_COUNT 7
#define I18N_EN 0
'''+types+'\n'+constants+r'''
static nyx_config n_cfg,saved;
static bool mounted,nyx_changes_made;
static int mount_error,stat_error,boot_error,write_error,writes,reloads,closed,notices,unmounts,pressed;
static int live_offset;
static rtc_time_t raw={.year=2026,.month=11,.day=6,.hour=18,.min=42};
static lv_obj_t rollers[5];
static struct{lv_obj_t *year,*month,*day,*hour,*min;}clock_ctxt;
static u32 clock_language;
static unsigned checks;
#define CHECK(x)do{++checks;if(!(x)){fprintf(stderr,"FAIL clock line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static u32 lv_roller_get_selected(lv_obj_t*o){return o->selected;}
static u32 lv_obj_get_free_num(lv_obj_t*o){return o->id;}
static int lv_btnm_get_pressed(lv_obj_t*o){(void)o;return pressed;}
static void max77620_rtc_get_time(rtc_time_t*t){*t=raw;}
static void max77620_rtc_set_epoch_offset(int offset){live_offset=offset;}
static void nyx_show_help(const char*s){(void)s;++notices;}
static int nyx_mbox_action(lv_obj_t*b,const char*s){(void)b;(void)s;++closed;return LV_RES_INV;}
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){mounted=false;++unmounts;}
static int f_stat(const char*p,void*x){(void)x;assert(!strcmp(p,"bootloader/sys/nyx.bin"));return stat_error;}
static bool nyx_options_commit_boot(void){return !boot_error;}
static int create_nyx_config_entry(bool force){assert(force);++writes;saved=n_cfg;mounted=false;return write_error;}
static void reload_nyx(lv_obj_t*b,bool force){assert(!b&&!force);++reloads;}
static void select_date(u32 year,u32 month,u32 day,u32 hour,u32 min){rollers[0].selected=year-CLOCK_MIN_YEAR;rollers[1].selected=month-1;rollers[2].selected=day-1;rollers[3].selected=hour;rollers[4].selected=min;}
static void reset(void){memset(&n_cfg,0,sizeof n_cfg);n_cfg.nova_logo_color=0x123456;n_cfg.nova_icon_color=0xABCDEF;n_cfg.nova_selection_color=0x334455;n_cfg.language=1;n_cfg.timeoffset=0;clock_language=1;mounted=nyx_changes_made=false;mount_error=stat_error=boot_error=write_error=writes=reloads=closed=notices=unmounts=live_offset=0;pressed=1;clock_ctxt.year=&rollers[0];clock_ctxt.month=&rollers[1];clock_ctxt.day=&rollers[2];clock_ctxt.hour=&rollers[3];clock_ctxt.min=&rollers[4];select_date(2026,11,6,19,42);}
'''
body=function(rtc,'max77620_rtc_date_to_epoch')+''.join(function(opts,n)for n in ('_clock_apply_selection','_action_clock_edit','_action_clock_edit_save','_clock_language_select'))+function((root/'frontend/gui_nova.c').read_text(),'nyx_nova_temperature_rgb')
cases=[]
raw_epoch=int(datetime.datetime(2026,11,6,18,42,tzinfo=datetime.timezone.utc).timestamp())
for year in range(2025,2036):
 for month in range(1,13):
  days=calendar.monthrange(year,month)[1]
  for day in (1,28,29,30,31):
   expected=int(datetime.datetime(year,month,min(day,days),18,42,tzinfo=datetime.timezone.utc).timestamp())-raw_epoch
   for dst in (0,1):
    offset=1 if not expected else expected-(3600 if dst and (month,min(day,days))>=(3,28) and (month,min(day,days))<(10,28) else 0)
    cases.append('{%d,%d,%d,%d,%du}'%(year,month,day,dst,offset&0xffffffff))
tests='static const u32 dates[][5]={'+','.join(cases)+'};\n'+r'''
int main(void){
 for(u32 i=0;i<sizeof(dates)/sizeof(*dates);++i){reset();select_date(dates[i][0],dates[i][1],dates[i][2],18,42);n_cfg.timedst=dates[i][3];_clock_apply_selection();CHECK(n_cfg.timeoffset==dates[i][4]&&nyx_changes_made);CHECK(!writes&&!reloads&&!closed);}
 for(u32 lang=0;lang<7;++lang){reset();lv_obj_t choice={.id=lang};_clock_language_select(&choice);CHECK(clock_language==lang&&n_cfg.language==1);_action_clock_edit_save(NULL,"");CHECK(writes==1&&reloads==1&&closed==1&&n_cfg.language==lang&&saved.language==lang);CHECK(n_cfg.timeoffset==3600&&!nyx_changes_made);CHECK(n_cfg.nova_logo_color==0x123456&&n_cfg.nova_icon_color==0xABCDEF&&n_cfg.nova_selection_color==0x334455);}
 reset();lv_obj_t invalid={.id=7};_clock_language_select(&invalid);CHECK(clock_language==1);
 for(int initial_mount=0;initial_mount<2;++initial_mount)for(int failure=0;failure<4;++failure){reset();mounted=initial_mount;clock_language=4;nyx_config before=n_cfg;mount_error=failure==0;stat_error=failure==1;boot_error=failure==2;write_error=failure==3;_action_clock_edit_save(NULL,"");CHECK(!memcmp(&before,&n_cfg,sizeof before)&&!live_offset&&!nyx_changes_made);CHECK(!reloads&&!closed&&notices==1);CHECK(writes==(failure==3));CHECK(mounted==(failure==0?!!initial_mount:failure<3?!!initial_mount:false));if(failure==3){write_error=0;_action_clock_edit_save(NULL,"");CHECK(writes==2&&reloads==1&&closed==1&&n_cfg.language==4);}}
 reset();pressed=2;clock_language=4;nyx_config before=n_cfg;_action_clock_edit_save(NULL,"");CHECK(!writes&&!reloads&&closed==1&&!memcmp(&before,&n_cfg,sizeof before));
 reset();clock_language=4;_action_clock_edit(NULL,"");CHECK(!writes&&!reloads&&closed==1&&n_cfg.language==1&&n_cfg.timeoffset==3600);
 for(u32 temp=0;temp<110;++temp){u32 expected=temp<45?0x8AE6B6:temp<55?0xF3D36A:temp<65?0xF4A261:0xFF6B6B;CHECK(nyx_nova_temperature_rgb(temp)==expected);}
 printf("PASS: %u actual-C clock/language/temperature checks; original startup and rollers, Python calendar/epoch oracle, leap days, DST compatibility, pending/cancel/retry, SD failures and seven saved languages.\n",checks);return 0;
}
'''
cc=work/'toolchains/w64devkit/bin/gcc.exe'
def run(name,program):
 p=directory/(name+'.c');p.write_text(program);exe=p.with_suffix('.exe')
 subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2',str(p),'-o',str(exe)],check=True)
 result=subprocess.check_output([str(exe)],text=True);print(result.strip());return result.strip()
summaries=[run('clock',pre+body+tests)]
# Fault-injected in-memory FAT filesystem for the actual shared Nyx writer.
tree=ast.parse((work/'check_nyx_config_writer.py').read_text());base=next(ast.literal_eval(n.value)for n in tree.body if isinstance(n,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='preamble'for t in n.targets))
base=base.replace('static int f_stat(const char *path,void *info){(void)path;(void)info;return FR_NO_FILE;}\nstatic int f_unlink(const char *path){(void)path;return FR_OK;}\nstatic int f_rename(const char *from,const char *to){(void)from;(void)to;return FR_OK;}',r'''
static bool fs_final,fs_backup,fs_temp;
static char final_bytes[8192],backup_bytes[8192];
static int fail_stat,fail_remove_backup,rename_count,fail_rename,fail_restore;
static int f_stat(const char *path,void *info){(void)info;if(fail_stat)return 1;if(!strcmp(path,"bootloader/nyx.ini"))return fs_final?FR_OK:FR_NO_FILE;return fs_backup?FR_OK:FR_NO_FILE;}
static int f_unlink(const char *path){if(strstr(path,".tmp")){fs_temp=false;return FR_OK;}if(fail_remove_backup)return 1;fs_backup=false;return FR_OK;}
static int f_rename(const char *from,const char *to){++rename_count;if(rename_count==fail_rename||(rename_count==3&&fail_restore))return 1;if(strstr(from,".tmp")){assert(!strcmp(to,"bootloader/nyx.ini"));fs_temp=false;fs_final=true;strcpy(final_bytes,written);}else if(strstr(from,".bak")){assert(!strcmp(to,"bootloader/nyx.ini"));fs_backup=false;fs_final=true;strcpy(final_bytes,backup_bytes);}else{assert(strstr(to,".bak"));fs_final=false;fs_backup=true;strcpy(backup_bytes,final_bytes);}return FR_OK;}
''').replace('#include <stdio.h>','#include <stdio.h>\n#include <assert.h>')
constants='\n'.join(re.findall(r'^#define (?:HOME_LAYOUT|I18N)_\w+[^\n]*',header+'\n'+(root/'localization.h').read_text(),re.M))
code=''.join(function(config,n)for n in ('set_nyx_default_configuration','valid_font_basename','valid_wallpaper_basename','nyx_sanitize_appearance','create_nyx_config_entry'))
a=loader.index('const char *nyx_ini=');b=loader.index('    if (ini_parse(&ini_nyx_sections, nyx_ini',a)
recovery='static const char *load_path(void)\n{\n'+loader[a:b]+'return nyx_ini;\n}\n'
faults=r'''
static void reset_fs(bool previous){mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;mounts=opens=closes=writes=unmounts=used=0;written[0]=0;fs_final=previous;fs_backup=false;fs_temp=true;strcpy(final_bytes,"previous-complete-config");backup_bytes[0]=0;fail_stat=fail_remove_backup=rename_count=fail_rename=fail_restore=0;set_nyx_default_configuration();n_cfg.nova_logo_color=0x123456;}
int main(void){
 for(int previous=0;previous<2;++previous){
  for(int at=1;at<=57;++at){reset_fs(previous);fail_write_at=at;require(create_nyx_config_entry(false)==1,"write failure returned");require(fs_final==!!previous&&!strcmp(final_bytes,"previous-complete-config")&&!fs_backup&&!fs_temp&&!rename_count,"failed write leaves final unchanged");++cases;}
  reset_fs(previous);fail_close=1;require(create_nyx_config_entry(false)==1&&!rename_count&&!fs_temp,"close error never commits");require(fs_final==!!previous&&!strcmp(final_bytes,"previous-complete-config"),"close error preserves final");++cases;
  reset_fs(previous);fail_stat=1;require(create_nyx_config_entry(false)==1&&!rename_count&&!fs_temp,"stat error never commits");require(fs_final==!!previous,"stat error preserves final");++cases;
  reset_fs(previous);require(create_nyx_config_entry(false)==0&&fs_final&&!fs_backup&&!fs_temp,"complete save committed");require(!strcmp(final_bytes,written)&&strstr(final_bytes,"nova_logo_color=123456"),"all bytes committed");++cases;
 }
 reset_fs(true);fail_remove_backup=1;require(create_nyx_config_entry(false)==1&&fs_final&&!strcmp(final_bytes,"previous-complete-config"),"backup cleanup error preserves final");++cases;
 for(int at=1;at<=2;++at){reset_fs(true);fail_rename=at;require(create_nyx_config_entry(false)==1&&fs_final&&!strcmp(final_bytes,"previous-complete-config")&&!fs_temp,"rename error restores previous final");++cases;}
 reset_fs(true);fail_rename=2;fail_restore=1;require(create_nyx_config_entry(false)==1&&!fs_final&&fs_backup&&!strcmp(backup_bytes,"previous-complete-config"),"failed restore retains recovery backup");require(!strcmp(load_path(),"bootloader/nyx.ini.nova.bak"),"startup selects recovery backup");++cases;
 reset_fs(false);fail_rename=1;require(create_nyx_config_entry(false)==1&&!fs_final&&!fs_temp,"first-install commit error does not create final");++cases;
 reset_fs(true);require(!strcmp(load_path(),"bootloader/nyx.ini"),"startup prefers complete final");fs_final=false;fs_backup=true;require(!strcmp(load_path(),"bootloader/nyx.ini.nova.bak"),"power-loss recovery selects old file");++cases;
 printf("PASS: %d actual-C transactional INI cases; all 57 write failures with/without prior config, close/stat/unlink/rename/restore failures and original startup backup recovery.\n",cases);return 0;
}
'''
summaries.append(run('transaction',base+'\n'+constants+'\n'+types+'\nstatic nyx_config n_cfg;\n'+code+recovery+faults))
(work/'nova-v12-check.json').write_text(json.dumps({'checks':summaries,'original_clock_preserved':True,'first_boot_entry_points':1,'hardware_validation':False},indent=2))
