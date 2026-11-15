# Hekate Nova Pro — beta v31
**Languages:** [Français](LIRE-MOI-V31.md) · **English** · [Español](LIRE-MOI-V31.es.md) · [Русский](LIRE-MOI-V31.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. This archive contains the complete bootloader folder and the corresponding payload.

## Expert partitioning menu

- Three settings across the full width, horizontal summary at the bottom.
- Longer sliders and −/+ buttons with repeat on held press.
- Sizes displayed in whole GB, without decimals. "29 GB Full" and "58 GB Full" markers, also for double emuMMCs.
- No modification of the real sizes, reservations or alignments: the displayed GB are rounded down to the whole number, like the Hekate markers. Android displays its total allocation, system included. The Hekate emuMMC steps are preserved; Linux and Android use 1 GB steps after their minimum.
- The existing erase confirmations remain active. The modification of the menu does not launch any partitioning.
- Nova theme preserved, including the wallpaper and the transparency.

## Bluetooth and smoothness

Bluetooth processing continues during long software drawings, image copies and display rotation waits. The transition from drawing is limited to a reception budget of 250 µs, at most once every 2 ms; the processing of a whole packet can exceed this budget. It launches neither another menu, nor a graphics task, nor any write to the SD card.

The operation of v30 is preserved: automatic start from the saved pairing file when both rails are empty at boot, without opening the Bluetooth menu. The presence of a Joy-Con on a rail prevents this automation. A manual stop remains respected until the next boot. The diagnostics that reserve the peripherals keep their dedicated operation.

This version adds read-only link measurements, spaced out (at most two requests per second in total). No power, EDR, UART rate, pairing or encryption setting has been changed. No additional HCD firmware is loaded.

**The range beyond 80 cm is not yet validated on console.** The v30 report shows a maximum of 118.5 ms between two processing passes on the host side and a longer reception interruption on the right; it does not prove a radio failure nor the exact cause of the distance. This beta improves the processing possibilities and makes it possible to measure what remains.

## Installation

1. Console switched off, keep a copy of your bootloader folder and of your personal files.
2. Merge the bootloader folder of this archive with that of the SD card, replacing the supplied files. Keep the new Nyx and the corresponding payload together.
3. Use hekate_nova_pro_6.5.5_v31.bin if your boot method uses an external payload. bootloader/update.bin is also supplied.
4. Keep bootloader/bluetooth/joycon-pairing.bin or its .bak backup: this update does not change their format. Do not erase the keys nor clean the whole SD card for this trial.

No private pairing file and no personal configuration file are included in the archive.

## Recommended trial

With the console in the same position, test one session at close range then another beyond 80 cm. In each session, move the sticks and press A/B for about 20 seconds. Choose "Stop Bluetooth" and immediately copy bootloader/bluetooth/last-session.txt before relaunching or reading the pairings again. Keep the two reports under distinct names.

Send only these text reports; the pairing .bin/.bak contain private keys.

The report adds:
- Capture_ms and number of passes during the drawing.
- Full_reports, age of the last full report, maximum between full reports and hole counters >50 / >250 ms. The acknowledgements are not counted as full reports. The session statistics do not constitute a button-to-screen latency measurement.
- RF kind=0: relative BR/EDR RSSI in dB; 0 means within the range of the controller, not a zero signal. It is not an absolute measurement in dBm.
- kind=1: vendor quality from 0 to 255, not a percentage.
- kind=2/3: current / maximum transmit power in dBm, read only.
- kind=4: counter of consecutive failed contacts of the controller, not a cumulation of all lost packets.
- For each measurement: number of samples, age, last value and session minimum/maximum. n/a means no valid measurement. Old values do not necessarily describe the link at the time of the export.
- Measurement errors: FD = power value out of range, FE = no response, FF = malformed response or wrong identifier. A measurement not supported or without response is disabled for this connection; it does not stop the commands. These codes are independent of the authentication errors.

## Verification

ARM build and automatic checks documented in VERIFICATION-V31.json. The PC trials cover the real LVGL rendering, the real partitioning callbacks, the simulated HCI/L2CAP/HID exchanges and the existing pairing paths. The peripherals and the SD card are substituted in these trials: no real partitioning operation and no physical range measurement have been performed.
