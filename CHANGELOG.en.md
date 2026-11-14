# History of Hekate Nova Pro
**Languages:** [Français](CHANGELOG.md) · **English** · [Español](CHANGELOG.es.md) · [Русский](CHANGELOG.ru.md)

History imported from the preserved archives. The tags describe source snapshots, not the original development commits. The detailed guides are kept for each version. The prototypes and Bluetooth versions remain pre-releases.

## v32 — Nova Pro 32

- Summary of the partitioning on a single line; values on the right, airy rows, separate handles and buttons.
- Restart of a silent Joy-Con stream, then reconnection of only the pair concerned, with protection against UART data still pending.
- WL_REG_ON radio state aligned with the HWTest reference during the session and restored on stop.
- Compilation and PC checks successful. Use at 2–3 metres remains to be validated on console; no range gain is guaranteed.

[Detailed notes](docs/releases/v32.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32)

## v31 — Nova Pro 31

- Full-width expert partitioning, lower summary and −/+ settings; whole GB sizes with 29 / 58 GB Full.
- Bluetooth processing during software drawing, image copies and display waits.
- Read-only HCI measurements: relative RSSI, quality, power and failed contacts; full report-gap statistics.

[Detailed notes](docs/releases/v31.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v31)

## v30 — Nova Pro 30

- At boot, SD loading and Bluetooth in the background only when both rails are empty.
- Short presses preserved, bounded decorative invalidations and cooperative processing in long tools.
- Audit of resource reads and cleanup; diagnostics keeping their reserved use of the peripherals.

[Detailed notes](docs/releases/v30.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v30)

## v29 — Nova Pro 29

- Acceptance of the already validated Nintendo and standard pairing profiles; rejection of truncated SPI responses.
- Private 72-byte SD save / load with checksum, backup copy and transactional replacement.
- Connection timeout of 5.12 s, spaced retries without a definitive stop at the third failure.

[Detailed notes](docs/releases/v29.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v29)

## v28 — Nova Pro 28

- Short 250 µs budget during display and less frequent refresh of the Bluetooth panel.
- Diagnosis of the pairing import, button counters and report rate.
- This version diagnoses the 0/2 but does not yet fix its classification; fixed later in v29.

[Detailed notes](docs/releases/v28.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v28)

## v27 — Nova Pro 27

- ROM profile adapted from HWTest-RCM: packets at base rate, sleep / active mode, QoS and extended UART reading.
- Power, clock and restoration of the previous state; enriched controller diagnostics.
- First hardware feedback on wireless commands, with latency and reliability still to be improved.

[Detailed notes](docs/releases/v27.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v27)

## v26 — Nova Pro 26

- Acceptance of simultaneous HID openings initiated by Nova and by the Joy-Con.
- Handling of stale channels, late refusals and timeouts, with retention of the valid session.
- Fixes verified by simulated HCI/L2CAP/HID negotiations, with no hardware guarantee of range.

[Detailed notes](docs/releases/v26.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v26)

## v25 — Nova Pro 25

- Preference for the central role during an incoming reconnection; serialised outgoing attempts.
- Encryption enabled and connection states better tracked; controller metadata and roles added to the report.

[Detailed notes](docs/releases/v25.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v25)

## v24 — Nova Pro 24

- Fix of the byte order of the imported keys and check of the console's Bluetooth identity.
- Import from the rails verified through the real SPI path and comparison against independent reference values.

[Detailed notes](docs/releases/v24.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v24)

## v23 — Nova Pro 23

- Resumption of the controller connection based on the HID failures observed; opening of the known channels without waiting for the SDP description.
- Anonymous report of the negotiations, authentications and HID steps; hardware validation still incomplete.

[Detailed notes](docs/releases/v23.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v23)

## v22 — Nova Pro 22

