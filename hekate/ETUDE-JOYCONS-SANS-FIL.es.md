> Historial v20: este estudio precede al prototipo 21-BT. Para las funciones ahora integradas y sus límites, consulta LIRE-MOI-V21-BT.md.

# Joy-Con inalámbricos directamente en Hekate: estado y vía de desarrollo
**Languages:** [Français](ETUDE-JOYCONS-SANS-FIL.md) · [English](ETUDE-JOYCONS-SANS-FIL.en.md) · **Español** · [Русский](ETUDE-JOYCONS-SANS-FIL.ru.md)

La función es concebible, pero no está disponible en esta
entrega. El trabajo necesario va más allá de añadir un interruptor Bluetooth.
No se ha añadido ningún botón falso de conexión inalámbrica.

## Lo que el proyecto ya contiene

- El driver `bdk/input/joycon.c` usa los UART B/C para los Joy-Con en los
  raíles. Los mensajes «HCI» enviados a los mandos no constituyen una pila
  Bluetooth que funcione en la consola.
- Los ajustes de Nyx saben exportar información de emparejamiento Joy-Con.
  Esa exportación se usa en particular para otros sistemas y no activa lo inalámbrico
  en Nyx. [Fuente Hekate](https://github.com/CTCaer/hekate/blob/master/bdk/input/joycon.c).
- La prueba de hardware local `diagnostics/vendor/probe_bt.c` sabe verificar que la
  radio BCM/CYW4356 responde a comandos HCI por UART-D. Realiza una
  preparación de hardware y luego un diagnóstico, con restauración por el marco
  de las pruebas. No gestiona ni conexión de mando, ni recepción HID continua.
  Esta prueba no debe convertirse en una tarea repetida para pilotar los Joy-Con.

## Lo que todavía hay que desarrollar y validar

1. Iniciar la radio una sola vez para una sesión, procesar su posible
   firmware de inicialización, y luego recibir los eventos Bluetooth sin
   bloquear el menú. Verificar en paralelo la coexistencia con la visualización TV.
2. Gestionar las conexiones Bluetooth Classic, los canales HID, la información
   de emparejamiento y las reconexiones. Preservar las asociaciones existentes y
   no colocar nunca sus claves en los registros de diagnóstico.
3. Decodificar los botones y joysticks de los dos Joy-Con, aplicar sus
   calibraciones, y luego alimentar los comandos Nyx existentes. Los informes
   Bluetooth Joy-Con están documentados, lo que proporciona una base de trabajo.
   [Notas técnicas de dekuNukem](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md).
4. Verificar las desconexiones, el retorno a los comandos en los raíles, los diagnósticos,
   la memoria disponible y el paso a un sistema u otro payload.
   La detención del servicio debe liberar sus recursos antes de este paso.

Una vía es [BTstack](https://github.com/bluekitchen/btstack), que propone
un funcionamiento embebido sin sistema operativo, un transporte H4 UART
y un ejemplo de host HID. Su integración en la Switch queda por hacer; no
basta con compilar la biblioteca. Consulta también su
[documentación de los chips](https://github.com/bluekitchen/btstack/blob/master/chipset/README.md)
y sus [condiciones de licencia](https://github.com/bluekitchen/btstack/blob/master/LICENSE),
que deben examinarse antes de su adopción o de una redistribución del proyecto.

## Conclusión para esta versión

La detección automática de la dock se implementa independientemente del Bluetooth.
El control de Nyx con los Joy-Con desprendidos sigue siendo un desarrollo aparte.
La primera prueba que obtener sería una conexión estable de un mando con
recepción real de sus botones, antes de presentar la función como utilizable.
Un resultado «radio HCI: PASS» no constituye esta prueba.
