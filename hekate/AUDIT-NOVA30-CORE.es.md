# Auditoría Nova Pro v30 — núcleo, arranque, almacenamiento y diagnósticos
**Languages:** [Français](AUDIT-NOVA30-CORE.md) · [English](AUDIT-NOVA30-CORE.en.md) · **Español** · [Русский](AUDIT-NOVA30-CORE.ru.md)

Auditoría estática de solo lectura del 13 de noviembre de 2026, sobre `work/hekate-nova-pro-v30`. Ningún archivo fuente modificado. Las referencias de líneas describen la versión leída durante la auditoría; el padre trabaja simultáneamente en el arranque. La GUI general y el Bluetooth son objeto de auditorías distintas.

Los resultados de hardware v29 anunciados como concluyentes se conservan como información proporcionada, sin pretender reproducirlos. No se ha medido aquí ninguna ganancia de FPS, de caudal SD/USB o de tiempo de hardware. Las duraciones siguientes provienen de los retardos y bucles explícitos del código.

## Prioridades

| Prioridad | Tema | Efecto demostrable | Alcance recomendado |
|---|---|---|---|
| P1 | Tamaños de los archivos binarios y `res.pak` | Acceso fuera del archivo o fuera de la región reservada con un archivo inválido | Validar antes de asignar/leer/ejecutar |
| P1 | Escritura SD señalada como correcta a pesar del fallo | Exportaciones incompletas presentadas como guardadas | Verificar cuenta escrita y cierre |
| P1 | Límites de tamaño de build no bloqueantes | Un binario fuera de capacidad puede entregarse | Hacer fallar el build, sin engordar el loader |
| P2 | Fuga del payload rechazado en Nyx | Consumo repetido del heap, sin limitación | Tamaño antes de la asignación, limpieza única |
| P2 | 1,1 s de animación añadida al arranque | Espera de software impuesta, incluso si el trabajo ya terminó | Animación sobre el trabajo, conservar aspecto y retardos de hardware |
| P2 | Diagnósticos largos sin refresco periódico | Impresión de congelamiento durante varios segundos | Heartbeat de visualización únicamente, cadenciado |
| P2 | Etapas TV todavía bloqueantes | Tarea GUI retenida durante espera/negociación | Estados temporizados, progreso y cancelación entre fases seguras |
| P3 | Asignaciones de traducción tras truncamiento del informe | Trabajo sin resultado conservado | Evitar la traducción cuando la escritura ya es imposible |

P1 designa aquí una fiabilidad prioritaria, no un incidente de hardware constatado. Los cambios BDK comunes tienen una superficie de regresión mayor que las correcciones locales; aislarlos y verificar cada llamador.

## 1. Archivos de arranque: limitar y verificar antes de usar — P1

**Pruebas.** `bootloader/main.c:585` lee `bootloader/sys/nyx.bin` con `sd_file_read(..., NULL)`. La lectura de la palabra de versión en `main.c:596` supone al menos `NYX_VER_OFF + 4` bytes (`NYX_VER_OFF = 0x9C`, línea 580). La dirección cargada se convierte después en una entrada ejecutada, líneas 656–657. `bdk/storage/sd.c:258–272` asigna todo el tamaño indicado por el archivo, sin límite de negocio; `f_read` no verifica la cuenta leída y `f_close` se ignora.

`nyx/nyx_gui/nyx.c:400–413` lee todo el tamaño de `bootloader/sys/res.pak` directamente en `NYX_RES_ADDR`. La región reservada es solo `NYX_RES_SZ = SZ_16M` (`bdk/memory_map.h:82–83`). Un tamaño excesivo escribe más allá de esta región. Un tamaño insuficiente puede dejar recursos ausentes o antiguos sin que esta función falle.

