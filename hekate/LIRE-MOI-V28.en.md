# Hekate Nova Pro 28 — responsiveness and diagnostics of the pairings
**Languages:** [Français](LIRE-MOI-V28.md) · **English** · [Español](LIRE-MOI-V28.es.md) · [Русский](LIRE-MOI-V28.ru.md)

This version reduces the repeated Bluetooth waits during the drawing of the
menus: the serial reading uses a short budget of 250 microseconds in
the display calls and in the window timer. The main loop
keeps the extended reading required by the HWTest-RCM radio profile.
The statistics panel refreshes twice per second.
This targets the slowdowns reported in v27; the result on console
remains to be measured. It is not a guarantee of display framerate.

The menu now indicates the failure category of the import: rails
unavailable, data not read, Nintendo data not recognised by the reader,
or compatibility checks not satisfied. A 0/2 failure can be recorded
without starting Bluetooth. The validations and the key format remain those
of v27: **the cause of the 0/2 observed on console is not yet fixed**.

The report adds import indicators and, for each Joy-Con,
the buttons observed since the start of the session, the dates of the first and
last reports and their average cadence. It contains neither address, nor key,
nor raw packet content. "Buttons seen" keeps a press even if the
button has already been released. The A button of the right Joy-Con corresponds to bit 000008.
The average cadence covers the decoded reports, including the command
responses, and does not measure the latency of a press.

Installation and trial for the current problem:

1. Extract the ZIP to the root of the SD card, replacing the files of the
   bootloader folder. Use hekate_nova_pro_6.5.5_v28.bin if necessary, then restart.
2. Attach both Joy-Con and choose "Read pairings".
3. If the result is still 0/2, note the new message and choose
   "Stop Bluetooth", even without having started it. Send only
   bootloader/bluetooth/last-session.txt.
4. If the import succeeds, detach the Joy-Con, start Bluetooth, move
   the cursor with L and press A of the R several times. Then stop
   Bluetooth and retrieve the report before any new session.

The menus, the radio profile, the management of simultaneous connections, the
diagnostics and the TV mode are preserved. No user setting is
supplied in the ZIP. No new pairing or key change is added.
Base: Hekate 6.5.5 / Nyx 1.9.5. Licences and provenance included.

The checks on PC and the build do not replace the hardware trial.
In v27, a movement with L was reported with a strong latency, without
validation with A of the R. The last reports supplied show a connection
not established, then an import at 0/2: they do not describe this active session.
