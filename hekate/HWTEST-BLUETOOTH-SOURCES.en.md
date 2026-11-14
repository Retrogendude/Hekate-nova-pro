# Nova27 — adaptation of HWTest-RCM's Bluetooth GUI host
**Languages:** [Français](HWTEST-BLUETOOTH-SOURCES.md) · **English** · [Español](HWTEST-BLUETOOTH-SOURCES.es.md) · [Русский](HWTEST-BLUETOOTH-SOURCES.ru.md)

Reference: Creased/HWTest-RCM, commit
`74b66aab2352ae35912744fcd5441b07a0146307`, inspected on 2026-11-13.

- [GUI host implementation](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/gui/gui_bt.c)
- [Bluetooth documentation](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/docs/bluetooth.md)
- [Radio diagnostic](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/probe_bt.c)

The current `probe_bt.c` and `probe_wifi.c` match Nova's existing diagnostic
sources after normalising line endings. The previously overlooked GUI host is
the relevant additional reference. It supports one controller and performs
pairing. Nova keeps its two-peer BTstack service, imported keys and security
level2; its pairing procedure is not replaced.

The reference's hardware notes report lost EDR-sized inputs with the radio ROM,
sniff-related disconnections, and UART restart gaps exceeding 300us. Its code
selects basic-rate packets, allows role changes/sniff initially, requests
low-latency QoS and returns sniff links to active mode. The receive loop permits
2ms quiet with an 8ms call limit and an 8-byte FIFO watermark. Its startup enables
LDO1/LDO7 and the PMIC reference-clock outputs. These facts guide Nova's port.

Nova27 expresses this profile through existing BTstack APIs and a small pending
command scheduler. BR types are selected for outgoing calls and requested again
per established link, including incoming ones. Received packet-type and mode
events are recorded separately from command submission. QoS parameters follow
the reference. No proprietary firmware or new pairing is introduced.

Nova's receive pump additionally returns immediately on an initially empty line
and has a 2048-byte cap. The two-controller service retains the v26 crossed-HID
fix. Rail voltage settings are preserved; teardown restores only owned PMIC
control bits and the previous Wi-Fi reset level, with no PH0 writes.

New tests model the documented EDR input loss and timed UART pauses. The actual
v26 stack fails those regressions; v27 receives full reports, applies BR/QoS,
handles sniff recovery and reads across 700us pauses. The 8ms pump budget and
multi-command configuration metadata are checked. Platform-register tests cover
pre-enabled supplies/clocks, restoration and unrelated PMIC-bit changes.
These PC tests substitute hardware; neither they nor upstream's bench results
establish successful operation on the user's console.

The audit JSON records upstream file hashes and the comparison result. Upstream
GUI source is referenced rather than bundled as an unmodified second backend.
The existing BTstack and HWTest notices remain in the project.
