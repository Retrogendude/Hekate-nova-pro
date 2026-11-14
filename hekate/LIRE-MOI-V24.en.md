# Hekate Nova Pro 24 — fix for the Joy-Con key import
**Languages:** [Français](LIRE-MOI-V24.md) · **English** · [Español](LIRE-MOI-V24.es.md) · [Русский](LIRE-MOI-V24.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. Nova interface and settings preserved.

The v23 report showed an authentication refusal 05 and no ACL exchange.
An error was identified in Nova: a second inversion of the bytes of
the key already prepared by Hekate. It is removed in this version.
The comparison with the import path of Switchroot is described in
AUTH-SOURCES-V24.md. The key type and the security are preserved.

The build and the software tests pass. The operation of the wireless
commands still has to be confirmed on a console.

## Installation and trial

1. Copy the content of the archive to the root of the SD card, replacing the files
   of the **bootloader** folder. The settings files are not supplied.
2. Use the **hekate_nova_pro_6.5.5_v24.bin** payload if necessary.
3. Completely restart the console. In Nyx, open **Settings → TV / Dock →
   Wireless Joy-Con**.
4. Attach the two Joy-Con and choose **Read pairings**.
5. Detach the Joy-Con, choose **Enable Bluetooth**, then press a button.
6. Check that HID and Reports increase, then try the stick and A / ZL / ZR.
7. Choose **Stop Bluetooth** to save the new report:
   `bootloader/bluetooth/last-session.txt`.

The pairings have to be read again after the restart: the keys remain only
in memory. It is not necessary to delete the settings or the existing
pairings. The service does not create a new pairing.

The report adds **HOST=1** when the Bluetooth identity read again corresponds to
the expected one. **HOST=2 / error 9** signals a different identity and interrupts
the connection. **AUTH=05** indicates an authentication refusal; **AUTH=00** is
a success only if an authentication event has actually taken place
(visible in the trace). HID and Reports remain the indicators of the commands.
The keys, addresses and raw packet contents are never logged.

The previous fixes of the Nyx header, the dock and the diagnostics are
preserved. BTstack keeps its distinct personal/non-commercial licence,
included in the documentation of the package.
