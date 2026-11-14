/* Native Nyx presentation for the supplied HWTest-RCM probes. GPL-2.0. */
#include "gui_diagnostics.h"
#include "../diagnostics/component_summary.h"
#include "../diagnostics/diag_format.h"
#include "gui_choices.h"
#include "gui_interactive.h"
#include "../diagnostics/nyx_diagnostics.h"
#include "../diagnostics/ram_handoff.h"
#include "../fonts/font_manager.h"
#include <stdlib.h>
#include <string.h>

#define ONE_PROBE (1u << 30)
#define REPORT_PAGE 12000u
static lv_obj_t *results_window, *report_label, *report_position;
static u32 report_start, report_length, report_page;
static nyx_ram_handoff_t last_ram;
static bool last_ram_valid;
static lv_res_t run_selection(lv_obj_t *btn);
static lv_res_t show_ram_service(lv_obj_t *btn);

static const struct {
    const char *title;
    const char *detail;
    u32 groups;
} groups[] = {
    {"Bilan HWTest", "Tous les groupes, hors stress RAM", NYX_DIAG_G_ALL},
    {"Identite", "SoC, fusibles et KFUSE", NYX_DIAG_G_IDENTITY},
    {"Alimentation", "Batterie, charge, tensions, thermique", NYX_DIAG_G_POWER},
    {"Memoire / horloges", "Identite DRAM et horloges", NYX_DIAG_G_MEMORY},
    {"Stockage", "SD, eMMC, partitions et lecteur", NYX_DIAG_G_STORAGE},
    {"Sans fil", "Bluetooth et Wi-Fi / PCIe", NYX_DIAG_G_WIRELESS},
    {"Audio", "Codec et horloges audio", NYX_DIAG_G_AUDIO},
    {"Test sonore", "Haut-parleurs et prise casque", NYX_DIAG_G_SOUND},
    {"Ecran", "Dalle et retroeclairage LCD / OLED", NYX_DIAG_G_DISPLAY},
    {"Commandes", "Tactile, Joy-Con, boutons, lumiere", NYX_DIAG_G_INPUTS},
    {"Etat bas niveau", "GPIO, UART et etat de demarrage", NYX_DIAG_G_RAW},
    {"Test RAM isole", "Passe complete puis retour Hekate", 0},
};

static lv_obj_t *text(lv_obj_t *parent, const char *value, lv_coord_t x,
    lv_coord_t y, lv_coord_t width, bool small)
{
    lv_obj_t *label = lv_label_create(parent, NULL);
    lv_label_set_style(label, small ? &hint_small_style : lv_theme_get_current()->label.prim);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, width);
    lv_label_set_text(label, value);
    lv_obj_set_pos(label, x, y);
    return label;
}

static lv_obj_t *button(lv_obj_t *parent, const char *title, lv_coord_t x,
    lv_coord_t y, lv_coord_t width, lv_action_t action, u32 id)
{
    lv_obj_t *btn = lv_btn_create(parent, NULL);
    lv_btn_set_fit(btn, false, false);
    lv_obj_set_size(btn, width, 64);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_free_num(btn, id);
    lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);
    lv_obj_t *label = lv_label_create(btn, NULL);
    lv_label_set_text(label, title);
    return btn;
}

static void notice(const char *title, const char *message)
{
    lv_obj_t *win = nyx_create_standard_window(title, NULL);
    text(win, message, 32, 28, 1180, false);
}

static const char *state_name(nyx_diag_state_t state)
{
    switch (state) {
    case NYX_DIAG_PASS: return nyx_tr("OK");
    case NYX_DIAG_WARN: return nyx_tr("AVERTISSEMENT");
    case NYX_DIAG_FAIL: return nyx_tr("DEFAUT");
    case NYX_DIAG_ERROR: return nyx_tr("ERREUR");
    case NYX_DIAG_OBSERVATION: return nyx_tr("OBSERVATION");
    default: return nyx_tr("NON EXECUTE");
    }
}

