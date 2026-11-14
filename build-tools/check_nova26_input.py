"""Test actual UI input merge: rail priority, source immutability and stale data."""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v26';d=w/'nova26-tests'
f=p/'nyx/nyx_gui/frontend/gui_bluetooth.c';s=f.read_text();s=s[s.index('jc_gamepad_rpt_t *gui_bluetooth_merge('):]
test=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <input/joycon.h>
#include "nova_bt.h"
static nova_bt_status fake;static u32 now=1000;
static u32 get_tmr_ms(void){return now;}
const nova_bt_status *nova_bt_get_status(void){return &fake;}
'''+s+r'''
int main(void){
 jc_gamepad_rpt_t wired={0},before;
 assert(gui_bluetooth_merge(&wired)==&wired);assert(!gui_bluetooth_merge(NULL));
 fake.pad[0]=(nova_bt_pad){.buttons=0x010000,.updated=now,.x=2100,.y=1900,.connected=1,.battery=8};
 fake.pad[1]=(nova_bt_pad){.buttons=8,.updated=now,.x=2000,.y=2001,.connected=1,.battery=6};
 jc_gamepad_rpt_t *m=gui_bluetooth_merge(NULL);
 assert(m->conn_l&&m->conn_r&&m->buttons==0x10008&&m->lstick_x==2100&&m->rstick_y==2001);
 wired.conn_l=wired.conn_r=1;wired.buttons=0x20004;wired.lstick_x=2300;wired.rstick_y=2200;before=wired;
 m=gui_bluetooth_merge(&wired);assert(m->buttons==wired.buttons&&m->lstick_x==2300&&m->rstick_y==2200);
 assert(!memcmp(&wired,&before,sizeof wired));
 wired.conn_l=0;before=wired;m=gui_bluetooth_merge(&wired);
 assert(m->conn_l&&m->conn_r&&m->buttons==0x10004&&m->lstick_x==2100&&m->rstick_y==2200);
 assert(!memcmp(&wired,&before,sizeof wired));
 now+=501;m=gui_bluetooth_merge(NULL);assert(!m->conn_l&&!m->conn_r&&!m->buttons&&m->lstick_x==2048);
 fake.pad[0].connected=fake.pad[1].connected=0;assert(!gui_bluetooth_merge(NULL));
 puts("PASS actual input merge: two wireless Joy-Con, wired priority, unchanged source, stale release and neutral sticks");
}
'''
(d/'input.c').write_text(test)
cmd=[str(w/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth'),str(d/'input.c'),'-o',str(d/'input.exe')]
r=subprocess.run(cmd,capture_output=True,text=True);assert r.returncode==0,r.stderr
r=subprocess.run([str(d/'input.exe')],capture_output=True,text=True,timeout=10);print(r.stdout,r.stderr);assert r.returncode==0
(w/'nova26-input-check.json').write_text(json.dumps({'passed':True,'output':r.stdout,'source_sha256':hashlib.sha256(f.read_bytes()).hexdigest()},indent=2))
