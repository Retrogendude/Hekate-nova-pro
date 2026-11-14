# Audit Nova 30 — wireless inputs, smoothness and budgets
**Languages:** [Français](AUDIT-NOVA30-WIRELESS.md) · **English** · [Español](AUDIT-NOVA30-WIRELESS.es.md) · [Русский](AUDIT-NOVA30-WIRELESS.ru.md)

Local audit of 13 November 2026. The physical trials of v29 are declared conclusive
by the user. The results below come from the sources and from native tests with simulated
radio/clock. **No console or RF link latency has been measured.** The automatic
recovery at boot is handled separately by the parent.

The source paths are relative to `work/hekate-nova-pro-v30/`.

## Results and modifications

### 1. Brief presses: reproducible software defect, bounded fix

Before the fix, each report replaced `pad.buttons`. If two reports press then release
arrived before the next LVGL reading, the click disappeared. The `buttons_seen` counter
kept a proof of the press, without giving it back at the entry of the interface. The nominal
period of LVGL reading is 33 ms (`bdk/libs/lv_conf.h:115`); it does not guarantee a reading every
33 ms if another task takes time.

Fix in `nyx/nyx_gui/bluetooth/nova_bt_protocol.c:7`: the rising edges are kept in
`pending_buttons`, with an independent timestamp per button. The simple reports `0x3f` and
full `0x30/0x21` use the same mechanism (`:78`, `:83`). The held state, the sticks, the battery and the report
counter remain those of the last report.

The API `nova_bt_take_buttons(side)` (`nova_bt_stack.c:299`) returns the buttons currently held plus
the edges of less than 100 ms, then consumes the edges. The following reading therefore sees the
release if the last report is released. Each button expires separately: a recent edge
does not rejuvenate an old edge. The ages use unsigned subtraction to cross the clock
wraparound. The edges are removed by `clear_pad` (`:156`), therefore by HCI disconnection,
HID closing, stale at 500 ms and Stop. A stale reading also consumes them without giving them back.

Integration required on the parent side: in each iteration of both sides of
`gui_bluetooth_merge`, call `uint32_t buttons=nova_bt_take_buttons(i);` before the freshness/rail priority
check, then use `buttons` instead of `p->buttons` during the merge. The rail side must also consume
the edges without applying them, so as not to replay a press when the source changes. A single
call per side and per interface reading. The present sub-work does not modify `gui_bluetooth.c`
nor `gui.c`.

Cost: 100 fixed bytes per Joy-Con (mask and 24 dates), without allocation. No change to the keys, to
the BR/sniff/QoS profile, to the page timeout, to the serialisation of the connections or to the retries.

Limits: several presses of the same button between two readings become a single edge;
this fix is not a click queue. A report left pending in the controller is dated at its reception by the
host: the physical age of the press cannot be deduced from this date.

### 2. RX budget and total cost: make the observation correct

The service remains cooperative. The loop calls the normal polling before `lv_task_handler`
(`frontend/gui.c`, GUI loop), while each display rectangle calls the fast variant
(`frontend/gui.c:334`). The high-priority Bluetooth task is created with a period of 2 ms and refreshes the
statistics only every 500 ms (`frontend/gui_bluetooth.c`, `tick` and `start`). The priority does not
preempt a task already in progress: `bdk/libs/lvgl/lv_misc/lv_task.c:80` organises cooperative
calls.

Budgets preserved: `nova_bt_poll()` = RX up to 8 000 µs, silence of 2 000 µs after
reception; `nova_bt_poll_fast()` = RX up to 250 µs without silence (`nova_bt_stack.c:422`). An empty line
returns immediately. These numbers bound an RX slice, not the radio time nor the button-to-screen
delay. A packet callback cannot be interrupted during processing.

Before the fix, the budget test used a date taken before `input_byte`, therefore before the
callbacks of the current packet. The same clock reading is now taken after the processing
(`nova_bt_stack.c:363`). This makes the callback count in the budget without adding a clock
reading per byte. `rx_budget_hits` counts the slices that reach their ceiling (`:371`).

The old `rx_max_us` remains a maximum of the RX phase. It did not include the TX, the loop
callbacks, the timers, the profile, the connections and the error closing. The new fields
(`nova_bt.h:32`) measure:

| Field | Meaning |
| --- | --- |
| `poll_max_us` | Maximum of a complete call, error closing included |
| `poll_fast_max_us` | Complete maximum of the fast variant |
| `poll_normal_max_us` | Complete maximum of the normal variant |
| `poll_gap_max_us` | Largest interval between starts of two calls to the service |
| `rx_budget_hits` | Number of RX slices reaching their ceiling |

