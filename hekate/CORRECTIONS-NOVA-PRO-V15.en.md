# Hekate Nova Pro — v15 fixes
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V15.md) · **English** · [Español](CORRECTIONS-NOVA-PRO-V15.es.md) · [Русский](CORRECTIONS-NOVA-PRO-V15.ru.md)

The eleven points of the list have been dealt with in the sources and integrated into the build. The checks below are carried out on PC with the LVGL engine of the project. The accesses to the hardware are replaced by test data in these previews; they do not constitute a backup, a partitioning or a diagnostic executed on a Switch.

1. **Report font fixed.** The resizing used the width of the fixed cell as the bitmap width of each character. The narrow glyphs were read with a wrong stride, which reproduced the blurry characters from the screenshots. The reader now uses the real width of the bitmap. The glyphs are centred in a fixed cell, including at the native size of 20 px.
2. **eMMC information realigned.** Each label and its value share a line height calculated according to their text. The properties and the partitions are split over two tabs. The partitions use Previous/Next pages; the table no longer stops after the first twenty rows.
3. **Backup and restore translated.** The title, the free space, the size, the folder and the file go through the catalogue of the seven languages. The fragments added to the log also go through the translation: end, failure, cancellation and errors. The paths, file names, numbers and addresses are preserved.
4. **Technical characters checked.** The digits, ASCII letters and Cyrillic letters are checked from the real bitmaps. The accents and the seven languages are checked in the technical views. The family chosen for the menus remains independent of the fixed report font.
5. **Screenshot views reproduced.** The eMMC information, the backup progress and the MBR warning are included in the language, size and font trials, as well as in the wallpaper and transparency trials. The data displayed in the previews are simulated. The log remains in a bounded zone; the long tables are paginated.
6. **Cumulative colour preview.** The provisional colours of the background, texts, outlines, logo, icons and selection remain present together. The logo and the settings icon each have their own colour. Changing target or tab no longer resets the preview. Save applies the draft; closing without saving keeps the previous settings.
7. **Accelerated reflection.** The band crosses the whole tile in about **0.48 second**, against 0.8 second in v14. It keeps its width of 129 px, its transparent ends and its fixed border. Ten redraw bands bound the work. The 2.4-second pause and the suspension during the interactions and diagnostics are preserved. No allocation is made on each frame.
8. **Progressive loading.** The bar advances to 100 % in six animated steps, with 1.8 seconds of animation in total, in addition to the real loading time. The blocking accesses to the SD card can still add a pause between these steps. The self-start delay, the cancellation by VOL− and the initial original time setting with language choice are preserved.
9. **emuMMC columns aligned.** The management panels share their upper and lower limits. Their layout no longer depends on the automatic placement of the old window.
10. **Choice of the emuMMC files without a vertical list.** Two columns of the same size, centred titles and four choices per page replace the drop-down list. Previous/Next gives access to the following choices. The choice always saves the path of the original folder; the unavailable RAW partitions remain disabled.
11. **Usable area of the sub-menus.** The windows use the space between the top menu and the bottom banner. The centred boxes are recentred in this area, with adaptation of the width and, if necessary, of the text size. The eMMC reports, the progress and the fixed selectors have bounded dimensions.

## Checks

The matrix includes 40 views or tabs, the seven languages and nine sizes from 20 to 36 px with Lecture, then the three other families integrated, fixed Lecture and reinforced Lecture at the sizes 20, 30 and 36 px. Additional trials cover the wallpaper, three levels of transparency, a custom palette, the accessible commands and the navigation by presses/releases.

The checks also concern the preservation of the colour draft, the access to all the pages of the reports and of the emuMMC files, the links of the LVGL windows, the saving of the settings, the loading of the fonts and the fixed Caviar converter. The transparency profile of the reflection is compared to the pixels actually drawn. The boot checks verify the monotonic fill, the screen rotation, the delay and the cancellation.

The binaries are compared to the ELF executables and to the size limits. The 42 probes are still present, as well as the separate RAM service and its return mechanism. The logs and the exact numbers are included in `verification/`.

**To check on the console:** cold boot, legibility of the three views on the real data, opening of the settings with the chosen wallpaper, smoothness of the reflection and touch navigation. No FPS measurement on Switch nor real storage operation has been carried out here.

## Installation

Back up the current files of the SD card. Merge the `bootloader` folder supplied, then use the payload `hekate_nova_pro_6.5.4_v15.bin` with the usual method. `bootloader/update.bin` contains this same payload. Replacing Nyx and the payload is necessary to have all the loading modifications.

The `fonts`, `wallpapers` folders and the tools are included. No `nyx.ini` nor `hekate_ipl.ini` is supplied: the existing settings are not replaced. No new software is necessary to use the compiled package.
