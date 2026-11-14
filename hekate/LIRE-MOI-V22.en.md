# Hekate Nova Pro 22
**Languages:** [Français](LIRE-MOI-V22.md) · **English** · [Español](LIRE-MOI-V22.es.md) · [Русский](LIRE-MOI-V22.ru.md)

Updated functional base: Hekate 6.5.5 / Nyx 1.9.5, with the Nova style.
This version fixes the erroneous Nyx warning and several possible blockers
of the wireless commands. The Bluetooth link remains experimental.

## Install

1. Keep a copy of your current bootloader folder.
2. Copy the bootloader folder supplied to the root of the SD card and accept
   the replacement of the files. It is not necessary to delete the folder.
3. Inject hekate_nova_pro_6.5.5_v22.bin, or use the bootloader/update.bin supplied.
4. Check "22" in the credits and "Hekate 6.5.5 / Nyx 1.9.5".

The personal files nyx.ini and hekate_ipl.ini are not supplied in this
package. Your settings, themes, backgrounds, fonts and boot configurations are
preserved. Replacing only the payload without updating nyx.bin is not enough.

## "Old Nyx GUI found" warning

The v21 delivered placed the Nyx metadata at a wrong position in the binary.
That triggered a false warning even when all the correct files were
present. The Nyx entry is now isolated, and the build is refused if
the placement of the metadata changes. The compatibility check of Hekate remains
active: a real mix of versions can still trigger this warning.

## Hekate / Nyx update

The official changes relating to HOS 23.5.0 are taken into the core of Nova,
with the NOGC fix for FAT32 published after the 6.5.5 tag. The panels, icons,
colours and Nova layouts remain those of the previous version.

The emuMMC module and the L4T component sc7exit_b01.bin come from the official
6.5.5 package updated, whose fingerprint was checked. Nyx and the payload remain the
Nova binaries built for this version, and are not replaced by the official
interface. The Nova resources and the diagnostics are preserved.

Porting references:
- [Hekate 6.5.5 / Nyx 1.9.5 release](https://github.com/CTCaer/hekate/releases/tag/v6.5.5)
- Tag code: 9593272c54f8dc127a5a25f91e1ac90e1be6205d.
- Final FAT32 fix: 8e1392df4569ac501a115867056b13c361429e9e.

## Wireless Joy-Con

In Nyx Settings → TV / Dock → Wireless Joy-Con:

1. Attach the Joy-Con already paired to this console, then choose Read pairings.
2. Check the number imported, detach the Joy-Con and choose Enable Bluetooth.
3. Wake the Joy-Con with a button, leave the sticks at rest for a moment,
   then move the stick and check A / ZL / ZR.

The keys stay in memory during the session. They are not written to the SD card
and the pairings stored in the Joy-Con are not modified. The service remains
disabled at each boot. It is stopped before the diagnostics, the USB mode and
the launch of another system; restart it manually after a diagnostic.

Navigation fixes:
- If the chosen side is absent, the stick of the other connected Joy-Con can drive
  the cursor. The saved setting of the preferred side is preserved.
- A / ZL / ZR wake the hidden cursor and remain usable during the centring.
- The simple 0x3F reports are decoded, in addition to 0x21 and 0x30. The confirmation
  of the full mode is tracked, and the old commands are released.

The HCI counters alone do not prove that the buttons are received. An additional
status line now makes it possible to distinguish:

| Field | Reading |
| --- | --- |
| Joy-Con reports | Number of reports actually decoded, left / right |
| ID | Report type, generally 30 in full mode, 3F in simple mode |
| HID | 1 when the controller connection is open |
| ACK | 1 when the full mode is confirmed or a 30 report is received |
| ERR | Last failure code when opening the connection, 00 when no error is recorded |
| BTN | Decoded buttons; the value must change during a press |
| XY | Stick values; about 2048,2048 at rest in full mode |

If the commands still do not respond, note this line while holding A,
then while moving the stick. This makes it possible to separate a connection problem, a decoding
problem and a navigation problem. No failure of the prototype is by itself a
diagnosis of hardware failure.

## Checks and limits

The ARM build, the positions and versions of the headers, the real decoding /
merging / pointer paths up to the LVGL click, the menus and the TV
regressions are checked on PC with substituted hardware inputs. Two navigation
blockers were reproduced on the v21 code before checking the fixes.
The verification program also refuses a Nyx input that shifts the header.

A physical validation of this v22 on your console remains necessary: the PC
tests do not prove a radio connection, an HOS boot or a real HDMI output.
The settings of the video driver and the dock detection remain those of Nova v20/v21.

## Licences

This version keeps the credits and licences of the components. BTstack is subject
to the personal and non-commercial BlueKitchen licence attached to the package; all
the components are therefore not presented as being under GPL. No proprietary
Bluetooth firmware is supplied. See also the documents of the previous versions.

Reference for the Joy-Con simple format: [SDL Switch driver](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_switch.c),
for the left/right orientations in vertical use. No SDL code is
incorporated in the stack; the Nova decoder uses the documented formats.
