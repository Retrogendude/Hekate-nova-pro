"""Execute the actual conservative label fitting code with LVGL object doubles."""
from pathlib import Path
import re
import subprocess

work = Path(__file__).resolve().parent
source = (work / 'hekate-6.5.4/nyx/nyx_gui/frontend/gui_readability.c').read_text(encoding='utf-8')
source = re.sub(r'^#include[^\n]*\n', '', source, flags=re.M)
directory = work / 'gui-readability-check'
directory.mkdir(exist_ok=True)
preamble = r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef uint32_t u32;typedef int lv_coord_t;typedef int lv_label_long_mode_t;typedef int lv_txt_flag_t;
enum{LV_LABEL_LONG_EXPAND,LV_LABEL_LONG_BREAK,LV_LABEL_LONG_SCROLL,LV_LABEL_LONG_DOT,LV_LABEL_LONG_ROLL,LV_LABEL_LONG_CROP};
enum{LV_TXT_FLAG_NONE,LV_TXT_FLAG_RECOLOR,LV_LAYOUT_OFF=0,LV_LAYOUT_CENTER=1,LV_TASK_PRIO_LOW=2};
typedef struct {int height;} lv_font_t;
typedef struct {struct {const lv_font_t *font;int letter_space;} text;struct {struct {int hor;}padding;}body;}lv_style_t;
typedef struct _object lv_obj_t;
struct _object {const char*type;lv_obj_t*parent,*children[4];u32 count;int width,height,x,y,mode,layout;bool fit,hidden,recolor;const char*text;lv_style_t*style;};
typedef struct {const char*type[4];}lv_obj_type_t;
typedef struct {void(*callback)(void*);bool once,ready;}lv_task_t;
static lv_obj_t *screen;static lv_task_t task;static u32 task_creations;
static void lv_obj_get_type(lv_obj_t*o,lv_obj_type_t*t){memset(t,0,sizeof(*t));t->type[0]=o->type;}
static lv_obj_t*lv_obj_get_parent(lv_obj_t*o){return o->parent;}
static bool lv_cont_get_hor_fit(lv_obj_t*o){return o->fit;}
static int lv_obj_get_width(lv_obj_t*o){return o->width;}
static int lv_obj_get_height(lv_obj_t*o){return o->height;}
static int lv_obj_get_x(lv_obj_t*o){return o->x;}
static int lv_obj_get_y(lv_obj_t*o){return o->y;}
static int lv_label_get_long_mode(lv_obj_t*o){return o->mode;}
static const char*lv_label_get_text(lv_obj_t*o){return o->text;}
static lv_style_t*lv_label_get_style(lv_obj_t*o){return o->style;}
static lv_style_t*lv_obj_get_style(lv_obj_t*o){return o->style;}
static int lv_font_get_height(const lv_font_t*f){return f->height;}
static lv_obj_t*lv_obj_get_child(lv_obj_t*o,lv_obj_t*last){if(!last)return o->count?o->children[0]:NULL;for(u32 i=0;i+1<o->count;i++)if(o->children[i]==last)return o->children[i+1];return NULL;}
static bool lv_label_get_recolor(lv_obj_t*o){return o->recolor;}
static int lv_txt_get_width(const char*t,size_t bytes,const lv_font_t*f,int spacing,int flags){(void)t;(void)f;(void)spacing;(void)flags;return bytes*18;}
static void lv_label_set_long_mode(lv_obj_t*o,int mode){o->mode=mode;}
static void lv_obj_set_width(lv_obj_t*o,int width){o->width=width;}
static void lv_obj_set_x(lv_obj_t*o,int x){o->x=x;}
static int lv_btn_get_layout(lv_obj_t*o){return o->layout;}
static bool lv_obj_get_hidden(lv_obj_t*o){return o->hidden;}
static lv_obj_t*lv_scr_act(void){return screen;}
static lv_task_t*lv_task_create(void(*cb)(void*),int period,int priority,void*data){(void)period;(void)priority;(void)data;task_creations++;task.callback=cb;return &task;}
static void lv_task_once(lv_task_t*t){t->once=true;}
static void lv_task_ready(lv_task_t*t){t->ready=true;}
'''
harness = r'''
static u32 cases;
#define CHECK(x)do{cases++;assert(x);}while(0)
int main(void){
lv_font_t font={36};lv_style_t style={.text={&font,1},.body={.padding={12}}};
const char*long_text="A translated label that is much longer than its fixed button";
lv_obj_t button={.type="lv_btn",.width=212,.height=64,.style=&style,.count=1};
lv_obj_t label={.type="lv_label",.parent=&button,.text=long_text,.style=&style,.height=36,.width=999,.x=-100};button.children[0]=&label;
nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_ROLL);CHECK(label.width==188);CHECK(label.x==12);CHECK(!strcmp(label.text,long_text));CHECK(button.height==64);
label.mode=LV_LABEL_LONG_EXPAND;label.text="Short";nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);
label.text=long_text;button.fit=true;nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);button.fit=false;
button.width=120;nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);button.width=212;
button.height=30;nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);button.height=64;
for(int mode=LV_LABEL_LONG_BREAK;mode<=LV_LABEL_LONG_CROP;mode++){if(mode==LV_LABEL_LONG_ROLL)continue;label.mode=mode;nyx_label_adapt(&label);CHECK(label.mode==mode);}label.mode=LV_LABEL_LONG_EXPAND;
label.text="First\nSecond";nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);label.text=long_text;
button.type="lv_cont";nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);button.type="lv_btn";
lv_obj_t state={.type="lv_label",.text="OFF",.parent=&button,.style=&style,.x=300,.y=10,.width=72};button.children[1]=&state;button.count=2;button.width=400;label.x=24;label.y=10;
nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_ROLL);CHECK(label.width==264);CHECK(state.width==72&&state.x==300);nyx_label_adapt(&state);CHECK(state.mode==LV_LABEL_LONG_EXPAND);
label.mode=LV_LABEL_LONG_EXPAND;state.y=100;nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);state.y=10;
state.type="lv_img";nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);state.type="lv_label";
lv_obj_t third={.type="lv_label",.text="detail",.parent=&button,.style=&style};button.count=3;button.children[2]=&third;nyx_label_adapt(&label);CHECK(label.mode==LV_LABEL_LONG_EXPAND);button.count=1;
lv_obj_t root={.type="lv_obj",.count=1};root.children[0]=&button;button.parent=&root;button.width=212;button.hidden=true;nyx_readability_refresh(&root);CHECK(label.mode==LV_LABEL_LONG_EXPAND);
button.hidden=false;nyx_readability_refresh(&root);CHECK(label.mode==LV_LABEL_LONG_ROLL);
screen=&root;nyx_readability_request();nyx_readability_request();CHECK(task_creations==1);CHECK(task.once&&task.ready);task.callback(NULL);CHECK(!pending);nyx_readability_request();CHECK(task_creations==2);
printf("PASS: %u actual-C readability assertions: long buttons, ON/OFF reservation, automatic layouts, compound cards and coalesced refresh\n",cases);return 0;}
'''
test = directory/'test.c'
test.write_text(preamble+source+harness, encoding='utf-8')
executable=directory/'test.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-Wall','-Wextra',str(test),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
