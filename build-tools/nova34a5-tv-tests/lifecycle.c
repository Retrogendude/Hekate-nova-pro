
#include <bdk.h>
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static int next_state=DOCK_WAIT_PLUG;
static int passive_calls,passive_result,passive_inserted,passive_dock;
int pd_dock_attached(bool*inserted,bool*dock) {passive_calls++;*inserted=passive_inserted;*dock=passive_dock;return passive_result;}

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
 gui_tv_shutdown(false);if(tv.win) _signal(tv.win,LV_SIGNAL_CLEANUP,NULL);
 memset(&tv,0,sizeof(tv));n_cfg.tv_auto=0;n_cfg.tv_keep_screen=1;h_cfg.backlight=130;bl_level=110;bl_calls=0;passive_result=passive_calls=passive_inserted=passive_dock=0;object_n=0;task_fail=0;hardware=0;alloc_fail=0;
 hwtype=0;id_ok=1;rail_fail=0;aux_fail=0;probe_fail=0;video_fail=0;hpd=1;mounts=0;save_fail=0;
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
 begin();id_ok=0;_start(NULL);stopped();assert(strstr(shown,"did not respond"));
 begin();rail_fail=1;_start(NULL);stopped();assert(strstr(shown,"power state"));
 begin();alloc_fail=1;_start(NULL);stopped();assert(strstr(shown,"Not enough memory"));
 begin();_start(NULL);assert(tv.running);int n=starts;_start(NULL);assert(starts==n);now+=45000;_tick(NULL);stopped();assert(strstr(shown,"timed out"));
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
 begin();_start(NULL);lv_task_t*kept=tv.task;_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);
 assert(!tv.win&&!tv.status&&tv.task==kept&&tv.running);_tick(NULL);assert(tv.running);
 gui_tv_open(NULL);assert(tv.win&&tv.status&&tv.running&&tv.task==kept);_cancel(NULL);stopped();
 begin();tv.log_size=0;char longline[40001];memset(longline,'x',40000);longline[40000]=0;_log(longline);assert(tv.log_full&&tv.log_size==0); // bounded log
 begin();_start(NULL);save_fail=1;_cancel(NULL);assert(strstr(shown,"could not be saved"));stopped();
 begin();_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);task_fail=1;gui_tv_open(NULL);n=starts;_start(NULL);assert(starts==n); // no task -> no handshake
 begin();_start(NULL);short_write=1;_cancel(NULL);assert(strstr(shown,"could not be saved"));stopped();
 begin();_start(NULL);close_fail=1;_cancel(NULL);assert(strstr(shown,"could not be saved"));stopped();
 begin();int sv=saved;_cancel(NULL);_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);assert(saved==sv); // preserve last report when no run
 begin();_start(NULL);volume_keys=BTN_VOL_DOWN;_tick(NULL);stopped();assert(strstr(shown,"TV stopped"));
 
 begin();_start(NULL);now+=77;_cancel(NULL);assert(strstr(tv.log,"[00000000 ms] Hekate")&&strstr(tv.log,"[00000077 ms]"));
 char first_path[64];strcpy(first_path,tv.run_path);int hn=history_n;
 _cancel(NULL);assert(history_n==hn&&!strcmp(first_path,tv.run_path)); // same run updates its own report
 _start(NULL);_cancel(NULL);assert(strcmp(first_path,tv.run_path)&&history_n==hn+1); // next attempt gets a new file
 strcpy(first_path,tv.run_path);begin();_start(NULL);_cancel(NULL);assert(strcmp(first_path,tv.run_path)); // simulated reboot preserves files
 begin();_start(NULL);history_collision_only=1;_cancel(NULL);assert(!tv.run_path[0]&&strstr(shown,"could not be saved")); // bounded full history, last-run still attempted
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.mirrored&&bpmp_fid_current==3);
 u32*samefb=tv.fb;_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);assert(tv.mirrored&&tv.fb==samefb&&!tv.status);
 int updates=ui_updates;source_frame[0]=0xAB123456;gui_tv_dirty(0,0);_tick(NULL);assert(tv.fb[0]==0xAB123456&&ui_updates==updates);
 hpd=0;_tick(NULL);stopped();assert(!tv.task&&strstr(tv.message,"disconnected"));
 begin();_start(NULL);int sv2=saved;hw_display_external_end();stopped();assert(!tv.task&&saved==sv2); // no SD access after sd_end
 begin();_start(NULL);gui_tv_shutdown(true);stopped();assert(!tv.task); // explicit shutdown with report
 begin();_start(NULL);mounts=1;sv2=saved;_cancel(NULL);assert(saved==sv2&&strstr(shown,"could not be saved"));stopped();
 
 begin();_signal(tv.win,LV_SIGNAL_CLEANUP,NULL);gui_tv_auto_configure();now+=10000;_tick(NULL);assert(!tv.task&&!passive_calls); // OFF has no polling task
 begin();n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();n=starts;
 now+=1999;_tick(NULL);assert(!passive_calls&&starts==n);now++;_tick(NULL);assert(passive_calls==1&&!tv.running);
 now+=500;_tick(NULL);assert(tv.running&&starts==n+1);_cancel(NULL);now+=5000;_tick(NULL);assert(starts==n+1&&!tv.running); // VOL/Stop holds current connection
 passive_inserted=passive_dock=0;now+=500;_tick(NULL);assert(!tv.auto_attempted);
 passive_inserted=passive_dock=1;now+=500;_tick(NULL);now+=500;_tick(NULL);assert(tv.running&&starts==n+2);_cancel(NULL); // re-dock rearms
 begin();n_cfg.tv_auto=1;passive_inserted=passive_dock=1;id_ok=0;gui_tv_auto_configure();now+=2000;_tick(NULL);now+=500;_tick(NULL);assert(!tv.running&&tv.auto_attempted);
 sn=snapshot_calls;now+=5000;_tick(NULL);assert(snapshot_calls==sn); // failed start not repeated continuously
 begin();n_cfg.tv_auto=1;passive_inserted=1;passive_dock=0;gui_tv_auto_configure();n=starts;
 for(int i=0;i<10;i++){now+=500;_tick(NULL);}assert(starts==n&&!tv.running); // charger is ignored
 begin();n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();now+=2000;_tick(NULL);passive_result=1;
 now+=500;_tick(NULL);assert(!tv.auto_samples&&!tv.running);passive_result=0;now+=2000;_tick(NULL);assert(!tv.running);now+=500;_tick(NULL);assert(tv.running);
 _cancel(NULL);passive_result=1;now+=500;_tick(NULL);assert(tv.auto_attempted);passive_result=0;now+=500;_tick(NULL);assert(!tv.running); // I2C error cannot fake removal
 begin();n_cfg.tv_auto=1;hwtype=FUSE_NX_HW_TYPE_HOAG;gui_tv_auto_configure();now+=10000;_tick(NULL);assert(!passive_calls&&!hardware); // no auto on Lite
 begin();n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();_start(NULL);gui_tv_shutdown(false);assert(tv.auto_suspended&&!tv.task);
 n=passive_calls;now+=10000;_tick(NULL);assert(passive_calls==n);gui_tv_resume_detection();assert(tv.task&&!tv.auto_suspended);
 now+=2000;_tick(NULL);now+=500;_tick(NULL);assert(tv.running);_cancel(NULL); // resume after exclusive operation
 begin();n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();_start(NULL);_cancel(NULL);gui_tv_shutdown(false);gui_tv_resume_detection();
 now+=3000;_tick(NULL);now+=500;_tick(NULL);assert(!tv.running&&tv.auto_attempted); // manual stop survives diagnostic suspension
 begin();n_cfg.tv_keep_screen=0;_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.mirrored&&gui_tv_console_dark()&&bl_level==0);
 h_cfg.backlight=190;hpd=0;_tick(NULL);stopped();assert(!gui_tv_console_dark()&&bl_level==170); // current brightness restored
 begin();n_cfg.tv_keep_screen=0;_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);int bc=bl_calls;
 hw_display_external_end();stopped();assert(!gui_tv_console_dark()&&bl_level==0&&bl_calls==bc); // no backlight flash before OS teardown
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(!tv.console_dark);
 n_cfg.tv_keep_screen=0;gui_tv_screen_configure();assert(tv.console_dark&&bl_level==0);n_cfg.tv_keep_screen=1;gui_tv_screen_configure();assert(!tv.console_dark&&bl_level==110);
 
 begin();_start(NULL);n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();_cancel(NULL);n=starts;
 now+=3000;_tick(NULL);now+=500;_tick(NULL);assert(starts==n&&!tv.running&&tv.auto_attempted); // enable while active then Stop holds
 begin();n_cfg.tv_auto=1;passive_result=1;gui_tv_auto_configure();now+=2000;_tick(NULL);assert(passive_calls==1);
 now+=500;_tick(NULL);assert(passive_calls==1);now+=1500;_tick(NULL);now+=2000;_tick(NULL);assert(passive_calls==3&&tv.auto_errors==3);
 now+=2000;_tick(NULL);assert(passive_calls==3);now+=3000;_tick(NULL);assert(passive_calls==4); // I2C failures back off
 begin();now=UINT32_MAX-1000;n_cfg.tv_auto=1;passive_inserted=passive_dock=1;gui_tv_auto_configure();now+=2000;_tick(NULL);now+=500;_tick(NULL);assert(tv.running); // auto timer rollover

 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.mirrored);
 _signal(tv.win,LV_SIGNAL_CLEANUP,NULL);int io=sd_calls,opened=saved;
 source_frame[12*1280]=0x11223344;gui_tv_dirty(12,12);now+=40;gui_tv_service();assert(tv.fb[12*1280]==0x11223344&&tv.mirrored&&saved==opened&&sd_calls==io);
 source_frame[13*1280]=0x55667788;gui_tv_dirty(13,13);gui_tv_present();assert(tv.fb[13*1280]==0x55667788&&sd_calls==io); // no scheduler delay after a complete refresh
 hpd=0;now+=40;gui_tv_service();assert(!tv.mirrored&&tv.report_pending&&tv.task&&saved==opened&&sd_calls==io);
 _tick(NULL);assert(!tv.report_pending&&saved>opened&&sd_calls>io&&!tv.task); // flush only on ordinary event loop
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;io=sd_calls;opened=saved;
 now+=40;gui_tv_service();assert(tv.mirrored&&tv.report_pending&&saved==opened&&sd_calls==io); // pending handshake may finish while busy, without touching SD
 int dp=starts;gui_tv_service();assert(starts==dp&&sd_calls==io); // 40ms budget
 tv.servicing=true;now+=40;gui_tv_service();tv.servicing=false;assert(sd_calls==io);
 _tick(NULL);assert(!tv.report_pending&&saved>opened); // eventually persist deferred log
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;now+=40;gui_tv_service();assert(tv.report_pending);
 opened=saved;hw_display_external_end();assert(!tv.report_pending&&!tv.task&&saved==opened); // no deferred storage access after teardown
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);io=sd_calls;opened=saved;
 volume_keys=BTN_VOL_DOWN;now+=40;gui_tv_service();assert(!tv.mirrored&&tv.report_pending&&saved==opened&&sd_calls==io);volume_keys=0;_tick(NULL);assert(!tv.report_pending);
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);now=UINT32_MAX-5;tv.service_at=now;now+=45;source_frame[0]=123;gui_tv_dirty(0,0);gui_tv_service();assert(tv.fb[0]==123); // unsigned timer wrap

 begin();_start(NULL);passive_inserted=passive_dock=1;next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;aux_fail=1;_tick(NULL);assert(tv.retry_pending&&tv.retries_left==1);
 n=starts;now+=1499;_tick(NULL);assert(starts==n);now++;_tick(NULL);assert(starts==n+1&&tv.running);
 tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.retry_pending&&!tv.retries_left);now+=3000;_tick(NULL);assert(starts==n+2&&tv.running);
 tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(!tv.retry_pending&&!tv.running);now+=10000;_tick(NULL);assert(starts==n+2); // initial + two retries maximum
 begin();_start(NULL);passive_inserted=passive_dock=1;next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;video_fail=1;_tick(NULL);assert(tv.retry_pending);n=starts;_cancel(NULL);now+=10000;_tick(NULL);assert(starts==n&&!tv.retry_pending);
 begin();_start(NULL);next_state=DOCK_FAILED;tv.dock.s1=PD_S1_FAULT_MASK;_tick(NULL);assert(!tv.retry_pending); // controller fault is never retried
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;aux_fail=1;_tick(NULL);assert(tv.retry_pending);n=starts;passive_inserted=passive_dock=0;now+=2000;_tick(NULL);assert(!tv.retry_pending&&starts==n);
 begin();_start(NULL);passive_inserted=passive_dock=1;next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;probe_fail=1;_tick(NULL);assert(tv.retry_pending);probe_fail=0;now+=1500;_tick(NULL);tv.dock.dp_on=tv.dock.hpd=true;_tick(NULL);assert(tv.mirrored&&!tv.retry_pending); // recovery succeeds
 begin();_start(NULL);next_state=DOCK_DONE;tv.dock.dp_on=tv.dock.hpd=true;aux_fail=1;_tick(NULL);assert(tv.retry_pending);hw_display_external_end();assert(!tv.retry_pending&&!tv.task);
 printf("PASS: %d TV lifecycle scenarios; no physical hardware exercised.\n",scenarios);
 return 0;
}
