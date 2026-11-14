"""Native tests of the real BTstack port, transport and Joy-Con decoder.
The radio is substituted. This verifies software, not a physical RF link.
"""
from pathlib import Path
import subprocess, json, hashlib
w=Path(__file__).resolve().parent
p=w/'hekate-nova-pro-v26/nyx/nyx_gui'
d=w/'nova26-tests';d.mkdir(exist_ok=True)
test=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "nova_bt.h"
static unsigned now,opened,closed,blocked,radio_error;
static unsigned in_n,in_at,out_n;
static uint8_t in[8192],out[8192];
uint32_t nova_bt_now(void){return now;}
int nova_bt_radio_open(void){opened++;return 1;}
void nova_bt_radio_close(void){closed++;}
int nova_bt_radio_read(void){if(radio_error)return -2;return in_at<in_n?in[in_at++]:-1;}
int nova_bt_radio_write(uint8_t v){if(blocked)return 0;assert(out_n<sizeof out);out[out_n++]=v;return 1;}
#include "nova_bt_stack.c"
static unsigned delivered,got_type,got_size;static uint8_t got[1030];
static void capture(uint8_t type,uint8_t *p,uint16_t n){delivered++;got_type=type;got_size=n;memcpy(got,p,n);}
static void parser_reset(void){rx_type=rx_len=rx_need=failure=delivered=0;receive_packet=capture;}
static void feed(const uint8_t *b,unsigned n){for(unsigned i=0;i<n;i++)input_byte(b[i]);}
static void reply(uint16_t opcode,const uint8_t *data,unsigned len)
{
    assert(in_at==in_n);in_at=in_n=0;
    in[in_n++]=4;in[in_n++]=0x0e;in[in_n++]=4+len;in[in_n++]=1;
    in[in_n++]=opcode&255;in[in_n++]=opcode>>8;in[in_n++]=0;
    memcpy(in+in_n,data,len);in_n+=len;
}
int main(void)
{
    nova_bt_peer pair[2]={0};assert(!nova_bt_validate_peers(pair));
    for(unsigned i=0;i<2;i++){
        pair[i].valid=1;pair[i].mac[0]=0x40;pair[i].mac[5]=i+1;
        pair[i].host[0]=0x60;pair[i].host[5]=3;memset(pair[i].key,0x70+i,16);
    }
    assert(nova_bt_validate_peers(pair)==2);pair[1].host[5]++;
    assert(!nova_bt_validate_peers(pair));pair[1].host[5]--;
    pair[1].mac[5]=1;assert(!nova_bt_validate_peers(pair));pair[1].mac[5]=2;
    uint8_t report[64]={0xa1,0x30,0,0x80,0xff,0xff,0xff,0x34,0x12,0x56,0x78,0x9a,0xbc,0};
    nova_bt_pad pad={0};
    for(unsigned n=0;n<14;n++)assert(!nova_bt_decode(&pad,0,report,n,123));
    assert(nova_bt_decode(&pad,0,report,14,123)&&pad.buttons==0xff2900&&pad.updated==123);
    assert(pad.x==0x234&&pad.y==0x561&&pad.battery==8);
    assert(nova_bt_decode(&pad,1,report,14,124)&&pad.buttons==0x0016ff);
    assert(pad.x==0xa78&&pad.y==0xbc9);
    report[1]=0x3f;assert(!nova_bt_decode(&pad,1,report,14,999)&&pad.updated==124);
    assert(!nova_bt_decode(&pad,2,report,14,999));
    uint8_t simple[12]={0x3f,1,0,8};nova_bt_pad sp={0};
    assert(!nova_bt_decode(&sp,0,simple,11,1));
    assert(nova_bt_decode(&sp,0,simple,12,2)&&sp.buttons==(1u<<19)&&sp.x==2048&&sp.y==2048);
    assert(nova_bt_decode(&sp,1,simple,12,3)&&sp.buttons==8&&sp.report_id==0x3f&&sp.reports==2);
    simple[3]=0;assert(nova_bt_decode(&sp,0,simple,12,4)&&sp.x==3072&&sp.y==2048);
    assert(nova_bt_decode(&sp,1,simple,12,5)&&sp.x==1024&&sp.y==2048);
    simple[3]=9;assert(!nova_bt_decode(&sp,1,simple,12,6)&&sp.updated==5);
    uint8_t ack[16]={0x21};ack[13]=0x80;ack[14]=3;ack[15]=0x30;
    assert(nova_bt_decode(&sp,1,ack,16,7)&&sp.mode_ack);
    /* Split frames, zero payload events, vendor diagnostics containing 0x04,
     * maximum ACL length, and an oversized ACL rejected before copying. */
    const uint8_t event_bytes[]={4,0x0e,4,1,3,0x0c,0};
    for(unsigned split=0;split<sizeof event_bytes;split++){
        parser_reset();feed(event_bytes,split);assert(!delivered);feed(event_bytes+split,sizeof event_bytes-split);
        assert(delivered==1&&got_type==4&&got_size==6&&!failure);
    }
    parser_reset();const uint8_t zero[]={0,0x31,0x34,4,0x10,0};feed(zero,sizeof zero);assert(delivered==1&&got_size==2);
    parser_reset();input_byte(7);for(unsigned n=0;n<63;n++)input_byte(4);assert(!delivered&&!rx_type);feed(event_bytes,sizeof event_bytes);assert(delivered==1);
    parser_reset();const uint8_t acl[]={2,1,0x20,0,4};feed(acl,sizeof acl);for(unsigned n=0;n<1024;n++)input_byte(n);assert(delivered==1&&got_size==1028&&!failure);
    parser_reset();const uint8_t too_large[]={2,1,0x20,0xff,0xff};feed(too_large,sizeof too_large);assert(failure==5&&!delivered);
    parser_reset();input_byte(0x81);assert(failure==5);
    /* Trace records contain protocol metadata, never link-key arguments. */
    nova_bt_trace_reset();uint8_t private_command[25]={0x0b,4,22};memset(private_command+3,0xa5,22);
    trace_frame(1,HCI_COMMAND_DATA_PACKET,private_command,sizeof private_command);
    nova_bt_trace tr;assert(nova_bt_trace_count()==1&&nova_bt_trace_get(0,&tr));
    assert(tr.kind==1&&tr.a==0x040b&&tr.b==22&&tr.c==0);
    nova_bt_trace_reset();for(unsigned n=0;n<160;n++)nova_bt_trace_add(99,n,0,0);
    assert(nova_bt_trace_count()==128&&nova_bt_trace_get(0,&tr)&&tr.a==32);
    assert(nova_bt_trace_get(127,&tr)&&tr.a==159&&!nova_bt_trace_get(128,&tr));
    /* Real stack initialization through the asynchronous port. The virtual
     * controller sends standard Command Complete responses only. */
    failure=0;unsigned boot_commands=0;
    for(unsigned round=0;round<3;round++){
        in_at=in_n=out_n=0;now=100;blocked=radio_error=0;
        assert(nova_bt_start(pair));
        for(unsigned step=0;step<20000&&status.state==NOVA_BT_STARTING;step++){
            nova_bt_poll();now++;
            if(out_n>=4&&out[0]==1&&out_n==(unsigned)out[3]+4){
                uint16_t op=out[1]|(out[2]<<8);uint8_t data[248]={0};unsigned n=0;
                switch(op){
                case 0x1001:n=8;data[0]=6;data[3]=6;data[4]=15;break;
                case 0x1002:n=64;memset(data,0xff,n);break;
                case 0x1003:n=8;data[0]=0xff;data[1]=0xff;data[2]=0xff;data[3]=0xfe;break;
                case 0x1004:n=10;data[0]=out[4];data[1]=0;break;
                case 0x1005:n=7;data[0]=0x00;data[1]=4;data[3]=8;break;
                case 0x1009:n=6;for(unsigned j=0;j<6;j++)data[j]=pair[0].host[5-j];break;
                case 0x0c14:n=248;break;
                default:break;
                }
                /* Local name is not queried by this port; avoid an accidental
                 * out-of-bounds emulator response if upstream changes. */
                assert(n<=sizeof(data));reply(op,data,n);out_n=0;boot_commands++;
            }
        }
        assert(status.state==NOVA_BT_READY&&boot_commands>5);assert(status.controller_hci_version==6&&status.controller_manufacturer==15);
        link_key_type_t key_type;assert(key_get(pair[0].mac,got,&key_type));
        assert(!memcmp(got,pair[0].key,16));
        bd_addr_t unknown={0x40,0,0,0,0,99};
        assert(!known_connection(unknown,HCI_LINK_TYPE_ACL));
        assert(known_connection(pair[0].mac,HCI_LINK_TYPE_ACL));
        assert(!known_connection(pair[0].mac,HCI_LINK_TYPE_SCO));
        uint8_t opened_event[15]={HCI_EVENT_HID_META,13,HID_SUBEVENT_CONNECTION_OPENED,9,0,0};
        reverse_bd_addr(pair[0].mac,opened_event+6);event(HCI_EVENT_PACKET,0,opened_event,15);
        uint8_t report_event[21]={HCI_EVENT_HID_META,19,HID_SUBEVENT_REPORT,9,0,14,0};
        report[1]=0x30;memcpy(report_event+7,report,14);event(HCI_EVENT_PACKET,0,report_event,sizeof report_event);
        assert(status.pad[0].connected&&status.pad[0].buttons==0xff2900);
        uint8_t closed_event[]={HCI_EVENT_HID_META,3,HID_SUBEVENT_CONNECTION_CLOSED,9,0};
        event(HCI_EVENT_PACKET,0,closed_event,sizeof closed_event);
        assert(!status.pad[0].connected&&!status.pad[0].buttons&&!link[0].cid);
        /* Drop stale presses and disconnect without leaving any held input. */
        status.pad[0].connected=1;status.pad[0].buttons=8;status.pad[0].updated=now-501;
        nova_bt_poll();assert(!status.pad[0].connected&&!status.pad[0].buttons);
        nova_bt_stop();assert(status.state==NOVA_BT_OFF&&!status.pad[0].connected);
        assert(!initialized&&!tx_len&&!rx_type&&!peers[0].valid);
    }
    /* Hardware no-response and backpressure have deadlines; they return to
     * Nyx with an error instead of looping forever. */
    in_at=in_n=out_n=0;now=0;assert(nova_bt_start(pair));blocked=1;now=2001;nova_bt_poll();assert(status.state==NOVA_BT_ERROR&&status.error==6);
    blocked=0;in_at=in_n=out_n=0;now=0;assert(nova_bt_start(pair));radio_error=1;nova_bt_poll();assert(status.state==NOVA_BT_ERROR&&status.error==4);radio_error=0;
    in_at=in_n=out_n=0;now=0;assert(nova_bt_start(pair));now=15001;nova_bt_poll();assert(status.state==NOVA_BT_ERROR);
    nova_bt_stop();assert(opened==closed);
    printf("PASS real BTstack initialization (%u HCI commands), framing, keys, input, restart and deadlines\n",boot_commands);
}
'''
(d/'test.c').write_text(test)
cc=w/'toolchains/w64devkit/bin/gcc.exe'
lib=p/'libs/btstack'
sources=[str(f) for f in lib.rglob('*.c')]
args=[str(cc),'-std=gnu11','-O1','-g','-Wall','-Wextra','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
      '-I'+str(p/'bluetooth'),'-I'+str(lib),str(d/'test.c'),str(p/'bluetooth/nova_bt_protocol.c'),str(p/'bluetooth/nova_bt_trace.c'),*sources,'-o',str(d/'test.exe')]
r=subprocess.run(args,capture_output=True,text=True)
if r.returncode:raise RuntimeError(r.stderr[-12000:])
r=subprocess.run([str(d/'test.exe')],capture_output=True,text=True,timeout=20)
print(r.stdout,r.stderr)
assert r.returncode==0,r.returncode
files=list((p/'bluetooth').glob('*.[ch]'))
(w/'nova26-bt-check.json').write_text(json.dumps({'passed':True,'physical_radio':False,'output':r.stdout,
    'source_sha256':{f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in files}},indent=2))
