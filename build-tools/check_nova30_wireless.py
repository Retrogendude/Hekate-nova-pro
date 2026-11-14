"""Real Nova decoder/stack tests. Radio and clock are simulated, never RF latency.

Current sources must retain brief presses and account for complete poll work;
the actual v29 sources must fail each targeted assertion. No source is patched.
"""
from pathlib import Path
import ast
import subprocess
import json,hashlib

w = Path(__file__).resolve().parent
cc = w / 'toolchains/w64devkit/bin/gcc.exe'
d = w / 'nova30-wireless-tests'
d.mkdir(exist_ok=True)

test = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nova_bt.h"
#undef assert
#define assert(x) do {if(!(x)){fprintf(stderr,"assertion line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static unsigned now,us,clock_reads,in_at,in_n,packet_cost;
static uint8_t in[16];
uint32_t nova_bt_now(void){return now;}
uint32_t nova_bt_now_us(void){clock_reads++;return ++us;}
int nova_bt_radio_open(void){return 1;}
void nova_bt_radio_close(void){}
int nova_bt_radio_read(void){return in_at<in_n?in[in_at++]:-1;}
int nova_bt_radio_write(uint8_t value){(void)value;return 1;}
#include "nova_bt_stack.c"
#ifdef AUDIT_OLD
static uint32_t nova_bt_take_buttons(unsigned side){return side<2?status.pad[side].buttons:0;}
#endif
static void decode(unsigned side,unsigned buttons,unsigned at)
{
    uint8_t r[14]={0x30};r[3]=buttons;r[4]=buttons>>8;r[5]=buttons>>16;
    r[6]=r[9]=0x45;r[7]=r[10]=0x38;r[8]=r[11]=0x67;
    now=at;assert(nova_bt_decode(&status.pad[side],side,r,sizeof r,now));
}
static void delayed_callback(void *unused){(void)unused;us+=3000;}
static void delayed_packet(uint8_t type,uint8_t *p,uint16_t n)
{(void)type;(void)p;(void)n;us+=packet_cost;}
int main(int argc,char **argv)
{
    nova_bt_peer pair[2]={0};pair[0].valid=1;pair[0].mac[0]=0x40;
    pair[0].host[0]=0x60;memset(pair[0].key,0x70,16);
    assert(nova_bt_start(pair));status.state=NOVA_BT_OFF;
    if(argc>1&&!strcmp(argv[1],"timing")){
        btstack_context_callback_registration_t cb={0};cb.callback=delayed_callback;
        btstack_run_loop_base_add_callback(&cb);us=clock_reads=0;
        nova_bt_poll_fast();
#ifdef AUDIT_OLD
        assert(status.rx_max_us>=3000); /* Old metric excludes the callback. */
#else
        assert(status.rx_max_us<100&&status.poll_max_us>=3000);
        assert(status.poll_fast_max_us==status.poll_max_us&&clock_reads==4);
        unsigned before=us;us+=500;now++;
        nova_bt_poll();assert(status.poll_gap_max_us>=500&&status.poll_normal_max_us>0);
        assert(us-before<520); /* Empty poll adds one clock sample, no wait. */
        receive_packet=delayed_packet;packet_cost=600;
        in[0]=4;in[1]=0x10;in[2]=0;in_at=0;in_n=3;
        nova_bt_poll_fast();
        assert(in_at==3&&status.rx_budget_hits==1&&status.rx_max_us>=600);
        assert(status.poll_fast_max_us>=3000);
#endif
        puts("PASS complete poll timing, bounded clock overhead and packet-callback budget accounting");
    }else{
        decode(1,8,1000);decode(1,0,1017);now=1033;
        assert(!status.pad[1].buttons&&nova_bt_take_buttons(1)==8);
        assert(!nova_bt_take_buttons(1)); /* Next UI read sees release. */
        assert(status.pad[1].x==0x845&&status.pad[1].y==0x673);
        decode(1,8,1040);assert(nova_bt_take_buttons(1)==8);
        assert(nova_bt_take_buttons(1)==8); /* Held state is not consumed. */
        decode(1,0,1050);assert(!nova_bt_take_buttons(1));
        decode(1,8,1100);decode(1,0,1110);decode(1,1,1201);decode(1,0,1202);
        assert(nova_bt_take_buttons(1)==1); /* Recent B cannot revive expired A. */
        decode(1,8,1300);decode(1,0,1310);now=1401;assert(!nova_bt_take_buttons(1));
        decode(1,8,1500);now=2001;assert(!nova_bt_take_buttons(1));
        decode(1,0,2100);decode(1,8,2101);link[1].handle=0x42;
        uint8_t disconnected[]={HCI_EVENT_DISCONNECTION_COMPLETE,4,0,0x42,0,8};
        event(HCI_EVENT_PACKET,0,disconnected,sizeof disconnected);
        assert(!nova_bt_take_buttons(1)&&!status.pad[1].connected);
        uint8_t simple[12]={0x3f,1,0,8};now=2200;
        assert(nova_bt_decode(&status.pad[1],1,simple,12,now));simple[1]=0;
        assert(nova_bt_decode(&status.pad[1],1,simple,12,++now));
        assert(nova_bt_take_buttons(1)==8&&!nova_bt_take_buttons(1));
        decode(0,1u<<16,2300);decode(0,0,2310);assert(nova_bt_take_buttons(0)==(1u<<16));
        assert(!nova_bt_take_buttons(2));
        decode(1,8,0xfffffff0u);decode(1,0,0xfffffff8u);now=10;
        assert(nova_bt_take_buttons(1)==8); /* Wrap-safe ages. */
        decode(1,8,2400);status.state=NOVA_BT_READY;peers[0].valid=0;now=2901;
        nova_bt_poll();assert(!status.pad[1].connected&&!nova_bt_take_buttons(1));
        decode(1,0,3000);decode(1,8,3001);nova_bt_stop();assert(!nova_bt_take_buttons(1));
        assert(nova_bt_start(pair));assert(!nova_bt_take_buttons(1));
        puts("PASS brief presses, consume/release, held state, independent expiry, stale/disconnect/stop, simple/full reports and wrap");
    }
    nova_bt_stop();
}
'''


def compile_test(name, version, source, extra=()):
    p = w / version / 'nyx/nyx_gui'
    lib = p / 'libs/btstack'
    c = d / (name + '.c')
    exe = d / (name + '.exe')
    c.write_text(source)
    sources = [p / 'bluetooth/nova_bt_protocol.c', p / 'bluetooth/nova_bt_trace.c', *lib.rglob('*.c')]
    args = [str(cc), '-std=gnu11', '-O1', '-Wall', '-Wextra', '-ffunction-sections',
            '-fdata-sections', '-Wl,--gc-sections', *extra, '-I' + str(p / 'bluetooth'),
            '-I' + str(lib), str(c), *map(str, sources), '-o', str(exe)]
    r = subprocess.run(args, capture_output=True, text=True)
    assert r.returncode == 0, r.stderr[-8000:]
    return exe


evidence=[]
for old in (False, True):
    exe = compile_test('old' if old else 'current', 'hekate-nova-pro-v29' if old else 'hekate-nova-pro-v30',
                       test, ('-DAUDIT_OLD',) if old else ())
    for mode in ('latch', 'timing'):
        r = subprocess.run([str(exe), mode], capture_output=True, text=True, timeout=15)
        evidence.append({'version':29 if old else 30,'mode':mode,'exit':r.returncode,'output':r.stdout.strip(),'assertion':r.stderr.strip()})
        if old:
            marker = 'nova_bt_take_buttons(1)==8' if mode == 'latch' else 'status.rx_max_us>=3000'
            assert r.returncode == 1 and marker in r.stderr, (r.returncode, r.stdout, r.stderr)
            print('PASS real v29 rejects ' + mode + ' regression: ' + r.stderr.strip())
        else:
            assert r.returncode == 0, r.stderr
            print(r.stdout.strip())

# Retain the existing real H4/BTstack regression suite; the complete-poll sample
# adds one 10-us simulated clock read to its original RX-only cost assertions.
tree = ast.parse((w / 'check_nova29_bt.py').read_text())
base = next(ast.literal_eval(n.value) for n in tree.body if isinstance(n, ast.Assign)
            and any(isinstance(t, ast.Name) and t.id == 'test' for t in n.targets))
base = base.replace('fake_us<=270', 'fake_us<=280').replace('fake_us-before_fast<=27000', 'fake_us-before_fast<=28000')
exe = compile_test('h4-current', 'hekate-nova-pro-v30', base)
r = subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)
assert r.returncode == 0, r.stderr
print(r.stdout.strip())
print('All timings above use a virtual clock; no physical radio latency was measured.')
project=w/'hekate-nova-pro-v30/nyx/nyx_gui/bluetooth'
(w/'nova30-wireless-check.json').write_text(json.dumps({'passed':True,'physical_validation':False,'evidence':evidence,'source_sha256':{n:hashlib.sha256((project/n).read_bytes()).hexdigest()for n in ['nova_bt.h','nova_bt_protocol.c','nova_bt_stack.c']}},indent=2))
