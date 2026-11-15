# Current presentation — Nova Pro 11
**Languages:** [Français](DIAGNOSTIC-INTEGRATION.md) · **English** · [Español](DIAGNOSTIC-INTEGRATION.es.md) · [Русский](DIAGNOSTIC-INTEGRATION.ru.md)

The twelve categories use selection then opening on two presses. The probes and results are presented in paginated cards, with readable names and access to the reports. The 42 measurement functions, their protections and the isolated RAM service remain those of the integration described below. The RAM return and the payload limits are checked in the v11 verification; the physical trials remain necessary.

---
## Hardware integration and operation

# Hardware diagnostic — native integration, prototype 5

The 42 entries of the HWTest-RCM 0.15 table are integrated in Nyx: identity,
fuses and KFUSE; power, charging, battery and thermal; memory and
clocks; SD, eMMC, partitions, GPT, PRODINFO and game card reader;
Bluetooth, Wi-Fi and CPU complex; codec, clocks and audio outputs; screen
and backlight; touch, ambient light, Joy-Con and buttons; GPIO,
UART, restart cause and PMC registers; synthesis and cross-checks.

The Diagnostic tab offers a summary, ten categories and the isolated RAM service.
Each category allows a collective run or probe by probe. The results
distinguish OK, warning, defect, error, observation and not
executed. An absence of measurement or of prerequisite does not become a success.
The reports can be viewed by pages then exported to the SD card
under `bootloader/diagnostics/hwtest_XXXXXXXX.txt`, without re-running the measurements.
The capture is bounded to 256 KiB with an explicit marker in case of truncation.

## Adaptation to Nyx

The probes are present under `nyx/nyx_gui/diagnostics/vendor/`. The main program,
the interface and the on-screen/UART logging of the original payload
are not used: a native adapter captures the report and publishes the
progress in the Nyx windows. The command driver and the maintenance
tasks are suspended during the measurements then restored. The data,
GPIO/pad states, fan, audio, PCIe, eMMC partition and key slots
modified are restored when their access is available. The reactivation of the
touch uses the path without persistent auto-calibration.

The Wi-Fi really includes the required AArch64 CPU0 program: the build fails
if this program is not available. It reserves A0000000..A003FFFF, outside
the Nyx heap and the screen images. The diagnostic messages are never
sent on the UART-B used by the right Joy-Con.

## Prerequisites and interpretation

Some measurements depend on the model or on components present. The decrypted
PRODINFO readings require `switch/prod.keys` of this console and readable/writable
SE slots; otherwise they are signalled as not
performed or with a warning, without overwriting the locked keys of Nyx.
The sound test plays the outputs successively and must be checked by listening.
A verdict of the audio transport does not prove that the speaker is audible.

## Separate RAM and return to Hekate

`bootloader/sys/hwtest_ram.bin` is a separate program: Nyx is exited in order
not to keep an interface in the memory subjected to the tests. The service
executes a single sequence of the seven advanced patterns on all the accessible
and unreserved zones, then reloads Hekate. The return forces the menu and
a new instance of Nyx displays the kept result, including after
cancellation or failure of the test. The first 4 MiB, including the return image and its
header, remain excluded. A corrupted return image is never executed;
a hardware failure or a blocking of the controller can prevent any return.

The size of both images is checked before allocation, and the short or failed
readings are rejected. The return image
comes from `bootloader/update.bin`, is recognised as Hekate, bounded to 128 KiB and
checked by fingerprint before use. The RAM service is bounded to 130392
bytes. The shared contract and its copy are verified identical.

## Validation

Hekate/Nyx ARM build, AArch64 Wi-Fi program and RAM service successful.
C tests on real functions: configuration backup, bounded readings,
capture/export and partial verdict, RAM return contract. The decompression of the
service is verified by the real C decoder and the binaries are compared to the
linked ELF. No trial on console nor physical validation of the measurements has yet
been carried out.

## Provenance

HWTest-RCM: user archive 0.15, commit
778e25b66a11c5cdd65c11a69ef8083aff1cd9f7, repository Creased/HWTest-RCM.
Hekate: user archive 6.5.4, commit
45cd81ed08e30a26ba380958c7e20b55c342834b. Atmosphère of the RAM service: commit
cb4b882e3b176480ac57a1161a85ff175c3f162c, exact dependency of HWTest.
The provenance comments, the Hekate licence and the Atmosphère licence
are preserved. The adaptations are described in diagnostics/PROVENANCE.md
and in the HWTest-RCM companion, ramtest/NYX-RETURN.md and DEPENDENCIES.json.
