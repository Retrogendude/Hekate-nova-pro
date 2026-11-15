# Audit Nova Pro v30 — core, boot, storage and diagnostics
**Languages:** [Français](AUDIT-NOVA30-CORE.md) · **English** · [Español](AUDIT-NOVA30-CORE.es.md) · [Русский](AUDIT-NOVA30-CORE.ru.md)

Read-only static audit of 13 November 2026, on `work/hekate-nova-pro-v30`. No source file modified. The line references describe the version read during the audit; the parent works simultaneously on the boot. General GUI and Bluetooth are the subject of separate audits.

The v29 hardware results announced as conclusive are kept as supplied information, without claiming to reproduce them. No FPS, SD/USB throughput or hardware time gain has been measured here. The durations below come from the explicit delays and loops of the code.

## Priorities

| Priority | Subject | Demonstrable effect | Recommended scope |
|---|---|---|---|
| P1 | Sizes of the binary files and `res.pak` | Access outside the file or outside the reserved region on an invalid file | Validate before allocation/reading/execution |
| P1 | SD write reported successful despite failure | Incomplete exports presented as saved | Check written count and closing |
| P1 | Non-blocking build size limits | A binary exceeding capacity can be delivered | Make the build fail, without enlarging the loader |
| P2 | Leak of the refused payload in Nyx | Repeated consumption of the heap, no bounding | Size before allocation, single cleanup |
| P2 | 1.1 s of animation added to the boot | Software wait imposed, even when the work is already finished | Animation on the work, preserve appearance and hardware delays |
| P2 | Long diagnostics without periodic refresh | Impression of a freeze for several seconds | Display heartbeat only, paced |
| P2 | TV steps still blocking | GUI task held during a wait/negotiation | Timed states, progress and cancellation between safe phases |
| P3 | Translation allocations after report truncation | Work without a kept result | Avoid the translation when the write is already impossible |

P1 designates here a priority reliability, not an observed hardware incident. The common BDK changes have a larger regression surface than the local fixes; isolate them and check each caller.

## 1. Boot files: bound and check before use — P1

**Evidence.** `bootloader/main.c:585` reads `bootloader/sys/nyx.bin` with `sd_file_read(..., NULL)`. The reading of the version word at `main.c:596` assumes at least `NYX_VER_OFF + 4` bytes (`NYX_VER_OFF = 0x9C`, line 580). The loaded address then becomes an executed entry, lines 656–657. `bdk/storage/sd.c:258–272` allocates the whole size indicated by the file, without a business bound; `f_read` does not check the count read and `f_close` is ignored.

`nyx/nyx_gui/nyx.c:400–413` reads the whole size of `bootloader/sys/res.pak` directly at `NYX_RES_ADDR`. The reserved region is only `NYX_RES_SZ = SZ_16M` (`bdk/memory_map.h:82–83`). An excessive size writes beyond this region. An insufficient size can leave resources absent or old without this function failing.

**Testable fix.** Request the size of Nyx, reject an image that is too small and impose a bound consistent with its region/format before reading and before the jump. For `res.pak`, check the expected size or at minimum the bounds of the format, then the count actually read and the return of the closing. Do not consider a simple version comparison as a validation of the format. Use local helpers or modestly extend the bounded reading already present: `_read_service_payload` in `nyx.c:168–198` validates the size before allocation and checks complete reading and closing. Avoid duplicating heavy infrastructure in the loader close to its limit.

**Validation.** Files absent, empty, 1 byte, `0x9F`, valid size, exact limit, limit + 1; short read with `FR_OK` return, read/closing error; sentinels before/after the res region. The expected result is an explicit refusal before version access, jump or overflow. Then check a hardware boot with the valid v29/v30 files.

**Limit.** Audit of the size check and of the input contract, not cryptographic verification of Nyx nor of the complete format of `res.pak`.

## 2. `sd_save_to_file` returns a false success — P1

**Evidence.** After a successful open, `bdk/storage/sd.c:291–294` ignores `f_write`, the number of bytes written and `f_close`, then returns zero. Callers use this zero as success: dumps `nyx/nyx_gui/frontend/gui_info.c:84,108,138`, captures `frontend/gui.c:300`, extraction `frontend/gui_tools.c:1006,1152`, Joy-Con export `frontend/gui_options.c:758`, fatal log `bootloader/hos/secmon_exo.c:418`.

**Impact.** Card full, card removal or deferred error during the closing: a partial file can be announced as saved. This is not a speed gain; it avoids useless retries and makes the failures usable.

**Testable fix.** Collect `UINT written`, check `FRESULT` and `written == size`, always close an opened file, propagate the closing error when the write has succeeded. Choose a consistent existing error return for the partial write. Do not implicitly modify the overwrite or erase policies of the exports.

**Validation.** FatFS substitutions: complete write, partial + `FR_OK`, write error, close error, open refused; each path closes exactly once if necessary and only complete files closed successfully return zero. The native diagnostic already gives a good local example in `nyx_diagnostics.c:346–357`, which checks the count written and the closing.

## 3. Binary limits: make them blocking — P1

