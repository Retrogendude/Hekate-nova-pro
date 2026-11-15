# Joy-Con authentication: byte-order correction
**Languages:** **Français** · [English](AUTH-SOURCES-V24.en.md) · [Español](AUTH-SOURCES-V24.es.md) · [Русский](AUTH-SOURCES-V24.ru.md)

The v23 physical report shows authentication status 0x05 after a successful
Link_Key_Request_Reply command, and zero ACL traffic. It does not show an input
mapping failure. The other side times out with 0x08; that alone cannot identify
its root cause.

The old Nova importer reversed `jc_bt_conn_t.ltk` a second time. This was based
on an incorrect assumption about the cached key representation. The BDK already
reverses the raw SPI field when constructing that cache. Nova24 copies the
cached value, just as the established Hekate export/consumer chain does:

1. [Hekate BDK](https://github.com/CTCaer/hekate/blob/master/bdk/input/joycon.c)
   prepares the cache. Hekate's `gui_options.c` exports that cached sequence as
   the `ltk` hex string in `joycon_mac.ini`.
2. [Switchroot init](https://gitlab.com/switchroot/kernel/l4t-initramfs/-/blob/master/init)
   reads that string and writes it directly into BlueZ's `[LinkKey] Key` field,
   using Type=4. It takes the host identity from the same record.
3. [BlueZ adapter.c](https://github.com/bluez/bluez/blob/master/src/adapter.c)
   `get_key_info` / `str2buf` decodes consecutive hex pairs, with no reversal;
   `load_link_keys` copies them to the management request.
4. Linux [mgmt.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/mgmt.c)
   and [hci_core.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/hci_core.c)
   load and retain the bytes; [hci_event.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/hci_event.c)
   `hci_link_key_request_evt` copies them into the HCI reply without reversing.
5. The vendored BTstack `hci_cmd.c` `P` format copies a 16-byte key unchanged.
   Its key database uses the same wire representation. Type=4 is retained.

Sources were inspected on 2026-11-13. Reference snapshot hashes are included
in the verification record; no console keys or addresses are collected.

The new native regression executes the actual BDK SPI parser, actual Nova GUI
import and actual BTstack HCI/L2CAP/HID negotiation. Invented non-palindromic
SPI keys are compared against independent expected wire bytes. The v23 importer
fails that check. Deliberately reversing the key in the virtual peer reproduces
authentication 0x05 with ACL 0/0. Normal and SSP/SC-capable controller profiles
are exercised. These are software tests, not proof of a working physical link.

Nova24 also verifies Read_BD_ADDR against the imported host before allowing
connections. Only match/mismatch is logged. Error 9 means identity verification
failed. AUTH records authentication separately from subsequent HID errors.
No pairing is created, no keys are written, and L2CAP security level 2 remains.
