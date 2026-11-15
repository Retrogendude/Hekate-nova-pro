# Hekate Nova Pro — v14 reflection
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V14.md) · **English** · [Español](CORRECTIONS-NOVA-PRO-V14.es.md) · [Русский](CORRECTIONS-NOVA-PRO-V14.ru.md)

The white reflection still crosses the whole tile, with a fixed border. This update makes it wider, softer at the edges and faster.

- Width of the band: 129 pixels, against 45 pixels in v13, that is about three times wider.
- Transparency: continuous rounded profile, light white at the centre then completely transparent at both ends. The low opacities remain present; they are no longer cut by the threshold of the uniform fill of LVGL.
- Pass: about 0.8 second, against 1.2 seconds in v13.
- Rest: about 2.4 seconds. The complete cycle lasts 3.2 seconds.

The drawing uses a single gradient line of 516 bytes, reused at each height. It does not create an image the size of the tile and allocates nothing during the animation. Six small zones follow the old and new positions of the reflection. The effect keeps a low priority and goes on hold during the interactions and the diagnostics. The Selection effects setting still makes it possible to disable it.

Only gui_nova_reflection.c changed compared with the sources delivered in v13. The fix for access to the menus with a wallpaper, the original time setting with the language and the saved settings are preserved.

The checks of this update cover 99 native views with background and transparency, 21 navigation sequences by presses/releases in the seven languages, and the pixel rendering of the gradient on a plain background then on a wallpaper. The profile is compared to a reference transparency curve: width, symmetry, transparency at the ends, low opacities and limits of the tile. The ARM binaries are compared to their ELF, their sizes are checked and the 42 probes are still linked. The Hekate boot and the RAM companion are identical to v13. The complete v13 report is supplied separately as history; its 4 407 views are not counted as trials replayed here.

The previews and the GIF come from the real LVGL rendering. The measurements of redrawn zones are made on PC, not in FPS on the console. **The smoothness on Switch still has to be confirmed on the device.**

To install, back up the current files, merge the bootloader folder and replace bootloader/sys/nyx.bin with the one from v14. Use the payload supplied with the usual method. No INI is included; no new software or SDK is necessary to use this version.
