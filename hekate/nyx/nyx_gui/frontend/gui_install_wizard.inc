/* Installation assistant first draft. Storage remains in the existing tools.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_partition_plan.h"

static struct {
    bool open,media_ready,erase_ack;
    u8 cid[16];
    u32 step,model,actual,systems,sectors,emmc_mib,os_mib[2];
    nova_partition_plan plan;
    nova_plan_result result;
    const char *media_error;
    lv_obj_t *window,*body,*back,*next,*progress,*model_choices,*systems_buttons[3],*parent_close,*sizes[2][2],*checks[2][2],*emu_size_hint;
} install_wizard;
static void install_wizard_render(void);
static lv_res_t install_wizard_close(lv_obj_t *button)
{
    lv_obj_t *parent_close=install_wizard.parent_close;install_wizard.open=false;
    lv_res_t result=nyx_win_close_action(button);memset(&install_wizard,0,sizeof(install_wizard));
    close_btn=parent_close;return result;
}
static lv_obj_t *install_wizard_button(lv_obj_t *parent,const char *caption,lv_action_t action,int x,int y,int width)
{
    lv_obj_t *button=lv_btn_create(parent,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    lv_btn_set_style(button,LV_BTN_STYLE_REL,&nova_panel);lv_btn_set_style(button,LV_BTN_STYLE_PR,&nova_selected);
    lv_obj_set_size(button,width,54);lv_obj_set_pos(button,x,y);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,action);
    lv_obj_t *label=nyx_nova_text(button,caption,18,14,width-36,NOVA_BODY);nyx_nova_fit_text(label,30);
    nyx_nova_selectable(button,button,NULL);return button;
}
static void install_wizard_emu_hint(void)
{
    if(!install_wizard.emu_size_hint)return;
    char text[96];s_printf(text,nyx_tr("Full emuMMC: %d GB"),install_wizard.model==3?58:29);
    lv_label_set_text(install_wizard.emu_size_hint,text);nyx_nova_fit_text(install_wizard.emu_size_hint,52);
}
static lv_res_t install_wizard_help(lv_obj_t *button)
{
    (void)button;nyx_show_help("Full emuMMC: 29 GB on Switch V1/V2 and Lite; 58 GB on OLED.\nChoose 16 GB minimum or 32 GB recommended for each selected Linux/Android system.\nFor custom partition sizes, close the assistant and open Tools > SD Partition Manager > Expert mode.\nSeveral systems can be selected. Back up the SD card before partitioning.");return LV_RES_OK;
}
static void install_wizard_sizes_refresh(void)
{
    for(u32 os=0;os<2;++os)for(u32 size=0;size<2;++size){
        lv_obj_t *button=install_wizard.sizes[os][size];if(!button)continue;
        bool enabled=(install_wizard.systems&(2u<<os))!=0,selected=install_wizard.os_mib[os]==(size?32768u:16384u);
        lv_obj_set_click(button,enabled);lv_btn_set_state(button,!enabled?LV_BTN_STATE_INA:selected?LV_BTN_STATE_TGL_REL:LV_BTN_STATE_REL);
        lv_obj_set_hidden(install_wizard.checks[os][size],!selected);
    }
}
static lv_res_t install_wizard_size(lv_obj_t *button)
{
    u32 value=lv_obj_get_free_num(button),os=value/2;if(os>1||!(install_wizard.systems&(2u<<os)))return LV_RES_OK;
    install_wizard.os_mib[os]=(value&1)?32768u:16384u;install_wizard_sizes_refresh();return LV_RES_OK;
}
static void install_wizard_size_buttons(lv_obj_t *body,u32 os)
{
    const char *names[]={"16 GB\nMinimum","32 GB\nRecommended"};
    static lv_style_t box_style,mark_style;
    static const lv_point_t tick[]={{0,5},{4,9},{11,0}};
    lv_style_copy(&box_style,&nova_frame);box_style.body.radius=3;
    lv_style_copy(&mark_style,&lv_style_plain);mark_style.line.color=nova_text[NOVA_BODY].text.color;mark_style.line.width=2;
    for(u32 i=0;i<2;++i){
        lv_obj_t *b=lv_btn_create(body,NULL);lv_btn_set_fit(b,false,false);lv_btn_set_layout(b,LV_LAYOUT_OFF);
        lv_btn_set_style(b,LV_BTN_STYLE_REL,&nova_panel);lv_btn_set_style(b,LV_BTN_STYLE_PR,&nova_selected);
        lv_btn_set_style(b,LV_BTN_STYLE_TGL_REL,&nova_selected);lv_btn_set_style(b,LV_BTN_STYLE_TGL_PR,&nova_selected);
        lv_obj_set_size(b,182,60);lv_obj_set_pos(b,420+os*396+i*190,280);lv_obj_set_free_num(b,os*2+i);
        lv_btn_set_action(b,LV_BTN_ACTION_CLICK,install_wizard_size);install_wizard.sizes[os][i]=b;
        lv_obj_t *square=nyx_nova_panel(b,10,20,22,22);
        lv_cont_set_style(square,&box_style);
        /* Draw the check directly so it is visible with every supported font. */
        lv_obj_t *check=lv_line_create(square,NULL);lv_line_set_style(check,&mark_style);lv_line_set_points(check,tick,3);
        lv_obj_set_pos(check,5,5);lv_obj_set_click(check,false);install_wizard.checks[os][i]=check;
        lv_obj_t *text=nyx_nova_text(b,names[i],42,6,132,NOVA_SMALL);nyx_nova_fit_text(text,48);lv_obj_align(text,b,LV_ALIGN_IN_LEFT_MID,42,0);
    }
}
static lv_res_t install_wizard_model(lv_obj_t *button)
{
    u32 value=lv_obj_get_free_num(button);install_wizard.model=value?value+1:0;install_wizard_emu_hint();return LV_RES_OK;
}
static lv_res_t install_wizard_system(lv_obj_t *button)
{
    u32 index=lv_obj_get_free_num(button);if(index>=3)return LV_RES_OK;
    nyx_generic_onoff_toggle(button);
    if(lv_btn_get_state(button)&LV_BTN_STATE_TGL_REL)install_wizard.systems|=1u<<index;
    else install_wizard.systems&=~(1u<<index);
    install_wizard_sizes_refresh();
    lv_obj_set_click(install_wizard.next,install_wizard.systems!=0);
    lv_btn_set_state(install_wizard.next,install_wizard.systems?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    return LV_RES_OK;
}
static void install_wizard_check(void)
{
    install_wizard.media_ready=nyx_nova_installation_read_media(&install_wizard.sectors,&install_wizard.emmc_mib,&install_wizard.actual);
    memset(&install_wizard.plan,0,sizeof(install_wizard.plan));
    install_wizard.plan.total_sectors=install_wizard.sectors;install_wizard.plan.fat_min_mib=2048;
    if(install_wizard.media_ready)install_wizard.media_ready=nyx_nova_installation_read_card(install_wizard.cid);
    install_wizard.media_error=nyx_nova_installation_storage_error();
    install_wizard.result=install_wizard.media_ready?nova_partition_installation_sizes(&install_wizard.plan,
        install_wizard.actual,install_wizard.model,install_wizard.emmc_mib,install_wizard.systems,install_wizard.os_mib[0],install_wizard.os_mib[1]):NOVA_PLAN_CAPACITY;
}
static lv_res_t install_wizard_next(lv_obj_t *button)
{
    (void)button;
    if(install_wizard.step==0){install_wizard.step=1;install_wizard_render();return LV_RES_OK;}
    if(!install_wizard.systems)return LV_RES_OK;
    if(install_wizard.step==1){install_wizard.step=2;install_wizard_check();install_wizard_render();return LV_RES_OK;}
    if(install_wizard.step==2){if(install_wizard.result!=NOVA_PLAN_OK)return LV_RES_OK;install_wizard.erase_ack=false;install_wizard.step=3;install_wizard_render();return LV_RES_OK;}
    if(!install_wizard.erase_ack)return LV_RES_OK;
    u8 cid[16];
    if(!nyx_nova_installation_read_card(cid)||memcmp(cid,install_wizard.cid,16)){
        install_wizard.step=2;install_wizard_check();install_wizard_render();
        nyx_show_help("Storage changed. Restart the installation assistant.");return LV_RES_OK;
    }
    install_wizard_check();
    if(install_wizard.result!=NOVA_PLAN_OK){install_wizard_render();return LV_RES_OK;}
    u32 model=install_wizard.model,systems=install_wizard.systems,linux_mib=install_wizard.os_mib[0],android_mib=install_wizard.os_mib[1];
    memcpy(cid,install_wizard.cid,16);
    install_wizard_close(close_btn);
    nyx_nova_installation_partition(model,systems,linux_mib,android_mib,cid);return LV_RES_INV;
}
static lv_res_t install_wizard_back(lv_obj_t *button)
{
    (void)button;if(install_wizard.step)--install_wizard.step;
    install_wizard_render();return LV_RES_OK;
}
static lv_res_t install_wizard_consent(lv_obj_t *button)
{
    nyx_generic_onoff_toggle(button);install_wizard.erase_ack=(lv_btn_get_state(button)&LV_BTN_STATE_TGL_REL)!=0;
    nova_toggle_ext *ext=lv_obj_get_ext_attr(button);
    lv_label_set_text(ext->state,install_wizard.erase_ack?"I agree":"I do not agree");nyx_nova_fit_text(ext->state,46);lv_obj_align(ext->state,button,LV_ALIGN_IN_LEFT_MID,916,0);
    lv_obj_set_click(install_wizard.next,install_wizard.erase_ack);lv_btn_set_state(install_wizard.next,install_wizard.erase_ack?LV_BTN_STATE_REL:LV_BTN_STATE_INA);return LV_RES_OK;
}
static void install_wizard_render(void)
{
    lv_obj_t *body=install_wizard.body;lv_obj_clean(body);
    memset(install_wizard.sizes,0,sizeof install_wizard.sizes);memset(install_wizard.checks,0,sizeof install_wizard.checks);install_wizard.emu_size_hint=NULL;
    char text[320];s_printf(text,nyx_tr("Step %d / 4"),install_wizard.step+1);lv_label_set_text(install_wizard.progress,text);
    lv_obj_set_hidden(install_wizard.back,!install_wizard.step);
    bool valid=install_wizard.step<2?install_wizard.step==0||install_wizard.systems!=0:install_wizard.result==NOVA_PLAN_OK&&(install_wizard.step==2||install_wizard.erase_ack);
    lv_obj_set_click(install_wizard.next,valid);lv_btn_set_state(install_wizard.next,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    if(install_wizard.step==0){
        nyx_nova_fit_text(nyx_nova_text(body,"Prepare your SD card",24,18,1164,NOVA_TITLE),44);
        const char *titles[]={"1. Choose your console","2. Prepare the partitions","3. Create your emuMMC"};
        const char *details[]={"Select your model and choose emuMMC, Linux or Android.","Partition sizes are calculated automatically. You confirm before the card is erased.","emuMMC copies the internal memory. Linux and Android need their installation files."};
        for(u32 i=0;i<3;++i){lv_obj_t *card=nyx_nova_panel(body,24+i*396,84,372,160);
            nyx_nova_fit_text(nyx_nova_text(card,titles[i],18,14,336,NOVA_BODY),54);
            nyx_nova_fit_text(nyx_nova_text(card,details[i],18,74,336,NOVA_SMALL),72);}
        nyx_nova_fit_text(nyx_nova_text(body,"After successful partitioning, emuMMC is copied and verified automatically when selected.",24,266,1164,NOVA_SMALL),58);
    }else if(install_wizard.step==1){
        nyx_nova_fit_text(nyx_nova_text(body,"Console model",24,14,1164,NOVA_TITLE),44);
        install_wizard.model_choices=nyx_choices_create(body,"Switch V1 V2\nSwitch Lite\nSwitch OLED",install_wizard.model?install_wizard.model-1:0,3,1164,46,install_wizard_model,86,false);
        lv_obj_set_pos(install_wizard.model_choices,24,64);
        nyx_nova_fit_text(nyx_nova_text(body,"Choose what to install",24,122,1164,NOVA_BODY),40);
        const char *names[]={"emuMMC / emuNAND","Linux","Android"};
        for(u32 i=0;i<3;++i){lv_obj_t *b=nyx_nova_toggle(body,names[i],install_wizard_system,(install_wizard.systems&(1u<<i))!=0,372,96);
            install_wizard.systems_buttons[i]=b;lv_obj_set_free_num(b,i);lv_obj_set_pos(b,24+i*396,176);}
        install_wizard.emu_size_hint=nyx_nova_text(body,"",24,284,372,NOVA_SMALL);install_wizard_emu_hint();
        install_wizard_size_buttons(body,0);install_wizard_size_buttons(body,1);install_wizard_sizes_refresh();
    }else if(install_wizard.step==2){
        nyx_nova_fit_text(nyx_nova_text(body,"Review your choices",24,18,1164,NOVA_TITLE),44);
        lv_obj_t *left=nyx_nova_panel(body,24,82,560,236),*right=nyx_nova_panel(body,604,82,584,236);
        const char *model=install_wizard.model==3?"Switch OLED":install_wizard.model==2?"Switch Lite":"Switch V1 / V2";
        nyx_nova_fit_text(nyx_nova_text(left,model,18,14,524,NOVA_BODY),36);
        nova_partition_plan *p=&install_wizard.plan;
        s_printf(text,nyx_tr("FAT32: %d GB\nemuMMC: %d GB\nLinux: %d GB\nAndroid: %d GB"),p->fat_mib>>10,p->emu_mib>>10,p->linux_mib>>10,p->android_mib>>10);
        nyx_nova_fit_text(nyx_nova_text(left,text,18,64,524,NOVA_BODY),154);
        const char *status=!install_wizard.media_ready?(install_wizard.media_error?install_wizard.media_error:"SD card unavailable."):install_wizard.result==NOVA_PLAN_OK?"The layout is ready. The next screen asks for confirmation before partitioning.":install_wizard.result==NOVA_PLAN_MODEL?"The selected model does not match this console.":install_wizard.result==NOVA_PLAN_EMMC_UPGRADED?"Modified internal storage: use the expert partition tool.":install_wizard.result==NOVA_PLAN_EMMC_UNAVAILABLE?"Internal memory unavailable.":"Not enough space. Select 16 GB, remove a system or use a larger SD card.";
        nyx_nova_fit_text(nyx_nova_text(right,status,18,14,548,NOVA_BODY),96);
        nyx_nova_fit_text(nyx_nova_text(right,"All data on the SD card will be lost unless backed up. Back up your files before continuing.",18,130,548,NOVA_SMALL),90);
    }else{
        lv_obj_t *title=nyx_nova_text(body,"Warning: the SD card will be erased",24,18,1164,NOVA_TITLE);
        static lv_style_t warning;lv_style_copy(&warning,nyx_nova_text_style(NOVA_TITLE));warning.text.color=LV_COLOR_HEX(0xF4B85F);lv_label_set_style(title,&warning);nyx_nova_fit_text(title,54);
        nyx_nova_fit_text(nyx_nova_text(body,"All data on the SD card will be lost unless backed up. Back up your files before continuing.",24,88,1164,NOVA_BODY),98);
        nyx_nova_fit_text(nyx_nova_text(body,"After successful partitioning, emuMMC is copied and verified automatically when selected.",24,184,1164,NOVA_SMALL),54);
        lv_obj_t *ack=nova_toggle_base(body,install_wizard_consent,install_wizard.erase_ack,1164,88,844,27,230);
        lv_obj_set_pos(ack,24,250);nyx_nova_glyph(ack,"shield-check",0,18,18);
        nyx_nova_fit_text(nyx_nova_text(ack,"I agree to partition this SD card.",30,12,790,NOVA_BODY),30);
        nyx_nova_fit_text(nyx_nova_text(ack,"Data that has not been backed up will be erased.",30,46,790,NOVA_SMALL),32);
        nova_toggle_ext *ext=lv_obj_get_ext_attr(ack);lv_label_set_text(ext->state,install_wizard.erase_ack?"I agree":"I do not agree");nyx_nova_fit_text(ext->state,46);lv_obj_align(ext->state,ack,LV_ALIGN_IN_LEFT_MID,916,0);
    }
}
lv_res_t nyx_nova_installation_wizard(lv_obj_t *button)
{
    (void)button;if(install_wizard.open)return LV_RES_OK;
    memset(&install_wizard,0,sizeof(install_wizard));install_wizard.open=true;install_wizard.systems=1;install_wizard.os_mib[0]=install_wizard.os_mib[1]=32768u;
    install_wizard.actual=nyx_nova_installation_model();
    install_wizard.model=install_wizard.actual<2?0:install_wizard.actual;install_wizard.parent_close=close_btn;
    lv_obj_t *win=nyx_create_standard_window("Automated installation assistant",install_wizard_close);install_wizard.window=win;
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page);
    install_wizard.progress=nyx_nova_text(page,"",34,24,900,NOVA_KICKER);
    lv_obj_t *help=install_wizard_button(page,"Help",install_wizard_help,1086,8,160);
    lv_obj_t *help_text=lv_obj_get_child(help,NULL);lv_obj_set_x(help_text,48);lv_obj_set_width(help_text,94);
    nyx_nova_fit_text(help_text,30);lv_obj_align(help_text,help,LV_ALIGN_IN_LEFT_MID,48,0);
    nyx_nova_glyph(help,"info",14,15,24);
    install_wizard.body=nyx_nova_panel(page,34,72,1212,height-160);
    install_wizard.back=install_wizard_button(page,"Previous",install_wizard_back,34,height-70,280);
    install_wizard.next=install_wizard_button(page,"Next",install_wizard_next,966,height-70,280);
    install_wizard_render();return LV_RES_OK;
}
