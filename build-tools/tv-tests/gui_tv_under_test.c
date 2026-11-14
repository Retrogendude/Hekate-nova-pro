/* Hekate TV experiment. Manual start only; no changes on normal boot.
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
    lv_obj_t *win, *status;
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
    u32 hpd_mux;
    u8 rails[2];
    int dirty_first, dirty_last;
    char log[TV_LOG_SIZE], message[768];
} tv;

static lv_res_t _cancel(lv_obj_t *btn);

static void _log(const char *line)
{
    u32 n = strlen(line);
    if (tv.log_size + n + 2 >= TV_LOG_SIZE) {
        tv.log_full = true;
        return;
    }
    memcpy(tv.log + tv.log_size, line, n);
    tv.log_size += n;
    tv.log[tv.log_size++] = '\n';
    tv.log[tv.log_size] = 0;
}

static void _show(const char *text)
{
    // Bounded messages are formed by the caller; no recolor in register logs.
    lv_label_set_text(tv.status, text);
}

static bool _save(void)
{
    if (!tv.log_size || !sd_mount())
        return false;
    f_mkdir("bootloader/tv-test");
    static const char clipped[] = "\n[Log full: subsequent messages omitted]\n";
    FIL fp;
    int r = f_open(&fp, "bootloader/tv-test/last-run.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (!r) {
        u32 written;
        r = f_write(&fp, tv.log, tv.log_size, &written);
        if (!r && written != tv.log_size) r = 1;
        if (!r && tv.log_full) {
            r = f_write(&fp, clipped, sizeof(clipped) - 1, &written);
            if (!r && written != sizeof(clipped) - 1) r = 1;
        }
        int closed = f_close(&fp);
        if (!r) r = closed;
    }
    sd_unmount();
    return r == 0;
}

static void _stop(void)
{
    // Stop DMA before freeing its framebuffer, including partial starts.
    tv.mirrored = false;
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

static void _tick(void *param)
{
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
            _show("Connexion TV perdue. Sortie arretee. Vous pouvez relancer le test.");
            return;
        }
        if (tv.dirty_last >= tv.dirty_first) {
            u32 first = tv.dirty_first * 1280;
            u32 count = (tv.dirty_last - tv.dirty_first + 1) * 1280;
            memcpy(tv.fb + first, (u32 *)NYX_FB2_ADDRESS + first, count * 4);
            bpmp_mmu_maintenance(BPMP_MMU_MAINT_CLEAN_PHY, false);
            tv.dirty_first = 720;
            tv.dirty_last = -1;
        }
        return;
    }
    if (!tv.running)
        return;
    // Includes the upstream's unbounded cable/hub waits; unsigned for rollover.
    if ((u32)(get_tmr_ms() - tv.started) >= TV_RUN_LIMIT_MS) {
        _log("Session timeout after 45 seconds.");
        _stop();
        _save();
        _show("Delai de 45 secondes depasse. Test arrete. Verifiez dock, alimentation et HDMI.");
        return;
    }
    dock_step(&tv.dock);
    if (tv.dock.log_new) {
        _log(tv.dock.log);
        tv.dock.log_new = false;
    }
    s_printf(tv.message, "Etape : %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s",
             dock_state_name(tv.dock.state), tv.dock.s1, tv.dock.s2, tv.dock.dp, tv.dock.log);
    _show(tv.message);
    if (tv.dock.state == DOCK_FAILED) {
        s_printf(tv.message, "Echec a l'etape %s\n%s", tv.dock.failed_at, tv.dock.fail);
        _log(tv.message);
        _stop();
        _save();
        _show(tv.message);
        return;
    }
    if (tv.dock.state != DOCK_DONE)
        return;
    if (!tv.dock.dp_on || !tv.dock.hpd) {
        _log("Handshake ended without DisplayPort + HPD; no video attempted.");
        _stop();
        _save();
        _show("Negociation terminee sans signal TV. Verifiez le dock et le branchement HDMI.");
        return;
    }
    int r = dpout_aux_init();
    if (!r)
        r = dpout_probe_sink(&tv.sink);
    if (r) {
        s_printf(tv.message, "Echec AUX/DPCD (%d) : %s", r, tv.sink.why);
    } else {
        memcpy(tv.fb, (void *)NYX_FB2_ADDRESS, NYX_FB_SZ);
        bpmp_mmu_maintenance(BPMP_MMU_MAINT_CLEAN_PHY, false);
        r = dpout_video_start(&tv.video, &tv.sink, (u32)tv.fb);
        s_printf(tv.message,
                 "Sortie 1280x720 / 60 Hz : %s\nTV : %s %s\nLien : %d voies, %s\n%s\n\n"
                 "Seul un controle visuel sur la TV confirme l'image.\nArreter, Fermer ou VOL- coupe la sortie TV.",
                 r ? "ECHEC" : "configuree (a verifier)", tv.sink.tv_maker, tv.sink.tv_name,
                 tv.video.lanes, dpout_rate_name(tv.video.rate), tv.video.why);
    }
    _log(tv.message);
    tv.running = false;
    if (r)
        _stop();
    else {
        tv.mirrored = true;
        tv.dirty_first = 0;
        tv.dirty_last = 719;
    }
    bool saved = _save();
    _show(tv.message);
    _log(saved ? "Report saved to SD." : "Report not saved (SD unavailable or write failed).");
}

static lv_res_t _start(lv_obj_t *btn)
{
    if (!tv.task)
        return LV_RES_OK;
    if (tv.running || tv.mirrored)
        return LV_RES_OK;
    if (fuse_read_hw_type() == FUSE_NX_HW_TYPE_HOAG) {
        _show("La Switch Lite ne possede pas de sortie TV.");
        return LV_RES_OK;
    }
    tv.log_size = 0;
    tv.log_full = false;
    memset(&tv.sink, 0, sizeof(tv.sink));
    memset(&tv.video, 0, sizeof(tv.video));
    _log("Hekate 6.5.5 / Nyx 1.9.5 - TV experiment 01");
    _log("HWTest-RCM upstream 3731bfb471515a5f83709aa496c9cdb0a9aeff83");
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
        _show("Le controleur BM92T36 ne repond pas. Aucun reset automatique effectue.");
        return LV_RES_OK;
    }
    if (i2c_recv_buf_small(&tv.rails[0], 1, I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO7_CFG) ||
        i2c_recv_buf_small(&tv.rails[1], 1, I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_LDO8_CFG)) {
        _log("Cannot preserve DP regulator state; test cancelled.");
        _stop();
        _save();
        _show("Impossible de lire les alimentations video. Test annule.");
        return LV_RES_OK;
    }
    tv.rails_saved = true;
    tv.allocation = malloc(NYX_FB_SZ + 4095);
    if (!tv.allocation) {
        _log("No memory for TV framebuffer.");
        _stop();
        _save();
        _show("Memoire insuffisante pour le tampon TV.");
        return LV_RES_OK;
    }
    tv.fb = (u32 *)ALIGN((uintptr_t)tv.allocation, 4096);
    dpout_set_logger(_log);
    dpout_lane_test(false, false);
    dock_start(&tv.dock);
    tv.started = get_tmr_ms();
    tv.running = true;
    _show("Negociation du dock en cours... Arreter ou VOL- pour annuler.");
    return LV_RES_OK;
}

static lv_res_t _cancel(lv_obj_t *btn)
{
    if (!tv.log_size) {
        _show("Aucun test en cours. Demarrer lance le test du dock.");
        return LV_RES_OK;
    }
    _log("User stopped the session.");
    _stop();
    bool saved = _save();
    _show(saved ? "Test arrete. Journal : bootloader/tv-test/last-run.txt" :
                 "Test arrete. Impossible de sauvegarder le journal sur la SD.");
    return LV_RES_OK;
}

static lv_res_t _signal(lv_obj_t *obj, lv_signal_t sign, void *param)
{
    lv_signal_func_t previous = tv.signal;
    if (sign == LV_SIGNAL_CLEANUP) {
        if (tv.task) lv_task_del(tv.task);
        tv.task = NULL;
        if (tv.log_size) _log("Test window closed.");
        _stop();
        _save();
        dpout_set_logger(NULL);
        tv.win = NULL;
    }
    return previous(obj, sign, param);
}

lv_res_t gui_tv_open(lv_obj_t *btn)
{
    if (tv.win)
        return LV_RES_OK;
    memset(&tv, 0, sizeof(tv));
    tv.win = nyx_create_standard_window("TV USB-C / Dock - EXPERIMENTAL 01", NULL);
    tv.signal = lv_obj_get_signal_func(tv.win);
    lv_obj_set_signal_func(tv.win, _signal);
    lv_win_add_btn(tv.win, NULL, "Demarrer", _start);
    lv_win_add_btn(tv.win, NULL, "Arreter", _cancel);
    lv_obj_t *page = lv_win_get_content(tv.win);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF);
    lv_obj_t *scrl = lv_page_get_scrl(page);
    tv.status = lv_label_create(scrl, NULL);
    lv_label_set_long_mode(tv.status, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(tv.status, LV_HOR_RES - 80);
    lv_obj_set_pos(tv.status, 30, 24);
    _show("Test manuel de la sortie TV, separe de Nova Pro v18.\n\n"
          "Branchez le dock officiel a son alimentation et a la TV en HDMI.\n"
          "Placez la console dans le dock, puis utilisez Demarrer.\n\n"
          "L'ecran de la console reste allume. Aucun reset du controleur automatique.\n"
          "Le test negocie l'alimentation et DisplayPort avec le dock.\n"
          "Fermer, Arreter ou VOL- termine le test. Switch Lite non prise en charge.\n\n"
          "Journal : bootloader/tv-test/last-run.txt");
    tv.task = lv_task_create(_tick, 40, LV_TASK_PRIO_MID, NULL);
    if (!tv.task)
        _show("Impossible de creer la tache de test. Fermez cette fenetre.");
    return LV_RES_OK;
}
