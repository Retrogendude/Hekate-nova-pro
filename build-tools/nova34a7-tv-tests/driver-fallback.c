#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)

static u8 s_stage;static int starts,stops,dumps,hpd,err,stage;
static u8 pass_rate,rates[4];
bool dpout_hpd(void){return hpd;}
const char*dpout_rate_name(u8 rate){return rate==0x14?"HBR2":rate==0x0A?"HBR":"RBR";}
static void _vlog(dpout_video_t*v,const char*line){}
static void _dump(dpout_video_t*v){dumps++;}
void dpout_video_stop(void){C(s_stage==3);stops++;s_stage=0;}
static int _video_start(dpout_video_t*v,const dpout_sink_t*s,u32 fb,u8 cap){
 C(starts<3);u8 actual=s->dpcd[1]<cap?s->dpcd[1]:cap;
 v->rate=actual;v->lane_status[0]=0x11;rates[starts++]=actual;s_stage=stage;
 return actual<=pass_rate?0:err;
}
int dpout_video_start(dpout_video_t *v, const dpout_sink_t *s, u32 fb)
{
    u8 max_rate = 0x14;
    int r;
    /* Training may pass but lose equalization when the video stream begins.
     * Retry the video link at the next lower standard rate before tearing
     * down the PD/dock session. Keep the first, previously working rate. */
    for (;;) {
        r = _video_start(v, s, fb, max_rate);
        if (r != 12 || s_stage < 3 || !dpout_hpd() || v->rate <= 0x06)
            break;
        u8 next = v->rate > 0x0A ? 0x0A : 0x06;
        char line[160];
        s_printf(line, "video did not synchronize at %s: lanes %02X %02X %02X; retrying %s",
                 dpout_rate_name(v->rate), v->lane_status[0], v->lane_status[1],
                 v->lane_status[2], dpout_rate_name(next));
        _vlog(v, line);
        dpout_video_stop();
        max_rate = next;
    }
    if (s_stage)
        _dump(v);
    return r;
}
static void reset(dpout_sink_t*s){memset(s,0,sizeof(*s));s->dpcd[1]=0x14;s_stage=starts=stops=dumps=0;hpd=1;err=12;stage=3;pass_rate=0;}
int main(void){dpout_sink_t s;dpout_video_t v={0};
 reset(&s);pass_rate=0x14;C(!dpout_video_start(&v,&s,1)&&starts==1&&!stops&&dumps==1);
 reset(&s);pass_rate=0x0A;C(!dpout_video_start(&v,&s,1)&&starts==2&&stops==1&&rates[0]==0x14&&rates[1]==0x0A);
 reset(&s);pass_rate=0x06;C(!dpout_video_start(&v,&s,1)&&starts==3&&stops==2&&rates[2]==6);
 reset(&s);C(dpout_video_start(&v,&s,1)==12&&starts==3&&stops==2&&dumps==1);
 reset(&s);s.dpcd[1]=0x0A;pass_rate=6;C(!dpout_video_start(&v,&s,1)&&starts==2&&rates[0]==0x0A&&rates[1]==6);
 reset(&s);s.dpcd[1]=6;C(dpout_video_start(&v,&s,1)==12&&starts==1&&!stops);
 reset(&s);stage=0;C(dpout_video_start(&v,&s,1)==12&&starts==1&&!stops&&!dumps);
 reset(&s);err=11;C(dpout_video_start(&v,&s,1)==11&&starts==1&&!stops);
 reset(&s);hpd=0;C(dpout_video_start(&v,&s,1)==12&&starts==1&&!stops);
 printf("PASS: %u fallback assertions; HBR2/HBR/RBR, cleanup order, sink cap, early error and unplug.\n",checks);
}
