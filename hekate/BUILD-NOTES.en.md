# Current version — Hekate Nova Pro 11 (6 November 2026)
**Languages:** [Français](BUILD-NOTES.md) · **English** · [Español](BUILD-NOTES.es.md) · [Русский](BUILD-NOTES.ru.md)

The twelve modifications and their operation are described in [NOVA-PRO-V11.md](NOVA-PRO-V11.md). The build and the whole set of v11 checks are validated. Hekate binary: 110036 bytes; Nyx: 1924293 bytes; RAM service: 77052 bytes. The compressed content of the final payload corresponds to the new decompressed Hekate; the loader dependencies are fixed for the incremental builds.

The checks include 4 260 LVGL scenes (fonts, languages, dimensions, hardware availabilities, palettes and transparencies), the settings, the SD errors, the image loading, the boot animation and the contracts of the RAM service. The hardware probes are not executed on the console during these PC checks.

The background animations have been removed. The reflection measurements concern surfaces redrawn by LVGL on computer, without any Switch FPS value. Physical trials of this version still to be carried out.

---
## History — previous versions

# Nova — prototype 10, 6 November 2026

The Nova presentation is implemented in the native LVGL components:
top navigation, full cards, detail banner, switches with written state
and Lucide 1.8.0 icons identical to the mock-up's SVG paths.
The 32 cards take their French texts, with a catalogue of seven
languages. The palette, the 8-pixel radius and the fixed 18-pixel halo come
from this reference. The content occupies 1 280 × 580 pixels, under a header
of 76 pixels and above the status bar at 656 pixels.

The alpha masks of the icons are precomputed in three sizes; their LVGL decoding
is enabled. The typographic roles are cached from the NYXF files or the
built-in font. The card callbacks keep their identifiers
and their guards; the selection is distinct from the opening of the action.
The boot options, the quick commands and the Appearance interface
remain accessible from the Settings titles and the HEKATE logo.

The native checks on computer cover 1 890 scenes and twelve additional
availability cases. They also check the pixels of the Lucide glyphs, the states of the switches,
the cancellation of the choices and the saving errors. The halo invalidates at most
its local zone and maintains no refresh at rest. This result does not measure the hardware FPS.
See verification/reference-nova.json and verification/performance-halo-nova.json.

The following notes describe the previous versions. BUILD-INFO.json
gives the sizes, fingerprints and results of the current build.

# Studio — prototype 9 (history)

The current version adds a native 224-pixel side navigation, a 64-pixel
header and a main area of 1 056 × 592 pixels.
The tab indices and the existing actions are preserved. The
anthracite/purple theme and the 6-pixel corners are shared with the detailed
windows. The icons are those of the official resources of the project.
The 28-pixel halo uses a local shadow on the selected icon,
without animation task or allocation per frame. The switches read
the real state of the button, including after closing the Auto Boot choice.

The LVGL rendering on computer checks the navigation, the switches,
the dimensions in the seven languages and the three fonts supplied.
The halo check bounds the drawing during a change and verifies
the absence of refresh at rest, background effects paused.
The current measurements are in verification/performance-halo-studio.json.
The hardware checks and the RAM service have not been modified.

The notes of prototype 8 below describe the optimizations kept
and the previous presentation. BUILD-INFO.json describes the current build.

# Fixes of prototype 8 (history)

The main tabs are compact and bounded by the top/bottom bars.
The theme uses a plain background and grey/white outlines with a radius of 8 pixels.
The colours of the pills are rendered explicitly; their styles are
reused rather than reallocated on each opening.

The driver waits for the end of the previous image before the first write,
then launches a single VIC rotation at the end of the LVGL refresh.
The sorted Unicode fonts use a binary search; the SD faces
have direct ASCII access. The search for the translation patterns is indexed
and the fragment search no longer creates a temporary copy of the key.
The animation offsets and identical dynamic texts no longer redraw
their labels without a change. No hardware frequency has been increased.

The effects no longer use twenty moving objects. Their drawing work
is bounded to a band of 64 pixels or to sixteen small zones, and suspends
during the gestures. The measurements of `verification/performance-fonds.json`
are those of the LVGL software rendering on computer, and not of console FPS.
The display checks verify the grouped presentations, the visible controls and the pills.
The physical trials of this version still have to be
done. The confirmations and the hardware operations are preserved.

# Windows build — custom prototype

Prototype 9 built on 5 November 2026 from the project's Makefiles.
The notes of the earlier prototypes below describe their historical state.
The current sizes and detailed results are in BUILD-INFO.json.

