# Hekate Nova Pro — reflejo v14
**Languages:** [Français](CORRECTIONS-NOVA-PRO-V14.md) · [English](CORRECTIONS-NOVA-PRO-V14.en.md) · **Español** · [Русский](CORRECTIONS-NOVA-PRO-V14.ru.md)

El reflejo blanco sigue atravesando toda la tarjeta, con un borde fijo. Esta actualización lo hace más ancho, más suave en los bordes y más rápido.

- Anchura de la banda: 129 píxeles, frente a 45 píxeles en la v13, es decir aproximadamente tres veces más ancha.
- Transparencia: perfil redondeado continuo, blanco suave en el centro y luego completamente transparente en los dos extremos. Las opacidades bajas siguen presentes; ya no las corta el umbral del relleno uniforme de LVGL.
- Paso: aproximadamente 0,8 segundos, frente a 1,2 segundos en v13.
- Descanso: aproximadamente 2,4 segundos. El ciclo completo dura 3,2 segundos.

El dibujado usa una sola línea de degradado de 516 bytes, reutilizada en cada altura. No crea ninguna imagen del tamaño de la tarjeta y no asigna nada durante la animación. Seis pequeñas zonas siguen las antiguas y nuevas posiciones del reflejo. El efecto mantiene una prioridad baja y se pone en espera durante las interacciones y los diagnósticos. El ajuste Efectos de selección sigue permitiendo desactivarlo.

Solo gui_nova_reflection.c ha cambiado respecto a las fuentes entregadas en la v13. Se conservan la corrección de acceso a los menús con fondo de pantalla, el ajuste de hora original con el idioma y los ajustes guardados.

Las comprobaciones de esta actualización abarcan 99 vistas nativas con fondo y transparencia, 21 secuencias de navegación por pulsaciones/sueltas en los siete idiomas, y la representación de píxeles del degradado sobre un fondo liso y luego sobre un fondo de pantalla. El perfil se compara con una curva de transparencia de referencia: anchura, simetría, transparencia en los extremos, opacidades bajas y límites de la tarjeta. Los binarios ARM se comparan con su ELF, se verifican sus tamaños y las 42 sondas siguen enlazadas. El arranque Hekate y el compañero RAM son idénticos a la v13. El informe completo de la v13 se entrega por separado como histórico; sus 4 407 vistas no se cuentan como pruebas repetidas aquí.

Las vistas previas y el GIF vienen de la representación LVGL real. Las mediciones de zonas redibujadas se hacen en PC, no en FPS en la consola. **La fluidez en Switch queda por confirmar en el dispositivo.**

Para instalar, guardar una copia de los archivos actuales, fusionar la carpeta bootloader y sustituir bootloader/sys/nyx.bin por el de la v14. Usar el payload incluido con el método habitual. No se incluye ningún INI; no se necesita ningún software ni SDK nuevo para usar esta versión.
