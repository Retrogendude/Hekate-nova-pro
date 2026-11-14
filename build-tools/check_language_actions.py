from pathlib import Path
import re
import subprocess

workspace = Path(__file__).resolve().parent.parent
source_root = workspace / 'work' / 'hekate-6.5.4'
source = (source_root / 'nyx/nyx_gui/frontend/gui_options.c').read_text(encoding='utf-8')
header = (source_root / 'nyx/nyx_gui/localization.h').read_text(encoding='utf-8')
language_macros = '\n'.join(re.findall(r'^#define I18N_\w+\s+\d+', header, re.M))


def function(name):
    signature = re.search(r'^static lv_res_t ' + name + r'\([^\n]*\)\s*\n\{', source, re.M)
    assert signature, name
    end = re.search(r'^\}', source[signature.end():], re.M)
    assert end, name
    return source[signature.start():signature.end() + end.end()]


preamble = r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;
typedef struct { u32 selected; } lv_obj_t;
typedef int lv_res_t;
typedef lv_res_t (*lv_action_t)(lv_obj_t *);
#define LV_RES_OK 0
#define LV_RES_INV 1
#define LV_BTN_ACTION_CLICK 0
#define LV_OPA_COVER 255
#define FR_OK 0
static struct { u32 language; } n_cfg;
static struct { lv_obj_t *mid; } status_bar;
static bool nyx_changes_made;
static u32 language_pending;
static lv_obj_t *close_btn, *language_parent_close_btn;
static bool mounted, fail_mount, missing_nyx, fail_write;
static int mounts, unmounts, stat_calls, write_calls, reloads, notices, close_calls, cases;
static int opacity, clickable;
static u32 written_language;
static const char *last_notice;
static lv_action_t status_action;
static lv_obj_t status_button;
static lv_obj_t parent_close_button, language_close_button;
static void require(bool ok, const char *message) {
    if (!ok) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}
