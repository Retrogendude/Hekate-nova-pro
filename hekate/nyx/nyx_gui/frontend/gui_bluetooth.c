#include <bdk.h>
#include <string.h>
#include "gui_bluetooth.h"
#include "../bluetooth/nova_bt.h"
#include "../config.h"
static nova_bt_peer paired[2];
static struct {lv_obj_t *win,*label;lv_task_t *task;lv_signal_func_t signal;u32 shown;char note[256];} ui;
static void refresh(void)
{
    if(!ui.label)return;
    const nova_bt_status *s=nova_bt_get_status();char text[2048], detail[384];
    s_printf(detail,"%s: %d / %d | ID: %02X / %02X | HID: %d / %d | ACK: %d / %d | ERR: %02X / %02X | BTN: %06X / %06X | XY: %d,%d / %d,%d",nyx_tr("Joy-Con reports"),(int)s->pad[0].reports,(int)s->pad[1].reports,s->pad[0].report_id,s->pad[1].report_id,s->pad[0].hid_open,s->pad[1].hid_open,s->pad[0].mode_ack,s->pad[1].mode_ack,s->pad[0].last_error,s->pad[1].last_error,(unsigned)s->pad[0].buttons,(unsigned)s->pad[1].buttons,s->pad[0].x,s->pad[0].y,s->pad[1].x,s->pad[1].y);
    const char *states[]={"Bluetooth off","Starting Bluetooth...","Bluetooth ready","Bluetooth error"};
    s_printf(text,"%s\n%s\n\n%s : %s    %s : %s\n%s : %d / %d\n%s\n\n%s\n%s",
        nyx_tr(states[s->state<4?s->state:3]),ui.note,
        nyx_tr("Left Joy-Con"),nyx_tr(s->pad[0].connected?"Receiving inputs":"No input"),
        nyx_tr("Right Joy-Con"),nyx_tr(s->pad[1].connected?"Receiving inputs":"No input"),
        nyx_tr("HCI packets RX / TX"),(int)s->rx_packets,(int)s->tx_packets,detail,
        nyx_tr("Read pairing while the Joy-Con are attached, then detach them and start Bluetooth. Press a Joy-Con button to wake it."),
        nyx_tr("Experimental ROM service. Console validation required. Keys stay in RAM; existing pairing is not changed."));
    if(s->state==NOVA_BT_ERROR){char error[64];s_printf(error,"\n%s : %d",nyx_tr("Error code"),s->error);strcat(text,error);}
    lv_label_set_text(ui.label,text);nyx_nova_fit_text(ui.label,270);
}
static void tick(void *unused)
{
    (void)unused;nova_bt_poll();
    if((u32)(get_tmr_ms()-ui.shown)>=250){ui.shown=get_tmr_ms();refresh();}
}
void gui_bluetooth_shutdown(void)
{
    nova_bt_stop();
    if(ui.task){lv_task_del(ui.task);ui.task=NULL;}
    refresh();
}
bool gui_bluetooth_active(void)
{return nova_bt_get_status()->state==NOVA_BT_READY;}
void gui_bluetooth_poll(void){nova_bt_poll();}
static lv_res_t import_pairing(lv_obj_t *button)
{
    (void)button;gui_bluetooth_shutdown();memset(paired,0,sizeof(paired));
    if(n_cfg.jc_disable||fuse_read_hw_type()==FUSE_NX_HW_TYPE_HOAG){
        strcpy(ui.note,nyx_tr("Attachable Joy-Con and the wired Joy-Con driver are required for pairing import."));refresh();return LV_RES_OK;
    }
    bool hos[2]={false,false};jc_gamepad_rpt_t *p=jc_get_bt_pairing_info(&hos[0],&hos[1]);
    if(p)for(unsigned i=0;i<2;i++){
        jc_bt_conn_t *b=i?&p->bt_conn_r:&p->bt_conn_l;
        if(!hos[i]||!(b->type&(i?2:1)))continue;
        paired[i].valid=1;memcpy(paired[i].mac,b->mac,6);memcpy(paired[i].host,b->host_mac,6);
        /* BDK reverses the SPI key for its exported big-endian string.
         * BTstack expects the original 16-byte HCI link-key wire order. */
        for(unsigned j=0;j<16;j++)paired[i].key[j]=b->ltk[15-j];
    }
    int count=nova_bt_validate_peers(paired);
    if(!count)memset(paired,0,sizeof(paired));
    s_printf(ui.note,"%s : %d / 2",nyx_tr("Pairing records imported"),count);refresh();return LV_RES_OK;
}
static lv_res_t start(lv_obj_t *button)
{
    (void)button;
    if(nova_bt_get_status()->state==NOVA_BT_STARTING||gui_bluetooth_active())return LV_RES_OK;
    /* Allocation precedes touching radio hardware. */
    if(!ui.task)ui.task=lv_task_create(tick,2,LV_TASK_PRIO_HIGH,NULL);
    if(!ui.task){strcpy(ui.note,nyx_tr("Bluetooth service could not start."));refresh();return LV_RES_OK;}
    if(!nova_bt_start(paired)){lv_task_del(ui.task);ui.task=NULL;}
    refresh();return LV_RES_OK;
}
static lv_res_t stop(lv_obj_t *button){(void)button;gui_bluetooth_shutdown();return LV_RES_OK;}
static lv_res_t forget(lv_obj_t *button)
{stop(button);memset(paired,0,sizeof(paired));strcpy(ui.note,nyx_tr("Session keys cleared."));refresh();return LV_RES_OK;}
static lv_res_t signal(lv_obj_t *obj,lv_signal_t sign,void *param)
{
    lv_signal_func_t previous=ui.signal;
    if(sign==LV_SIGNAL_CLEANUP){ui.win=NULL;ui.label=NULL;}
    return previous(obj,sign,param);
}
lv_res_t gui_bluetooth_open(lv_obj_t *button)
{
    (void)button;if(ui.win)return LV_RES_OK;
    ui.win=nyx_create_standard_window(nyx_tr("Wireless Joy-Con"),NULL);
    ui.signal=lv_obj_get_signal_func(ui.win);lv_obj_set_signal_func(ui.win,signal);
    lv_obj_t *page=lv_win_get_content(ui.win);nyx_nova_fixed_page(page);
    lv_obj_t *scrl=lv_page_get_scrl(page);
    lv_obj_t *panel=nyx_nova_panel(scrl,24,16,lv_obj_get_width(page)-48,306);
    ui.label=nyx_nova_text(panel,"",20,18,lv_obj_get_width(panel)-40,NOVA_BODY);
    const char *titles[]={"Read pairing","Start Bluetooth","Stop Bluetooth","Clear session keys"};
    const char *icons[]={"download","bluetooth","square","trash-2"};
    lv_action_t actions[]={import_pairing,start,stop,forget};
    int width=(lv_obj_get_width(page)-84)/4;
    for(unsigned i=0;i<4;i++)nyx_nova_action(scrl,titles[i],NULL,icons[i],actions[i],0,24+i*(width+12),338,width,120);
    refresh();return LV_RES_OK;
}
jc_gamepad_rpt_t *gui_bluetooth_merge(jc_gamepad_rpt_t *wired)
{
    static jc_gamepad_rpt_t merged;
    const nova_bt_status *s=nova_bt_get_status();
    if(!s->pad[0].connected&&!s->pad[1].connected)return wired;
    if(wired)memcpy(&merged,wired,sizeof(merged));else{
        memset(&merged,0,sizeof(merged));merged.lstick_x=merged.lstick_y=merged.rstick_x=merged.rstick_y=0x800;
    }
    for(unsigned i=0;i<2;i++){
        const nova_bt_pad *p=&s->pad[i];
        if(!p->connected||(u32)(get_tmr_ms()-p->updated)>500)continue;
        if(i?!merged.conn_r:!merged.conn_l){
            u32 mask=i?0x0016ff:0xff2900;
            merged.buttons=(merged.buttons&~mask)|p->buttons;
            if(i){merged.conn_r=true;merged.rstick_x=p->x;merged.rstick_y=p->y;merged.batt_info_r=p->battery;}
            else{merged.conn_l=true;merged.lstick_x=p->x;merged.lstick_y=p->y;merged.batt_info_l=p->battery;}
        }
    }
    return &merged;
}
