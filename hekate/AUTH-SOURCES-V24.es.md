# Autenticación Joy-Con: corrección del orden de bytes
**Languages:** [Français](AUTH-SOURCES-V24.md) · [English](AUTH-SOURCES-V24.en.md) · **Español** · [Русский](AUTH-SOURCES-V24.ru.md)

El informe físico v23 muestra un estado de autenticación 0x05 tras un comando
Link_Key_Request_Reply correcto, y ningún tráfico ACL. No muestra un fallo del
mapeado de entradas. El otro lado agota el tiempo de espera con 0x08; eso solo no puede identificar
su causa raíz.

El antiguo importador de Nova invertía `jc_bt_conn_t.ltk` una segunda vez. Esto se basaba
en una suposición incorrecta sobre la representación de la clave almacenada en caché. El BDK ya
invierte el campo SPI en bruto al construir esa caché. Nova24 copia el
valor almacenado en caché, tal como hace la cadena establecida de exportación/consumo de Hekate:

1. [BDK de Hekate](https://github.com/CTCaer/hekate/blob/master/bdk/input/joycon.c)
   prepara la caché. El `gui_options.c` de Hekate exporta esa secuencia almacenada en caché como
   la cadena hex `ltk` en `joycon_mac.ini`.
2. [init de Switchroot](https://gitlab.com/switchroot/kernel/l4t-initramfs/-/blob/master/init)
   lee esa cadena y la escribe directamente en el campo `[LinkKey] Key` de BlueZ,
   usando Type=4. Toma la identidad del host del mismo registro.
3. [adapter.c de BlueZ](https://github.com/bluez/bluez/blob/master/src/adapter.c)
   `get_key_info` / `str2buf` decodifican pares hex consecutivos, sin invertir;
   `load_link_keys` los copia a la petición de gestión.
4. Linux [mgmt.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/mgmt.c)
   y [hci_core.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/hci_core.c)
   cargan y retienen los bytes; [hci_event.c](https://github.com/torvalds/linux/blob/master/net/bluetooth/hci_event.c)
   `hci_link_key_request_evt` los copia a la respuesta HCI sin invertir.
5. El `hci_cmd.c` del BTstack vendorizado, formato `P`, copia una clave de 16 bytes sin cambios.
   Su base de datos de claves usa la misma representación en el cable. Se conserva Type=4.

Las fuentes se inspeccionaron el 2026-11-13. Los hashes de la instantánea de referencia se incluyen
en el registro de verificación; no se recogen claves ni direcciones de consola.

La nueva regresión nativa ejecuta el parser SPI real del BDK, la importación real de la GUI
de Nova y la negociación HCI/L2CAP/HID real de BTstack. Claves SPI inventadas no palindrómicas
se comparan con bytes en el cable esperados e independientes. El importador v23
falla esa comprobación. Invertir deliberadamente la clave en el interlocutor virtual reproduce
la autenticación 0x05 con ACL 0/0. Se ejercitan perfiles de controlador
normales y compatibles con SSP/SC. Son pruebas de software, no prueba de un enlace físico funcional.

Nova24 también verifica Read_BD_ADDR contra el host importado antes de permitir
conexiones. Solo se registra la coincidencia/la diferencia. El error 9 significa que falló la verificación de identidad.
AUTH registra la autenticación por separado de los errores HID posteriores.
No se crea ningún emparejamiento, no se escribe ninguna clave, y se mantiene el nivel de seguridad 2 de L2CAP.
