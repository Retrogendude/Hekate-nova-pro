# Hekate Nova Pro — corrección v13
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V13.md) · [English](CORRECTIONS-NOVA-PRO-V13.en.md) · **Español** · [Русский](CORRECTIONS-NOVA-PRO-V13.ru.md)

Esta versión corrige el bloqueo de los menús tras activar un fondo de pantalla y sustituye el reflejo del símbolo por un reflejo sobre toda la tarjeta seleccionada.

## Fondo de pantalla y acceso a las ventanas

El defecto se reprodujo antes de la corrección en el código LVGL real: tras abrir la ventana Colores con un fondo activo, el primer elemento de la lista de hijos tenía un predecesor. El contenido de la ventana ya no era accesible por el recorrido usado para encontrar el botón pulsado, aunque el dibujado todavía podía recorrerlo en el otro sentido. Las antiguas pruebas de imagen no verificaban este invariante.

La función lv_ll_move_before ahora también actualiza la cabeza de la lista cuando un elemento pasa delante del primero. Es el caso al colocar el contenido de una ventana delante de su fondo. Los fondos siguen protegidos, sin interacción y detrás del contenido. La corrección afecta al orden interno de los objetos y a su acceso al toque.

Los controles añadidos verifican la coherencia de los dos sentidos del recorrido en todo el árbol de las ventanas. Una prueba de la función C real compara 316 646 desplazamientos con un modelo independiente. Otras pruebas envían pulsaciones y sueltas reales al driver de entrada LVGL: apertura de Nyx, Colores, elección de un color, cierre, apertura de Apariencia, cierre y luego cambio de cada una de las pestañas principales. Cada secuencia se repite tres veces, en los siete idiomas, en 20, 30 y 36 píxeles.

## Reflejo sobre toda la tarjeta

Una banda blanca diagonal atraviesa toda la superficie de la tarjeta seleccionada: fondo, pictograma y texto. No gira sobre los contornos; el borde permanece fijo. Se aplica a las tarjetas de la pantalla de inicio, al carrusel y a las selecciones de las rejillas Herramientas, Información y Diagnóstico.

El paso dura aproximadamente 1,2 segundos, seguido de unos 2,4 segundos de descanso. El blanco permanece suave, con varios niveles de transparencia. El dibujado se realiza directamente en pequeñas bandas, sin imagen transparente del tamaño de la tarjeta y sin asignación durante la animación. Cuatro zonas siguen la antigua y la nueva posición del reflejo para evitar redibujar sistemáticamente toda la tarjeta o toda la pantalla. El efecto se pone en espera durante las interacciones y los diagnósticos. El botón Efectos de selección permite cortarlo.

## Instalación y conservación de los ajustes

Fusionar la carpeta bootloader del archivo con la de la SD tras una copia de seguridad. Usar el payload hekate_nova_pro_6.5.4_v13.bin incluido. El nuevo archivo bootloader/sys/nyx.bin contiene las correcciones; no conservar el de la v12. No se entrega ningún archivo nyx.ini o hekate_ipl.ini. Se conservan los colores, el fondo, el idioma y las fuentes ya configurados. El ajuste de hora original con el idioma añadido sigue sin cambios. No es necesario instalar otro programa para usar esta versión.

## Validación

Compilación devkitARM y verificación del código real en PC, con las fuentes, los fondos y las transparencias. Las pruebas incluyen el acceso a los botones, las aperturas y cierres de ventanas superpuestas, las copias de seguridad, los límites de los binarios y las 42 sondas de diagnóstico. El arranque Hekate y el compañero RAM siguen siendo los de la v12; sus binarios se comparan durante la preparación de la entrega.

Los resultados detallados están en verification/. Las vistas previas y el GIF provienen de la representación LVGL real. **No se ha realizado aquí ninguna prueba de esta versión en Switch ni medición de FPS de consola.** Verificar en el dispositivo el acceso a los ajustes con el fondo activo y la fluidez del reflejo.
