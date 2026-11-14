# Hekate Nova Pro 23 — prueba de conexión Joy-Con
**Languages:** [Français](LIRE-MOI-V23.md) · [English](LIRE-MOI-V23.en.md) · **Español** · [Русский](LIRE-MOI-V23.ru.md)

Base conservada: Hekate 6.5.5 / Nyx 1.9.5. Interfaz Nova, ajustes, dock y
diagnósticos conservados. La conexión de radio de esta versión todavía debe validarse en
la consola: las pruebas en PC descritas abajo no sustituyen una prueba física.

## Lo que muestra tu captura v22

Emparejamientos importados 2/2, ningún informe, HID 0/0 y ERR 69/00. La radio responde,
pero ninguna conexión de mando transmite comandos. El código hexadecimal 69
es un tiempo de espera agotado durante una negociación L2CAP; no designa un botón
averiado. Las únicas estadísticas HCI no prueban una conexión de los Joy-Con.

La v22 empezaba con una búsqueda de descriptor de servicio SDP antes de abrir
los canales de mando. Esta v23 usa directamente los canales HID estándar conocidos
de los Joy-Con: canal de comandos 0x0011 y luego canales de entrada 0x0013. La autenticación con las claves
importadas sigue siendo obligatoria. El decodificador propio de Nova no necesita un
descriptor descargado para reconocer los informes de los Joy-Con de Nintendo.

El cambio evita el paso previo SDP. No prueba que ese paso previo sea
la única causa del bloqueo en tu consola. El informe de sesión permite
localizar un posible fallo en la nueva ruta.

## Instalar y probar

1. Conserva una copia de tu carpeta bootloader actual.
2. Copia la carpeta bootloader incluida en la SD, aceptando la sustitución
   de los archivos. No elimines tus configuraciones. Comprueba **23** en los créditos.
3. Abre Ajustes de Nyx → TV / Dock → Joy-Con inalámbricos.
4. Fija los Joy-Con ya emparejados con esta consola y elige Leer los emparejamientos.
5. Despréndelos, elige Activar Bluetooth y luego pulsa un botón para despertarlos.
6. Comprueba que HID pasa a 1 y que Informes aumenta, y luego prueba el stick y A / ZL / ZR.
7. Elige **Detener Bluetooth** tras la prueba: el informe se guarda aquí:
   `bootloader/bluetooth/last-session.txt`.

Si los comandos siguen sin funcionar, conserva este archivo y la captura
del estado mostrado. El informe contiene las etapas y códigos de protocolo; no
contiene direcciones Bluetooth, claves de emparejamiento ni contenido bruto de los paquetes.
Un mensaje confirma si el guardado ha funcionado. No se escribe ningún informe
automáticamente durante el lanzamiento de otro sistema o un diagnóstico.

El payload incluido es `hekate_nova_pro_6.5.5_v23.bin`. El payload Hekate sigue
funcionalmente idéntico al de v22; las modificaciones están en Nyx.
Copiar solo el payload sin sustituir la carpeta bootloader no pone
el servicio al día. Los archivos nyx.ini y hekate_ipl.ini no se incluyen.

## Cambios de conexión

- Apertura directa y autenticada de los canales HID, sin petición SDP obligatoria.
- Misma ruta sin SDP cuando el mando inicia la conexión.
- Comando de paso a modo completo enviado en un informe de tamaño completo.
- Cierre del canal de comandos si falla la apertura del canal de entradas,
  para liberar la conexión antes de un nuevo intento.
- Estado inicial explícito: «Radio lista: conexión de los Joy-Con en espera».
  «Comandos Joy-Con activos» aparece solo tras un informe decodificado reciente.
- Añadido el canal solicitado PSM y la etapa STEP a la información mostrada.
- Informe de sesión limitado a 128 entradas de metadatos, exportado con Detener Bluetooth.

Las correcciones v22 de navegación, de cabecera Nyx y de compatibilidad HOS 23.5 se
conservan. El servicio sigue desactivado al arranque y las claves permanecen en RAM.
Los emparejamientos de los Joy-Con no se reescriben y no se incluye ningún firmware Bluetooth
propietario. La importación por raíles no afecta a la Switch Lite.

## Lectura del informe

PSM 0011 corresponde al canal de comandos; 0013 al canal de entradas. ERR 69 indica
un tiempo de espera agotado; otro código puede señalar un error distinto de autenticación
o de apertura. STEP distingue el intento de conexión, el enlace, la autenticación,
los canales y la recepción de informes. El detalle de la traza sirve para analizar la prueba.

Los registros de tipo 1 solo conservan el opcode de un comando HCI y su
longitud, nunca sus parámetros. Tipos 2/3: fin o estado de un comando; 4: evento
de enlace; 5/6: señalización L2CAP emitida/recibida; 7: emisión HID; 10: resultado
de apertura HID; 11: primer informe. Los informes de botones periódicos no
llenan la traza: solo se conservan sus contadores y el primer evento.

## Comprobaciones realizadas

Un interlocutor virtual intercambia tramas H4 reales con la pila BTstack de Nova.
Los dos mandos se autentican con sus claves, se configuran los canales comando/entradas,
se recibe el comando completo, y luego el ACK, los botones y los
sticks atraviesan toda la pila. Una desconexión suelta los comandos afectados.
Esta prueba no fabrica directamente un evento «HID abierto» para el servicio.

También se cubren tres escenarios negativos: búsqueda SDP sin respuesta que da
69 en la ruta antigua, canal de comandos sin respuesta que da 69 con PSM0011, y
canal de entradas sin respuesta que provoca el cierre del canal de comandos.
El interlocutor virtual no demuestra el comportamiento de radio del Joy-Con real.

La exportación se comprueba con los errores de tarjeta, carpeta, apertura, escritura corta
y cierre; un fallo no debe anunciarse como un informe guardado.
Los controles de interfaz y de navegación de Nova siguen ejecutándose en PC.

## Fuentes y licencias

BTstack conserva su licencia BlueKitchen personal y no comercial, adjunta
al paquete. El archivo classic/hid_host.c incluye una pequeña adaptación Nova
condicionada por NOVA_HID_KNOWN_PSMS; sus demás rutas siguen siendo las de upstream.
La modificación y la revisión original se describen en BTSTACK-UPSTREAM.txt.
Se conservan los demás créditos y licencias. Lee también LIRE-MOI-V22.md.
