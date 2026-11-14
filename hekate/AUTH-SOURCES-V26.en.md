# Nova26 — crossed HID channel establishment
**Languages:** [Français](AUTH-SOURCES-V26.md) · **English** · [Español](AUTH-SOURCES-V26.es.md) · [Русский](AUTH-SOURCES-V26.ru.md)

The new physical Nova25 report changes the diagnosis. HCI 85/46, ACL 5/6,
HOST 1, controller HCI 08/revision 2000/manufacturer 000F/subversion 2409. Both pad
records contain successful authentication and encryption events. ROLE 00 appears
in received Role Change events. The prior auth 05/encryption 08 failures are not
the failing stage shown in this session. ENABLED 0 in an already stopped session
does not reverse the successful encryption events in the retained trace.

For handle 000D, around 1949–1959ms, Nova transmits a control PSM 0011 connection
request while receiving the peer's own PSM 0011 request. Nova replies with result
0004 (resources refusal), and the peer replies pending 0001 to the outbound
request. The peer then disconnects with reason 13. The same crossed request and
local refusal appear for handle 000B around 2394–2399ms. No HID report is received.

The vendored HID host rejected every incoming control request when a HID
context already existed for that address, including an outgoing context still
waiting for control establishment. This condition is also visible in the
[primary upstream HID host source](https://github.com/bluekitchen/btstack/blob/master/src/classic/hid_host.c).
It explains the local refusal; it does not prove this was the only obstacle on
the console, or that radio input will now work without a physical test.

Nova26 changes only the optional NOVA_HID_KNOWN_PSMS report-mode path:

- A crossed incoming control request takes over the existing pending HID
  context, retaining its HID CID and going through the normal application
  acceptance callback and L2CAP security checks.
- An incoming interrupt request can similarly supersede an outgoing interrupt
  open still awaiting completion. Duplicate incoming and already established
  channels remain protected from replacement.
- CHANNEL_OPENED events are matched by current L2CAP CID and peer, rather than
  peer address alone. Late success on a noncanonical CID is disconnected; a
  late failure cannot destroy the active incoming HID context.
- Superseded pending entries remain under L2CAP's normal response/RTX cleanup.
  Two peers need six dynamic entries at the tested overlap, versus the fixed
  pool of eight. Tests verify that this returns to four established channels
  after late failure, success or expiration, and zero after Stop. Duplicate
  incoming requests are declined and their temporary entries freed.

No L2CAP, HCI, radio driver, key import or input-decoder change is made. The
v24 key format and v25 role preference, serial attempts and retained session
metadata remain. Level 2 security is retained, no new pairing is accepted and
no firmware is loaded. Generic vendor behavior remains when the macro is off.
The separate BTstack license is retained; the complete Nova vendor patch is
included in the bundle.

The native test now sends actual incoming L2CAP requests while outgoing opens
are pending, with independently generated local/remote CIDs. The actual v25
HID source produces result 0004 and fails the non-refusal assertion. Nova26
accepts both channels and receives ACK, buttons and sticks through real BTstack.
Additional tests send duplicate requests and late success/refusal/RTX timeout,
check channel counts, and confirm that the active session remains usable.
The virtual controller version mirrors the new reported HCI version/revision.
These tests substitute the radio and do not establish physical functionality.
