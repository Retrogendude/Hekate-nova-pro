"""Exercise the real shared save action: both files, failures and dirty state."""
from pathlib import Path
import re
import subprocess

work=Path(__file__).resolve().parent
source=(work/'hekate-6.5.4/nyx/nyx_gui/frontend/gui_options.c').read_text(encoding='utf-8')
def function(name):
    m=re.search(r'^(?:bool|lv_res_t) '+name+r'\([^;]+?\)\s*\{',source,re.M);assert m,name
    depth=1;end=m.end()
    while depth:
        if source[end]=='{':depth+=1
        if source[end]=='}':depth-=1
        end+=1
    return source[m.start():end]
stub=r'''
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
typedef struct{int unused;}lv_obj_t;typedef int lv_res_t;
#define LV_RES_OK 0
#define LV_OPA_TRANSP 0
static struct{lv_obj_t*mid;}status_bar;
static bool ini_changes_made,nyx_changes_made,mounted;
static int mount_error,boot_error,nyx_error,mounts,unmounts,boot_writes,nyx_writes;
static char message[256];
static int sd_mount(void){mounts++;if(mount_error)return 1;mounted=true;return 0;}
static void sd_unmount(void){unmounts++;mounted=false;}
static int create_config_entry(void){assert(mounted);boot_writes++;return boot_error;}
static int create_nyx_config_entry(bool force){assert(mounted&&force);nyx_writes++;sd_unmount();return nyx_error;}
static void nyx_show_help(const char*m){strcpy(message,m);}
static void lv_obj_set_opa_scale(lv_obj_t*b,int opacity){(void)b;(void)opacity;}
static void lv_obj_set_click(lv_obj_t*b,bool click){(void)b;(void)click;}
'''
test=r'''
static unsigned checks;
#define CHECK(c)do{checks++;assert(c);}while(0)
static void reset(void){ini_changes_made=nyx_changes_made=true;mounted=false;mount_error=boot_error=nyx_error=mounts=unmounts=boot_writes=nyx_writes=0;message[0]=0;}
int main(void){
 reset();CHECK(nyx_options_has_changes());nyx_options_save_all(NULL);CHECK(boot_writes==1&&nyx_writes==1);CHECK(!nyx_options_has_changes());CHECK(!mounted&&unmounts==1);CHECK(!strcmp(message,"Settings saved."));
 reset();mount_error=1;nyx_options_save_all(NULL);CHECK(!boot_writes&&!nyx_writes);CHECK(nyx_options_has_changes());CHECK(!mounted);CHECK(strstr(message,"unavailable"));
 reset();boot_error=1;nyx_options_save_all(NULL);CHECK(boot_writes==1&&!nyx_writes);CHECK(ini_changes_made&&nyx_changes_made);CHECK(!mounted&&unmounts==1);CHECK(strstr(message,"Could not save"));
 reset();nyx_error=1;nyx_options_save_all(NULL);CHECK(boot_writes==1&&nyx_writes==1);CHECK(!ini_changes_made&&nyx_changes_made);CHECK(!mounted);CHECK(strstr(message,"Boot options were saved"));
 nyx_error=0;nyx_options_save_all(NULL);CHECK(boot_writes==2&&nyx_writes==2);CHECK(!nyx_options_has_changes());CHECK(!mounted);
 reset();ini_changes_made=nyx_changes_made=false;nyx_options_save_all(NULL);CHECK(boot_writes==1&&nyx_writes==1);CHECK(!nyx_options_has_changes());
 reset();ini_changes_made=false;CHECK(nyx_options_commit_boot());CHECK(!boot_writes);CHECK(nyx_changes_made);
 reset();mounted=true;boot_error=1;CHECK(!nyx_options_commit_boot());CHECK(ini_changes_made);CHECK(boot_writes==1&&mounted);
 boot_error=0;CHECK(nyx_options_commit_boot());CHECK(!ini_changes_made);CHECK(boot_writes==2&&mounted);
 printf("PASS: %u actual-C Nova save assertions; mount/boot/Nyx failures, partial-save reporting, retry, dirty flags and shared Appearance commit.\n",checks);return 0;
}
'''
directory=work/'nova-save-check';directory.mkdir(exist_ok=True)
path=directory/'test.c';path.write_text(stub+'\n'.join(function(n)for n in ['nyx_options_has_changes','nyx_options_commit_boot','nyx_options_save_all'])+test,encoding='utf-8')
exe=directory/'test.exe';subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11',str(path),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
