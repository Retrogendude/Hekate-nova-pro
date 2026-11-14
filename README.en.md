# Hekate Nova Pro
**Languages:** [Français](README.md) · **English** · [Español](README.es.md) · [Русский](README.ru.md)

Experimental customisation of Hekate / Nyx: Nova interface, hardware diagnostics, customisation and support for the dock and wireless Joy-Con. Unofficial project; the authors of the original components retain their rights and their attribution.

## Download

**[Beta v32: files and sources](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32)** · [All versions](https://github.com/Retrogendude/Hekate-nova-pro/releases) · [Full changelog](CHANGELOG.md) · [User guide](docs/NOTICE-UTILISATION.md)

v32 improves expert partitioning and the recovery of a silent Joy-Con. PC testing and compilation are verified; **the 2–3 metre wireless range remains to be validated on console**. The early versions are kept for the record and are not recommended updates.

![Partitioning menu v32, rendering of the code on PC](docs/images/partitionnement-v32.png)

## Features

- Nova home screen with tiles or carousel, settings split into tabs and windows sized to fit the banners.
- Seven languages: English, French, Spanish, Italian, Russian, German, Portuguese; SD fonts and adjustable text size.
- Independent colours, palette with preview, wallpaper, transparency, PNG icons and a selection reflection that can be disabled.
- Native diagnostics from HWTest-RCM: 42 entries, exportable reports, per-component summary, interactive checks and a separate RAM test with return to Hekate.
- Simplified / expert partitioning, sizes read in GB, Hekate reservations and confirmations retained.
- TV output via dock on compatible consoles: TV only or TV + console screen. The Switch Lite has no TV output.
- Bluetooth Joy-Con: reading of pairings, private SD backup, automatic loading without Joy-Con on the rails and background processing. Connection and range remain experimental.

## Installation

**[Open the full guide](docs/NOTICE-UTILISATION.md)**: installation, initial settings, wallpapers, fonts, icons, diagnostics, dock and wireless Joy-Con.

Download the compiled package of the chosen version from Releases. With the console powered off, keep a copy of the bootloader, then merge the supplied folders with the root of the SD card, replacing only the files provided. Keep personal configurations and pairings. Use the payload matching the version, or the supplied bootloader/update.bin, depending on the boot method.

The instructions specific to each version, its limits and its verifications are in docs/releases and in its archive. Versions v1 to v5 are kept as sources only, among the files available here.

## Sources and building

The hekate folder contains the Hekate / Nyx sources of the selected snapshot. The original source archives remain attached to each release, together with the associated build and verification scripts. They keep their historical layout and are the reference for reproducing an old delivery.

In the repository: enter hekate, then use the project Makefiles. The required tools include devkitARM, devkitA64, GNU Make, Python 3 and a native C compiler for the components that need one. The HWTest-RCM-0.15 companion and the additional tools are kept where they were present in the archive. See BUILDING.txt and the version's guide for Windows specifics.

Local working directories, SDKs and console data are not imported. The compiled archives are Release assets, not ZIP files added with each Git commit.

## History and testing status

34 snapshots are kept: v1 to v32 and two standalone TV experiments. The tags and commits were reconstructed from the archives when they were published; they do not claim to restore the original development dates or commits. A backup of the v15 sources was found and compared against the hashes of the modified sources kept in that version's evidence; the two generated payload files are not part of this source archive.

Native previews, tests with simulated peripherals and console feedback are distinguished in the guides. A PASS verdict only covers the checks actually run. The RAM, touchscreen and screen results have their own limits, detailed in the documentation.

## Origins, credits and licences

- [Hekate / Nyx — CTCaer](https://github.com/CTCaer/hekate) and contributors: original licence kept in hekate/LICENSE; upstream README kept in hekate/README.md.
- [HWTest-RCM — Creased](https://github.com/Creased/HWTest-RCM): probes, companion and provenance comments kept in the relevant files. See hekate/nyx/nyx_gui/diagnostics/PROVENANCE.md.
- [BTstack — BlueKitchen](https://github.com/bluekitchen/btstack): separate licence kept in hekate/nyx/nyx_gui/libs/btstack/LICENSE, including a personal and non-commercial use restriction. This component is not presented as covered by Hekate's GPL licence alone.
- LodePNG, icons, fonts and other components keep their own notices in the sources and packages.

No affiliation with Nintendo, CTCaer, Creased or BlueKitchen is claimed. The component licences and the original notices remain applicable.

## Bluetooth reports

To report a problem, stop the Bluetooth service after the test and keep bootloader/bluetooth/last-session.txt. State the version, the model, the distance and whether or not a dock is used. **Do not publish the joycon-pairing.bin / .bak files, prod.keys, title.keys or personal dumps.**
