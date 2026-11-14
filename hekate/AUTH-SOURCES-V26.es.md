# Nova26 — establecimiento cruzado de los canales HID
**Languages:** [Français](AUTH-SOURCES-V26.md) · [English](AUTH-SOURCES-V26.en.md) · **Español** · [Русский](AUTH-SOURCES-V26.ru.md)

El nuevo informe físico de Nova25 cambia el diagnóstico. HCI 85/46, ACL 5/6,
HOST 1, controlador HCI 08/revisión 2000/fabricante 000F/subversión 2409. Los dos registros
pad contienen eventos de autenticación y cifrado correctos. ROLE 00 aparece
en los eventos Role Change recibidos. Los fallos anteriores auth 05/cifrado 08 no son
la etapa que falla en esta sesión. ENABLED 0 en una sesión ya detenida
no invierte los eventos de cifrado correctos de la traza conservada.

Para el handle 000D, alrededor de 1949–1959 ms, Nova transmite una petición de conexión
PSM 0011 del canal de comandos mientras recibe la petición PSM 0011 del propio interlocutor. Nova responde con el resultado
0004 (rechazo por recursos), y el interlocutor responde pendiente 0001 a la petición
saliente. El interlocutor se desconecta entonces con el motivo 13. La misma petición cruzada y el
rechazo local aparecen para el handle 000B alrededor de 2394–2399 ms. No se recibe ningún informe HID.

El host HID vendorizado rechazaba toda petición de control entrante cuando ya existía un
contexto HID para esa dirección, incluido un contexto saliente todavía
en espera del establecimiento del control. Esta condición también es visible en el
[fuente principal upstream del host HID](https://github.com/bluekitchen/btstack/blob/master/src/classic/hid_host.c).
Explica el rechazo local; no prueba que fuera el único obstáculo en
la consola, ni que la entrada de radio funcione ahora sin una prueba física.

Nova26 cambia solo la vía opcional de modo de informe NOVA_HID_KNOWN_PSMS:

- Una petición de control entrante cruzada toma el control del contexto HID pendiente
  existente, reteniendo su CID HID y pasando por la callback de aceptación de la aplicación
  normal y por los controles de seguridad L2CAP.
- Una petición de interrupción entrante puede igualmente sustituir una apertura de interrupción
  saliente todavía pendiente de finalización. Las peticiones entrantes duplicadas y los canales
  ya establecidos siguen protegidos frente a la sustitución.
- Los eventos CHANNEL_OPENED se emparejan por el CID L2CAP actual y el interlocutor, en lugar de
  por la dirección del interlocutor únicamente. El éxito tardío en un CID no canónico se desconecta; un
  fallo tardío no puede destruir el contexto HID entrante activo.
- Las entradas pendientes sustituidas permanecen bajo la limpieza normal de respuestas/RTX de L2CAP.
  Dos interlocutores necesitan seis entradas dinámicas en el solapamiento probado, frente al pool
  fijo de ocho. Las pruebas verifican que se vuelve a cuatro canales establecidos
  tras fallo tardío, éxito o caducidad, y a cero tras Stop. Las peticiones entrantes
  duplicadas se rechazan y sus entradas temporales se liberan.

No se hace ningún cambio en L2CAP, HCI, el driver de radio, la importación de claves ni el decodificador de entradas. Se
conservan el formato de clave v24 y la preferencia de rol v25, los intentos en serie y los metadatos de sesión
retenidos. Se conserva la seguridad de nivel 2, no se acepta ningún nuevo emparejamiento y
no se carga ningún firmware. El comportamiento genérico del vendor permanece cuando la macro está desactivada.
Se conserva la licencia separada de BTstack; el parche vendor completo de Nova está
incluido en el paquete.

La prueba nativa envía ahora peticiones L2CAP entrantes reales mientras las aperturas salientes
están pendientes, con CIDs locales/remotos generados independientemente. El fuente HID real
de la v25 produce el resultado 0004 y falla la aserción de no rechazo. Nova26
acepta los dos canales y recibe ACK, botones y sticks a través del BTstack real.
Pruebas adicionales envían peticiones duplicadas y éxito/rechazo/timeout RTX tardíos,
controlan el número de canales, y confirman que la sesión activa sigue siendo utilizable.
La versión del controlador virtual refleja la nueva versión/revisión HCI reportada.
Estas pruebas sustituyen la radio y no establecen el funcionamiento físico.
