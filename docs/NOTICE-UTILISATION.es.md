# Hekate Nova Pro — guía de uso
**Languages:** [Français](NOTICE-UTILISATION.md) · [English](NOTICE-UTILISATION.en.md) · **Español** · [Русский](NOTICE-UTILISATION.ru.md)

Guía para la **beta v32**, sobre base Hekate 6.5.5 / Nyx 1.9.5. Actualización: 14 de noviembre de 2026.

[Paquete de instalación v32](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32) · [Historial](../CHANGELOG.md) · [Guía técnica de la v32](releases/v32.md)

Nova se usa en una consola que ya dispone de un método de arranque compatible con Hekate. El método de lanzamiento depende del modelo y de la instalación existente.

## Instalación y primer arranque

1. En los archivos de la [release v32](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32), descargar **hekate-nova-pro-v32.zip** y descomprimirlo en el ordenador.
2. Con la consola apagada, conservar una copia de la carpeta bootloader y de los ajustes de la tarjeta SD.
3. Copiar las carpetas suministradas **bootloader**, **fonts**, **wallpapers** e **icone** a la raíz de la SD. Fusionar las carpetas existentes y reemplazar los archivos entregados por la actualización.
4. Conservar las configuraciones de arranque, los fondos y fuentes personales, así como los archivos privados de emparejamiento. No es necesario ningún formateo para instalar esta actualización.
5. Usar **hekate_nova_pro_6.5.5_v32.bin** con el método habitual si este requiere un payload externo. También se suministra el archivo **bootloader/update.bin** correspondiente.
6. Volver a poner la SD en la consola y arrancar Nova. El payload y el archivo bootloader/sys/nyx.bin deben pertenecer a un conjunto coherente.

En el primer arranque, el ajuste de fecha y hora original se acompaña de la elección de idioma. El fondo Carbon, los colores Nova y las ventanas al 75 % de transparencia son los valores por defecto de una instalación nueva. Las preferencias ya guardadas tienen prioridad durante una actualización.

### Organización de la tarjeta SD

```text
Raíz de la tarjeta SD/
├── bootloader/
│   ├── sys/
│   └── update.bin
├── fonts/
│   ├── Lecture.nyxf
│   ├── Lecture-renforcee.nyxf
│   ├── Lecture-fixe.nyxf
│   └── MaPolice.nyxf          # opcional
├── wallpapers/
│   ├── Nova-Carbon.bmp
│   └── MonFond.bmp            # opcional
└── icone/
    └── diagnostic.png        # ejemplo opcional
```

Los archivos MaPolice.nyxf, MonFond.bmp y diagnostic.png anteriores son ejemplos que hay que añadir uno mismo.

## Orientarse en los menús

| Menú | Uso |
|---|---|
| Inicio | Configuraciones de arranque, payloads, emuMMC y acceso a los diagnósticos. |
| Herramientas | Almacenamiento, copia de seguridad/restauración, acceso USB y particionado. |
| Información | Información de hardware y del sistema. |
| Diagnóstico | Categorías de comprobaciones, pruebas individuales o completas, resultados y exportación. |
| Ajustes | Idioma, lectura, apariencia y opciones del sistema. |

En las rejillas **Herramientas, Información y Diagnóstico**, una primera pulsación selecciona el mosaico y muestra su detalle; una segunda pulsación sobre el mismo mosaico lo abre. El icono de ajustes rápidos de la parte superior derecha da acceso a los ajustes de Nyx.

Para arrancar un sistema, abrir **Iniciar** y elegir una configuración ya presente en la SD. Las entradas mostradas dependen de los archivos de arranque de la instalación existente.

Después de una personalización, usar **Guardar y recargar** para aplicar y conservar las elecciones. Los cambios propios del logo de arranque son visibles en el siguiente arranque de Hekate.

## Añadir y elegir un fondo de pantalla

1. Copiar las imágenes en la carpeta **wallpapers** de la raíz de la SD.
2. Usar **BMP sin comprimir, de 24 o 32 bits**, de como máximo **1920 × 1080 píxeles**.
3. Elegir un nombre simple, por ejemplo **MonFond.bmp**, sin acentos ni subcarpeta.
4. En **Ajustes Nyx → Temas y colores → Fondo y transparencia**, seleccionar el fondo deseado.
5. Ajustar la opacidad y después elegir **Guardar y recargar**.

