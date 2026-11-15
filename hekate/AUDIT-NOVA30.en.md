# Hekate Nova Pro — global audit and v30 optimizations
**Languages:** [Français](AUDIT-NOVA30.md) · **English** · [Español](AUDIT-NOVA30.es.md) · [Русский](AUDIT-NOVA30.ru.md)

Review of the rendering, scheduling, wireless commands, inputs,
boot, SD loadings, memory, diagnostics and TV mode sources. The tests
use the real functions with peripherals substituted on PC. No v30 display
framerate or physical latency result is announced before a trial on console.

## Integrated changes

| Area | Finding | Fix and proof |
| --- | --- | --- |
| Boot | An empty session forced the keys to be loaded by hand | After initialization of the driver, physical detection of the rails. If no Joy-Con: loading of the validated `.bin`, then start in the background without a window. Tests for absence/1 rail/2 rails, SD error, connection race and manual stop. |
| Reflection | The 32-zone queue could overflow before the refresh and force a whole screen | Pre-calculation of the global budget, deferred transitions and real-time phase. 9 scenarios fixed without periodic overflow; rendering of three phases identical to v29. |
| Brief presses | Press and release received between two UI readings could cancel each other out | Single presentation of the recent edges, 100 ms expiry per button, no added wait; cleanup on cut/Stop and priority to the rails. The real v29 fails the targeted test. |
| Radio service | The duration counter covered only the RX part | New maxima of full call, of fast/normal variant and of interval between calls. The processing of the current packet counts in the RX budget. |
| Long SD/USB tools | Some processing does not maintain the complete GUI loop | The manual maintenance point calls the fast polling, outside diagnostics that reserve the hardware. Test of the polling and of the suspension. |
| Resources | `res.pak` could exceed its region or be partially read | Bounds before reading, minimum size of the referenced resources, count actually read and closing checked. |
| Payloads | A refused file could remain allocated; size checked after allocation | Verification before allocation and common cleanup; incomplete or failed readings refused. 20 I/O cases pass, 8 old defects reproduced with the real v29 code. |

## Rendering measurements on PC

With 33 ms between refreshes over a simulated 8.48 s cycle, the surfaces
redrawn decrease from 52.1 % on the home screen, 73.7 % in Tools and 53.4 % in the
carousel. The full refreshes caused by the reflection go
respectively from 5/5/3 to 0 in these scenes. These are surfaces, not FPS gains.
The tests also delay the refresh to 66 and 100 ms and preserve the
invalidations of the other widgets when the queue is already full.

## Points preserved

BR/sniff/QoS radio profile, UART rate, flow control, key policy,
72-byte backup file, choice of language/font/colours and Nova style
remain compatible. The loader and the system modules do not receive the
GUI optimizations. The preparation of the distribution checks the real sizes and
headers in order to refuse an incompatible binary.

The service is cooperative: it works on the home screen and the ordinary menus
without an open Bluetooth panel. The hardware probes that reserve their
peripherals keep their suspension of the commands; the audit does not re-enable
the tasks during the measurement windows.

## Recommended follow-up

| Priority | Identified lead | Remaining work |
| --- | --- | --- |
| Reliability | Common SD helper able to return success after an incomplete write | Fix its contract and test all the calling exports/configurations. The dedicated Bluetooth backup already checks these errors. |
| Reliability | Reading of Nyx by the bootloader insufficiently bounded | Add a local validation compatible with the small IRAM margin; keep the fixed header and the payload limits. |
| Smoothness | PNG decoding and font scans during the reconstructions | Measure then introduce bounded caches with invalidation of the replaced files. |
| Boot | Animation delays independent of the work performed | Overlay animation and readings without changing the electrical delays, the memory training and the screen ramp. |
| Diagnostics | Long phases without intermediate state | Add a limited refresh between safe phases, without disturbing the measurements. |
| TV | Link negotiation and presentation sometimes blocking | Split into timed states after measurements on the dock. |

These points come from identified code paths. They are not described
as observed hardware failures and are not all modified in
v30. The details and references are in the three specialised reports.

## Verification on console

Keep a valid pairing backup. Test a start without rails,
with only L, with only R and with both Joy-Con attached. Without rails,
check the navigation from the home screen and in the windows without opening
Bluetooth. A manual stop must stay stopped until a manual action or
a new boot. Compare the same paths and send only the
`last-session.txt` report, never the files containing the keys.

Details: [rendering](AUDIT-NOVA30-GUI.md), [Bluetooth](AUDIT-NOVA30-WIRELESS.md), [core and loadings](AUDIT-NOVA30-CORE.md).
