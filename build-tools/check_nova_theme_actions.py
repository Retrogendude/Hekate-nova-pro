"""Run the real theme save/apply actions with controlled SD failures."""
from pathlib import Path
import re,subprocess
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui'
source=(root/'frontend/gui_nova_theme.c').read_text();header=(root/'config.h').read_text()
types=re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;',header,re.S).group()
def function(name):
    m=re.search(r'^static (?:u32\s*\*|void|lv_res_t)\s*'+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert m,name
    end=re.search(r'^\}',source[m.end():],re.M);return source[m.start():m.end()+end.end()]
preamble=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;typedef int lv_res_t;typedef struct{int x;}lv_obj_t;typedef struct{int x;}lv_img_dsc_t;
#define LV_RES_OK 0
#define FR_OK 0
enum{NOVA_COLOR_BACKGROUND,NOVA_COLOR_TEXT,NOVA_COLOR_BORDER,NOVA_COLOR_REFLECTION};
'''+types+r'''
static nyx_config n_cfg,saved;static struct{nyx_config pending;u32 target;}nova_theme;
static bool mounted,dirty,invalid_wallpaper,active,saved_flag;
static int mount_error,stat_error,boot_error,write_error,mounts,unmounts,boot_writes,writes,reloads,notices,wallpaper_reads,wallpaper_frees;
static char message[256];static lv_img_dsc_t image;
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){mounts++;if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){unmounts++;mounted=false;}
static int f_stat(const char*p,void*x){(void)x;assert(!strcmp(p,"bootloader/sys/nyx.bin"));return stat_error;}
static lv_img_dsc_t*wallpaper_read(const char*p,u32 w,u32 h){wallpaper_reads++;assert(!strcmp(p,"Nova-Carbon.bmp")&&w==32&&h==18);return invalid_wallpaper?NULL:&image;}
static void wallpaper_free(lv_img_dsc_t*p){assert(p==&image);wallpaper_frees++;}
static bool nyx_options_commit_boot(void){if(!dirty)return true;boot_writes++;if(boot_error)return false;dirty=false;return true;}
static int create_nyx_config_entry(bool force){assert(force&&mounted);writes++;saved=n_cfg;sd_unmount();return write_error;}
static void nyx_options_appearance_active(bool a,bool s){active=a;saved_flag=s;}
static void reload_nyx(lv_obj_t*o,bool force){assert(!o&&!force);reloads++;}
static void theme_notice(const char*s){strcpy(message,s);notices++;}
'''
tests=r'''
static unsigned checks;
#define CHECK(x)do{checks++;assert(x);}while(0)
static void reset(void){memset(&n_cfg,0,sizeof n_cfg);n_cfg.theme_bg=0x141A19;n_cfg.theme_color=173;n_cfg.nova_text_color=0xF2F5FA;n_cfg.nova_border_color=0x334050;n_cfg.nova_reflection_color=0x55C9F1;n_cfg.nova_menu_opacity=n_cfg.nova_icon_opacity=100;strcpy(n_cfg.font_file,"Lecture.nyxf");n_cfg.font_size=30;n_cfg.language=4;n_cfg.verification=2;nova_theme.pending=n_cfg;nova_theme.pending.theme_bg=0x080A0D;nova_theme.pending.nova_text_color=0xAABBCC;nova_theme.pending.nova_border_color=0x112233;nova_theme.pending.nova_reflection_color=0x778899;nova_theme.pending.nova_menu_opacity=50;nova_theme.pending.nova_icon_opacity=25;strcpy(nova_theme.pending.wallpaper_file,"Nova-Carbon.bmp");mounted=invalid_wallpaper=active=saved_flag=false;dirty=true;mount_error=stat_error=boot_error=write_error=mounts=unmounts=boot_writes=writes=reloads=notices=wallpaper_reads=wallpaper_frees=0;message[0]=0;}
int main(void){
 for(u32 role=0;role<4;role++){reset();nova_theme.target=role;u32*target=theme_target_color();CHECK(target==(role==0?&nova_theme.pending.theme_bg:role==1?&nova_theme.pending.nova_text_color:role==2?&nova_theme.pending.nova_border_color:&nova_theme.pending.nova_reflection_color));}
 reset();nyx_config before=n_cfg;mount_error=1;theme_save(NULL);CHECK(!writes&&!reloads&&!boot_writes);CHECK(!memcmp(&before,&n_cfg,sizeof before));CHECK(!mounted&&notices==1);
 for(int was_mounted=0;was_mounted<2;was_mounted++){
  reset();mounted=was_mounted;before=n_cfg;stat_error=1;theme_save(NULL);CHECK(!writes&&!reloads&&!boot_writes&&!wallpaper_reads);CHECK(!memcmp(&before,&n_cfg,sizeof before));CHECK(mounted==!!was_mounted&&unmounts==!was_mounted);
  reset();mounted=was_mounted;before=n_cfg;invalid_wallpaper=true;theme_save(NULL);CHECK(wallpaper_reads==1&&!wallpaper_frees&&!writes&&!reloads);CHECK(!memcmp(&before,&n_cfg,sizeof before));CHECK(mounted==!!was_mounted&&unmounts==!was_mounted);
  reset();mounted=was_mounted;before=n_cfg;boot_error=1;theme_save(NULL);CHECK(wallpaper_reads==1&&wallpaper_frees==1&&boot_writes==1&&!writes&&!reloads);CHECK(!memcmp(&before,&n_cfg,sizeof before)&&dirty);CHECK(mounted==!!was_mounted&&unmounts==!was_mounted);
 }
 reset();before=n_cfg;write_error=1;theme_save(NULL);CHECK(boot_writes==1&&writes==1&&!reloads);CHECK(!memcmp(&before,&n_cfg,sizeof before)&&!dirty);CHECK(!active&&!saved_flag&&!mounted);
 write_error=0;theme_save(NULL);CHECK(boot_writes==1&&writes==2&&reloads==1);CHECK(active&&saved_flag&&!mounted);CHECK(n_cfg.nova_menu_opacity==50&&n_cfg.nova_icon_opacity==25&&!strcmp(n_cfg.wallpaper_file,"Nova-Carbon.bmp"));CHECK(n_cfg.nova_text_color==0xAABBCC&&n_cfg.nova_border_color==0x112233&&n_cfg.nova_reflection_color==0x778899&&n_cfg.theme_bg==0x080A0D);CHECK(n_cfg.theme_color==173&&n_cfg.language==4&&n_cfg.font_size==30&&n_cfg.verification==2&&!strcmp(n_cfg.font_file,"Lecture.nyxf"));
 reset();nova_theme.pending.wallpaper_file[0]=0;theme_save(NULL);CHECK(!wallpaper_reads&&!wallpaper_frees&&writes==1&&reloads==1&&!n_cfg.wallpaper_file[0]);
 printf("PASS: %u actual-C theme action checks; independent targets, missing SD/Nyx/wallpaper, mount ownership, boot-save failure, write rollback/retry, successful save and unrelated settings preserved.\n",checks);return 0;
}
'''
directory=work/'nova-theme-actions-check';directory.mkdir(exist_ok=True);program=directory/'check.c'
program.write_text(preamble+'\n'+'\n'.join(function(name)for name in ('theme_target_color','theme_apply','theme_save'))+tests)
exe=directory/'check.exe';cc=work/'toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2',str(program),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
