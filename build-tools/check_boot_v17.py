"""Exercise actual splash rendering, cooperative timing, color parser and countdown."""
from pathlib import Path
import re,subprocess,json
from PIL import Image
work=Path(__file__).parent.resolve();root=work/'hekate-6.5.4';folder=work/'boot-check-v17';folder.mkdir(exist_ok=True)
header=(root/'bdk/display/nova_splash.h').read_text().replace('#include <utils/types.h>','')
(folder/'under_test.h').write_text(header)
source=(root/'bootloader/gfx/logos.c').read_text()
def function(name):
    m=re.search(r'^(?:static )?(?:u8 \*|bool |void |u32 )'+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert m,name
    end=re.search(r'^\}',source[m.end():],re.M);return source[m.start():m.end()+end.end()]
animation=re.sub(r'^#include[^\n]*\n','',(root/'bdk/display/nova_splash.c').read_text(),flags=re.M)
code=r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
typedef uint32_t u32;typedef uint8_t u8;
#define BTN_VOL_DOWN 2
#define FR_OK 0
#define INI_CHOICE 3
#include "under_test.h"
static struct{u32*fb;}gfx_ctxt;
static u32 now,button_calls,press_at,backlights,checks;
void (*timer_progress_hook)(u32);
static u32 get_tmr_us(void){if(timer_progress_hook)timer_progress_hook(now);return now;}
static void usleep(u32 us){now+=us;get_tmr_us();}
static u32 btn_read_vol(void){++button_calls;return press_at&&button_calls>=press_at?BTN_VOL_DOWN:0;}
static void display_backlight_brightness(u32 brightness,u32 fade){assert(brightness==100&&fade==1000);++backlights;}
typedef struct{void *first;}List;
typedef struct kv{char*key,*val;struct kv*next;}ini_kv_t;
typedef struct sec{char*name;u32 type;List kvs;struct sec*next;}ini_sec_t;
#define LIST_INIT(x) List x={0}
#define LIST_FOREACH_ENTRY(type,var,list,member) for(type *var=(list)->first;var;var=var->next)
static bool mounted,load_once,primary;
static int mount_error,parse_error,mounts,unmounts,parses,frees;
static char parsed_path[128];
static ini_kv_t bar={"boot_bar_color","123456",NULL},logo={"boot_logo_color","ABCDEF",&bar};
static ini_sec_t config={"config",INI_CHOICE,{&logo},NULL};
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){++mounts;if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){++unmounts;mounted=false;}
static int f_stat(const char *path,void *info){(void)path;(void)info;return primary?FR_OK:4;}
static int ini_parse(List *list,const char *path,bool directory){assert(!directory);++parses;strcpy(parsed_path,path);list->first=&config;return parse_error;}
static void ini_free(List *list){(void)list;++frees;}
'''+animation+'\n'+function('splash_rgb')+'\n'+function('splash_colors_load').replace('static bool loaded;','').replace('loaded','load_once')+'\n'+function('render_static_bootlogo')+'\n'+function('render_ticker_logo')+r'''
#define CHECK(x)do{checks++;assert(x);}while(0)
static void reset_load(void){nova_splash_set_colors(0x55C9F1,0x55C9F1);load_once=mounted=false;mount_error=parse_error=mounts=unmounts=parses=frees=0;primary=true;config.name="config";config.type=INI_CHOICE;logo.val="ABCDEF";bar.val="123456";}
static u32 pixel(u32 x,u32 y){return gfx_ctxt.fb[(1279-x)*720+y];}
static u32 filled(void){u32 n=0;for(u32 x=520;x<760;++x)if(pixel(x,426)!=0x273241)++n;return n;}
int main(void){
 u32*guarded=malloc((720*1280+32)*4);assert(guarded);for(u32 i=0;i<720*1280+32;i++)guarded[i]=0xDEADBEEF;gfx_ctxt.fb=guarded+16;
 u32 position=0;for(u32 i=0;i<sizeof(nova_boot_rle);i+=2){CHECK(nova_boot_rle[i]&&nova_boot_rle[i+1]<16);position+=nova_boot_rle[i];}CHECK(position==400*190);
 CHECK(splash_rgb("000000")==0&&splash_rgb("ffffff")==0xFFFFFF&&splash_rgb("aBcDeF")==0xABCDEF);
 const char*bad[]={"","12345","1234567","#ABCDE","12345g"," 12345","12345\n"};for(u32 i=0;i<7;++i)CHECK(splash_rgb(bad[i])==0x55C9F1);
 for(u32 own=0;own<2;++own)for(u32 exists=0;exists<2;++exists){reset_load();mounted=own;primary=exists;splash_colors_load();CHECK(nova_splash_logo_color==0xABCDEF&&nova_splash_bar_color==0x123456&&unmounts==!own&&mounted==own&&frees==1);CHECK(!strcmp(parsed_path,exists?"bootloader/nyx.ini":"bootloader/nyx.ini.nova.bak"));splash_colors_load();CHECK(parses==1);}
 reset_load();mount_error=1;splash_colors_load();CHECK(!parses&&!unmounts&&nova_splash_bar_color==0x55C9F1);
 reset_load();parse_error=1;splash_colors_load();CHECK(frees==1&&unmounts==1&&nova_splash_logo_color==0x55C9F1);
 reset_load();config.name="other";splash_colors_load();CHECK(nova_splash_logo_color==0x55C9F1);
 reset_load();logo.val="broken";bar.val="000000";splash_colors_load();CHECK(nova_splash_logo_color==0x55C9F1&&nova_splash_bar_color==0);
 reset_load();mount_error=1;render_static_bootlogo();
 FILE*image=fopen("splash.ppm","wb");assert(image);fprintf(image,"P6\n1280 720\n255\n");for(u32 y=0;y<720;++y)for(u32 x=0;x<1280;++x){u32 c=pixel(x,y);u8 rgb[]={c>>16,c>>8,c};fwrite(rgb,1,3,image);}fclose(image);
 const u32 waits[]={0,1,3,20,21,0xFFFFFFFF};
 for(u32 i=0;i<6;i++){now=button_calls=press_at=backlights=0;u32 duration=waits[i]>20?3000000:waits[i]*1000000;CHECK(!render_ticker_logo(waits[i],100));CHECK(now==duration&&backlights==1);}
 for(u32 n=1;n<=8;++n){now=button_calls=backlights=0;press_at=n;CHECK(render_ticker_logo(3,100));CHECK(now==(n-1)*50000&&backlights==1);}
 now=0xFFFFFFFF-100000;u32 start=now;button_calls=press_at=backlights=0;CHECK(!render_ticker_logo(3,100));CHECK((u32)(now-start)==3000000);
 nova_splash_set_colors(0xFF0000,0x00FF00);nova_splash_render(gfx_ctxt.fb);CHECK(nova_splash_tint(0xFF0000,0)==0x10151D&&nova_splash_tint(0xFF0000,255)==0xFF0000);
 nova_splash_progress(gfx_ctxt.fb,0,100);CHECK(pixel(520,425)==0x00FF00&&pixel(759,427)==0x00FF00);
 nova_splash_set_colors(0x55C9F1,0x55C9F1);now=0;u32 last=0; nova_splash_stage(gfx_ctxt.fb,0,35);
 for(u32 i=0;i<1000;++i){now+=12345;get_tmr_us();u32 f=filled();CHECK(f>=last&&f<84);last=f;}
 CHECK(splash_percent==34&&timer_progress_hook);
 nova_splash_animate(gfx_ctxt.fb,0,35,300);CHECK(filled()==84&&!timer_progress_hook);
 nova_splash_stage(gfx_ctxt.fb,35,50);for(u32 i=0;i<100;++i){now+=20000;get_tmr_us();}CHECK(splash_percent==49);
 nova_splash_pause();CHECK(!timer_progress_hook&&splash_frame==gfx_ctxt.fb);u32 held=filled();now+=1000000;get_tmr_us();CHECK(filled()==held);
 nova_splash_animate(gfx_ctxt.fb,35,50,200);CHECK(filled()==120&&!timer_progress_hook);
 now=UINT32_MAX-5000;nova_splash_stage(gfx_ctxt.fb,35,65);for(u32 i=0;i<200;++i){now+=20000;get_tmr_us();CHECK(splash_percent>=35&&splash_percent<65);}CHECK(splash_percent==64);
 nova_splash_stop();CHECK(!timer_progress_hook&&!splash_frame);u32 saved=filled();now+=1000000;get_tmr_us();CHECK(filled()==saved);
 nova_splash_animate(gfx_ctxt.fb,85,100,500);CHECK(filled()==240);
 for(u32 i=0;i<16;++i){CHECK(guarded[i]==0xDEADBEEF);CHECK(guarded[720*1280+16+i]==0xDEADBEEF);}
 free(guarded);printf("PASS: %u actual C splash checks; centered bounds, independent colors, INI fallback/ownership, countdown/cancel, cooperative progress caps, wraparound and callback cleanup.\n",checks);return 0;
}
'''
program=folder/'check.c';program.write_text(code);exe=folder/'check.exe';cc=work/'toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2','-I'+str(root/'bdk/display'),str(program),'-o',str(exe)],check=True)
run=subprocess.run([str(exe)],cwd=folder,capture_output=True,text=True,check=True);print(run.stdout.strip())
Image.open(folder/'splash.ppm').save(folder/'splash.png')
(work/'boot-check-v17.json').write_text(json.dumps({'checks':int(re.search(r'(\d+) actual C',run.stdout)[1]),'art':[440,245,400,190],'progress':[520,425,240,3],'update_ms':20,'method':'cooperative timer polling; suspended for training and handoff','hardware':'not tested'},indent=2))
