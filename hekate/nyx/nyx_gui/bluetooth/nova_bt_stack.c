/* Cooperative Classic HID host. BTstack stays isolated from BDK headers.
 * No background discovery, key writes, pairing changes or SD key files. */
#include "nova_bt.h"
#include "btstack_memory.h"
#include "btstack_run_loop.h"
#include "btstack_event.h"
#include "btstack_chipset.h"
#include "hci.h"
#include "hci_cmd.h"
#include "l2cap.h"
#include "classic/hid_host.h"
#include <string.h>

static nova_bt_status status;
static nova_bt_peer peers[2];
static int initialized, failure;
static uint32_t started, tx_started, rx_started;
static uint8_t tx[1030], rx[1030];
static unsigned tx_len, tx_pos, rx_len, rx_need, rx_header;
static uint8_t rx_type;
static void (*receive_packet)(uint8_t,uint8_t *,uint16_t);
static btstack_packet_callback_registration_t event_registration;
static uint8_t descriptors[1024];
static struct {uint16_t cid,handle;uint8_t retries, sequence, mode_tries, output[48];uint32_t next, mode_next;} link[2];
static int side_for(const uint8_t *addr);
static int side_handle(uint16_t handle)
{
    hci_connection_t *conn=hci_connection_for_handle(handle);
    if(conn)return side_for(conn->address);
    for(unsigned i=0;i<2;i++)if(link[i].handle==handle)return i;
    return -1;
}
static void trace_frame(int outgoing,uint8_t type,const uint8_t *p,unsigned size)
{
    if(type==HCI_COMMAND_DATA_PACKET&&outgoing&&size>=3){
        nova_bt_trace_add(1,little_endian_read_16(p,0),p[2],0);return;
    }
    if(type==HCI_EVENT_PACKET&&!outgoing&&size>=2){
        if(p[0]==HCI_EVENT_COMMAND_COMPLETE&&size>=6)nova_bt_trace_add(2,little_endian_read_16(p,3),p[5],0);
        else if(p[0]==HCI_EVENT_COMMAND_STATUS&&size>=6)nova_bt_trace_add(3,little_endian_read_16(p,4),p[2],0);
        else if(p[0]==HCI_EVENT_CONNECTION_COMPLETE||p[0]==HCI_EVENT_DISCONNECTION_COMPLETE||
                p[0]==HCI_EVENT_AUTHENTICATION_COMPLETE||p[0]==HCI_EVENT_ENCRYPTION_CHANGE||p[0]==HCI_EVENT_HARDWARE_ERROR)
            nova_bt_trace_add(4,p[0],p[0]==HCI_EVENT_DISCONNECTION_COMPLETE&&size>=6?p[5]:size>=3?p[2]:0,size>=5?little_endian_read_16(p,3):0);
        return;
    }
    if(type!=HCI_ACL_DATA_PACKET||size<8)return;
    if((little_endian_read_16(p,0)&0x3000)==0x1000)return; /* continuation has no L2CAP header */
    uint16_t handle=little_endian_read_16(p,0)&0xfff;
    if(little_endian_read_16(p,6)==1&&size>=12){
        uint8_t code=p[8];uint16_t value=size>=14?little_endian_read_16(p,12):0;
        if((code==3||code==5)&&size>=18)value=little_endian_read_16(p,16);
        nova_bt_trace_add(outgoing?5:6,handle,(code<<8)|p[9],value);
        if(outgoing&&code==2&&size>=16){int i=side_handle(handle);if(i>=0){status.pad[i].last_psm=value;status.pad[i].stage=value==0x11?4:value==0x13?5:2;}}
    }else if(outgoing&&size>=10)nova_bt_trace_add(7,handle,p[9],size);
}