**Corrección comprobable.** Pedir el tamaño de Nyx, rechazar una imagen demasiado pequeña e imponer un límite coherente con su región/formato antes de la lectura y antes del salto. Para `res.pak`, verificar el tamaño esperado o como mínimo los límites del formato, y luego la cuenta realmente leída y el retorno del cierre. No considerar una simple comparación de versión como validación del formato. Usar helpers locales o ampliar sobriamente la lectura limitada ya presente: `_read_service_payload` en `nyx.c:168–198` valida el tamaño antes de la asignación y verifica la lectura completa y el cierre. Evitar duplicar una infraestructura pesada en el loader cerca de su límite.

**Validación.** Archivos ausentes, vacíos, de 1 byte, `0x9F`, tamaño válido, límite exacto, límite + 1; lectura corta con retorno `FR_OK`, error de lectura/cierre; centinelas antes/después de la región res. El resultado esperado es un rechazo explícito antes del acceso a la versión, el salto o el desbordamiento. Verificar después un arranque de hardware con los archivos v29/v30 válidos.

**Límite.** Auditoría del control de tamaño y del contrato de entrada, no verificación criptográfica de Nyx ni del formato completo de `res.pak`.

## 2. `sd_save_to_file` devuelve un falso éxito — P1

**Pruebas.** Tras una apertura correcta, `bdk/storage/sd.c:291–294` ignora `f_write`, el número de bytes escritos y `f_close`, y luego devuelve cero. Los llamadores usan ese cero como éxito: dumps `nyx/nyx_gui/frontend/gui_info.c:84,108,138`, capturas `frontend/gui.c:300`, extracción `frontend/gui_tools.c:1006,1152`, exportación Joy-Con `frontend/gui_options.c:758`, registro fatal `bootloader/hos/secmon_exo.c:418`.

**Impacto.** Tarjeta llena, retirada de tarjeta o error diferido en el cierre: un archivo parcial puede anunciarse como guardado. No es una ganancia de velocidad; evita reanudaciones inútiles y hace explotables los fallos.

**Corrección comprobable.** Recoger `UINT written`, verificar `FRESULT` y `written == size`, cerrar siempre un archivo abierto, propagar el error de cierre cuando la escritura haya tenido éxito. Elegir un retorno de error existente coherente para la escritura parcial. No modificar implícitamente las políticas de sobrescritura o de borrado de las exportaciones.

**Validación.** Sustituciones FatFS: escritura completa, parcial + `FR_OK`, error de write, error de close, apertura rechazada; cada ruta cierra exactamente una vez si es necesario y solo los archivos completos cerrados con éxito devuelven cero. El diagnóstico nativo ya da un buen ejemplo local en `nyx_diagnostics.c:346–357`, que verifica la cuenta escrita y el cierre.

## 3. Límites del binario: hacerlos bloqueantes — P1

**Pruebas.** `Makefile:108` y `112` solo muestran un aviso más allá de respectivamente 140288 y 126296 bytes. Los comandos se realizan correctamente a pesar de una superación. El padre anuncia actualmente 139273 bytes brutos: margen declarado de 1015 bytes, valor no reconstruido de forma independiente durante esta auditoría.

**Corrección comprobable.** Hacer fallar la generación/entrega al superar los dos límites, conservar sus valores existentes, controlar también el tamaño de los artefactos entregados. Esta guarda no consume ningún byte del payload. Evaluar el tamaño tras cada corrección BDK compilada en Hekate; privilegiar las correcciones locales frente al código genérico voluminoso.

**Validación.** Verificar límite − 1, límite, límite + 1: los dos primeros casos pasan, el último falla realmente. Rehacer una compilación incremental y limpia. El loader ya tiene la dependencia explícita de `loader.o` con las dos cabeceras de payload y con `Versions.inc` (`loader/Makefile:67–71`): conservarla.

**A preservar.** `loader/loader.c:80–105` reubica y luego descomprime dos bloques embebidos de tamaño de compilación conocido. Ningún beneficio concreto justifica aquí añadir caché, nuevo codec o representación adicional en esta etapa. No se ha medido ningún coste de descompresión.

