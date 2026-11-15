# Hekate Nova Pro 21-BT — personal prototype
**Languages:** [Français](LIRE-MOI-V21-BT.md) · **English** · [Español](LIRE-MOI-V21-BT.es.md) · [Русский](LIRE-MOI-V21-BT.ru.md)

Base Hekate 6.5.4 / Nyx 1.9.4, from Nova 20-Dock.

## Fixes from your screenshots

- USB tools: a single frame for the Read-only banner. The switch and
  the eMMC / emuMMC protection keep their operation.
- Credits: Nova panel restored behind all the tabs. The wallpaper
  stays behind the content, with the transparency chosen in the settings.
- New Changelog tab: versions 21, 20, 19 and 18 on a single page.
  The tabs stay at the top; no pagination added at the bottom.

## Install

Keep a copy of your current bootloader folder. Copy the bootloader folder
supplied to the root of the SD card and launch hekate_nova_pro_6.5.4_v21_bt.bin.
The package does not contain any nyx.ini or hekate_ipl.ini: your settings stay
those of your card. Check "21-BT" in the Nova Pro tab of the credits.

The TV functions of v20 are preserved. Insert the Switch OLED directly
into the official dock, with power and HDMI. The intermediate cable that
you were using during the first trials is not part of the reference setup.

## Trying the wireless Joy-Con

The service is experimental and disabled at each boot. It contains a
real Bluetooth Classic / L2CAP / HID stack and a link with the Nyx
controls. The software trials do not yet validate a real radio connection.

1. Use Nintendo Joy-Con already paired to this console in its system.
2. Attach them to both rails. The wired Joy-Con driver of Nyx must be active.
3. Open Nyx Settings → TV / Dock → Wireless Joy-Con.
4. Press Read pairings. The screen must indicate 2 / 2; a single Joy-Con
   can also be imported if necessary.
5. Detach them, then press Enable Bluetooth. Press a button of each
   Joy-Con to wake them. Leave the sticks at rest during the centring.
6. Watch the transition to Bluetooth ready then Commands received. Check the cursor,
   the selection with A / ZL / ZR, then the navigation when the TV output is active.

Closing this window leaves the service active. Stopping Bluetooth releases the radio.
Erasing the keys also deletes the pairings imported in the session memory.
The reading has to be done again after a restart. No key is saved to the SD card,
and this prototype does not modify the pairings stored in the Joy-Con.

The rails have priority if a Joy-Con is attached. The wireless commands are
released after 500 ms without a report received, in order to avoid a held selection.
The service is stopped before a diagnostic, a USB mode or the hardware end of Nyx.
After a diagnostic or a USB mode, restart it manually if necessary.

This first path uses the pairings of the detachable Joy-Con. It does not yet add
the search and pairing of new controllers, nor the Pro controllers.
The import via rails is not available on Switch Lite.

## If the connection fails

Photograph the state, the counters and the code displayed in Wireless Joy-Con.
An error of this experimental service is not a verdict of hardware failure.

| Code | Step concerned |
| --- | --- |
| 1 | Pairings absent, invalid or belonging to different hosts |
| 2 | UART-D already busy: the radio has not been reset |
| 3 | HCI initialization not completed within 15 seconds |
| 4 | UART reception error |
| 5 | Invalid, too large or incomplete H4 frame after 2 seconds |
| 6 | Transmission blocked for more than 2 seconds |
| 8 | Hardware error event signalled by the HCI controller |

Startup uses the ROM code of the CYW4356. No proprietary firmware
is supplied or installed. The eventual need for a firmware patch
remains to be determined from a real trial on the console.

## Checks performed

- ARM build of Hekate and Nyx.
- Initialization of the real BTstack stack with a substituted HCI controller,
  three start / stop cycles, partial and too large frames,
  pairings, buttons, sticks, disconnections and error timeouts.
- Real UART / GPIO path exercised with substituted registers: restoration of
  the pins and the clock, burst rate, CTS and UART faults. PH0 remains intact.
- Merging of commands: priority to the rails, no modification of the original
  wired report, neutralisation of old data.
- 462 native LVGL views: 7 languages, 3 fonts, sizes 20 and 36, Carbon background and
  25 % opacity. Simulated clicks on Read-only in both of its states.
- 43 scenarios of the TV controller and 18 regressions of the dock protocol.

These checks substitute the hardware inputs. They do not constitute
a Bluetooth or HDMI trial on this v21 integration. The only physical feedback received
remains that of prototype TV01: OLED directly into the official dock, stable
image and shutdown via VOL−.

## Sources and licences

BTstack is supplied with its own BlueKitchen licence, to be kept with its
sources and the binaries. It includes a personal and non-commercial use condition.
This personal prototype therefore does not present all of its components
as being under GPL. The full text is in documentation/BTSTACK-LICENSE.txt.
The licences and credits of the other components remain attached to the sources.

References:
- [BTstack, sources and licence](https://github.com/bluekitchen/btstack)
- [Joy-Con pairing format](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md)
- [Joy-Con Bluetooth HID reports](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md)
- See also TV-SOURCES.md for the origin of the dock driver.
