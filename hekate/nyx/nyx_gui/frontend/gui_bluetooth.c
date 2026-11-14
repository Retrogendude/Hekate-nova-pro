#include <bdk.h>
#include <string.h>
#include <libs/fatfs/ff.h>
#include "gui_bluetooth.h"
#include "../bluetooth/nova_bt.h"
#include "../config.h"
#include "../nova_version.h"
static nova_bt_peer paired[2];
static bool boot_checked;
static u8 boot_status; /* 1 rails, 2 started, 3 no file, 4 disabled/Lite,
                       * 5 detection unavailable, 6 already active, 7 failure. */
static struct {u8 attempted,rail[2],record[2],hos[2],compatible,recognized[2],source;} import_info;
static struct {lv_obj_t *win,*label;lv_task_t *task;lv_signal_func_t signal;u32 shown;char note[512];} ui;
static void refresh(void)
{
    if(!ui.label)return;
    const nova_bt_status *s=nova_bt_get_status();char text[3072], detail[512];
    s_printf(detail,"%s: %d / %d | ID: %02X / %02X | HID: %d / %d | ACK: %d / %d | ERR: %02X / %02X | BTN: %06X / %06X | XY: %d,%d / %d,%d | PSM: %04X / %04X | STEP: %d / %d",nyx_tr("Joy-Con reports"),(int)s->pad[0].reports,(int)s->pad[1].reports,s->pad[0].report_id,s->pad[1].report_id,s->pad[0].hid_open,s->pad[1].hid_open,s->pad[0].mode_ack,s->pad[1].mode_ack,s->pad[0].last_error,s->pad[1].last_error,(unsigned)s->pad[0].buttons,(unsigned)s->pad[1].buttons,s->pad[0].x,s->pad[0].y,s->pad[1].x,s->pad[1].y,(unsigned)s->pad[0].last_psm,(unsigned)s->pad[1].last_psm,s->pad[0].stage,s->pad[1].stage);
    const char *states[]={"Bluetooth off","Starting Bluetooth...","Bluetooth ready","Bluetooth error"};
    const char *state=s->state==NOVA_BT_READY?
        (s->pad[0].connected||s->pad[1].connected?"Joy-Con input active":"Radio ready - waiting for Joy-Con"):states[s->state<4?s->state:3];
    s_printf(text,"%s\n%s\n\n%s : %s    %s : %s\n%s : %d / %d\n%s\n\n%s\n%s",
        nyx_tr(state),ui.note,
        nyx_tr("Left Joy-Con"),nyx_tr(s->pad[0].connected?"Receiving inputs":"No input"),
        nyx_tr("Right Joy-Con"),nyx_tr(s->pad[1].connected?"Receiving inputs":"No input"),
        nyx_tr("HCI packets RX / TX"),(int)s->rx_packets,(int)s->tx_packets,detail,
        nyx_tr("Attach Joy-Con, read pairing, detach them, then start Bluetooth. Press a button to wake them."),
        nyx_tr("Stop Bluetooth saves a session report to SD. The radio service remains experimental."));
    if(s->state==NOVA_BT_ERROR){char error[64];s_printf(error,"\n%s : %d",nyx_tr("Error code"),s->error);strcat(text,error);}
    lv_label_set_text(ui.label,text);nyx_nova_fit_text(ui.label,250);
}
static void tick(void *unused)
{
    (void)unused;nova_bt_poll_fast();
    if((u32)(get_tmr_ms()-ui.shown)>=500){ui.shown=get_tmr_ms();refresh();}
}
void gui_bluetooth_shutdown(void)
{
    /* A manual Stop or a diagnostic must not be undone by the boot task. */
    boot_checked=true;
    nova_bt_stop();
    if(ui.task){lv_task_del(ui.task);ui.task=NULL;}
    refresh();
}
bool gui_bluetooth_active(void)
{return nova_bt_get_status()->state==NOVA_BT_READY;}
void gui_bluetooth_poll(void){nova_bt_poll();}
void gui_bluetooth_poll_fast(void){nova_bt_poll_fast();}
static lv_res_t import_pairing(lv_obj_t *button)
{
    (void)button;gui_bluetooth_shutdown();memset(paired,0,sizeof(paired));
    memset(&import_info,0,sizeof(import_info));import_info.attempted=1;import_info.source=1;
    if(n_cfg.jc_disable||fuse_read_hw_type()==FUSE_NX_HW_TYPE_HOAG){
        strcpy(ui.note,nyx_tr("Attachable Joy-Con and the wired Joy-Con driver are required for pairing import."));refresh();return LV_RES_OK;
    }
    bool hos[2]={false,false},recognized[2]={false,false};
    jc_gamepad_rpt_t *p=jc_get_bt_pairing_info_ex(&hos[0],&hos[1],&recognized[0],&recognized[1]);
    if(p)for(unsigned i=0;i<2;i++){
        jc_bt_conn_t *b=i?&p->bt_conn_r:&p->bt_conn_l;
        import_info.rail[i]=i?p->conn_r:p->conn_l;import_info.hos[i]=hos[i];
        import_info.recognized[i]=recognized[i];
        unsigned present=0;for(unsigned j=0;j<16;j++)present|=b->ltk[j];
        import_info.record[i]=present!=0;
        /* The standard 0x08 host profile is a valid stored pairing too.
         * Keep BDK's active-record checksum and both peer/host checks. */
        if(!recognized[i]||!import_info.rail[i]||!(b->type&(i?2:1)))continue;
        paired[i].valid=1;memcpy(paired[i].mac,b->mac,6);memcpy(paired[i].host,b->host_mac,6);
        /* BDK already prepares the exported/HCI key order. Switchroot loads
         * this same byte sequence into BlueZ without another reversal.
         * BTstack's Link_Key_Request_Reply "P" field copies these 16 bytes. */
        memcpy(paired[i].key,b->ltk,16);
    }
    int count=nova_bt_validate_peers(paired);
    import_info.compatible=count!=0;
    if(!count)memset(paired,0,sizeof(paired));
    s_printf(ui.note,"%s : %d / 2",nyx_tr("Pairing records imported"),count);
    if(!count){
        const char *reason=!import_info.rail[0]||!import_info.rail[1]?"Rail connection not ready. Attach both Joy-Con and retry.":
            !import_info.record[0]||!import_info.record[1]?"Pairing data could not be read from the rails.":
            !import_info.recognized[0]||!import_info.recognized[1]?"Pairing checksum or host profile not recognized.":
            "Pairing records did not pass compatibility checks.";
        strcat(ui.note,"\n");strncat(ui.note,nyx_tr(reason),sizeof(ui.note)-strlen(ui.note)-1);
    }
    refresh();return LV_RES_OK;
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
/* Only a deliberate Stop writes to SD. Teardown/diagnostics never use SD. */
static bool save_report(void)
{
    char *buffer=malloc(8192);if(!buffer)return false;
    const nova_bt_status *s=nova_bt_get_status();
    s_printf(buffer,"Hekate Nova Pro " NYX_NOVA_VERSION " Bluetooth\n"
        "Metadata only: no addresses, pairing keys or packet payloads.\n"
        "HCI RX/TX %d/%d; ACL RX/TX %d/%d; state %d error %d; HOST=%d (0=unknown 1=match 2=mismatch)\n",
        (int)s->rx_packets,(int)s->tx_packets,(int)s->rx_acl,(int)s->tx_acl,s->state,s->error,s->host_identity);
    u32 length=strlen(buffer);
    s_printf(buffer+length,"Autostart=%d (1=rails 2=started 3=no-file 4=disabled 5=unknown 6=active 7=failed)\n",boot_status);length=strlen(buffer);
    s_printf(buffer+length,"Import: attempted=%d rail=%d/%d record=%d/%d Nintendo=%d/%d compatible=%d recognized=%d/%d source=%d (1=rails 2=SD)\n",
        import_info.attempted,import_info.rail[0],import_info.rail[1],import_info.record[0],import_info.record[1],
        import_info.hos[0],import_info.hos[1],import_info.compatible,import_info.recognized[0],import_info.recognized[1],import_info.source);length=strlen(buffer);
    s_printf(buffer+length,"Controller: HCI=%02X revision=%04X manufacturer=%04X subversion=%04X\n",
        s->controller_hci_version,s->controller_revision,s->controller_manufacturer,s->controller_subversion);
    length=strlen(buffer);
    s_printf(buffer+length,"RX bytes=%d polls=%d max_call_us=%d; profile=HWTest-RCM BR/sniff/QoS\n",
        (int)s->rx_bytes,(int)s->rx_polls,(int)s->rx_max_us);length=strlen(buffer);
    s_printf(buffer+length,"Poll total_max_us=%d fast_max_us=%d normal_max_us=%d gap_max_us=%d RX_budget_hits=%d\n",
        (int)s->poll_max_us,(int)s->poll_fast_max_us,(int)s->poll_normal_max_us,(int)s->poll_gap_max_us,(int)s->rx_budget_hits);length=strlen(buffer);
    for(unsigned side=0;side<2;side++){
        const nova_bt_pad *p=&s->pad[side];
        s_printf(buffer+length,"side %d: HID=%d reports=%d ID=%02X ACK=%d ERR=%02X PSM=%04X STEP=%d AUTH=%02X SEEN=%d ENC=%02X SEEN=%d ENABLED=%d DISC=%02X ROLE=%02X BR_ONLY=%d MODE=%d SEEN=%d\n",
            side,p->hid_open,(int)p->reports,p->report_id,p->mode_ack,p->last_error,(unsigned)p->last_psm,p->stage,p->auth_error,p->auth_seen,
            p->encryption_error,p->encryption_seen,p->encrypted,p->disconnect_reason,p->role,p->br_only,p->radio_mode,p->mode_seen);
        length=strlen(buffer);
        u32 age=p->reports?(u32)(get_tmr_ms()-p->first_report_ms):0;
        u32 rate=age?(u32)((u64)p->reports*1000/age):0;
        s_printf(buffer+length,"  Buttons seen=%06X first_report_ms=%d last_report_ms=%d avg_reports_s=%d\n",
            (unsigned)p->buttons_seen,(int)p->first_report_ms,(int)p->updated,(int)rate);
        length=strlen(buffer);
    }
    strcpy(buffer+length,"Trace: ms kind a b c (hex)\n1=TXcmd 2=CC 3=CS 4=event 5=TXsignal 6=RXsignal 7=TXhid 10=HIDopen 11=firstReport 12=hostIdentity 13=role(side,handle,0central/1peripheral) 14=acceptPolicy(side,opcode,role) 15=packetTypes(handle,opcode,mask) 16=mode(handle,mode,interval) 17=config(CID,flags,MTU) 19=unusualEvent(code,size,declared) 20=pageTimeout(opcode,slots,reserved)\n");
    length=strlen(buffer);
    for(unsigned i=0;i<nova_bt_trace_count()&&length+80<8192;i++){
        nova_bt_trace r;if(!nova_bt_trace_get(i,&r))break;
        s_printf(buffer+length,"%08d %02d %04X %04X %04X\n",(int)r.ms,r.kind,r.a,r.b,r.c);length=strlen(buffer);
    }
    bool was_mounted=sd_get_card_mounted();
    if(sd_mount()!=0){free(buffer);return false;}
    int result=f_mkdir("bootloader/bluetooth");
    if(result==FR_EXIST)result=0;
    FIL fp;u32 written=0;
    if(!result){
        result=f_open(&fp,"bootloader/bluetooth/last-session.txt",FA_WRITE|FA_CREATE_ALWAYS);
        if(!result){
            result=f_write(&fp,buffer,length,&written);
            if(!result&&written!=length)result=1;
            int closed=f_close(&fp);if(!result)result=closed;
        }
    }
    if(!was_mounted)sd_unmount();
    free(buffer);
    return result==0;
}
static lv_res_t stop(lv_obj_t *button)
{
    (void)button;bool had_trace=nova_bt_trace_count()!=0||import_info.attempted||boot_status;
    bool saved=had_trace&&save_report();gui_bluetooth_shutdown();
    if(had_trace){strcpy(ui.note,nyx_tr(saved?"Bluetooth report saved: bootloader/bluetooth/last-session.txt":"Bluetooth stopped. Report could not be saved."));refresh();}
    return LV_RES_OK;
}
static lv_res_t forget(lv_obj_t *button)
{stop(button);memset(paired,0,sizeof(paired));strcpy(ui.note,nyx_tr("Session keys cleared."));refresh();return LV_RES_OK;}
/* The only persistent pairing copy is an explicit user action. The live
 * radio is stopped before SD access; failed loads keep current RAM keys. */
static int pairing_file(bool write)
{
    static const char *path="bootloader/bluetooth/joycon-pairing.bin";
    static const char *backup="bootloader/bluetooth/joycon-pairing.bak";
    static const char *temp="bootloader/bluetooth/joycon-pairing.tmp";
    u8 bytes[NOVA_BT_PAIRING_SIZE];nova_bt_peer candidate[2];int count=0;
    memset(bytes,0,sizeof(bytes));memset(candidate,0,sizeof(candidate));
    bool mounted=sd_get_card_mounted();
    if(write&&!nova_bt_pairing_encode(bytes,paired))goto done;
    if(sd_mount()!=0)goto done;
    FIL fp;u32 transferred=0;int result;
    if(write){
        result=f_mkdir("bootloader/bluetooth");if(result&&result!=FR_EXIST)goto unmount;
        result=f_open(&fp,temp,FA_WRITE|FA_CREATE_ALWAYS);if(result)goto unmount;
        result=f_write(&fp,bytes,sizeof(bytes),&transferred);int closed=f_close(&fp);
        if(result||closed||transferred!=sizeof(bytes)){f_unlink(temp);goto unmount;}
        result=f_stat(path,NULL);bool had_old=result==FR_OK;
        if(result!=FR_OK&&result!=FR_NO_FILE){f_unlink(temp);goto unmount;}
        if(had_old){
            result=f_unlink(backup);if(result!=FR_OK&&result!=FR_NO_FILE){f_unlink(temp);goto unmount;}
            if(f_rename(path,backup)!=FR_OK){f_unlink(temp);goto unmount;}
        }
        if(f_rename(temp,path)!=FR_OK){if(had_old)f_rename(backup,path);f_unlink(temp);goto unmount;}
        count=nova_bt_validate_peers(paired);
    }else{
        const char *paths[]={path,backup};
        for(unsigned i=0;i<2&&!count;i++){
            result=f_open(&fp,paths[i],FA_READ);if(result)continue;
            bool size_ok=f_size(&fp)==sizeof(bytes);
            result=size_ok?f_read(&fp,bytes,sizeof(bytes),&transferred):1;int closed=f_close(&fp);
            if(!result&&!closed&&transferred==sizeof(bytes))count=nova_bt_pairing_decode(candidate,bytes,sizeof(bytes));
        }
        if(count){
            memcpy(paired,candidate,sizeof(paired));memset(&import_info,0,sizeof(import_info));
            import_info.attempted=1;import_info.compatible=1;import_info.source=2;
            for(unsigned i=0;i<2;i++)import_info.record[i]=import_info.recognized[i]=paired[i].valid;
        }
    }
unmount:
    if(!mounted)sd_unmount();
done:
    memset(bytes,0,sizeof(bytes));memset(candidate,0,sizeof(candidate));return count;
}
static lv_res_t store_pairing(lv_obj_t *button)
{
    bool write=lv_obj_get_free_num(button)!=0;
    if(nova_bt_get_status()->state==NOVA_BT_STARTING||gui_bluetooth_active()){
        strcpy(ui.note,nyx_tr("Stop Bluetooth before saving or loading pairing."));refresh();return LV_RES_OK;
    }
    int count=pairing_file(write);
    const char *message=write?(count?"Pairing saved to SD. Keep this file private.":"Pairing could not be saved. Read valid pairing first."):
        (count?"Pairing loaded from SD.":"No valid pairing file found. Current session keys kept.");
    s_printf(ui.note,"%s\n%s : %d / 2",nyx_tr(message),nyx_tr("Pairing records imported"),nova_bt_validate_peers(paired));
    refresh();return LV_RES_OK;
}
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
    lv_obj_t *panel=nyx_nova_panel(scrl,24,16,lv_obj_get_width(page)-48,286);
    ui.label=nyx_nova_text(panel,"",20,18,lv_obj_get_width(panel)-40,NOVA_BODY);
    const char *titles[]={"Read pairing","Start Bluetooth","Stop Bluetooth","Save pairing","Load pairing","Clear session keys"};
    const char *icons[]={"download","bluetooth","square","hard-drive","key-round","trash-2"};
    lv_action_t actions[]={import_pairing,start,stop,store_pairing,store_pairing,forget};
    int width=(lv_obj_get_width(page)-72)/3;
    for(unsigned i=0;i<6;i++)nyx_nova_action(scrl,titles[i],NULL,icons[i],actions[i],i==3,24+(i%3)*(width+12),318+(i/3)*88,width,76);
    refresh();return LV_RES_OK;
}
void gui_bluetooth_autostart(void)
{
    if(boot_checked)return;
    boot_checked=true;
    if(n_cfg.jc_disable||fuse_read_hw_type()==FUSE_NX_HW_TYPE_HOAG){boot_status=4;return;}
    int attached=jc_get_attached_mask();
    if(attached<0){boot_status=5;return;}
    if(attached){boot_status=1;return;}
    const nova_bt_status *s=nova_bt_get_status();
    if(s->state==NOVA_BT_STARTING||s->state==NOVA_BT_READY){boot_status=6;return;}
    if(!pairing_file(false)){boot_status=3;return;}
    /* Recheck after SD I/O: a newly attached controller always wins. */
    attached=jc_get_attached_mask();
    if(attached){
        memset(paired,0,sizeof(paired));memset(&import_info,0,sizeof(import_info));
        boot_status=attached<0?5:1;return;
    }
    s_printf(ui.note,"%s\n%s : %d / 2",nyx_tr("Saved pairing loaded. Bluetooth starts automatically."),nyx_tr("Pairing records imported"),nova_bt_validate_peers(paired));
    start(NULL);
    s=nova_bt_get_status();boot_status=(s->state==NOVA_BT_STARTING||s->state==NOVA_BT_READY)?2:7;
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
        u32 buttons=nova_bt_take_buttons(i);
        if(!p->connected||(u32)(get_tmr_ms()-p->updated)>500)continue;
        if(i?!merged.conn_r:!merged.conn_l){
            u32 mask=i?0x0016ff:0xff2900;
            merged.buttons=(merged.buttons&~mask)|buttons;
            if(i){merged.conn_r=true;merged.rstick_x=p->x;merged.rstick_y=p->y;merged.batt_info_r=p->battery;}
            else{merged.conn_l=true;merged.lstick_x=p->x;merged.lstick_y=p->y;merged.batt_info_l=p->battery;}
        }
    }
    return &merged;
}
