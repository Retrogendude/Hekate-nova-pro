# Hekate Nova Pro 21-BT — prototipo personal
**Languages:** [Français](LIRE-MOI-V21-BT.md) · [English](LIRE-MOI-V21-BT.en.md) · **Español** · [Русский](LIRE-MOI-V21-BT.ru.md)

Base Hekate 6.5.4 / Nyx 1.9.4, a partir de Nova 20-Dock.

## Correcciones de tus capturas

- Herramientas USB: un solo marco para la banda de Solo lectura. El interruptor y
  la protección eMMC / emuMMC conservan su funcionamiento.
- Créditos: panel Nova restaurado detrás de todas las pestañas. El fondo de pantalla
  permanece detrás del contenido, con la transparencia elegida en los ajustes.
- Nueva pestaña Changelog: versiones 21, 20, 19 y 18 en una sola página.
  Las pestañas permanecen arriba; no se añadió paginación abajo.

## Instalar

Conserva una copia de tu carpeta bootloader actual. Copia la carpeta bootloader
incluida en la raíz de la SD y lanza hekate_nova_pro_6.5.4_v21_bt.bin.
El paquete no contiene nyx.ini ni hekate_ipl.ini: tus ajustes siguen siendo
los de tu tarjeta. Comprueba «21-BT» en la pestaña Nova Pro de los créditos.

Se conservan las funciones TV de la v20. Inserta directamente la Switch OLED
en la dock oficial, con alimentación y HDMI. El cable intermedio que
usabas en las primeras pruebas no forma parte del montaje de referencia.

## Probar los Joy-Con inalámbricos

El servicio es experimental y se desactiva en cada arranque. Contiene una
verdadera pila Bluetooth Classic / L2CAP / HID y un enlace con los comandos
de Nyx. Las pruebas de software todavía no validan una conexión de radio real.

1. Usa Joy-Con de Nintendo ya emparejados con esta consola en su sistema.
2. Fíjalos en los dos raíles. El driver Joy-Con con cable de Nyx debe estar activo.
3. Abre Ajustes de Nyx → TV / Dock → Joy-Con inalámbricos.
4. Pulsa Leer los emparejamientos. La pantalla debe indicar 2 / 2; un solo Joy-Con
   también puede importarse si es necesario.
5. Despréndelos y pulsa Activar Bluetooth. Pulsa un botón de cada
   Joy-Con para despertarlos. Deja los sticks en reposo durante el centrado.
6. Observa el paso a Bluetooth listo y luego Comandos recibidos. Comprueba el cursor,
   la selección con A / ZL / ZR, y luego la navegación cuando la salida TV está activa.

Cerrar esta ventana deja el servicio activo. Detener Bluetooth libera la radio.
Borrar las claves también elimina los emparejamientos importados de la memoria de la sesión.
Hay que volver a hacer la lectura tras un reinicio. Ninguna clave se guarda en la SD,
y este prototipo no modifica los emparejamientos almacenados en los Joy-Con.

Los raíles tienen prioridad si un Joy-Con está fijado. Los comandos inalámbricos se
sueltan tras 500 ms sin informe recibido, para evitar una selección mantenida.
El servicio se detiene antes de un diagnóstico, de un modo USB o del fin de hardware de Nyx.
Tras un diagnóstico o un modo USB, relánzalo manualmente si es necesario.

Esta primera vía usa los emparejamientos de los Joy-Con desmontables. Todavía no añade
la búsqueda y el emparejamiento de nuevos mandos, ni los mandos Pro.
La importación por raíles no está disponible en Switch Lite.

## Si la conexión falla

Fotografía el estado, los contadores y el código mostrados en Joy-Con inalámbricos.
Un error de este servicio experimental no es un veredicto de fallo de hardware.

| Código | Etapa afectada |
| --- | --- |
| 1 | Emparejamientos ausentes, inválidos o pertenecientes a hosts distintos |
| 2 | UART-D ya ocupado: la radio no se ha reiniciado |
| 3 | Inicialización HCI no terminada en 15 segundos |
| 4 | Error de recepción UART |
| 5 | Trama H4 inválida, demasiado grande o incompleta tras 2 segundos |
| 6 | Emisión bloqueada durante más de 2 segundos |
| 8 | Evento de error de hardware señalado por el controlador HCI |

El arranque usa el código ROM del CYW4356. No se incluye ni se instala ningún
firmware propietario. La posible necesidad de un parche de firmware
queda por determinar a partir de una prueba real en la consola.

## Comprobaciones realizadas

- Compilación ARM de Hekate y Nyx.
- Inicialización de la pila BTstack real con un controlador HCI sustituido,
  tres ciclos de arranque / parada, tramas parciales y demasiado grandes,
  emparejamientos, botones, sticks, desconexiones y tiempos de error.
- Ruta UART / GPIO real ejercitada con registros sustituidos: restauración de
  los pines y del reloj, velocidad por ráfagas, CTS y faltas UART. PH0 permanece intacto.
- Fusión de los comandos: prioridad a los raíles, ausencia de modificación del informe
  con cable original, neutralización de los datos antiguos.
- 462 vistas LVGL nativas: 7 idiomas, 3 fuentes, tamaños 20 y 36, fondo Carbon y
  opacidad 25 %. Clics simulados en Solo lectura en sus dos estados.
- 43 escenarios del controlador TV y 18 regresiones del protocolo de dock.

Estas comprobaciones sustituyen las entradas de hardware. No constituyen
una prueba Bluetooth o HDMI en esta integración v21. El único comentario físico recibido
es el del prototipo TV01: OLED directamente en la dock oficial, imagen
estable y parada por VOL−.

## Fuentes y licencias

BTstack se incluye con su propia licencia BlueKitchen, que debe conservarse con sus
fuentes y los binarios. Incluye una condición de uso personal y no
comercial. Por lo tanto, este prototipo personal no presenta todos sus componentes
como bajo GPL. El texto completo está en documentation/BTSTACK-LICENSE.txt.
Las licencias y créditos de los demás componentes siguen adjuntos a las fuentes.

Referencias:
- [BTstack, fuentes y licencia](https://github.com/bluekitchen/btstack)
- [Formato de los emparejamientos Joy-Con](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/spi_flash_notes.md)
- [Informes Bluetooth HID de los Joy-Con](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md)
- Consulta también TV-SOURCES.md para el origen del driver de la dock.