El fondo **Nova-Carbon.bmp** está incluido. Las imágenes se recortan y se escalan al cargarse. El selector presenta hasta 32 fondos.

Las ventanas y los iconos disponen de ajustes de opacidad distintos:

- **0 % de opacidad**: fondo transparente.
- **100 % de opacidad**: fondo opaco.
- **25 % de opacidad** corresponde a **75 % de transparencia**.

El texto y los símbolos siguen visibles sobre los fondos. Las animaciones de fondo no forman parte de la v32; el reflejo de selección sigue pudiendo desactivarse en los ajustes.

### Convertir una imagen PNG o JPG

En un ordenador Windows, conservar juntos los archivos de la carpeta **outils-fonds** suministrada en el archivo, después arrastrar una imagen PNG o JPG sobre **convertir-fond.cmd**. Copiar el BMP obtenido en la carpeta wallpapers de la SD.

Este conversor requiere Python 3 y Pillow. Su preparación se describe más abajo.

## Elegir y añadir una fuente

Se suministran tres fuentes compatibles en **fonts**, en la raíz del paquete:

| Archivo | Presentación |
|---|---|
| Lecture.nyxf | Fuente regular, derivada de Noto Sans. |
| Lecture-renforcee.nyxf | Trazo reforzado para el contraste. |
| Lecture-fixe.nyxf | Caracteres de anchura fija. |

En **Ajustes → Apariencia y lectura**, elegir la fuente y el tamaño del texto, y después **Guardar y recargar**. Los tamaños principales propuestos van de 20 a 36 píxeles; las etiquetas pequeñas usan un tamaño adaptado. Los informes técnicos conservan una fuente de paso fijo para alinear sus columnas.

### Añadir su propia fuente

1. En el ordenador, extraer la carpeta **outils-polices** completa. Conservar juntos convertir-police.cmd y sus subcarpetas **tools** y **source**.
2. Arrastrar el archivo **TTF u OTF** sobre **outils-polices/convertir-police.cmd**.
3. Recuperar el archivo **.nyxf** creado junto a la fuente original.
4. Copiar este archivo en **fonts**, en la raíz de la SD, con un nombre simple como **MaPolice.nyxf**.
5. Recargar Nyx para actualizar la lista, seleccionar la fuente y guardar.

La consola carga los archivos **NYXF**; por tanto, un archivo TTF u OTF debe convertirse antes de añadirlo. El conversor produce los tamaños previstos y usa Noto Sans como respaldo para los caracteres que faltan. Su cobertura corresponde a los alfabetos latinos, al cirílico y a la puntuación necesarios para los siete idiomas propuestos; ello no constituye una fuente universal para todos los sistemas de escritura.

El selector acepta hasta 16 archivos adicionales. Emplear un nombre de menos de 64 caracteres, compuesto por letras sin acentos, cifras, espacios, guiones, puntos o guiones bajos. Un archivo ausente o no válido provoca el uso de la fuente integrada.

Usar fuentes cuya licencia permita la conversión y, en caso de compartir, la redistribución. Las variantes Lecture suministradas van acompañadas de la licencia SIL OFL de su fuente.

### Preparar los conversores en Windows

Los dos conversores, fondos de pantalla y fuentes, requieren **Python 3** y **Pillow** en el ordenador. Una vez que Python esté disponible, instalar Pillow en un terminal con:

```text
python -m pip install Pillow
```

Después usar los programas .cmd suministrados arrastrando y soltando. No se necesita ningún conversor ni Python en la consola. Las fuentes y el fondo Carbon ya suministrados están listos para su uso.

## Personalizar los iconos y el mosaico Iniciar

Para reemplazar los cinco pictogramas pequeños de la pantalla de inicio, copiar PNG con transparencia en **icone**, con estos nombres:

| Nombre del archivo | Mosaico |
|---|---|
| launch.png | Iniciar |
| configs.png | Configuraciones |
| payloads.png | Payloads |
| emummc.png | emuMMC |
| diagnostic.png | Diagnóstico |

Preferir una imagen cuadrada con márgenes transparentes. Se conservan las proporciones. Las imágenes deben mantenerse dentro de los límites de **1920 × 1080 píxeles** y **8 MiB**. Recargar Nyx después de modificarlas; un icono ausente o no válido deja el pictograma Nova visible.

