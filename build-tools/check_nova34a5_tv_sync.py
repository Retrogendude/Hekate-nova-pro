"""Run the real stream-sync wait with simulated AUX, HPD and clock."""
from pathlib import Path
import hashlib, json, re, subprocess

w = Path(__file__).resolve().parent
p = w / 'hekate-nova-pro-v34-alpha5'
source = p / 'bdk/display/tv/dpout.c'
code = source.read_text()
body = re.search(r'static int _wait_sink_sync\(dpout_video_t \*v\)\n\{.*?\n\}', code, re.S).group()
tail = code[code.index('    _video_check(v);', code.index('int dpout_video_start(')):]
assert tail.startswith('    _video_check(v);\n    if (_wait_sink_sync(v)) {')
assert tail.index('return 12;') < tail.index('return 0;')
d = w / 'nova34a5-tv-tests'
harness = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <display/tv/dpout.h>
#define DPCD_SINK_STATUS 0x205
#define SINK_PORT0_IN_SYNC 1
#define s_printf sprintf
static uint32_t now, calls, ready, drop;
static int errors;
u32 get_tmr_ms(void) { return now; }
void msleep(u32 ms) { now += ms; }
bool dpout_hpd(void) { return calls < drop; }
int dpout_dpcd_read(u32 addr, u8 *b, u32 n) {
    assert(addr == DPCD_SINK_STATUS && n == 1); calls++;
    if(errors == 1 || (errors == 2 && calls < 4)) return -1;
    *b = calls >= ready ? 1 : 0; return 0;
}
FUNCTION
static void reset(dpout_video_t *v) {
    memset(v, 0, sizeof(*v)); v->in_sync = true; v->dpcd_200[5] = 1;
    now = calls = errors = 0; ready = 1; drop = UINT32_MAX;
}
int main(void) {
    dpout_video_t v;
    reset(&v); assert(!_wait_sink_sync(&v) && v.in_sync && calls == 1 && !now);
    reset(&v); ready=26; assert(!_wait_sink_sync(&v) && v.in_sync && now==500);
    reset(&v); ready=51; assert(_wait_sink_sync(&v)==12 && !v.in_sync && now==1000 && calls==50);
    assert(strstr(v.why,"not synchronized") && !v.dpcd_200[5]);
    reset(&v); errors=1; assert(_wait_sink_sync(&v)==12 && !v.in_sync && now==1000);
    assert(strstr(v.why,"AUX did not answer"));
    reset(&v); errors=2; assert(!_wait_sink_sync(&v) && v.in_sync && now==60);
    reset(&v); drop=0; assert(_wait_sink_sync(&v)==12 && !calls && !v.in_sync);
    assert(strstr(v.why,"HPD went away"));
    reset(&v); ready=30; drop=4; assert(_wait_sink_sync(&v)==12 && calls==4 && !v.in_sync);
    reset(&v); now=UINT32_MAX-200; ready=26;
    assert(!_wait_sink_sync(&v) && v.in_sync && now==299);
    reset(&v); now=UINT32_MAX-200; ready=100;
    assert(_wait_sink_sync(&v)==12 && !v.in_sync && now==799);
    puts("PASS: 9 stream-sync scenarios; real function, simulated AUX/HPD/timer; hardware pending.");
}
'''.replace('FUNCTION', body)
(d / 'sync.c').write_text(harness)
args = [str(w/'toolchains/w64devkit/bin/gcc.exe'), str(d/'sync.c'),
        '-I'+str(p/'bdk'), '-std=gnu11', '-Wall', '-Wextra', '-Werror', '-o', str(d/'sync.exe')]
subprocess.run(args, check=True)
r = subprocess.run([str(d/'sync.exe')], check=True, capture_output=True, text=True)
print(r.stdout.strip())
(w/'nova34a5-tv-sync-check.json').write_text(json.dumps({
    'passed': True, 'result': r.stdout.strip(), 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
    'hardware_validated': False, 'success_path_requires_fresh_sync': True}, indent=2))
