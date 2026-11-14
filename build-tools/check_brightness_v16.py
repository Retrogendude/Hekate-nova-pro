from pathlib import Path
import re,subprocess
work=Path(__file__).resolve().parent
source=(work/'hekate-6.5.4/nyx/nyx_gui/frontend/gui_options.c').read_text(encoding='utf-8')
def function(name):
    match=re.search(r'^static lv_res_t '+name+r'\([^;]*?\)\s*\n\{',source,re.M);assert match
    return source[match.start():source.index('\n}',match.end())+2]
directory=work/'brightness-v16';directory.mkdir(exist_ok=True)
code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
typedef struct{int value;}lv_obj_t;typedef int lv_res_t;
#define LV_RES_OK 0
static struct{int backlight;}h_cfg;
static bool ini_changes_made;
static int applied,delay_value,saves;
static int lv_slider_get_value(lv_obj_t *slider){return slider->value;}
static void display_backlight_brightness(int value,int delay){applied=value;delay_value=delay;}
static int nyx_options_save_all(lv_obj_t *button){assert(button);saves++;return 17;}
'''+function('_slider_brightness_action')+'\n'+function('_save_nyx_options_action')+r'''
int main(void){
 int count=0;for(int value=30;value<=220;++value){lv_obj_t slider={value};ini_changes_made=false;
 assert(_slider_brightness_action(&slider)==0);assert(applied==value-20&&delay_value==0);
 assert(h_cfg.backlight==value&&ini_changes_made);count+=3;
 }lv_obj_t button={0};assert(_save_nyx_options_action(&button)==17&&saves==1);count++;
 printf("PASS: %d actual-C brightness assertions; complete existing range, live output, dirty state and shared save action.\n",count);return 0;
}
'''
(directory/'check.c').write_text(code,encoding='utf-8');exe=directory/'check.exe'
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(directory/'check.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
ui=source[source.index('lv_res_t create_win_nyx_options'):source.index('void create_tab_boot_options')]
assert ui.index('_settings_card(system, 2, 2)')<ui.index('_settings_card(system, 3, 2)')
assert 'lv_bar_set_range(brightness, 30, 220)'in ui and '_slider_brightness_action'in ui
assert '_slider_brightness_action'not in (work/'hekate-6.5.4/nyx/nyx_gui/frontend/gui_nova.c').read_text(encoding='utf-8')
