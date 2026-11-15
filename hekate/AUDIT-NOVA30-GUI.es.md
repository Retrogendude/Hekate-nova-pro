# Auditoría gráfica Nova30 — 13 de noviembre de 2026
**Languages:** [Français](AUDIT-NOVA30-GUI.md) · [English](AUDIT-NOVA30-GUI.en.md) · **Español** · [Русский](AUDIT-NOVA30-GUI.ru.md)

El reflejo seleccionado producía invalidaciones cada 20 ms, mientras que
el refresco LVGL interviene cada 33 ms. La cola contiene 32 zonas.
Dos fases podían acumularse antes de su consumo y provocar el
repliegue de LVGL a la pantalla completa. La prueba de representación anterior vaciaba esta cola
con `lv_refr_now()` tras cada fase; no reproducía este caso.

La corrección en `frontend/gui_nova_reflection.c` calcula el número de zonas
de una transición antes de la inserción. Si no caben en la cola
global, la transición decorativa espera; las invalidaciones de los demás objetos
siguen intactas. La siguiente transición se une a la fase del reloj real,
sin reproducir una lista de estados intermedios. Los objetivos se recorren en
alternancia. El degradado blanco, el borde y las dimensiones siguen idénticos.

`check_nova30_reflection_timing.py` usa los objetos y dibujos LVGL reales,
una copia instrumentada de `lv_refr.c` para contar los desbordamientos, y
refrescos de 33/66/100 ms. Compara el efecto real v29 con el nuevo efecto,
sin cambiar el LVGL distribuido. Los píxeles en las fases 0/5/16 son idénticos.
Tras una pausa estabilizada, no se redibuja ningún píxel decorativo.

Resultados en un ciclo simulado de 8,48 s, refresco de 33 ms:

| Escena | Píxeles redibujados v29 | Píxeles redibujados v30 | Reducción | Pantallas completas v29 → v30 |
| --- | ---: | ---: | ---: | ---: |
| Pantalla de inicio | 8 073 993 | 3 868 461 | 52,1 % | 5 → 0 |
| Herramientas | 6 106 329 | 1 605 560 | 73,7 % | 5 → 0 |
| Carrusel | 4 946 025 | 2 306 984 | 53,4 % | 3 → 0 |

Los 9 casos corregidos (3 escenas × 3 cadencias) no tienen ningún desbordamiento periódico,
con un pico de 28 zonas. Con 30 invalidaciones de usuario ya presentes,
el efecto conserva estas 30 zonas y no provoca su supresión por desbordamiento.
Una apertura de ventana, una pausa u otro productor siempre puede
pedir un dibujado completo; esta corrección apunta al productor periódico.

Estos números son superficies dibujadas en PC, no FPS ni una medida de
tiempo en Switch. Las fases intermedias pueden saltarse cuando el
refresco está ocupado. La cadencia de hardware y los ajustes visuales de
Nova se conservan.

Otras vías a medir después: decodificación de los PNG personalizados y escaneos de
fuentes durante las reconstrucciones, coste de las grandes superficies transparentes,
refresco de las estadísticas y tiempo de presentación TV. Una caché limitada
puede ayudar, pero debe conservar la sustitución de los archivos SD y la
liberación de recursos en los cambios de idioma/fuente/tema.

La v30 conserva las pruebas de interfaz con 3 fuentes, 7 idiomas y 2 tamaños,
incluido el nuevo mensaje de arranque automático.
