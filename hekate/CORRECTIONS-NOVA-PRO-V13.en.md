# Hekate Nova Pro — v13 fix
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V13.md) · **English** · [Español](CORRECTIONS-NOVA-PRO-V13.es.md) · [Русский](CORRECTIONS-NOVA-PRO-V13.ru.md)

This version fixes the blocking of the menus after activating a wallpaper and replaces the reflection of the symbol with a reflection over the whole selected tile.

## Wallpaper and access to the windows

The defect was reproduced before the fix in the real LVGL code: after opening the Colours window with an active wallpaper, the first element of the list of children had a predecessor. The content of the window was no longer accessible by the traversal used to find the touched button, although the drawing could still traverse it in the other direction. The old image trials did not check this invariant.

The function lv_ll_move_before now also updates the head of the list when an element moves in front of the first one. This is the case when placing the content of a window in front of its background. The wallpapers remain protected, non-interactive and behind the content. The fix concerns the internal order of the objects and their access to touch.

The added checks verify the consistency of the two traversal directions over the whole tree of the windows. A test of the real C function compares 316 646 moves with an independent model. Other trials send real presses and releases to the LVGL input driver: opening Nyx, Colours, choosing a colour, closing, opening Appearance, closing then switching each of the main tabs. Each sequence is repeated three times, in the seven languages, at 20, 30 and 36 pixels.

## Reflection over the whole tile

A diagonal white band crosses the whole surface of the selected tile: background, pictogram and text. It does not run over the outlines; the border stays fixed. It applies to the home screen tiles, to the carousel and to the selections of the Tools, Info and Diagnostic grids.

The pass lasts about 1.2 seconds, followed by about 2.4 seconds of rest. The white stays light, with several levels of transparency. The drawing is done directly in small bands, without a transparent image the size of the tile and without allocation during the animation. Four zones follow the old and the new position of the reflection to avoid systematically redrawing the whole tile or the whole screen. The effect goes on hold during the interactions and the diagnostics. The Selection effects button makes it possible to cut it.

## Installation and preservation of the settings

Merge the bootloader folder of the archive with that of the SD card after backing up. Use the payload hekate_nova_pro_6.5.4_v13.bin supplied. The new file bootloader/sys/nyx.bin contains the fixes; do not keep the one from v12. No nyx.ini or hekate_ipl.ini file is delivered. The colours, the wallpaper, the language and the fonts already configured are preserved. The original time setting with the added language remains unchanged. It is not necessary to install another program to use this version.

## Validation

devkitARM build and checks of the real code on PC, with the fonts, the wallpapers and the transparencies. The trials include the access to the buttons, the openings and closings of overlapping windows, the saves, the binary limits and the 42 diagnostic probes. The Hekate boot and the RAM companion remain those of v12; their binaries are compared during the preparation of the delivery.

The detailed results are in verification/. The previews and the GIF come from the real LVGL rendering. **No trial of this version on Switch nor measurement of console FPS has been carried out here.** Check on the device the access to the settings with the wallpaper active and the smoothness of the reflection.
