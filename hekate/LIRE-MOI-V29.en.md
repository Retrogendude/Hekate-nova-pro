# Hekate Nova Pro 29 — pairings, SD backup and reconnection
**Languages:** [Français](LIRE-MOI-V29.md) · **English** · [Español](LIRE-MOI-V29.es.md) · [Русский](LIRE-MOI-V29.ru.md)

Formatting the SD card is not a fix for the observed problem.
The pairing data are read in the Joy-Con. The v28 received both
controllers on one OLED, while the R lost its connection on the other.
Going through Nintendo restored the import 2/2; a new trial then
reproduced 0/2 on the second OLED. The exact cause on hardware remains to be verified.

This version accepts the two host profiles already validated by the BDK reader:
Nintendo and standard. The v28 required the Nintendo classification even for a
standard record recognised as valid. The new path keeps the checksum
check, the active entry, the non-zero keys, the sides and the
compatibility of the two hosts. Truncated SPI responses no longer fill
the cache. The historical Hekate export keeps its Nintendo classification.

The timeout of a connection attempt goes from 15.36 s to 5.12 s. The attempts
continue, spaced out, as long as the Bluetooth service is enabled; they no longer
stop at the third failure. Only one outgoing attempt remains active
at a time. A re-established connection resets the retry counter to zero.
This improves recovery after wake-up or a cut; wake-up by a normal button
and the absence of cuts on both consoles still have to be tested.
The radio rate, the encryption, the channel management and the buttons are preserved.

Two new actions appear in the Wireless Joy-Con menu:

- **Save the pairings**: after a valid reading, Bluetooth stopped.
  A 72-byte file contains the data needed for both controllers,
  in `bootloader/bluetooth/joycon-pairing.bin`. It is not a complete
  copy of the Joy-Con memory. The keys are saved in clear text:
  keep this file private and do not send it with the reports.
- **Load the pairings**: explicitly reloads this file into the
  session after a restart. A format or checksum error makes it
  rejected, keeping the keys already present in memory.

The backup uses a temporary file then keeps the previous version
in `joycon-pairing.bak`. The loading tries this copy if the main file
is absent or damaged. The `.bak` file also contains private keys.
A backup with a single valid controller remains a 1/2 backup.
"Erase the keys" erases the session in memory, not the backup files.
The files are to be used with the associated controllers. To compare the
two consoles, read and save the pairings corresponding to each trial.
If a new pairing is made in the Nintendo system, read again then
save the updated data before reusing the SD backup.

Installation and check:

1. Extract the ZIP to the root of the SD card, replacing `bootloader`, without formatting.
   Use `hekate_nova_pro_6.5.5_v29.bin` if necessary, then restart.
2. With the Joy-Con attached, read the pairings. If 2/2, save them before
   enabling Bluetooth. If 0/2 persists, stop and retrieve the report.
3. Detach the controllers, enable Bluetooth and first try a normal button
   of each controller to wake them. Test the movement and A/B.
4. Stop Bluetooth and immediately retrieve `bootloader/bluetooth/last-session.txt`,
   without erasing the keys or reading the pairings again. Do not attach the pairing
   files `.bin` or `.bak`.
5. To check persistence: restart Nova, load the pairings from
   the SD card, then redo the test. No reading of the rails is necessary for
   this loading if the backup is valid.

The report adds `recognized` (valid profile recognised) and `source` (1 rails,
2 SD). During an SD loading, the Nintendo classification is not filled in;
the rails indicators do not represent a new physical reading.
The trace type 20 indicates the requested attempt timeout, in Bluetooth units.
No key, address or raw data is added to the report.

Build and checks on PC do not replace the hardware validation.
The recovery after timeout is tested with a simulated radio; it does not
prove that the cause of the R cuts has disappeared. No erasure or
new pairing is sent to the Joy-Con. Base: Hekate 6.5.5 / Nyx 1.9.5.
The user settings are not included in the ZIP. Licences preserved.
