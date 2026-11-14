"""Compile and execute the actual localization runtime with small LVGL stubs.

Only platform include directives are replaced. All runtime functions and the
generated firmware catalog are compiled unchanged. Allocation tracking belongs
to the runtime alone; LVGL stubs model copied text and retained button maps.
"""
from pathlib import Path
import json
import re
import subprocess

WORK = Path(__file__).resolve().parent
ROOT = WORK / "hekate-6.5.4/nyx/nyx_gui"
HOST = WORK / "localization-host"
HOST.mkdir(exist_ok=True)

catalog = json.loads((ROOT / "localization_catalog.json").read_text(encoding="utf-8"))
formats = re.compile(r"%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcspn]|%)")
for key, values in catalog.items():
    assert len(values) == 7 and all(isinstance(v, str) and v for v in values), key
    for value in values:
        assert formats.findall(key) == formats.findall(value), key

api = (ROOT / "localization.h").read_text(encoding="utf-8")
api = re.sub(r'^#include [<"](?:bdk\.h|libs/lvgl/lvgl\.h)[>"]\s*$', '', api, flags=re.M)
(HOST / "localization_host_api.h").write_text(api, encoding="utf-8")
runtime = (ROOT / "localization.c").read_text(encoding="utf-8")
runtime = runtime.replace('#include "localization.h"', '#include "localization_stubs.h"\n#include "localization_host_api.h"')
runtime = runtime.replace('#include "config.h"', '')
runtime = runtime.replace('#include "fonts/unicode_fonts.h"', '')
(HOST / "localization_under_test.c").write_text(runtime, encoding="utf-8")

stubs = r'''
#ifndef LOCALIZATION_STUBS_H
#define LOCALIZATION_STUBS_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;
typedef uint8_t u8;
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
typedef enum { LV_RES_OK, LV_RES_INV } lv_res_t;
typedef enum { LV_SIGNAL_OTHER, LV_SIGNAL_CLEANUP } lv_signal_t;
typedef struct lv_obj_t lv_obj_t;
typedef lv_res_t (*lv_signal_func_t)(lv_obj_t *, lv_signal_t, void *);
typedef lv_res_t (*lv_action_t)(lv_obj_t *);
typedef lv_res_t (*lv_btnm_action_t)(lv_obj_t *, const char *);
typedef struct { lv_obj_t *btnm; } lv_mbox_ext_t;
struct lv_obj_t {
    char *owned_text;
    const char *text;
    const char **map;
    lv_signal_func_t signal;
    unsigned copied, borrowed;
};
static struct { u32 language; } n_cfg;
static unsigned font_inits, signal_calls;
static lv_res_t next_signal_result = LV_RES_OK;
static void nyx_unicode_fonts_init(void) { ++font_inits; }
static lv_res_t original_signal(lv_obj_t *object, lv_signal_t signal, void *param) {
    (void)object; (void)signal; (void)param; ++signal_calls;
    return next_signal_result;
}
static void lv_label_set_text(lv_obj_t *object, const char *text) {
    size_t length = text ? strlen(text) : 0;
    char *copy = malloc(length + 1);
    if (!copy) abort();
    if (text) memcpy(copy, text, length + 1); else copy[0] = 0;
    free(object->owned_text);
    object->owned_text = copy; object->text = copy; ++object->copied;
}
static void lv_label_set_static_text(lv_obj_t *object, const char *text) {
    free(object->owned_text); object->owned_text = NULL;
    object->text = text; ++object->borrowed;
}
static void lv_mbox_set_text(lv_obj_t *object, const char *text) { lv_label_set_text(object, text); }
static void lv_ddlist_set_options(lv_obj_t *object, const char *text) { lv_label_set_text(object, text); }
static void lv_roller_set_options(lv_obj_t *object, const char *text) { lv_label_set_text(object, text); }
static void lv_win_set_title(lv_obj_t *object, const char *text) { lv_label_set_text(object, text); }
static lv_obj_t *lv_win_add_btn(lv_obj_t *object, const void *image, const char *text, lv_action_t action) {
    (void)image; (void)action; lv_label_set_text(object, text); return object;
}
static lv_obj_t *lv_tabview_add_tab(lv_obj_t *object, const char *text) {
    lv_label_set_text(object, text); return object;
}
static lv_signal_func_t lv_obj_get_signal_func(lv_obj_t *object) { return object->signal; }
static void lv_obj_set_signal_func(lv_obj_t *object, lv_signal_func_t signal) { object->signal = signal; }
static void lv_btnm_set_map(lv_obj_t *object, const char **map) { object->map = map; }
static void lv_mbox_add_btns(lv_obj_t *object, const char **map, lv_btnm_action_t action) {
    (void)action; object->map = map;
}
static void *lv_obj_get_ext_attr(lv_obj_t *object) {
    static lv_mbox_ext_t ext; ext.btnm = object; return &ext;
}
#endif
'''
(HOST / "localization_stubs.h").write_text(stubs, encoding="utf-8")

