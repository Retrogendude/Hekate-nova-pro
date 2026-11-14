# Current customisation — Nova Pro 11
**Languages:** [Français](PERSONNALISATION.md) · **English** · [Español](PERSONNALISATION.es.md) · [Русский](PERSONNALISATION.ru.md)

See [NOVA-PRO-V11.md](NOVA-PRO-V11.md) for the independent colours, the fixed wallpaper, the transparencies, the reflection and the sub-menus. The carousel, the seven languages and the SD fonts remain available. The waves, particles and auroras are removed; their descriptions below belong to the history.

---
## History — customisation before version 11

# Appearance, languages and reading — Nova, version 10

Open **Settings** in the top navigation. The language codes, the
Font line and the proposed sizes open the relevant selector directly.
The title **Appearance and reading** opens the detailed Nyx settings; then choose
**Appearance** for the other choices. The same screen groups
the language, the font, the text size, the home screen layout and the
background animations. **Save and reload** applies the choices. Closing
the window cancels the provisional choices.

## Fonts and sizes

Nine main sizes are offered, from 20 to 36 pixels. The captions
use a proportionate size and the tables keep a fixed-width
font. Nova keeps distinct roles for the titles, the commands
and the captions, proportionate to the chosen size and loaded only once.
The cards use a line break or an ellipsis; the
detail banner and the help give the full text. Very long choices can use the caption size
of the same font to stay
legible in their button.

Three ready-to-use fonts are supplied in `fonts/`, to be copied to the
root of the SD card: Lecture, reinforced Lecture and fixed Lecture. The menu
rereads this folder at the next loading of Nyx. Up to sixteen SD fonts
can be added. An invalid file uses the built-in font.

For a personal TTF or OTF font, use the converter supplied
in `outils-polices/`, then copy the resulting `.nyxf` file into `fonts/`.
The TTF/OTF are not read directly by the console. Python 3 and Pillow
are necessary only on the computer for this conversion.
The detailed instructions are in `documentation/POLICES.md`.

## Backgrounds and smoothness

The reference background is `#10151D`, the cards `#19212D`, the outlines
`#334050` and the accent `#55C9F1`. Nova takes the Lucide icons of the mock-up,
their thin strokes, the filled cards and the 8-pixel corners.
The 18-pixel halo stays fixed
around the selected icon: no timer animates it. The cards
display solid backgrounds, with cut-out icons. The custom SD
wallpaper is no longer drawn in this presentation; its file is
kept. The colour pills have their own fill, distinct from the
Nova cards. The first pill corresponds to the default blue accent.
**Selection effects** activates or deactivates the halo; save the options
to keep this choice.

The animation choices are Disabled, Waves, Particles and Aurora, with
speed and intensity. The effects are concentrated in a bottom band
of 64 pixels. They freeze during the use of the touch/Joy-Con and
resume after 900 ms of rest; the diagnostics also suspend them.
A single presentation of the image is performed per LVGL refresh,
even when several small zones change. The glyph searches
and the translations have been optimized without modifying the hardware measurements.

## Layouts

The Tiles home screen takes the large Start card and the four accesses of the
mock-up. The Carousel remains available. The main pages use the
top navigation and an area of 1 280 × 580 pixels above the status bar.
Tools and Info have four columns over two rows; Diagnostic presents
twelve cards over three rows, without vertical scrolling.
Touching a card selects its detail. The arrow of the bottom banner opens
the function; touching the banner text displays the full help.
Words that are too wide first use the caption size of the chosen
font. The two-state options have a switch and a written state;
Auto Boot continues to open the choice of the boot entry. It is
with the other boot options behind the title **System options**.
The HEKATE logo opens the quick commands and the Nyx settings; a long press
opens the credits.

Nyx Settings includes two internal tabs, Interface and System.
Appearance includes Language, Font, Layout and Effect. The font is
selected with the arrows; the other current choices are visible.
The detailed reports and the file lists can always scroll.

## Languages and data

The seven languages are English, French, Spanish, Italian, Russian,
German and Portuguese. The catalogue includes the labels, messages,
help and warnings of the interface as well as the listed diagnostic
texts. The measurements, component names, registers, paths and
boot entry names remain identifiable and are not renamed.
The detailed comments of the probes are now present in the
seven languages. Their complement was translated offline, with checks
on the values and the paths as well as terminology fixes.
A linguistic review of the automatic descriptions remains to be done;
the exact scope is described in `TRADUCTIONS.md`.

The RAM test launched from Nyx receives a small pack of texts and glyphs
in the memory zone already excluded from the test. The pack is checked before
use. If it is missing or corrupted, the service keeps its English
fallback display. The standalone boot and the early recovery
messages remain available without language resources.

The settings are added to `bootloader/nyx.ini`. No settings file
nor any boot entry is supplied in the archive. Merge the
`bootloader/` and `fonts/` folders with the existing SD card.

## Verification

The build and the computer checks are supplied in the archive.
The graphical checks cover 1 890 LVGL scenes: the seven languages,
the nine main sizes, the three fonts supplied and the built-in font.
Twelve additional scenes check the guards according to the availability of the
hardware or of a tool. The pixels of the cut-out icons are checked.
They check notably the configuration, the cancellation, the reading
errors, the font formats, the UTF-8 texts and the preserved commands.
They do not replace the trial on console: check the reading at 20 and
36 pixels, the seven languages, the effects and the touch/Joy-Con navigation.
