# Hekate Nova Pro — beta v31
**Languages:** [Français](LIRE-MOI-V31.md) · [English](LIRE-MOI-V31.en.md) · **Español** · [Русский](LIRE-MOI-V31.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. Este archivo contiene la carpeta bootloader completa y el payload correspondiente.

## Menú experto de particionado

- Tres ajustes a todo el ancho, resumen horizontal abajo.
- Cursores más largos y botones −/+ con repetición al mantener pulsado.
- Tamaños mostrados en GB enteros, sin decimales. Marcas «29 GB Full» y «58 GB Full», también para las emuMMC dobles.
- Ninguna modificación de los tamaños reales, reservas o alineaciones: los GB mostrados se redondean al entero inferior, como las marcas de Hekate. Android muestra su asignación total, sistema incluido. Se conservan los niveles emuMMC de Hekate; Linux y Android usan pasos de 1 GB tras su mínimo.
- Las confirmaciones de borrado existentes siguen activas. La modificación del menú no lanza ningún particionado.
- Tema Nova conservado, incluido el fondo de pantalla y la transparencia.

## Bluetooth y fluidez

El procesamiento Bluetooth continúa durante los dibujados de software largos, las copias de imagen y las esperas de rotación de pantalla. El paso desde el dibujado se limita a un presupuesto de recepción de 250 µs, como máximo una vez cada 2 ms; el procesamiento de un paquete entero puede superar este presupuesto. No lanza ningún otro menú, ni tarea gráfica, ni escritura en la SD.

Se conserva el funcionamiento de v30: arranque automático desde el archivo de emparejamiento guardado cuando los dos raíles están vacíos al arrancar, sin abrir el menú Bluetooth. La presencia de un Joy-Con en un raíl impide este automatismo. Una detención manual sigue respetándose hasta el próximo arranque. Los diagnósticos que reservan los periféricos conservan su funcionamiento dedicado.

Esta versión añade mediciones de enlace de solo lectura, espaciadas (como máximo dos peticiones por segundo en total). No se ha cambiado ningún ajuste de potencia, EDR, velocidad UART, emparejamiento o cifrado. No se carga ningún firmware HCD adicional.

**El alcance más allá de 80 cm todavía no está validado en consola.** El informe v30 muestra un máximo de 118,5 ms entre dos pasos de procesamiento en el host y una interrupción de recepción más larga a la derecha; no prueba un fallo de radio ni la causa exacta de la distancia. Esta beta mejora las posibilidades de procesamiento y permite medir lo que queda.

## Instalación

1. Consola apagada, conservar una copia de vuestra carpeta bootloader y de vuestros archivos personales.
2. Fusionar la carpeta bootloader de este archivo con la de la SD sustituyendo los archivos incluidos. Mantener juntos el nuevo Nyx y el payload correspondiente.
3. Usar hekate_nova_pro_6.5.5_v31.bin si vuestro método de arranque usa un payload externo. También se incluye bootloader/update.bin.
4. Conservar bootloader/bluetooth/joycon-pairing.bin o su copia de seguridad .bak: esta actualización no cambia su formato. No borrar las claves ni limpiar toda la SD para esta prueba.

No se incluye en el archivo ningún archivo privado de emparejamiento ni ningún archivo de configuración personal.

## Prueba recomendada

Con la consola en la misma posición, probar una sesión cerca y luego otra más allá de 80 cm. En cada sesión, mover los sticks y pulsar A/B durante unos 20 segundos. Elegir «Detener Bluetooth» y copiar inmediatamente bootloader/bluetooth/last-session.txt antes de relanzar o volver a leer los emparejamientos. Conservar los dos informes con nombres distintos.

Enviar únicamente estos informes de texto; los .bin/.bak de emparejamiento contienen claves privadas.

El informe añade:
- Capture_ms y número de pasos durante el dibujado.
- Full_reports, antigüedad del último informe completo, máximo entre informes completos y contadores de huecos >50 / >250 ms. Los acuses de recibo no cuentan como informes completos. Las estadísticas de sesión no constituyen una medida de latencia botón-pantalla.
- RF kind=0: RSSI BR/EDR relativo en dB; 0 significa dentro del rango del controlador, no una señal nula. No es una medida absoluta en dBm.
- kind=1: calidad del proveedor de 0 a 255, no un porcentaje.
- kind=2/3: potencia de emisión actual / máxima en dBm, solo leída.
- kind=4: contador de contactos fallidos consecutivos del controlador, no un acumulado de todos los paquetes perdidos.
- Para cada medición: número de muestras, antigüedad, último valor y mínimo/máximo de sesión. n/a significa ninguna medición válida. Los valores antiguos no describen necesariamente el enlace en el momento de la exportación.
- Errores de medición: FD = valor de potencia fuera de rango, FE = respuesta ausente, FF = respuesta mal formada o identificador incorrecto. Una medición no soportada o sin respuesta se desactiva para esta conexión; no detiene los comandos. Estos códigos son independientes de los errores de autenticación.

## Verificación

Compilación ARM y controles automáticos documentados en VERIFICATION-V31.json. Las pruebas en PC cubren la representación LVGL real, los callbacks reales del particionado, los intercambios HCI/L2CAP/HID simulados y las rutas de emparejamiento existentes. Los periféricos y la SD se sustituyen en estas pruebas: no se ha realizado ninguna operación de particionado real ni ninguna medición de alcance físico.
