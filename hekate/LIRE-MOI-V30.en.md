# Hekate Nova Pro 30 — Bluetooth in the background and smoothness
**Languages:** [Français](LIRE-MOI-V30.md) · **English** · [Español](LIRE-MOI-V30.es.md) · [Русский](LIRE-MOI-V30.ru.md)

At the start of Nova, the driver checks the physical presence on the rails.

- No Joy-Con attached: loading of `bootloader/bluetooth/joycon-pairing.bin`,
  then automatic activation of Bluetooth. The `.bak` copy is tried if
  necessary. The service starts from the home screen without opening a window.
- At least one Joy-Con attached: no SD loading and no automatic Bluetooth
  start. The usual operation of the rails is preserved.
- No valid pairing saved: the start continues with Bluetooth
  stopped, without radio error or blocking window.

This decision is taken once per start, after initialization of the
rails. A connection detected during the SD reading cancels the automatic
start. The manual stop is respected. Detaching a Joy-Con afterwards does not
restart the service automatically in that same session. The disabled driver
and the models without rails supported by this driver are excluded
from this automatic start.

The v29 backups are compatible. Use the one corresponding to
this console and its controllers, in particular for the trials on two OLEDs. If they do not exist yet,
attach the controllers, read the pairings and save them once from
the Wireless Joy-Con menu, Bluetooth stopped. The `.bin` and `.bak` files
contain private keys: do not attach them to the reports.

The service works in the background on the home screen and the ordinary menus.
Pressing a normal button may still be necessary to wake a
controller that is switched off. The hardware tests keep their necessary suspensions
during the measurements; v30 does not force their radio to stay busy.

The audit led to the following optimizations:

- Reflection: avoids the periodic overflows of the drawing queue, while
  preserving its appearance. Reduction of the redrawn surfaces verified on PC.
- Commands: keeps once the brief presses received between two readings,
  with expiry so as not to replay an old command after a freeze.
- Maintenance: the fast polling continues at the maintenance points of the long
  tools, outside the suspended diagnostics.
- Measurements: the report includes the maximum full-service time, its variants,
  the interval between calls and the number of RX slices at the ceiling.
- Resources: check of the sizes and the readings of `res.pak`; cleanup
  of the refused payloads, verification before allocation and complete reading.

Installation: merge the `bootloader` folder of the ZIP with that of the SD card,
replacing the supplied files in order to keep your private backup,
then restart.
Use `hekate_nova_pro_6.5.5_v30.bin` if necessary. No SD formatting or
replacement of your settings is necessary. The keys saved on the
card are not supplied or erased by this distribution.

After the trial, "Stop Bluetooth" saves
`bootloader/bluetooth/last-session.txt`. `Autostart=2` means that the automatic
launch was requested; it does not prove that the controllers are already
woken and connected. `Autostart=1` indicates the presence on the rails,
`3` the absence of a valid backup and `5` a detection unavailable.
The polling maxima describe the service on the host, not the complete latency
between a physical button and the image. No hardware FPS target is announced.

Build and PC tests are to be completed by a trial on both OLEDs.
Base: Hekate 6.5.5 / Nyx 1.9.5. Licences and audit reports included.