## 4. Fuga y asignación antes de la validación de un payload Nyx — P2

**Pruebas.** `nyx/nyx_gui/nyx.c:133–145` carga un payload con `sd_file_read`, y luego rechaza un tamaño superior a `0x30000`. La ruta `out` (`nyx.c:164–168`) desmonta la tarjeta pero no libera `buf`. La versión bootloader tiene, en cambio, `free(buf)` en `bootloader/main.c:213`. Un payload sobredimensionado puede así consumir su tamaño en cada nuevo intento en Nyx.

**Corrección comprobable.** Verificar el tamaño del payload antes de la asignación, conservar un puntero inicializado a `NULL` en toda la función y una limpieza única para cada salida que vuelve a Nyx. Una imagen de cero bytes debe rechazarse antes del lanzamiento. Tener en cuenta las salidas incluso antes de que `buf` se haya declarado en la función actual.

**Validación.** Cien rechazos sucesivos del mismo payload > `0x30000`: ocupación del heap estable y cero salto de ejecución; éxito y luego retorno simulado; ruta sin nombre de archivo; archivo ausente y lectura fallida. El contador debe distinguir memoria viva y memoria solo reservada por el heap.

**Contexto del heap.** `bdk/mem/heap.c:39–112` busca linealmente un bloque libre y luego extiende el heap sin límite máximo; `calloc` multiplica sin guarda (`heap.c:179–181`). El mapa de memoria reserva sin embargo 256 MiB (`memory_map.h:52–53`). Por lo tanto, las pruebas `if (!malloc(...))` no proporcionan un control fiable de agotamiento. Priorizar los tamaños de entrada y la fuga local. Una limitación global del heap requeriría un cambio de contrato y la auditoría de las llamadas `zalloc`/`calloc`, que hacen `memset` sin verificar un retorno nulo (`heap.c:172–181`): no es una modificación aislada que deba hacerse con prisa.

## 5. Arranque: cubrir la animación con los trabajos — P2

**Pruebas.** `nova_splash_animate` (`bdk/display/nova_splash.c:46–57`) realiza todas sus etapas y esperas incluso cuando `from == to`. Puede precisamente retomar `from` en el progreso ya alcanzado, línea 48, conservando la duración total. La ruta normal añade animaciones de 300 + 200 ms en `bootloader/main.c:590,655` y luego 300 + 300 ms en `nyx/nyx_gui/nyx.c:575,619`, es decir 1100 ms de duración solicitada, sin contar el coste de la representación y la granularidad de los temporizadores.

**Corrección comprobable.** Conservar el splash, sus colores y sus etapas pero animarlo durante las lecturas/inicializaciones. Terminar inmediatamente o con una breve recuperación limitada cuando el trabajo de la etapa haya terminado realmente. El mecanismo de techo existente (`nova_splash.c:29–31`) evita anunciar un trabajo terminado demasiado pronto: conservarlo. Las nuevas fases de carga `.bin` del padre deben usar la misma lógica.

**Validación.** Medir por separado la lectura de Nyx, la configuración, los recursos y la primera imagen interactiva; tarjeta rápida y lenta; splash personalizado; entrada Nyx tras el retorno de RAM. Una sustitución del timer puede verificar que una etapa ya terminada no espera ya 300 ms. Los 1100 ms son un presupuesto de software identificable, no una ganancia de reloj garantizada de 1100 ms en consola.

**A preservar imperativamente.** La espera de ciclo de alimentación SD (`main.c:652–653`), la rampa de retroiluminación, el training Minerva y `nova_splash_pause()` antes del training (`main.c:645–647`). No acelerar los retardos eléctricos para ganar tiempo aparente. Los hooks de timer ejecutan representación desde `get_tmr_ms/us` (`bdk/soc/timer.c:39–51`); no ampliarlos sin límite ni reactivarlos durante mediciones sensibles.

## 6. Diagnósticos: progreso regular sin perturbar las mediciones — P2

