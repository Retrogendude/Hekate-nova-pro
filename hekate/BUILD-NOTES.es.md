# Versión actual — Hekate Nova Pro 11 (6 de noviembre de 2026)
**Languages:** [Français](BUILD-NOTES.md) · [English](BUILD-NOTES.en.md) · **Español** · [Русский](BUILD-NOTES.ru.md)

Las doce modificaciones y su funcionamiento se describen en [NOVA-PRO-V11.md](NOVA-PRO-V11.md). La compilación y el conjunto de las comprobaciones v11 están validados. Binario Hekate: 110036 bytes; Nyx: 1924293 bytes; servicio RAM: 77052 bytes. El contenido comprimido del payload final corresponde al nuevo Hekate descomprimido; las dependencias del cargador están corregidas para las compilaciones incrementales.

Los controles comprenden 4 260 escenas LVGL (fuentes, idiomas, dimensiones, disponibilidades de hardware, paletas y transparencias), los ajustes, los errores SD, la carga de imágenes, la animación de arranque y los contratos del servicio RAM. Las sondas de hardware no se ejecutan en la consola durante estos controles en PC.

Las animaciones de fondo se han retirado. Las mediciones del reflejo conciernen a superficies redibujadas por LVGL en ordenador, sin valor de FPS de Switch. Pruebas físicas de esta versión aún por realizar.

---
## Historial — versiones anteriores

# Nova — prototipo 10, 6 de noviembre de 2026

La presentación Nova está implementada en los componentes LVGL nativos:
navegación superior, tarjetas completas, banda de detalle, interruptores con estado
escrito e iconos Lucide 1.8.0 idénticos a las rutas SVG de la maqueta.
Las 32 tarjetas retoman sus textos franceses, con un catálogo de siete
idiomas. La paleta, el radio de 8 píxeles y el halo fijo de 18 píxeles vienen
de esta referencia. El contenido ocupa 1 280 × 580 píxeles, bajo una cabecera
de 76 píxeles y por encima de la barra de estado a 656 píxeles.

Las máscaras alfa de los iconos están precalculadas en tres tamaños; su decodificación
LVGL está activada. Los roles tipográficos se almacenan en caché desde los archivos
NYXF o la fuente integrada. Las callbacks de tarjetas conservan sus identificadores
y sus guardas; la selección es distinta de la apertura de la acción.
Las opciones de arranque, los comandos rápidos y la interfaz Apariencia
siguen accesibles desde los títulos de Ajustes y el logo HEKATE.

Los controles nativos en ordenador cubren 1 890 escenas y doce casos
adicionales de disponibilidad. También verifican los píxeles de los glifos
Lucide, los estados de los interruptores, la cancelación de las elecciones y los errores
de guardado. El halo invalida como máximo su zona local y no mantiene
ningún refresco en reposo. Este resultado no mide los FPS de hardware.
Ver verification/reference-nova.json y verification/performance-halo-nova.json.

Las notas siguientes describen las versiones anteriores. BUILD-INFO.json
da los tamaños, huellas y resultados de la construcción actual.

# Studio — prototipo 9 (histórico)

La versión actual añade una navegación lateral nativa de 224 píxeles,
una cabecera de 64 píxeles y una zona principal de 1 056 × 592 píxeles.
Se conservan los índices de pestañas y las acciones existentes. El tema
antracita/violeta y los ángulos de 6 píxeles se comparten con las ventanas
detalladas. Los iconos son los de los recursos oficiales del proyecto.
El halo de 28 píxeles usa una sombra local en el icono seleccionado,
sin tarea de animación ni asignación por imagen. Los interruptores leen
el estado real del botón, incluso tras cerrar la elección Auto Boot.

La representación LVGL en ordenador verifica la navegación, los interruptores,
las dimensiones en los siete idiomas y las tres fuentes incluidas.
El control del halo limita el dibujado durante un cambio y verifica
la ausencia de refresco en reposo, con los efectos de fondo en pausa.
Las mediciones actuales figuran en verification/performance-halo-studio.json.
Los controles de hardware y el servicio RAM no se han modificado.

