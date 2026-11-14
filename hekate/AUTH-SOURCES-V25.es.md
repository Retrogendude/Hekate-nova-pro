# Nova25 — política de reconexión y evidencias de diagnóstico conservadas
**Languages:** [Français](AUTH-SOURCES-V25.md) · [English](AUTH-SOURCES-V25.en.md) · **Español** · [Русский](AUTH-SOURCES-V25.ru.md)

## Evidencia física proporcionada para Nova24

La nueva sesión del usuario avanza más allá del fallo de autenticación anterior:

- Read_BD_ADDR coincide con el host importado (HOST=1).
- Conexión entrante aceptada alrededor de 5183 ms, éxito a 5367 ms.
- Authentication Complete a 5494 ms tiene estado 00 para el handle 000E.
- Set_Connection_Encryption 0413 se acepta a 5513 ms.
- Encryption Change a 10516 ms tiene estado 08, seguido de desconexión 08.
- Ningún dato ACL llega a ninguno de los dos interlocutores. No se establece ningún canal HID ni informe de entrada.

Esto confirma la autenticación de un enlace, no de los dos Joy-Con, y no
establece que el cifrado en sí esté roto. El estado 08 es Connection Timeout:
[códigos de error del controlador de Bluetooth Core](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-61/out/en/architecture,-change-history,-and-conventions/controller-error-codes.html).
El estado 0 de la cabecera y los campos pad a cero resultan de que Stop borra el estado
de ejecución conservando la traza. No invalidan el evento de autenticación
correcta de esa traza.

## Cambios acotados

El código anterior de Nova dejaba la preferencia de rol entrante por defecto de BTstack en 1
(periférico), e iniciaba las dos conexiones salientes en el mismo ciclo de polling.
Nova25 pide central en las reconexiones entrantes y permite solo una configuración HID
saliente pendiente hasta que se abra o falle. Las peticiones entrantes se siguen
gestionando; el segundo intento saliente puede continuar tras el primer resultado.

Pedir central es la política de host HID usada explícitamente por
[Bluepad32 uni_bt_bredr.c](https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/bt/uni_bt_bredr.c)
en `uni_bt_bredr_setup` (`hci_set_master_slave_policy(HCI_ROLE_MASTER)`).
El `hci.c` local de BTstack inicializa esa preferencia a 1. Se usa su setter
público sin modificar el código del vendor. Es una hipótesis de compatibilidad para el
fallo físico; ni esta comparación de fuentes ni un interlocutor virtual prueban que
resolverá el cifrado en el controlador real.

Nivel de seguridad 2, claves existentes/type 4, orden de bytes de clave de v24, verificación de
lectura del host, política SSP/SC y rechazo de un nuevo emparejamiento siguen sin cambios. No se carga ni se
distribuye ningún blob de firmware. El informe incluye ahora la versión/revisión/fabricante/subversión
no única del controlador para seguir investigando si es necesario.

Stop libera todo el estado de entrada y los recursos de radio pero retiene los contadores y la última
evidencia de autenticación/cifrado/desconexión hasta el siguiente Start. Un nuevo
Start borra la sesión. El informe distingue los eventos de seguridad vistos/ausentes.
ROLE es el rol conocido por BTstack, actualizado por las notificaciones Role Change; la
política de aceptación solicitada se registra por separado (traza14). No se registra ninguna dirección, clave ni
payload de paquete. Los handles HCI y los códigos de estado son metadatos de sesión.

## Alcance de las pruebas

El código BTstack real recibe paquetes HCI virtuales, se autentica con el
fixture SPI/import sintético, y negocia mensajes L2CAP/HID reales. Los casos añadidos
ejercitan reconexiones entrantes con páginas de características compatibles SSP/SC, una
colisión saliente/entrante (estado 0B), una primera configuración bloqueada, y la secuencia exacta
auth 00 -> cifrado 08 -> desconexión 08. La evidencia del fallo sobrevive a
Stops repetidos sin dejar ninguna entrada activa. El código v24 anterior se compila
contra la misma ABI de prueba y falla las aserciones precisas de rol de aceptación y de
paging concurrente. Esto verifica los cambios de software; no reproduce
ni explica la causa interna del timeout en el controlador físico.
