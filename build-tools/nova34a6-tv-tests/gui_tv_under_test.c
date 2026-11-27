#define NYX_NOVA_VERSION "34-alpha6"
static void gui_bluetooth_shutdown(void){}
/* Optional dock service. Automatic detection is disabled unless enabled in Nyx.
 * Integration code licensed under GPL-2.0, as Hekate.
 * Dock/video driver provenance is recorded in TV-SOURCES.md.
 */
#include <bdk.h>
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
#include <power/max77620.h>
#include <libs/fatfs/ff.h>
#include <string.h>







#define TV_LOG_SIZE 32768
#define TV_RUN_LIMIT_MS 45000
extern bpmp_freq_t bpmp_fid_current;
static struct {
    lv_obj_t *win, *status, *panel;
    lv_task_t *task;
    lv_signal_func_t signal;
    dock_t dock;
    dpout_sink_t sink;
    dpout_video_t video;
    pd_snapshot_t snap;
    void *allocation;
    u32 *fb;
    u32 started, log_size;
    bpmp_freq_t clock;
    bool clock_saved, running, mirrored, rails_saved, log_full;
    bool pad_saved, hpd_gpio, hpd_oe;
    bool auto_attempted, auto_suspended, resume_arm;
    bool console_dark, hardware_exit;
    u32 auto_since, auto_poll_at, auto_samples, auto_errors, service_at;
    bool cooperative, servicing, report_pending, retry_pending, retry_start;
    u32 retry_at,retry_delay,retries_left;
    u32 hpd_mux;
    u8 rails[2];
    int dirty_first, dirty_last;
    char log[TV_LOG_SIZE], message[768], run_path[64];
} tv;

static lv_res_t _cancel(lv_obj_t *btn);
static lv_res_t _begin(bool automatic);
static void _tick(void *param);
static void _retry(void);

bool gui_tv_console_dark(void)
{
    return tv.console_dark;
}

void gui_tv_screen_configure(void)
{
    bool dark = tv.mirrored && !n_cfg.tv_keep_screen;
    if (dark && !tv.console_dark) {
        display_backlight_brightness(0, 150);
        tv.console_dark = true;
    } else if (!dark && tv.console_dark) {
        if (!tv.hardware_exit) {
            int level = (int)h_cfg.backlight - 20;
            if (level < 10) level = 10;
            if (level > 200) level = 200;
            display_backlight_brightness(level, 150);
        }
        tv.console_dark = false;
    }
}

static bool _auto_enabled(void)
{
    return n_cfg.tv_auto == 1 && !tv.auto_suspended && fuse_read_hw_type() != FUSE_NX_HW_TYPE_HOAG;
}

static void _auto_poll(void)
{
    u32 now = get_tmr_ms();
    u32 interval = tv.auto_errors >= 3 ? 5000 : tv.auto_errors ? 2000 : 500;
    if (!_auto_enabled() || (u32)(now - tv.auto_since) < 2000 ||
        (u32)(now - tv.auto_poll_at) < interval)
        return;
    tv.auto_poll_at = now;
    bool inserted, dock;
    bpmp_freq_t clock = bpmp_fid_current;
    bpmp_clk_rate_set(BPMP_CLK_NORMAL);
    int r = pd_dock_attached(&inserted, &dock);
    bpmp_clk_rate_set(clock);
    if (r) {
        if (tv.auto_errors < 3) tv.auto_errors++;
        tv.auto_samples = 0; // An I2C error is not an unplug event.
        return;
    }
    tv.auto_errors = 0;
    if (!inserted) {
        tv.auto_attempted = false;
        tv.auto_samples = 0;
        return;
    }
    if (!dock || tv.auto_attempted) {
        tv.auto_samples = 0;
        return;
    }
    if (++tv.auto_samples >= 2) {
        tv.auto_samples = 0;
        tv.auto_attempted = true; // One automatic attempt per physical insertion.
        _begin(true);
    }
}

