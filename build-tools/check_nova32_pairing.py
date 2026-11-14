"""Exercise real BDK SPI parsing and GUI import, with synthetic keys only.
The expected wire bytes follow Switchroot's exported LTK -> BlueZ -> HCI path.
Also compile the previous import to demonstrate that this fixture rejects it.
"""
from pathlib import Path
import subprocess, json, hashlib, re
w=Path(__file__).resolve().parent
p=w/'hekate-nova-pro-v32';d=w/'nova32-tests';d.mkdir(exist_ok=True)
bdk=(p/'bdk/input/joycon.c').read_text()
gui=(p/'nyx/nyx_gui/frontend/gui_bluetooth.c').read_text()
def function(text,start,end):return text[text.index(start):text.index(end,text.index(start))]
types=bdk[bdk.index('typedef struct _jc_hid_in_rpt_t'):bdk.index('typedef struct _jc_sio_out_rpt_t')]
parser=function(bdk,'static void _jc_parse_wired_hid(', 'static void _jc_parse_wired_init(')
constants='\n'.join(line for line in bdk.splitlines() if re.match(r'#define (JC_ID_[LR]|JC_ID_HORI|JC_HORI_INPUT_RPT |JC_HID_INPUT_RPT |JC_HID_SUBMCD_RPT |JC_HID_SUBCMD_SPI_READ |JC_HID_SUBCMD_CHARGE_SET )',line))
prefix=r'''
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <input/joycon.h>
#include "nova_bt.h"
typedef struct {u8 type,mac[6];int connected,charger_req;} jc_dev_t;
static jc_gamepad_rpt_t jc_gamepad;
static jc_dev_t jc_l,jc_r;
'''+constants+'\n'+types+r'''
static void _jc_parse_input(jc_dev_t *d,const jc_hid_in_rpt_t *p){(void)d;(void)p;}
'''+parser+r'''
typedef int lv_res_t;
typedef void lv_obj_t;
#define LV_RES_OK 0
#define FUSE_NX_HW_TYPE_HOAG 4
static struct {int jc_disable;} n_cfg;
static struct {char note[512];} ui;
static nova_bt_peer paired[2];
static struct {u8 attempted,rail[2],record[2],hos[2],compatible,recognized[2],source;} import_info;
static unsigned hw_type, hos_mask=3, recognized_mask=3;
static int fuse_read_hw_type(void){return hw_type;}
static void gui_bluetooth_shutdown(void){}
static void refresh(void){}
static const char *nyx_tr(const char *t){return t;}
#define s_printf sprintf
jc_gamepad_rpt_t *jc_get_bt_pairing_info(bool *l,bool *r){*l=hos_mask&1;*r=hos_mask&2;return &jc_gamepad;}
jc_gamepad_rpt_t *jc_get_bt_pairing_info_ex(bool *l,bool *r,bool *vl,bool *vr){*vl=recognized_mask&1;*vr=recognized_mask&2;return jc_get_bt_pairing_info(l,r);}
'''
suffix=r'''
/* Explicit raw-SPI fixture and independent exported/BlueZ/HCI golden bytes.
 * These are invented values, never read from a console or a pairing dump. */
static const u8 raw_keys[2][16]={
 {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
 {32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47}};
static const u8 hci_keys[2][16]={
 {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
 {47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32}};
void nova_test_import(nova_bt_peer out[2]){
 memset(&jc_gamepad,0,sizeof jc_gamepad);jc_gamepad.conn_l=jc_gamepad.conn_r=1;hw_type=0;n_cfg.jc_disable=0;hos_mask=recognized_mask=3;
 for(unsigned side=0;side<2;side++){
  u8 packet[128]={0};jc_hid_in_rpt_t *hid=(void*)packet;
  hid->cmd=JC_HID_SUBMCD_RPT;hid->subcmd=JC_HID_SUBCMD_SPI_READ;
  jc_hid_in_spi_read_t *spi=(void*)hid->subcmd_data;spi->size=0x1a;spi->addr=0x2000;
  jc_hid_in_pair_data_t *data=(void*)spi->data;data->magic=0x95;data->size=0x22;
  data->mac[0]=0x60;data->mac[5]=3;memcpy(data->ltk,raw_keys[side],16);
  jc_dev_t dev={0};dev.type=side?JC_ID_R:JC_ID_L;dev.mac[0]=0x40;dev.mac[5]=side+1;
  _jc_parse_wired_hid(&dev,packet,sizeof packet);
 }
 import_pairing(NULL);memcpy(out,paired,sizeof paired);
}
#ifdef FIXTURE_MAIN
int main(void){
 nova_bt_peer out[2];nova_test_import(out);assert(nova_bt_validate_peers(out)==2);
 for(unsigned i=0;i<2;i++){
  if(memcmp(out[i].key,hci_keys[i],16)){fputs("FAIL: imported key byte order\n",stderr);return 42;}
  assert(out[i].mac[0]==0x40&&out[i].mac[5]==i+1&&out[i].host[0]==0x60&&out[i].host[5]==3);
 }
 hos_mask=recognized_mask=1;import_pairing(NULL);assert(paired[0].valid&&!paired[1].valid);
 hos_mask=recognized_mask=2;import_pairing(NULL);assert(!paired[0].valid&&paired[1].valid);
 hos_mask=0;recognized_mask=3;import_pairing(NULL);assert(paired[0].valid&&paired[1].valid);
 recognized_mask=0;import_pairing(NULL);assert(!paired[0].valid&&!paired[1].valid);
 assert(import_info.attempted&&import_info.rail[0]&&import_info.rail[1]&&import_info.record[0]&&import_info.record[1]);
 assert(!import_info.hos[0]&&!import_info.hos[1]&&strstr(ui.note,"Pairing checksum or host profile not recognized"));
 hos_mask=recognized_mask=3;jc_gamepad.bt_conn_r.host_mac[5]^=1;import_pairing(NULL);
 assert(!import_info.compatible&&!paired[0].valid&&!paired[1].valid&&strstr(ui.note,"compatibility checks"));jc_gamepad.bt_conn_r.host_mac[5]^=1;
 memset(jc_gamepad.bt_conn_l.ltk,0,16);import_pairing(NULL);assert(!import_info.record[0]&&strstr(ui.note,"could not be read"));
 jc_gamepad.conn_l=jc_gamepad.conn_r=0;import_pairing(NULL);assert(strstr(ui.note,"Rail connection not ready"));
 hos_mask=3;hw_type=FUSE_NX_HW_TYPE_HOAG;import_pairing(NULL);assert(!paired[0].valid&&!paired[1].valid);
 hw_type=0;n_cfg.jc_disable=1;import_pairing(NULL);assert(!paired[0].valid&&!paired[1].valid);
 puts("PASS: real SPI parser -> real GUI import matches independent HCI key bytes; one/two/no pairing and rail guards");return 0;
}
#endif
'''
cc=w/'toolchains/w64devkit/bin/gcc.exe'
args=[str(cc),'-std=gnu11','-O1','-g','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth')]
outputs=[]
for old in (False,True):
 text=(w/'hekate-nova-pro-v23/nyx/nyx_gui/frontend/gui_bluetooth.c').read_text() if old else gui
 body=function(text,'static lv_res_t import_pairing(', 'static lv_res_t start(')
 f=d/('pairing-old.c' if old else 'pairing.c');f.write_text(prefix+body+suffix)
 exe=f.with_suffix('.exe')
 r=subprocess.run([*args,'-DFIXTURE_MAIN',str(f),str(p/'nyx/nyx_gui/bluetooth/nova_bt_protocol.c'),'-o',str(exe)],capture_output=True,text=True)
 assert r.returncode==0,r.stderr
 r=subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)
 if old:assert r.returncode!=0,'Old second-reversal error was not detected'
 else:assert r.returncode==0,r.stderr;print(r.stdout);outputs.append(r.stdout.strip())
r=subprocess.run([*args,'-c',str(d/'pairing.c'),'-o',str(d/'pairing.o')],capture_output=True,text=True)
assert r.returncode==0,r.stderr
(w/'nova32-pairing-check.json').write_text(json.dumps({'passed':True,'old_double_reversal_rejected':True,'synthetic_keys_only':True,'physical_rail_io':False,'logs':outputs,
 'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['bdk/input/joycon.c','bdk/input/joycon.h','nyx/nyx_gui/frontend/gui_bluetooth.c'] }},indent=2))
