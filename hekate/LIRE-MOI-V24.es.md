# Hekate Nova Pro 24 — corrección de la importación de las claves Joy-Con
**Languages:** [Français](LIRE-MOI-V24.md) · [English](LIRE-MOI-V24.en.md) · **Español** · [Русский](LIRE-MOI-V24.ru.md)

Base: Hekate 6.5.5 / Nyx 1.9.5. Interfaz Nova y ajustes conservados.

El informe v23 mostraba un rechazo de autenticación 05 y ningún intercambio ACL.
Se identificó un error en Nova: una segunda inversión de los bytes de
la clave ya preparada por Hekate. Se elimina en esta versión.
La comparación con la ruta de importación de Switchroot se describe en
AUTH-SOURCES-V24.md. Se conservan el tipo de clave y la seguridad.

La compilación y las pruebas de software pasan. El funcionamiento de los comandos
inalámbricos todavía debe confirmarse en una consola.

## Instalación y prueba

1. Copiar el contenido del archivo en la raíz de la SD sustituyendo los archivos
   de la carpeta **bootloader**. Los archivos de ajustes no se incluyen.
2. Usar el payload **hekate_nova_pro_6.5.5_v24.bin** si es necesario.
3. Reiniciar completamente la consola. En Nyx, abrir **Ajustes → TV / Dock →
   Joy-Con inalámbricos**.
4. Fijar los dos Joy-Con y elegir **Leer los emparejamientos**.
5. Desprender los Joy-Con, elegir **Activar Bluetooth** y luego pulsar una tecla.
6. Comprobar que HID e Informes aumentan, y luego probar el stick y A / ZL / ZR.
7. Elegir **Detener Bluetooth** para guardar el nuevo informe:
   `bootloader/bluetooth/last-session.txt`.

Hay que volver a leer los emparejamientos tras el reinicio: las claves permanecen únicamente
en memoria. No es necesario eliminar los ajustes ni los emparejamientos
existentes. El servicio no crea un nuevo emparejamiento.

El informe añade **HOST=1** cuando la identidad Bluetooth releída corresponde a
la esperada. **HOST=2 / error 9** señala una identidad distinta e interrumpe
la conexión. **AUTH=05** indica un rechazo de autenticación; **AUTH=00** no
es un éxito salvo si se ha producido realmente un evento de autenticación
(visible en la traza). HID e Informes siguen siendo los indicadores de los comandos.
Las claves, direcciones y contenidos brutos de los paquetes nunca se registran.

Se conservan las correcciones anteriores de la cabecera Nyx, de la dock y de los diagnósticos.
BTstack mantiene su licencia personal/no comercial distinta,
incluida en la documentación del paquete.
