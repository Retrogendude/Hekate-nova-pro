# Hekate Nova Pro25 — prueba de reconexión de los Joy-Con
**Languages:** [Français](LIRE-MOI-V25.md) · [English](LIRE-MOI-V25.en.md) · **Español** · [Русский](LIRE-MOI-V25.ru.md)

Base conservada: Hekate 6.5.5 / Nyx 1.9.5. Interfaz y ajustes Nova conservados.

El informe v24 confirma una autenticación correcta para una conexión, y luego
una pérdida de enlace durante la activación del cifrado (08). Se conserva la corrección
del formato de las claves de la v24. Sigue habiendo un problema de conexión.

Esta versión exige que la consola asuma el rol principal cuando un Joy-Con
inicia la reconexión, y lanza los intentos salientes sucesivamente.
Estos cambios están verificados en PC. Su eficacia en la consola sigue
por confirmar: el informe solo no permite atribuir con certeza el
corte al rol Bluetooth.

## Instalar y probar

1. Copiar el contenido del ZIP en la raíz de la SD, sustituyendo los archivos
   de la carpeta **bootloader**. Los archivos de ajustes no se incluyen.
2. Usar **hekate_nova_pro_6.5.5_v25.bin** si es necesario, y luego reiniciar
   completamente la consola.
3. Abrir **Ajustes → TV / Dock → Joy-Con inalámbricos**. Fijar los Joy-Con,
   elegir **Leer los emparejamientos**, y luego desprenderlos y activar Bluetooth.
4. Pulsar una tecla para despertarlos y probar los botones/sticks.
5. Elegir **Detener Bluetooth** tras la prueba. El informe se encuentra en
   `bootloader/bluetooth/last-session.txt`.

No usar únicamente el estado de los LED como criterio: los contadores de
informes y el funcionamiento de los comandos son los indicadores útiles.
Las claves permanecen en memoria y deben releerse tras cada reinicio.

## Nuevo informe

- AUTH=00 SEEN=1: un evento confirma una autenticación correcta.
- ENC=00 SEEN=1 ENABLED=1 o 2: cifrado activado mientras el enlace está activo.
- ENC=08 SEEN=1: el enlace ha caducado durante esta etapa.
- DISC: motivo de desconexión conservado.
- ROLE=00: rol principal conocido por la pila; 01: secundario; FF: desconocido.
  Los cambios de rol recibidos aparecen en la traza13; la política
  solicitada en la aceptación aparece en la traza14.
- HOST=1: identidad del controlador conforme a la de los emparejamientos importados.
- SEEN=0: ningún evento correspondiente observado; 00 solo no prueba un éxito.

Los últimos resultados siguen disponibles tras Detener, incluso si se pulsa
una segunda vez. El próximo arranque del Bluetooth abre una nueva sesión.
El informe incluye la versión del controlador, sin direcciones, claves ni paquetes brutos.

Se conserva la seguridad del enlace. No se realiza ningún nuevo emparejamiento ni escritura de
claves. Se conservan las correcciones anteriores de Nyx, de la dock y de los diagnósticos.
La licencia propia de BTstack se incluye en la documentación.
