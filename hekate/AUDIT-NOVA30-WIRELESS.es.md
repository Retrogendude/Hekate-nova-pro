# Auditoría Nova 30 — entradas inalámbricas, fluidez y presupuestos
**Languages:** [Français](AUDIT-NOVA30-WIRELESS.md) · [English](AUDIT-NOVA30-WIRELESS.en.md) · **Español** · [Русский](AUDIT-NOVA30-WIRELESS.ru.md)

Auditoría local del 13 de noviembre de 2026. El usuario declara concluyentes
las pruebas físicas de la v29. Los resultados siguientes provienen de las fuentes y de pruebas nativas con radio/reloj
simulados. **No se ha medido ninguna latencia de consola ni de enlace RF.** El autorreinicio
al arranque lo trata por separado el padre.

Las rutas de fuentes son relativas a `work/hekate-nova-pro-v30/`.

## Resultados y modificaciones

### 1. Pulsaciones breves: defecto de software reproducible, corrección limitada

Antes de la corrección, cada informe sustituía `pad.buttons`. Si dos informes, pulsación y luego suelta, llegaban
antes de la siguiente lectura LVGL, el clic desaparecía. El contador `buttons_seen` conservaba una prueba de la pulsación,
sin devolverla a la entrada de la interfaz. El periodo nominal de lectura LVGL
es de 33 ms (`bdk/libs/lv_conf.h:115`); no garantiza una lectura cada 33 ms si otra tarea
consume tiempo.

Corrección en `nyx/nyx_gui/bluetooth/nova_bt_protocol.c:7`: los flancos ascendentes se conservan en
`pending_buttons`, con una marca temporal independiente por botón. Los informes simples `0x3f` y
completos `0x30/0x21` usan el mismo mecanismo (`:78`, `:83`). El estado mantenido, los sticks, la batería y el contador
de informes siguen siendo los del último informe.

La API `nova_bt_take_buttons(side)` (`nova_bt_stack.c:299`) devuelve los botones actualmente mantenidos más
los flancos de menos de 100 ms, y luego consume los flancos. La lectura siguiente
ve por tanto la suelta si el último informe está soltado. Cada botón caduca por separado: un flanco
reciente no rejuvenece un flanco antiguo. Las edades usan la resta sin signo para atravesar el
desbordamiento del reloj. Los flancos se suprimen mediante `clear_pad` (`:156`), por lo tanto por desconexión HCI,
cierre HID, stale a 500 ms y Stop. Una lectura stale también los consume sin devolverlos.

Integración requerida en el lado del padre: en cada iteración de los dos lados de
`gui_bluetooth_merge`, llamar a `uint32_t buttons=nova_bt_take_buttons(i);` antes del control de frescura/prioridad de raíl,
y luego usar `buttons` en lugar de `p->buttons` durante la fusión. El lado raíl también debe consumir
los flancos sin aplicarlos, para no reproducir una pulsación al cambiar de fuente. Una sola
llamada por lado y por lectura de la interfaz. Este subtrabajo no modifica `gui_bluetooth.c`
ni `gui.c`.

Coste: 100 bytes fijos por Joy-Con (máscara y 24 fechas), sin asignación. Ningún cambio en las claves, en el perfil
BR/sniff/QoS, en el page timeout, en la serialización de las conexiones ni en los reintentos.

Límites: varias pulsaciones del mismo botón entre dos lecturas se convierten en un solo flanco;
esta corrección no es una cola de clics. Un informe que se queda pendiente en el controlador se fecha en su recepción por el
host: la antigüedad física de la pulsación no puede deducirse de esta fecha.

### 2. Presupuesto RX y coste total: hacer correcta la observación

El servicio sigue siendo cooperativo. El bucle llama al polling normal antes de `lv_task_handler`
(`frontend/gui.c`, bucle GUI), mientras que cada rectángulo de visualización llama a la variante rápida
(`frontend/gui.c:334`). La tarea Bluetooth de alta prioridad se crea con un periodo de 2 ms y solo refresca
las estadísticas cada 500 ms (`frontend/gui_bluetooth.c`, `tick` y `start`). La prioridad no
interrumpe una tarea ya en curso: `bdk/libs/lvgl/lv_misc/lv_task.c:80` organiza llamadas
cooperativas.

Presupuestos conservados: `nova_bt_poll()` = RX hasta 8 000 µs, silencio de 2 000 µs tras la
recepción; `nova_bt_poll_fast()` = RX hasta 250 µs sin silencio (`nova_bt_stack.c:422`). Una línea vacía vuelve a devolver
inmediatamente. Estos números limitan un segmento RX, no el tiempo de radio ni el retardo botón-pantalla.
Una callback de paquete no puede interrumpirse durante el procesamiento.

Antes de la corrección, la prueba del presupuesto usaba una fecha tomada antes de `input_byte`, por lo tanto antes de
las callbacks del paquete actual. La misma lectura de reloj se toma ahora tras el procesamiento
(`nova_bt_stack.c:363`). Esto hace contar la callback en el presupuesto sin añadir lectura de reloj
por byte. `rx_budget_hits` cuenta los segmentos que llegan a su techo (`:371`).

El antiguo `rx_max_us` sigue siendo un máximo de fase RX. No comprendía el TX, las callbacks de bucle, los
timers, el perfil, las conexiones y el cierre en error. Los nuevos campos
(`nova_bt.h:32`) miden:

| Campo | Sentido |
| --- | --- |
| `poll_max_us` | Máximo de una llamada completa, cierre en error incluido |
| `poll_fast_max_us` | Máximo completo de la variante rápida |
| `poll_normal_max_us` | Máximo completo de la variante normal |
| `poll_gap_max_us` | Mayor intervalo entre inicios de dos llamadas al servicio |
| `rx_budget_hits` | Número de segmentos RX que llegan a su techo |