**Evidence.** `Makefile:108` and `112` only show a warning beyond respectively 140288 and 126296 bytes. The commands succeed despite an overrun. The parent currently announces 139273 raw bytes: declared margin of 1015 bytes, value not independently rebuilt during this audit.

**Testable fix.** Make the generation/delivery fail on overrun of both limits, keep their existing values, also check the size of the delivered artifacts. This guard consumes no byte of the payload. Evaluate the size after each BDK fix compiled into Hekate; prefer the local fixes to the bulky generic code.

**Validation.** Check limit − 1, limit, limit + 1: the first two cases pass, the last one really fails. Do an incremental then a clean build again. The loader already has the explicit dependency of `loader.o` on the two payload headers and on `Versions.inc` (`loader/Makefile:67–71`): keep it.

**To preserve.** `loader/loader.c:80–105` relocates then decompresses two embedded blocks of known compile-time size. No concrete benefit justifies here adding a cache, a new codec or additional rendering at this step. No decompression cost has been measured.

## 4. Leak and allocation before validation of a Nyx payload — P2

**Evidence.** `nyx/nyx_gui/nyx.c:133–145` loads a payload with `sd_file_read`, then refuses a size greater than `0x30000`. The `out` path (`nyx.c:164–168`) unmounts the card but does not free `buf`. The bootloader version, for its part, has `free(buf)` at `bootloader/main.c:213`. An oversized payload can thus consume its size on each new attempt in Nyx.

**Testable fix.** Check the size of the payload before allocation, keep a pointer initialised to `NULL` throughout the function and a single cleanup for each exit that returns to Nyx. A zero-byte image must be refused before launching. Take into account the exits even before `buf` has been declared in the current function.

**Validation.** One hundred successive refusals of the same payload > `0x30000`: heap occupancy stable and zero execution jump; success then simulated return; path without a file name; file absent and reading failed. The counter must distinguish live memory and memory only reserved by the heap.

**Heap context.** `bdk/mem/heap.c:39–112` searches linearly for a free block then extends the heap without a maximum bound; `calloc` multiplies without a guard (`heap.c:179–181`). The memory map nevertheless reserves 256 MiB (`memory_map.h:52–53`). The tests `if (!malloc(...))` therefore do not provide a reliable check of exhaustion. Prioritise the input sizes and the local leak. A global bounding of the heap would require a change of contract and the audit of the `zalloc`/`calloc` calls, which do `memset` without checking a null return (`heap.c:172–181`): this is not an isolated modification to be made in a hurry.

## 5. Boot: overlay the animation with the work — P2

**Evidence.** `nova_splash_animate` (`bdk/display/nova_splash.c:46–57`) performs all of its steps and waits even when `from == to`. It can precisely resume `from` at the progress already reached, line 48, while keeping the total duration. The normal path adds animations of 300 + 200 ms in `bootloader/main.c:590,655` then 300 + 300 ms in `nyx/nyx_gui/nyx.c:575,619`, that is 1100 ms of requested duration, excluding the cost of the rendering and the granularity of the delays.

**Testable fix.** Keep the splash, its colours and its steps but animate it during the readings/initializations. Finish immediately or with a brief bounded catch-up when the work of the step is really finished. The existing cap mechanism (`nova_splash.c:29–31`) avoids announcing a finished job too early: preserve it. The new `.bin` loading phases of the parent must use the same logic.

**Validation.** Measure separately the Nyx reading, the configuration, the resources and the first interactive image; fast and slow card; custom splash; Nyx entry after RAM return. A substitution of the timer can check that a step already finished no longer waits 300 ms. The 1100 ms are an identifiable software budget, not a guaranteed wall gain of 1100 ms on console.

**To preserve imperatively.** The SD power cycle wait (`main.c:652–653`), the backlight ramp, the Minerva training and `nova_splash_pause()` before the training (`main.c:645–647`). Do not speed up the electrical delays to gain apparent time. The timer hooks execute rendering from `get_tmr_ms/us` (`bdk/soc/timer.c:39–51`); do not extend them without a bound nor re-enable them during sensitive measurements.

## 6. Diagnostics: regular progress without disturbing the measurements — P2

**Evidence.** The adapter `nyx_diagnostics.c:132` defines `msleep_poll(ms)` as a simple `msleep(ms)`, contrary to the apparent meaning of the name. The display callback is called only by `status_set` / `status_step` (`nyx_diagnostics.c:133–143`) and at the entry of each probe (`:261`). The GUI callback refreshes immediately (`frontend/gui_diagnostics.c:261–269`); there is no heartbeat during the waits.

Examples: fan 600 ms of kick, 2000 ms of spin-up then the BDK measurement able to wait 2 s (`diagnostics/vendor/probe_power.c:806,813,974–993`, `bdk/thermal/fan.c:86–96`) and a clean measurement of 1 s. The Wi-Fi waits up to 40000 iterations of 500 µs per attempt, with up to three attempts (`vendor/probe_wifi.c:501–541`). The exit branches can shorten these caps; 60 s must not be announced systematically.

