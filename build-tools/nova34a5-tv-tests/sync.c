
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
static int _wait_sink_sync(dpout_video_t *v)
{
    u32 start = get_tmr_ms();
    bool answered = false;
    v->in_sync = false;
    do {
        u8 status = 0;
        if (!dpout_hpd()) {
            s_printf(v->why, "HPD went away while waiting for video sync");
            return 12;
        }
        if (!dpout_dpcd_read(DPCD_SINK_STATUS, &status, 1)) {
            answered = true;
            v->dpcd_200[5] = status;
            if (status & SINK_PORT0_IN_SYNC) {
                v->in_sync = true;
                return 0;
            }
        }
        msleep(20);
    } while ((u32)(get_tmr_ms() - start) < 1000);
    s_printf(v->why, answered ? "video stream not synchronized after 1000 ms"
                              : "AUX did not answer the video sync check");
    return 12;
}
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
