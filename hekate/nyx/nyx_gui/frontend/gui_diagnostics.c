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
    (void)fmt;
    /* Every key in this subset is emitted exactly once per run. */
    if (sev == DX_WARN) warnings++;
    if (sev == DX_FAIL) failures++;
    diag_log(COL_DEFAULT, "  [%s] %s\n", key,
        sev == DX_FAIL ? "FAIL" : sev == DX_WARN ? "WARN" : "PASS");
}

static lv_res_t run_diagnostic(lv_obj_t *btn)
{
    u32 group = lv_obj_get_free_num(btn);
    report = malloc(REPORT_SIZE);
    if (!report) return LV_RES_OK;
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
    lv_obj_t *win = nyx_create_standard_window("Diagnostic - Results", NULL);
    lv_obj_t *label = lv_label_create(win, NULL);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, LV_HOR_RES - 80);
    lv_label_set_style(label, &monospace_text);
    lv_label_set_text(label, report);
    free(report);
    report = NULL;
    return LV_RES_OK;
}

void create_tab_diagnostics(lv_theme_t *th, lv_obj_t *parent)
{
    (void)th;
    lv_obj_t *intro = lv_label_create(parent, NULL);
    lv_label_set_text(intro,
        "HWTest-RCM - Native diagnostics (preview)\n"
        "Display identification, backlight state and DRAM identity.\n"
        "Audio, radios, inputs, storage and full RAM testing are not integrated yet.");
    lv_obj_set_pos(intro, 20, 15);
    static const char *names[] = { "Run available checks", "Display / backlight", "DRAM identity" };
    for (u32 i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(parent, NULL);
        lv_obj_set_size(btn, 440, 65);
        lv_obj_set_pos(btn, 20, 125 + i * 85);
        lv_obj_set_free_num(btn, i);
        lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, run_diagnostic);
        lv_obj_t *label = lv_label_create(btn, NULL);
        lv_label_set_text(label, names[i]);
    }
}
