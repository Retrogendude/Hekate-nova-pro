# Hekate Nova Pro — beta v32
**Languages:** [Français](LIRE-MOI-V32.md) · **English** · [Español](LIRE-MOI-V32.es.md) · [Русский](LIRE-MOI-V32.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. Complete bootloader archive + corresponding payload.

## Expert partitioning

- Compact summary on a single line: title, allocations and validity state.
- Three rows more spaced out. The sizes are to the right of the sliders and the buttons.
- −/+ buttons redrawn, without overflowing shadow, with margins distinct from the slider and its handle. The handle stays entirely inside the bar, including at zero.
- Fine adjustment by −/+ preserved, with repeat on held press. Linux/Android: 1 GB step after their minimum. Hekate emuMMC steps preserved, including 29 GB Full, 58 GB Full and double emuMMCs.
- Whole GB on display; sizes, reservations, alignments and erase confirmations unchanged. No partitioning operation is launched by the update.
- Nova theme, wallpaper and transparency preserved. The new labels are translated into the seven languages.

## Bluetooth: recovery of a silent Joy-Con

When a complete stream already established stays silent for one second, Nova requests the full report mode again. If the silence lasts four seconds, Nova requests a disconnection of this Joy-Con only and tries a new connection with the existing pairings, one second after the effective HID closing. Without a first complete report, a margin of eight seconds leaves time for the initial configuration.

The other Joy-Con continues its processing. The recovery does not restart the radio and does not modify the keys. It waits for the end of the UART data already being read: a partial packet or a reached read budget must not trigger this recovery. The simple mode, which sends its changes on demand, is not treated as a missing periodic stream. The manual stop and the automatic start of v31 remain respected.

The initialization also holds WL_REG_ON high during the Bluetooth session, as the HWTest-RCM reference does after its common reset. The previous level is restored at stop, whether it was high or low. No Wi-Fi network service is started. This modification brings the sequence closer to the reference; its effect on the range is to be checked on the console.

**The 2–3 metre objective remains a hardware trial to be carried out. This beta does not yet guarantee this range.** The v31 report received indicates report cuts of up to 2.8 s, while the maximum processing on the host side is about 22 ms. It also shows negative relative RSSI and a drop in the vendor link quality. These data point towards the radio link; they do not prove a precise hardware cause nor a gain brought by v32.

No forced increase in power, EDR/UART modification or HCD firmware load is applied. The radio stays in ROM. The HCI measurements added in v31 remain available. A connection recovery improves the recovery after an interruption; it does not by itself increase the range of the signal.

The v32 report adds Radio_power_policy, as well as Stream report_restarts / reconnects per Joy-Con and the recovery events (kind=21). These counters distinguish the recovery requests from the reconnections actually completed; the presence of new reports and HID=1 makes it possible to confirm the return of the commands. The counters are kept during the export, until the next start of Bluetooth.

## Installation and trial

1. Console switched off, keep a copy of bootloader and of the personal files.
2. Merge the bootloader folder of the archive with that of the SD card, replacing the supplied files. Keep the personal configuration and pairing files.
3. Use hekate_nova_pro_6.5.5_v32.bin if the boot uses an external payload. bootloader/update.bin is also supplied.
4. Keep bootloader/bluetooth/joycon-pairing.bin and its .bak backup: format unchanged, no reading again or deletion of keys necessary to update.

Test the sticks and A/B at close range, then at 2 m and 3 m keeping the console in the same place. A prolonged stop of commands can now cause an automatic reconnection of the Joy-Con concerned. After each session, choose "Stop Bluetooth" and immediately keep last-session.txt under a distinct name. Indicate whether the console is in the dock.

Send only the text reports; the pairing .bin/.bak files contain private keys. The archive contains no private pairing data nor personal configuration.

## Validation

ARM build successful. 720 partitioning scenes with the real callbacks, seven languages, three fonts, two text sizes, wallpaper and transparency, including a capacity of 955 GB similar to the capture received. Coordinate checks: summary centred on a single row, separated −/+/handle/values zones and no scrolling.

882 complementary scenes and 28 wireless navigation trials check the existing rendering. The HCI/HID tests reproduce a silent right Joy-Con: v31 stays blocked, v32 requests the reports again then reconnects with the same keys while the left reports continue. Additional checks cover the timeouts, the pending data, the simple mode, the clock overflow, the stop and the restoration of the GPIOs.

The radios, video peripherals and storage media are substituted in these PC trials. No physical range measurement and no real partitioning have been executed. The details and fingerprints are in documentation/VERIFICATION-V32.json.

## Technical references

- HWTest-RCM radio sequence, frozen reference: https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/gui/gui_bt.c — _wl_reg_on_restore and _power_up.
- GAP API of BlueKitchen / BTstack: https://github.com/bluekitchen/btstack/blob/master/src/gap.h — gap_disconnect.
- Firmware lead, not integrated: the Switchroot repository lists a BT patch coming from the Nintendo system, distinct from the current ROM operation: https://gitlab.com/switchroot/android/proprietary_vendor_firmware/-/blob/main/README.md. This does not prove that its absence alone explains the present range defect.
