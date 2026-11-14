#include "vendor/hwtest.h"
const struct page_entry g_pages[] = {
    /* Index 0: top-level verdict. Depends on globals populated by every
     * other probe, so run_all_probes runs it LAST. The pager shows it
     * as the first navigable page. */
    {probe_verdict,    "Diagnostics"},

    /* Identity. */
    {probe_soc,        "SoC"},
    {probe_fuses,      "Fuses"},
    {probe_kfuse,      "Fuses"},

    /* Power & charging. Each probe gets its own LCD page; they all
     * share the "Power & charging" name so the host viewer groups them. */
    {probe_pmic,       "Power & charging"},
    {probe_max77812,   "Power & charging"},
    {probe_pmic_gpios, "Power & charging"},
    {probe_regulators, "Power & charging"},
    {probe_5v,         "Power & charging"},
    {probe_battery,    "Power & charging"},
    {probe_charger,    "Power & charging"},
    {probe_usbpd,      "Power & charging"},
    {probe_thermal,    "Power & charging"},
    {probe_fan,        "Power & charging"},

    /* Memory & clocks. */
    {probe_dram,       "Memory & clocks"},
    {probe_clocks,     "Memory & clocks"},

    /* All storage probes share one display name, so partitions, GPT and
     * health land in a single "Storage" pane on the host viewer. */
    {probe_storage,    "Storage"},
    {probe_serial,     "Storage"},
    {probe_partitions, "Storage"},
    {probe_emmc_health,"Storage"},
    {probe_gpt,        "Storage"},
    {probe_boot0_pkg1, "Storage"},
    {probe_autorcm,    "Storage"},
    {probe_sd_content, "Storage"},
    /* Last in the group: the counters must account for every read the
     * probes above performed. */
    {probe_storage_errors, "Storage"},
    {probe_gamecard,   "Storage"},

    /* Wireless: the two halves of the CYW4356. BT first - it is the
     * cheaper measurement and its result frames the PCIe one. */
    {probe_bt_radio,   "Wireless"},
    {probe_wifi_pcie,  "Wireless"},
    {probe_wifi_link,  "Wireless"},

    /* Audio codec. */
    {probe_audio,        "Audio"},
    {probe_audio_clocks, "Audio"},
    {probe_audio_beep,   "Audio"},

    /* Display + backlight. */
    {probe_display,    "Display"},
    {probe_backlight,  "Display"},

    /* Touch + Joy-Con + buttons + ambient light. */
    {probe_touch,      "Inputs"},
    {probe_als,        "Inputs"},
    {probe_joycon,     "Inputs"},
    {probe_inputs,     "Inputs"},

    /* Low-level dumps. UART_B is always the debug/report console. */
    {probe_gpio_census,"Raw state"},
    {probe_uart_b,     "Raw state"},
    {probe_reset,      "Raw state"},
    {probe_pmc_scratch,"Raw state"},
};
const u32 g_page_groups[] = {
    0,
    PROBE_G_IDENTITY, PROBE_G_IDENTITY, PROBE_G_IDENTITY,
    PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER,
    PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER, PROBE_G_POWER,
    PROBE_G_MEMORY, PROBE_G_MEMORY,
    PROBE_G_STORAGE, PROBE_G_STORAGE, PROBE_G_STORAGE, PROBE_G_STORAGE,
    PROBE_G_STORAGE, PROBE_G_STORAGE, PROBE_G_STORAGE, PROBE_G_STORAGE,
    PROBE_G_STORAGE, PROBE_G_STORAGE,
    PROBE_G_WIRELESS, PROBE_G_WIRELESS, PROBE_G_WIRELESS,
    PROBE_G_AUDIO, PROBE_G_AUDIO, PROBE_G_SOUND,
    PROBE_G_DISPLAY, PROBE_G_DISPLAY,
    PROBE_G_INPUTS, PROBE_G_INPUTS, PROBE_G_INPUTS, PROBE_G_INPUTS,
    PROBE_G_RAW, PROBE_G_RAW, PROBE_G_RAW, PROBE_G_RAW,
};
const char *const nyx_diag_function_names[] = {
"probe_verdict",
"probe_soc",
"probe_fuses",
"probe_kfuse",
"probe_pmic",
"probe_max77812",
"probe_pmic_gpios",
"probe_regulators",
"probe_5v",
"probe_battery",
"probe_charger",
"probe_usbpd",
"probe_thermal",
"probe_fan",
"probe_dram",
"probe_clocks",
"probe_storage",
"probe_serial",
"probe_partitions",
"probe_emmc_health",
"probe_gpt",
"probe_boot0_pkg1",
"probe_autorcm",
"probe_sd_content",
"probe_storage_errors",
"probe_gamecard",
"probe_bt_radio",
"probe_wifi_pcie",
"probe_wifi_link",
"probe_audio",
"probe_audio_clocks",
"probe_audio_beep",
"probe_display",
"probe_backlight",
"probe_touch",
"probe_als",
"probe_joycon",
"probe_inputs",
"probe_gpio_census",
"probe_uart_b",
"probe_reset",
"probe_pmc_scratch",
};
