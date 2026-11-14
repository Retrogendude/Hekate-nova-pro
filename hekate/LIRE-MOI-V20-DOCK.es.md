# Hekate Nova Pro 20-Dock
**Languages:** [Français](LIRE-MOI-V20-DOCK.md) · [English](LIRE-MOI-V20-DOCK.en.md) · **Español** · [Русский](LIRE-MOI-V20-DOCK.ru.md)

Esta versión añade la detección automática de la dock a las funciones TV de
Nova19-TV. La conexión de referencia es una Switch OLED insertada directamente
en una dock oficial de Nintendo, con alimentación y HDMI conectados.
La activación automática de esta versión todavía debe probarse en la consola.

## Instalar

Haz una copia de seguridad de tu carpeta `bootloader`, o utiliza una tarjeta SD de prueba. Copia la
carpeta `bootloader` incluida en la raíz y lanza
`hekate_nova_pro_6.5.4_v20_dock.bin`. Comprueba **20-Dock** en los créditos.
El paquete conserva tus archivos personales `nyx.ini` y `hekate_ipl.ini`.
Nyx está en `bootloader/sys/nyx.bin`: sustituir solo el payload no es suficiente.

## Activación automática: ON / OFF

En **Ajustes de Nyx → TV / Dock**, activa **Detección automática de la dock**
y luego usa **Guardar** para mantener tu elección tras el reinicio.

- **ON**: cuando Nyx arranca ya en la dock, o cuando colocas después
  la consola sobre ella, la conexión TV se inicia automáticamente tras una breve
  comprobación de presencia. El televisor debe estar encendido y en la entrada HDMI correcta.
- **OFF**: ninguna detección automática en segundo plano. El botón **Activar
  la TV** sigue disponible. Desactivar el automatismo no corta una conexión
  TV ya iniciada: usa **Detener la TV** o **VOL-** para ello.

La nueva opción está desactivada por defecto para conservar el comportamiento
de las configuraciones anteriores. Una vez guardada, se vuelve a leer en cada
arranque de Nyx. No modifica las funciones TV de un sistema ya iniciado.

La detección espera unos dos segundos tras la inicialización de la interfaz,
y luego confirma la presencia de la dock con dos lecturas espaciadas. La negociación
y la inicialización del vídeo toman después su propio tiempo.

## Pantalla de la consola

El ajuste **Visualización TV** también está en la misma pestaña:

- **TV + consola** conserva las dos pantallas; es el ajuste inicial.
- **Solo TV** apaga la pantalla de la consola tras la inicialización correcta de la TV.
  Se vuelve a encender cuando se detiene o se desconecta la salida TV.

Usa **Guardar** para conservar esta elección. El brillo ajustado en Nyx
se conserva para el retorno a la pantalla de la consola. No se sobrescribe
al pasar a TV.

## Detención, retirada y nuevos intentos

**VOL-** o **Detener la TV** corta la salida. El automatismo no la vuelve a encender
inmediatamente: retira la consola y vuelve a colocarla en la dock para rearmar la
detección, o usa **Activar la TV** manualmente.

Un intento automático fallido no se repite en bucle. Si el televisor o
el HDMI no estaban listos, comprueba la conexión y luego reinicia manualmente o
retira/vuelve a colocar la consola. Un error de lectura del controlador no se
interpreta como una desconexión.

El detector reconoce la oferta de alimentación habitual de las docks Nintendo,
y luego el driver realiza la negociación completa. Los demás cargadores/adaptadores
no constituyen la conexión de referencia; la activación manual sigue
disponible. La Switch Lite no es compatible con la salida TV.

Los diagnósticos de hardware y las funciones USB exclusivas suspenden el servicio
TV. Su retorno restablece la supervisión automática; una salida TV que estaba
activa puede reactivarse. El automatismo respeta una detención manual previa.
Un lanzamiento de sistema, de payload, un apagado o un reinicio terminan el servicio.

## Informes y validación

Los informes están en `bootloader/tv-test/last-run.txt` y `run-NNN.txt`.
Para la prueba en la consola, comprueba:

1. ON guardado, reinicio de Nyx con la consola en la dock;
2. arranque fuera de la dock y luego colocación en la dock;
3. detención con VOL-, ausencia de relanzamiento inmediato, y luego retirada/reconexión;
4. retorno a la pantalla de la consola y conservación de las elecciones tras el reinicio.

La compilación y las pruebas en PC verifican el código, las preferencias y los casos
de error. No sustituyen estas pruebas físicas. El driver de vídeo y sus
ajustes se conservan del prototipo01 probado en inserción directa.
La salida HDMI con audio y los cables intermedios no se validan aquí.

## Joy-Con inalámbricos

No están activados en esta versión. El código actual prueba la radio,
pero falta el servicio Bluetooth de conexión y de comandos de los mandos.
Consulta `documentation/ETUDE-JOYCONS-SANS-FIL.md` para el análisis y las referencias.
Los Joy-Con en los raíles conservan su compatibilidad existente.

Los archivos de Nova19-TV y del prototipo01 se conservan para volver a la
versión anterior. Consulta `documentation/TV-SOURCES.md` para la procedencia.