static void timer_set(btstack_timer_source_t *t,uint32_t ms){t->timeout=nova_bt_now()+ms+1;}
static void noop(void){}
static const btstack_run_loop_t run_loop={
    .init=btstack_run_loop_base_init,
    .add_data_source=btstack_run_loop_base_add_data_source,
    .remove_data_source=btstack_run_loop_base_remove_data_source,
    .enable_data_source_callbacks=btstack_run_loop_base_enable_data_source_callbacks,
    .disable_data_source_callbacks=btstack_run_loop_base_disable_data_source_callbacks,
    .set_timer=timer_set,.add_timer=btstack_run_loop_base_add_timer,
    .remove_timer=btstack_run_loop_base_remove_timer,.execute=noop,
    .dump_timer=btstack_run_loop_base_dump_timer,.get_time_ms=nova_bt_now,
    .poll_data_sources_from_irq=noop,.execute_on_main_thread=btstack_run_loop_base_add_callback,
    .trigger_exit=noop
};
static void transport_init(const void *config){(void)config;}
static int transport_open(void){return 0;}
static int transport_close(void){tx_len=rx_len=rx_need=0;return 0;}
static void transport_register(void (*cb)(uint8_t,uint8_t *,uint16_t)){receive_packet=cb;}
static int transport_ready(uint8_t type){(void)type;return tx_len==0;}
static int transport_send(uint8_t type,uint8_t *packet,int size)
{
    if(tx_len||size<0||(unsigned)size+1>sizeof(tx))return -1;
    tx[0]=type;memcpy(tx+1,packet,size);tx_len=size+1;tx_pos=0;tx_started=nova_bt_now();return 0;
}
static const hci_transport_t transport={.name="Nova UART-D H4",.init=transport_init,
    .open=transport_open,.close=transport_close,.register_packet_handler=transport_register,
    .can_send_packet_now=transport_ready,.send_packet=transport_send};

/* Broadcom Write_BD_ADDR: documented BTstack bcm chipset command. RAM only.
 * ROM startup is experimental: no proprietary firmware is redistributed. */
static void bcm_address(bd_addr_t addr,uint8_t *cmd)
{cmd[0]=1;cmd[1]=0xfc;cmd[2]=6;reverse_bd_addr(addr,cmd+3);}
static const btstack_chipset_t chipset={.name="CYW4356 ROM",.set_bd_addr_command=bcm_address};
static int side_for(const uint8_t *addr)
{for(unsigned i=0;i<2;i++)if(peers[i].valid&&!memcmp(addr,peers[i].mac,6))return i;return -1;}
static int known_connection(bd_addr_t addr,hci_link_type_t type)
{return type==HCI_LINK_TYPE_ACL && side_for(addr)>=0;}
static int key_get(bd_addr_t addr,link_key_t key,link_key_type_t *type)
{int i=side_for(addr);if(i<0)return 0;memcpy(key,peers[i].key,16);*type=UNAUTHENTICATED_COMBINATION_KEY_GENERATED_FROM_P192;return 1;}
static void key_put(bd_addr_t addr,link_key_t key,link_key_type_t type){(void)addr;(void)key;(void)type;}
static void key_delete(bd_addr_t addr){(void)addr;}
static void key_local(bd_addr_t addr){(void)addr;}
static int key_iter(btstack_link_key_iterator_t *it){(void)it;return 0;}
static int key_next(btstack_link_key_iterator_t *it,bd_addr_t addr,link_key_t key,link_key_type_t *type)
{(void)it;(void)addr;(void)key;(void)type;return 0;}
static void key_done(btstack_link_key_iterator_t *it){(void)it;}
static const btstack_link_key_db_t keys={noop,key_local,noop,key_get,key_put,key_delete,key_iter,key_next,key_done};

