# Personalización actual — Nova Pro 11
**Languages:** [Français](PERSONNALISATION.md) · [English](PERSONNALISATION.en.md) · **Español** · [Русский](PERSONNALISATION.ru.md)

Consultar [NOVA-PRO-V11.md](NOVA-PRO-V11.md) para los colores independientes, el fondo de pantalla fijo, las transparencias, el reflejo y los submenús. El carrusel, los siete idiomas y las fuentes SD siguen disponibles. Las olas, partículas y auroras se suprimen; sus descripciones abajo pertenecen al histórico.

---
## Historial — personalización antes de la versión 11

# Apariencia, idiomas y lectura — Nova, versión 10

Abrir **Ajustes** en la navegación superior. Los códigos de idioma, la línea
Fuente y los tamaños propuestos abren directamente el selector correspondiente.
El título **Apariencia y lectura** abre los ajustes de Nyx detallados; elegir
luego **Apariencia** para las demás opciones. La misma pantalla agrupa
el idioma, la fuente, el tamaño del texto, la disposición del inicio y las
animaciones de fondo. **Guardar y recargar** aplica las opciones. Cerrar
la ventana cancela las opciones provisionales.

## Fuentes y tamaños

Se ofrecen nueve tamaños principales, de 20 a 36 píxeles. Las leyendas
usan un tamaño proporcionado y las tablas conservan una fuente
de ancho fijo. Nova conserva roles distintos para los títulos, los comandos
y las leyendas, proporcionados al tamaño elegido y cargados una sola vez.
Las tarjetas usan un salto de línea o puntos suspensivos; la
banda de detalle y la ayuda dan el texto completo. Las opciones muy
largas pueden emplear el tamaño de leyenda de la misma fuente para mantenerse
legibles en su botón.

Se incluyen tres fuentes listas para usar en `fonts/`, que hay que copiar en la
raíz de la tarjeta SD: Lecture, Lecture reforzada y Lecture fija. El menú
vuelve a leer esta carpeta en la próxima carga de Nyx. Se pueden añadir hasta dieciséis fuentes SD.
Un archivo inválido usa la fuente integrada.

Para una fuente personal TTF u OTF, usar el conversor incluido
en `outils-polices/` y luego copiar el archivo `.nyxf` obtenido en `fonts/`.
La consola no lee directamente los TTF/OTF. Python 3 y Pillow
son necesarios únicamente en el ordenador para esta conversión.
Las instrucciones detalladas están en `documentation/POLICES.md`.

## Fondos y fluidez

El fondo de referencia es `#10151D`, las tarjetas `#19212D`, los contornos
`#334050` y el acento `#55C9F1`. Nova retoma los iconos Lucide de la maqueta,
sus trazos finos, las tarjetas rellenas y los ángulos de 8 píxeles.
El halo de 18 píxeles permanece fijo
alrededor del icono seleccionado: ningún temporizador lo anima. Las tarjetas
muestran fondos lisos, con iconos recortados. El fondo SD
personalizado ya no se dibuja en esta presentación; su archivo se
conserva. Las pastillas de color tienen un relleno propio, distinto de las
tarjetas Nova. La primera pastilla corresponde al acento azul por defecto.
**Efectos de selección** activa o desactiva el halo; guardar las opciones
para conservar esta elección.

Las opciones de animación son Desactivado, Olas, Partículas y Aurora, con
velocidad e intensidad. Los efectos se concentran en una banda inferior
de 64 píxeles. Se congelan durante el uso del táctil/Joy-Con y
se reanudan tras 900 ms de reposo; los diagnósticos también los suspenden.
Se realiza una sola presentación de la imagen por refresco LVGL,
incluso cuando cambian varias zonas pequeñas. Las búsquedas de glifos
y las traducciones se han optimizado sin modificar las mediciones de hardware.

## Disposiciones

El inicio Tarjetas retoma la gran tarjeta Iniciar y los cuatro accesos de la
maqueta. El Carrusel sigue disponible. Las páginas principales usan la
navegación superior y una zona de 1 280 × 580 píxeles por encima de la barra de estado.
Herramientas e Información tienen cuatro columnas en dos filas; Diagnóstico presenta
doce tarjetas en tres filas, sin desplazamiento vertical.
Tocar una tarjeta selecciona su detalle. La flecha de la banda inferior abre
la función; tocar el texto de la banda muestra la ayuda completa.
Las palabras demasiado anchas usan primero el tamaño de leyenda de la fuente
elegida. Las opciones de dos estados tienen un interruptor y un estado escrito;
Auto Boot sigue abriendo la elección de la entrada de arranque. Se encuentra
con las demás opciones de arranque detrás del título **Opciones de sistema**.
El logo HEKATE abre los comandos rápidos y los ajustes de Nyx; una pulsación larga
abre los créditos.

Los Ajustes de Nyx comprenden dos pestañas internas, Interfaz y Sistema.
Apariencia comprende Idioma, Fuente, Disposición y Efecto. La fuente se
selecciona con las flechas; las demás opciones actuales son visibles.
Los informes detallados y las listas de archivos siempre pueden desplazarse.

## Idiomas y datos

Los siete idiomas son inglés, francés, español, italiano, ruso,
alemán y portugués. El catálogo comprende las etiquetas, mensajes,
ayudas y avisos de la interfaz así como los textos de diagnóstico
listados. Las mediciones, nombres de componentes, registros, rutas y nombres
de entradas de arranque siguen siendo identificables y no se renombran.
Los comentarios detallados de las sondas ahora están presentes en los
siete idiomas. Su complemento se tradujo fuera de línea, con controles
sobre los valores y las rutas así como correcciones de terminología.
Queda por hacer una revisión lingüística de las descripciones automáticas;
el alcance exacto se describe en `TRADUCTIONS.md`.

La prueba RAM lanzada desde Nyx recibe un pequeño paquete de textos y glifos
en la zona de memoria ya excluida de la prueba. El paquete se verifica antes
de su uso. Si falta o está corrupto, el servicio conserva su visualización
de rescate en inglés. El arranque autónomo y los mensajes tempranos de
recuperación siguen disponibles sin recursos de idioma.

Los ajustes se añaden a `bootloader/nyx.ini`. No se incluye ningún archivo de ajustes
ni ninguna entrada de arranque en el archivo. Fusionar las
carpetas `bootloader/` y `fonts/` con la tarjeta SD existente.

## Verificación

La compilación y los controles en ordenador se incluyen en el archivo.
Las verificaciones gráficas cubren 1 890 escenas LVGL: los siete idiomas,
los nueve tamaños principales, las tres fuentes incluidas y la fuente integrada.
Doce escenas adicionales verifican las guardas según la disponibilidad del
hardware o de una herramienta. Se controlan los píxeles de los iconos recortados.
Verifican en particular la configuración, la cancelación, los errores de
lectura, los formatos de fuente, los textos UTF-8 y los comandos conservados.
No sustituyen la prueba en consola: verificar la lectura a 20 y
36 píxeles, los siete idiomas, los efectos y la navegación táctil/Joy-Con.