void gui_tv_auto_configure(void)
{
    tv.auto_suspended = false;
    tv.resume_arm = false;
    tv.auto_attempted = false;
    tv.auto_samples = 0;
    tv.auto_errors = 0;
    tv.auto_since = tv.auto_poll_at = get_tmr_ms();
    if (_auto_enabled() && !tv.task)
        tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
    if (!_auto_enabled() && !tv.running && !tv.mirrored && !tv.win && tv.task) {
        lv_task_del(tv.task);
        tv.task = NULL;
    }
}

void gui_tv_resume_detection(void)
{
    if (!tv.auto_suspended)
        return;
    tv.auto_suspended = false;
    if (tv.resume_arm)
        tv.auto_attempted = false;
    tv.resume_arm = false;
    tv.auto_samples = 0;
    tv.auto_errors = 0;
    tv.auto_since = tv.auto_poll_at = get_tmr_ms();
    if (_auto_enabled() && !tv.task)
        tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
}

static void _log(const char *line)
{
    u32 n = strlen(line);
    char prefix[24];
    s_printf(prefix, "[%08d ms] ", (u32)(get_tmr_ms() - tv.started));
    u32 p = strlen(prefix);
    if (tv.log_size + n + p + 2 >= TV_LOG_SIZE) {
        tv.log_full = true;
        return;
    }
    memcpy(tv.log + tv.log_size, prefix, p);
    tv.log_size += p;
    memcpy(tv.log + tv.log_size, line, n);
    tv.log_size += n;
    tv.log[tv.log_size++] = '\n';
    tv.log[tv.log_size] = 0;
}

static void _show(const char *text)
{
    if (text != tv.message) {
        strncpy(tv.message, text, sizeof(tv.message) - 1);
        tv.message[sizeof(tv.message) - 1] = 0;
    }
    if (tv.status) {
        nyx_label_set_text(tv.status, tv.message);
        nyx_nova_fit_text(tv.status, lv_obj_get_height(tv.panel) - 48);
    }
}

static int _write_report(FIL *fp)
{
    static const char clipped[] = "\n[Log full: subsequent messages omitted]\n";
    u32 written;
    int r = f_write(fp, tv.log, tv.log_size, &written);
    if (!r && written != tv.log_size) r = 1;
    if (!r && tv.log_full) {
        r = f_write(fp, clipped, sizeof(clipped) - 1, &written);
        if (!r && written != sizeof(clipped) - 1) r = 1;
    }
    int closed = f_close(fp);
    return r ? r : closed;
}

static bool _save(void)
{
    /* Never mount, write or unmount SD from a storage-loop service call. */
    if(tv.cooperative){
        if(tv.log_size){
            tv.report_pending=true;
            if(!tv.task)tv.task=lv_task_create(_tick,40,LV_TASK_PRIO_MID,NULL);
        }
        return false;
    }
    if (!tv.log_size || sd_mount() != 0)
        return false;
    f_mkdir("bootloader/tv-test");
    FIL fp;
    int history_error = 1;
    if (!tv.run_path[0]) {
        // CREATE_NEW preserves preceding attempts even after a cold restart.
        for (u32 i = 1; i <= 999; i++) {
            s_printf(tv.run_path, "bootloader/tv-test/run-%03d.txt", i);
            history_error = f_open(&fp, tv.run_path, FA_CREATE_NEW | FA_WRITE);
            if (history_error != FR_EXIST)
                break;
        }
        if (history_error)
            tv.run_path[0] = 0;
    } else {
        history_error = f_open(&fp, tv.run_path, FA_CREATE_ALWAYS | FA_WRITE);
    }
    if (!history_error)
        history_error = _write_report(&fp);
    int r = f_open(&fp, "bootloader/tv-test/last-run.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (!r)
        r = _write_report(&fp);
    sd_unmount();
    return r == 0 && history_error == 0;
}

static void _stop(void)
{
    // Stop DMA before freeing its framebuffer, including partial starts.
    tv.mirrored = false;
    gui_tv_screen_configure();
    if (tv.clock_saved)
        bpmp_clk_rate_set(BPMP_CLK_NORMAL);
    dpout_video_stop();
    dock_stop(&tv.dock);
    dpout_aux_end();
    if (tv.pad_saved) {
        PINMUX_AUX(0x19C) = tv.hpd_mux;
        gpio_config(GPIO_PORT_CC, GPIO_PIN_1, tv.hpd_gpio);
        gpio_output_enable(GPIO_PORT_CC, GPIO_PIN_1, tv.hpd_oe);
        tv.pad_saved = false;
    }
    if (tv.rails_saved) {
        int r = i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO7_CFG, tv.rails[0]);
        r |= i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO8_CFG, tv.rails[1]);
        if (r) _log("Warning: failed to restore DP regulator configuration.");
        tv.rails_saved = false;
    }
    if (tv.clock_saved) {
        bpmp_clk_rate_set(tv.clock);
        tv.clock_saved = false;
    }
    tv.running = false;
    if (tv.allocation)
        free(tv.allocation);
    tv.allocation = NULL;
    tv.fb = NULL;
    if (!tv.win && tv.task && !_auto_enabled()) {
        lv_task_del(tv.task);
        tv.task = NULL;
    }
}

