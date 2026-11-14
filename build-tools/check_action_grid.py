"""Execute real action-card builders and preserve destructive action bindings."""
from pathlib import Path
import re
import subprocess

work=Path(__file__).resolve().parent
frontend=work/'hekate-6.5.4/nyx/nyx_gui/frontend'
layout=(frontend/'gui_layout.c').read_text(encoding='utf-8')
layout=re.sub(r'^#include[^\n]*\n','',layout,flags=re.M)
constructors=[]
for filename,name in [('gui_tools.c','create_tab_tools'),('gui_info.c','create_tab_info')]:
    text=(frontend/filename).read_text(encoding='utf-8')
    constructors.append(text[text.index('void '+name+'('):])
preamble=r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;typedef int lv_coord_t;typedef int lv_res_t;
typedef struct object lv_obj_t;typedef lv_res_t(*lv_action_t)(lv_obj_t*);
typedef struct{int height;} font_t;
typedef struct{struct{bool empty;struct{int hor,ver,inner;}padding;}body;struct{font_t*font;}text;}lv_style_t;
typedef struct{lv_style_t*panel;}lv_theme_t;
struct object{lv_obj_t*parent,*children[12];u32 count;int kind,x,y,w,h,scroll,layout,state,mode,id;bool clickable,hidden;void*free_ptr;const char*text;lv_style_t*style;lv_action_t click,long_press;};
typedef struct{lv_theme_t*theme;lv_obj_t*page,*previous_card;lv_coord_t y,row_height;u32 count;}nyx_action_grid;
enum{LV_LAYOUT_OFF,LV_LAYOUT_CENTER,LV_LABEL_LONG_BREAK,LV_LABEL_ALIGN_CENTER,LV_BTN_ACTION_CLICK,LV_BTN_ACTION_LONG_PR,LV_BTN_STATE_REL,LV_BTN_STATE_TGL_REL,LV_BTN_STATE_INA,LV_PAGE_STYLE_BG,LV_PAGE_STYLE_SCRL,LV_SB_MODE_AUTO,LV_SB_MODE_OFF};
#define LV_HOR_RES 1280
#define LV_RES_OK 0
#define SYMBOL_UPLOAD ""
#define SYMBOL_DOWNLOAD ""
#define SYMBOL_SD ""
#define SYMBOL_USB ""
#define SYMBOL_DIRECTORY ""
#define SYMBOL_KEYBOARD ""
#define SYMBOL_REFRESH ""
#define SYMBOL_MODULES ""
#define SYMBOL_CHIP ""
#define SYMBOL_KEY ""
#define SYMBOL_CIRCUIT ""
#define SYMBOL_SHUFFLE ""
#define SYMBOL_BATTERY_FULL ""
static font_t regular_font={30},small_font={20};static lv_style_t regular_style={.text={&regular_font}},hint_small_style={.text={&small_font}},lv_style_transp_tight;
static lv_theme_t theme={&regular_style};
static lv_obj_t pool[128];static u32 used;static int expansion=1;static bool lockpick_available,autorcm_enabled;static struct{bool rcm_patched;}h_cfg;static lv_obj_t*autorcm_btn;
static lv_obj_t*create(lv_obj_t*p,int kind){assert(used<128);lv_obj_t*o=&pool[used++];memset(o,0,sizeof(*o));o->parent=p;o->kind=kind;o->clickable=true;o->style=&regular_style;if(p){assert(p->count<12);p->children[p->count++]=o;}return o;}
static lv_obj_t*lv_cont_create(lv_obj_t*p,void*c){(void)c;return create(p,1);}
static lv_obj_t*lv_btn_create(lv_obj_t*p,void*c){(void)c;return create(p,2);}
static lv_obj_t*lv_label_create(lv_obj_t*p,void*c){(void)c;return create(p,3);}
static void lv_style_copy(lv_style_t*d,lv_style_t*s){*d=*s;}
static void lv_cont_set_style(lv_obj_t*o,lv_style_t*s){o->style=s;}
static void lv_cont_set_fit(lv_obj_t*o,bool h,bool v){(void)o;(void)h;(void)v;}
static void lv_cont_set_layout(lv_obj_t*o,int l){o->layout=l;}
static void lv_page_set_style(lv_obj_t*o,int t,lv_style_t*s){(void)o;(void)t;(void)s;}
static void lv_page_set_scrl_layout(lv_obj_t*o,int l){o->layout=l;}
static void lv_page_set_scrl_fit(lv_obj_t*o,bool h,bool v){(void)o;(void)h;(void)v;}
static void lv_page_set_scrl_width(lv_obj_t*o,int w){o->w=w;}
static void lv_page_set_scrl_height(lv_obj_t*o,int h){o->scroll=h;}
static lv_obj_t*lv_page_get_scrl(lv_obj_t*o){return o;}
static void lv_obj_set_drag(lv_obj_t*o,bool drag){(void)o;(void)drag;}
static void lv_page_set_sb_mode(lv_obj_t*o,int m){(void)o;(void)m;}
static void lv_obj_set_click(lv_obj_t*o,bool click){o->clickable=click;}
static void lv_obj_set_width(lv_obj_t*o,int w){o->w=w;}
static void lv_obj_set_height(lv_obj_t*o,int h){o->h=h;}
static int lv_obj_get_height(lv_obj_t*o){return o->h;}
static int lv_font_get_height(const font_t*f){return f->height;}
static int lv_obj_get_width(lv_obj_t*o){return o->w;}
static void lv_obj_set_size(lv_obj_t*o,int w,int h){o->w=w;o->h=h;}
static void lv_obj_set_hidden(lv_obj_t*o,bool h){o->hidden=h;}
static void lv_obj_set_free_ptr(lv_obj_t*o,void*p){o->free_ptr=p;}
static void*lv_obj_get_free_ptr(lv_obj_t*o){return o->free_ptr;}
static const char*shown_help;
static void nyx_show_help(const char*t){shown_help=t;}
static void lv_label_set_text(lv_obj_t*,const char*);
static void lv_obj_set_pos(lv_obj_t*o,int x,int y){o->x=x;o->y=y;}
static void lv_obj_set_free_num(lv_obj_t*o,u32 id){o->id=id;}
static void lv_btn_set_action(lv_obj_t*o,int type,lv_action_t a){if(type==LV_BTN_ACTION_CLICK)o->click=a;else o->long_press=a;}
static void lv_btn_set_fit(lv_obj_t*o,bool h,bool v){(void)o;(void)h;(void)v;}
static void lv_btn_set_layout(lv_obj_t*o,int l){o->layout=l;}
static void lv_btn_set_state(lv_obj_t*o,int s){o->state=s;}
static void lv_label_set_style(lv_obj_t*o,lv_style_t*s){o->style=s;if(o->text)lv_label_set_text(o,o->text);}
static void lv_label_set_recolor(lv_obj_t*o,bool r){(void)o;(void)r;}
static void lv_label_set_long_mode(lv_obj_t*o,int m){o->mode=m;}
static void lv_label_set_align(lv_obj_t*o,int a){(void)o;(void)a;}
static void lv_label_set_text(lv_obj_t*o,const char*t){o->text=t;int h=o->style->text.font->height;int pixels=strlen(t)*h/2*expansion;if(!o->w)o->w=o->parent?o->parent->w:1280;int lines=(pixels+o->w-1)/o->w;if(lines<1)lines=1;o->h=lines*h+(lines-1)*2;}

