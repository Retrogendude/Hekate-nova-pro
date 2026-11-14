# Nova30 graphics audit — 13 November 2026
**Languages:** [Français](AUDIT-NOVA30-GUI.md) · **English** · [Español](AUDIT-NOVA30-GUI.es.md) · [Русский](AUDIT-NOVA30-GUI.ru.md)

The selected reflection produced invalidations every 20 ms, whereas
the LVGL refresh occurs every 33 ms. The queue contains 32 zones.
Two phases could accumulate before their consumption and cause the
LVGL fallback to the whole screen. The previous rendering test emptied this queue
with `lv_refr_now()` after each phase; it did not reproduce this case.

The fix in `frontend/gui_nova_reflection.c` calculates the number of zones
of a transition before insertion. If they do not fit in the global
queue, the decorative transition waits; the invalidations of the other objects
remain intact. The next transition joins the real-clock phase,
without replaying a list of intermediate states. The targets are traversed in
alternation. The white gradient, the border and the dimensions remain identical.

`check_nova30_reflection_timing.py` uses the real LVGL objects and drawings,
an instrumented copy of `lv_refr.c` to count the overflows, and
33/66/100 ms refreshes. It compares the real v29 effect with the new effect,
without changing the distributed LVGL. The pixels at phases 0/5/16 are identical.
After a stabilised pause, no decorative pixel is redrawn.

Results over a simulated 8.48 s cycle, 33 ms refresh:

| Scene | Pixels redrawn v29 | Pixels redrawn v30 | Reduction | Whole screens v29 → v30 |
| --- | ---: | ---: | ---: | ---: |
| Home screen | 8 073 993 | 3 868 461 | 52.1 % | 5 → 0 |
| Tools | 6 106 329 | 1 605 560 | 73.7 % | 5 → 0 |
| Carousel | 4 946 025 | 2 306 984 | 53.4 % | 3 → 0 |

The 9 fixed cases (3 scenes × 3 cadences) have no periodic overflow,
with a peak of 28 zones. With 30 user invalidations already present,
the effect keeps these 30 zones and does not cause their removal by overflow.
A window opening, a pause or another producer can still
request a complete drawing; this fix targets the periodic producer.

These numbers are surfaces drawn on PC, not FPS nor a measurement of
time on Switch. The intermediate phases can be skipped when the
refresh is busy. The hardware cadence and the visual settings of
Nova are preserved.

Other leads to measure afterwards: decoding of the custom PNGs and scans of
fonts during the reconstructions, cost of the large transparent surfaces,
refresh of the statistics and TV presentation time. A bounded cache
can help, but it must preserve the replacement of the SD files and the
release of resources during the language/font/theme changes.

The v30 keeps the interface tests with 3 fonts, 7 languages and 2 sizes,
including the new automatic start message.
