# Hekate Nova Pro27 — perfil Bluetooth adaptado de HWTest-RCM
**Languages:** [Français](LIRE-MOI-V27.md) · [English](LIRE-MOI-V27.en.md) · **Español** · [Русский](LIRE-MOI-V27.ru.md)

La v27 toma los ajustes del servicio de mandos inalámbricos de HWTest-RCM:
velocidad de radio básica, gestión de suspensión, ritmo de transmisión y lectura serie
prolongada. El arranque usa también sus ajustes de alimentación y de reloj,
con restauración del estado anterior en la detención.

Se conservan la interfaz Nova, los dos Joy-Con, las claves importadas, la seguridad y las
correcciones de conexión anteriores. Los diagnósticos Wi-Fi y
Bluetooth ya integrados siguen en su sitio. Base: Hekate 6.5.5 / Nyx 1.9.5.

Compilación y pruebas de software correctas. El funcionamiento en consola sigue
por confirmar. El detalle de las adaptaciones figura en HWTEST-BLUETOOTH-SOURCES.md.

1. Copiar el ZIP en la raíz de la SD sustituyendo los archivos **bootloader**.
2. Usar **hekate_nova_pro_6.5.5_v27.bin** si es necesario, y luego reiniciar.
3. Fijar los Joy-Con y elegir **Leer los emparejamientos** en su menú inalámbrico.
4. Desprenderlos, activar Bluetooth, pulsar un botón y probar los comandos.
5. Elegir **Detener Bluetooth** tras la prueba para guardar
   `bootloader/bluetooth/last-session.txt`.

El informe añade BR_ONLY, los cambios de modo y estadísticas de
recepción. Las configuraciones agrupadas ahora se detallan todas,
sin direcciones, claves ni datos brutos. Tras Stop, HID/ENABLED pueden valer 0;
los contadores de informes y los últimos resultados se conservan.

Los ajustes de usuario no se incluyen en el ZIP. Las claves permanecen
en memoria y deben releerse tras el reinicio. Se incluyen las licencias existentes,
incluida la de BTstack.
