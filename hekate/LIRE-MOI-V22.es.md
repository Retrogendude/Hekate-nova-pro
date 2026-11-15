# Hekate Nova Pro 22
**Languages:** [Français](LIRE-MOI-V22.md) · [English](LIRE-MOI-V22.en.md) · **Español** · [Русский](LIRE-MOI-V22.ru.md)

Base funcional actualizada: Hekate 6.5.5 / Nyx 1.9.5, con el estilo Nova.
Esta versión corrige el aviso erróneo de Nyx y varios bloqueos posibles
de los comandos inalámbricos. El enlace Bluetooth sigue siendo experimental.

## Instalar

1. Conserva una copia de tu carpeta bootloader actual.
2. Copia la carpeta bootloader incluida en la raíz de la tarjeta SD y acepta
   la sustitución de los archivos. No es necesario eliminar la carpeta.
3. Inyecta hekate_nova_pro_6.5.5_v22.bin, o usa el bootloader/update.bin incluido.
4. Comprueba «22» en los créditos y «Hekate 6.5.5 / Nyx 1.9.5».

Los archivos personales nyx.ini y hekate_ipl.ini no se incluyen en este
paquete. Tus ajustes, temas, fondos, fuentes y configuraciones de arranque se
conservan. Sustituir solo el payload sin actualizar nyx.bin no es suficiente.

## Aviso «Old Nyx GUI found»

La v21 entregada colocaba los metadatos de Nyx en una posición incorrecta dentro del binario.
Eso provocaba un falso aviso incluso cuando todos los archivos buenos estaban
presentes. La entrada de Nyx ahora está aislada, y la compilación se rechaza si
cambia la colocación de los metadatos. El control de compatibilidad de Hekate sigue
activo: una mezcla real de versiones todavía puede provocar este aviso.

## Actualización Hekate / Nyx

Los cambios oficiales relativos a HOS 23.5.0 se incorporan en el núcleo de Nova,
con el parche NOGC para FAT32 publicado después de la etiqueta 6.5.5. Los paneles, iconos,
colores y disposiciones Nova siguen siendo los de la versión anterior.

El módulo emuMMC y el componente L4T sc7exit_b01.bin provienen del paquete oficial
6.5.5 actualizado cuya huella se ha comprobado. Nyx y el payload siguen siendo los
binarios Nova compilados para esta versión, y no se sustituyen por la interfaz
oficial. Se conservan los recursos Nova y los diagnósticos.

Referencias del port:
- [Publicación Hekate 6.5.5 / Nyx 1.9.5](https://github.com/CTCaer/hekate/releases/tag/v6.5.5)
- Código de la etiqueta: 9593272c54f8dc127a5a25f91e1ac90e1be6205d.
- Parche FAT32 final: 8e1392df4569ac501a115867056b13c361429e9e.

## Joy-Con inalámbricos

En Ajustes de Nyx → TV / Dock → Joy-Con inalámbricos:

1. Fija los Joy-Con ya emparejados con esta consola y luego elige Leer los emparejamientos.
2. Comprueba el número importado, desprende los Joy-Con y elige Activar Bluetooth.
3. Despierta los Joy-Con con un botón, deja los sticks en reposo un momento,
   y luego mueve el stick y comprueba A / ZL / ZR.

Las claves permanecen en memoria durante la sesión. No se escriben en la SD
y los emparejamientos almacenados en los Joy-Con no se modifican. El servicio sigue
desactivado en cada arranque. Se detiene antes de los diagnósticos, del modo USB y
del lanzamiento de otro sistema; relánzalo manualmente tras un diagnóstico.

Correcciones de navegación:
- Si el lado elegido está ausente, el stick del otro Joy-Con conectado puede pilotar
  el cursor. Se conserva el ajuste guardado del lado preferido.
- A / ZL / ZR despiertan el cursor oculto y siguen utilizables durante el centrado.
- Se decodifican los informes simples 0x3F, además de 0x21 y 0x30. Se sigue la confirmación
  del modo completo, y se sueltan los comandos antiguos.

Los contadores HCI solos no prueban que se reciban los botones. Una línea de estado
adicional permite ahora distinguir:

| Campo | Lectura |
| --- | --- |
| Informes Joy-Con | Número de informes realmente decodificados, izquierda / derecha |
| ID | Tipo de informe, generalmente 30 en modo completo, 3F en modo simple |
| HID | 1 cuando la conexión del mando está abierta |
| ACK | 1 cuando el modo completo está confirmado o se recibe un informe 30 |
| ERR | Último código de fallo de apertura de la conexión, 00 si no hay error registrado |
| BTN | Botones decodificados; el valor debe cambiar durante una pulsación |
| XY | Valores de los sticks; aproximadamente 2048,2048 en reposo en modo completo |

Si los comandos siguen sin responder, anota esta línea manteniendo A,
y luego moviendo el stick. Esto permite separar un problema de conexión, de
decodificación y de navegación. Ningún fallo del prototipo constituye por sí solo un
diagnóstico de fallo de hardware.

## Comprobaciones y límites

La compilación ARM, las posiciones y versiones de las cabeceras, las rutas reales
de decodificación / fusión / puntero hasta el clic LVGL, los menús y las regresiones
TV se controlan en PC con entradas de hardware sustituidas. Dos bloqueos de
navegación se reprodujeron en el código v21 antes de comprobar las correcciones.
El programa de verificación también rechaza una entrada Nyx que desplaza la cabecera.

Sigue siendo necesaria una validación física de esta v22 en tu consola: las pruebas
en PC no prueban una conexión de radio, un arranque de HOS ni una salida HDMI real.
Los ajustes del driver de vídeo y la detección de la dock siguen siendo los de Nova v20/v21.

## Licencias

Esta versión conserva los créditos y licencias de los componentes. BTstack está sujeto
a la licencia personal y no comercial BlueKitchen adjunta al paquete; por lo tanto,
no se presentan todos los componentes como bajo GPL. No se incluye ningún firmware
Bluetooth propietario. Consulta también los documentos de las versiones anteriores.

Referencia del formato simple Joy-Con: [driver SDL Switch](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_switch.c),
para las orientaciones izquierda/derecha en uso vertical. No se incorpora ningún código SDL
en la pila; el decodificador Nova usa los formatos documentados.