void gui_tv_shutdown(bool save_report)
{
    bool was_active = tv.running || tv.mirrored || tv.clock_saved;
    tv.report_pending=false;tv.retry_pending=false;tv.retries_left=0;
    tv.resume_arm |= was_active && n_cfg.tv_auto == 1;
    tv.auto_suspended = true;
    if (was_active)
        _log("Stopping external display before subsystem handoff.");
    _stop();
    if (save_report)
        _save();
    if (tv.task) {
        lv_task_del(tv.task);
        tv.task = NULL;
    }
    dpout_set_logger(NULL);
    if (was_active)
        _show("TV output stopped.");
}

// Strong override of the BDK teardown hook; no SD access after sd_end().
void hw_display_external_end(void)
{
    gui_bluetooth_shutdown();
    tv.hardware_exit = true;
    gui_tv_shutdown(false);
    tv.hardware_exit = false;
}

void gui_tv_dirty(int y1, int y2)
{
    if (!tv.mirrored)
        return;
    if (y1 < 0) y1 = 0;
    if (y2 >= 720) y2 = 719;
    if (y1 < tv.dirty_first) tv.dirty_first = y1;
    if (y2 > tv.dirty_last) tv.dirty_last = y2;
}

/* Called only after the completed LVGL frame has been presented. No UI or SD access. */
void gui_tv_present(void)
{
    if(!tv.mirrored||!tv.fb)return;
        if (tv.dirty_last >= tv.dirty_first) {
            u32 first = tv.dirty_first * 1280;
            u32 count = (tv.dirty_last - tv.dirty_first + 1) * 1280;
            memcpy(tv.fb + first, (u32 *)NYX_FB2_ADDRESS + first, count * 4);
            bpmp_mmu_maintenance(BPMP_MMU_MAINT_CLEAN_PHY, false);
            tv.dirty_first = 720;
            tv.dirty_last = -1;
        }
}

/* Safe from synchronous copy loops and physical-button waits. No LVGL scheduler. */
void gui_tv_service(void)
{
    if(tv.servicing||(!tv.task&&!tv.running&&!tv.mirrored))return;
    tv.servicing=true;
    if((u32)(get_tmr_ms()-tv.service_at)>=40){
        tv.cooperative=true;_tick(NULL);tv.cooperative=false;
    }
    tv.servicing=false;
}
void btn_wait_service(void){gui_tv_service();}

