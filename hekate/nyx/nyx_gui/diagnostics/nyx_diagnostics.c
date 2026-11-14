/* Native host for HWTest-RCM 0.15. The original probe implementations are
 * retained under vendor/. Only this adapter owns UI/report lifetime. */
#include "nyx_diagnostics.h"
#include "vendor/hwtest.h"
#include <stdlib.h>

extern const struct page_entry g_pages[N_PAGES];
extern const u32 g_page_groups[N_PAGES];
extern const char *const nyx_diag_function_names[N_PAGES];
extern u32 dx_count(void);
extern const dx_finding_t *dx_at(u32 index);

char _log_buf[4096];
bool _log_uart_only = true, _log_no_uart = true;
char *_log_capture, *g_report_body;
u32 _log_capture_pos, _log_capture_cap;
bool g_show_status;
u32 g_run_mask;

static bool running, truncated, muted, snapshot_valid, slots_available;
static char *report;
static u32 report_used, active_index, current_step, total_steps, active_groups;
static nyx_diag_probe_t metadata[NYX_DIAG_PROBE_COUNT];
static nyx_diag_hooks_t callbacks;
static bool has_finding;
static u32 worst_finding;
#define CAPACITY (256u * 1024u)

/* Only readable pad configuration and GPIO output/configuration words are
 * restored. No interrupt-status, W1C, fuse or memory-controller word is
 * bulk-written. Inputs are repowered by the Nyx resume hook after cleanup. */
static const u32 pad_offsets[] = {
    0x1B4, 0x1B8, 0x1BC, 0x1C0, 0x1C4, 0x1C8, 0x1CC,
    PINMUX_AUX_AUD_MCLK, PINMUX_AUX_GPIO_PZ4,
    PINMUX_AUX_CAM1_PWDN,
    PINMUX_AUX_SDMMC2_CLK, PINMUX_AUX_SDMMC2_CMD,
    PINMUX_AUX_SDMMC2_DAT0, PINMUX_AUX_SDMMC2_DAT1,
    PINMUX_AUX_SDMMC2_DAT2, PINMUX_AUX_SDMMC2_DAT3,
    PINMUX_AUX_SDMMC2_DAT4, PINMUX_AUX_SDMMC2_DAT5,
    PINMUX_AUX_SDMMC2_DAT6, PINMUX_AUX_SDMMC2_DAT7,
    0x44, 0x48, 0x254, 0x258, 0x25C,
    0x124, 0x128, 0x12C, 0x130,
    0x134, 0x138, 0x13C, 0x140,
    PINMUX_AUX_UARTX_TX(UART_D), PINMUX_AUX_UARTX_RX(UART_D),
    PINMUX_AUX_UARTX_RTS(UART_D), PINMUX_AUX_UARTX_CTS(UART_D)
};
static u32 saved_pads[ARRAY_SIZE(pad_offsets)];
static const u32 gpio_ports[] = { GPIO_PORT_H, GPIO_PORT_I, GPIO_PORT_K,
    GPIO_PORT_S, GPIO_PORT_V, GPIO_PORT_Z, GPIO_PORT_BB };
static u32 saved_gpio[ARRAY_SIZE(gpio_ports)][3];
static u32 saved_pwm, saved_clkout, saved_audpad;
static u8 saved_bis_keys[2][32];
static const u32 audio_clock_offsets[] = {
    CLK_RST_CONTROLLER_PLLA_BASE, CLK_RST_CONTROLLER_PLLA_MISC,
    CLK_RST_CONTROLLER_PLLA_OUT, CLK_RST_CONTROLLER_CLK_SOURCE_I2S1,
    CLK_RST_CONTROLLER_CLK_SOURCE_I2S2, CLK_RST_CONTROLLER_CLK_SOURCE_EXTPERIPH1
};
static u32 saved_audio_clock[ARRAY_SIZE(audio_clock_offsets)];
static const u32 clock_enable_offsets[] = { 0x10, 0x18, 0x360, 0x298 };
static const u32 clock_reset_offsets[] = { 0x4, 0xC, 0x358, 0x2A4 };
static u32 saved_enable[4], saved_reset[4], saved_domains;
static u32 saved_pwrdet, saved_pwrdetval, saved_iopower, saved_das[2];
static const u8 pmic_registers[] = { 0x25, 0x31 };
static u8 saved_pmic[ARRAY_SIZE(pmic_registers)];
static bool saved_pmic_valid[ARRAY_SIZE(pmic_registers)];
static bool restore_failed;
static u32 saved_uartd_source;
extern void nyx_diag_audio_stop(void);

