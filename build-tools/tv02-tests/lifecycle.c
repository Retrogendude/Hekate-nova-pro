
#include <bdk.h>
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static int next_state=DOCK_WAIT_PLUG;
void pd_snapshot(pd_snapshot_t*s,bool alert) {assert(!alert);snapshot_calls++;memset(s,0,sizeof(*s));s->ok=id_ok;fake_mux=99;fake_gpio[0]|=2;fake_gpio[1]&=~2;}
void dock_start(dock_t*d) {starts++;memset(d,0,sizeof(*d));d->state=DOCK_WAIT_PLUG;}
void dock_step(dock_t*d) {d->state=next_state;if(next_state==DOCK_FAILED){d->failed_at="TEST";strcpy(d->fail,"injected failure");}}
void dock_stop(dock_t*d) {dock_stops++;d->state=DOCK_IDLE;}
const char *dock_state_name(dock_state_t s) {return "test state";}
int dpout_aux_init(void) {aux_inits++;return aux_fail;}
void dpout_aux_end(void) {aux_ends++;}
bool dpout_hpd(void) {return hpd;}
int dpout_probe_sink(dpout_sink_t*s) {strcpy(s->why,"injected AUX result");return probe_fail;}
int dpout_video_start(dpout_video_t*v,const dpout_sink_t*s,u32 fb) {video_starts++;gpu_dma_active=1;v->lanes=2;return video_fail;}
void dpout_video_stop(void) {video_stops++;gpu_dma_active=0;}
const char *dpout_rate_name(u8 r) {return "HBR2";}
void dpout_set_logger(void(*f)(const char*)) {}
void dpout_lane_test(bool a,bool b) {assert(!a&&!b);}
#include "gui_tv_under_test.c"
static int scenarios;
static void begin(void) {
 if(tv.win) _signal(tv.win,LV_SIGNAL_CLEANUP,NULL);
 memset(&tv,0,sizeof(tv));object_n=0;task_fail=0;hardware=0;alloc_fail=0;
 hwtype=0;id_ok=1;rail_fail=0;aux_fail=0;probe_fail=0;video_fail=0;hpd=1;mounts=1;save_fail=0;
 history_collision_only=0;volume_keys=0;short_write=close_fail=0;next_state=DOCK_WAIT_PLUG;now=100;bpmp_fid_current=3;fake_mux=42;fake_gpio[0]=0;fake_gpio[1]=2;
 gui_tv_open(NULL);scenarios++;
}
static void stopped(void) {
 assert(!tv.running&&!tv.mirrored&&!tv.fb&&!tv.allocation);
 assert(bpmp_fid_current==3&&fake_mux==42&&fake_gpio[0]==0&&fake_gpio[1]==2);
}
int main(void) {
 begin();assert(!hardware&&!tv.running&&!tv.mirrored); // opening is inert
 begin();hwtype=FUSE_NX_HW_TYPE_HOAG;int sn=snapshot_calls;_start(NULL);assert(snapshot_calls==sn&&!hardware);
 begin();id_ok=0;_start(NULL);stopped();assert(strstr(shown,"ne repond pas"));
 begin();rail_fail=1;_start(NULL);stopped();assert(strstr(shown,"alimentations"));
 begin();alloc_fail=1;_start(NULL);stopped();assert(strstr(shown,"insuffisante"));
 begin();_start(NULL);assert(tv.running);int n=starts;_start(NULL);assert(starts==n);now+=45000;_tick(NULL);stopped();assert(strstr(shown,"45 secondes"));
 begin();now=UINT32_MAX-200;_start(NULL);now+=45000;_tick(NULL);stopped(); // timer wrap
 begin();_start(NULL);next_state=DOCK_FAILED;_tick(NULL);stopped();assert(strstr(shown,"injected failure"));
 begin();_start(NULL);next_state=DOCK_DONE;_tick(NULL);stopped(); // done, no HPD/DP
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;aux_fail=1;_tick(NULL);stopped();
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;probe_fail=1;_tick(NULL);stopped();
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;video_fail=1;_tick(NULL);stopped(); // DMA stopped before free
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.mirrored&&tv.fb);
 _tick(NULL);source_frame[10*1280]=0x12345678;source_frame[100*1280]=0xABCDEF12;gui_tv_dirty(10,10);_tick(NULL);
 assert(tv.fb[10*1280]==0x12345678&&tv.fb[100*1280]!=0xABCDEF12); // dirty rows only
 gui_tv_dirty(-100,1000);_tick(NULL);assert(tv.fb[100*1280]==0xABCDEF12); // clipped rows
 hpd=0;_tick(NULL);stopped();hpd=1;_start(NULL);assert(tv.running);_cancel(NULL);stopped();
 begin();_cancel(NULL);_cancel(NULL);stopped(); // repeated stop is safe
 begin();_start(NULL);int td=task_deleted;_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);assert(!tv.win&&!tv.task&&task_deleted==td+1);stopped();
 begin();tv.log_size=0;char longline[40001];memset(longline,'x',40000);longline[40000]=0;_log(longline);assert(tv.log_full&&tv.log_size==0); // bounded log
 begin();_start(NULL);save_fail=1;_cancel(NULL);assert(strstr(shown,"Impossible"));stopped();
 begin();_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);task_fail=1;gui_tv_open(NULL);n=starts;_start(NULL);assert(starts==n); // no task -> no handshake
 begin();_start(NULL);short_write=1;_cancel(NULL);assert(strstr(shown,"Impossible"));stopped();
 begin();_start(NULL);close_fail=1;_cancel(NULL);assert(strstr(shown,"Impossible"));stopped();
 begin();int sv=saved;_cancel(NULL);_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);assert(saved==sv); // preserve last report when no run
 begin();_start(NULL);volume_keys=BTN_VOL_DOWN;_tick(NULL);stopped();assert(strstr(shown,"Test arrete"));
 
 begin();_start(NULL);now+=77;_cancel(NULL);assert(strstr(tv.log,"[00000000 ms] Hekate")&&strstr(tv.log,"[00000077 ms]"));
 char first_path[64];strcpy(first_path,tv.run_path);int hn=history_n;
 _cancel(NULL);assert(history_n==hn&&!strcmp(first_path,tv.run_path)); // same run updates its own report
 _start(NULL);_cancel(NULL);assert(strcmp(first_path,tv.run_path)&&history_n==hn+1); // next attempt gets a new file
 strcpy(first_path,tv.run_path);begin();_start(NULL);_cancel(NULL);assert(strcmp(first_path,tv.run_path)); // simulated reboot preserves files
 begin();_start(NULL);history_collision_only=1;_cancel(NULL);assert(!tv.run_path[0]&&strstr(shown,"Impossible")); // bounded full history, last-run still attempted
 printf("PASS: %d TV lifecycle scenarios; no physical hardware exercised.\n",scenarios);
 return 0;
}