static bool get_set_autorcm_status(bool toggle){(void)toggle;return autorcm_enabled;}
static void nyx_generic_onoff_toggle(lv_obj_t*b){assert(b->count==1&&b->children[0]->kind==3);}
static bool _lockpick_exists_check(void){return lockpick_available;}
#define ACTION(name) static lv_res_t name(lv_obj_t*b){(void)b;return 0;}
ACTION(create_window_backup_restore_tool) ACTION(create_window_sd_partition_manager) ACTION(create_window_emmc_partition_manager)
ACTION(_create_window_usb_tools) ACTION(_create_window_unset_abit_tool) ACTION(_create_mbox_fix_touchscreen) ACTION(_create_mbox_autorcm_status) ACTION(_create_window_dump_pk12_tool)
ACTION(_create_window_bootrom_info_status) ACTION(_create_mbox_lockpick) ACTION(_create_window_hw_info_status) ACTION(_kfuse_dump_window_action) ACTION(_create_window_emmc_info_status) ACTION(_create_window_sdcard_info_status) ACTION(_create_window_battery_status)
'''
harness=r'''
static unsigned assertions;
#define CHECK(c)do{assertions++;assert(c);}while(0)
static lv_obj_t*button(lv_obj_t*p,u32 index){CHECK(p->count>index);CHECK(p->children[index]->kind==1);CHECK(p->children[index]->count==3);return p->children[index]->children[0];}
static void geometry(lv_obj_t*p){
for(u32 i=0;i<p->count;i++){
 lv_obj_t*c=p->children[i],*b=c->children[0],*h=c->children[1],*info=c->children[2];
 CHECK(c->x>=32&&c->x+c->w<=1280-32);CHECK(c->y>=16&&c->y+c->h<=p->h-16);
 CHECK(b->y+b->h<=c->h-12);CHECK(b->x+b->w<=info->x-8);CHECK(b->count==1);
 if(!h->hidden){CHECK(b->y+b->h<h->y);CHECK(h->y+h->h<=c->h-12);}
 CHECK(info->click!=NULL);info->click(info);CHECK(shown_help==h->text);
 if(i&1){lv_obj_t*left=p->children[i-1];CHECK(c->y==left->y);CHECK(c->h==left->h);CHECK(c->x>=left->x+left->w+16);}
 else if(i>=2){lv_obj_t*previous=p->children[i-2];CHECK(c->y>=previous->y+previous->h+12);}
}CHECK(p->scroll==p->h);}