Las notas del prototipo 8 siguientes describen las optimizaciones conservadas
y la presentación anterior. BUILD-INFO.json describe la construcción actual.

# Correcciones del prototipo 8 (histórico)

Las pestañas principales son compactas y limitadas por las barras superior/inferior.
El tema emplea un fondo liso y contornos gris/blanco de radio 8 píxeles.
Los colores de las pastillas se representan explícitamente; sus estilos se
reutilizan en lugar de reasignarse en cada apertura.

El driver espera el final de la imagen anterior antes de la primera escritura,
y luego lanza una sola rotación VIC al final del refresco LVGL.
Las fuentes Unicode ordenadas usan una búsqueda binaria; las caras SD
tienen acceso ASCII directo. La búsqueda de los patrones de traducción está indexada
y la búsqueda de fragmentos ya no crea ninguna copia temporal de clave.
Los desplazamientos de animación y los textos dinámicos idénticos ya no redibujan
sus labels sin cambio. No se ha aumentado ninguna frecuencia de hardware.

Los efectos ya no usan veinte objetos móviles. Su trabajo de dibujado
se limita a una banda de 64 píxeles o a dieciséis pequeñas zonas, y se suspende
durante los gestos. Las mediciones de `verification/performance-fonds.json`
son las de la representación por software LVGL en ordenador, y no las de FPS de consola.
Los controles de visualización verifican las presentaciones agrupadas, los controles
visibles y las pastillas. Las pruebas físicas de esta versión quedan por
hacer. Las confirmaciones y las operaciones de hardware se conservan.

# Construcción Windows — prototipo personalizado

Prototipo 9 construido el 5 de noviembre de 2026 a partir de los Makefiles del proyecto.
Las notas de los prototipos anteriores siguientes describen su estado histórico.
Los tamaños actuales y los resultados detallados están en BUILD-INFO.json.

- Objetivo ARM: devkitARM GCC 16.1.0 instalado por devkitPro.
- Herramientas Windows: GCC 16.2.0, GNU Make 4.4.1 y shell BusyBox de w64devkit 2.11.0.
- El archivo w64devkit se obtuvo del repositorio oficial skeeto/w64devkit y
  su SHA-256 se verificó contra los metadatos de publicación de GitHub.
- MSYS2 no pudo crear su canal de señales en el entorno de ejecución.
  La build usó las herramientas Windows portátiles y una copia local de
  `base_tools` sin la reescritura de PATH específica de MSYS2. Se conservan los Makefiles,
  las opciones de compilación ARM y los scripts de enlace del proyecto.
- No se ha instalado ningún software adicional en devkitPro por la build.

Resultados: Hekate sin comprimir 134 529 bytes (límite 140 288), payload final
110 204 bytes (límite 126 296). Nyx comprende ahora el motor de fuentes SD,
los efectos y el catálogo ampliado. Los tamaños actuales de Nyx y del servicio
RAM se consignan en BUILD-INFO.json (servicio: límite 130 392). Los límites se verifican
sobre los archivos finales, incluido el sufijo de seis bytes del payload.

Las salidas ARM son ELF32 ARM EABI5, con puntos de entrada 0x40007000
(cargador) y 0x81000000 (Nyx). Los archivos binarios del cargador y de Nyx
se verificaron contra los ELF enlazados; el payload incluye el sufijo ICTC49.

El compilador emitió avisos no bloqueantes por variables
sin usar de LVGL, una antigua función de botones que ha quedado sin usar y
los segmentos de enlace RWX de los programas embebidos. GNU Make también señala
la reinicialización del paralelismo en las sub-builds.

La copia de seguridad de los ajustes pasó 96 casos C sobre las funciones reales extraídas
de las fuentes, con dobles de la tarjeta SD: modos 0/1, valores inválidos,
conservación de los ajustes anteriores y errores de montaje, apertura, escritura y
cierre. Esto no sustituye las pruebas de hardware.