static u32 gpio_offset(u32 port) { return ((port >> 2) << 8) + (port % 4) * 4; }

static void append(const char *text, u32 length)
{
    static const char marker[] = "\n[REPORT TRUNCATED]\n";
    if (!report || muted || truncated) return;
    if (length > CAPACITY - report_used - sizeof(marker)) {
        memcpy(report + report_used, marker, sizeof(marker));
        report_used += sizeof(marker) - 1;
        truncated = true;
        return;
    }
    memcpy(report + report_used, text, length);
    report_used += length;
    report[report_used] = 0;
}

void _log_emit(u32 color)
{
    if (!muted && running && active_index < NYX_DIAG_PROBE_COUNT) {
        /* Missing prerequisites are observations, never implicit PASS. */
        if (color == COL_ERR && metadata[active_index].state < NYX_DIAG_FAIL)
            metadata[active_index].state = NYX_DIAG_FAIL;
        else if (color == COL_WARN && metadata[active_index].state < NYX_DIAG_WARN)
            metadata[active_index].state = NYX_DIAG_WARN;
    }
    append(_log_buf, strlen(_log_buf));
}

void _log_color_impl(u32 color, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    s_vprintf(_log_buf, fmt, ap);
    va_end(ap);
    _log_emit(color);
}

void nyx_diag_record_severity(u32 severity)
{
    if (!running || muted) return;
    has_finding = true;
    if (severity > worst_finding) worst_finding = severity;
}

void log_mute(bool on) { muted = on; }
void log_capture_arm(char *buf, u32 cap)
{
    _log_capture = buf; _log_capture_cap = cap; _log_capture_pos = 0;
    if (buf && cap) buf[0] = 0;
}
/* UART-B belongs to the right Joy-Con in Nyx. Never transmit diagnostic
 * text or consume bytes from that protocol, including during long waits. */
void uart_send_crlf(u32 idx, const u8 *buf, u32 n) { (void)idx; (void)buf; (void)n; }
int uart_getc(void) { return -1; }
void uart_ungetc(u8 c) { (void)c; }
void uart_poll_reboot(void) {}
void msleep_poll(u32 ms) { msleep(ms); }
void status_set(const char *message)
{
    if (callbacks.progress)
        callbacks.progress(current_step, total_steps, message, callbacks.context);
}
void status_step(const char *message)
{
    log_color(COL_INFO, "[step] %s\n", message);
    status_set(message);
}

