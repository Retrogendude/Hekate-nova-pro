/* Installation assistant first draft. Storage remains in the existing tools.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_partition_plan.h"

static struct {
    bool open,media_ready;
    u32 step,model,actual,systems,sectors,emmc_mib;
    nova_partition_plan plan;
    nova_plan_result result;
    lv_obj_t *window,*body,*back,*next,*progress,*model_choices,*systems_buttons[3],*parent_close;
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
static lv_res_t install_wizard_model(lv_obj_t *button)
{
    u32 value=lv_obj_get_free_num(button);install_wizard.model=value?value+1:0;return LV_RES_OK;
}
static lv_res_t install_wizard_system(lv_obj_t *button)
{
    u32 index=lv_obj_get_free_num(button);if(index>=3)return LV_RES_OK;
    nyx_generic_onoff_toggle(button);
    if(lv_btn_get_state(button)&LV_BTN_STATE_TGL_REL)install_wizard.systems|=1u<<index;
    else install_wizard.systems&=~(1u<<index);
    lv_obj_set_click(install_wizard.next,install_wizard.systems!=0);
    lv_btn_set_state(install_wizard.next,install_wizard.systems?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    return LV_RES_OK;
}
static void install_wizard_check(void)
{
    install_wizard.media_ready=nyx_nova_installation_read_media(&install_wizard.sectors,&install_wizard.emmc_mib,&install_wizard.actual);
    memset(&install_wizard.plan,0,sizeof(install_wizard.plan));
    install_wizard.plan.total_sectors=install_wizard.sectors;install_wizard.plan.fat_min_mib=2048;
    install_wizard.result=install_wizard.media_ready?nova_partition_installation(&install_wizard.plan,
        install_wizard.actual,install_wizard.model,install_wizard.emmc_mib,install_wizard.systems):NOVA_PLAN_CAPACITY;
}
static lv_res_t install_wizard_next(lv_obj_t *button)
{
    (void)button;
    if(install_wizard.step==0){install_wizard.step=1;install_wizard_render();return LV_RES_OK;}
    if(!install_wizard.systems)return LV_RES_OK;
    if(install_wizard.step==1){install_wizard.step=2;install_wizard_check();install_wizard_render();return LV_RES_OK;}
    install_wizard_check();
    if(install_wizard.result!=NOVA_PLAN_OK){install_wizard_render();return LV_RES_OK;}
    u32 model=install_wizard.model,systems=install_wizard.systems;
    install_wizard_close(close_btn);
    nyx_nova_installation_partition(model,systems);return LV_RES_INV;
}
static lv_res_t install_wizard_back(lv_obj_t *button)
{
    (void)button;if(install_wizard.step)--install_wizard.step;
    install_wizard_render();return LV_RES_OK;
}
static void install_wizard_render(void)
{
    lv_obj_t *body=install_wizard.body;lv_obj_clean(body);
    char text[320];s_printf(text,nyx_tr("Step %d / 3"),install_wizard.step+1);lv_label_set_text(install_wizard.progress,text);
    lv_obj_set_hidden(install_wizard.back,!install_wizard.step);
    bool valid=install_wizard.step!=2?install_wizard.step==0||install_wizard.systems!=0:install_wizard.result==NOVA_PLAN_OK;
    lv_obj_set_click(install_wizard.next,valid);lv_btn_set_state(install_wizard.next,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    if(install_wizard.step==0){
        nyx_nova_fit_text(nyx_nova_text(body,"Prepare your SD card",24,18,1164,NOVA_TITLE),44);
        const char *titles[]={"1. Choose your console","2. Prepare the partitions","3. Create your emuMMC"};
        const char *details[]={"Select your model and choose emuMMC, Linux or Android.","Partition sizes are calculated automatically. You confirm before the card is erased.","emuMMC copies the internal memory. Linux and Android need their installation files."};
        for(u32 i=0;i<3;++i){lv_obj_t *card=nyx_nova_panel(body,24+i*396,84,372,160);
            nyx_nova_fit_text(nyx_nova_text(card,titles[i],18,14,336,NOVA_BODY),54);
            nyx_nova_fit_text(nyx_nova_text(card,details[i],18,74,336,NOVA_SMALL),72);}
        nyx_nova_fit_text(nyx_nova_text(body,"First draft: partition setup is available. Automatic emuMMC copying is not enabled yet.",24,266,1164,NOVA_SMALL),58);
    }else if(install_wizard.step==1){
        nyx_nova_fit_text(nyx_nova_text(body,"Console model",24,18,1164,NOVA_TITLE),44);
        install_wizard.model_choices=nyx_choices_create(body,"Switch V1 V2\nSwitch Lite\nSwitch OLED",install_wizard.model?install_wizard.model-1:0,3,1164,50,install_wizard_model,86,false);
        lv_obj_set_pos(install_wizard.model_choices,24,76);
        nyx_nova_fit_text(nyx_nova_text(body,"Choose what to install",24,142,1164,NOVA_BODY),40);
        const char *names[]={"emuMMC / emuNAND","Linux","Android"};
        for(u32 i=0;i<3;++i){lv_obj_t *b=nyx_nova_toggle(body,names[i],install_wizard_system,(install_wizard.systems&(1u<<i))!=0,372,96);
            install_wizard.systems_buttons[i]=b;lv_obj_set_free_num(b,i);lv_obj_set_pos(b,24+i*396,196);}
        nyx_nova_fit_text(nyx_nova_text(body,"You can select more than one system.",24,302,1164,NOVA_SMALL),30);
    }else{
        nyx_nova_fit_text(nyx_nova_text(body,"Review your choices",24,18,1164,NOVA_TITLE),44);
        lv_obj_t *left=nyx_nova_panel(body,24,82,560,236),*right=nyx_nova_panel(body,604,82,584,236);
        const char *model=install_wizard.model==3?"Switch OLED":install_wizard.model==2?"Switch Lite":"Switch V1 / V2";
        nyx_nova_fit_text(nyx_nova_text(left,model,18,14,524,NOVA_BODY),36);
        nova_partition_plan *p=&install_wizard.plan;
        s_printf(text,nyx_tr("FAT32: %d GB\nemuMMC: %d GB\nLinux: %d GB\nAndroid: %d GB"),p->fat_mib>>10,p->emu_mib>>10,p->linux_mib>>10,p->android_mib>>10);
        nyx_nova_fit_text(nyx_nova_text(left,text,18,64,524,NOVA_BODY),154);
        const char *status=!install_wizard.media_ready?"SD card unavailable.":install_wizard.result==NOVA_PLAN_OK?"The layout is ready. The next screen asks for confirmation before partitioning.":install_wizard.result==NOVA_PLAN_MODEL?"The selected model does not match this console.":install_wizard.result==NOVA_PLAN_EMMC_UPGRADED?"Modified internal storage: use the expert partition tool.":install_wizard.result==NOVA_PLAN_EMMC_UNAVAILABLE?"Internal memory unavailable.":"Not enough space. Change the selected systems or use a larger SD card.";
        nyx_nova_fit_text(nyx_nova_text(right,status,18,14,548,NOVA_BODY),96);
        nyx_nova_fit_text(nyx_nova_text(right,"Back up the SD card first. Automatic emuMMC copying will be added after this first test.",18,130,548,NOVA_SMALL),90);
    }
}
lv_res_t nyx_nova_installation_wizard(lv_obj_t *button)
{
    (void)button;if(install_wizard.open)return LV_RES_OK;
    memset(&install_wizard,0,sizeof(install_wizard));install_wizard.open=true;install_wizard.systems=1;
    install_wizard.actual=nyx_nova_installation_model();
    install_wizard.model=install_wizard.actual<2?0:install_wizard.actual;install_wizard.parent_close=close_btn;
    lv_obj_t *win=nyx_create_standard_window("Installation assistant",install_wizard_close);install_wizard.window=win;
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page);
    install_wizard.progress=nyx_nova_text(page,"",34,24,1212,NOVA_KICKER);
    install_wizard.body=nyx_nova_panel(page,34,72,1212,height-160);
    install_wizard.back=install_wizard_button(page,"Previous",install_wizard_back,34,height-70,280);
    install_wizard.next=install_wizard_button(page,"Next",install_wizard_next,966,height-70,280);
    install_wizard_render();return LV_RES_OK;
}