El prototipo 7 había añadido controles sobre las fuentes SD, las acciones del menú
Apariencia, los efectos y las disposiciones. Se produjeron representaciones con
el verdadero LVGL en ordenador para las tarjetas y la pantalla Apariencia, a 36 píxeles
en francés y en ruso. Los selectores se corrigieron tras la inspección.
Los recursos de la prueba RAM se validan con su verdadero motor y su dibujado
de framebuffer; permanecen en los primeros 4 MiB ya excluidos de la prueba.
Ver PERSONNALISATION.md y TRADUCTIONS.md para el alcance actual.

El catálogo final comprende 3 688 claves. Los complementos de los informes de las
sondas se tradujeron fuera de línea con modelos Argos/CTranslate2;
los menús redactados y 201 entradas revisadas mantienen la prioridad. Los controles
automáticos verifican las 1 268 cadenas origen de las sondas, el orden de los
parámetros, las notaciones numéricas y las rutas. La calidad lingüística
de todos los comentarios automáticos no ha sido validada por hablantes
nativos. Los modelos de traducción no forman parte del paquete SD.

El paquete contiene los recursos precompilados sin cambios de la publicación
oficial Hekate 6.5.4 / Nyx 1.9.4, así como los binarios personalizados
recién construidos. Su procedencia y sus huellas figuran en
BUILD-INFO.json junto a las instrucciones del paquete binario.

Para reconstruir en un entorno devkitPro/MSYS2 funcional con GCC nativo disponible,
el comando estándar sigue siendo `make -j4` en la raíz del proyecto.

## Integración completa del prototipo 5

El programa AArch64 Wi-Fi de 8 128 bytes está incluido y es necesario en la compilación.
La tabla nativa contiene las 42 entradas HWTest-RCM. El servicio RAM usa la
revisión exacta de Atmosphère indicada en DEPENDENCIES.json y conserva una
imagen de retorno Hekate en la zona ya excluida de los primeros 4 MiB de DRAM.

Comprobaciones complementarias sobre el código real: 108 aserciones del lector
limitado de payloads; captura limitada y diez casos de exportación; seis casos de síntesis
con claves ausentes, éxito parcial, avisos y fallos; 28 casos del contrato RAM/retorno.
El decodificador C del cargador reconstituye todos los bytes de
la imagen RAM descomprimida. Ninguna de estas comprobaciones simula las mediciones
físicas de los periféricos de la consola.

Las fuentes del compañero RAM y sus dependencias necesarias se incluyen
con las fuentes de Hekate. En un sistema con devkitARM, devkitA64,
Python 3, Git, GCC nativo y Make, construir Hekate en su raíz, y luego el servicio
RAM en HWTest-RCM-0.15/ramtest. Los scripts Windows de build-tools son
referencias para el entorno portátil utilizado en esta sesión.

## Interfaz e idiomas del prototipo 6

Fondos transparentes en los componentes de navegación; icono Diagnóstico en
la pestaña y en la pantalla de inicio. Siete idiomas, acentos y cirílico: las nuevas
fuentes son bitmaps de 4 bpp de 707 caracteres en las alturas de 20 y 30 píxeles.
La fuente Noto Sans y su licencia SIL OFL se incluyen con su generador.
El paso a UTF-8 conserva los símbolos Hekate y corrige los cortes de
texto que antes usaban los desplazamientos de bytes como números de caracteres.

La copia de seguridad/restauración eMMC usa identificadores de botones estables,
independientes de las etiquetas traducidas. Las matrices de botones conservan sus
bytes de control y liberan sus traducciones al cerrar. Los nombres
de usuario, rutas e informes técnicos no se traducen. El servicio
RAM separado mantiene su propia interfaz en inglés.

Los controles complementarios ejecutan el código C de configuración, acciones del menú
Idioma y del motor de traducción, con errores de asignación
y limpieza de los menús. Las fuentes se controlan con el verdadero lector
LVGL; su vista previa decodificada se ha inspeccionado en ordenador. Estas comprobaciones
no sustituyen la validación visual y de hardware en una Switch.