static void snapshot(void)
{
    for (u32 i = 0; i < ARRAY_SIZE(pad_offsets); i++) saved_pads[i] = PINMUX_AUX(pad_offsets[i]);
    for (u32 i = 0; i < ARRAY_SIZE(gpio_ports); i++) {
        u32 off = gpio_offset(gpio_ports[i]);
        saved_gpio[i][0] = GPIO(off);
        saved_gpio[i][1] = GPIO(off + 0x10);
        saved_gpio[i][2] = GPIO(off + 0x20);
    }
    saved_pwm = PWM(PWM_CONTROLLER_PWM_CSR_1);
    saved_clkout = PMC(APBDEV_PMC_CLK_OUT_CNTRL);
    saved_audpad = APB_MISC(APB_MISC_GP_AUD_MCLK_CFGPADCTRL);
    saved_uartd_source = CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UARTD);
    saved_domains = PMC(APBDEV_PMC_PWRGATE_STATUS);
    saved_pwrdet = PMC(APBDEV_PMC_PWR_DET);
    saved_pwrdetval = PMC(APBDEV_PMC_PWR_DET_VAL);
    saved_iopower = PMC(APBDEV_PMC_NO_IOPOWER);
    saved_das[0] = APB_MISC(0xC00);
    saved_das[1] = APB_MISC(0xC40);
    for (u32 i = 0; i < ARRAY_SIZE(audio_clock_offsets); i++) saved_audio_clock[i] = CLOCK(audio_clock_offsets[i]);
    for (u32 i = 0; i < 4; i++) {
        saved_enable[i] = CLOCK(clock_enable_offsets[i]);
        saved_reset[i] = CLOCK(clock_reset_offsets[i]);
    }
    for (u32 i = 0; i < ARRAY_SIZE(pmic_registers); i++)
        saved_pmic_valid[i] = !(active_groups & NYX_DIAG_G_WIRELESS) ||
            i2c_recv_buf_small(&saved_pmic[i], 1, I2C_5, MAX77620_I2C_ADDR, pmic_registers[i]) == 0;
    slots_available = ((se_key_acc_ctrl_get(0) & 3) == 3) && ((se_key_acc_ctrl_get(1) & 3) == 3);
    if (slots_available && (active_groups & NYX_DIAG_G_STORAGE)) {
        se_aes_key_get(0, saved_bis_keys[0], 32);
        se_aes_key_get(1, saved_bis_keys[1], 32);
    }
    snapshot_valid = true;
}

bool nyx_diag_keyslots_available(void) { return slots_available; }

