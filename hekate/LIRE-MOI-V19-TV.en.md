# Hekate Nova Pro 19-TV — first integration of the TV output
**Languages:** [Français](LIRE-MOI-V19-TV.md) · **English** · [Español](LIRE-MOI-V19-TV.es.md) · [Русский](LIRE-MOI-V19-TV.ru.md)

This version builds on Nova Pro v18 and adds a manual TV output that stays
active in the menus. Base Hekate 6.5.4 / Nyx 1.9.4, Nova interface preserved.
The standalone prototype 01, based on 6.5.5, remains available separately.

## What has been validated

You confirmed a stable image and shutdown via VOL- with prototype 01 on
your Switch OLED inserted directly into an official dock. The reported failures
used a cable between the console and the dock. They are therefore not considered
as a confirmed driver failure in a direct connection. The exact type of
that cable and how it was connected have not been identified.

This Nova integration is new: it still has to be tried on the console.
Its PC tests do not replace that trial. The video driver and its parameters
remain identical to those of prototype 01; the transitions in Nova are new.

## Installation

1. Back up your current `bootloader` folder, or use a test SD card.
2. Copy to the root the `bootloader` folder supplied in this archive.
   The personal configuration files `hekate_ipl.ini` and `nyx.ini`
   are not supplied, in order to preserve your preferences.
3. Inject `hekate_nova_pro_6.5.4_v19_tv.bin`. Check in the credits that the
   indicated Nova version is **19-TV**. Copying only the payload without the
   files from `bootloader/sys` does not update Nyx.

## Enabling the TV and returning to the menus

1. Connect the official dock to its power supply and to the television via HDMI.
2. Open **Nyx Settings → TV / Dock → TV USB-C / Dock**.
   The quick settings button at the top right opens the Nyx settings.
3. Choose **Enable TV** and insert the console directly into the dock.
   You can launch it outside the dock: you have 45 seconds to put it in place.
4. After connection, close this window. The TV output stays active on the home
   screen, the tabs and the Nova windows. The mode is 1280 × 720 at 60 Hz.
5. **VOL-** stops the TV output at any time between initialization phases.
   The **Stop TV** button is also available by reopening its settings.
6. Removing the console or losing the dock signal stops the session. To
   resume it, put the console back and use **Enable TV** again.

The console screen stays on. The existing Nyx controls are preserved,
including the pointer with the Joy-Con on the rails when that option is active.
This version does not add Bluetooth connection of detached controllers.

## Shutdown before another hardware function

Nova cuts the TV output before a system or payload launch, a
reload, a shutdown/restart, an active hardware diagnostic or USB
use of the storage/controller type. The TV buffer is released after the display
engine has stopped. The menu frequency is restored after dock negotiation.

An active diagnostic can therefore come back on the console screen. The result
remains accessible in Nova; you can then re-enable the TV manually.
The memory test keeps its separate payload and its existing return to Hekate.

HDMI audio, automatic activation at boot and connection with an
intermediate cable are not validated by this delivery.

## Reports

The reports are written to the SD card in `bootloader/tv-test/`:

- `last-run.txt`: last trial;
- `run-001.txt`, `run-002.txt`, etc.: distinct copy of each trial, up to 999.

An error of mine in prototypes 01/02 prevented the normal writing
of the reports: the result of `sd_mount()` was interpreted the wrong way round.
This version uses the real BDK contract, verified separately on PC.
It is therefore not necessary to look for a missing log from prototype 01.

To validate this integration, check the image on the home screen after closing the
TV window, the opening of a few sub-menus, VOL-, then a reconnection.
In case of a problem, attach the report of the trial together with what appears on the screen.

## Provenance and feedback

The source archive contains the exact files used, the tests, their
results, the provenance of the drivers and the Hekate licence. See
`documentation/TV-SOURCES.md` before any public redistribution.

Nova v18 and prototypes 01/02 are kept in their previous archives.
To roll back, restore the backed-up `bootloader` folder and launch
the corresponding payload. No modification of your source folder `Desktop/nd5`
was made during this development.
