# tv-experiment-02 — Standalone TV trial 02
**Languages:** [Français](tv-experiment-02.md) · **English** · [Español](tv-experiment-02.es.md) · [Русский](tv-experiment-02.ru.md)

- Standalone reference TV program on a Hekate 6.5.5 base; distinct from the Nova versions.
- Trial on official dock; keep the limits and the VOL− procedure described in the original notice.

**Delivery type:** compiled preview, sources and original files preserved.

## Files and checksums

| File | Bytes | SHA-256 |
|---|---:|---|
| [hekate-tv-experiment-02-sources.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-02/hekate-tv-experiment-02-sources.zip) | 1,692,017 | cbe08ee9441b68a086e29cc358dac1d681500b2d70ef12325fbbd37dcb99468a |
| [hekate-tv-experiment-02.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-02/hekate-tv-experiment-02.zip) | 912,982 | 3098248e4d29e1fa175a5fc06cb557d34839638cff7c54fdfe67cc3814cc8ceb |

## Full historical notice

The instructions and paths below relate to this version's archive. They do not automatically describe the latest version. The limitations and validations remain those stated at the time.

# Hekate — TV prototype 02

Reliability prototype for the TV output integrated into your Hekate 6.5.5 / Nyx 1.9.5 base.
It uses the classic interface of this base, distinct from Nova Pro v18.
The existing BM92T36 diagnostics are kept.

Prototype 01 displayed a stable image on a Switch OLED with official dock,
and VOL− worked. Failing trials were then reported,
after VOL− as well as after a full restart. The TV output remains intermittent.

This version 02 fixes two defects reproduced in the code: the loss
of a command confirmation while the controller is busy, and
the resetting of the counter during new requests to the dock hub.
The reports are timestamped and separated per trial. The hardware protocol and
the video link parameters are kept. There is no reset nor new
automatic complete attempt. These fixes do not yet prove
that they resolve the failures observed on the console.

This prototype is compiled and verified on PC. The image, its stability and the
connections/reconnections still have to be validated on console.

## Installation and launch

1. Use a test SD card, or fully back up your `bootloader` folder before
   copying the files of the prototype to the root.
   The package replaces neither `hekate_ipl.ini`, nor `nyx.ini`, nor `update.bin`.
2. Inject **hekate_tv_experiment_02_6.5.5.bin**. This file must be launched
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
of the DisplayPort receiver and diagnostic of the video link. A distinct copy of each trial is kept under `run-001.txt`,
`run-002.txt`, etc. The `last-run.txt` file remains a shortcut to the
last trial. Opening then closing the window without starting preserves the reports.
The closing of the test saves the last lines. If the SD is absent,
full or unreadable, the test can work without a saved report.

To continue the development, provide the displayed error message and the `run-NNN.txt` files
of a successful and a failed trial, with: console model
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

See `documentation/TV-SOURCES.md` and `documentation/VERIFICATION-TV-02.json`
for the provenance and the verifications of this build.