- ARM target: devkitARM GCC 16.1.0 installed by devkitPro.
- Windows tools: GCC 16.2.0, GNU Make 4.4.1 and the BusyBox shell of w64devkit 2.11.0.
- The w64devkit archive was obtained from the official skeeto/w64devkit repository and
  its SHA-256 was verified against the GitHub release metadata.
- MSYS2 could not create its signal channel in the runtime environment.
  The build used the portable Windows tools and a local copy of
  `base_tools` without the PATH rewriting specific to MSYS2. The Makefiles,
  the ARM compilation options and the link scripts of the project are preserved.
- No additional software has been installed in devkitPro by the build.

Results: uncompressed Hekate 134 529 bytes (limit 140 288), final payload
110 204 bytes (limit 126 296). Nyx now includes the SD font engine,
the effects and the extended catalogue. The current sizes of Nyx and of the RAM
service are recorded in BUILD-INFO.json (service: limit 130 392). The limits are checked
on the final files, including the six-byte suffix of the payload.

The ARM outputs are ELF32 ARM EABI5, with entry points 0x40007000
(loader) and 0x81000000 (Nyx). The binary files of the loader and of Nyx
were verified against the linked ELF; the payload includes the ICTC49 suffix.

The compiler issued non-blocking warnings for unused variables of LVGL, an old
button function that has become unused and the RWX link segments
of the embedded programs. GNU Make also reports
the reset of parallelism in the sub-builds.

The saving of the settings passed 96 C cases on the real functions extracted
from the sources, with SD card doubles: modes 0/1, invalid values,
preservation of the old settings and mount, opening, writing and closing
failures. This does not replace the hardware trials.

Prototype 7 had added checks on the SD fonts, the actions of the Appearance
menu, the effects and the layouts. Renderings were produced with
the real LVGL on computer for the cards and the Appearance screen, at 36 pixels
in French and in Russian. The selectors were fixed after inspection.
The resources of the RAM test are validated with its real engine and its framebuffer
drawing; they remain in the first 4 MiB already excluded from the test.
See PERSONNALISATION.md and TRADUCTIONS.md for the current scope.

The final catalogue includes 3 688 keys. The complements of the probe reports
were translated offline with Argos/CTranslate2 models;
the written menus and 201 reviewed entries keep their priority. The automatic
checks verify the 1 268 source strings of the probes, the order of the
parameters, the numeric notations and the paths. The linguistic quality
of all the automatic comments has not been validated by native
speakers. The translation models are not part of the SD package.

The package contains the unchanged precompiled resources of the official
Hekate 6.5.4 / Nyx 1.9.4 release, as well as the newly built custom
binaries. Their provenance and their fingerprints are in
BUILD-INFO.json next to the instructions of the binary package.

To rebuild in a working devkitPro/MSYS2 environment with a native GCC available,
the standard command remains `make -j4` at the root of the project.

## Complete integration of prototype 5

The 8 128-byte AArch64 Wi-Fi program is included and required at build time.
The native table contains the 42 HWTest-RCM entries. The RAM service uses the exact
Atmosphère revision indicated in DEPENDENCIES.json and keeps a Hekate return
image in the zone already excluded from the first 4 MiB of DRAM.

Additional checks on the real code: 108 assertions of the bounded payload
reader; bounded capture and ten export cases; six synthesis cases
with missing keys, partial success, warnings and failures; 28 cases of the RAM/return
contract. The C decoder of the loader reconstructs all the bytes of
the decompressed RAM image. None of these checks simulates the physical
measurements of the console peripherals.

The sources of the RAM companion and its necessary dependencies are supplied
with the Hekate sources. On a system with devkitARM, devkitA64,
Python 3, Git, native GCC and Make, build Hekate at its root, then the RAM service
in HWTest-RCM-0.15/ramtest. The Windows scripts under build-tools are
references for the portable environment used in this session.

## Interface and languages of prototype 6

Transparent backgrounds on the navigation components; Diagnostic icon in
the tab and on the home screen. Seven languages, accents and Cyrillic: the new
fonts are 4 bpp bitmaps of 707 characters at heights 20 and 30 pixels.
The Noto Sans font and its SIL OFL licence are supplied with their generator.
The switch to UTF-8 preserves the Hekate symbols and fixes the text cuts
that previously used the byte offsets as character counts.

The eMMC backup/restore uses stable button identifiers,
independent of the translated labels. The button matrices keep their
control bytes and release their translations on closing. The user
names, paths and technical reports are not translated. The separate RAM
service keeps its own interface in English.

The additional checks execute the C configuration code, actions of the Language
menu and of the translation engine, with allocation errors
and menu cleanup. The fonts are checked with the real LVGL reader; their decoded
preview was inspected on computer. These checks
do not replace the visual and hardware validation on a Switch.
