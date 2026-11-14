# Hekate Nova Pro26 — fix for the simultaneous Joy-Con channel openings
**Languages:** [Français](LIRE-MOI-V26.md) · **English** · [Español](LIRE-MOI-V26.es.md) · [Русский](LIRE-MOI-V26.ru.md)

Preserved base: Hekate 6.5.5 / Nyx 1.9.5. Nova interface and settings preserved.

Your v25 report confirms successful authentications and enablements of encryption,
as well as the first ACL exchanges. The next blocker is visible:
Nova and the Joy-Con simultaneously request the opening of the controller channel, then
Nova refuses the incoming request with 0004. No button report arrives.

The v26 accepts the incoming request when an outgoing attempt is still
pending. It keeps the same controller session and isolates the late responses of
the old attempt. Duplicates are checked and the old channels cleaned up.
The security, the key format and the primary role setting are preserved.

The refusal was reproduced on PC with the v25 code. The v26 tests pass for the
simultaneous openings, the commands, the duplicates, the late responses and the
cleanup. The real operation still has to be confirmed on your console.

## Install and try

1. Copy the content of the ZIP to the root of the SD card, replacing the files of the
   **bootloader** folder. The settings files are not supplied.
2. Use **hekate_nova_pro_6.5.5_v26.bin** if necessary, then completely
   restart.
3. Open **Settings → TV / Dock → Wireless Joy-Con**. Attach the Joy-Con and
   choose **Read pairings**.
4. Detach the Joy-Con, enable Bluetooth and press a button to
   wake them. Try the sticks and the buttons A / ZL / ZR.
5. Choose **Stop Bluetooth** after the trial. The report remains saved in
   `bootloader/bluetooth/last-session.txt`.

The counters **Reports**, **ACK** and the commands actually received are
the useful indicators. After the stop, HID and ENABLED may be 0: the
service is cut. The counters and the last events remain preserved.
SEEN=1 indicates that an event has been observed; 00 with SEEN=0 does not prove a
success. The addresses, keys and raw packet contents are not recorded.

The next start of Bluetooth resets the session report to zero. The
keys remain in memory; they have to be read again after a restart.
The distinct licence of BTstack and the detail of the modifications are included.
