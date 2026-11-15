# Inicio actual — Hekate Nova Pro 11
**Languages:** [Français](INTERFACE-ACCUEIL.md) · [English](INTERFACE-ACCUEIL.en.md) · **Español** · [Русский](INTERFACE-ACCUEIL.ru.md)

La maqueta Nova se conserva con las doce modificaciones solicitadas: navegación a la izquierda, cuatro controles a la derecha, porcentaje de batería junto a su icono, nuevo logo y reflejo de selección. Las insignias de cabecera se han retirado. Herramientas, Información y Diagnóstico usan dos pulsaciones en la tarjeta, sin flecha de apertura. Ver [NOVA-PRO-V11.md](NOVA-PRO-V11.md).

---
## Historial — inicio de las versiones anteriores

# Nova — versión 10

Nova usa la navegación superior de la maqueta, los mismos iconos Lucide,
la paleta antracita/azul, tarjetas rellenas de radio 8 píxeles y un halo
de selección fijo de 18 píxeles. El inicio Tarjetas retoma la gran tarjeta
Iniciar y los cuatro accesos secundarios. Herramientas e Información están en cuatro
columnas; Diagnóstico tiene doce tarjetas y una banda de detalle.

Los textos y nombres de iconos de las 32 tarjetas vienen directamente de la maqueta.
Los datos de batería, las herramientas disponibles y la fuente aplicada son
los de la consola. Las vistas previas en ordenador usan datos de ejemplo.
Ver LIRE-MOI-V10.txt para los comandos y PERSONNALISATION.md para los ajustes.

# Studio — versión 9 (histórico)

Studio conserva Tarjetas y Carrusel en un tema antracita/violeta,
con navegación lateral, iconos enmarcados, ángulos de 6 píxeles y un halo
fijo de 28 píxeles en la selección. Los interruptores muestran su estado
por su posición y su texto traducido. Las páginas principales usan
1 056 × 592 píxeles, entre la cabecera y la barra de estado, sin desplazamiento vertical. Ajustes de Nyx
y Apariencia se reparten en pestañas internas; las ayudas están detrás de «?».
Ver PERSONNALISATION.md para el uso y BUILD-INFO.json para las pruebas.
Las notas siguientes describen las etapas anteriores de la interfaz.

# Hekate 6.5.4 — interfaz e idiomas, prototipo 7

Esta versión integra dos disposiciones del inicio de Nyx, seleccionables
en los ajustes. Las funciones Hekate y la nueva pestaña Diagnóstico siguen
accesibles en los dos modos.

## Elegir la disposición

1. Abrir los ajustes de Nyx desde el inicio.
2. Abrir «Apariencia» y luego la sección «Disposición del inicio».
3. Elegir «Tarjetas» o «Carrusel».
4. Usar «Guardar y recargar» para aplicar la elección.

Cerrar el selector sin guardar conserva la disposición anterior.
La elección se almacena en la sección `[config]` de `bootloader/nyx.ini`:

```ini
home_layout=0
```

`0` selecciona las tarjetas y `1` el carrusel. Un ajuste ausente o inválido
usa las tarjetas. El ajuste existente `homescreen` sigue siendo independiente:
para mostrar el inicio al arranque, elegir «Main menu» en «Home Screen».

## Los dos modos

**Tarjetas**: un gran comando de arranque y cuatro accesos a las configuraciones
adicionales, payloads, gestión emuMMC y Diagnóstico.

**Carrusel**: las tarjetas se desplazan en una zona horizontal. Las flechas
o una tarjeta visible cambian la selección; «Abrir» lanza únicamente la
función elegida. Una selección nunca lanza un payload automáticamente.
El modo no promete una navegación directa con la cruceta:
los Joy-Con conservan el funcionamiento de puntero ya presente en Nyx.

Los dos modos usan formas y fuentes nativas, sin imágenes
adicionales que colocar en la tarjeta SD. Un icono de chip identifica
Diagnóstico en la barra de pestañas y en su tarjeta de inicio.

## Idiomas

Abrir «Nyx Settings → Appearance», elegir un idioma y luego «Save & reload».
El menú ofrece English, Français, Español, Italiano, Русский, Deutsch y
Português. Cerrar cancela la selección. Las modificaciones ya presentes en
los ajustes de Nyx también se guardan con este comando.

El campo `language` de `bootloader/nyx.ini` vale respectivamente de 0 a 6 en este
orden. Un valor ausente o inválido usa el inglés. La elección de idioma
es independiente de `home_layout`.

Los menús, comandos principales, categorías de diagnóstico, mensajes y
avisos listados usan un catálogo de siete idiomas. Los nombres de
arranque, rutas e informes técnicos se mantienen tal cual. Algunas
informaciones técnicas permanecen en inglés. El programa RAM separado recibe
recursos en el idioma elegido, con una visualización de rescate en inglés.
Los acentos y el cirílico son compatibles con las fuentes Unicode
integradas Noto Sans, bajo licencia SIL OFL incluida con las fuentes.

## Copia de seguridad y diagnóstico

El selector verifica la presencia de Nyx antes de la recarga. Un error de
guardado debe mostrar un mensaje e impedir la recarga. Los errores
de escritura y de cierre del archivo de configuración ahora los tiene en cuenta
el escritor de configuración Nyx.

El tema del inicio también se aplica a los componentes de los menús y
submenús: ventanas, paneles, botones, listas, ajustes, selectores,
interruptores, cuadros de diálogo y barra de pestañas. Los colores de alerta
y los textos numéricos en fuente fija se conservan. Los fondos y matices
personalizados siguen teniéndose en cuenta. El tema por defecto usa el fondo
141A19, el acento 93E1AE y ángulos redondeados. Las tarjetas, discos de iconos,
paneles, botones y selecciones tienen un fondo transparente; los contornos y el texto
señalan la selección. Las ventanas superpuestas, diálogos, teclado y
listas desplegables mantienen un fondo oscuro para evitar textos superpuestos.
Los indicadores y barras de progreso conservan su color.

El Diagnóstico contiene ahora toda la tabla de sondas HWTest, la exportación
y el servicio RAM con retorno a Hekate. Ver DIAGNOSTIC-INTEGRATION.md.

## Validación y límites

La compilación completa se realizó correctamente con devkitARM GCC 16.1.0.
Se generaron Hekate, Nyx y los módulos. Los tamaños del payload y de
Hekate sin comprimir respetan los límites del Makefile. Los archivos binarios
se compararon con su ELF enlazado. El programa Wi-Fi AArch64 y el servicio
RAM separado también se construyeron. La copia de seguridad de los ajustes pasó 96 casos C
ejecutados con una tarjeta SD simulada, incluidos los errores de escritura.

Todavía no se ha realizado ninguna prueba en consola. Las traducciones, fuentes y
su gestión de memoria se han controlado en ordenador con el código C real.

Para la validación de hardware, verificar las dos disposiciones en la pantalla 1280 × 720,
los atajos, los ajustes, la conservación de la elección tras la recarga,
la cancelación, los errores de escritura y la navegación táctil/Joy-Con.

Ver `DIAGNOSTIC-INTEGRATION.md` para la procedencia y el alcance de las sondas.

Ver `PERSONNALISATION.md` para las fuentes SD, los tamaños y los efectos;
`TRADUCTIONS.md` describe los límites actuales de la traducción de los informes.