**Pruebas.** El adaptador `nyx_diagnostics.c:132` define `msleep_poll(ms)` como un simple `msleep(ms)`, al contrario del sentido aparente del nombre. La callback de visualización solo la llama `status_set` / `status_step` (`nyx_diagnostics.c:133–143`) y en la entrada de cada sonda (`:261`). La callback GUI refresca inmediatamente (`frontend/gui_diagnostics.c:261–269`); no hay heartbeat durante las esperas.

Ejemplos: ventilador 600 ms de kick, 2000 ms de spin-up y luego la medición BDK que puede esperar 2 s (`diagnostics/vendor/probe_power.c:806,813,974–993`, `bdk/thermal/fan.c:86–96`) y medición propia de 1 s. El Wi-Fi espera hasta 40000 iteraciones de 500 µs por intento, con hasta tres intentos (`vendor/probe_wifi.c:501–541`). Las ramas de salida pueden acortar estos techos; no hay que anunciar sistemáticamente 60 s.

**Corrección comprobable.** Añadir un refresco de estado cadenciado a baja frecuencia a las esperas largas y a los límites seguros de las fases, con nombre de fase/tiempo transcurrido. Respetar los periodos de muestreo sin representación; no acortar las ventanas de medición para embellecer la duración. `manual_system_maintenance` ya sabe limitarse al refresco cuando `diagnostics_suspended` es verdadero (`frontend/gui.c:784–791`): un punto de integración posible a examinar sin importar el frontend en el código vendor.

**Riesgos.** No lanzar `lv_task_handler()` ni reactivar sin distinción táctil, raíles, Bluetooth, SD o tareas de hardware desde una sonda. Los diagnósticos tienen snapshot/restauración y suspensión de las entradas; cualquier cancelación futura debe pasar por la misma limpieza. El texto de usuario anuncia actualmente de forma explícita los comandos suspendidos (`gui_diagnostics.c:284`): así pues, la ausencia de interactividad no se describe como un botón Cancelar roto.

**Validación.** Timer simulado: número de refrescos limitado, ningún acceso a los periféricos suspendidos, misma duración/orden de las mediciones y mismo veredicto; limpieza/restauración ejecutada una vez tras éxito y error. Luego comparación de los resultados de hardware con la referencia v29.

## 7. TV: conservar los retardos, hacer las etapas cooperativas — P2

**Pruebas.** La negociación es mayoritariamente una máquina de estados, pero `bdk/display/tv/usbpd.c:957–958` espera todavía de forma síncrona el final de un retardo de 100 ms, y `:1032` espera 250 ms para `ND_BUSY`. La tarea GUI llama a `dock_step` y luego, cuando la dock está lista, a `dpout_aux_init`, `dpout_probe_sink` y `dpout_video_start` de forma síncrona (`frontend/gui_tv.c:329,354–365`). Los bucles de training DisplayPort, los repliegues de velocidad y los ensayos de margen tienen varias esperas (`bdk/display/tv/dpout.c:1817–1860`). El límite de sesión de 45 s solo se evalúa en la entrada de `_tick` (`gui_tv.c:321`), no durante estas llamadas.

**Corrección comprobable.** Transformar las esperas de 100/250 ms en estados con vencimiento, sin cambiar su duración. Capturar un único valor `elapsed` en lugar de recalcular entre condición y sustracción en `usbpd.c:957–958`. Dividir lectura sink, training y margen en fases limitadas solo si se identifican pausas seguras; mostrar la etapa y verificar desconexión/cancelación entre estas fases. Empezar por los estados PD simples, menos arriesgados que la secuenciación SOR/PLL.

**Validación.** Reloj simulado: mismo orden de comandos, ninguna llamada antes de su retardo, `ND_BUSY` hasta su límite, retirada HPD, retorno a un estado limpio, restauración de reloj y raíles. La estabilidad de vídeo, la negociación de docks variadas y los timings PLL siguen siendo pruebas de consola. No proponer una ganancia de FPS TV: salida configurada a 720p/60 Hz y cadencia efectiva de la interfaz son mediciones distintas.