harness = r'''
#include "localization_stubs.h"
static unsigned cases;
static size_t live_count, allocation_calls, fail_at;
static void *live[65536];
static void require(int ok, const char *message) {
    if (!ok) { fprintf(stderr, "FAIL: %s (case %u, allocation %zu, failing %zu, live %zu)\n",
                       message, cases, allocation_calls, fail_at, live_count); exit(1); }
}
static size_t locate(void *ptr) {
    for (size_t i = 0; i < live_count; ++i) if (live[i] == ptr) return i;
    require(0, "runtime must only free its own live allocations"); return 0;
}
static int fail_request(void) { return ++allocation_calls == fail_at; }
static void *tracked_malloc(size_t bytes) {
    if (fail_request()) return NULL;
    void *ptr = malloc(bytes); require(ptr != NULL, "host allocation");
    require(live_count < ARRAY_SIZE(live), "tracker capacity"); live[live_count++] = ptr; return ptr;
}
static void *tracked_calloc(size_t count, size_t bytes) {
    if (fail_request()) return NULL;
    void *ptr = calloc(count, bytes); require(ptr != NULL, "host allocation");
    require(live_count < ARRAY_SIZE(live), "tracker capacity"); live[live_count++] = ptr; return ptr;
}
static void tracked_free(void *ptr) {
    if (!ptr) return;
    size_t at = locate(ptr); live[at] = live[--live_count]; free(ptr);
}
#define malloc tracked_malloc
#define calloc tracked_calloc
#define free tracked_free
#include "localization_under_test.c"
#undef malloc
#undef calloc
#undef free

static lv_obj_t object(void) { lv_obj_t value = {0}; value.signal = original_signal; return value; }
static void text_is(lv_obj_t *value, const char *expected) {
    require(value->text && !strcmp(value->text, expected), "exact translated UI bytes");
    require(live_count == 0, "temporary translation released after copied text"); ++cases;
}
static void cleanup(lv_obj_t *value) {
    require(value->signal(value, LV_SIGNAL_CLEANUP, NULL) == LV_RES_OK, "cleanup return");
    free(value->owned_text); value->owned_text = NULL; value->text = NULL; value->map = NULL;
}
static void reset_allocations(size_t failure) {
    require(live_count == 0, "no allocations carried between cases");
    allocation_calls = 0; fail_at = failure;
}
static void verify_map(lv_obj_t *value, unsigned count) {
    require(value->map != NULL, "map present");
    for (unsigned i = 0; i < count; ++i) require(value->map[i] != NULL, "all map entries present");
    require(value->map[count] && !value->map[count][0], "map terminator retained");
}

int main(void) {
    nyx_localization_init(); require(font_inits == 1, "font initialization connected"); ++cases;
    const char *expected[] = {"Launch", "Démarrer", "Iniciar", "Avvia", "Запуск", "Starten", "Iniciar"};
    for (unsigned language = 0; language < I18N_LANGUAGE_COUNT; ++language) {
        n_cfg.language = language;
        require(!strcmp(nyx_tr("Launch"), expected[language]), "exact translation for all seven languages");
        const char unknown[] = "bootloader/payloads/Launch.bin";
        require(nyx_tr(unknown) == unknown, "unknown path pointer and bytes unchanged");
        require(nyx_tr(NULL) == NULL, "NULL translation"); ++cases;
    }
    n_cfg.language = 99; require(!strcmp(nyx_tr("Reglages Nyx"), "Nyx Settings"), "invalid language falls back to English"); ++cases;
    n_cfg.language = I18N_FR;
    require(!strcmp(nyx_tr("Demarrer"), "Démarrer"), "French ASCII source alias has accents"); ++cases;
    n_cfg.language = I18N_RU;
    require(!strcmp(nyx_tr("Reglages Nyx"), "Настройки Nyx"), "French ASCII source alias to Cyrillic"); ++cases;
    require(!strcmp(nyx_tr("Tools"), "Утилиты") && !strcmp(nyx_tr("Console Info"), "О консоли") &&
            !strcmp(nyx_tr("Options"), "Опции") && !strcmp(nyx_tr("Diagnostic"), "Тесты") &&
            !strcmp(nyx_tr("Save Options"), "Сохранить"), "Russian navigation uses measured short labels"); ++cases;
    n_cfg.language = I18N_ES;
    require(!strcmp(nyx_tr("Console Info"), "Info consola"), "short Spanish console caption"); ++cases;
    n_cfg.language = I18N_PT;
    require(!strcmp(nyx_tr("Console Info"), "Info consola"), "short Portuguese console caption"); ++cases;
    n_cfg.language = I18N_FR;
    require(!strcmp(nyx_tr("Save Options"), "Enregistrer") && !strcmp(nyx_tr("Save options"), "Enregistrer") &&
            !strcmp(nyx_tr("Enregistrer les options"), "Enregistrer"), "short French save caption and aliases"); ++cases;
    lv_obj_t label = object();
    nyx_label_set_text(&label, "Home"); text_is(&label, "Accueil");
    nyx_label_set_text(&label, "  Home\t"); text_is(&label, "  Accueil\t");
    nyx_label_set_text(&label, "Home\nTools\nOptions\nLaunch"); text_is(&label, "Accueil\nOutils\nOptions\nDémarrer");
    nyx_label_set_text(&label, "#FF8000 Home#\n#C7EA46 Tools#"); text_is(&label, "#FF8000 Accueil#\n#C7EA46 Outils#");
    nyx_label_set_text(&label, "#FF8000 Home# / #C7EA46 Tools#"); text_is(&label, "#FF8000 Accueil# / #C7EA46 Outils#");
    nyx_label_set_text(&label, "\xEF\xA0\x80  Home"); text_is(&label, "\xEF\xA0\x80  Accueil");
    nyx_label_set_text(&label, "\222Home"); text_is(&label, "\222Accueil");
    nyx_label_set_text(&label, "\262\xEF\xA0\x80  Home"); text_is(&label, "\262\xEF\xA0\x80  Accueil");
    nyx_label_set_text(&label, "bootloader/Home.bin"); text_is(&label, "bootloader/Home.bin");
    nyx_label_set_text(&label, "#FF8000 Custom boot profile#"); text_is(&label, "#FF8000 Custom boot profile#");
    nyx_label_set_static_text(&label, "Launch"); text_is(&label, "Démarrer");
    require(label.borrowed == 0 && label.owned_text && label.text == label.owned_text, "translated static label gets an LVGL-owned copy"); ++cases;
    const char static_unknown[] = "My unchanged system";
    nyx_label_set_static_text(&label, static_unknown);
    require(label.borrowed == 1 && label.text == static_unknown && !label.owned_text, "unknown static text retains original lifetime"); ++cases;
    nyx_mbox_set_text(&label, "Cancel"); text_is(&label, "Annuler");
    nyx_ddlist_set_options(&label, "Home\nTools"); text_is(&label, "Accueil\nOutils");
    nyx_win_set_title(&label, "Diagnostics"); text_is(&label, "Diagnostic");
    require(nyx_win_add_btn(&label, NULL, "Close", NULL) == &label, "window button result"); text_is(&label, "Fermer");
    require(nyx_tabview_add_tab(&label, "Tools") == &label, "tab result"); text_is(&label, "Outils");
    const char *months = "January\nFebruary\nMarch\nApril\nMay\nJune\nJuly\nAugust\nSeptember\nOctober\nNovember\nDecember";
    const char *month_text[] = {
        "January\nFebruary\nMarch\nApril\nMay\nJune\nJuly\nAugust\nSeptember\nOctober\nNovember\nDecember",
        "Janvier\nFévrier\nMars\nAvril\nMai\nJuin\nJuillet\nAoût\nSeptembre\nOctobre\nNovembre\nDécembre",
        "Enero\nFebrero\nMarzo\nAbril\nMayo\nJunio\nJulio\nAgosto\nSeptiembre\nOctubre\nNoviembre\nDiciembre",
        "Gennaio\nFebbraio\nMarzo\nAprile\nMaggio\nGiugno\nLuglio\nAgosto\nSettembre\nOttobre\nNovembre\nDicembre",
        "Январь\nФевраль\nМарт\nАпрель\nМай\nИюнь\nИюль\nАвгуст\nСентябрь\nОктябрь\nНоябрь\nДекабрь",
        "Januar\nFebruar\nMärz\nApril\nMai\nJuni\nJuli\nAugust\nSeptember\nOktober\nNovember\nDezember",
        "Janeiro\nFevereiro\nMarço\nAbril\nMaio\nJunho\nJulho\nAgosto\nSetembro\nOutubro\nNovembro\nDezembro"
    };
    const char *home_text[] = {"Home", "Accueil", "Inicio", "Home", "Главная", "Startseite", "Início"};
    for (unsigned language = 0; language < I18N_LANGUAGE_COUNT; ++language) {
        n_cfg.language = language;
        nyx_roller_set_options(&label, months); text_is(&label, month_text[language]);
        char long_source[1024] = {0}, long_expected[1024] = {0};
        for (unsigned line = 0; line < 24; ++line) {
            if (line) { strcat(long_source, "\n"); strcat(long_expected, "\n"); }
            strcat(long_source, "Home"); strcat(long_expected, home_text[language]);
        }
        nyx_ddlist_set_options(&label, long_source); text_is(&label, long_expected);
    }
    n_cfg.language = I18N_FR;
    cleanup(&label); require(live_count == 0, "label cleanup no temporary leaks"); ++cases;

    const char *original[] = {"\222Home", "Tools", "\n", "bootloader/Home.bin", ""};
    const char *replacement[] = {"Launch", "Cancel", ""};
    const char *unchanged[] = {"Custom path/Tools.bin", ""};
    lv_obj_t buttons = object();
    nyx_btnm_set_map(&buttons, original);
    verify_map(&buttons, 4);
    require(buttons.map != original, "translated map owns a new pointer array");
    require(!strcmp(buttons.map[0], "\222Accueil") && !strcmp(buttons.map[1], "Outils"), "map translations with control byte");
    require(!strcmp(buttons.map[2], "\n") && !strcmp(buttons.map[3], original[3]), "map row break and path preserved");
    require(!strcmp(original[0], "\222Home") && !strcmp(original[1], "Tools"), "original source map never mutated");
    require(live_count > 0, "translated map remains owned after call"); ++cases;
    size_t retained = live_count;
    next_signal_result = LV_RES_INV;
    require(buttons.signal(&buttons, LV_SIGNAL_OTHER, NULL) == LV_RES_INV, "non-cleanup signal return forwarded");
    require(live_count == retained, "non-cleanup signal retains map"); next_signal_result = LV_RES_OK; ++cases;
    nyx_btnm_set_map(&buttons, replacement); verify_map(&buttons, 2);
    require(!strcmp(buttons.map[0], "Démarrer") && !strcmp(buttons.map[1], "Annuler"), "replacing map installs new translations"); ++cases;
    nyx_btnm_set_map(&buttons, unchanged); verify_map(&buttons, 1);
    require(buttons.map == unchanged, "untranslated replacement retains original map");
    require(live_count == 1, "old translated array and entries released; signal record retained"); ++cases;
    cleanup(&buttons); require(live_count == 0, "matrix cleanup releases record"); ++cases;

    buttons = object(); nyx_mbox_add_btns(&buttons, replacement, NULL);
    verify_map(&buttons, 2); require(!strcmp(buttons.map[0], "Démarrer"), "message box maps use retained translations");
    cleanup(&buttons); require(live_count == 0, "message box map cleanup"); ++cases;
    buttons = object(); const char *empty[] = {""}; nyx_btnm_set_map(&buttons, empty);
    require(buttons.map == empty && live_count == 0, "empty map needs no retained allocation"); cleanup(&buttons); ++cases;

    reset_allocations(0); buttons = object(); nyx_btnm_set_map(&buttons, original);
    size_t map_cost = allocation_calls; cleanup(&buttons); require(live_count == 0, "baseline map cleanup");
    for (size_t failure = 1; failure <= map_cost + 2; ++failure) {
        reset_allocations(failure); buttons = object(); nyx_btnm_set_map(&buttons, original);
        verify_map(&buttons, 4);
        require(!strcmp(original[0], "\222Home"), "allocation failures never mutate source map");
        cleanup(&buttons); require(live_count == 0, "every new-map allocation failure cleans up"); ++cases;
    }
    for (size_t failure = 1; failure <= map_cost + 2; ++failure) {
        reset_allocations(0); buttons = object(); nyx_btnm_set_map(&buttons, replacement);
        fail_at = allocation_calls + failure; nyx_btnm_set_map(&buttons, original);
        verify_map(&buttons, 4); cleanup(&buttons);
        require(live_count == 0, "every replacement-map allocation failure cleans up"); ++cases;
    }
    reset_allocations(0); label = object(); nyx_label_set_text(&label, "#FF8000 Home#\nTools");
    size_t text_cost = allocation_calls; cleanup(&label);
    for (size_t failure = 1; failure <= text_cost + 2; ++failure) {
        reset_allocations(failure); label = object(); nyx_label_set_text(&label, "#FF8000 Home#\nTools");
        require(label.text && (!strcmp(label.text, "#FF8000 Home#\nTools") || !strcmp(label.text, "#FF8000 Accueil#\nOutils")), "text allocation failure falls back atomically");
        require(live_count == 0, "text allocation failure frees partial builder"); cleanup(&label); ++cases;
    }
    reset_allocations(0);
    require(maps == NULL, "all runtime matrix ownership records removed");
    printf("PASS: %u executed C cases; seven languages, all 12 month roller options, 24-line lists, aliases, Cyrillic/accent bytes, icons/control bytes, colors/newlines, copied static labels, retained/replaced maps and cleanup; %zu map allocation positions and %zu text allocation positions.\n", cases, map_cost, text_cost);
    return 0;
}
'''
(HOST / "localization_harness.c").write_text(harness, encoding="utf-8")
gcc = WORK / "toolchains/w64devkit/bin/gcc.exe"
exe = HOST / "localization_harness.exe"
subprocess.run([str(gcc), "-std=c11", "-Wall", "-Wextra", "-Werror", "-Wno-misleading-indentation", "-O2", "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-I", str(ROOT), str(HOST / "localization_harness.c"), "-o", str(exe)], check=True)
subprocess.run([str(exe)], check=True)
print(f"Catalog verified: {len(catalog)} exact keys, seven strings per key, exact printf placeholder sequences.")
