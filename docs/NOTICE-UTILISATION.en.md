# Hekate Nova Pro — user guide
**Languages:** [Français](NOTICE-UTILISATION.md) · **English** · [Español](NOTICE-UTILISATION.es.md) · [Русский](NOTICE-UTILISATION.ru.md)

Guide for the **v32 beta**, based on Hekate 6.5.5 / Nyx 1.9.5. Updated: 14 November 2026.

[v32 installation package](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32) · [History](../CHANGELOG.md) · [Technical guide for v32](releases/v32.md)

Nova is used on a console that already has a boot method compatible with Hekate. The launch method depends on the model and on the existing installation.

## Installation and first boot

1. In the files of the [v32 release](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32), download **hekate-nova-pro-v32.zip** and extract it on the computer.
2. With the console powered off, keep a copy of the bootloader folder and of the SD card settings.
3. Copy the supplied **bootloader**, **fonts**, **wallpapers** and **icone** folders to the root of the SD card. Merge the existing folders and replace the files provided by the update.
4. Keep the boot configurations, the personal wallpapers and fonts, and the private pairing files. No formatting is required to install this update.
5. Use **hekate_nova_pro_6.5.5_v32.bin** with the usual method if that method requires an external payload. The corresponding **bootloader/update.bin** file is also supplied.
6. Put the SD card back in the console and boot Nova. The payload and the bootloader/sys/nyx.bin file must belong to a consistent set.

On the first launch, the original date and time setting is accompanied by the language choice. The Carbon wallpaper, the Nova colours and the windows at 75 % transparency are the default values of a new installation. Preferences already saved take precedence during an update.

### SD card layout

```text
SD card root/
├── bootloader/
│   ├── sys/
│   └── update.bin
├── fonts/
│   ├── Lecture.nyxf
│   ├── Lecture-renforcee.nyxf
│   ├── Lecture-fixe.nyxf
│   └── MaPolice.nyxf          # optional
├── wallpapers/
│   ├── Nova-Carbon.bmp
│   └── MonFond.bmp            # optional
└── icone/
    └── diagnostic.png        # optional example
```

The MaPolice.nyxf, MonFond.bmp and diagnostic.png files above are examples to add yourself.

## Finding your way around the menus

| Menu | Use |
|---|---|
| Home | Boot configurations, payloads, emuMMC and access to the diagnostics. |
| Tools | Storage, backup/restore, USB access and partitioning. |
| Info | Hardware and system information. |
| Diagnostic | Check categories, individual or full tests, results and export. |
| Settings | Language, reading, appearance and system options. |

In the **Tools, Info and Diagnostic** grids, a first press selects the tile and shows its detail; a second press on the same tile opens it. The quick settings icon at the top right gives access to the Nyx settings.

To boot a system, open **Launch** and choose a configuration already present on the SD card. The entries displayed depend on the boot files of the existing installation.

After a customisation, use **Save and reload** to apply and keep the choices. Changes specific to the boot logo are visible at the next boot of Hekate.

## Adding and choosing a wallpaper

1. Copy the images to the **wallpapers** folder at the root of the SD card.
2. Use **uncompressed BMP, in 24 or 32 bits**, of at most **1920 × 1080 pixels**.
3. Choose a simple name, for example **MonFond.bmp**, without accents and without a subfolder.
4. In **Nyx Settings → Themes and colours → Wallpaper and transparency**, select the desired wallpaper.
5. Adjust the opacity, then choose **Save and reload**.

The **Nova-Carbon.bmp** wallpaper is included. The images are cropped and scaled on load. The selector shows up to 32 wallpapers.

The windows and the icons have separate opacity settings:

- **0 % opacity**: transparent wallpaper.
- **100 % opacity**: opaque wallpaper.
- **25 % opacity** corresponds to **75 % transparency**.

The text and symbols remain visible above the wallpapers. Background animations are not part of v32; the selection reflection can still be disabled in the settings.

### Converting a PNG or JPG image

On a Windows computer, keep the files of the **outils-fonds** folder supplied in the archive together, then drag a PNG or JPG image onto **convertir-fond.cmd**. Copy the resulting BMP into the wallpapers folder of the SD card.

This converter requires Python 3 and Pillow. Their preparation is described below.

## Choosing and adding a font

Three compatible fonts are supplied in **fonts**, at the root of the package:

| File | Presentation |
|---|---|
| Lecture.nyxf | Regular font, derived from Noto Sans. |
| Lecture-renforcee.nyxf | Reinforced stroke for contrast. |
| Lecture-fixe.nyxf | Fixed-width characters. |

In **Settings → Appearance and reading**, choose the font and the text size, then **Save and reload**. The main sizes offered range from 20 to 36 pixels; the small labels use a suitable size. The technical reports keep a fixed-width font to align their columns.

### Adding your own font

1. On the computer, extract the complete **outils-polices** folder. Keep convertir-police.cmd and its **tools** and **source** subfolders together.
2. Drag the **TTF or OTF** file onto **outils-polices/convertir-police.cmd**.
3. Retrieve the **.nyxf** file created next to the original font.
4. Copy this file into **fonts**, at the root of the SD card, with a simple name such as **MaPolice.nyxf**.
5. Reload Nyx to refresh the list, select the font and save.

