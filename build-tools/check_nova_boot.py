"""Render/test the exact early boot splash and cancellation loop on the host."""
from pathlib import Path
import re,subprocess,json
from PIL import Image
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4';folder=work/'nova-boot-check';folder.mkdir(exist_ok=True)
header=(root/'bdk/display/nova_splash.h').read_text().replace('#include <utils/types.h>','')
(folder/'nova_splash_under_test.h').write_text(header)
source=(root/'bootloader/gfx/logos.c').read_text()
def function(name):
    m=re.search(r'^(?:u8 \*|bool )'+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert m,name
    end=re.search(r'^\}',source[m.end():],re.M);return source[m.start():m.end()+end.end()]
code=r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
typedef uint32_t u32;typedef uint8_t u8;typedef int bool;
#define true 1
#define false 0
#define BTN_VOL_DOWN 2
#include "nova_splash_under_test.h"
static struct{u32*fb;}gfx_ctxt;
static u32 now,button_calls,press_at,backlights,checks;
static u32 get_tmr_us(void){return now;}
static void usleep(u32 us){now+=us;}
static u32 btn_read_vol(void){++button_calls;return press_at&&button_calls>=press_at?BTN_VOL_DOWN:0;}
static void display_backlight_brightness(u32 brightness,u32 fade){assert(brightness==100&&fade==1000);++backlights;}
'''+function('render_static_bootlogo')+'\n'+function('render_ticker_logo')+r'''
#define CHECK(x)do{checks++;assert(x);}while(0)
static void write_pixel(FILE*f,u32 x,u32 y){u32 color=gfx_ctxt.fb[(1279-x)*720+y];u8 rgb[]={color>>16,color>>8,color};assert(fwrite(rgb,1,3,f)==3);}
int main(void){
 u32*guarded=malloc((720*1280+32)*4);assert(guarded);for(u32 i=0;i<720*1280+32;i++)guarded[i]=0xDEADBEEF;gfx_ctxt.fb=guarded+16;
 u32 position=0;for(u32 i=0;i<sizeof(nova_boot_rle);i+=2){CHECK(nova_boot_rle[i]&&nova_boot_rle[i+1]<16);position+=nova_boot_rle[i];}CHECK(position==400*190);
 const u32 waits[]={0,1,3,20,21,0xFFFFFFFF};
 for(u32 i=0;i<6;i++){now=button_calls=press_at=backlights=0;u32 duration=waits[i]>20?3000000:waits[i]*1000000;CHECK(!render_ticker_logo(waits[i],100));CHECK(now==duration&&backlights==1);}
 for(u32 n=1;n<=8;++n){now=button_calls=backlights=0;press_at=n;CHECK(render_ticker_logo(3,100));CHECK(now==(n-1)*50000&&backlights==1);}
 now=0xFFFFFFFF-100000;u32 start=now;button_calls=press_at=backlights=0;CHECK(!render_ticker_logo(3,100));CHECK((u32)(now-start)==3000000);
 for(u32 i=0;i<16;++i){CHECK(guarded[i]==0xDEADBEEF);CHECK(guarded[720*1280+16+i]==0xDEADBEEF);}
 render_static_bootlogo();FILE*image=fopen("splash.ppm","wb");assert(image);fprintf(image,"P6\n1280 720\n255\n");for(u32 y=0;y<720;++y)for(u32 x=0;x<1280;++x)write_pixel(image,x,y);fclose(image);
 FILE*frames=fopen("loading.rgb","wb");assert(frames);for(u32 phase=0;phase<60;phase++){nova_splash_progress(gfx_ctxt.fb,phase,phase*100/59);for(u32 y=434;y<490;y++)for(u32 x=520;x<760;x++)write_pixel(frames,x,y);}fclose(frames);
 free(guarded);printf("PASS: %u exact-C boot checks; RLE bounds/palette, 720x1280 rotation, countdown duration, VOL- cancellation and timer wraparound.\n",checks);return 0;
}
'''
program=folder/'check.c';program.write_text(code);exe=folder/'check.exe';cc=work/'toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2','-I'+str(root/'bdk/display'),str(program),'-o',str(exe)],check=True)
run=subprocess.run([str(exe)],cwd=folder,capture_output=True,text=True,check=True);print(run.stdout.strip())
image=Image.open(folder/'splash.ppm').convert('RGB');image.save(work.parent/'outputs/apercu-demarrage-nova-pro-v11.png')
data=(folder/'loading.rgb').read_bytes();stride=240*56*3;frames=[]
for i in range(60):
    scene=image.copy();scene.paste(Image.frombytes('RGB',(240,56),data[i*stride:(i+1)*stride]),(520,434));frames.append(scene)
frames[0].save(work.parent/'outputs/chargement-nova-pro-v11.gif',save_all=True,append_images=frames[1:],duration=50,loop=0,optimize=True)
(work/'nova-boot-check.json').write_text(json.dumps({'checks':int(re.search(r'(\d+) exact-C',run.stdout)[1]),'framebuffer':'720x1280 rotated counterclockwise','animation_frames':60,'animation_frame_ms':50,'hardware':'not tested'},indent=2))