static void render_report_page(void)
{
    const char *report = nyx_diag_report();
    u32 offset = report_page * REPORT_PAGE;
    u32 end = offset + REPORT_PAGE;
    if (end > report_length) end = report_length;
    /* The report now contains UTF-8. Adjacent pages meet after a complete
     * codepoint, rather than dropping or splitting a Cyrillic/accent byte. */
    while (report && offset < report_length && ((u8)report[report_start + offset] & 0xC0) == 0x80) ++offset;
    while (report && end < report_length && ((u8)report[report_start + end] & 0xC0) == 0x80) ++end;
    u32 count = end > offset ? end - offset : 0;
    char *page = malloc(count + 1);
    if (!page) { lv_label_set_text(report_label, "Memoire insuffisante."); return; }
    if (count && report) memcpy(page, report + report_start + offset, count);
    page[count] = 0;
    if (count)
        lv_label_set_text(report_label, page);
    else
        lv_label_set_text(report_label, "Aucun rapport pour cette selection.");
    free(page);
    char position[80];
    s_printf(position, nyx_tr("Page %d / %d"), report_page + 1,
        report_length ? (report_length + REPORT_PAGE - 1) / REPORT_PAGE : 1);
    lv_label_set_text(report_position, position);
}

static lv_res_t report_step(lv_obj_t *btn)
{
    if (lv_obj_get_free_num(btn)) {
        if ((report_page + 1) * REPORT_PAGE < report_length) report_page++;
    } else if (report_page) report_page--;
    render_report_page();
    return LV_RES_OK;
}

