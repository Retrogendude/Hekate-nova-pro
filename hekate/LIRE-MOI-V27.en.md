# Hekate Nova Pro27 — Bluetooth profile adapted from HWTest-RCM
**Languages:** [Français](LIRE-MOI-V27.md) · **English** · [Español](LIRE-MOI-V27.es.md) · [Русский](LIRE-MOI-V27.ru.md)

The v27 takes the settings of the wireless controller service of HWTest-RCM:
basic radio rate, sleep management, transmission cadence and extended serial
reading. The startup also uses its power supply and clock settings,
with restoration of the previous state at stop.

The Nova interface, the two Joy-Con, the imported keys, the security and the
previous connection fixes are preserved. The Wi-Fi and Bluetooth diagnostics
already integrated remain in place. Base: Hekate 6.5.5 / Nyx 1.9.5.

Build and software tests successful. The operation on console remains
to be confirmed. The detail of the adaptations is in HWTEST-BLUETOOTH-SOURCES.md.

1. Copy the ZIP to the root of the SD card, replacing the **bootloader** files.
2. Use **hekate_nova_pro_6.5.5_v27.bin** if necessary, then restart.
3. Attach the Joy-Con and choose **Read pairings** in their wireless menu.
4. Detach them, enable Bluetooth, press a button and test the commands.
5. Choose **Stop Bluetooth** after the trial to save
   `bootloader/bluetooth/last-session.txt`.

The report adds BR_ONLY, the mode changes and reception statistics.
The grouped configurations are now all detailed,
without addresses, keys or raw data. After Stop, HID/ENABLED may be 0;
the report counters and the last results remain preserved.

The user settings are not supplied in the ZIP. The keys remain
in memory and have to be read again after the restart. The existing licences,
including that of BTstack, are included.
