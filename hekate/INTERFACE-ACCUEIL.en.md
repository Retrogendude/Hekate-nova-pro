# Current home screen — Hekate Nova Pro 11
**Languages:** [Français](INTERFACE-ACCUEIL.md) · **English** · [Español](INTERFACE-ACCUEIL.es.md) · [Русский](INTERFACE-ACCUEIL.ru.md)

The Nova mock-up is preserved with the twelve requested modifications: navigation on the left, four commands on the right, battery percentage next to its icon, new logo and selection reflection. The header badges have been removed. Tools, Info and Diagnostic use two presses on the tile, without an opening arrow. See [NOVA-PRO-V11.md](NOVA-PRO-V11.md).

---
## History — home screen of the previous versions

# Nova — version 10

Nova uses the top navigation of the mock-up, the same Lucide icons,
the anthracite/blue palette, filled cards with a radius of 8 pixels and a fixed
selection halo of 18 pixels. The Tiles home screen takes the large Start card
and the four secondary accesses. Tools and Info are in four
columns; Diagnostic has twelve cards and a detail banner.

The texts and icon names of the 32 cards come directly from the mock-up.
The battery data, the available tools and the applied font are
those of the console. The computer previews use sample data.
See LIRE-MOI-V10.txt for the commands and PERSONNALISATION.md for the settings.

# Studio — version 9 (history)

Studio keeps Tiles and Carousel in an anthracite/purple theme,
with side navigation, framed icons, 6-pixel corners and a fixed
halo of 28 pixels on the selection. The switches display their state
by their position and their translated text. The main pages use
1 056 × 592 pixels, between the header and the status bar, without vertical scrolling. Nyx Settings
and Appearance are divided into internal tabs; the help is behind "?".
See PERSONNALISATION.md for the use and BUILD-INFO.json for the tests.
The notes below describe the previous steps of the interface.

# Hekate 6.5.4 — interface and languages, prototype 7

This version integrates two layouts of the Nyx home screen, selectable
in the settings. The Hekate functions and the new Diagnostic tab remain
accessible in both modes.

## Choosing the layout

1. Open the Nyx settings from the home screen.
2. Open "Appearance", then the section "Home screen layout".
3. Choose "Tiles" or "Carousel".
4. Use "Save and reload" to apply the choice.

Closing the selector without saving keeps the previous layout.
The choice is stored in the `[config]` section of `bootloader/nyx.ini`:

```ini
home_layout=0
```

`0` selects the tiles and `1` the carousel. A missing or invalid setting
uses the tiles. The existing `homescreen` setting remains independent:
to display the home screen at boot, choose "Main menu" in "Home Screen".

## The two modes

**Tiles**: a large start command and four accesses to the additional
configurations, payloads, emuMMC management and Diagnostic.

**Carousel**: cards move in a horizontal zone. The arrows
or a visible card change the selection; "Open" launches only the chosen
function. A selection never launches a payload automatically.
The mode does not promise direct navigation with the D-pad:
the Joy-Con keep the pointer operation already present in Nyx.

Both modes use native shapes and fonts, without additional images to place
on the SD card. A chip icon identifies Diagnostic in the tab bar and on its home
card.

## Languages

Open "Nyx Settings → Appearance", choose a language then "Save & reload".
The menu offers English, Français, Español, Italiano, Русский, Deutsch and
Português. Closing cancels the selection. The modifications already present in
the Nyx settings are also saved by this command.

The `language` field of `bootloader/nyx.ini` is respectively 0 to 6 in this
order. A missing or invalid value uses English. The language choice
is independent of `home_layout`.

The menus, main commands, diagnostic categories, messages and
listed warnings use a catalogue of seven languages. The boot names, paths and technical
reports remain as they are. Some technical information remains in English. The separate
RAM program receives resources in the chosen language, with an English fallback display.
The accents and the Cyrillic are supported by the embedded Unicode fonts
Noto Sans, under the SIL OFL licence supplied with the sources.

## Saving and diagnostic

The selector checks the presence of Nyx before the reload. A saving
error must display a message and prevent the reload. The write and closing
errors of the configuration file are now taken into account by the Nyx
configuration writer.

The home screen theme is also applied to the components of the menus and
sub-menus: windows, panels, buttons, lists, settings, selectors,
switches, dialogue boxes and tab bar. The alert colours
and the numeric texts in fixed font are preserved. The custom backgrounds and tints
remain taken into account. The default theme uses the background
141A19, the accent 93E1AE and rounded corners. The cards, icon discs,
panels, buttons and selections have a transparent background; outlines and text
signal the selection. The overlapping windows, dialogues, keyboard and
drop-down lists keep a dark background to avoid overlapping texts.
The indicators and progress bars keep their colour.

The Diagnostic now contains the whole HWTest probe table, the export
and the RAM service with return to Hekate. See DIAGNOSTIC-INTEGRATION.md.

## Validation and limits

The complete build succeeded with devkitARM GCC 16.1.0.
Hekate, Nyx and the modules were generated. The sizes of the payload and of
the uncompressed Hekate respect the limits of the Makefile. The binary files
were compared to their linked ELF. The AArch64 Wi-Fi program and the separate RAM
service were also built. The saving of the settings passed 96 C cases
executed with a simulated SD card, including the write errors.

No trial on console has yet been carried out. The translations, fonts and
their memory management were checked on computer with the real C code.

For the hardware validation, check both layouts on the 1280 × 720 screen,
the shortcuts, the settings, the preservation of the choice after a reload,
the cancellation, the write errors and the touch/Joy-Con navigation.

See `DIAGNOSTIC-INTEGRATION.md` for the provenance and the scope of the probes.

See `PERSONNALISATION.md` for the SD fonts, the sizes and the effects;
`TRADUCTIONS.md` describes the current limits of the translation of the reports.
