# Hekate Nova Pro — correcciones v15
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V15.md) · [English](CORRECTIONS-NOVA-PRO-V15.en.md) · **Español** · [Русский](CORRECTIONS-NOVA-PRO-V15.ru.md)

Los once puntos de la lista se han tratado en las fuentes y se han integrado en la compilación. Las comprobaciones siguientes se realizan en PC con el motor LVGL del proyecto. Los accesos al hardware se sustituyen por datos de prueba en estas vistas previas; no constituyen una copia de seguridad, un particionado ni un diagnóstico ejecutado en una Switch.

1. **Fuente de los informes corregida.** El redimensionamiento usaba la anchura de la celda fija como anchura del bitmap de cada carácter. Los glifos estrechos se leían con un paso incorrecto, lo que reproducía los caracteres borrosos de las capturas. El lector usa ahora la anchura real del bitmap. Los glifos se centran en una celda fija, incluido en el tamaño nativo de 20 px.
2. **Información eMMC realineada.** Cada etiqueta y su valor comparten una altura de línea calculada según su texto. Las propiedades y las particiones se reparten en dos pestañas. Las particiones usan páginas Anterior/Siguiente; la tabla ya no se detiene tras las veinte primeras filas.
3. **Copia de seguridad y restauración traducidas.** El título, el espacio libre, el tamaño, la carpeta y el archivo pasan por el catálogo de los siete idiomas. Los fragmentos añadidos al registro también pasan por la traducción: fin, fallo, cancelación y errores. Se conservan las rutas, nombres de archivos, números y direcciones.
4. **Caracteres técnicos verificados.** Los dígitos, letras ASCII y letras cirílicas se controlan a partir de los bitmaps reales. Los acentos y los siete idiomas se verifican en las vistas técnicas. La familia elegida para los menús sigue siendo independiente de la fuente fija de los informes.
5. **Vistas de las capturas reproducidas.** La información eMMC, el progreso de la copia de seguridad y el aviso MBR se incluyen en las pruebas de idiomas, tamaños y fuentes, así como en las pruebas de fondo de pantalla y transparencia. Los datos mostrados en las vistas previas son simulados. El registro permanece en una zona limitada; las tablas largas están paginadas.
6. **Vista previa de colores acumulativa.** Los colores provisionales del fondo, los textos, los contornos, el logo, los iconos y la selección siguen presentes juntos. El logo y el icono de ajustes tienen cada uno su color. Cambiar de objetivo o de pestaña ya no reinicia la vista previa. Guardar aplica el borrador; cerrar sin guardar conserva los ajustes anteriores.
7. **Reflejo acelerado.** La banda atraviesa toda la tarjeta en aproximadamente **0,48 segundos**, frente a 0,8 segundos en v14. Conserva su anchura de 129 px, sus extremos transparentes y su borde fijo. Diez bandas de redibujado limitan el trabajo. Se conservan la pausa de 2,4 segundos y la suspensión durante las interacciones y diagnósticos. No se realiza ninguna asignación en cada imagen.
8. **Carga progresiva.** La barra avanza hasta el 100 % en seis etapas animadas, con 1,8 segundos de animación en total, además del tiempo real de carga. Los accesos bloqueantes a la tarjeta SD todavía pueden añadir una pausa entre estas etapas. Se conservan el retardo de autoarranque, la cancelación por VOL− y el ajuste inicial de hora original con elección de idioma.
9. **Columnas emuMMC alineadas.** Los paneles de gestión comparten sus límites superior e inferior. Su disposición ya no depende de la colocación automática de la antigua ventana.
10. **Elección de los archivos emuMMC sin lista vertical.** Dos columnas del mismo tamaño, títulos centrados y cuatro opciones por página sustituyen la lista desplegable. Anterior/Siguiente da acceso a las siguientes opciones. La elección siempre guarda la ruta de la carpeta original; las particiones RAW no disponibles siguen desactivadas.
11. **Zona útil de los submenús.** Las ventanas usan el espacio entre el menú superior y la banda inferior. Los cuadros centrados se recentran en esta zona, con adaptación de la anchura y, si es necesario, del tamaño del texto. Los informes eMMC, el progreso y los selectores corregidos tienen dimensiones limitadas.

## Comprobaciones

La matriz comprende 40 vistas o pestañas, los siete idiomas y nueve tamaños de 20 a 36 px con Lecture, y luego las otras tres familias integrada, Lecture fija y Lecture reforzada en los tamaños 20, 30 y 36 px. Pruebas adicionales cubren el fondo de pantalla, tres niveles de transparencia, una paleta personalizada, los controles accesibles y la navegación por pulsaciones/sueltas.

Los controles afectan también a la conservación del borrador de colores, el acceso a todas las páginas de los informes y de los archivos emuMMC, los enlaces de ventanas LVGL, el guardado de los ajustes, la carga de las fuentes y el conversor Caviar corregido. El perfil de transparencia del reflejo se compara con los píxeles realmente dibujados. Los controles de arranque verifican el relleno monótono, la rotación de pantalla, el retardo y la cancelación.

Los binarios se comparan con los ejecutables ELF y con los límites de tamaño. Las 42 sondas siguen presentes, así como el servicio RAM separado y su mecanismo de retorno. Los registros y los números exactos están incluidos en `verification/`.

**A verificar en la consola:** arranque en frío, legibilidad de las tres vistas con los datos reales, apertura de los ajustes con el fondo elegido, fluidez del reflejo y navegación táctil. No se ha realizado aquí ninguna medición de FPS en Switch ni operación de almacenamiento real.

## Instalación

Guardar una copia de los archivos actuales de la tarjeta SD. Fusionar la carpeta `bootloader` incluida y luego usar el payload `hekate_nova_pro_6.5.4_v15.bin` con el método habitual. `bootloader/update.bin` contiene este mismo payload. Sustituir Nyx y el payload es necesario para disponer de todas las modificaciones de la carga.

Las carpetas `fonts`, `wallpapers` y las herramientas están incluidas. No se incluye ningún `nyx.ini` ni `hekate_ipl.ini`: los ajustes existentes no se sustituyen. No se necesita ningún software nuevo para usar el paquete compilado.