void nyx_diag_cleanup(void)
{
    if (!snapshot_valid) return;
    /* The Wi-Fi probe supervises and powergates CPU0 on every completed
     * path. Its reserved image/mailbox at A0000000..A003FFFF is outside
     * the Nyx heap and framebuffers. Codec output is muted by the probe. */
    if (active_groups & (NYX_DIAG_G_AUDIO | NYX_DIAG_G_SOUND)) {
        nyx_diag_audio_stop();
        for (u32 i = 0; i < ARRAY_SIZE(audio_clock_offsets); i++) CLOCK(audio_clock_offsets[i]) = saved_audio_clock[i];
        const u32 mask[] = {
            BIT(CLK_L_I2S1) | BIT(CLK_L_I2S2) | BIT(CLK_L_I2S3) | BIT(CLK_L_SPDIF),
            0,
            BIT(CLK_V_AHUB) | BIT(CLK_V_APB2APE) | BIT(CLK_V_I2S4) | BIT(CLK_V_I2S5) | BIT(CLK_V_EXTPERIPH1),
            BIT(CLK_Y_APE)
        };
        for (u32 i = 0; i < 4; i++) {
            CLOCK(clock_reset_offsets[i]) = (CLOCK(clock_reset_offsets[i]) & ~mask[i]) | (saved_reset[i] & mask[i]);
            CLOCK(clock_enable_offsets[i]) = (CLOCK(clock_enable_offsets[i]) & ~mask[i]) | (saved_enable[i] & mask[i]);
        }
        const u32 audio_hv = BIT(18);
        PMC(APBDEV_PMC_PWR_DET) = (PMC(APBDEV_PMC_PWR_DET) & ~audio_hv) | (saved_pwrdet & audio_hv);
        PMC(APBDEV_PMC_PWR_DET_VAL) = (PMC(APBDEV_PMC_PWR_DET_VAL) & ~audio_hv) | (saved_pwrdetval & audio_hv);
        PMC(APBDEV_PMC_NO_IOPOWER) = (PMC(APBDEV_PMC_NO_IOPOWER) & ~audio_hv) | (saved_iopower & audio_hv);
        APB_MISC(0xC00) = saved_das[0]; APB_MISC(0xC40) = saved_das[1];
        if (!(saved_domains & BIT(POWER_RAIL_AUD)) && pmc_domain_pwrgate_set(POWER_RAIL_AUD, 0)) restore_failed = true;
    }
    if (active_groups & NYX_DIAG_G_WIRELESS) {
        if (!(saved_domains & BIT(POWER_RAIL_PCIE)) && pmc_domain_pwrgate_set(POWER_RAIL_PCIE, 0)) restore_failed = true;
        u32 masks[4] = { 0, BIT(CLK_U_PCIE) | BIT(CLK_U_AFI) | BIT(CLK_U_PCIEXCLK) | BIT(CLK_U_UARTD), BIT(CLK_V_MSELECT), 0 };
        for (u32 i = 0; i < 4; i++) {
            CLOCK(clock_reset_offsets[i]) = (CLOCK(clock_reset_offsets[i]) & ~masks[i]) | (saved_reset[i] & masks[i]);
            CLOCK(clock_enable_offsets[i]) = (CLOCK(clock_enable_offsets[i]) & ~masks[i]) | (saved_enable[i] & masks[i]);
        }
        for (u32 i = 0; i < ARRAY_SIZE(pmic_registers); i++) {
            if (!saved_pmic_valid[i]) { restore_failed = true; continue; }
            if (i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, pmic_registers[i], saved_pmic[i])) restore_failed = true;
        }
        CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UARTD) = saved_uartd_source;
    }
    for (u32 i = 0; i < ARRAY_SIZE(gpio_ports); i++) {
        u32 off = gpio_offset(gpio_ports[i]);
        /* Disable outputs before changing their latch and pad routing. */
        GPIO(off + 0x10) = 0;
        GPIO(off + 0x20) = saved_gpio[i][2];
        GPIO(off) = saved_gpio[i][0];
        GPIO(off + 0x10) = saved_gpio[i][1];
    }
    for (u32 i = 0; i < ARRAY_SIZE(pad_offsets); i++) PINMUX_AUX(pad_offsets[i]) = saved_pads[i];
    PWM(PWM_CONTROLLER_PWM_CSR_1) = saved_pwm;
    PMC(APBDEV_PMC_CLK_OUT_CNTRL) = saved_clkout;
    APB_MISC(APB_MISC_GP_AUD_MCLK_CFGPADCTRL) = saved_audpad;
    if (slots_available && (active_groups & NYX_DIAG_G_STORAGE)) {
        se_aes_key_set(0, saved_bis_keys[0], 32);
        se_aes_key_set(1, saved_bis_keys[1], 32);
        memset(saved_bis_keys, 0, sizeof(saved_bis_keys));
    }
    if (emmc_storage.initialized)
        sdmmc_storage_set_mmc_partition(&emmc_storage, 0);
    snapshot_valid = false;
}

static void init_metadata(void)
{
    for (u32 i = 0; i < N_PAGES; i++) {
        metadata[i].name = g_pages[i].name;
        metadata[i].function = nyx_diag_function_names[i];
        metadata[i].group = g_page_groups[i];
    }
}

const nyx_diag_probe_t *nyx_diag_probe(u32 index)
{
    if (index >= N_PAGES) return NULL;
    init_metadata();
    return &metadata[index];
}

static void execute(u32 index)
{
    active_index = index;
    metadata[index].state = NYX_DIAG_OBSERVATION;
    metadata[index].report_offset = report_used;
    has_finding = false; worst_finding = 0;
    status_set(g_pages[index].name);
    log_color(COL_HEADER, "\n--- %s [%s] ---\n", g_pages[index].name, metadata[index].function);
    g_pages[index].fn();
    if (has_finding) {
        nyx_diag_state_t state = worst_finding == DX_FAIL ? NYX_DIAG_FAIL :
            worst_finding == DX_WARN ? NYX_DIAG_WARN : NYX_DIAG_PASS;
        if (metadata[index].state < state) metadata[index].state = state;
        /* Observation has a lower enum value than PASS, so real positive
         * findings can be distinguished from data-only pages. */
    }
    metadata[index].report_length = report_used - metadata[index].report_offset;
    current_step++;
}

