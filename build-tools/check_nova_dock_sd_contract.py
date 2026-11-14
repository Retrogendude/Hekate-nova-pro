"""Compile and exercise the real BDK sd_mount contract, with SD I/O substituted."""
from pathlib import Path
import re, subprocess
w = Path(__file__).resolve().parent
p = w / 'hekate-nova-pro-v20-dock'
s = (p / 'bdk/storage/sd.c').read_text()
start = s.index('int sd_mount()')
end = s.index('\n}', start) + 2
function = s[start:end].replace('int sd_mount()', 'int real_sd_mount()')
fixture = '''#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
static bool sd_init_done, sd_mounted;
static int init_result, mount_result, initializations, mounts, sd_fs;
static struct { bool mute; } gfx_con;
#define FR_OK 0
#define EPRINTF(...) ((void)0)
#define EPRINTFARGS(...) ((void)0)
static bool sdmmc_get_sd_inserted(void) {return true;}
static int sd_initialize(bool b) {initializations++;sd_init_done=!init_result;return init_result;}
static int f_mount(void*p,const char*s,int v) {mounts++;return mount_result;}
''' + function + '''
int main(void) {
 sd_init_done=sd_mounted=true;assert(real_sd_mount()==0&&!initializations&&!mounts);
 sd_init_done=sd_mounted=false;assert(real_sd_mount()==0&&sd_init_done&&sd_mounted&&initializations==1&&mounts==1);
 sd_init_done=sd_mounted=false;init_result=1;assert(real_sd_mount()!=0&&!sd_mounted);
 sd_init_done=sd_mounted=false;init_result=0;mount_result=13;assert(real_sd_mount()!=0&&!sd_mounted);
 sd_init_done=true;mount_result=0;assert(real_sd_mount()==0&&sd_mounted);
 puts("PASS: real BDK sd_mount returns zero on success; five paths checked.");
}
'''
directory = w / 'nova-dock-tests'
(directory / 'sd-contract.c').write_text(fixture)
subprocess.run([str(w / 'toolchains/w64devkit/bin/gcc.exe'), str(directory / 'sd-contract.c'),
                '-o', str(directory / 'sd-contract.exe'), '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter'], check=True)
subprocess.run([str(directory / 'sd-contract.exe')], check=True)
assert 'sd_mount() != 0' in (p / 'nyx/nyx_gui/frontend/gui_tv.c').read_text()
