#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)

#define STEP_TIMEOUT_MS 2500
#define STOP_HPD_EXIT_DP_CMD 0x3B3B
#define DP_ENTER_MODE_CMD 0x3131
static u32 now;static int commands;static u16 last_cmd;
u32 get_tmr_ms(void){return now;}
static void _cmd(u16 cmd){last_cmd=cmd;commands++;}
const char*pd_lastcmd_name(u16 st){return "injected refusal";}
static const char *s_state_name[] = {
    "idle", "waiting for a cable", "leaving the previous DisplayPort mode", "asking for power", "sink ready",
    "asking to be the data host", "asking who it is", "reading who it is",
    "entering the dock's mode", "the dock's mode", "asking for DisplayPort",
    "reading DisplayPort support", "asking for DisplayPort mode",
    "reading DisplayPort mode", "entering DisplayPort", "configuring DisplayPort",
    "waiting for the TV's hot plug detect",
    "asking the dock's state", "reading the dock's state", "lighting the dock",
    "the dock's light", "powering the dock's hub", "the dock's hub",
    "done", "failed",
};
const char *dock_state_name(dock_state_t s)
{
    return s_state_name[s];
}
static void _go(dock_t *d, dock_state_t s)
{
    bool retrying_hub = (d->state == DOCK_HUB_ACC && s == DOCK_HUB_SENT) ||
                        (d->state == DOCK_HUB_SENT && s == DOCK_HUB_ACC);
    d->state = s;
    d->since = get_tmr_ms();
    if (!retrying_hub)
        d->tries = 0;
    s_printf(d->log, "gui: dock %s: alert %04X s1 %04X s2 %04X dp %04X\n",
             dock_state_name(s), d->alerts, d->s1, d->s2, d->dp);
    d->log_new = true;
}
static void _fail(dock_t *d, const char *why)
{
    d->failed_state = d->state;
    d->failed_at = dock_state_name(d->state);
    s_printf(d->fail, "%s", why);
    _go(d, DOCK_FAILED);
}
static bool _take(dock_t *d, u16 bit)
{
    if (!(d->alerts & bit))
        return false;
    d->alerts &= ~bit;
    return true;
}
static bool _timed_out(dock_t *d)
{
    return get_tmr_ms() - d->since > STEP_TIMEOUT_MS;
}
static void _cmd_go(dock_t *d, u16 cmd, dock_state_t next)
{
    d->alerts &= ~PD_AL_CMD_DONE;
    if (cmd == DP_ENTER_MODE_CMD)
        d->dp_on = true;
    _cmd(cmd);
    _go(d, next);
}
static int _cmd_done(dock_t *d)
{
    if (!(d->alerts & PD_AL_CMD_DONE))
        return _timed_out(d) ? -1 : 0;
    if (d->s1 & PD_S1_CMD_BUSY)
        return _timed_out(d) ? -1 : 0;
    _take(d, PD_AL_CMD_DONE);
    if ((d->s1 & PD_S1_LASTCMD_MASK) != 0)
        return -2;
    return 1;
}
static void _cmd_fail(dock_t *d, const char *what, int r)
{
    char why[96];
    if (r == -1)
        s_printf(why, "no answer to %s", what);
    else
        s_printf(why, "%s was refused (last command %s)", what, pd_lastcmd_name(d->s1));
    _fail(d, why);
}
static bool _prepare_existing_dp(dock_t *d)
{
    u16 active = d->dp & PD_DP_EN;
    if (!d->dp_prepared) {
        d->dp_prepared = true;
        if (active) {
            d->dp_on = true; /* Stop must also clean a failed exit. */
            _cmd_go(d, STOP_HPD_EXIT_DP_CMD, DOCK_EXIT_DP_SENT);
            return false;
        }
    } else if (d->dp & PD_DP_EN) {
        /* SIGNAL_ON may remain set while the TV is still connected. It is
         * not proof that the controller failed to leave its DP mode. */
        _fail(d, "the previous DisplayPort mode did not exit");
        return false;
    }
    return true;
}
static void _finish_existing_dp(dock_t *d)
{
    int r = _cmd_done(d);
    if (r < 0)
        _cmd_fail(d, "leaving the previous DisplayPort mode", r);
    else if (r) {
        d->dp_on = false;
        d->plug_seen = false;
        _go(d, DOCK_WAIT_PLUG);
    }
}
static void reset(dock_t*d){memset(d,0,sizeof(*d));d->state=DOCK_WAIT_PLUG;d->dp=0x4000;now=100;commands=0;}
int main(void){dock_t d;
 C(sizeof(s_state_name)/sizeof(s_state_name[0])==DOCK_FAILED+1);
 reset(&d);C(_prepare_existing_dp(&d)&&!commands&&d.dp_prepared); // clean boot unchanged
 reset(&d);d.dp=0xC082;d.alerts=PD_AL_CMD_DONE;
 C(!_prepare_existing_dp(&d)&&commands==1&&last_cmd==0x3B3B&&d.state==DOCK_EXIT_DP_SENT&&d.dp_on);
 C(!(d.alerts&PD_AL_CMD_DONE));_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT); // stale completion cannot acknowledge exit
 d.alerts=PD_AL_CMD_DONE;d.s1=PD_S1_CMD_BUSY;_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT);
 d.s1=PD_S1_INSERT;d.dp=0x4000;d.plug_seen=true;_finish_existing_dp(&d);
 C(d.state==DOCK_WAIT_PLUG&&!d.dp_on&&!d.plug_seen&&d.dp_prepared);
 C(_prepare_existing_dp(&d)&&commands==1); // actual enabled mode cleaned before Discover Identity
 d.dp=0x4182;C(_prepare_existing_dp(&d)&&commands==1); // TV-connected signal may stay high after acknowledged exit
 reset(&d);d.dp=0xC082;C(!_prepare_existing_dp(&d));d.alerts=PD_AL_CMD_DONE;_finish_existing_dp(&d);
 C(!_prepare_existing_dp(&d)&&d.state==DOCK_FAILED&&strstr(d.fail,"did not exit")); // no repeated commands if bits remain
 reset(&d);d.dp=0xC082;_prepare_existing_dp(&d);now+=2501;_finish_existing_dp(&d);
 C(d.state==DOCK_FAILED&&d.dp_on); // caller must still clean a timed out exit
 reset(&d);d.dp=0xC082;_prepare_existing_dp(&d);d.alerts=PD_AL_CMD_DONE;d.s1=2<<PD_S1_LASTCMD_SHIFT;_finish_existing_dp(&d);
 C(d.state==DOCK_FAILED&&strstr(d.fail,"refused"));
 reset(&d);now=UINT32_MAX-100;d.dp=0xC082;_prepare_existing_dp(&d);now=50;_finish_existing_dp(&d);C(d.state==DOCK_EXIT_DP_SENT);
 now=3000;_finish_existing_dp(&d);C(d.state==DOCK_FAILED);
 reset(&d);d.dp=0x4002;C(_prepare_existing_dp(&d)&&!commands); // configured pin latch alone is not active video
 reset(&d);d.dp=0x4182;d.s1=0x46F0;C(_prepare_existing_dp(&d)&&!commands&&!d.dp_on); // run-026/028: TV connected with DP already disabled must not issue refused exit
 printf("PASS: %u inherited-DP assertions; clean boot, stale mode, ack/busy, timeout, refusal and timer wrap.\n",checks);
}