static void clear_pad(unsigned i){status.pad[i].connected=0;status.pad[i].buttons=0;status.pad[i].x=status.pad[i].y=2048;}
static int side_cid(uint16_t cid)
{for(unsigned i=0;i<2;i++)if(cid&&link[i].cid==cid)return i;return -1;}
static void event(uint8_t type,uint16_t channel,uint8_t *p,uint16_t size)
{
    (void)channel;if(type!=HCI_EVENT_PACKET||size<2)return;
    bd_addr_t addr;int i;uint16_t cid;
    switch(hci_event_packet_get_type(p)){
    case HCI_EVENT_CONNECTION_COMPLETE:
        if(size<13)break;
        hci_event_connection_complete_get_bd_addr(p,addr);i=side_for(addr);if(i<0)break;
        status.pad[i].last_error=hci_event_connection_complete_get_status(p);
        if(status.pad[i].last_error)status.pad[i].stage=7;
        else{link[i].handle=hci_event_connection_complete_get_connection_handle(p);status.pad[i].stage=2;}
        break;
    case HCI_EVENT_AUTHENTICATION_COMPLETE:
    case HCI_EVENT_ENCRYPTION_CHANGE:
        if(size<5)break;
        i=side_handle(little_endian_read_16(p,3));if(i<0)break;
        status.pad[i].last_error=p[2];status.pad[i].stage=p[2]?7:3;break;
    case BTSTACK_EVENT_STATE:
        if(btstack_event_state_get_state(p)==HCI_STATE_WORKING){status.state=NOVA_BT_READY;}
        break;
    case HCI_EVENT_HARDWARE_ERROR: failure=8;break;
    case HCI_EVENT_PIN_CODE_REQUEST:
        hci_event_pin_code_request_get_bd_addr(p,addr);gap_pin_code_negative(addr);break;
    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
        hci_event_user_confirmation_request_get_bd_addr(p,addr);gap_ssp_confirmation_negative(addr);break;
    case HCI_EVENT_USER_PASSKEY_REQUEST:
        hci_event_user_passkey_request_get_bd_addr(p,addr);gap_ssp_passkey_negative(addr);break;
    case HCI_EVENT_HID_META:
        if(size<3)break;
        switch(hci_event_hid_meta_get_subevent_code(p)){
        case HID_SUBEVENT_INCOMING_CONNECTION:
            hid_subevent_incoming_connection_get_address(p,addr);
            cid=hid_subevent_incoming_connection_get_hid_cid(p);i=side_for(addr);
            if(i>=0 && !hid_subevent_incoming_connection_get_status(p)){
                link[i].cid=cid;hid_host_accept_connection(cid,HID_PROTOCOL_MODE_REPORT);
            }else hid_host_decline_connection(cid);
            break;
        case HID_SUBEVENT_CONNECTION_OPENED:
            hid_subevent_connection_opened_get_bd_addr(p,addr);i=side_for(addr);if(i<0)break;
            if(hid_subevent_connection_opened_get_status(p)){
                status.pad[i].last_error=hid_subevent_connection_opened_get_status(p);status.pad[i].hid_open=0;
                status.pad[i].stage=7;nova_bt_trace_add(10,i,status.pad[i].last_error,status.pad[i].last_psm);
                link[i].cid=0;link[i].next=nova_bt_now()+5000;clear_pad(i);
            }else{
                status.pad[i].hid_open=1;status.pad[i].last_error=0;status.pad[i].mode_ack=0;
                status.pad[i].stage=5;nova_bt_trace_add(10,i,0,status.pad[i].last_psm);
                link[i].cid=hid_subevent_connection_opened_get_hid_cid(p);
                link[i].mode_tries=0;link[i].mode_next=nova_bt_now()+150;
            }break;
        case HID_SUBEVENT_REPORT:
            i=side_cid(hid_subevent_report_get_hid_cid(p));if(i<0)break;
            if(!status.pad[i].reports)nova_bt_trace_add(11,i,hid_subevent_report_get_report_len(p),0);
            nova_bt_decode(&status.pad[i],i,hid_subevent_report_get_report(p),hid_subevent_report_get_report_len(p),nova_bt_now());
            if(status.pad[i].connected)status.pad[i].stage=6;
            break;
        case HID_SUBEVENT_CONNECTION_CLOSED:
            i=side_cid(hid_subevent_connection_closed_get_hid_cid(p));if(i<0)break;
            clear_pad(i);status.pad[i].hid_open=0;status.pad[i].stage=0;link[i].cid=0;link[i].handle=HCI_CON_HANDLE_INVALID;link[i].next=nova_bt_now()+5000;break;
        default:break;
        }break;
    default:break;
    }
}
static void input_byte(uint8_t value)
{
    if(!rx_type){
        if(value==0||value==0x31||value==0x34)return;
        rx_type=value;rx_len=0;rx_started=nova_bt_now();
        switch(value){case 4:rx_header=2;break;case 2:rx_header=4;break;
        case 3:rx_header=3;break;case 7:rx_header=0;break;default:failure=5;return;}
        rx_need=rx_header?rx_header:63;return;
    }
    if(rx_len>=sizeof(rx)){failure=5;return;}
    rx[rx_len++]=value;
    if(rx_header&&rx_len==rx_header){
        unsigned payload=rx_type==4?rx[1]:rx_type==2?(rx[2]|(rx[3]<<8)):rx[2];
        rx_need=rx_header+payload;
        if(rx_need>sizeof(rx)){failure=5;return;}
    }
    if(rx_len==rx_need){
        uint8_t type=rx_type;unsigned len=rx_len;rx_type=0;rx_len=rx_need=0;
        if(type==2||type==4){status.rx_packets++;if(type==2)status.rx_acl++;trace_frame(0,type,rx,len);receive_packet(type,rx,len);}
    }
}
const nova_bt_status *nova_bt_get_status(void){return &status;}
void nova_bt_stop(void)
{
    if(initialized){
        /* Power is removed first; callbacks cannot outlive the stack pools. */
        nova_bt_radio_close();hid_host_deinit();l2cap_deinit();hci_close();btstack_memory_deinit();
        initialized=0;
    }
    memset(peers,0,sizeof(peers));memset(link,0,sizeof(link));
    tx_len=rx_len=rx_need=rx_type=0;receive_packet=0;memset(status.pad,0,sizeof(status.pad));status.state=NOVA_BT_OFF;status.error=0;
}
int nova_bt_start(const nova_bt_peer p[2])
{
    nova_bt_stop();memset(&status,0,sizeof(status));nova_bt_trace_reset();
    for(unsigned i=0;i<2;i++)link[i].handle=HCI_CON_HANDLE_INVALID;
    if(!nova_bt_validate_peers(p)){status.state=NOVA_BT_ERROR;status.error=1;return 0;}
    if(!nova_bt_radio_open()){status.state=NOVA_BT_ERROR;status.error=2;return 0;}
    memcpy(peers,p,sizeof(peers));failure=0;started=nova_bt_now();
    btstack_memory_init();btstack_run_loop_init(&run_loop);hci_init(&transport,0);initialized=1;
    hci_set_chipset(&chipset);hci_set_link_key_db(&keys);hci_set_bd_addr(peers[peers[0].valid?0:1].host);
    gap_set_security_level(LEVEL_2);gap_ssp_set_auto_accept(0);
    gap_register_classic_connection_filter(known_connection);gap_discoverable_control(0);gap_connectable_control(1);
    l2cap_init();hid_host_init(descriptors,sizeof(descriptors));hid_host_register_packet_handler(event);
    event_registration.callback=event;hci_add_event_handler(&event_registration);
    status.state=NOVA_BT_STARTING;hci_power_control(HCI_POWER_ON);return 1;
}
void nova_bt_poll(void)
{
    if(!initialized)return;
    /* Every loop has a byte budget; no waiting for RX, TX or connection. */
    for(unsigned n=0;n<256&&!failure;n++){
        int value=nova_bt_radio_read();if(value==-2){failure=4;break;}if(value<0)break;input_byte(value);
    }
    if(!failure&&tx_len){
        for(unsigned n=0;n<32&&tx_pos<tx_len;n++)if(nova_bt_radio_write(tx[tx_pos]))tx_pos++;else break;
        if(tx_pos==tx_len){
            trace_frame(1,tx[0],tx+1,tx_len-1);if(tx[0]==HCI_ACL_DATA_PACKET)status.tx_acl++;
            tx_len=0;status.tx_packets++;uint8_t sent[]={HCI_EVENT_TRANSPORT_PACKET_SENT,0};
            receive_packet(HCI_EVENT_PACKET,sent,sizeof(sent));
        }else if((uint32_t)(nova_bt_now()-tx_started)>2000)failure=6;
    }
    if(!failure){
        btstack_run_loop_base_execute_callbacks();btstack_run_loop_base_poll_data_sources();
        btstack_run_loop_base_process_timers(nova_bt_now());
    }
    uint32_t now=nova_bt_now();
    if(rx_type&&(uint32_t)(now-rx_started)>2000)failure=5;
    if(status.state==NOVA_BT_STARTING&&(uint32_t)(now-started)>15000)failure=3;
    for(unsigned i=0;i<2&&!failure&&status.state==NOVA_BT_READY;i++){
        if(status.pad[i].connected&&(uint32_t)(now-status.pad[i].updated)>500)clear_pad(i);
        if(peers[i].valid&&!link[i].cid&&link[i].retries<3&&(int32_t)(now-link[i].next)>=0){
            link[i].retries++;link[i].next=now+5000;
            status.pad[i].stage=1;
            uint8_t result=hid_host_connect(peers[i].mac,HID_PROTOCOL_MODE_REPORT,&link[i].cid);
            if(result){link[i].cid=0;status.pad[i].last_error=result;status.pad[i].stage=7;nova_bt_trace_add(10,i,result,0);}
        }
        if(link[i].cid&&link[i].mode_next&&!status.pad[i].mode_ack&&(int32_t)(now-link[i].mode_next)>=0&&link[i].mode_tries<3){
            /* Keep the output storage alive until BTstack sends it. No rumble. */
            static const uint8_t neutral[]={0,1,0x40,0x40,0,1,0x40,0x40};
            if(!link[i].mode_tries){
                link[i].output[0]=link[i].sequence&15;memcpy(link[i].output+1,neutral,8);
                link[i].output[9]=3;link[i].output[10]=0x30;
            }
            if(!hid_host_send_report(link[i].cid,1,link[i].output,sizeof(link[i].output)))link[i].mode_tries++;
            link[i].mode_next=now+1000;
        }
    }
    if(failure){int code=failure;nova_bt_stop();status.state=NOVA_BT_ERROR;status.error=code;}
}