**Testable fix.** Add a state refresh paced at a low frequency to the long waits and to the safe limits of the phases, with phase name/elapsed time. Respect the sampling periods without rendering; do not shorten the measurement windows to embellish the duration. `manual_system_maintenance` already knows how to limit itself to the refresh when `diagnostics_suspended` is true (`frontend/gui.c:784–791`): a possible integration point to examine without importing the frontend into the vendor code.

**Risks.** Do not launch `lv_task_handler()` or re-enable without distinction touch, rails, Bluetooth, SD or hardware tasks from a probe. The diagnostics have snapshot/restoration and suspension of the inputs; any future cancellation must go through the same cleanup. The user text currently explicitly announces the suspended commands (`gui_diagnostics.c:284`): the absence of interactivity is therefore not described as a broken Cancel button.

**Validation.** Simulated timer: bounded number of refreshes, no access to the suspended peripherals, same duration/order of the measurements and same verdict; cleanup/restoration executed once after success and error. Then comparison of the hardware results with the v29 reference.

## 7. TV: keep the delays, make the steps cooperative — P2

**Evidence.** The negotiation is mostly a state machine, but `bdk/display/tv/usbpd.c:957–958` still waits synchronously for the end of a 100 ms delay, and `:1032` waits 250 ms for `ND_BUSY`. The GUI task calls `dock_step` then, when the dock is ready, `dpout_aux_init`, `dpout_probe_sink` and `dpout_video_start` synchronously (`frontend/gui_tv.c:329,354–365`). The DisplayPort training loops, rate fallbacks and margin trials have several waits (`bdk/display/tv/dpout.c:1817–1860`). The 45 s session limit is evaluated only at the entry of `_tick` (`gui_tv.c:321`), not during these calls.

**Testable fix.** Transform the 100/250 ms waits into states with a deadline, without changing their duration. Capture a single `elapsed` value instead of recalculating between condition and subtraction at `usbpd.c:957–958`. Split the sink reading, the training and the margin into bounded phases only if safe pauses are identified; display the step and check disconnection/cancellation between these phases. Start with the simple PD states, less risky than the SOR/PLL sequencing.

**Validation.** Simulated clock: same order of commands, no call before its delay, `ND_BUSY` up to its limit, HPD removal, return to a clean state, restoration of the clock and rails. The video stability, the negotiation of varied docks and the PLL timings remain console tests. Do not propose a TV FPS gain: output configured 720p/60 Hz and effective frame rate of the interface are different measurements.

## 8. Diagnostic report: avoid the work after truncation — P3

**Evidence.** `append` abandons if the report is muted or truncated (`nyx_diagnostics.c:77–90`). Yet `_log_emit` always translates and allocates the line before calling it (`:98–101`). After saturation of the 256 KiB report (`:29`), talkative probes thus continue the translation searches and allocations freed without text added. The translation can recursively build a new string (`localization.c:235–251`).

**Testable fix.** Keep first the recording of the severity in `_log_emit`, then short-circuit only the translation/append when no output is consumable (`muted`, `truncated` or report absent according to contract). Do not skip the discoveries/verdicts. Avoid a cache of all the lines: the variable values would have little reuse and would increase the heap.

**Validation.** Saturated report: same truncation marker, same results and findings counter, zero additional translation expected after truncation; non-truncated case identical in the seven languages. CPU gain not quantified before measurement.

## USB, SD and memory: conservative decisions

The UMS service already has periodic maintenance and training (`bdk/usb/usb_gadget_ums.c:1794–1810,1914`), Erista/Mariko controller selection and fixed buffers. No element read justifies enlarging the transfers or increasing the rates arbitrarily. The UMS writes and caches require real host/card tests; the PC substitutions demonstrate neither throughput nor integrity after a brutal removal.

The UMS dates use absolute comparisons `timer < time` (`:1801,1806`) rather than a calculation of elapsed time. A long session crossing the millisecond counter overflow can delay the maintenance. This is a secondary robustness improvement: safe deadlines at the rollover and tests near `UINT32_MAX`, without shortening the existing intervals.

The SD keeps its speed fallbacks and retries (`bdk/storage/sd.c:93–179`); removing these fallbacks or increasing the maximum mode in the name of performance would compromise the marginal cards. A useful optimization must separate initialization time, sequential throughput, small files and rendering time; a global reading throughput does not prove an improvement of the boot.

## Validation and limits of this audit

Reading of the sources and of the build chain only; no hardware test, benchmark, build or execution of the v29 substitutes performed by this audit. The scripts `work/check_nova29_*.py` inspected concern the v29 reference and several explicitly target the `hekate-nova-pro-v29` directory. Running them unchanged would not validate the v30 modifications. For any future fix, explicitly target the right tree and record the hashes of the tested sources.

Recommended order: build guards; refusal of invalid files and SD write contract; local leak; animation overlay; diagnostics heartbeat; simple TV states. Reserve the global evolution of the heap and the splitting of the video training to independent changes. For each step, keep the Nova features, the custom resources, the languages, the RAM return behaviour and the v29 hardware results.

Post-audit validation: local fixes of nyx.c integrated and check_nova30_core_io.py executed by the parent; 20 v30 cases pass and 8 real v29 regressions are reproduced. No modification of the common BDK helpers nor of the loader in this version.
