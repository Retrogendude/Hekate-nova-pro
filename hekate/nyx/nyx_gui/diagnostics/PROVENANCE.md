# Native diagnostic sources

The files under `vendor/` originate from the user-supplied HWTest-RCM0.15
archive, source repository `https://github.com/Creased/HWTest-RCM`, archive
commit `778e25b66a11c5cdd65c11a69ef8083aff1cd9f7`. Their existing copyright
and implementation comments are retained. Hekate's own LICENSE applies to
the Nyx integration; verify upstream licensing before a public distribution.

Integration-specific changes:

- The original main program, GUI, framebuffer logger and UART command loop
  are not linked into Nyx. `nyx_diagnostics.c` hosts the original42probe
  functions through the original page/group table in `hwtest_pages.c`.
- Report capture is bounded to256KiB and visibly marks truncation; the
  findings registry has128entries and reports real positive findings
  separately from pages that only print observations.
- SD access reuses Nyx's mounted volume. Export writes captured text only,
  checks partial writes/close failures, and never reruns a probe.
- PRODINFO is conditional on `sd:/switch/prod.keys` and readable/updateable
  SE keyslots0/1. The adapter snapshots/restores those keyslots. Locked slots
  are left untouched and the source reports the missing prerequisite.
- UART-B is owned by Joy-Con. No diagnostic logging or input polling uses it;
  the register observation avoids an unclocked controller and does not judge
  Nyx's baud rate against the standalone payload's115200debug setting.
- The AArch64 Wi-Fi CPU0stub is required at build time; its8KiB image is
  embedded in Nyx. Runtime image/stack/mailbox reserve A0000000..A003FFFF,
  outside Nyx's heap and display buffers. The original supervised CPU0
  handoff and timeout/error reporting are retained.
- The host pauses input/maintenance without powering Joy-Con down. Cleanup
  restores owned GPIO/pad configuration, fan PWM, audio clocks/power/pad
  routing, PCIe rails/power state, SE keys and eMMC user partition. The host
  then reinitializes the touch controller through the non-autotune helper
  and resynchronizes Joy-Con. A restoration failure is returned distinctly.

All42regular probe entries target-compile against the supplied Hekate6.5.4
BDK. This includes observations and prerequisite-dependent probes; it is not
a claim that every probe can pass on every console or that physical hardware
validation has occurred. Full-memory stress testing is a separate service,
since it must not overwrite the memory of a running Nyx instance.
