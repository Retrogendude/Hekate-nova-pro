"""Actual BDK record scanner -> SPI parser -> GUI import, synthetic rails only."""
from pathlib import Path
import subprocess,json,hashlib,re
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32';d=w/'nova32-tests';d.mkdir(exist_ok=True)
bdk=(p/'bdk/input/joycon.c').read_text();gui=(p/'nyx/nyx_gui/frontend/gui_bluetooth.c').read_text()
def section(s,a,b):return s[s.index(a):s.index(b,s.index(a))]
types=section(bdk,'typedef struct _jc_hid_in_rpt_t','typedef struct _jc_sio_out_rpt_t')
const='\n'.join(x for x in bdk.splitlines() if re.match(r'#define (JC_ID_[LR]|JC_ID_HORI|JC_HORI_INPUT_RPT |JC_HID_INPUT_RPT |JC_HID_SUBMCD_RPT |JC_HID_SUBCMD_SPI_READ |JC_HID_SUBCMD_CHARGE_SET )',x))
prefix=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <input/joycon.h>
#include "nova_bt.h"
#define assert(x) do{if(!(x)){fprintf(stderr,"FAIL line%d %s\n",__LINE__,#x);exit(1);}}while(0)
#define SPI_READ_OFFSET 0x20
#define UART_AO_TX_HW_RX 1
#define UART_AO_TX_MN_RX 2
#define LV_RES_OK 0
#define FUSE_NX_HW_TYPE_HOAG 4
typedef int lv_res_t;typedef void lv_obj_t;
typedef struct{u8 type,mac[6],buf[256];int connected,charger_req;u32 uart,last_status_req_time;}jc_dev_t;
static jc_dev_t jc_l,jc_r;static jc_gamepad_rpt_t jc_gamepad;static bool jc_init_done;
static u32 time_ms;static u32 get_tmr_ms(void){return time_ms++;}
static void msleep(u32 n){time_ms+=n;}
static void uart_set_mode(u32 uart,u32 mode){(void)uart;(void)mode;}
jc_gamepad_rpt_t *joycon_poll(void){return &jc_gamepad;}
static void _jc_parse_input(jc_dev_t*d,const void*p){(void)d;(void)p;}
'''+const+'\n'+types+'\n'+section(bdk,'static void _jc_parse_wired_hid(','static void _jc_parse_wired_init(')+r'''
static u8 caps[2],corrupt[2],inactive[2],truncated[2],missing[2],mismatch;
static u32 queued_addr[2];static bool queued[2];static unsigned requests[2];
static void _jc_send_hid_cmd(jc_dev_t*d,u8 cmd,u8*data,u32 n){
 unsigned i=d->uart;assert(i<2&&cmd==0x10&&n==5);jc_hid_out_spi_read_t*s=(void*)data;
 assert(s->size==0x1a&&s->addr>=0x2000&&s->addr<0x2100);queued_addr[i]=s->addr;queued[i]=true;requests[i]++;
}
static void _jc_rcv_pkt(jc_dev_t*d){
 unsigned i=d->uart;if(!queued[i])return;queued[i]=false;if(missing[i])return;
 memset(d->buf,0,sizeof d->buf);
 unsigned offset=SPI_READ_OFFSET-sizeof(jc_hid_in_rpt_t)-sizeof(jc_hid_in_spi_read_t);
 jc_hid_in_rpt_t *h=(void*)(d->buf+offset);h->cmd=0x21;h->subcmd=0x10;h->submcd_ack=0x90;
 jc_hid_in_spi_read_t*s=(void*)h->subcmd_data;s->addr=queued_addr[i];s->size=0x1a;
 jc_hid_in_pair_data_t full={0};full.magic=inactive[i]&&s->addr==0x2000?0:0x95;full.size=0x22;
 full.mac[0]=0x60;full.mac[5]=mismatch?i:3;for(unsigned k=0;k<16;k++)full.ltk[k]=1+k+i*32;
 full.bt_caps=caps[i];u8 crc=0;for(unsigned k=0;k<0x22;k++)crc+=((u8*)&full)[4+k];
 full.checksum=(crc^0x55)^corrupt[i];memcpy(s->data,&full,0x1a);
 unsigned length=sizeof(*h)+sizeof(*s)+0x1a;
 if(truncated[i]){memset(s->data+20,0,6);length-=6;}
 _jc_parse_wired_hid(d,(u8*)h,length);
}
'''+section(bdk,'static bool _jc_validate_pairing_info(','static void _jc_conn_init(')+r'''
static struct{int jc_disable;}n_cfg;static struct{char note[512];}ui;static nova_bt_peer paired[2];
'''+next(x for x in gui.splitlines() if x.startswith('static struct {u8 attempted'))+r'''
static unsigned fuse_read_hw_type(void){return 0;}
static void gui_bluetooth_shutdown(void){}
static void refresh(void){}
static const char*nyx_tr(const char*s){return s;}
#define s_printf sprintf
'''
suffix=r'''
static void reset(void){
 memset(&jc_gamepad,0,sizeof jc_gamepad);memset(&jc_l,0,sizeof jc_l);memset(&jc_r,0,sizeof jc_r);
 jc_l.connected=jc_r.connected=jc_gamepad.conn_l=jc_gamepad.conn_r=jc_init_done=true;
 jc_l.type=1;jc_r.type=2;jc_l.uart=0;jc_r.uart=1;jc_l.mac[0]=jc_r.mac[0]=0x40;jc_l.mac[5]=1;jc_r.mac[5]=2;
 memset(corrupt,0,2);memset(inactive,0,2);memset(truncated,0,2);memset(missing,0,2);memset(queued,0,sizeof queued);memset(requests,0,sizeof requests);time_ms=0;mismatch=0;caps[0]=caps[1]=0x68;
}
int main(void){
 reset();caps[0]=caps[1]=8;import_pairing(NULL);
 assert(nova_bt_validate_peers(paired)==2);assert(!import_info.hos[0]&&!import_info.hos[1]);assert(import_info.recognized[0]&&import_info.recognized[1]);
 for(unsigned k=0;k<16;k++)assert(paired[0].key[k]==16-k);
 memset(paired,0,sizeof paired);import_pairing(NULL);assert(nova_bt_validate_peers(paired)==2);
 reset();import_pairing(NULL);assert(nova_bt_validate_peers(paired)==2&&import_info.hos[0]&&import_info.hos[1]);
 reset();caps[1]=8;import_pairing(NULL);assert(nova_bt_validate_peers(paired)==2&&import_info.hos[0]&&!import_info.hos[1]);
 reset();inactive[0]=inactive[1]=1;import_pairing(NULL);assert(nova_bt_validate_peers(paired)==2&&requests[0]>=2&&requests[1]>=2);
 reset();corrupt[1]=1;import_pairing(NULL);assert(paired[0].valid&&!paired[1].valid&&!import_info.recognized[1]);
 reset();corrupt[0]=corrupt[1]=1;import_pairing(NULL);assert(!nova_bt_validate_peers(paired)&&!import_info.recognized[0]&&!import_info.recognized[1]);assert(time_ms<4000);
 reset();caps[0]=caps[1]=0x18;import_pairing(NULL);assert(!nova_bt_validate_peers(paired));
 reset();truncated[0]=truncated[1]=1;import_pairing(NULL);assert(!nova_bt_validate_peers(paired));
 reset();jc_l.connected=jc_gamepad.conn_l=false;import_pairing(NULL);assert(!paired[0].valid&&paired[1].valid);
 reset();mismatch=1;import_pairing(NULL);assert(!nova_bt_validate_peers(paired)&&!import_info.compatible);
 reset();missing[0]=missing[1]=1;import_pairing(NULL);assert(!nova_bt_validate_peers(paired));
 reset();jc_init_done=false;bool l=true,r=true,vl=true,vr=true;assert(!jc_get_bt_pairing_info_ex(&l,&r,&vl,&vr)&&!l&&!r&&!vl&&!vr);
 reset();caps[0]=caps[1]=8;l=r=true;assert(jc_get_bt_pairing_info(&l,&r)&&!l&&!r);
 puts("PASS: actual BDK scanner/SPI/GUI: Nintendo, standard, mixed, repeat import, inactive slots, corrupt/truncated/missing records, disconnected rail, mismatched host and legacy API");
}
'''
cc=w/'toolchains/w64devkit/bin/gcc.exe';args=[str(cc),'-std=gnu11','-O1','-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui/bluetooth')]
logs=[]
for old in (False,True):
 text=(w/'hekate-nova-pro-v28/nyx/nyx_gui/frontend/gui_bluetooth.c').read_text() if old else gui
 f=d/('reader-old.c' if old else 'reader.c');f.write_text(prefix+section(text,'static lv_res_t import_pairing(','static lv_res_t start(')+suffix)
 r=subprocess.run([*args,str(f),str(p/'nyx/nyx_gui/bluetooth/nova_bt_protocol.c'),'-o',str(f.with_suffix('.exe'))],capture_output=True,text=True);assert not r.returncode,r.stderr
 r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True,timeout=10)
 if old:assert r.returncode==1 and 'validate_peers(paired)==2' in r.stderr,r.stderr
 else:assert not r.returncode,r.stderr;print(r.stdout);logs.append(r.stdout)
(w/'nova32-reader-check.json').write_text(json.dumps({'passed':True,'old_v28_standard_profile_rejected':True,'physical_rails':False,'logs':logs,'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['bdk/input/joycon.c','bdk/input/joycon.h','nyx/nyx_gui/frontend/gui_bluetooth.c']}},indent=2))