The instrumentation adds one clock reading per call, after all the phases (`nova_bt_stack.c:415`),
and no wait. It does not claim to provide a percentile nor an interval between LVGL readings.
These UI measurements must be added if needed before attributing the jerks to the radio. The counters
must be exposed in the SD report by the parent.

### 3. UART rate and RTS: limited margin, physical settings to preserve

The radio uses 115 200 baud, 8 bits and one stop bit (`nova_bt_radio.c:67`), the RTS/CTS hardware flow
control (`:80`) and the RX threshold of 8 bytes taken from the HWTest profile (`:18`). The Nova path
reads the FIFO directly and immediately returns `-1` if it is empty (`:101`); it does not use the
generic `uart_recv` of the BDK, which waits up to 250 µs after each byte (`bdk/soc/uart.c:98`). Replacing this
path with the BDK helper would reintroduce a wait unsuited to the redrawing.

At 115 200 baud in 8N1, the theoretical capacity is 11 520 bytes/s. The wire time of an N-byte frame
is `N × 10 / 115200` seconds: 59 bytes occupy 5.12 ms; 63 bytes occupy 5.47 ms. These sizes
illustrate the calculation, they do not constitute a measurement of the console's reports. As a
scenario, 110 frames/s of 59 bytes use about 56 % of the gross rate, before HCI events, signals and RTS
pauses. The stream can therefore bring several reports closer together and occupy the 8 ms RX slice,
while remaining below the nominal rate.

The RTS pauses protect the FIFO during the graphics tasks but can increase the delay of the pending
reports. The HWTest comment reports a UART recovery greater than 300 µs (`nova_bt_stack.c:356`); this is a
result prior to the present audit. The existing tests simulate recovery holes of 700 µs. **Do not
arbitrarily lower 8 ms / 2 ms, remove RTS/CTS, change the FIFO threshold or increase the baudrate in this
iteration.** First compare `poll_gap_max_us`, `rx_budget_hits`, UART errors and report regularity on the
console.

### 4. Allocations and statistics: secondary testable optimization

The BTstack objects are in fixed pools: 2 HCI connections, 8 L2CAP channels and 2 HID
connections (`bluetooth/btstack_config.h:9`, `libs/btstack/btstack_memory.c:140`, `:257`, `:844`). No malloc has
been added to the reception path or to the latch.

The statistics panel formats a text containing counters and positions every 500 ms then calls
`lv_label_set_text` and adjusts the size. The identical guard of `bdk/libs/lvgl/lv_objx/lv_label.c:150` avoids
the work if the text does not change, but the counters change during the streaming. The modified
path frees/allocates and recalculates the label (`:169` to `:183`). This constitutes a source of occasional
redraws, without proof that it is perceptible on console. Future testable optimization: freeze the
advanced details outside their panel, keep the useful information visible and compare the frame durations
with the panel open/closed. The explicit Stop writes the SD report; this one-off cost must not
be confused with that of the streaming.

## Native verification

`python work/check_nova30_wireless.py` passes against the real v30 sources, with radio and clock
substituted. The script also compiles the real v29 sources to demonstrate the two targeted
failures:

- v29 loses the press at 1 000 ms / release at 1 017 ms before a reading at 1 033 ms; v30 returns it once then
  returns the release.
- v29 does not count a loop callback of 3 000 µs in `rx_max_us`; v30 counts it in the maximum of the complete poll.
- v30 checks the held state, the independent expiration of two buttons, the stale, a real HCI
  disconnection event, the periodic stale cleanup, Stop/restart, simple/full reports, the left/right side and the
  clock wraparound.
- v30 checks exactly four clock readings in an instrumented empty poll, without added wait; a simulated
  packet callback of 600 µs does reach the fast ceiling of 250 µs, which also shows that a callback is not
  preemptible.
- The existing native H4/BTstack suite is reused and passes: real initialization of 57 HCI commands,
  parsing, keys, input, recovery and deadlines. Only its cost assertions with a virtual clock are adjusted from
  270 to 280 µs (and 27 000 to 28 000 for one hundred calls), that is one additional virtual clock
  reading per call.

The existing `input` tests using a merge extraction and a fake status will have to declare a stub
`nova_bt_take_buttons` when the parent integrates the API. To test the latch end to end, connect the real
implementation rather than a stub that only returns the current buttons. The physical
negotiation/radio suites remain necessary after integration; this report does not announce their
execution on a console.

Integrated validation by the parent: consumption in the merge before rail priority, brief button
tests in this merge, maxima exposed to the SD report and generation of nova30-wireless-check.json. The physical radio
budgets remain preserved.