The console loads **NYXF** files; a TTF or OTF file must therefore be converted before it is added. The converter produces the expected sizes and uses Noto Sans as a fallback for missing characters. Its coverage corresponds to the Latin alphabets, Cyrillic and the punctuation needed for the seven languages offered; this does not constitute a universal font for all writing systems.

The selector accepts up to 16 additional files. Use a name of fewer than 64 characters, made up of letters without accents, digits, spaces, hyphens, dots or underscores. A missing or invalid file leads to the use of the built-in font.

Use fonts whose licence allows conversion and, in the case of sharing, redistribution. The Lecture variants supplied come with the SIL OFL licence of their source.

### Preparing the converters under Windows

The two converters, wallpapers and fonts, require **Python 3** and **Pillow** on the computer. Once Python is available, install Pillow in a terminal with:

```text
python -m pip install Pillow
```

Then use the supplied .cmd programs by drag and drop. No converter and no Python is required on the console. The fonts and the Carbon wallpaper already supplied are ready to use.

## Customising the icons and the Launch tile

To replace the five small pictograms of the home screen, copy PNGs with transparency into **icone**, with these names:

| File name | Tile |
|---|---|
| launch.png | Launch |
| configs.png | Configurations |
| payloads.png | Payloads |
| emummc.png | emuMMC |
| diagnostic.png | Diagnostic |

Prefer a square image with transparent margins. The proportions are preserved. The images must remain within the limits of **1920 × 1080 pixels** and **8 MiB**. Reload Nyx after modifying them; a missing or invalid icon leaves the Nova pictogram displayed.

For the image of the large Launch tile, copy a separate image into icone, then choose it in **Nyx Settings → Themes and colours → Launch tile**. **No image** restores the normal wallpaper of this tile. The general wallpaper remains chosen separately from wallpapers and uses BMPs.

## Diagnostics and memory test

In Diagnostic, select a category then launch the desired check or the full test. The results and the per-component summary can be viewed and exported from the results screens. The reports are saved in **bootloader/diagnostics**.

The interactive checks allow, among other things, checking buttons/sticks, touchscreen coverage and screen test patterns. The memory test runs in a separate program with a return to Hekate; choose its mode then export the results on return. Unstable hardware may prevent the normal return.

A PASS verdict concerns only the checks actually run. Correct identification of a chip does not certify all its functions. The summary distinguishes measured results, checks not performed and manual observations.

## Expert partitioning

The expert mode offers three rows: emuMMC, Linux and Android. The sizes are placed to the right of the bars; the **−/+** buttons refine the setting, with repeat if the press is held.

- The **29 GB Full** and **58 GB Full** markers keep the full Hekate allocations according to the model.
- The sizes displayed are whole numbers; the reservations and alignments remain counted in the calculations.
- Android displays its total allocation, system included.
- The bottom summary groups the allocations and their state on one line.

Partitioning changes the medium after the confirmations provided by Hekate. Keep a backup of the data before this operation. Installing or updating Nova launches no partitioning.

![Nova v32 expert partitioning, rendering of the code on PC](images/partitionnement-v32.png)

## TV and wireless Joy-Con

### TV output

In the Nyx settings, open the **TV / Dock** tab, then the **USB-C TV / Dock** settings. Choose the display on the television alone or on the television and the console, as well as the dock auto-detection option.

The reference connection is a compatible console inserted directly into a powered official dock, with HDMI connected to the television. The **Switch Lite does not have a TV output**. Hardware validation depends on the model and on the connection; the TV version guides describe the tests kept.

### First Bluetooth use

1. Use Joy-Con already paired to this console, then attach them to their rails.
2. Open **Wireless Joy-Con** from the TV / Dock settings and choose **Read pairings**.
3. After a valid import, with Bluetooth stopped, choose **Save pairings**.
4. Detach the Joy-Con, choose **Enable Bluetooth** and press a button to wake them. Leave the sticks at rest for a moment before navigating.

On subsequent boots, if both rails are empty and a valid pairing file has been saved, Nova can reload it and start the service automatically, without opening its window. The presence of a Joy-Con on a rail prevents this automatic behaviour. A manual stop is still respected for this boot. Some tools and diagnostics reserve the peripherals and stop the service: restart it manually afterwards if necessary.

v32 requests the reports of a Joy-Con that has become silent again and can reconnect that one pair only. **The 2–3 metre range is not yet guaranteed and remains to be validated on console.**

### Reporting a problem

After the test, choose **Stop Bluetooth** and immediately keep **bootloader/bluetooth/last-session.txt**, before restarting or re-reading the pairings. State the version, the model, the distance and whether or not a dock is present.

Send only the text report. **Do not publish joycon-pairing.bin, joycon-pairing.bak, prod.keys, title.keys or personal dumps.** The pairing backups contain private keys; they are not included in the project archives.

## Sources and limits

This guide describes v32. The historical guides and verification results remain in [the documents of each version](releases/v32.md) and in [the releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). The checks on PC and with simulated peripherals do not replace validation on the real console.