int main(void){
for(int large=0;large<2;large++)for(int guarded=0;guarded<2;guarded++){
memset(pool,0,sizeof(pool));used=0;regular_font.height=large?36:30;small_font.height=large?24:20;expansion=1;autorcm_enabled=guarded;h_cfg.rcm_patched=guarded;lockpick_available=!guarded;
lv_obj_t tools={.w=1280,.h=592};create_tab_tools(&theme,&tools);CHECK(tools.count==8);geometry(&tools);
lv_action_t tools_actions[]={create_window_backup_restore_tool,create_window_backup_restore_tool,create_window_sd_partition_manager,_create_window_usb_tools,_create_window_unset_abit_tool,_create_mbox_fix_touchscreen,_create_mbox_autorcm_status,_create_window_dump_pk12_tool};
for(u32 i=0;i<8;i++){CHECK(button(&tools,i)->click==tools_actions[i]);}
CHECK(button(&tools,0)->id==0);CHECK(button(&tools,1)->id==1);CHECK(button(&tools,2)->long_press==create_window_emmc_partition_manager);CHECK(autorcm_btn==button(&tools,6));CHECK(autorcm_btn->state==(guarded?LV_BTN_STATE_INA:LV_BTN_STATE_REL));CHECK(autorcm_btn->clickable==!guarded);
lv_obj_t info={.w=1280,.h=592};create_tab_info(&theme,&info);CHECK(info.count==7);geometry(&info);
lv_action_t info_actions[]={_create_window_bootrom_info_status,_create_mbox_lockpick,_create_window_hw_info_status,_kfuse_dump_window_action,_create_window_emmc_info_status,_create_window_sdcard_info_status,_create_window_battery_status};
for(u32 i=0;i<7;i++){CHECK(button(&info,i)->click==info_actions[i]);}
CHECK(button(&info,1)->clickable==!guarded);CHECK(button(&info,1)->state==(guarded?LV_BTN_STATE_INA:0));
}
printf("PASS: %u actual-C action-grid assertions; Tools/Info callbacks, backup/restore IDs, long-press, AutoRCM/Lockpick guards and large-font geometry\n",assertions);return 0;}
'''
directory=work/'action-grid-check';directory.mkdir(exist_ok=True)
test=directory/'test.c';test.write_text(preamble+layout+'\n'.join(constructors)+harness,encoding='utf-8')
executable=directory/'test.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-Wall','-Wextra',str(test),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
