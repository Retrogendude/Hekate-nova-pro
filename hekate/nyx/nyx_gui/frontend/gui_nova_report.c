/* Bounded Nova reports, properties and transfer views. GPL-2.0-or-later. */
#include "gui.h"
#include "gui_nova.h"
#include "../fonts/font_manager.h"
#include <string.h>
#include <stdlib.h>

void nyx_nova_fixed_page(lv_obj_t *page)
{
    lv_page_set_style(page,LV_PAGE_STYLE_BG,&lv_style_transp_tight);
    lv_page_set_style(page,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
    lv_page_set_scrl_fit(page,false,false);lv_page_set_scrl_layout(page,LV_LAYOUT_OFF);
    lv_page_set_scrl_width(page,lv_obj_get_width(page));lv_page_set_scrl_height(page,lv_obj_get_height(page));
    lv_obj_set_pos(lv_page_get_scrl(page),0,0);lv_page_set_sb_mode(page,LV_SB_MODE_OFF);lv_obj_set_drag(lv_page_get_scrl(page),false);
}

void nyx_nova_align(lv_obj_t *object,const lv_obj_t *base,lv_align_t alignment,lv_coord_t x,lv_coord_t y)
{
    lv_obj_type_t type;lv_obj_get_type(object,&type);
    bool modal=!base&&alignment==LV_ALIGN_CENTER&&type.type[0]&&!strcmp(type.type[0],"lv_mbox");
    int available=NYX_NOVA_BOTTOM-NYX_NOVA_HEADER-24;
    if(modal&&lv_obj_get_height(object)>available){
        lv_obj_set_width(object,LV_HOR_RES-68);
        lv_mbox_ext_t *ext=lv_obj_get_ext_attr(object);
        if(lv_obj_get_height(object)>available)lv_label_set_style(ext->text,(lv_style_t *)nyx_nova_text_style(NOVA_SMALL));
        if(lv_obj_get_height(object)>available)lv_label_set_style(ext->text,(lv_style_t *)nyx_nova_text_style(NOVA_KICKER));
    }
    (lv_obj_align)(object,base,alignment,x,y);
    if(modal){
        int height=lv_obj_get_height(object),top=NYX_NOVA_HEADER+(NYX_NOVA_BOTTOM-NYX_NOVA_HEADER-height)/2+y;
        if(top<NYX_NOVA_HEADER+12)top=NYX_NOVA_HEADER+12;
        if(top+height>NYX_NOVA_BOTTOM-12)top=NYX_NOVA_BOTTOM-12-height;
        lv_obj_set_y(object,top);
    }
}

typedef struct {
    lv_cont_ext_t container;
    lv_signal_func_t base_signal;
    char *text,*slice;
    u32 starts[128],page,next;
    int text_height;
    lv_obj_t *label,*counter,*previous,*following;
} report_ext;
static void report_fill(lv_obj_t *panel)
{
    report_ext *ext=lv_obj_get_ext_attr(panel);u32 start=ext->starts[ext->page],at=start,last=start;
    size_t length=strlen(ext->text);
    while(at<length){
        const char *newline=strchr(ext->text+at,'\n');u32 end=newline?(u32)(newline-ext->text)+1:length;
        memcpy(ext->slice,ext->text+start,end-start);ext->slice[end-start]=0;
        (lv_label_set_text)(ext->label,ext->slice);
        if(lv_obj_get_height(ext->label)>ext->text_height&&last>start)break;
        last=end;at=end;
    }
    if(last==start&&at<length)last=length;
    memcpy(ext->slice,ext->text+start,last-start);ext->slice[last-start]=0;
    (lv_label_set_text)(ext->label,ext->slice);ext->next=last;
    bool has_next=last<length&&ext->page+1<128;
    lv_obj_set_click(ext->previous,ext->page!=0);lv_btn_set_state(ext->previous,ext->page?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    lv_obj_set_click(ext->following,has_next);lv_btn_set_state(ext->following,has_next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    char position[32];s_printf(position,nyx_tr("Page %d"),ext->page+1);(lv_label_set_text)(ext->counter,position);
}
static lv_res_t report_step(lv_obj_t *button)
{
    lv_obj_t *panel=lv_obj_get_parent(button);report_ext *ext=lv_obj_get_ext_attr(panel);
    if(lv_obj_get_free_num(button)){
        if(ext->next>=strlen(ext->text)||ext->page+1>=128)return LV_RES_OK;
        ext->starts[++ext->page]=ext->next;
    }else if(ext->page)--ext->page;
    report_fill(panel);return LV_RES_OK;
}
static lv_res_t report_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    report_ext *ext=lv_obj_get_ext_attr(object);lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP){free(ext->text);free(ext->slice);}
    return result;
}
static lv_obj_t *report_button(lv_obj_t *parent,const char *text,int x,int y,int width,u32 id)
{
    lv_obj_t *button=lv_btn_create(parent,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    lv_obj_set_pos(button,x,y);lv_obj_set_size(button,width,42);lv_obj_set_free_num(button,id);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,report_step);
    lv_obj_t *label=nyx_nova_text(button,text,8,10,width-16,NOVA_SMALL);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);return button;
}
lv_obj_t *nyx_nova_report(lv_obj_t *parent,const char *text,int x,int y,int width,int height)
{
    lv_obj_t *panel=nyx_nova_panel(parent,x,y,width,height);lv_signal_func_t base=lv_obj_get_signal_func(panel);
    report_ext *ext=lv_obj_allocate_ext_attr(panel,sizeof(*ext));if(!ext)return panel;
    /* Preserve the inherited container extension when initializing our tail. */
    memset((u8 *)ext+sizeof(ext->container),0,sizeof(*ext)-sizeof(ext->container));ext->base_signal=base;
    char *translated=nyx_translate_text(text);ext->text=translated;
    if(!ext->text){ext->text=malloc(strlen(text)+1);if(ext->text)strcpy(ext->text,text);}
    ext->slice=ext->text?malloc(strlen(ext->text)+1):NULL;
    if(!ext->text||!ext->slice){free(ext->text);free(ext->slice);ext->text=ext->slice=NULL;return panel;}
    lv_obj_set_signal_func(panel,report_signal);ext->text_height=height-92;
    ext->label=lv_label_create(panel,NULL);lv_label_set_style(ext->label,&monospace_text);lv_label_set_recolor(ext->label,true);lv_label_set_long_mode(ext->label,LV_LABEL_LONG_BREAK);
    lv_obj_set_width(ext->label,width-48);lv_obj_set_pos(ext->label,24,18);
    ext->previous=report_button(panel,"Précédent",24,height-60,180,0);
    ext->following=report_button(panel,"Suivant",width-204,height-60,180,1);
    ext->counter=nyx_nova_text(panel,"",220,height-48,width-440,NOVA_SMALL);lv_label_set_align(ext->counter,LV_LABEL_ALIGN_CENTER);
    report_fill(panel);return panel;
}

