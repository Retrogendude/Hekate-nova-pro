"""Real driver helpers with simulated registers; no physical hardware access."""
from pathlib import Path
import hashlib, json, re, subprocess
w=Path(__file__).resolve().parent; p=w/'hekate-nova-pro-v34-alpha6'; d=w/'nova34a6-tv-tests'
dp=(p/'bdk/display/tv/dpout.c').read_text(); pd=(p/'bdk/display/tv/usbpd.c').read_text()
def fn(s,name):
    m=re.search(r'^(?:static )?(?:int|bool|void|const char \*)\s*'+name+r'\([^;{}]*\)\s*\n\{',s,re.M)
    assert m,name
    a=s.index('{',m.start()); i=a+1; depth=1
    while depth: depth+=(s[i]=='{')-(s[i]=='}'); i+=1
    return s[m.start():i]
base='''#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)
'''
results={}
def run(name,code):
    f=d/(name+'.c'); f.write_text(base+code)
    subprocess.run([str(w/'toolchains/w64devkit/bin/gcc.exe'),str(f),'-I'+str(p/'bdk'),'-std=gnu11',
                    '-Wall','-Wextra','-Werror','-Wno-unused-parameter','-o',str(f.with_suffix('.exe'))],check=True)
    r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True)
    assert r.returncode==0,r.stdout+r.stderr
    results[name]=r.stdout.strip();print(r.stdout.strip())

sync=r'''
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
'''+fn(dp,'_wait_sink_sync')+r'''
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
'''
run('driver-sync',sync)

fallback=r'''
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
'''+fn(dp,'dpout_video_start')+r'''
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
'''
run('driver-fallback',fallback)

pd_prefix=r'''
#define STEP_TIMEOUT_MS 2500
#define STOP_HPD_EXIT_DP_CMD 0x3B3B
#define DP_ENTER_MODE_CMD 0x3131
static u32 now;static int commands;static u16 last_cmd;
u32 get_tmr_ms(void){return now;}
static void _cmd(u16 cmd){last_cmd=cmd;commands++;}
const char*pd_lastcmd_name(u16 st){return "injected refusal";}
'''
array=re.search(r'static const char \*s_state_name\[\] = \{.*?\n\};',pd,re.S).group()
helpers='\n'.join(fn(pd,n) for n in ('dock_state_name','_go','_fail','_take','_timed_out','_cmd_go','_cmd_done','_cmd_fail','_prepare_existing_dp','_finish_existing_dp'))
pd_case=pd_prefix+array+'\n'+helpers+r'''
static void reset(dock_t*d){memset(d,0,sizeof(*d));d->state=DOCK_WAIT_PLUG;d->dp=0x4000;now=100;commands=0;}
int main(void){dock_t d;
 C(sizeof(s_state_name)/sizeof(s_state_name[0])==DOCK_FAILED+1);
 reset(&d);C(_prepare_existing_dp(&d)&&!commands&&d.dp_prepared); // clean boot unchanged
 reset(&d);d.dp=0x4182;d.alerts=PD_AL_CMD_DONE;
 C(!_prepare_existing_dp(&d)&&commands==1&&last_cmd==0x3B3B&&d.state==DOCK_EXIT_DP_SENT&&d.dp_on);
 C(!(d.alerts&PD_AL_CMD_DONE));_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT); // stale completion cannot acknowledge exit
 d.alerts=PD_AL_CMD_DONE;d.s1=PD_S1_CMD_BUSY;_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT);
 d.s1=PD_S1_INSERT;d.dp=0x4000;d.plug_seen=true;_finish_existing_dp(&d);
 C(d.state==DOCK_WAIT_PLUG&&!d.dp_on&&!d.plug_seen&&d.dp_prepared);
 C(_prepare_existing_dp(&d)&&commands==1); // run-021/022 inherited mode cleaned before Discover Identity
 d.dp=0x4182;C(_prepare_existing_dp(&d)&&commands==1); // TV-connected signal may stay high after acknowledged exit
 reset(&d);d.dp=0xC082;C(!_prepare_existing_dp(&d));d.alerts=PD_AL_CMD_DONE;_finish_existing_dp(&d);
 C(!_prepare_existing_dp(&d)&&d.state==DOCK_FAILED&&strstr(d.fail,"did not exit")); // no repeated commands if bits remain
 reset(&d);d.dp=0x4182;_prepare_existing_dp(&d);now+=2501;_finish_existing_dp(&d);
 C(d.state==DOCK_FAILED&&d.dp_on); // caller must still clean a timed out exit
 reset(&d);d.dp=0x4182;_prepare_existing_dp(&d);d.alerts=PD_AL_CMD_DONE;d.s1=2<<PD_S1_LASTCMD_SHIFT;_finish_existing_dp(&d);
 C(d.state==DOCK_FAILED&&strstr(d.fail,"refused"));
 reset(&d);now=UINT32_MAX-100;d.dp=0x4182;_prepare_existing_dp(&d);now=50;_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT);
 now=3000;_finish_existing_dp(&d);C(d.state==DOCK_FAILED);
 reset(&d);d.dp=0x4002;C(_prepare_existing_dp(&d)&&!commands); // configured pin latch alone is not active video
 printf("PASS: %u inherited-DP assertions; clean boot, stale mode, ack/busy, timeout, refusal and timer wrap.\n",checks);
}
'''
run('driver-pd',pd_case)

defs=dp[dp.index('typedef struct {\n    u32 pclk_khz'):dp.index('/* How far the last start got:')]
calc=defs+fn(dp,'_link_calc')+r'''
int main(void){dpout_link_t l={0};l.lanes=2;l.enhanced=true;
 u8 rates[]={0x14,0x0A,6};for(u32 i=0;i<3;i++){l.bw=rates[i];C(!_link_calc(&l,&s_mode));C(l.tu>=32&&l.tu<=64&&l.active_count<l.tu&&l.watermark<=30);}
 l.lanes=1;l.bw=6;C(_link_calc(&l,&s_mode)!=0);
 printf("PASS: %u real 720p60 link-budget/TU assertions at all fallback rates.\n",checks);
}
'''
run('driver-link-budget',calc)
wait_body=fn(pd,'dock_step')
assert wait_body.index('d->s1 & PD_S1_FAULT_MASK')<wait_body.index('_prepare_existing_dp(d)')<wait_body.index('d->pdo_n = _read_pdos')
assert 'case DOCK_EXIT_DP_SENT:\n        _finish_existing_dp(d);' in wait_body
assert 'if (max > max_rate)' in fn(dp,'_video_start')
(w/'nova34a6-tv-driver-check.json').write_text(json.dumps({'passed':True,'tests':results,
 'real_driver_helpers':True,'hardware_validated':False,'source_sha256':{
 'bdk/display/tv/dpout.c':hashlib.sha256((p/'bdk/display/tv/dpout.c').read_bytes()).hexdigest(),
 'bdk/display/tv/usbpd.c':hashlib.sha256((p/'bdk/display/tv/usbpd.c').read_bytes()).hexdigest()}},indent=2))