/* Bounded restarts of the existing handshake; never force-reset the PD chip. */
static void _retry(void)
{
    if(!tv.retries_left||tv.auto_suspended||(tv.dock.s1&PD_S1_FAULT_MASK)||(tv.snap.status1&PD_S1_FAULT_MASK))return;
    tv.retry_delay=tv.retries_left==2?1500:3000;tv.retries_left--;
    tv.retry_at=get_tmr_ms();tv.retry_pending=true;
    if(!tv.task)tv.task=lv_task_create(_tick,40,LV_TASK_PRIO_MID,NULL);
    _log("Connection failed; a bounded retry is scheduled after cleanup.");
}

static void _tick(void *param)
{
    tv.service_at=get_tmr_ms();
    if(tv.report_pending&&!tv.cooperative){
        tv.report_pending=false;
        _save();
        if(!tv.running&&!tv.mirrored&&!tv.win&&!tv.retry_pending&&!_auto_enabled()&&tv.task){lv_task_del(tv.task);tv.task=NULL;}
    }
    if(tv.retry_pending&&!tv.running&&!tv.mirrored){
        if(btn_read_vol()&BTN_VOL_DOWN){_cancel(NULL);return;}
        if(tv.report_pending||((u32)(get_tmr_ms()-tv.retry_at)<tv.retry_delay))return;
        bool inserted=false,dock=false;
        bpmp_freq_t clock=bpmp_fid_current;bpmp_clk_rate_set(BPMP_CLK_NORMAL);
        int r=pd_dock_attached(&inserted,&dock);bpmp_clk_rate_set(clock);
        if(r){tv.retry_pending=false;tv.retries_left=0;return;} /* No reset or busy retry loop on an unreadable controller. */
        tv.retry_pending=false;
        if(!inserted||!dock){tv.retries_left=0;return;}
        tv.retry_start=true;_begin(false);tv.retry_start=false;return;
    }
    if ((tv.running || tv.mirrored) && (btn_read_vol() & BTN_VOL_DOWN)) {
        _cancel(NULL);
        return;
    }
    if (tv.mirrored) {
        // This task runs outside LVGL's refresh, after all frame chunks.
        if (!dpout_hpd()) {
            _log("HPD lost: stopping video and dock session.");
            _stop();
            _save();
            _show("TV disconnected. Output stopped. Start it again after docking.");
            return;
        }
        gui_tv_present();
        return;
    }
    if (!tv.running) {
        if (_auto_enabled() && (btn_read_vol() & BTN_VOL_DOWN))
            tv.auto_attempted = true;
        _auto_poll();
        return;
    }
    // Includes the upstream's unbounded cable/hub waits; unsigned for rollover.
    if ((u32)(get_tmr_ms() - tv.started) >= TV_RUN_LIMIT_MS) {
        _log("Session timeout after 45 seconds.");
        _stop();
        _save();
        _show("Dock negotiation timed out. Check power and HDMI, then start again.");
        _retry();
        return;
    }
    dock_step(&tv.dock);
    if (tv.dock.log_new) {
        _log(tv.dock.log);
        tv.dock.log_new = false;
    }
    s_printf(tv.message, nyx_tr("Stage: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s"),
             dock_state_name(tv.dock.state), tv.dock.s1, tv.dock.s2, tv.dock.dp, tv.dock.log);
    _show(tv.message);
    if (tv.dock.state == DOCK_FAILED) {
        s_printf(tv.message, nyx_tr("Failed at %s\n%s"), tv.dock.failed_at, tv.dock.fail);
        _log(tv.message);
        _stop();
        _save();
        _show(tv.message);
        _retry();
        return;
    }
    if (tv.dock.state != DOCK_DONE)
        return;
    if (!tv.dock.dp_on || !tv.dock.hpd) {
        _log("Handshake ended without DisplayPort + HPD; no video attempted.");
        _stop();
        _save();
        _show("No TV signal after negotiation. Check the dock and HDMI connection.");
        _retry();
        return;
    }
    int r = dpout_aux_init();
    if (!r)
        r = dpout_probe_sink(&tv.sink);
    if (r) {
        s_printf(tv.message, nyx_tr("AUX/DPCD failed (%d): %s"), r, tv.sink.why);
    } else {
        memcpy(tv.fb, (void *)NYX_FB2_ADDRESS, NYX_FB_SZ);
        bpmp_mmu_maintenance(BPMP_MMU_MAINT_CLEAN_PHY, false);
        r = dpout_video_start(&tv.video, &tv.sink, (u32)tv.fb);
        s_printf(tv.message, nyx_tr(
                 "1280x720 / 60 Hz: %s\nTV: %s %s\nLink: %d lanes, %s\n%s\n\n"
                 "Check the image on your TV. Close this window to use Nova.\nStop TV or VOL- ends TV output."),
                 nyx_tr(r ? "Failed" : "Configured - check TV"), tv.sink.tv_maker, tv.sink.tv_name,
                 tv.video.lanes, dpout_rate_name(tv.video.rate), tv.video.why);
    }
    _log(tv.message);
    tv.running = false;
    if (r) {
        _stop();
    } else {
        tv.retry_pending=false;tv.retries_left=0;
        tv.mirrored = true;
        tv.dirty_first = 0;
        tv.dirty_last = 719;
        // Only PD negotiation needs the base clock. Restore UI performance.
        bpmp_clk_rate_set(tv.clock);
        gui_tv_screen_configure();
    }
    bool saved = _save();
    _show(tv.message);
    if(r)_retry();
    _log(saved ? "Report saved to SD." : tv.report_pending ? "Report deferred until storage operation completes." : "Report not saved (SD unavailable or write failed).");
}