static int run(u32 groups, int single, const nyx_diag_hooks_t *hooks)
{
    if (running) return 1;
    if (!groups || (groups & ~NYX_DIAG_G_ALL)) return 3;
    if (!report) report = malloc(CAPACITY);
    if (!report) return 2;
    memset(metadata, 0, sizeof(metadata)); init_metadata();
    report_used = 0; report[0] = 0; truncated = muted = false;
    callbacks = hooks ? *hooks : (nyx_diag_hooks_t){0};
    running = true; restore_failed = false; active_groups = g_run_mask = groups;
    current_step = 0; total_steps = 1;
    for (u32 i = 1; i < N_PAGES; i++)
        if ((single < 0 && (g_page_groups[i] & groups)) || (int)i == single) total_steps++;
    if (callbacks.suspend) callbacks.suspend(groups, callbacks.context);
    snapshot(); dx_reset();
    g_sd_ok = sd_storage.initialized;
    g_emmc_ok = emmc_storage.initialized;
    g_wifi_armed = true; g_wifi_brought_up = false;
    probe_reset_snapshot();
    log_color(COL_INFO, "HWTest-RCM 0.15 / Native Nyx diagnostics\n");
    /* Explicit prerequisites for single-page runs and sound-only groups. */
    if ((groups & NYX_DIAG_G_STORAGE) && single > 16) probe_storage();
    if ((groups & NYX_DIAG_G_SOUND) && !(groups & NYX_DIAG_G_AUDIO)) { probe_audio(); probe_audio_clocks(); }
    for (u32 i = 1; i < N_PAGES; i++) {
        if ((single < 0 && (g_page_groups[i] & groups)) || (int)i == single) execute(i);
    }
    execute(0);
    nyx_diag_cleanup();
    if (restore_failed)
        log_color(COL_WARN, "\n[Nyx] Restoration error: reload Nyx before booting.\n");
    if (callbacks.resume) callbacks.resume(groups, callbacks.context);
    running = false; active_index = N_PAGES;
    g_report_body = report;
    memset(&callbacks, 0, sizeof(callbacks));
    return restore_failed ? 4 : 0;
}

int nyx_diag_run(u32 groups, const nyx_diag_hooks_t *hooks) { return run(groups, -1, hooks); }
int nyx_diag_run_probe(u32 index, const nyx_diag_hooks_t *hooks)
{
    if (index == 0 || index >= N_PAGES) return 3;
    return run(g_page_groups[index], (int)index, hooks);
}
bool nyx_diag_running(void) { return running; }
bool nyx_diag_truncated(void) { return truncated; }
const char *nyx_diag_report(void) { return report ? report : "Aucun diagnostic execute.\n"; }
u32 nyx_diag_finding_count(void) { return dx_count(); }
bool nyx_diag_finding(u32 index, nyx_diag_finding_t *out)
{
    const dx_finding_t *f = dx_at(index);
    if (!f || !out) return false;
    out->key = f->key; out->severity = f->sev;
    memcpy(out->detail, f->detail, sizeof(out->detail));
    return true;
}
int nyx_diag_export(char *path)
{
    if (running || !report || !report_used) return 1;
    bool mounted = sd_get_card_mounted();
    if (sd_mount()) return 2;
    f_mkdir("bootloader/diagnostics");
    char destination[128];
    s_printf(destination, "bootloader/diagnostics/hwtest_%08X.txt", get_tmr_ms());
    FIL file;
    FRESULT fr = f_open(&file, destination, FA_WRITE | FA_CREATE_NEW);
    UINT written = 0;
    int result = 3;
    if (fr == FR_OK) {
        fr = f_write(&file, report, report_used, &written);
        FRESULT closed = f_close(&file);
        result = (fr == FR_OK && closed == FR_OK && written == report_used) ? 0 : 4;
    }
    if (!mounted) sd_unmount();
    if (!result && path) strcpy(path, destination);
    return result;
}
int save_report(void) { return nyx_diag_export(NULL); }
