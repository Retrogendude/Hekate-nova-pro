# Nova25 — reconnection policy and retained diagnostic evidence

## Physical evidence supplied for Nova24

The user's new session progresses beyond the previous authentication failure:

- Read_BD_ADDR matches the imported host (HOST=1).
- Incoming connection accepted around 5183ms, success at5367ms.
- Authentication Complete at5494ms has status00 for handle000E.
- Set_Connection_Encryption0413 is accepted at5513ms.
- Encryption Change at10516ms has status08, followed by disconnection08.
- No ACL data reaches either peer. No HID channel or input report is established.

This confirms authentication for one link, not for both Joy-Con, and does not
establish that the cipher itself is broken. Status08 is Connection Timeout:
[Bluetooth Core controller error codes](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-61/out/en/architecture,-change-history,-and-conventions/controller-error-codes.html).
The header state0 and all-zero pad fields result from Stop clearing runtime
status while retaining the trace. They do not invalidate the successful
authentication event in that trace.

## Bounded changes

The prior Nova code left BTstack's default incoming role preference at1
(peripheral), and initiated both outgoing connections in the same poll cycle.
Nova25 asks for central on incoming reconnects and permits only one outstanding
outgoing HID setup until it either opens or fails. Incoming requests are still
handled; the second outgoing attempt can proceed after the first result.

Requesting central is the HID host policy explicitly used by
[Bluepad32 uni_bt_bredr.c](https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/bt/uni_bt_bredr.c)
in `uni_bt_bredr_setup` (`hci_set_master_slave_policy(HCI_ROLE_MASTER)`).
BTstack's local `hci.c` initialises that preference to1. Its public setter is
used without modifying vendor code. This is a compatibility hypothesis for the
physical failure; neither this source comparison nor a virtual peer proves it
will resolve encryption on the actual controller.

Security level2, existing keys/type4, v24 key byte order, host readback check,
SSP/SC policy and rejection of new pairing remain unchanged. No firmware blob
is loaded or distributed. The report now includes the non-unique controller
version/revision/manufacturer/subversion for further investigation if needed.

Stop releases all input state and radio resources but retains counters and last
authentication/encryption/disconnection evidence until the next Start. A new
Start clears the session. The report distinguishes seen/absent security events.
ROLE is the role known by BTstack, updated by Role Change notifications; the
requested accept policy is logged separately (trace14). No address, key or
packet payload is recorded. HCI handles and status codes are session metadata.

## Test scope

The real BTstack code receives virtual HCI packets, authenticates with the
synthetic SPI/import fixture, and negotiates real L2CAP/HID messages. Added
cases exercise incoming reconnects with SSP/SC-capable feature pages, an
outgoing/incoming collision (status0B), a stalled first setup, and the exact
auth00 -> encryption08 -> disconnect08 sequence. The failure evidence survives
repeated Stop without leaving active input. Previous v24 code is compiled
against the same test ABI and fails the precise accept-role and concurrent
paging assertions. This verifies the software changes; it does not reproduce
or explain the physical controller's internal cause of the timeout.
