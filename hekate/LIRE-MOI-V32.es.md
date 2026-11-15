# Hekate Nova Pro — beta v32
**Languages:** [Français](LIRE-MOI-V32.md) · [English](LIRE-MOI-V32.en.md) · **Español** · [Русский](LIRE-MOI-V32.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. Archivo completo bootloader + payload correspondiente.

## Particionado experto

- Resumen compacto en una sola línea: título, asignaciones y estado de validez.
- Tres filas más espaciadas. Los tamaños están a la derecha de los cursores y de los botones.
- Botones −/+ rediseñados, sin sombra que sobresalga, con márgenes distintos del cursor y de su tirador. El tirador permanece completamente dentro de la barra, incluso a cero.
- Ajuste fino por −/+ conservado, con repetición al mantener pulsado. Linux/Android: paso de 1 GB tras su mínimo. Niveles emuMMC Hekate conservados, incluidos 29 GB Full, 58 GB Full y las emuMMC dobles.
- GB enteros en la visualización; tamaños, reservas, alineaciones y confirmaciones de borrado sin cambios. La actualización no lanza ninguna operación de particionado.
- Tema Nova, fondo de pantalla y transparencia conservados. Las nuevas etiquetas están traducidas en los siete idiomas.

## Bluetooth: recuperación de un Joy-Con silencioso

Cuando un flujo completo ya establecido permanece silencioso durante un segundo, Nova vuelve a pedir el modo de informes completos. Si el silencio dura cuatro segundos, Nova pide una desconexión de ese único Joy-Con e intenta una nueva conexión con los emparejamientos existentes, un segundo después del cierre HID efectivo. Sin un primer informe completo, un margen de ocho segundos deja tiempo a la configuración inicial.

El otro Joy-Con continúa su procesamiento. La recuperación no reinicia la radio ni modifica las claves. Espera el final de los datos UART ya en curso de lectura: un paquete parcial o un presupuesto de lectura alcanzado no deben activar esta recuperación. El modo simple, que envía sus cambios a la demanda, no se trata como un flujo periódico ausente. Se siguen respetando la detención manual y el arranque automático de v31.

La inicialización también mantiene WL_REG_ON alto durante la sesión Bluetooth, como hace la referencia HWTest-RCM tras su reinicio común. El nivel anterior se restaura en la detención, ya fuera alto o bajo. No se inicia ningún servicio de red Wi-Fi. Esta modificación acerca la secuencia a la referencia; su efecto sobre el alcance debe verificarse en la consola.

**El objetivo de 2–3 metros sigue siendo una prueba de hardware por realizar. Esta beta todavía no garantiza este alcance.** El informe v31 recibido indica cortes de informes de hasta 2,8 s, mientras que el máximo de procesamiento en el host es de unos 22 ms. También presenta RSSI relativos negativos y una bajada de la calidad de enlace del proveedor. Estos datos orientan hacia el enlace de radio; no prueban una causa de hardware precisa ni una ganancia aportada por v32.

No se aplica ningún aumento forzado de potencia, modificación EDR/UART ni carga de firmware HCD. La radio permanece en ROM. Las mediciones HCI añadidas en v31 siguen disponibles. Una recuperación de conexión mejora la recuperación tras una interrupción; no aumenta por sí sola el alcance de la señal.

El informe v32 añade Radio_power_policy, así como Stream report_restarts / reconnects por Joy-Con y los eventos de recuperación (kind=21). Estos contadores distinguen las peticiones de recuperación de las reconexiones realmente terminadas; la presencia de nuevos informes y HID=1 permite confirmar el retorno de los comandos. Los contadores se conservan en la exportación, hasta el próximo arranque del Bluetooth.

## Instalación y prueba

1. Consola apagada, guardar una copia de bootloader y de los archivos personales.
2. Fusionar la carpeta bootloader del archivo con la de la SD, sustituyendo los archivos incluidos. Conservar los archivos de configuración y de emparejamiento personales.
3. Usar hekate_nova_pro_6.5.5_v32.bin si el arranque usa un payload externo. También se incluye bootloader/update.bin.
4. Conservar bootloader/bluetooth/joycon-pairing.bin y su copia de seguridad .bak: formato sin cambios, no es necesaria ninguna relectura ni supresión de claves para actualizar.

Probar los sticks y A/B cerca, y luego a 2 m y 3 m manteniendo la consola en el mismo sitio. Una detención prolongada de los comandos ahora puede provocar una reconexión automática del Joy-Con afectado. Tras cada sesión, elegir «Detener Bluetooth» y conservar inmediatamente last-session.txt con un nombre distinto. Indicar si la consola está en la dock.

Transmitir únicamente los informes de texto; los archivos .bin/.bak de emparejamiento contienen claves privadas. El archivo no contiene ningún dato privado de emparejamiento ni configuración personal.

## Validación

Compilación ARM correcta. 720 escenas de particionado con los callbacks reales, siete idiomas, tres fuentes, dos tamaños de texto, fondo de pantalla y transparencia, incluida una capacidad de 955 GB similar a la captura recibida. Verificaciones de coordenadas: resumen centrado en una sola fila, zonas −/+/tirador/valores separadas y ausencia de desplazamiento.

882 escenas complementarias y 28 pruebas de navegación inalámbrica verifican la representación existente. Las pruebas HCI/HID reproducen un Joy-Con derecho silencioso: v31 se queda bloqueado, v32 vuelve a pedir los informes y luego se reconecta con las mismas claves mientras continúan los informes izquierdos. Controles adicionales cubren los tiempos de espera, los datos pendientes, el modo simple, el desbordamiento del reloj, la detención y la restauración de los GPIO.

Las radios, los periféricos de vídeo y los soportes de almacenamiento se sustituyen en estas pruebas en PC. No se ha ejecutado ninguna medición física de alcance ni ningún particionado real. Los detalles y las huellas están en documentation/VERIFICATION-V32.json.

## Referencias técnicas

- Secuencia de radio HWTest-RCM, referencia congelada: https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/gui/gui_bt.c — _wl_reg_on_restore y _power_up.
- API GAP de BlueKitchen / BTstack: https://github.com/bluekitchen/btstack/blob/master/src/gap.h — gap_disconnect.
- Pista de firmware, no integrada: el repositorio Switchroot lista un parche BT procedente del sistema Nintendo, distinto del funcionamiento ROM actual: https://gitlab.com/switchroot/android/proprietary_vendor_firmware/-/blob/main/README.md. Esto no prueba que su ausencia explique por sí sola el presente defecto de alcance.
