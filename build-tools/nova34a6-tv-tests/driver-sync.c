#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)

#define DPCD_LANE0_1_STATUS 0x202
#define SINK_PORT0_IN_SYNC 1
#define ALIGN_DONE 1
static u32 now,calls,ready,drop;
static u8 lane_status,align;
static int errors;
u32 get_tmr_ms(void){return now;}
void msleep(u32 ms){now+=ms;}
bool dpout_hpd(void){return calls<drop;}
int dpout_dpcd_read(u32 addr,u8*b,u32 n){
 C(addr==0x202&&n==4);calls++;
 if(errors==1||(errors==2&&calls<4))return -1;
 b[0]=lane_status;b[1]=0;b[2]=align;b[3]=calls>=ready?1:0;return 0;
}
static int _wait_sink_sync(dpout_video_t *v)
{
    u32 start = get_tmr_ms();
    bool answered = false;
    v->in_sync = false;
    do {
        u8 status[4] = { 0 };
        if (!dpout_hpd()) {
            s_printf(v->why, "HPD went away while waiting for video sync");
            return 12;
        }
        if (!dpout_dpcd_read(DPCD_LANE0_1_STATUS, status, sizeof(status))) {
            answered = true;
            memcpy(v->lane_status, status, 3);
            memcpy(v->dpcd_200 + 2, status, sizeof(status));
            bool aligned = (status[2] & ALIGN_DONE) != 0;
            for (u32 lane = 0; lane < v->lanes; lane++)
                aligned &= ((status[lane / 2] >> (4 * (lane & 1))) & 7) == 7;
            if (aligned && (status[3] & SINK_PORT0_IN_SYNC)) {
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
static void reset(dpout_video_t*v){memset(v,0,sizeof(*v));v->in_sync=true;v->lanes=2;now=calls=errors=0;ready=1;drop=UINT32_MAX;lane_status=0x77;align=0x81;}
int main(void){dpout_video_t v;
 reset(&v);C(!_wait_sink_sync(&v)&&v.in_sync&&calls==1);
 reset(&v);ready=26;C(!_wait_sink_sync(&v)&&now==500);
 reset(&v);ready=51;C(_wait_sink_sync(&v)==12&&!v.in_sync&&now==1000&&calls==50);
 reset(&v);errors=1;C(_wait_sink_sync(&v)==12&&!v.in_sync&&strstr(v.why,"AUX"));
 reset(&v);errors=2;C(!_wait_sink_sync(&v)&&now==60);
 reset(&v);drop=0;C(_wait_sink_sync(&v)==12&&!calls&&!v.in_sync);
 reset(&v);drop=4;ready=10;C(_wait_sink_sync(&v)==12&&calls==4);
 reset(&v);now=UINT32_MAX-200;ready=26;C(!_wait_sink_sync(&v)&&now==299);
 reset(&v);now=UINT32_MAX-200;ready=100;C(_wait_sink_sync(&v)==12&&now==799);
 reset(&v);lane_status=0x11;align=0x80;C(_wait_sink_sync(&v)==12&&!v.in_sync); // run-024: clock recovered but EQ lost
 C(v.lane_status[0]==0x11&&v.dpcd_200[4]==0x80);
 reset(&v);lane_status=0x17;C(_wait_sink_sync(&v)==12&&!v.in_sync); // second lane failed despite sink sync
 reset(&v);align=0x80;C(_wait_sink_sync(&v)==12&&!v.in_sync);
 reset(&v);v.lanes=1;lane_status=7;C(!_wait_sink_sync(&v));
 printf("PASS: %u sync assertions, including lost equalization, stale sync, AUX/HPD and timer wrap.\n",checks);
}