static lv_res_t _begin(bool automatic)
{
    if (!tv.task)
        tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
    if (!tv.task) {
        _show("TV service could not start. Close this window and try again.");
        return LV_RES_OK;
    }
    if (tv.running || tv.mirrored)
        return LV_RES_OK;
    if (fuse_read_hw_type() == FUSE_NX_HW_TYPE_HOAG) {
        _show("Switch Lite does not support TV output.");
        return LV_RES_OK;
    }
    if(!tv.retry_start){tv.retry_pending=false;tv.retries_left=2;}
    tv.auto_attempted = true;
    tv.log_size = 0;
    tv.log_full = false;
    tv.run_path[0] = 0;
    tv.started = get_tmr_ms();
    memset(&tv.sink, 0, sizeof(tv.sink));
    memset(&tv.video, 0, sizeof(tv.video));
    _log(automatic ? "Hekate Nova Pro " NYX_NOVA_VERSION " - automatic dock output"
                   : "Hekate Nova Pro " NYX_NOVA_VERSION " - manual dock output");
    _log("HWTest-RCM upstream 3731bfb471515a5f83709aa496c9cdb0a9aeff83");
    char attempt[64];s_printf(attempt,"Connection attempt %d of 3.",3-tv.retries_left);_log(attempt);
    tv.clock = bpmp_fid_current;
    tv.clock_saved = true;
    bpmp_clk_rate_set(BPMP_CLK_NORMAL);
    // Save the HPD pad before pd_snapshot() switches it to a GPIO input.
    tv.hpd_mux = PINMUX_AUX(0x19C);
    tv.hpd_gpio = (GPIO(0x700) & GPIO_PIN_1) != 0;
    tv.hpd_oe = (GPIO(0x710) & GPIO_PIN_1) != 0;
    tv.pad_saved = true;
    pd_snapshot(&tv.snap, false);
    s_printf(tv.message, "BM92T36 ok=%d man=%04X dev=%04X status1=%04X status2=%04X dp=%04X",
             tv.snap.ok, tv.snap.man_id, tv.snap.dev_id, tv.snap.status1, tv.snap.status2, tv.snap.dp_status);
    _log(tv.message);
    if (!tv.snap.ok) {
        _stop();
        _save();
        _show("BM92T36 did not respond. No automatic reset was performed.");
        return LV_RES_OK;
    }
    if (i2c_recv_buf_small(&tv.rails[0], 1, I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO7_CFG) ||
        i2c_recv_buf_small(&tv.rails[1], 1, I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO8_CFG)) {
        _log("Cannot preserve DP regulator state; test cancelled.");
        _stop();
        _save();
        _show("Video power state could not be read. TV start cancelled.");
        return LV_RES_OK;
    }
    tv.rails_saved = true;
    tv.allocation = malloc(NYX_FB_SZ + 4095);
    if (!tv.allocation) {
        _log("No memory for TV framebuffer.");
        _stop();
        _save();
        _show("Not enough memory for TV output.");
        return LV_RES_OK;
    }
    tv.fb = (u32 *)ALIGN((uintptr_t)tv.allocation, 4096);
    dpout_set_logger(_log);
    dpout_lane_test(false, false);
    dock_start(&tv.dock);
    tv.running = true;
    _show("Connecting to the dock... Stop TV or VOL- cancels.");
    return LV_RES_OK;
}

