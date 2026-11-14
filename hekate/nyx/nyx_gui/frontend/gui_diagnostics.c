/* Native Nyx presentation for the supplied HWTest-RCM probes. GPL-2.0. */
#include "gui_diagnostics.h"
#include "../diagnostics/nyx_diagnostics.h"
#include "../diagnostics/ram_handoff.h"
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
    u32 count = report_length > offset ? report_length - offset : 0;
    if (count > REPORT_PAGE) count = REPORT_PAGE;
    char *page = malloc(count + 1);
    if (!page) { lv_label_set_text(report_label, "Memoire insuffisante."); return; }
    if (count && report) memcpy(page, report + report_start + offset, count);
    page[count] = 0;
    if (count)
        (lv_label_set_text)(report_label, page);
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
    report_position = text(win, "", 28, 12, 1100, true);
    report_label = text(win, "", 28, 56, 1180, true);
    lv_label_set_style(report_label, &monospace_text);
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

static void show_results(int result)
{
    results_window = nyx_create_standard_window("Diagnostic - Derniere execution", close_results);
    lv_obj_t *full = lv_win_add_btn(results_window, NULL, "Rapport", show_report);
    lv_obj_set_free_num(full, NYX_DIAG_PROBE_COUNT);
    lv_win_add_btn(results_window, NULL, "Exporter", export_report);
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
    text(results_window, summary, 32, 16, 1180, true);
    lv_obj_t *list = lv_list_create(results_window, NULL);
    lv_obj_set_size(list, 1180, 440);
    lv_obj_set_pos(list, 32, 108);
    for (u32 i = 0; i < NYX_DIAG_PROBE_COUNT; i++) {
        const nyx_diag_probe_t *p = nyx_diag_probe(i);
        if (!p || p->state == NYX_DIAG_NOT_RUN) continue;
        char row[512];
        s_printf(row, "[%s] %s / %s", state_name(p->state), nyx_tr(p->name), p->function);
        lv_obj_t *entry = lv_list_add(list, NULL, row, show_report);
        lv_obj_set_free_num(entry, i);
        lv_label_set_style(lv_list_get_btn_label(entry), &hint_small_style);
    }
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
    show_results(result);
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
    button(win, "Executer ce groupe", 32, 20, 416, run_selection, groups[index].groups);
    text(win, groups[index].groups == NYX_DIAG_G_SOUND ?
        "Un son est emis sur chaque sortie. Verifiez le resultat a l'ecoute." :
        groups[index].detail, 480, 32, 720, true);
    lv_obj_t *list = lv_list_create(win, NULL);
    lv_obj_set_size(list, 1180, 420);
    lv_obj_set_pos(list, 32, 110);
    for (u32 i = 1; i < NYX_DIAG_PROBE_COUNT; i++) {
        const nyx_diag_probe_t *p = nyx_diag_probe(i);
        if (p && (p->group & groups[index].groups)) {
            char label[512];
            s_printf(label, "%s / %s", nyx_tr(p->name), p->function);
            lv_obj_t *entry = lv_list_add(list, NULL, label, run_selection);
            lv_obj_set_free_num(entry, ONE_PROBE | i);
            lv_label_set_style(lv_list_get_btn_label(entry), &hint_small_style);
        }
    }
    return LV_RES_OK;
}

static const char *ram_state(u32 state)
{
    if (state == NYX_RAM_RESULT_PASS) return nyx_tr("OK");
    if (state == NYX_RAM_RESULT_FAIL) return nyx_tr("DEFAUT");
    if (state == NYX_RAM_RESULT_ABORT) return nyx_tr("INTERROMPU");
    return nyx_tr("ERREUR");
}

static lv_res_t launch_ram(lv_obj_t *btn)
{
    (void)btn;
    int result = nyx_launch_ram_service();
    char message[1024];
    s_printf(message, nyx_tr("Le service RAM n'a pas pu etre lance (code %d).\nVerifiez bootloader/sys/hwtest_ram.bin et bootloader/update.bin."), result);
    notice("Test RAM", message);
    return LV_RES_OK;
}

static lv_res_t show_ram_service(lv_obj_t *btn)
{
    (void)btn;
    lv_obj_t *win = nyx_create_standard_window("Test RAM isole", NULL);
    text(win, "Passe complete et retour automatique a Hekate", 32, 28, 1180, false);
    text(win,
        "Nyx sera quitte pour tester la memoire disponible avec les 7 motifs avances.\n"
        "La zone de retour Hekate et les zones reservees restent exclues du test.\n"
        "A la fin, Hekate est recharge et le resultat est affiche.\n"
        "En cas de panne materielle ou de memoire instable, le retour peut echouer.",
        32, 94, 1180, true);
    button(win, "Lancer le test RAM", 32, 250, 480, launch_ram, 0);
    if (last_ram_valid) {
        char message[1024];
        s_printf(message, nyx_tr("Dernier resultat : %s\n%d / %d motifs, zone prevue : %d MiB sur %d MiB\n%d erreur(s), duree %d s, erreurs MTC : %08X"),
            ram_state(last_ram.result), last_ram.patterns_completed, last_ram.patterns_total,
            last_ram.tested_mib, last_ram.dram_mib, last_ram.errors,
            last_ram.elapsed_ms / 1000, last_ram.mtc_errors);
        text(win, message, 32, 352, 1180, true);
    }
    return LV_RES_OK;
}

static void notify_ram_result(void *context)
{
    (void)context;
    show_ram_service(NULL);
}

void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent)
{
    (void)th;
    static lv_style_t page;
    lv_style_copy(&page, &lv_style_transp);
    page.body.padding.hor = page.body.padding.ver = 0;
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &page);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &page);
    lv_page_set_scrl_layout(parent, LV_LAYOUT_OFF);
    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_width(parent, LV_HOR_RES);
    lv_page_set_scrl_height(parent, 592);
    text(parent, "Diagnostic materiel", 32, 16, 1180, false);
    text(parent, "HWTest-RCM : 42 controles natifs et un service RAM isole. Choisissez une categorie.", 32, 58, 1180, true);
    for (u32 i = 0; i < ARRAY_SIZE(groups); i++) {
        lv_coord_t x = 32 + (i % 3) * 400, y = 108 + (i / 3) * 116;
        lv_obj_t *tile = button(parent, "", x, y, 384, show_group, i);
        lv_btn_set_layout(tile, LV_LAYOUT_OFF);
        lv_obj_set_height(tile, 100);
        text(tile, groups[i].title, 20, 14, 344, false);
        text(tile, groups[i].detail, 20, 57, 344, true);
    }
    nyx_ram_handoff_t *saved = (nyx_ram_handoff_t *)NYX_RAM_HANDOFF_ADDR;
    if (saved->result_magic == NYX_RAM_RESULT_MAGIC && saved->version == NYX_RAM_VERSION &&
        saved->result >= NYX_RAM_RESULT_PASS && saved->result <= NYX_RAM_RESULT_ERROR &&
        saved->result_hash == nyx_ram_hash(&saved->result, 8 * sizeof(u32))) {
        memcpy(&last_ram, saved, sizeof(last_ram));
        last_ram_valid = true;
        saved->result_magic = 0;
        lv_task_t *task = lv_task_create(notify_ram_result, 600, LV_TASK_PRIO_MID, NULL);
        lv_task_once(task);
    }
}
