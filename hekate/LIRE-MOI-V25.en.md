# Hekate Nova Pro25 — Joy-Con reconnection trial
**Languages:** [Français](LIRE-MOI-V25.md) · **English** · [Español](LIRE-MOI-V25.es.md) · [Русский](LIRE-MOI-V25.ru.md)

Preserved base: Hekate 6.5.5 / Nyx 1.9.5. Nova interface and settings preserved.

The v24 report confirms a successful authentication for one connection, then
a loss of the link during the enabling of encryption (08). The fix
of the key format from v24 is preserved. A connection problem remains.

This version requires the console to assume the primary role when a Joy-Con
initiates the reconnection, and starts the outgoing attempts successively.
These changes are verified on PC. Their effectiveness on the console remains
to be confirmed: the report alone does not make it possible to attribute with certainty the
cut to the Bluetooth role.

## Install and try

1. Copy the content of the ZIP to the root of the SD card, replacing the files
   of the **bootloader** folder. The settings files are not supplied.
2. Use **hekate_nova_pro_6.5.5_v25.bin** if necessary, then completely restart
   the console.
3. Open **Settings → TV / Dock → Wireless Joy-Con**. Attach the Joy-Con,
   choose **Read pairings**, then detach them and enable Bluetooth.
4. Press a button to wake them and try the buttons/sticks.
5. Choose **Stop Bluetooth** after the trial. The report is in
   `bootloader/bluetooth/last-session.txt`.

Do not use the state of the LEDs alone as a criterion: the report
counters and the operation of the commands are the useful indicators.
The keys remain in memory and have to be read again after each restart.

## New report

- AUTH=00 SEEN=1: an event confirms a successful authentication.
- ENC=00 SEEN=1 ENABLED=1 or 2: encryption enabled as long as the link is active.
- ENC=08 SEEN=1: the link expired during this step.
- DISC: disconnection reason preserved.
- ROLE=00: primary role known to the stack; 01: secondary; FF: unknown.
  The role changes received appear in trace13; the policy
  requested at acceptance appears in trace14.
- HOST=1: identity of the controller conforming to that of the imported pairings.
- SEEN=0: no corresponding event observed; 00 alone does not prove a success.

The last results remain available after Stop, even if it is pressed
a second time. The next start of Bluetooth opens a new session.
The report includes the controller version, without addresses, keys or raw packets.

The security of the link is preserved. No new pairing and no key writing are
performed. The previous fixes of Nyx, the dock and the diagnostics are
preserved. The own licence of BTstack is included in the documentation.