static int property_rows(lv_obj_t *parent,const char *labels,const char *values,int width)
{
    int y=20;
    while(*labels){
        const char *label_end=strchr(labels,'\n'),*value_end=strchr(values,'\n');
        u32 ll=label_end?(u32)(label_end-labels):strlen(labels),vl=value_end?(u32)(value_end-values):strlen(values);
        char label[192],value[512];if(ll>=sizeof(label))ll=sizeof(label)-1;if(vl>=sizeof(value))vl=sizeof(value)-1;
        memcpy(label,labels,ll);label[ll]=0;memcpy(value,values,vl);value[vl]=0;
        int row=8;
        if(ll||vl){
            lv_obj_t *name=nyx_nova_text(parent,label,24,y,width*48/100-36,NOVA_BODY);lv_label_set_recolor(name,true);
            lv_obj_t *data=nyx_nova_text(parent,value,width*48/100,y,width*52/100-24,NOVA_BODY);lv_label_set_recolor(data,true);
            row=lv_obj_get_height(name)>lv_obj_get_height(data)?lv_obj_get_height(name):lv_obj_get_height(data);row+=7;
        }
        y+=row;if(!label_end)break;labels=label_end+1;values=value_end?value_end+1:values+strlen(values);
    }
    return y;
}
lv_obj_t *nyx_nova_emmc_summary(lv_obj_t *win,const char *values)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *tabs=lv_tabview_create(page,NULL);lv_obj_set_size(tabs,lv_obj_get_width(page),lv_obj_get_height(page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *device=lv_tabview_add_tab(tabs,"Informations"),*partitions=lv_tabview_add_tab(tabs,"Partitions");nyx_nova_fixed_page(device);nyx_nova_fixed_page(partitions);
    int height=lv_obj_get_height(device)-36;
    lv_obj_t *left=nyx_nova_panel(device,34,18,594,height),*right=nyx_nova_panel(device,652,18,594,height);
    const char *rest=values;for(u32 i=0;i<7&&*rest;++i){const char *next=strchr(rest,'\n');rest=next?next+1:rest+strlen(rest);}
    property_rows(left,"#00DDFF CID:#\nVendor ID:\nModel:\nProd Rev:\nS/N:\nMonth/Year:",values,594);
    property_rows(right,"#00DDFF Ext CSD:#\nCmd Classes:\nMax Bus Rate:\nCurrent Rate:\nEnhanced Area:\nWrite Cache:\n\nMaintenance:\nEstimated Life:\nReserved Used:",rest,594);
    return partitions;
}

void nyx_nova_transfer(lv_obj_t *win,emmc_tool_gui_t *context)
{
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);int height=lv_obj_get_height(page)-36;
    lv_obj_t *left=nyx_nova_panel(page,34,18,680,height),*right=nyx_nova_panel(page,738,18,508,height);
    context->label_info=nyx_nova_text(left,"",24,20,632,NOVA_BODY);lv_label_set_recolor(context->label_info,true);
    context->bar=lv_bar_create(left,NULL);lv_obj_set_size(context->bar,400,20);lv_obj_set_pos(context->bar,24,height-116);lv_bar_set_range(context->bar,0,100);
    context->label_pct=nyx_nova_text(left,"0%",438,height-116,210,NOVA_BODY);lv_label_set_recolor(context->label_pct,true);
    context->label_finish=nyx_nova_text(left,"",24,height-72,632,NOVA_BODY);lv_label_set_recolor(context->label_finish,true);
    lv_obj_t *log=lv_page_create(right,NULL);lv_obj_set_pos(log,20,20);lv_obj_set_size(log,468,height-40);
    lv_page_set_style(log,LV_PAGE_STYLE_BG,&lv_style_transp_tight);lv_page_set_style(log,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);lv_page_set_scrl_layout(log,LV_LAYOUT_OFF);lv_page_set_scrl_fit(log,false,true);
    context->label_log=lv_label_create(log,NULL);lv_label_set_style(context->label_log,&monospace_text);lv_label_set_recolor(context->label_log,true);lv_label_set_long_mode(context->label_log,LV_LABEL_LONG_BREAK);lv_obj_set_width(context->label_log,448);lv_obj_set_pos(context->label_log,0,0);
    static lv_style_t background,indicator,orange_bg,orange_ind,white;
    lv_style_copy(&background,lv_theme_get_current()->bar.bg);lv_style_copy(&indicator,lv_theme_get_current()->bar.indic);
    lv_style_copy(&orange_bg,&background);lv_style_copy(&orange_ind,&indicator);orange_ind.body.main_color=orange_ind.body.grad_color=LV_COLOR_HEX(0xF4A261);
    lv_style_copy(&white,&indicator);white.body.main_color=white.body.grad_color=LV_COLOR_HEX(0xF2F5FA);
    context->bar_teal_bg=&background;context->bar_teal_ind=&indicator;context->bar_orange_bg=&orange_bg;context->bar_orange_ind=&orange_ind;context->bar_white_bg=&background;context->bar_white_ind=&white;
}
