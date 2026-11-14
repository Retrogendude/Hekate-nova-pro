# Hekate Nova Pro26 — corrección de las aperturas simultáneas de los canales Joy-Con
**Languages:** [Français](LIRE-MOI-V26.md) · [English](LIRE-MOI-V26.en.md) · **Español** · [Русский](LIRE-MOI-V26.ru.md)

Base conservada: Hekate 6.5.5 / Nyx 1.9.5. Interfaz y ajustes Nova conservados.

Tu informe v25 confirma autenticaciones y activaciones del cifrado
correctas, así como los primeros intercambios ACL. El siguiente bloqueo es visible:
Nova y el Joy-Con piden simultáneamente la apertura del canal de mando, y luego
Nova rechaza la petición entrante con 0004. No llega ningún informe de botones.

La v26 acepta la petición entrante cuando un intento saliente sigue
pendiente. Conserva la misma sesión de mando y aísla las respuestas tardías del
intento antiguo. Se controlan los duplicados y se limpian los canales antiguos.
Se conservan la seguridad, el formato de las claves y el ajuste del rol principal.

El rechazo se reprodujo en PC con el código v25. Las pruebas v26 pasan para las
aperturas simultáneas, los comandos, los duplicados, las respuestas tardías y la
limpieza. El funcionamiento real todavía debe confirmarse en tu consola.

## Instalar y probar

1. Copiar el contenido del ZIP en la raíz de la SD sustituyendo los archivos de la
   carpeta **bootloader**. Los archivos de ajustes no se incluyen.
2. Usar **hekate_nova_pro_6.5.5_v26.bin** si es necesario, y luego reiniciar
   completamente.
3. Abrir **Ajustes → TV / Dock → Joy-Con inalámbricos**. Fijar los Joy-Con y
   elegir **Leer los emparejamientos**.
4. Desprender los Joy-Con, activar Bluetooth y pulsar un botón para
   despertarlos. Probar los sticks y los botones A / ZL / ZR.
5. Elegir **Detener Bluetooth** tras la prueba. El informe se queda guardado en
   `bootloader/bluetooth/last-session.txt`.

Los contadores **Informes**, **ACK** y los comandos realmente recibidos son
los indicadores útiles. Tras la detención, HID y ENABLED pueden valer 0: el
servicio está cortado. Los contadores y los últimos eventos se conservan.
SEEN=1 indica que se ha observado un evento; 00 con SEEN=0 no prueba un
éxito. Las direcciones, claves y contenidos brutos de los paquetes no se registran.

El próximo arranque del Bluetooth reinicia el informe de sesión a cero. Las
claves permanecen en memoria; deben releerse tras un reinicio.
Se incluyen la licencia distinta de BTstack y el detalle de las modificaciones.
