# Isolated RAM service and Hekate return

When started by the custom Nyx entry, this service runs one finite advanced
test across every testable 4 MiB region, using all seven supplied patterns.
Nyx is not kept alive while its RAM is tested. On completion, abort, training
failure or SMMU failure, the service records a result and reloads Hekate.
Hekate then starts a fresh Nyx instance, which can inspect the saved result.

The return payload is read from `bootloader/update.bin` by Nyx before launch.
It is staged at `0x803E0000`, with a 128 KiB maximum, and its handoff/result
record is at `0x803DF000`. Both are entirely inside the first 4 MiB of DRAM,
which the original RAM service already excludes from destructive testing.
The shared binary contract is `source/nyx_ram_handoff.h`, copied verbatim to
the Hekate diagnostics directory as `ram_handoff.h`.

The service checks the request version, size, magic and FNV-1a checksum after
hardware initialization, again after memory training, and before return.
If the return image is damaged, it refuses to jump to corrupted code and
offers restart/power-off controls. That is the deliberate exception to the
automatic-return behavior. A catastrophic RAM/EMC fault can prevent any code
from progressing and cannot be recovered by a software guarantee.

For return, the SMMU is disabled first, training clocks are restored, the
validated payload is copied to `0xC0000000`, and the small relocator runs from
upper IRAM `0x4003F000`. It copies the payload to `0x40010000` and jumps there.
IRQs are stopped; hardware deinitialization flushes/disables the BPMP cache.
Only the request marker is consumed. The independent result marker, hash and
summary fields remain for the next Nyx instance.

Address audit: the pinned Atmosphere MTC engines use statically embedded
IRAM timing tables / the IRAM MTC buffer and MMIO accesses to EMC, MC and
CLKRST registers; they contain no CPU writes to the `0x803xxxxx` return area.
BDK hardware initialization also contains no writes to that area. Training
patterns are written through EMC's training-pattern RAM registers. The
post-training checksum verifies the retained bytes before the test begins.
This source audit is not a physical-console validation.

The standalone, directly injected service still offers its original physical
button menus when no valid Nyx handoff request is present. Its palette and
rounded selection rows match the custom Nyx green/dark theme without adding
the full LVGL runtime to the IRAM service.

Exact dependency provenance and archive hash are in `DEPENDENCIES.json`.
Atmosphere's LICENSE is retained under `third_party/Atmosphere/LICENSE`.
The port uses the supplied Hekate 6.5.4 BDK; the original HWTest gitlink is
also recorded so this compatibility choice remains explicit.

Build: `python work/build_ram.py` in the prepared Windows workspace, or use
the RAM Makefile with devkitARM and host Python/Git configured. The Makefile
copies its generated Atmosphere tree instead of creating privileged Windows
symlinks, and applies the original zero-context adaptation patch with
`git apply --unidiff-zero`.
