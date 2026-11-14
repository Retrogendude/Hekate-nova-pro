#include "gui_diagnostics.h"
#include "gui_nova.h"
#include "gui_choices.h"
#include "../diagnostics/component_summary.h"
#include <stdlib.h>
#include <string.h>

static lv_action_t component_report,component_export;
static lv_obj_t *component_holder,*component_counter,*component_previous,*component_next;
static u32 component_page;
static lv_style_t component_styles[4];

static lv_res_t component_leads(lv_obj_t *button)
{
    (void)button;char *summary=nyx_component_summary_text();
    if(!summary){nyx_show_help("Memoire insuffisante.");return LV_RES_OK;}
    lv_obj_t *win=nyx_create_standard_window("Bilan et pistes de panne",NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *report=nyx_nova_report(page,summary,34,18,1212,lv_obj_get_height(page)-36);
    nyx_nova_report_style(report,nyx_nova_text_style(NOVA_SMALL));free(summary);return LV_RES_OK;
}
static lv_res_t component_detail(lv_obj_t *button)
{
    u32 index=lv_obj_get_free_num(button);nyx_component_result result;
    if(!nyx_component_get(index,&result))return LV_RES_OK;
    char *summary=nyx_component_detail_text(index);if(!summary){nyx_show_help("Memoire insuffisante.");return LV_RES_OK;}
    lv_obj_t *win=nyx_create_standard_window(result.definition->name,NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    lv_obj_t *tabs=lv_tabview_create(page,NULL);lv_obj_set_size(tabs,1280,lv_obj_get_height(page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *facts=lv_tabview_add_tab(tabs,"Résultat et preuves");nyx_nova_fixed_page(facts);
    lv_obj_t *probes=lv_tabview_add_tab(tabs,"Rapports détaillés");nyx_nova_fixed_page(probes);
    lv_obj_t *report=nyx_nova_report(facts,summary,34,14,1212,lv_obj_get_height(facts)-28);
    nyx_nova_report_style(report,nyx_nova_text_style(NOVA_SMALL));free(summary);
    for(u32 i=0;i<result.definition->probe_count;++i){
        u32 probe_index=result.definition->probes[i];const nyx_diag_probe_t *probe=nyx_diag_probe(probe_index);
        if(!probe)continue;
        lv_obj_t *open=nyx_nova_action(probes,nyx_diag_probe_caption(probe_index),"Rapport enregistré", "file-text",component_report,probe_index,34+(i%3)*410,18+(i/3)*138,392,124);
        if(probe->state==NYX_DIAG_NOT_RUN||!probe->report_length){lv_obj_set_click(open,false);lv_btn_set_state(open,LV_BTN_STATE_INA);}
    }
    return LV_RES_OK;
}
static void component_fill(void)
{
    lv_obj_clean(component_holder);u32 ordered[32],count=0;
    for(int state=NYX_COMPONENT_TEST_ERROR;state>=0;--state)for(u32 i=0;i<nyx_component_count();++i){
        nyx_component_result result;nyx_component_get(i,&result);if(result.state==(u32)state&&count<32)ordered[count++]=i;
    }
    for(u32 slot=0;slot<4&&component_page*4+slot<count;++slot){
        u32 index=ordered[component_page*4+slot];nyx_component_result result;nyx_component_get(index,&result);
        char detail[512];s_printf(detail,"%s%s\n%s",nyx_component_state(result.state),result.partial?nyx_tr(" · couverture partielle"):"",result.measurement);
        lv_obj_t *open=nyx_nova_action(component_holder,result.definition->name,detail,"activity",component_detail,index,(slot%2)*618,(slot/2)*184,594,168);
        lv_style_copy(&component_styles[slot],lv_cont_get_style(lv_obj_get_parent(open)));
        u32 color=result.state>=NYX_COMPONENT_ANOMALY?0xFF6B6B:result.state==NYX_COMPONENT_WARN?0xF4B85F:result.state==NYX_COMPONENT_PASS?0x8AE6B6:0x667386;
        component_styles[slot].body.border.color=LV_COLOR_HEX(color);lv_cont_set_style(lv_obj_get_parent(open),&component_styles[slot]);
    }
    char position[64];s_printf(position,nyx_tr("Page %d / %d"),component_page+1,(count+3)/4);lv_label_set_text(component_counter,position);
    bool previous=component_page>0,next=(component_page+1)*4<count;
    lv_obj_set_click(component_previous,previous);lv_btn_set_state(component_previous,previous?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    lv_obj_set_click(component_next,next);lv_btn_set_state(component_next,next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t component_step(lv_obj_t *button)
{
    if(lv_obj_get_free_num(button)){if((component_page+1)*4<nyx_component_count())++component_page;}
    else if(component_page)--component_page;
    component_fill();return LV_RES_OK;
}
void nyx_components_window(lv_action_t report,lv_action_t export_action)
{
    component_report=report;component_export=export_action;component_page=0;
    lv_obj_t *win=nyx_create_standard_window("Bilan par composant",NULL);
    lv_obj_t *full=lv_win_add_btn(win,NULL,"Rapport",component_report);lv_obj_set_free_num(full,NYX_DIAG_PROBE_COUNT);
    lv_win_add_btn(win,NULL,"Exporter",component_export);lv_win_add_btn(win,NULL,"Pistes",component_leads);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    const char *guidance=nyx_diag_truncated()?"Rapport détaillé tronqué. PASS concerne uniquement les contrôles réellement réalisés.":"PASS : contrôles réalisés uniquement. Les contrôles partiels et le stress RAM restent distincts.";
    lv_obj_t *hint=nyx_nova_text(page,guidance,34,18,1212,NOVA_SMALL);nyx_nova_fit_text(hint,54);
    component_holder=lv_cont_create(page,NULL);lv_cont_set_fit(component_holder,false,false);lv_cont_set_layout(component_holder,LV_LAYOUT_OFF);lv_cont_set_style(component_holder,&lv_style_transp_tight);
    lv_obj_set_pos(component_holder,34,84);lv_obj_set_size(component_holder,1212,352);
    component_previous=lv_btn_create(page,NULL);lv_btn_set_fit(component_previous,false,false);lv_obj_set_size(component_previous,220,44);lv_obj_set_pos(component_previous,34,454);lv_obj_set_free_num(component_previous,0);lv_btn_set_action(component_previous,LV_BTN_ACTION_CLICK,component_step);
    lv_obj_t *label=lv_label_create(component_previous,NULL);lv_label_set_text(label,"Précédent");
    component_next=lv_btn_create(page,component_previous);lv_obj_set_pos(component_next,1026,454);lv_obj_set_free_num(component_next,1);label=lv_label_create(component_next,NULL);lv_label_set_text(label,"Suivant");
    lv_btn_set_action(component_next,LV_BTN_ACTION_CLICK,component_step);
    component_counter=nyx_nova_text(page,"",300,462,680,NOVA_SMALL);lv_label_set_align(component_counter,LV_LABEL_ALIGN_CENTER);component_fill();
}