## 8. Informe de diagnóstico: evitar el trabajo tras el truncamiento — P3

**Pruebas.** `append` abandona si el informe está silenciado o truncado (`nyx_diagnostics.c:77–90`). Sin embargo, `_log_emit` traduce y asigna siempre la línea antes de llamarla (`:98–101`). Tras la saturación del informe de 256 KiB (`:29`), las sondas habladoras continúan así las búsquedas de traducción y las asignaciones liberadas sin texto añadido. La traducción puede construir recursivamente una nueva cadena (`localization.c:235–251`).

**Corrección comprobable.** Conservar en primer lugar el registro de gravedad en `_log_emit`, y luego cortocircuitar solo la traducción/append cuando ninguna salida sea consumible (`muted`, `truncated` o informe ausente según contrato). No saltar los descubrimientos/veredictos. Evitar una caché de todas las líneas: los valores variables tendrían poca reutilización y aumentarían el heap.

**Validación.** Informe saturado: mismo marcador de truncamiento, mismos resultados y contador de findings, cero traducción adicional esperada tras el truncamiento; caso no truncado idéntico en los siete idiomas. Ganancia de CPU no cuantificada antes de la medición.

## USB, SD y memoria: decisiones conservadoras

El servicio UMS ya tiene mantenimiento periódico y training (`bdk/usb/usb_gadget_ums.c:1794–1810,1914`), selección de controlador Erista/Mariko y búferes fijos. Ningún elemento leído justifica engordar las transferencias o aumentar las frecuencias arbitrariamente. Las escrituras UMS y las cachés exigen verdaderas pruebas host/tarjeta; las sustituciones en PC no demuestran ni caudal ni integridad tras una retirada brutal.

Las fechas UMS usan comparaciones absolutas `timer < time` (`:1801,1806`) en lugar de un cálculo de tiempo transcurrido. Una sesión larga que atraviese el desbordamiento del contador de milisegundos puede retrasar el mantenimiento. Es una mejora de robustez secundaria: vencimientos seguros en el rollover y pruebas cerca de `UINT32_MAX`, sin acortar los intervalos existentes.

La SD conserva sus repliegues de velocidad y reintentos (`bdk/storage/sd.c:93–179`); retirar estos repliegues o aumentar el modo máximo en nombre del rendimiento comprometería las tarjetas marginales. Una optimización útil debe separar tiempo de inicialización, caudal secuencial, archivos pequeños y tiempo de representación; un caudal de lectura global no prueba una mejora del arranque.

## Validación y límites de esta auditoría

Lectura de las fuentes y de la cadena de compilación únicamente; ninguna prueba de hardware, benchmark, compilación o ejecución de las sustituciones v29 realizada por esta auditoría. Los scripts `work/check_nova29_*.py` inspeccionados conciernen a la referencia v29 y varios apuntan explícitamente al directorio `hekate-nova-pro-v29`. Lanzarlos sin cambios no validaría las modificaciones v30. Para cualquier corrección futura, apuntar explícitamente al árbol correcto y consignar los hashes de las fuentes probadas.

Orden recomendado: guardas de build; rechazo de archivos inválidos y contrato de escritura SD; fuga local; recubrimiento de las animaciones; heartbeat diagnósticos; estados TV simples. Reservar la evolución global del heap y la división del training de vídeo a cambios independientes. Para cada etapa, conservar las funcionalidades Nova, los recursos personalizados, los idiomas, el comportamiento del retorno de RAM y los resultados de hardware v29.

Validación post-auditoría: correcciones locales de nyx.c integradas y check_nova30_core_io.py ejecutado por el padre; 20 casos v30 pasan y 8 regresiones v29 reales se reproducen. Ninguna modificación de los helpers BDK comunes ni del loader en esta versión.