Para la imagen del gran mosaico Iniciar / Launch, copiar una imagen distinta en icone y después elegirla en **Ajustes Nyx → Temas y colores → Mosaico Launch**. **Ninguna imagen** restaura el fondo normal de este mosaico. El fondo de pantalla general se sigue eligiendo por separado desde wallpapers y usa los BMP.

## Diagnósticos y prueba de memoria

En Diagnóstico, seleccionar una categoría y después lanzar la comprobación deseada o la prueba completa. Los resultados y el balance por componente pueden consultarse y exportarse desde las pantallas de resultados. Los informes se guardan en **bootloader/diagnostics**.

Las comprobaciones interactivas permiten, entre otras cosas, verificar botones/sticks, cobertura del táctil y mires de pantalla. La prueba de memoria se ejecuta en un programa separado con regreso a Hekate; elegir su modo y después exportar los resultados al volver. Un hardware inestable puede impedir el regreso normal.

Un veredicto PASS solo concierne a las comprobaciones realmente ejecutadas. Una identificación correcta de un chip no certifica todas sus funciones. El balance distingue resultados medidos, comprobaciones no efectuadas y observaciones manuales.

## Particionado experto

El modo experto propone tres filas: emuMMC, Linux y Android. Los tamaños se colocan a la derecha de las barras; los botones **−/+** afinan el ajuste, con repetición si se mantiene pulsado.

- Las marcas **29 GB Full** y **58 GB Full** conservan las asignaciones completas de Hekate según el modelo.
- Los tamaños mostrados son enteros; las reservas y alineaciones siguen contándose en los cálculos.
- Android muestra su asignación total, sistema incluido.
- El resumen inferior agrupa las asignaciones y su estado en una línea.

El particionado modifica el soporte después de las confirmaciones previstas por Hekate. Conservar una copia de seguridad de los datos antes de esta operación. Instalar o actualizar Nova no lanza ningún particionado.

![Particionado experto Nova v32, renderizado del código en PC](images/partitionnement-v32.png)

## TV y Joy-Con inalámbricos

### Salida TV

En los ajustes de Nyx, abrir la pestaña **TV / Dock** y después los ajustes **USB-C TV / Dock**. Elegir la visualización en el televisor solo o en el televisor y la consola, así como la opción de detección automática del dock.

La conexión de referencia es una consola compatible insertada directamente en un dock oficial alimentado, con HDMI conectado al televisor. La **Switch Lite no dispone de salida TV**. La validación de hardware depende del modelo y de la conexión; las guías de las versiones TV describen los ensayos conservados.

### Primer uso de Bluetooth

1. Usar Joy-Con ya emparejados con esta consola y después fijarlos en sus raíles.
2. Abrir **Joy-Con inalámbricos** desde los ajustes TV / Dock y elegir **Leer los emparejamientos**.
3. Después de una importación válida, con Bluetooth detenido, elegir **Guardar los emparejamientos**.
4. Separar los Joy-Con, elegir **Activar Bluetooth** y pulsar un botón para despertarlos. Dejar los sticks en reposo un instante antes de navegar.

En los arranques siguientes, si ambos raíles están vacíos y se ha guardado un archivo de emparejamiento válido, Nova puede recargarlo y arrancar el servicio automáticamente, sin abrir su ventana. La presencia de un Joy-Con en un raíl impide este automatismo. Una parada manual se sigue respetando para este arranque. Algunas herramientas y diagnósticos reservan los periféricos y detienen el servicio: volver a lanzarlo manualmente después si es necesario.

La v32 vuelve a pedir los informes de un Joy-Con que se ha vuelto silencioso y puede reconectar solo ese par. **El alcance de 2–3 metros todavía no está garantizado y queda por validar en consola.**

### Notificar un problema

Después del ensayo, elegir **Detener Bluetooth** y conservar inmediatamente **bootloader/bluetooth/last-session.txt**, antes de relanzar o de volver a leer los emparejamientos. Indicar la versión, el modelo, la distancia y la presencia o no de un dock.

Transmitir únicamente el informe de texto. **No publicar joycon-pairing.bin, joycon-pairing.bak, prod.keys, title.keys ni volcados personales.** Las copias de seguridad de emparejamiento contienen claves privadas; no están incluidas en los archivos del proyecto.

## Fuentes y límites

Esta guía describe la v32. Las guías históricas y los resultados de verificación permanecen en [los documentos de cada versión](releases/v32.md) y en [las releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Las comprobaciones en PC y con periféricos simulados no reemplazan una validación en la consola real.
