# tv-experiment-01 — Standalone TV trial 01
**Languages:** [Français](tv-experiment-01.md) · **English** · [Español](tv-experiment-01.es.md) · [Русский](tv-experiment-01.ru.md)

- Standalone reference TV program on a Hekate 6.5.5 base; distinct from the Nova versions.
- Trial on official dock; keep the limits and the VOL− procedure described in the original notice.

**Delivery type:** compiled preview, sources and original files preserved.

## Files and checksums

| File | Bytes | SHA-256 |
|---|---:|---|
| [hekate-tv-experiment-01-sources.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-01/hekate-tv-experiment-01-sources.zip) | 1,690,037 | e8bc2344798462014b5236979c013f36dbe583e7ec70b7cc2a3573ace816e476 |
| [hekate-tv-experiment-01.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-01/hekate-tv-experiment-01.zip) | 912,066 | 5bbcf52e99cbae1be19dc4b2092e84c178c153ae7757a1b7eb1ac6a387fa9dfc |

## Full historical notice

The instructions and paths below relate to this version's archive. They do not automatically describe the latest version. The limitations and validations remain those stated at the time.

# Hekate — TV prototype 01

First TV output trial integrated into your Hekate 6.5.5 / Nyx 1.9.5 base.
It uses the classic interface of this base, distinct from Nova Pro v18.
The existing BM92T36 diagnostics are kept.

This prototype is compiled and verified on PC. The image, its stability and the
connections/reconnections still have to be validated on console.

## Installation and launch

1. Use a test SD card, or fully back up your `bootloader` folder before
   copying the files of the prototype to the root.
   The package replaces neither `hekate_ipl.ini`, nor `nyx.ini`, nor `update.bin`.
2. Inject **hekate_tv_experiment_01_6.5.5.bin**. This file must be launched
   explicitly; the Nova Pro v18 payload does not load this version of Nyx.
3. Outside the dock, open **Console Info → Battery → I2C Diagnostics →
   TV Test (experimental)**.
4. Connect the official dock to its power supply and to the TV via HDMI.
   Switch on the TV and choose the corresponding HDMI input.
5. Press **Start**, then place the console in the dock within
   45 seconds. You can also start with the console already connected to the dock
   if the button is accessible.
6. Observe the TV. The requested mode is **1280 × 720 at 60 Hz**. The text
   "configured (to be checked)" means that the software initialisation succeeded;
   the image on the TV must be confirmed visually.
7. **VOL-**, **Stop** or **Close** cuts the session. The console screen
   remains active. A detected disconnection also cuts the output.

## Report to recover after the trial

On the SD: **bootloader/tv-test/last-run.txt**.
The log contains the identifiers, negotiation steps, registers, reading
of the DisplayPort receiver and diagnostic of the video link. It is replaced at each
new trial; opening then closing the window without starting preserves the report.
The closing of the test saves the last lines. If the SD is absent,
full or unreadable, the test can work without a saved report.

To continue the development, provide this file with: console model
(V1/V2/OLED), dock model, presence or absence of image, stability of the image,
and behaviour after stop/removal. A black TV screen is not enough to designate
a faulty component.

## Scope of this stage

- Negotiation of USB-C power / dock mode / DisplayPort on demand.
- AUX, DPCD and EDID reading; link training; second display head.
- Copy of the test window, without rotation of the TV image.
- No automatic reset of the BM92T36. Switch Lite excluded.
- Explicit stop, 45 s limit on the negotiation, log bounded to 32 KiB.
- Release of the buffer after stop of the video engine; restoration of the saved
  LDO7/LDO8 settings, HPD pad and BPMP frequency.

The video initialisation phase is synchronous, with its own bounded waits.
During this phase, the buttons are processed when it hands back control; the
45 seconds do not constitute an absolute maximum delay of this phase.
The negotiation may leave the power contract active after the stop of the
test; a removal from the dock terminates this contract. The complete restoration of
all the port states and the transitions to the OS remain to be validated.

The mirror stops when leaving this window. The permanent TV output in
all the Nova menus, the HDMI audio, the touchless navigation and the automatic
startup in dock mode are not yet delivered.

To return to Nova Pro v18, restore the saved `bootloader` folder then
relaunch its payload. No file of the `Desktop/nd5` source folder has been changed.

See `documentation/TV-SOURCES.md` and `documentation/VERIFICATION-TV-01.json`
for the provenance and the verifications of this build.
