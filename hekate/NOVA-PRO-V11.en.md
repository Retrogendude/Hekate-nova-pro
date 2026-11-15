# Hekate Nova Pro — version 11
**Languages:** [Français](NOVA-PRO-V11.md) · **English** · [Español](NOVA-PRO-V11.es.md) · [Русский](NOVA-PRO-V11.ru.md)

This version applies the twelve changes recorded in MODIFICATIONS-NOVA-PRO.md. The Nova mock-up remains the base, with the expressly requested changes: direct commands, reflection, removal of the badges, new interactions and customisation.

The tabs occupy the top bar; the windows remain between 76 and 656 pixels, above the status banner. The commands of the top bar are unavailable during the opening of a window, then return to their state when it closes. The cards keep 8-pixel corners, an anthracite palette and thin pictograms. The backup/restore, USB, emuMMC, diagnostic, appearance and credits pages use these components. The technical reports remain viewable in their Nova window.

The first press on a Tools, Info or Diagnostic tile selects it. The second press on that same tile opens its function. A pre-selected tile also requires two presses after the opening of the menu. The special actions by long press, the confirmations and the restrictions of the original functions are preserved.

The reflection is drawn every 80 ms on small segments of the visible outlines. Only the zones corresponding to its old and new positions are refreshed. The masked surfaces are ignored; the reflection stops during the diagnostics and the interactions. The waves, particles and auroras have been removed from the code and from the interface. The historical field nova_halo now controls the reflection.

The background, the text, the outlines and the reflection have four independent colours. Two opacity settings concern only the menu backgrounds and the icon frames. The editor presents a preview, RGB values and samples. Closing cancels the choices; Save and reload applies them. The language and font settings remain distinct.

The wallpapers are fixed, in non-compressed 24/32-bit BMP format, in /wallpapers. The decoder checks the dimensions, the sizes, the orientation and the accesses to the file. The cropping and the scaling take place at loading, without SD access during the drawing. A single source line and the final image are allocated. The final 1280 × 720 image occupies about 3.5 MiB only when a wallpaper is chosen. The converter supplied prepares the personal PNG/JPG images; Nova-Carbon is delivered ready to choose.

The boot presents the original Nova Pro monogram and the name Hekate Nova Pro. The new animation uses a bar and three dots; the boot delay and the cancellation by VOL− are preserved. The new payload supplied must be launched to benefit from this presentation. The loader is rebuilt when the compressed Hekate data change; the final check decompresses the real payload and compares its content with the current binary.

The build and the checks concern the software. The renderings use the real LVGL with the hardware actions replaced by demonstration values. They do not constitute a measurement of smoothness on console. The physical validation on Switch remains necessary, notably for the diagnostics and the return of the RAM service.