static lv_res_t _start(lv_obj_t *btn)
{
    (void)btn;
    return _begin(false);
}

static lv_res_t _cancel(lv_obj_t *btn)
{
    tv.retry_pending=false;tv.retries_left=0;
    tv.auto_attempted = true;
    tv.auto_samples = 0;
    if (!tv.log_size) {
        _show("TV output is off. Start TV connects to the dock.");
        return LV_RES_OK;
    }
    _log("User stopped the session.");
    _stop();
    bool saved = _save();
    _show(saved ? "TV stopped. Report: bootloader/tv-test/last-run.txt" : tv.report_pending ? "TV output stopped." :
                 "TV stopped. Report could not be saved to SD.");
    return LV_RES_OK;
}

static lv_res_t _signal(lv_obj_t *obj, lv_signal_t sign, void *param)
{
    lv_signal_func_t previous = tv.signal;
    if (sign == LV_SIGNAL_CLEANUP) {
        // The session outlives its settings window; never retain dead widgets.
        tv.win = NULL;
        tv.status = NULL;
        tv.panel = NULL;
        if (!tv.running && !tv.mirrored && !tv.report_pending && !tv.retry_pending && !_auto_enabled()) {
            if (tv.task) lv_task_del(tv.task);
            tv.task = NULL;
            dpout_set_logger(NULL);
        }
    }
    return previous(obj, sign, param);
}

lv_res_t gui_tv_open(lv_obj_t *btn)
{
    if (tv.win)
        return LV_RES_OK;
    tv.win = nyx_create_standard_window(nyx_tr("USB-C TV / Dock"), NULL);
    tv.signal = lv_obj_get_signal_func(tv.win);
    lv_obj_set_signal_func(tv.win, _signal);
    lv_win_add_btn(tv.win, NULL, nyx_tr("Start TV"), _start);
    lv_win_add_btn(tv.win, NULL, nyx_tr("Stop TV"), _cancel);
    lv_obj_t *page = lv_win_get_content(tv.win);
    nyx_nova_fixed_page(page);
    lv_obj_t *scrl = lv_page_get_scrl(page);
    tv.panel = nyx_nova_panel(scrl, 24, 16, lv_obj_get_width(page) - 48, lv_obj_get_height(page) - 32);
    tv.status = nyx_nova_text(tv.panel, "", 24, 24, lv_obj_get_width(tv.panel) - 48, NOVA_BODY);
    if (!tv.message[0])
        _show("Use the console directly in the official dock.\nConnect dock power and HDMI, then select Start TV.\n\n"
              "You can start outside the dock, then dock within 45 seconds.\nClose this window to keep using Nova on your TV.\n"
              "VOL- stops TV output and restores the console screen.");
    else
        _show(tv.message);
    if (!tv.task)
        tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
    if (!tv.task)
        _show("TV service could not start. Close this window and try again.");
    return LV_RES_OK;
}
