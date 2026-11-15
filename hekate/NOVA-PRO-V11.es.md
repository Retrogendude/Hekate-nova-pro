# Hekate Nova Pro — versión 11
**Languages:** [Français](NOVA-PRO-V11.md) · [English](NOVA-PRO-V11.en.md) · **Español** · [Русский](NOVA-PRO-V11.ru.md)

Esta versión aplica las doce modificaciones consignadas en MODIFICATIONS-NOVA-PRO.md. La maqueta Nova sigue siendo la base, con los cambios expresamente solicitados: controles directos, reflejo, supresión de las insignias, nuevas interacciones y personalización.

Las pestañas ocupan la barra superior; las ventanas permanecen entre 76 y 656 píxeles, por encima de la banda de estado. Los controles de la barra superior no están disponibles durante la apertura de una ventana, y luego recuperan su estado al cerrarla. Las tarjetas conservan esquinas de 8 píxeles, una paleta antracita y pictogramas finos. Las páginas de copia de seguridad/restauración, USB, emuMMC, diagnóstico, apariencia y créditos retoman estos componentes. Los informes técnicos siguen consultables en su ventana Nova.

La primera pulsación en una tarjeta de Herramientas, Información o Diagnóstico la selecciona. La segunda pulsación en esa misma tarjeta abre su función. Una tarjeta preseleccionada también exige dos pulsaciones tras la apertura del menú. Se conservan las acciones especiales por pulsación larga, las confirmaciones y las restricciones de las funciones originales.

El reflejo se dibuja cada 80 ms en pequeños segmentos de los contornos visibles. Solo se refrescan las zonas correspondientes a sus antiguas y nuevas posiciones. Las superficies enmascaradas se ignoran; el reflejo se interrumpe durante los diagnósticos y las interacciones. Las olas, partículas y auroras se han quitado del código y de la interfaz. El campo histórico nova_halo controla ahora el reflejo.

El fondo, el texto, los contornos y el reflejo tienen cuatro colores independientes. Dos ajustes de opacidad afectan únicamente a los fondos de los menús y a los marcos de iconos. El editor presenta una vista previa, valores RGB y muestras. Cerrar cancela las elecciones; Guardar y recargar las aplica. Los ajustes de idioma y de fuente siguen siendo distintos.

Los fondos de pantalla son fijos, en formato BMP 24/32 bits sin compresión, en /wallpapers. El decodificador verifica las dimensiones, los tamaños, la orientación y los accesos al archivo. El recorte y el escalado tienen lugar en la carga, sin acceso SD durante el dibujado. Se asignan una sola línea origen y la imagen final. La imagen final de 1280 × 720 ocupa aproximadamente 3,5 MiB únicamente cuando se elige un fondo. El conversor incluido prepara las imágenes PNG/JPG personales; Nova-Carbon se entrega listo para elegir.

El arranque presenta el monograma original Nova Pro y el nombre Hekate Nova Pro. La nueva animación usa una barra y tres puntos; se conservan el retardo de arranque y la cancelación por VOL−. Hay que lanzar el nuevo payload incluido para beneficiarse de esta presentación. El loader se reconstruye cuando cambian los datos Hekate comprimidos; la verificación final descomprime el verdadero payload y compara su contenido con el binario actual.

La compilación y las comprobaciones afectan al software. Las representaciones usan el verdadero LVGL con las acciones de hardware sustituidas por valores de demostración. No constituyen una medida de fluidez en consola. La validación física en Switch sigue siendo necesaria, en particular para los diagnósticos y el retorno del servicio RAM.
