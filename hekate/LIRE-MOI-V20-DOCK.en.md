# Hekate Nova Pro 20-Dock
**Languages:** [Français](LIRE-MOI-V20-DOCK.md) · **English** · [Español](LIRE-MOI-V20-DOCK.es.md) · [Русский](LIRE-MOI-V20-DOCK.ru.md)

This version adds automatic dock detection to the TV functions of
Nova19-TV. The reference connection is a Switch OLED inserted directly
into an official Nintendo dock, with power and HDMI connected.
The automatic activation of this version still has to be tried on the console.

## Install

Back up your `bootloader` folder, or use a test SD card. Copy the
`bootloader` folder supplied to the root, then launch
`hekate_nova_pro_6.5.4_v20_dock.bin`. Check **20-Dock** in the credits.
The package keeps your personal files `nyx.ini` and `hekate_ipl.ini`.
Nyx is in `bootloader/sys/nyx.bin`: replacing only the payload is not enough.

## Automatic activation: ON / OFF

In **Nyx Settings → TV / Dock**, enable **Automatic dock detection**,
then use **Save** to keep your choice after a restart.

- **ON**: when Nyx starts already on the dock, or when you then place the
  console on it, the TV connection starts automatically after a short
  presence check. The TV must be switched on and on the correct HDMI input.
- **OFF**: no automatic detection in the background. The **Enable TV**
  button remains available. Disabling the automation does not cut a TV
  connection already started: use **Stop TV** or **VOL-** for that.

The new option is disabled by default in order to preserve the behaviour
of previous configurations. Once saved, it is read again at each
start of Nyx. It does not modify the TV functions of an already started system.

The detection waits about two seconds after the initialization of the interface,
then confirms the presence of the dock on two spaced readings. The negotiation
and the initialization of the video then take their own time.

## Console screen

The **TV display** setting is also in the same tab:

- **TV + console** keeps both screens; this is the initial setting.
- **TV only** turns off the console screen after the successful initialization of the TV.
  It comes back on when the TV output is stopped or disconnected.

Use **Save** to keep this choice. The brightness set in Nyx
is kept for the return to the console screen. It is not overwritten
by switching to TV.

## Stopping, removing and new attempts

**VOL-** or **Stop TV** cuts the output. The automation does not turn it back on
immediately: remove the console then put it back on the dock to re-arm the
detection, or use **Enable TV** manually.

An automatic attempt that fails is not repeated in a loop. If the TV or
the HDMI was not ready, check the connection then restart manually or
remove/put back the console. A controller read error is not
interpreted as a disconnection.

The detector recognises the usual power supply offer of Nintendo docks,
then the driver performs the full negotiation. Other chargers/adapters
are not the reference connection; manual activation remains
available. The Switch Lite is not supported for TV output.

Hardware diagnostics and exclusive USB functions suspend the TV
service. Their return restores the automatic monitoring; a TV output that was
active can be re-enabled. The automation respects a previous manual stop.
A system launch, a payload launch, a shutdown or a restart ends the service.

## Reports and validation

The reports are in `bootloader/tv-test/last-run.txt` and `run-NNN.txt`.
For the trial on the console, check:

1. ON saved, restart of Nyx with the console on the dock;
2. starting outside the dock then placing on the dock;
3. stop with VOL-, no immediate restart, then removal/reconnection;
4. return to the console screen and preservation of the choices after a restart.

The build and the PC tests check the code, the preferences and the error
cases. They do not replace these physical trials. The video driver and its
settings are kept from prototype01 tested in a direct insertion.
HDMI audio output and intermediate cables are not validated here.

## Wireless Joy-Con

They are not enabled in this version. The current code tests the radio,
but the Bluetooth service for connection and for controller commands is missing.
See `documentation/ETUDE-JOYCONS-SANS-FIL.md` for the analysis and the references.
Joy-Con on the rails keep their existing support.

The Nova19-TV and prototype01 archives are kept to go back to the
previous version. See `documentation/TV-SOURCES.md` for the provenance.
