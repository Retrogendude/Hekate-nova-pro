# Hekate Nova Pro v33.1 beta

- Launch tile: the small /icone/launch.png icon is independent of the large /icone/Launcher.png background; replace the latter in Nova with a matching preview.
- Credits: HWTest-RCM 0.15 is attributed to Creased (Baptiste MOINE) under Free software.
- Appearance and options: removed the row backgrounds for backup verification and selection effects; matched the System options panel outline.
- emuMMC backup and restore: the active outline belongs to the outer panel, removing the offset double frame while preserving button behavior.
- Separate standard and experimental 8GB packs for physically upgraded consoles: Erista V1 and Mariko V2/Lite/OLED. The latter uses the official Hekate software flag and does not program fuses.
- RAM diagnostic preserves the 8GB mode on entry and return, selects training profiles by platform and capacity, and stops if capacity changes after training.
- High addresses use 64-bit calculations and results report actual capacity: 8,176 MiB testable out of 8,192 MiB, with 16 MiB protected for working data.
- Atmosphère 1.11.0 or newer is required for memory modes above 4GB: memmode=1 in an fss0 Hekate entry, or enable_mem_mode=1 in exosphere.ini for fusee. Existing configurations are not edited automatically.

Beta: ARM builds and PC checks passed. The 8GB mode, especially the adapted Erista profile, still requires hardware validation on each platform; compatibility with every replacement chip is not certified. Long Chinese reports retain machine translations pending review.

## v33

- Simplified Chinese is now the eighth language in Settings and first-start setup.
- Chinese uses a required dedicated font. Personal font selection is preserved for other languages, while text size remains adjustable.
- Lecture.nyxf is the automatic font for Latin-script languages; Russian defaults to the built-in family. The original Hekate font is available in the selector.
- Includes three static wallpapers, five Nova icons and the supplied Launcher image for the main tile.
- Fresh-install defaults: French, Carbon Cyan, 75% panel and icon opacity, 30px text, the requested colors and a light startup logo. The original clock setup remains console-specific.
- Clearly marked boot options: Auto Boot, delay, Auto NoGC and Auto HOS Power Off. Backlight links to the existing setting without a second slider.
- Paged Nova-style Auto Boot selector without vertical scrolling, preserving Hekate's main and extra configuration indices.
- Standalone memory test: Chinese UI with 16 × 16 glyphs and compatibility with the previous seven language packs.
- Readable English fallback if the Chinese font resource is missing or invalid, preserving the personal font. Existing saved settings are not overwritten by the bundle.
