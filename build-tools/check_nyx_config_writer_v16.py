from pathlib import Path
import re
import subprocess

workspace = Path(__file__).resolve().parent.parent
source_root = workspace / 'work' / 'hekate-6.5.4'
config_source = (source_root / 'nyx/nyx_gui/config.c').read_text(encoding='utf-8')
header = (source_root / 'nyx/nyx_gui/config.h').read_text(encoding='utf-8')
localization_header = (source_root / 'nyx/nyx_gui/localization.h').read_text(encoding='utf-8')

def function(name):
    signature = re.search(r'^(?:static )?(?:void|int|bool|u32) ' + name + r'\([^\n]*\)\s*\n\{', config_source, re.M)
    assert signature, name
    start = signature.start()
    end = re.search(r'^\}', config_source[signature.end():], re.M)
    assert end, name
    return config_source[start:signature.end() + end.end()]

defaults = function('set_nyx_default_configuration')
writer = function('create_nyx_config_entry')
appearance_helpers = function('valid_font_basename') + '\n' + function('valid_wallpaper_basename') + '\n' + function('nyx_sanitize_appearance')
config_type = re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;', header, re.S).group()
layout_macros = '\n'.join(re.findall(r'^#define HOME_LAYOUT_\w+\s+\d+', header, re.M))
language_macros = '\n'.join(re.findall(r'^#define I18N_\w+\s+\d+', localization_header, re.M))

preamble = r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t u32;
typedef struct { bool open; } FIL;
typedef int FRESULT;
#define FR_NO_FILE 4
#define FR_NO_PATH 5
#define FR_OK 0
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 8
static bool mounted;
static int fail_mount, fail_open, fail_close, fail_write_at;
static int mounts, opens, closes, writes, unmounts;
static char written[8192];
static size_t used;
static int cases;

