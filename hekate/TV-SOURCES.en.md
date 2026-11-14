# Origin of the Nova 20-Dock TV integration
**Languages:** [Français](TV-SOURCES.md) · **English** · [Español](TV-SOURCES.es.md) · [Русский](TV-SOURCES.ru.md)

Nova Pro v18 base: separate copy of `work/hekate-6.5.4`, without replacing
the version already delivered. Video driver `dpout.c/.h` identical to the TV01 prototype.
The `usbpd.c/.h` driver takes the two TV02 protocol fixes, verified
by a regression on the command confirmation and on the hub budget.

HWTest-RCM / Creased origin:
https://github.com/Creased/HWTest-RCM/tree/3731bfb471515a5f83709aa496c9cdb0a9aeff83

Upstream driver references: BM92TXX and Tegra NVIDIA drivers from CTCaer /
switchroot L4T, and DisplayPort Tegra210 driver from coreboot.
Original comments and tuning tables kept. No adjustment of the
voltages, drive levels, link rates or timing diagrams for this port.

Nova changes: session independent of the settings window, copy of the
modified lines of its 1280x720 buffer, stop before hardware end/USB/diagnostic,
menu frequency restored after connection, new views in the Nova theme
and 36 TV/dock sentences translated into the seven available languages. The step names,
registers and raw driver details remain diagnostic information.

Fix separate from the SD recording: use of `sd_mount() == 0`
to recognise success, with a test of the real body of this function.
The previous prototypes used an inverted test and an incorrect mock.

Reported physical validation: prototype TV01, OLED inserted directly into
an official dock, stable image and functional VOL- stop. The trials using
a console/dock cable do not serve to qualify the direct connection.
The physical validation of this new Nova port remains to be carried out.

Hekate and the integration follow the licences supplied in the project.
The HWTest-RCM repository consulted does not state an autonomous licence for these
new files: clarification necessary before a public distribution of the whole.
Personal development delivery, with provenance preserved.

## Automatic detection in 20-Dock

Preferences `tv_auto` (default 0) and `tv_keep_screen` (default 1) added at the end
of the Nyx configuration in order to preserve the old fields. Strict loading,
atomic writing by the existing mechanism. Activation with Save.

The passive detector reads STATUS1, the PDO offer and the controller identifier
when no cable is signalled. It does not read ALERT_STATUS and sends no
command. It uses the power supply signature already recognised by this driver
for a Nintendo dock; the complete identification/negotiation comes afterwards.
Two positive samples are required after an initial delay of 2 seconds.
One attempt per detected insertion; the errors or a manual stop do not
trigger an immediate repetition. The reading errors space the polls to 2 then 5
seconds and are not assimilated to a removal.

The service suspends its monitoring during USB/diagnostics and restores it
on their return, without cancelling a prior manual stop. The TV only choice
turns off the backlight after video initialization, and restores it at stop
with the current brightness. The output to another system avoids a backlight
flash just before the end of the LCD driver.

No new Bluetooth service is integrated: see the dedicated Joy-Con study.
The new automatic detection and the screen switching remain to be validated
on console, separately from the success of prototype 01 in a direct dock.
