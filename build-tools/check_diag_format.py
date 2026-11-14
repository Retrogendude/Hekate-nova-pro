"""Run actual bounded report formatter together with the actual translation runtime."""
from pathlib import Path
import runpy
import subprocess

work = Path(__file__).resolve().parent
runpy.run_path(str(work / 'check_localization.py'))
host = work / 'localization-host'
source = (work / 'hekate-6.5.4/nyx/nyx_gui/diagnostics/diag_format.c').read_text(encoding='utf-8')
source = source.replace('#include "diag_format.h"', '#include <stdarg.h>')
source = source.replace('#include "../localization.h"', '')
harness = (host / 'localization_harness.c').read_text(encoding='utf-8')
harness = harness.replace('int main(void)', 'int localization_main(void)')
harness += '\n#define free tracked_free\n' + source + '\n#undef free\n' + r'''
static void valid_utf8(const char *value) {
    const unsigned char *p = (const unsigned char *)value;
    while (*p) {
        unsigned length = *p < 0x80 ? 1 : *p < 0xE0 ? 2 : *p < 0xF0 ? 3 : 4;
        if (length == 1) { ++p; continue; }
        require((*p & 0xC0) != 0x80, "UTF-8 begins at a leading byte");
        for (unsigned i = 1; i < length; ++i) require(p[i] && (p[i] & 0xC0) == 0x80, "complete UTF-8 character");
        p += length;
    }
}
int main(void) {
    localization_main();
    n_cfg.language = I18N_FR;
    char result[256];
    nyx_diag_snprintf(result, sizeof result, "Code %08X %d [%s]", 0xABu, (u32)-27, "Home");
    require(!strcmp(result, "Code 000000AB -27 [Accueil]"), "format hex and signed values; localize string arguments"); ++cases;
    nyx_diag_snprintf(result, sizeof result, "Page %d / %d", 12u, 345u);
    require(!strcmp(result, "Page 12 / 345"), "localized numeric format"); ++cases;
    n_cfg.language = I18N_RU;
    nyx_diag_snprintf(result, sizeof result, "Page %d / %d", 12u, 345u);
    require(!strcmp(result, "Страница 12 / 345"), "translate format before numeric interpolation"); ++cases;
    nyx_diag_snprintf(result, sizeof result, "value=%u / %c%%", UINT32_MAX, (unsigned)'X');
    require(!strcmp(result, "value=4294967295 / X%"), "unsigned, character and literal percent"); ++cases;
    for (unsigned capacity = 0; capacity < 40; ++capacity) {
        unsigned char guarded[64]; memset(guarded, 0xA5, sizeof guarded);
        nyx_diag_snprintf((char *)guarded + 8, capacity, "%s", "Русский длинный текст");
        for (unsigned i = 0; i < 8; ++i) require(guarded[i] == 0xA5, "leading canary");
        for (unsigned i = 8 + capacity; i < sizeof guarded; ++i) require(guarded[i] == 0xA5, "trailing canary");
        if (capacity) { require(memchr(guarded + 8, 0, capacity) != NULL, "bounded termination"); valid_utf8((char *)guarded + 8); }
        require(live_count == 0, "bounded formatting releases translations"); ++cases;
    }
    n_cfg.language = I18N_FR;
    reset_allocations(0);
    nyx_diag_snprintf(result, sizeof result, "Page %d / %d [%s]", 1u, 2u, "Home");
    size_t cost = allocation_calls;
    require(live_count == 0, "baseline formatter cleanup");
    for (size_t fail = 1; fail <= cost + 2; ++fail) {
        reset_allocations(fail);
        nyx_diag_snprintf(result, sizeof result, "Page %d / %d [%s]", 1u, 2u, "Home");
        require(live_count == 0, "allocation failure frees every format/argument translation");
        require(strstr(result, "1 / 2") != NULL, "failure preserves raw values"); ++cases;
    }
    printf("PASS: bounded localized diagnostics, canaries for 40 capacities, UTF-8 truncation, allocation failures; %u combined cases.\n", cases);
    return 0;
}
'''
target = host / 'diag_format_check.c'
target.write_text(harness, encoding='utf-8')
exe = host / 'diag_format_check.exe'
subprocess.run([str(work / 'toolchains/w64devkit/bin/gcc.exe'), '-std=c11', '-Wall', '-Wextra', '-Werror', '-Wno-misleading-indentation', '-O2', '-I', str(work / 'hekate-6.5.4/nyx/nyx_gui'), str(target), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