static u32 lv_ddlist_get_selected(lv_obj_t *list) { return list->selected; }
static bool sd_get_card_mounted(void) { return mounted; }
static int sd_mount(void) { mounts++; if (fail_mount) return 1; mounted = true; return 0; }
static void sd_unmount(void) { unmounts++; mounted = false; }
static int f_stat(const char *path, void *info) {
    (void)info; stat_calls++;
    require(!strcmp(path, "bootloader/sys/nyx.bin"), "preflight checks actual reload target");
    return missing_nyx ? 1 : FR_OK;
}
static int create_nyx_config_entry(bool force_unmount) {
    write_calls++;
    require(force_unmount, "writer forced cleanup requested");
    if (!fail_write) written_language = n_cfg.language;
    sd_unmount();
    return fail_write;
}
static void reload_nyx(lv_obj_t *obj, bool force) {
    require(!obj && !force, "guarded reload without deleting selector");
    reloads++;
}
static const char *nyx_tr(const char *text) { return text; }
static void _home_layout_notice(const char *text) { notices++; last_notice = text; }
static lv_res_t nyx_win_close_action(lv_obj_t *btn) { (void)btn; close_btn = NULL; close_calls++; return LV_RES_INV; }
static lv_res_t _save_nyx_options_action(lv_obj_t *btn) { (void)btn; return LV_RES_OK; }
static void lv_btn_set_action(lv_obj_t *obj, int action, lv_action_t fn) {
    require(obj == status_bar.mid && action == LV_BTN_ACTION_CLICK, "restore parent save callback"); status_action = fn;
}
static void lv_obj_set_opa_scale(lv_obj_t *obj, int value) {
    require(obj == status_bar.mid, "restore parent status visibility"); opacity = value;
}
static void lv_obj_set_click(lv_obj_t *obj, bool enabled) {
    require(obj == status_bar.mid, "restore parent status clickability"); clickable = enabled;
}
'''

tests = r'''
static void reset(bool initially_mounted) {
    mounted = initially_mounted;
    fail_mount = missing_nyx = fail_write = false;
    mounts = unmounts = stat_calls = write_calls = reloads = notices = close_calls = 0;
    last_notice = NULL;
    n_cfg.language = I18N_FR;
    language_pending = I18N_ES;
    written_language = UINT32_MAX;
    nyx_changes_made = true;
    opacity = clickable = 0;
    status_action = NULL;
    status_bar.mid = &status_button;
    close_btn = &language_close_button;
    language_parent_close_btn = &parent_close_button;
}
int main(void) {
    for (u32 selected = 0; selected < I18N_LANGUAGE_COUNT + 2; selected++) {
        reset(false);
        lv_obj_t list = { .selected = selected };
        require(_language_action(&list) == LV_RES_OK, "selection succeeds");
        require(language_pending == (selected < I18N_LANGUAGE_COUNT ? selected : I18N_EN), "pending selection clamps invalid locale");
        require(n_cfg.language == I18N_FR && !write_calls, "selection never commits locale");
        cases++;
    }
    for (u32 pending = 0; pending < I18N_LANGUAGE_COUNT; pending++) {
        reset(false); language_pending = pending;
        require(_close_language_window(NULL) == LV_RES_INV, "close result propagated");
        require(n_cfg.language == I18N_FR && language_pending == I18N_FR, "cancel retains active locale");
        require(!write_calls && !reloads && close_calls == 1, "cancel never writes or reloads");
        require(status_action == _save_nyx_options_action && opacity == LV_OPA_COVER && clickable, "parent Save restored");
        require(close_btn == &parent_close_button && !language_parent_close_btn, "parent Close restored without retaining stale child pointer");
        cases++;
    }
    for (int initial = 0; initial <= 1; initial++) {
        reset(initial); fail_mount = true;
        _save_language(NULL);
        require(n_cfg.language == I18N_FR && nyx_changes_made, "mount failure retains locale and dirty state");
        require(!stat_calls && !write_calls && !reloads && notices == 1, "mount failure stops before persistence");
        require(mounted == initial, "mount failure retains ownership"); cases++;

        reset(initial); missing_nyx = true;
        _save_language(NULL);
        require(n_cfg.language == I18N_FR && nyx_changes_made, "missing Nyx retains locale and dirty state");
        require(stat_calls == 1 && !write_calls && !reloads && notices == 1, "missing Nyx stops before persistence");
        require(mounted == initial && unmounts == !initial, "missing Nyx preserves parent mount"); cases++;

        for (u32 pending = 0; pending < I18N_LANGUAGE_COUNT; pending++) {
            reset(initial); language_pending = pending; fail_write = true;
            _save_language(NULL);
            require(n_cfg.language == I18N_FR && nyx_changes_made, "write failure restores active locale and dirty state");
            require(write_calls == 1 && !reloads && notices == 1, "write failure prevents reload");
            require(written_language == UINT32_MAX && !mounted, "write failure does not report persisted locale"); cases++;

            reset(initial); language_pending = pending;
            _save_language(NULL);
            require(n_cfg.language == pending && written_language == pending, "success saves selected language");
            require(write_calls == 1 && reloads == 1 && !nyx_changes_made, "success reloads once after save");
            require(notices == 1 && strstr(last_notice, "Language saved."), "reload return reports saved state"); cases++;
        }
    }
    reset(false); language_pending = UINT32_MAX;
    _save_language(NULL);
    require(n_cfg.language == I18N_EN && written_language == I18N_EN && reloads == 1, "invalid pending locale safely saves English"); cases++;
    printf("PASS: %d actual-C language action cases: pending/cancel, SD/Nyx preflight, write rollback, seven locales and reload return.\n", cases);
    return 0;
}
'''

generated = preamble + '\n' + language_macros + '\n' + '\n'.join(function(name) for name in (
    '_language_action', '_close_language_window', '_save_language')) + '\n' + tests
harness = workspace / 'work' / 'language_actions_generated.c'
executable = workspace / 'work' / 'language_actions_generated.exe'
harness.write_text(generated, encoding='utf-8')
gcc = workspace / 'work' / 'toolchains' / 'w64devkit' / 'bin' / 'gcc.exe'
subprocess.run([str(gcc), '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', str(harness), '-o', str(executable)], check=True)
subprocess.run([str(executable)], check=True)
