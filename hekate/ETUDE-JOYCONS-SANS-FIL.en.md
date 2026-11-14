> v20 history: this study precedes the 21-BT prototype. For the functions now integrated and their limits, see LIRE-MOI-V21-BT.md.

# Joy-Con wireless directly in Hekate: state and development lead
**Languages:** [Français](ETUDE-JOYCONS-SANS-FIL.md) · **English** · [Español](ETUDE-JOYCONS-SANS-FIL.es.md) · [Русский](ETUDE-JOYCONS-SANS-FIL.ru.md)

The feature is conceivable, but it is not available in this
delivery. The work required goes beyond adding a Bluetooth switch.
No dummy wireless connection button has been added.

## What the project already contains

- The `bdk/input/joycon.c` driver uses UART B/C for the Joy-Con on the
  rails. The "HCI" messages sent to the controllers do not constitute a
  Bluetooth stack working on the console.
- The Nyx settings know how to export Joy-Con pairing information.
  This export is notably used by other systems and does not enable wireless
  in Nyx. [Hekate source](https://github.com/CTCaer/hekate/blob/master/bdk/input/joycon.c).
- The local hardware test `diagnostics/vendor/probe_bt.c` knows how to check that the
  BCM/CYW4356 radio responds to HCI commands over UART-D. It performs a
  hardware preparation then a diagnostic, with restoration by the test
  framework. It handles neither controller connection, nor continuous HID reception.
  This test must not become a repeated task for driving the Joy-Con.

## What still has to be developed and validated

1. Start the radio once for a session, process its eventual initialisation
   firmware, then receive the Bluetooth events without
   blocking the menu. Verify in parallel the coexistence with the TV display.
2. Manage the Bluetooth Classic connections, the HID channels, the pairing
   information and the reconnections. Preserve the existing associations and
   never place their keys in the diagnostic logs.
3. Decode the buttons and joysticks of both Joy-Con, apply their
   calibrations, then feed the existing Nyx controls. The Joy-Con
   Bluetooth reports are documented, which provides a basis for work.
   [dekuNukem technical notes](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md).
4. Check the disconnections, the return to the commands on the rails, the diagnostics,
   the available memory and the transition to a system or another payload.
   The stop of the service must release its resources before this transition.

One lead is [BTstack](https://github.com/bluekitchen/btstack), which offers
an embedded operation without an operating system, an H4 UART transport
and a HID host example. Its integration on the Switch remains to be done; it is not
enough to compile the library. See also its
[chipset documentation](https://github.com/bluekitchen/btstack/blob/master/chipset/README.md)
and its [licence terms](https://github.com/bluekitchen/btstack/blob/master/LICENSE),
to be examined before its adoption or any redistribution of the project.

## Conclusion for this version

The automatic dock detection is implemented independently of Bluetooth.
The Nyx control with detached Joy-Con remains a separate development.
The first proof to obtain would be a stable connection of one controller with
real reception of its buttons, before presenting the feature as usable.
A "HCI radio: PASS" result does not constitute this proof.
