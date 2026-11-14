/* Native Nyx front end for the first HWTest-RCM probe subset. GPL-2.0. */
#include "gui_diagnostics.h"
#include "diagnostics/diagnostic_bridge.h"
#include <stdlib.h>

/* Probes run synchronously; LVGL copies the completed report into its label.
 * No UART ownership, framebuffer replacement or payload handoff is involved. */
static char *report;
static u32 used;
static u32 warnings, failures;
#define REPORT_SIZE (16 * 1024)


void diag_log(u32 color, const char *fmt, ...)
{
    (void)color;
    if (!report) return;
    /* The imported, fixed probe formats produce lines shorter than 512 bytes. */
    char line[512];
    va_list ap;
    va_start(ap, fmt);
    s_vprintf(line, fmt, ap);
    va_end(ap);
    u32 n = strlen(line);
    if (n > REPORT_SIZE - used - 1) n = REPORT_SIZE - used - 1;
    memcpy(report + used, line, n);
    used += n;
    report[used] = 0;
}

void dx_set(const char *key, dx_sev_t sev, const char *fmt, ...)
{
    /* Every key in this subset is emitted exactly once per run. */
    if (sev == DX_WARN) warnings++;
    if (sev == DX_FAIL) failures++;
    diag_log(COL_DEFAULT, "  [%s] %s\n", key,
        sev == DX_FAIL ? "FAIL" : sev == DX_WARN ? "WARN" : "PASS");
    if (fmt && *fmt) {
        char detail[256];
        va_list ap;
        va_start(ap, fmt);
        s_vprintf(detail, fmt, ap);
        va_end(ap);
        diag_log(COL_DEFAULT, "    %s\n", detail);
    }
}

static lv_res_t run_diagnostic(lv_obj_t *btn)
{
    u32 group = lv_obj_get_free_num(btn);
    report = malloc(REPORT_SIZE);
    if (!report) {
        lv_obj_t *win = nyx_create_standard_window("Diagnostic", NULL);
        lv_obj_t *label = lv_label_create(win, NULL);
        lv_label_set_text(label, "Memoire insuffisante. Fermez cette fenetre puis reessayez.");
        return LV_RES_OK;
    }
    used = warnings = failures = 0;
    report[0] = 0;
    diag_log(COL_INFO, "HWTest-RCM 0.15 / Nyx - preliminary integration\n\n");
    if (group == 0 || group == 1) {
        probe_display();
        probe_backlight();
    }
    if (group == 0 || group == 2) probe_dram();
    diag_log(COL_INFO, "\nFindings: %d failure(s), %d warning(s).\n", failures, warnings);
    diag_log(COL_DEFAULT, "Observations only: no full RAM stress test or panel image test.\n");
    lv_obj_t *win = nyx_create_standard_window("Diagnostic - Resultats", NULL);
    lv_obj_t *summary = lv_label_create(win, NULL);
    lv_label_set_recolor(summary, true);
    char status[160];
    s_printf(status, "%s%d echec(s)#    %s%d avertissement(s)#",
        failures ? "#FF7070 " : "#96FF00 ", failures,
        warnings ? "#FFDD00 " : "#C0D0E0 ", warnings);
    lv_label_set_text(summary, status);
    lv_obj_set_pos(summary, 20, 12);
    lv_obj_t *label = lv_label_create(win, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, LV_HOR_RES - 80);
    lv_label_set_style(label, &monospace_text);
    lv_label_set_text(label, report);
    lv_obj_set_pos(label, 20, 64);
    free(report);
    report = NULL;
    return LV_RES_OK;
}

void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent)
{
    /* Match the native Tools pages and retain the user's Nyx theme. */
    static lv_style_t page_style;
    lv_style_copy(&page_style, &lv_style_transp);
    page_style.body.padding.hor = 0;
    page_style.body.padding.ver = 0;
    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &page_style);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &page_style);
    lv_page_set_scrl_layout(parent, LV_LAYOUT_OFF);
    lv_page_set_scrl_fit(parent, false, false);
    lv_page_set_scrl_width(parent, LV_HOR_RES);
    lv_page_set_scrl_height(parent, 592);

    lv_obj_t *intro = lv_label_create(parent, NULL);
    lv_label_set_style(intro, th->label.prim);
    lv_label_set_text(intro, "Diagnostic materiel");
    lv_obj_set_pos(intro, 50, 20);

    lv_obj_t *hint = lv_label_create(parent, NULL);
    lv_label_set_style(hint, &hint_small_style);
    lv_label_set_text(hint, "Choisissez un controle. Fermez les resultats pour revenir a cet ecran.");
    lv_obj_set_pos(hint, 50, 65);

    static const lv_point_t separator_points[] = { {0, 0}, {1180, 0} };
    lv_obj_t *separator = lv_line_create(parent, NULL);
    lv_line_set_points(separator, separator_points, 2);
    lv_line_set_style(separator, th->line.decor);
    lv_obj_set_pos(separator, 50, 103);

    static const char *names[] = { "Bilan disponible", "Ecran", "Memoire DRAM" };
    static const char *descriptions[] = {
        "Execute les controles de l'ecran et de la memoire DRAM.\n"
        "Affiche un bilan des controles actuellement integres.",
        "Identifie la dalle et consulte l'etat du retroeclairage.\n"
        "Prend en compte les ecrans LCD et OLED.",
        "Consulte le fabricant et la densite de la memoire.\n"
        "Compare les informations des canaux DRAM."
    };
    for (u32 i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(parent, NULL);
        lv_btn_set_style(btn, LV_BTN_STYLE_REL,
            hekate_bg ? &btn_transp_rel : th->btn.rel);
        lv_btn_set_style(btn, LV_BTN_STYLE_PR,
            hekate_bg ? &btn_transp_pr : th->btn.pr);
        lv_btn_set_fit(btn, false, false);
        lv_obj_set_size(btn, 320, 64);
        lv_obj_set_pos(btn, 50, 135 + i * 105);
        lv_obj_set_free_num(btn, i);
        lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, run_diagnostic);
        lv_obj_t *label = lv_label_create(btn, NULL);
        lv_label_set_text(label, names[i]);

        lv_obj_t *description = lv_label_create(parent, NULL);
        lv_label_set_style(description, &hint_small_style);
        lv_label_set_long_mode(description, LV_LABEL_LONG_BREAK);
        lv_obj_set_width(description, 810);
        lv_label_set_text(description, descriptions[i]);
        lv_obj_set_pos(description, 410, 143 + i * 105);
    }

    separator = lv_line_create(parent, separator);
    lv_obj_set_pos(separator, 50, 442);

    lv_obj_t *scope = lv_label_create(parent, NULL);
    lv_label_set_style(scope, &hint_small_style);
    lv_label_set_long_mode(scope, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(scope, 1180);
    lv_label_set_text(scope,
        "Controles issus de HWTest-RCM 0.15\n\n"
        "L'identification DRAM ne remplace pas un test complet de la RAM.\n"
        "Audio, radios, stockage et commandes : integration a venir.");
    lv_obj_set_pos(scope, 50, 468);
}