static lv_res_t show_report(lv_obj_t *btn)
{
    u32 index = btn ? lv_obj_get_free_num(btn) : NYX_DIAG_PROBE_COUNT;
    const nyx_diag_probe_t *probe = index < NYX_DIAG_PROBE_COUNT ? nyx_diag_probe(index) : NULL;
    const char *report = nyx_diag_report();
    report_start = probe ? probe->report_offset : 0;
    report_length = probe ? probe->report_length : (report ? strlen(report) : 0);
    report_page = 0;
    lv_obj_t *win = nyx_create_standard_window(probe ? probe->name : "Rapport HWTest", NULL);
    lv_obj_t *prev = lv_win_add_btn(win, NULL, "Precedent", report_step);
    lv_obj_set_free_num(prev, 0);
    lv_obj_t *next = lv_win_add_btn(win, NULL, "Suivant", report_step);
    lv_obj_set_free_num(next, 1);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    report_position=text(page,"",34,12,1212,true);
    lv_obj_t *body=lv_page_create(page,NULL);lv_obj_set_pos(body,34,56);lv_obj_set_size(body,1212,lv_obj_get_height(page)-74);
    lv_page_set_style(body,LV_PAGE_STYLE_BG,&lv_style_transp_tight);lv_page_set_style(body,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
    lv_page_set_scrl_fit(body,false,true);lv_page_set_scrl_width(body,1212);lv_page_set_sb_mode(body,LV_SB_MODE_AUTO);
    report_label=text(body,"",12,8,1176,true);lv_label_set_style(report_label,&monospace_text);
    render_report_page();
    return LV_RES_OK;
}

static lv_res_t export_report(lv_obj_t *btn)
{
    (void)btn;
    char path[128] = {0}, message[1024];
    int result = nyx_diag_export(path);
    if (result) s_printf(message, nyx_tr("Echec de l'export du rapport (code %d).\nVerifiez la carte SD."), result);
    else s_printf(message, nyx_tr("Rapport enregistre :\n%s"), path);
    notice("Exporter le rapport", message);
    return LV_RES_OK;
}

static lv_res_t close_results(lv_obj_t *btn)
{
    results_window = NULL;
    return nyx_win_close_action(btn);
}

/* Stable pages of native Nova cards replace the long vertical probe lists. */
static const char *const diagnostic_titles[NYX_DIAG_PROBE_COUNT]={
    "Bilan général","SoC","Fusibles","KFUSE","PMIC","Régulateur CPU","Entrées/sorties PMIC","Alimentations","Tension 5 V","Batterie","Chargeur","USB-C Power Delivery","Températures","Ventilateur",
    "Mémoire DRAM","Horloges","Stockage","Numéros de série","Partitions","Santé eMMC","Table GPT","Démarrage BOOT0","AutoRCM","Contenu de la carte SD","Erreurs de stockage","Lecteur de jeux",
    "Bluetooth","Wi-Fi PCIe","Connexion Wi-Fi","Codec audio","Horloges audio","Test sonore","Dalle écran","Rétroéclairage","Écran tactile","Capteur de luminosité","Joy-Con","Boutons","GPIO","UART B","État de réinitialisation","Registres PMC"
};
const char *nyx_diag_probe_caption(u32 index)
{
    return index<NYX_DIAG_PROBE_COUNT?nyx_tr(diagnostic_titles[index]):nyx_tr("Diagnostic");
}

typedef struct {
    lv_cont_ext_t container;
    u32 indices[NYX_DIAG_PROBE_COUNT],count,page;
    bool results;
    lv_obj_t *tiles,*position,*previous,*next;
} diag_tiles_ext;

static void diagnostic_tiles_fill(lv_obj_t *holder)
{
    diag_tiles_ext *ext=lv_obj_get_ext_attr(holder);
    lv_obj_clean(ext->tiles);
    int width=(lv_obj_get_width(holder)-24)/3,height=(lv_obj_get_height(ext->tiles)-24)/3;
    for(u32 slot=0;slot<9&&ext->page*9+slot<ext->count;++slot){
        u32 index=ext->indices[ext->page*9+slot];const nyx_diag_probe_t *probe=nyx_diag_probe(index);
        char detail[192];
        if(ext->results)s_printf(detail,"[%s] %s",state_name(probe->state),nyx_tr(probe->name));
        else s_printf(detail,"%s",nyx_tr(probe->name));
        nyx_nova_action(ext->tiles,diagnostic_titles[index],detail,"activity",ext->results?show_report:run_selection,
            ext->results?index:ONE_PROBE|index,slot%3*(width+12),slot/3*(height+12),width,height);
    }
    char position[64];s_printf(position,nyx_tr("Page %d / %d"),ext->page+1,ext->count?(ext->count+8)/9:1);lv_label_set_text(ext->position,position);
    lv_obj_set_click(ext->previous,ext->page>0);lv_btn_set_state(ext->previous,ext->page?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
    bool next=(ext->page+1)*9<ext->count;lv_obj_set_click(ext->next,next);lv_btn_set_state(ext->next,next?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t diagnostic_tiles_step(lv_obj_t *button)
{
    lv_obj_t *holder=lv_obj_get_free_ptr(button);diag_tiles_ext *ext=lv_obj_get_ext_attr(holder);
    if(lv_obj_get_free_num(button)){if((ext->page+1)*9<ext->count)++ext->page;}else if(ext->page)--ext->page;
    diagnostic_tiles_fill(holder);return LV_RES_OK;
}
static void diagnostic_tiles(lv_obj_t *win,u32 group,bool results)
{
    lv_win_ext_t *window=lv_obj_get_ext_attr(win);
    lv_obj_t *holder=lv_cont_create(win,NULL);lv_cont_set_fit(holder,false,false);lv_cont_set_layout(holder,LV_LAYOUT_OFF);lv_cont_set_style(holder,&lv_style_transp_tight);
    lv_obj_set_pos(holder,34,108);lv_obj_set_size(holder,1212,lv_obj_get_height(window->page)-122);
    diag_tiles_ext *ext=lv_obj_allocate_ext_attr(holder,sizeof(*ext));if(!ext){lv_obj_del(holder);return;}
    ext->count=ext->page=0;ext->results=results;
    for(u32 i=results?0:1;i<NYX_DIAG_PROBE_COUNT;++i){const nyx_diag_probe_t *probe=nyx_diag_probe(i);if(probe&&(results?probe->state!=NYX_DIAG_NOT_RUN:!!(probe->group&group)))ext->indices[ext->count++]=i;}
    ext->tiles=lv_cont_create(holder,NULL);lv_cont_set_fit(ext->tiles,false,false);lv_cont_set_layout(ext->tiles,LV_LAYOUT_OFF);lv_cont_set_style(ext->tiles,&lv_style_transp_tight);
    lv_obj_set_size(ext->tiles,1212,lv_obj_get_height(holder)-58);lv_obj_set_pos(ext->tiles,0,0);
    int y=lv_obj_get_height(holder)-46;
    ext->previous=button(holder,"Precedent",0,y,200,diagnostic_tiles_step,0);
    ext->next=button(holder,"Suivant",1012,y,200,diagnostic_tiles_step,1);
    lv_obj_set_height(ext->previous,44);lv_obj_set_height(ext->next,44);
    lv_obj_set_free_ptr(ext->previous,holder);lv_obj_set_free_ptr(ext->next,holder);
    ext->position=nyx_nova_text(holder,"",300,y+12,612,NOVA_SMALL);lv_label_set_align(ext->position,LV_LABEL_ALIGN_CENTER);
    diagnostic_tiles_fill(holder);
}

static lv_res_t show_components(lv_obj_t *button)
{
    (void)button;nyx_components_window(show_report,export_report);return LV_RES_OK;
}
static void show_results(int result)
{
    results_window = nyx_create_standard_window("Diagnostic - Derniere execution", close_results);
    lv_obj_t *full = lv_win_add_btn(results_window, NULL, "Rapport", show_report);
    lv_obj_set_free_num(full, NYX_DIAG_PROBE_COUNT);
    lv_win_add_btn(results_window, NULL, "Exporter", export_report);
    lv_win_add_btn(results_window, NULL, "Bilan par composant", show_components);
    u32 counts[6] = {0};
    for (u32 i = 0; i < NYX_DIAG_PROBE_COUNT; i++) {
        const nyx_diag_probe_t *p = nyx_diag_probe(i);
        if (p && p->state <= NYX_DIAG_ERROR) counts[p->state]++;
    }
    char summary[2048];
    s_printf(summary, nyx_tr("%d OK  /  %d avert.  /  %d defaut(s)  /  %d erreur(s)  /  %d observation(s)\n%s%s"),
        counts[NYX_DIAG_PASS], counts[NYX_DIAG_WARN], counts[NYX_DIAG_FAIL],
        counts[NYX_DIAG_ERROR], counts[NYX_DIAG_OBSERVATION],
        result ? nyx_tr("Execution ou restauration incomplete. ") : "",
        nyx_diag_truncated() ? nyx_tr("Rapport tronque : consultez le marqueur de fin.") :
        nyx_tr("Les observations sans verdict ne constituent pas un test reussi."));
    lv_obj_t *summary_label=text(results_window, summary, 34, 16, 1212, true);
    nyx_nova_fit_text(summary_label,80);
    diagnostic_tiles(results_window,0,true);
    if (!gui_diagnostics_inputs_ready())
        notice("Commandes tactiles", "La restauration du tactile a echoue.\nUtilisez les Joy-Con ou rechargez Nyx.");
}

typedef struct { lv_obj_t *label; lv_obj_t *bar; } progress_t;
static void suspend_probes(u32 selected, void *context)
{
    (void)context;
    gui_diagnostics_suspend(selected);
}
static void resume_probes(u32 selected, void *context)
{
    (void)context;
    gui_diagnostics_resume(selected);
}
static void progress(u32 current, u32 total, const char *message, void *context)
{
    progress_t *p = context;
    char value[1024];
    s_printf(value, "%d / %d\n%s", current, total, nyx_tr(message ? message : "Diagnostic"));
    lv_label_set_text(p->label, value);
    lv_bar_set_value(p->bar, total ? current * 100 / total : 0);
    lv_refr_now();
}

static lv_res_t run_selection(lv_obj_t *btn)
{
    u32 selection = lv_obj_get_free_num(btn);
    if (nyx_diag_running()) return LV_RES_OK;
    if (results_window) { lv_obj_del(results_window); results_window = NULL; }
    lv_obj_t *win = nyx_create_standard_window("Diagnostic en cours", NULL);
    nyx_window_toggle_buttons(win, true);
    progress_t p;
    p.label = text(win, "Preparation des controles...", 50, 90, 1100, false);
    p.bar = lv_bar_create(win, NULL);
    lv_obj_set_size(p.bar, 1100, 24);
    lv_obj_set_pos(p.bar, 50, 210);
    lv_bar_set_range(p.bar, 0, 100);
    text(win, "Les commandes sont suspendues pendant les mesures materielles.", 50, 280, 1100, true);
    lv_refr_now();
    nyx_diag_hooks_t hooks = { suspend_probes, progress, resume_probes, &p };
    int result = selection & ONE_PROBE ? nyx_diag_run_probe(selection & ~ONE_PROBE, &hooks) : nyx_diag_run(selection, &hooks);
    lv_obj_del(win);
    if(result!=0&&result!=4){notice("Diagnostic", "La nouvelle exécution n'a pas produit de résultats. Le rapport précédent reste conservé.");return LV_RES_OK;}
    show_results(result);
    if(selection==NYX_DIAG_G_ALL)nyx_components_window(show_report,export_report);
    if(result==4)nyx_show_help("Mesures capturées, mais restauration matérielle incomplète. Recharger Nyx avant de démarrer.");
    return LV_RES_OK;
}

static lv_res_t show_group(lv_obj_t *btn)
{
    u32 index = lv_obj_get_free_num(btn);
    if (index >= ARRAY_SIZE(groups)) return LV_RES_OK;
    if (!groups[index].groups) return show_ram_service(btn);
    if (!index) {
        lv_obj_set_free_num(btn, NYX_DIAG_G_ALL);
        lv_res_t result = run_selection(btn);
        lv_obj_set_free_num(btn, 0);
        return result;
    }
    lv_obj_t *win = nyx_create_standard_window(groups[index].title, NULL);
    if(index==9){lv_win_add_btn(win,NULL,"Joy-Con interactif",nyx_joycon_test);lv_win_add_btn(win,NULL,"Grille tactile",nyx_touch_test);}
    if(index==8)lv_win_add_btn(win,NULL,"Mires écran",nyx_screen_patterns);
    button(win, "Executer ce groupe", 32, 20, 416, run_selection, groups[index].groups);
    text(win, groups[index].groups == NYX_DIAG_G_SOUND ?
        "Un son est emis sur chaque sortie. Verifiez le resultat a l'ecoute." :
        groups[index].detail, 480, 32, 720, true);
    diagnostic_tiles(win,groups[index].groups,false);
    return LV_RES_OK;
}

static const char *ram_state(u32 state)
{
    if (state == NYX_RAM_RESULT_PASS) return nyx_tr("OK");
    if (state == NYX_RAM_RESULT_FAIL) return nyx_tr("DEFAUT");
    if (state == NYX_RAM_RESULT_ABORT) return nyx_tr("INTERROMPU");
    return nyx_tr("ERREUR");
}

static u32 ram_selected_mode=NYX_RAM_MODE_ADVANCED,ram_custom_mask=NYX_RAM_PATTERN_ALL;
static lv_obj_t *ram_mode_choices,*ram_pattern_choices,*ram_start;

static lv_res_t launch_ram(lv_obj_t *btn)
{
    (void)btn;
    u32 mask=ram_selected_mode==NYX_RAM_MODE_QUICK?1u:ram_selected_mode==NYX_RAM_MODE_ADVANCED?NYX_RAM_PATTERN_ALL:ram_custom_mask;
    int result = nyx_launch_ram_service(ram_selected_mode,mask);
    char message[1024];
    s_printf(message, nyx_tr("Le service RAM n'a pas pu etre lance (code %d).\nVerifiez bootloader/sys/hwtest_ram.bin et bootloader/update.bin."), result);
    notice("Test RAM", message);
    return LV_RES_OK;
}

static void ram_controls_refresh(void)
{
    bool custom=ram_selected_mode==NYX_RAM_MODE_CUSTOM;
    for(lv_obj_t *choice=lv_obj_get_child(ram_pattern_choices,NULL);choice;choice=lv_obj_get_child(ram_pattern_choices,choice)){
        u32 index=lv_obj_get_free_num(choice);bool selected=custom?!!(ram_custom_mask&(1u<<index)):ram_selected_mode==NYX_RAM_MODE_ADVANCED||index==0;
        lv_obj_set_click(choice,custom);lv_btn_set_state(choice,selected?LV_BTN_STATE_TGL_REL:LV_BTN_STATE_REL);
    }
    bool valid=!custom||ram_custom_mask;
    lv_obj_set_click(ram_start,valid);lv_btn_set_state(ram_start,valid?LV_BTN_STATE_REL:LV_BTN_STATE_INA);
}
static lv_res_t ram_mode_choice(lv_obj_t *choice)
{
    ram_selected_mode=lv_obj_get_free_num(choice);ram_controls_refresh();return LV_RES_OK;
}
static lv_res_t ram_pattern_choice(lv_obj_t *choice)
{
    ram_custom_mask^=1u<<lv_obj_get_free_num(choice);ram_controls_refresh();return LV_RES_OK;
}

static lv_res_t show_ram_service(lv_obj_t *btn)
{
    (void)btn;
    lv_obj_t *win=nyx_create_standard_window("Test RAM isole",NULL);
    lv_obj_t *page=((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page;nyx_nova_fixed_page(page);
    ram_start=lv_win_add_btn(win,NULL,"Lancer le test RAM",launch_ram);
    ram_mode_choices=nyx_choices_create(page,"Rapide\nAvancé\nPersonnalisé",ram_selected_mode,3,1212,48,ram_mode_choice,80,false);
    lv_obj_set_pos(ram_mode_choices,34,18);
    lv_obj_t *label=nyx_nova_text(page,"Rapide : adresse. Avancé : 7 motifs. Personnalisé : choisir au moins un motif.",34,82,1212,NOVA_SMALL);nyx_nova_fit_text(label,48);
    ram_pattern_choices=nyx_choices_create(page,"Adresse\nAdresse inversée\nZéros\nUns\nAlternance AA\nAlternance 55\nPseudo-aléatoire",0,4,1212,54,ram_pattern_choice,81,false);
    lv_obj_set_pos(ram_pattern_choices,34,142);
    label=nyx_nova_text(page,"Programme séparé puis retour automatique à Hekate. Les zones réservées au retour restent exclues. Un matériel instable peut empêcher le retour.",34,278,1212,NOVA_SMALL);nyx_nova_fit_text(label,64);
    if(last_ram_valid){
        char message[1024];
        const char *mode=last_ram.mode==NYX_RAM_MODE_QUICK?"Rapide":last_ram.mode==NYX_RAM_MODE_ADVANCED?"Avancé":"Personnalisé";
        s_printf(message,"%s · %s · %s\n%d / %d motifs · zone prévue %d / %d MiB · %d s · %d erreur(s)\nMotifs choisis : %02X · terminés : %02X · MTC : %08X\nZone relue avec au moins un motif : %d MiB",
            nyx_tr("Dernier resultat"),ram_state(last_ram.result),nyx_tr(mode),last_ram.patterns_completed,last_ram.patterns_total,last_ram.tested_mib,last_ram.dram_mib,last_ram.elapsed_ms/1000,last_ram.errors,last_ram.selected_mask,last_ram.completed_mask,last_ram.mtc_errors,last_ram.verified_mib);
        label=nyx_nova_text(page,message,34,362,1212,NOVA_SMALL);nyx_nova_fit_text(label,126);
    }
    ram_controls_refresh();return LV_RES_OK;
}

static void notify_ram_result(void *context)
{
    (void)context;
    show_ram_service(NULL);
}

void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent)
{
    (void)th;
    nyx_nova_grid_begin(parent, NOVA_DIAGNOSTICS);
    for (u32 i = 0; i < ARRAY_SIZE(groups); i++)
        nyx_nova_grid_add(parent, groups[i].detail, show_group, NULL, i);
    nyx_nova_grid_finish(parent);
    nyx_ram_handoff_t *saved = (nyx_ram_handoff_t *)NYX_RAM_HANDOFF_ADDR;
    if (nyx_ram_result_valid(saved)) {
        memcpy(&last_ram, saved, sizeof(last_ram));
        last_ram_valid = true;
        char record[640];
        nyx_diag_snprintf(record,sizeof(record),"RAM isolée : état=%s, mode=%d, sélection=0x%02X, achevés=0x%02X, motifs=%d/%d, zone prévue=%d MiB, préfixe relu=%d MiB, DRAM=%d MiB, durée=%d ms, erreurs=%d, erreurs MTC=%d. Les 4 premiers MiB sont réservés. Une interruption ne valide pas les motifs inachevés.",
            ram_state(last_ram.result),last_ram.mode,last_ram.selected_mask,last_ram.completed_mask,last_ram.patterns_completed,last_ram.patterns_total,last_ram.tested_mib,last_ram.verified_mib,last_ram.dram_mib,last_ram.elapsed_ms,last_ram.errors,last_ram.mtc_errors);
        nyx_component_observation(NYX_MANUAL_RAM,record,last_ram.errors||last_ram.mtc_errors);
        nyx_nova_diagnostic_result();
        saved->result_magic = 0;
        lv_task_t *task = lv_task_create(notify_ram_result, 600, LV_TASK_PRIO_MID, NULL);
        lv_task_once(task);
    }
}