static void require(int ok, const char *what) {
    if (!ok) { fprintf(stderr, "FAIL: %s (write failure position %d)\n", what, fail_write_at); exit(1); }
}
static bool sd_get_card_mounted(void) { return mounted; }
static int sd_mount(void) { mounts++; if (fail_mount) return 1; mounted = true; return 0; }
static void sd_unmount(void) { unmounts++; mounted = false; }
static int f_mkdir(const char *path) { require(!strcmp(path, "bootloader"), "mkdir path"); return 0; }
static int f_open(FIL *file, const char *path, int flags) {
    opens++;
    require(!strcmp(path, "bootloader/nyx.ini.nova.tmp"), "writer path");
    require(flags == (FA_WRITE | FA_CREATE_ALWAYS), "writer mode");
    if (fail_open) return 1;
    file->open = true; return 0;
}
static int f_puts(const char *text, FIL *file) {
    require(file->open, "write only to open file");
    writes++;
    if (writes == fail_write_at) return -1;
    size_t len = strlen(text);
    require(used + len < sizeof(written), "stub capture size");
    memcpy(written + used, text, len + 1); used += len;
    return (int)len;
}
static int f_close(FIL *file) {
    require(file->open, "close only once");
    file->open = false; closes++;
    return fail_close ? 1 : FR_OK;
}
static int f_stat(const char *path,void *info){(void)path;(void)info;return FR_NO_FILE;}
static int f_unlink(const char *path){(void)path;return FR_OK;}
static int f_rename(const char *from,const char *to){(void)from;(void)to;return FR_OK;}
static char *test_itoa(unsigned value, char *output, int base) {
    require(base == 10 || base == 16, "supported numeric base");
    sprintf(output, base == 16 ? "%x" : "%u", value);
    return output;
}
#define itoa test_itoa
#define s_printf sprintf
'''

tests = r'''
static void reset(bool initially_mounted) {
    mounted = initially_mounted;
    fail_mount = fail_open = fail_close = fail_write_at = 0;
    mounts = opens = closes = writes = unmounts = 0;
    used = 0; written[0] = 0;
    set_nyx_default_configuration();
}
static void set_distinct_values(void) {
    n_cfg.theme_bg = 0x1a2b3c;
    n_cfg.theme_color = 173;
    n_cfg.entries_5_col = 1;
    n_cfg.timeoffset = 0x1234abcd;
    n_cfg.timedst = 1;
    n_cfg.home_screen = 3;
    n_cfg.verification = 2;
    n_cfg.ums_emmc_rw = 1;
    n_cfg.jc_disable = 0;
    n_cfg.jc_force_right = 1;
    n_cfg.bpmp_clock = 5;
    n_cfg.language = I18N_RU;
    strcpy(n_cfg.font_file, "Readable.nyxf");
    n_cfg.font_size = 36;
    n_cfg.background_effect = 3;
    n_cfg.background_speed = 1;
    n_cfg.background_opacity = 50;
}
int main(void) {
    reset(false);
    require(n_cfg.home_layout == HOME_LAYOUT_TILES, "default tiles");
    require(n_cfg.language == I18N_EN, "default English language");
    require(!n_cfg.font_file[0] && n_cfg.font_size == 30, "default embedded font at 30px");
    require(n_cfg.background_effect == 0 && n_cfg.background_speed == 2 && n_cfg.background_opacity == 20, "quiet default background");
    require(n_cfg.theme_bg == 0x10151d && n_cfg.theme_color == 0, "modern default theme");
    cases++;
    int successful_writes = 0;
    for (unsigned layout = 0; layout <= 1; layout++) {
        reset(false); set_distinct_values(); n_cfg.home_layout = layout;
        require(create_nyx_config_entry(false) == 0, "valid layout writes successfully");
        char expected[1024];
        sprintf(expected,
            "[config]\nthemebg=1a2b3c\nthemecolor=173\nentries5col=1\ntimeoffset=1234abcd"
            "\ntimedst=1\nhomescreen=3\nhome_layout=%u\nverification=2\numsemmcrw=1"
            "\njcdisable=0\njcforceright=1\nbpmpclock=5\nlanguage=%u"
            "\nfont_file=Readable.nyxf\nfont_size=36\nbackground_effect=3\nbackground_speed=1\nbackground_opacity=50\nnova_halo=1\nnova_text_color=F2F5FA\nnova_border_color=334050\nnova_reflection_color=55C9F1\nnova_menu_opacity=25\nnova_icon_opacity=100\nnova_logo_color=55C9F1\nnova_icon_color=55C9F1\nnova_selection_color=1C2937\nwallpaper_file=Nova-Carbon.bmp\n", layout, I18N_RU);
        require(!strcmp(written, expected), "exact full configuration and bases");
        require(closes == 1 && unmounts == 1 && !mounted, "success cleanup");
        require(n_cfg.home_layout == layout, "valid layout remains chosen");
        require(n_cfg.language == I18N_RU, "valid language remains chosen independently");
        require(writes == 57, "all 57 configuration writes execute");
        successful_writes = writes; cases++;
    }
    unsigned invalid_language[] = {I18N_LANGUAGE_COUNT, 17, UINT32_MAX};
    for (unsigned i = 0; i < sizeof(invalid_language)/sizeof(*invalid_language); i++) {
        reset(false); n_cfg.language = invalid_language[i];
        require(create_nyx_config_entry(false) == 0, "invalid language write succeeds with fallback");
        require(n_cfg.language == I18N_EN && strstr(written, "\nlanguage=0\n"), "invalid language normalized");
        cases++;
    }
    unsigned invalid[] = {2, 17, UINT32_MAX};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(*invalid); i++) {
        reset(false); n_cfg.home_layout = invalid[i];
        require(create_nyx_config_entry(false) == 0, "invalid layout write succeeds with fallback");
        require(n_cfg.home_layout == HOME_LAYOUT_TILES && strstr(written, "\nhome_layout=0\n"), "invalid layout normalized");
        cases++;
    }
    reset(false); fail_mount = 1;
    require(create_nyx_config_entry(true) == 1 && opens == 0 && writes == 0 && closes == 0, "mount failure never succeeds"); cases++;
    for (int initial = 0; initial <= 1; initial++) {
        for (int force = 0; force <= 1; force++) {
            reset(initial); fail_open = 1;
            require(create_nyx_config_entry(force) == 1 && writes == 0 && closes == 0, "open failure never succeeds");
            require(unmounts == (force || !initial), "open failure mount ownership"); cases++;
            reset(initial); fail_close = 1;
            require(create_nyx_config_entry(force) == 1 && closes == 1, "close failure never succeeds");
            require(unmounts == (force || !initial), "close failure mount ownership"); cases++;
            reset(initial);
            require(create_nyx_config_entry(force) == 0, "all mount ownership success variants");
            require(unmounts == (force || !initial), "success mount ownership"); cases++;
        }
    }
    for (int position = 1; position <= successful_writes; position++) {
        for (int initial = 0; initial <= 1; initial++) {
            reset(initial); fail_write_at = position;
            require(create_nyx_config_entry(false) == 1, "each write failure never succeeds");
            require(writes == position && closes == 1, "write failure stops and closes");
            require(unmounts == !initial, "write failure mount ownership"); cases++;
        }
    }
    printf("PASS: %d executed C cases; both layout values, exact field and language serialization, invalid values, mount/open/write/close failures and mount ownership. %d write failure positions covered.\n", cases, successful_writes);
    return 0;
}
'''

generated = preamble + '\n' + layout_macros + '\n' + language_macros + '\n' + config_type + '\nstatic nyx_config n_cfg;\n' + defaults + '\n' + appearance_helpers + '\n' + writer + '\n' + tests
harness = workspace / 'work' / 'nyx_config_writer_harness_v16.c'
executable = workspace / 'work' / 'nyx_config_writer_harness_v16.exe'
harness.write_text(generated, encoding='utf-8')
gcc = workspace / 'work' / 'toolchains' / 'w64devkit' / 'bin' / 'gcc.exe'
subprocess.run([str(gcc), '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', str(harness), '-o', str(executable)], check=True)
subprocess.run([str(executable)], check=True)