- Core update to Hekate 6.5.5 / Nyx 1.9.5 and incorporation of the documented upstream fixes, without replacing the Nova theme.
- Fix of the false Old Nyx GUI found warning: metadata at the expected offset and linkage guard.
- Navigation with the Joy-Con available, waking of the hidden cursor, handling of simple reports and retention of the settings.

[Detailed notes](docs/releases/v22.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v22)

## v21 — Nova Pro 21 — Bluetooth

- First personal Bluetooth service for detached Joy-Con, with pairing import and navigation.
- Read-only banner of USB Tools reformatted; Credits panel background restored.
- Changelog tab added to the credits. Historical experimental version, followed by several Bluetooth fixes.

[Detailed notes](docs/releases/v21.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v21)

## v20 — Nova Pro 20 — Dock

- Automatic dock detection and display switching after boot.
- TV only or TV + console screen settings; TV driver kept between the menus.
- Reference connection: compatible console inserted directly into the official dock; the Switch Lite does not provide a TV output.

[Detailed notes](docs/releases/v20.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v20)

## v19 — Nova Pro 19 — TV

- First manual TV integration in Nova, maintained between the menus.
- Reported hardware reference: TV01 prototype, OLED directly in an official dock, stable image and VOL− stop.
- Do not present this prototype observation as a validation of all consoles or of all TV versions.

[Detailed notes](docs/releases/v19.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v19)

## tv-experiment-02 — Standalone TV experiment 02

- Standalone TV program of reference on Hekate 6.5.5 base; distinct from the Nova versions.
- Test on official dock; keep the limits and the VOL− procedure described in the original guide.

[Detailed notes](docs/releases/tv-experiment-02.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-02)

## tv-experiment-01 — Standalone TV experiment 01

- Standalone TV program of reference on Hekate 6.5.5 base; distinct from the Nova versions.
- Test on official dock; keep the limits and the VOL− procedure described in the original guide.

[Detailed notes](docs/releases/tv-experiment-01.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-01)

## v18 — Nova Pro 18

- Credits limited to the top tabs, authors and licences kept.
- Transparent PNG icons from the icone folder, Launch image and BMP compatibility.
- Outlines removed from System options; V1/V2 grouped together and FAT32 capacity calculated in the compatible profiles.
- Minimum / Recommended for Linux and Android, GB sizes in expert mode. TV output is still a study at this stage.

[Detailed notes](docs/releases/v18.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v18)

## v17 — Nova Pro 17

- Rectangular palette, preview persistent between the targets and independent boot colours.
- Customisation of the Launch tile, logo centring and loading progress.
- Summary of fifteen components, actual test coverage and research leads; export of the additional checks.
- Interactive Joy-Con/Lite tests, sticks, touchscreen grid, screen test patterns and RAM test with fast / advanced / custom modes.

[Detailed notes](docs/releases/v17.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v17)

## v16 — Nova Pro 16

- Carbon wallpaper and Nova colours by default; windows at 75 % transparency on first launch.
- Compact simplified / expert partitioning: console profiles, full emuMMC, independent Linux and Android.
- Manual brightness to the right of the clock; backup/restore, USB and logs re-laid out.
- Blue/purple selection border and accelerated reflection; background animations still removed.

[Detailed notes](docs/releases/v16.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v16)

## v15 — Nova Pro 15

- Readability of the reports, eMMC alignment and table pagination; translation of the backups and logs.
- Cumulative colour preview, progressive introduction and 0.48 second reflection.
- Aligned emuMMC columns, paged file selection and re-centred submenus.
- Font converter fixed for accents and Caviar. Source archive restored from the preserved v15 snapshot; 23 hashes of modified sources match, the two generated payload files being excluded.

[Detailed notes](docs/releases/v15.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v15)

## v14 — Nova Pro 14

- Reflection widened to 129 pixels with transparent gradient at the ends.
- Accelerated transition to about 0.8 second, always on the complete tile.

[Detailed notes](docs/releases/v14.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v14)

## v13 — Nova Pro 13

