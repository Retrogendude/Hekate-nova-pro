# Hekate Nova Pro 23 — Joy-Con connection trial
**Languages:** [Français](LIRE-MOI-V23.md) · **English** · [Español](LIRE-MOI-V23.es.md) · [Русский](LIRE-MOI-V23.ru.md)

Preserved base: Hekate 6.5.5 / Nyx 1.9.5. Nova interface, settings, dock and
diagnostics preserved. The radio connection of this version still has to be validated on
the console: the PC tests described below do not replace a physical trial.

## What your v22 capture shows

Pairings imported 2/2, no report, HID 0/0 and ERR 69/00. The radio answers,
but no controller connection transmits commands. The hexadecimal code 69
is a timeout during an L2CAP negotiation; it does not designate a faulty
button. The HCI statistics alone do not prove a Joy-Con connection.

The v22 began with a search for an SDP service descriptor before opening
the controller channels. This v23 uses directly the standard HID channels known
to the Joy-Con: command channel 0x0011 then input channels 0x0013. Authentication with the imported keys
remains mandatory. The decoder specific to Nova does not need a
downloaded descriptor to recognise the reports of Nintendo Joy-Con.

The change avoids the SDP preliminary. It does not prove that this preliminary is
the only cause of the blocker on your console. The session report makes it possible to
locate a possible failure on the new path.

## Install and try

1. Keep a copy of your current bootloader folder.
2. Copy the bootloader folder supplied to the SD card, accepting the replacement
   of the files. Do not delete your configurations. Check **23** in the credits.
3. Open Nyx Settings → TV / Dock → Wireless Joy-Con.
4. Attach the Joy-Con already paired to this console and choose Read pairings.
5. Detach them, choose Enable Bluetooth, then press a button to wake them.
6. Check that HID goes to 1 and that Reports increases, then try the stick and A / ZL / ZR.
7. Choose **Stop Bluetooth** after the trial: the report is saved here:
   `bootloader/bluetooth/last-session.txt`.

If the commands still do not work, keep this file and the screenshot
of the displayed state. The report contains the steps and protocol codes; it does not
contain any Bluetooth addresses, pairing keys or raw packet content.
A message confirms whether the save succeeded. No report is written
automatically while another system is being launched or during a diagnostic.

The payload supplied is `hekate_nova_pro_6.5.5_v23.bin`. The Hekate payload remains
functionally identical to that of v22; the modifications are in Nyx.
Copying only the payload without replacing the bootloader folder does not
bring the service up to date. The files nyx.ini and hekate_ipl.ini are not supplied.

## Connection changes

- Direct and authenticated opening of the HID channels, without a mandatory SDP request.
- Same path without SDP when the controller initiates the connection.
- Command to switch to full mode sent in a report of full size.
- Closing of the command channel if the opening of the input channel fails,
  in order to release the connection before a new attempt.
- Explicit initial state: "Radio ready: Joy-Con connection pending".
  "Joy-Con commands active" appears only after a recent decoded report.
- Addition of the PSM requested channel and of the STEP stage to the displayed information.
- Session report bounded to 128 metadata entries, exported with Stop Bluetooth.

The v22 fixes for navigation, the Nyx header and HOS 23.5 support are
preserved. The service remains disabled at boot and the keys remain in RAM.
The Joy-Con pairings are not rewritten and no proprietary Bluetooth
firmware is supplied. The import via rails does not concern the Switch Lite.

## Reading the report

PSM 0011 corresponds to the command channel; 0013 to the input channel. ERR 69 indicates
a timeout; another code can signal an authentication or opening error that is
distinct. STEP distinguishes the connection attempt, the link, the authentication,
the channels and the reception of reports. The detail of the trace serves to analyse the trial.

Records of type 1 keep only the opcode of an HCI command and its
length, never its parameters. Types 2/3: end or status of a command; 4: link
event; 5/6: L2CAP signalling sent/received; 7: HID transmission; 10: HID opening
result; 11: first report. The periodic button reports do not
fill the trace: only their counters and the first event are kept.

## Checks performed

A virtual peer exchanges real H4 frames with the BTstack stack of Nova.
Both controllers are authenticated with their keys, the command/input channels
are configured, the full command is received, then the ACK, the buttons and the
sticks go through the whole stack. A disconnection releases the commands concerned.
This test does not directly fabricate an "HID open" event for the service.

Three negative scenarios are also covered: SDP search without response giving
69 on the old path, command channel without response giving 69 with PSM0011, and
input channel without response leading to the closing of the command channel.
The virtual peer does not demonstrate the radio behaviour of the real Joy-Con.

The export is checked with card, folder, opening, short write
and closing errors; a failure must not be announced as a saved report.
The interface and navigation checks of Nova remain executed on PC.

## Sources and licences

BTstack keeps its personal and non-commercial BlueKitchen licence, attached
to the package. The file classic/hid_host.c includes a small Nova adaptation
conditioned by NOVA_HID_KNOWN_PSMS; its other paths remain those of upstream.
The modification and the original revision are described in BTSTACK-UPSTREAM.txt.
The other credits and licences are preserved. Read also LIRE-MOI-V22.md.
