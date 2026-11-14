# Hekate Nova Pro
**Languages:** [Français](README.md) · [English](README.en.md) · **Español** · [Русский](README.ru.md)

Personalización experimental de Hekate / Nyx: interfaz Nova, diagnósticos de hardware, personalización y compatibilidad con el dock y los Joy-Con inalámbricos. Proyecto no oficial; los autores de los componentes originales conservan sus derechos y su atribución.

## Descargar

**[Beta v32: archivos y fuentes](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32)** · [Todas las versiones](https://github.com/Retrogendude/Hekate-nova-pro/releases) · [Historial completo](CHANGELOG.md) · [Guía de uso](docs/NOTICE-UTILISATION.md)

La v32 mejora el particionado experto y la recuperación de un Joy-Con silencioso. Las pruebas en PC y la compilación están verificadas; **el alcance inalámbrico de 2–3 metros queda por validar en consola**. Las primeras versiones se conservan por su valor histórico y no son actualizaciones recomendadas.

![Menú de particionado v32, renderizado del código en PC](docs/images/partitionnement-v32.png)

## Funciones

- Inicio Nova en mosaicos o carrusel, ajustes repartidos en pestañas y ventanas adaptadas a los bandeados (banners).
- Siete idiomas: inglés, francés, español, italiano, ruso, alemán y portugués; fuentes de la SD y tamaño de texto ajustable.
- Colores independientes, paleta con vista previa, fondo de pantalla, transparencia, iconos PNG y reflejo de selección que puede desactivarse.
- Diagnósticos nativos procedentes de HWTest-RCM: 42 entradas, informes exportables, balance por componente, comprobaciones interactivas y prueba de RAM separada con regreso a Hekate.
- Particionado simplificado / experto, lectura de los tamaños en GB, reservas y confirmaciones de Hekate conservadas.
- Salida TV mediante dock en consolas compatibles: solo TV o TV + pantalla de la consola. La Switch Lite no tiene salida TV.
- Joy-Con por Bluetooth: lectura de los emparejamientos, copia de seguridad privada en SD, carga automática sin Joy-Con en los raíles y procesamiento en segundo plano. La conexión y el alcance siguen siendo experimentales.

## Instalación

**[Abrir la guía completa](docs/NOTICE-UTILISATION.md)**: instalación, primeros ajustes, fondos de pantalla, fuentes, iconos, diagnósticos, dock y Joy-Con inalámbricos.

Descargar el paquete compilado de la versión elegida en Releases. Con la consola apagada, conservar una copia del bootloader y después fusionar las carpetas suministradas con la raíz de la SD, reemplazando únicamente los archivos entregados. Mantener las configuraciones y los emparejamientos personales. Usar el payload correspondiente a la versión, o el bootloader/update.bin suministrado, según el método de arranque.

Las instrucciones propias de cada versión, sus límites y sus verificaciones figuran en docs/releases y en su archivo. Las versiones v1 a v5 se conservan únicamente como fuentes, entre los archivos disponibles aquí.

## Fuentes y compilación

La carpeta hekate contiene las fuentes de Hekate / Nyx de la instantánea seleccionada. Los archivos de fuentes originales siguen adjuntos a cada release, junto con los scripts de compilación y de verificación asociados. Conservan su disposición histórica y constituyen la referencia para reproducir una entrega antigua.

En el repositorio: entrar en hekate y después usar los Makefiles del proyecto. Las herramientas necesarias incluyen devkitARM, devkitA64, GNU Make, Python 3 y un compilador de C nativo para los componentes que lo requieren. El compañero HWTest-RCM-0.15 y las herramientas complementarias se conservan allí donde estaban presentes en el archivo. Véanse BUILDING.txt y la guía de la versión para las particularidades de Windows.

Los directorios de trabajo locales, SDK y datos de consola no se importan. Los archivos compilados son assets de Releases, no archivos ZIP añadidos a cada commit de Git.

## Historial y estado de las pruebas

Se conservan 34 instantáneas: v1 a v32 y dos ensayos TV autónomos. Las etiquetas y los commits se reconstituyeron a partir de los archivos en el momento de su publicación; no pretenden restituir las fechas ni los commits originales del desarrollo. Se encontró una copia de seguridad de las fuentes v15 y se comparó con las huellas de las fuentes modificadas conservadas en las evidencias de esa versión; los dos archivos de payload generados no forman parte de este archivo de fuentes.

Las vistas previas nativas, las pruebas con periféricos simulados y los comentarios desde consola se distinguen en las guías. Un veredicto PASS solo cubre las comprobaciones realmente ejecutadas. Los resultados de RAM, táctil y pantalla tienen sus propios límites, detallados en la documentación.

## Orígenes, créditos y licencias

- [Hekate / Nyx — CTCaer](https://github.com/CTCaer/hekate) y colaboradores: licencia original conservada en hekate/LICENSE; README original (upstream) conservado en hekate/README.md.
- [HWTest-RCM — Creased](https://github.com/Creased/HWTest-RCM): sondas, compañero y comentarios de procedencia conservados en los archivos correspondientes. Véase hekate/nyx/nyx_gui/diagnostics/PROVENANCE.md.
- [BTstack — BlueKitchen](https://github.com/bluekitchen/btstack): licencia separada conservada en hekate/nyx/nyx_gui/libs/btstack/LICENSE, que incluye una restricción de uso personal y no comercial. Este componente no se presenta como cubierto por la única licencia GPL de Hekate.
- LodePNG, iconos, fuentes y otros componentes conservan sus propios avisos en las fuentes y los paquetes.

No se reclama ninguna afiliación con Nintendo, CTCaer, Creased ni BlueKitchen. Las licencias propias de los componentes y los avisos originales siguen siendo aplicables.

## Informes de Bluetooth

Para notificar un problema, detener el servicio Bluetooth después del ensayo y conservar bootloader/bluetooth/last-session.txt. Indicar la versión, el modelo, la distancia y si se usa un dock o no. **No publicar los archivos joycon-pairing.bin / .bak, prod.keys, title.keys ni volcados personales.**