- Fix of the window order and of the touch navigation when a wallpaper is active.
- Diagonal white reflection across the whole selected tile, fixed outlines.

[Detailed notes](docs/releases/v13.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v13)

## v12 — Nova Pro 12

- Wallpaper display order fixed; six independent colours, including logo, icons and selection.
- Access to the credits via the logo, coloured temperature and switches matching the theme.
- Original Hekate date/time setting kept and enriched with the language choice at first boot.

[Detailed notes](docs/releases/v12.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v12)

## v11 — Nova Pro 11

- Battery on the right; five tabs on the left and four quick actions on the right.
- Nova style extended to the submenus, removal of the banners, two-press selection and new monogram.
- Independent colours, BMP wallpaper, opacity of the menus and icons, Hekate Nova Pro boot.
- Removal of the waves, particles and other background animations; effects limited to the selection.

[Detailed notes](docs/releases/v11.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v11)

## v10 — Nova

- Adoption of the Nova rendering: top tabs, anthracite/blue cards and thin Lucide icons.
- Large boot card and four secondary accesses; Tools / Info grids and twelve Diagnostic categories.
- Selection detail banner and Tiles / Carousel home kept.

[Detailed notes](docs/releases/v10.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v10)

## v9 — Studio

- Studio presentation with side navigation, anthracite background, purple accents and six-pixel corners.
- Fixed selection halo without continuous refresh; translated ON/OFF switches and menus framed between the banners.

[Detailed notes](docs/releases/v9.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v9)

## v8 — Compact interface and smoothness

- Removal of the large vertical lists in favour of compact tabs and visible selectors.
- Content contained between the top navigation and the bottom status bar; Interface / System and Appearance settings split up.
- Dark transparent cards, less rounded outlines and long help accessible via buttons.

[Detailed notes](docs/releases/v8.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v8)

## v7 — Customisation

- Appearance menu: language, SD font in NYXF format, text size, layout and background effects.
- Extended translation of the menus and technical texts; appearance controls grouped together.
- The background animations of this version were removed later to lighten the rendering.

[Detailed notes](docs/releases/v7.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v7)

## v6 — Transparency and languages — prototype 6

- Modern transparent style in the menus, controls and cards, with a Diagnostic icon.
- Seven languages: English, French, Spanish, Italian, Russian, German and Portuguese; accents and Cyrillic.
- Tiles / Carousel, 42 native diagnostics and isolated RAM test with restoration of the result on return.

[Detailed notes](docs/releases/v6.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v6)

## v5 — Full diagnostics — prototype 5

- Integration of the 42 native HWTest-RCM entries, categories, individual execution and exportable reports.
- Separate memory test service with return to Hekate; retention of the probes and provenance comments.
- The source archive is available; no specific v5 binary package was found among the preserved deliveries.

[Detailed notes](docs/releases/v5.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v5)

## v4 — Tiles and carousel — prototype 4

- Two Tiles / Carousel home screens selectable from the Nyx settings.
- The custom-sources archive also belongs to this stage; its slightly different notes are kept as a historical document. Sources only.

[Detailed notes](docs/releases/v4.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v4)

## v3 — Original interface — prototype 3

- Return to the original interface layout, with Diagnostic access.
- DRAM read fix kept. Sources only.

[Detailed notes](docs/releases/v3.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v3)

## v2 — Console interface — prototype 2

- Console-type home screen with large cards and access to the main functions.
- Read fix for the second byte of the DRAM channels; diagnostics kept. Sources only.

[Detailed notes](docs/releases/v2.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v2)

## v1 — Native diagnostic — prototype 1

- New Diagnostic tab in Nyx; screen identification, backlight and DRAM.
- PASS/WARN/FAIL verdicts and Nyx adapter for the HWTest probes. Sources only; the DRAM test identifies the memory without scanning all its cells.

[Detailed notes](docs/releases/v1.md) · [Downloads](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v1)