La instrumentación añade una lectura de reloj por llamada, tras todas las fases (`nova_bt_stack.c:415`),
y ninguna espera. No pretende proporcionar un percentil ni un intervalo entre lecturas LVGL.
Hay que añadir estas mediciones UI cuando sea necesario antes de atribuir los tirones a la radio. Los contadores
deben exponerse en el informe SD por el padre.

### 3. Velocidad UART y RTS: margen limitado, ajustes físicos que conservar

La radio usa 115 200 baudios, 8 bits y un stop (`nova_bt_radio.c:67`), el control de flujo hardware
RTS/CTS (`:80`) y el umbral RX de 8 bytes tomado del perfil HWTest (`:18`). La vía Nova lee
directamente el FIFO y devuelve inmediatamente `-1` si está vacío (`:101`); no usa el `uart_recv` genérico
del BDK, que espera hasta 250 µs tras cada byte (`bdk/soc/uart.c:98`). Sustituir esta vía por el
helper BDK reintroduciría una espera inadecuada para el redibujado.

A 115 200 baudios en 8N1, la capacidad teórica es de 11 520 bytes/s. El tiempo de cable de una trama de N bytes
es `N × 10 / 115200` segundos: 59 bytes ocupan 5,12 ms; 63 bytes ocupan 5,47 ms. Estos tamaños
ilustran el cálculo, no constituyen una medición de los informes de la consola. A modo de
escenario, 110 tramas/s de 59 bytes usan aproximadamente el 56 % del caudal bruto, antes de eventos HCI, señales y pausas
RTS. El flujo puede por tanto acercar varios informes y ocupar el segmento RX de 8 ms, a la vez que se queda por debajo del caudal nominal.

Las pausas RTS protegen el FIFO durante las tareas gráficas pero pueden aumentar el retardo de los informes
pendientes. El comentario HWTest menciona una recuperación UART superior a 300 µs (`nova_bt_stack.c:356`); es
un resultado anterior a la presente auditoría. Las pruebas existentes simulan huecos de recuperación de 700 µs. **No
bajar arbitrariamente 8 ms / 2 ms, suprimir RTS/CTS, cambiar el umbral FIFO ni aumentar el baudrate en esta
iteración.** Comparar primero `poll_gap_max_us`, `rx_budget_hits`, errores UART y regularidad de los informes en la
consola.

### 4. Asignaciones y estadísticas: optimización secundaria comprobable

Los objetos BTstack están en pools fijos: 2 conexiones HCI, 8 canales L2CAP y 2 conexiones HID
(`bluetooth/btstack_config.h:9`, `libs/btstack/btstack_memory.c:140`, `:257`, `:844`). No se ha
añadido ningún malloc a la vía de recepción ni al latch.

El panel de estadísticas formatea un texto que contiene contadores y posiciones cada 500 ms y luego llama a
`lv_label_set_text` y ajusta el tamaño. La guarda idéntica de `bdk/libs/lvgl/lv_objx/lv_label.c:150` evita el trabajo
si el texto no cambia, pero los contadores cambian durante el streaming. La vía modificada
libera/asigna y recalcula el label (`:169` a `:183`). Esto constituye una fuente de redibujados
ocasionales, sin prueba de que sea perceptible en consola. Optimización futura comprobable: fijar
los detalles avanzados fuera de su panel, conservar la información útil visible y comparar las duraciones de frame con el panel
abierto/cerrado. El Stop explícito escribe el informe SD; este coste puntual no debe
confundirse con el del streaming.

## Verificación nativa

`python work/check_nova30_wireless.py` pasa contra las fuentes v30 reales, con radio y reloj
sustituidos. El script también compila las fuentes v29 reales para demostrar los dos fallos
específicos:

- v29 pierde la pulsación a 1 000 ms / suelta a 1 017 ms antes de una lectura a 1 033 ms; v30 la devuelve una vez y luego devuelve la suelta.
- v29 no cuenta una callback de bucle de 3 000 µs en `rx_max_us`; v30 la cuenta en el máximo de poll completo.
- v30 verifica el estado mantenido, la caducidad independiente de dos botones, el stale, un verdadero evento de desconexión
  HCI, la limpieza periódica stale, Stop/reinicio, informes simples/completos, el lado izquierdo/derecho y el desbordamiento de
  reloj.
- v30 verifica exactamente cuatro lecturas de reloj en un poll vacío instrumentado, sin espera añadida; una callback de paquete
  simulada de 600 µs llega realmente al techo rápido de 250 µs, lo que muestra también que una callback no puede ser
  interrumpida.
- La suite nativa H4/BTstack existente se reutiliza y pasa: inicialización real de 57 comandos HCI,
  parsing, claves, entrada, recuperación y vencimientos. Solo sus aserciones de coste con reloj virtual se ajustan de
  270 a 280 µs (y de 27 000 a 28 000 para cien llamadas), es decir una lectura de reloj virtual
  adicional por llamada.

Las pruebas `input` existentes que usan una extracción del merge y un falso estado deberán declarar un stub
`nova_bt_take_buttons` cuando el padre integre la API. Para probar el latch de extremo a extremo, conectar la implementación
real en lugar de un stub que solo devuelve los botones actuales. Las suites de negociación/radio físicas
siguen siendo necesarias tras la integración; este informe no anuncia su ejecución en una consola.

Validación integrada por el padre: consumo en el merge antes de la prioridad de raíles, pruebas de botones
breves en este merge, máximos expuestos al informe SD y generación de nova30-wireless-check.json. Los presupuestos de radio
físicos se conservan.
