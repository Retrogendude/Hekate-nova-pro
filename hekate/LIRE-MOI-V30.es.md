# Hekate Nova Pro 30 — Bluetooth en segundo plano y fluidez
**Languages:** [Français](LIRE-MOI-V30.md) · [English](LIRE-MOI-V30.en.md) · **Español** · [Русский](LIRE-MOI-V30.ru.md)

Al arrancar Nova, el driver comprueba la presencia física en los raíles.

- Ningún Joy-Con fijado: carga de `bootloader/bluetooth/joycon-pairing.bin`,
  y luego activación automática del Bluetooth. Se intenta la copia `.bak` si es
  necesario. El servicio arranca desde la pantalla de inicio sin abrir ninguna ventana.
- Al menos un Joy-Con fijado: ninguna carga SD ni arranque Bluetooth
  automático. Se conserva el funcionamiento habitual de los raíles.
- Ningún emparejamiento válido guardado: el arranque continúa con Bluetooth
  detenido, sin error de radio ni ventana bloqueante.

Esta decisión se toma una vez por arranque, tras la inicialización de los
raíles. Una fijación detectada durante la lectura SD cancela el arranque
automático. Se respeta la detención manual. Desprender luego un Joy-Con no
relanza el servicio automáticamente en esa misma sesión. El driver
desactivado y los modelos sin raíles compatibles con este driver quedan excluidos
de este arranque automático.

Las copias de seguridad de v29 son compatibles. Usar la correspondiente a
esta consola y a sus mandos, en particular para las pruebas en dos OLED. Si aún no existen,
fijar los mandos, leer los emparejamientos y guardarlos una vez desde
el menú Joy-Con inalámbricos, con Bluetooth detenido. Los archivos `.bin` y `.bak`
contienen claves privadas: no adjuntarlos a los informes.

El servicio funciona en segundo plano en la pantalla de inicio y los menús ordinarios.
Puede seguir siendo necesario pulsar un botón normal para despertar un
mando apagado. Las pruebas de hardware conservan sus suspensiones necesarias
durante las mediciones; la v30 no fuerza su radio a permanecer ocupada.

La auditoría condujo a las optimizaciones siguientes:

- Reflejo: evita los desbordamientos periódicos de la cola de dibujo, a la vez que
  conserva su aspecto. Reducción de las superficies redibujadas verificada en PC.
- Comandos: conserva una vez las pulsaciones breves recibidas entre dos lecturas,
  con caducidad para no reproducir un comando antiguo tras un congelamiento.
- Mantenimiento: el polling rápido continúa en los puntos de mantenimiento de las herramientas
  largas, fuera de los diagnósticos suspendidos.
- Mediciones: el informe incluye el tiempo máximo de servicio completo, sus variantes,
  el intervalo entre llamadas y el número de segmentos RX en el techo.
- Recursos: control de los tamaños y de las lecturas de `res.pak`; limpieza
  de los payloads rechazados, verificación antes de la asignación y lectura completa.

Instalación: fusionar la carpeta `bootloader` del ZIP con la de la SD,
sustituyendo los archivos incluidos para conservar vuestra copia de seguridad privada,
y luego reiniciar.
Usar `hekate_nova_pro_6.5.5_v30.bin` si es necesario. No es necesario formatear la SD ni
sustituir vuestros ajustes. Las claves guardadas en la
tarjeta no se incluyen ni se borran por esta distribución.

Tras la prueba, «Detener Bluetooth» guarda
`bootloader/bluetooth/last-session.txt`. `Autostart=2` significa que se solicitó el lanzamiento
automático; no prueba que los mandos ya estén
despiertos y conectados. `Autostart=1` indica la presencia en los raíles,
`3` la ausencia de copia de seguridad válida y `5` una detección no disponible.
Los máximos de polling describen el servicio en el host, no la latencia completa
entre un botón físico y la imagen. No se anuncia ningún objetivo de FPS de hardware.

La compilación y las pruebas en PC deben completarse con una prueba en las dos OLED.
Base: Hekate 6.5.5 / Nyx 1.9.5. Licencias e informes de auditoría incluidos.
