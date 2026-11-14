"""Regression tests on the real dock command-completion and transition helpers."""
from pathlib import Path
import hashlib, json, re, subprocess, sys
work = Path(__file__).resolve().parent
root = work / (sys.argv[1] if len(sys.argv) > 1 else 'hekate-tv-02')
tests = work / 'tv-protocol-tests'
tests.mkdir(exist_ok=True)
driver = root / 'bdk/display/tv/usbpd.c'
source = driver.read_text()

def function(name):
    m = re.search(r'^static [^\n]*\b' + re.escape(name) + r'\([^\n]*\)\n\{', source, re.M)
    assert m, name
    level = 0
    for i in range(source.index('{', m.start()), len(source)):
        if source[i] == '{': level += 1
        elif source[i] == '}':
            level -= 1
            if level == 0: return source[m.start():i + 1]
    raise AssertionError(name)

code = '''#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define BIT(n) (1u<<(n))
#include <display/tv/usbpd.h>
static u32 now;
static u32 get_tmr_ms(void) { return now; }
const char *dock_state_name(dock_state_t s) { return "test state"; }
#define s_printf sprintf
'''
code += re.search(r'^#define STEP_TIMEOUT_MS[^\n]*', source, re.M)[0] + '\n'
code += '\n'.join(function(name) for name in ('_go', '_take', '_timed_out', '_cmd_done'))
code += r'''
#define CHECK(expr) do { if(!(expr)) {fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#expr);return 1;} ++checks; } while(0)
int main(void) {
 unsigned checks=0;
 dock_t d={0};now=1000;d.since=900;
 d.alerts=PD_AL_CMD_DONE;d.s1=PD_S1_CMD_BUSY;
 CHECK(_cmd_done(&d)==0);
 CHECK((d.alerts & PD_AL_CMD_DONE)!=0); // retain an early completion while busy
 now=1040;d.s1=0;
 CHECK(_cmd_done(&d)==1); // no new alert needed on next polling round
 CHECK(!(d.alerts & PD_AL_CMD_DONE));
 d.alerts=PD_AL_CMD_DONE;d.s1=PD_S1_CMD_BUSY;now=5000;
 CHECK(_cmd_done(&d)==-1); // a stuck busy bit must still time out
 memset(&d,0,sizeof(d));d.since=900;now=1000;
 CHECK(_cmd_done(&d)==0);
 now=5000;CHECK(_cmd_done(&d)==-1);
 d.alerts=PD_AL_CMD_DONE;d.s1=2<<PD_S1_LASTCMD_SHIFT;now=1000;
 CHECK(_cmd_done(&d)==-2);
 d.state=DOCK_HUB_ACC;d.tries=6;_go(&d,DOCK_HUB_SENT);
 CHECK(d.tries==6); // preserve hub retry count across message re-send
 _go(&d,DOCK_HUB_ACC);CHECK(d.tries==6); // preserve it through reply acceptance too
 for(unsigned i=6;i<10;i++) {d.tries++;_go(&d,DOCK_HUB_SENT);_go(&d,DOCK_HUB_ACC);CHECK(d.tries==i+1);}
 d.state=DOCK_LED_ACC;d.tries=6;_go(&d,DOCK_HUB_SENT);
 CHECK(d.tries==0); // first hub request starts a fresh retry budget
 d.state=DOCK_HUB_ACC;d.tries=10;_go(&d,DOCK_DONE);
 CHECK(d.tries==0);
 d.since=UINT32_MAX-100;now=50;d.s1=0;d.alerts=0;
 CHECK(_cmd_done(&d)==0);
 now=3000;CHECK(_cmd_done(&d)==-1);
 printf("PASS: %u protocol regression checks (no hardware).\n",checks);
 return 0;
}
'''
testfile = tests / 'protocol.c'
testfile.write_text(code)
subprocess.run([str(work / 'toolchains/w64devkit/bin/gcc.exe'), str(testfile),
                '-I' + (work / 'tv-tests/stubs').as_posix(), '-I' + (root / 'bdk').as_posix(),
                '-o', str(tests / 'protocol.exe'), '-Wall', '-Wextra', '-Werror',
                '-Wno-unused-parameter'], check=True)
result = subprocess.run([str(tests / 'protocol.exe')], text=True, capture_output=True)
print(result.stdout.strip() or result.stderr.strip())
if not result.returncode:
    (work / 'tv-protocol-check.json').write_text(json.dumps({
        'result': result.stdout.strip(), 'source': str(driver),
        'source_sha256': hashlib.sha256(driver.read_bytes()).hexdigest(),
        'hardware_validation': False}, indent=2))
raise SystemExit(result.returncode)
