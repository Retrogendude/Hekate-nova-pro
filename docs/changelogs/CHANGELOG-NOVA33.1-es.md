# Hekate Nova Pro v33.1 beta

- Baldosa Iniciar: icono pequeño /icone/launch.png independiente del fondo grande /icone/Launcher.png, sustituible en Nova con vista previa coherente.
- Créditos: HWTest-RCM 0.15 atribuido a Creased (Baptiste MOINE) en Software libre.
- Apariencia y opciones: eliminados los fondos de Verificación de copias y Efectos de selección; borde de Opciones del sistema armonizado.
- Copia y restauración emuMMC: el borde activo se sitúa en el panel exterior, sin doble marco desplazado; se conserva el comportamiento de los botones.
- Paquetes separados estándar y experimental de 8 GB para consolas modificadas físicamente: Erista V1 y Mariko V2/Lite/OLED. Se utiliza la marca de software oficial de Hekate sin programar fusibles.
- Diagnóstico RAM: conserva el modo de 8 GB al iniciar y volver, selecciona perfiles por plataforma y capacidad y detiene la prueba si la capacidad cambia tras el entrenamiento.
- Direcciones altas calculadas en 64 bits e informe de capacidad real: 8.176 MiB comprobables de 8.192 MiB; 16 MiB protegidos para datos de trabajo.
- Atmosphère 1.11.0 o posterior para modos superiores a 4 GB: memmode=1 en una entrada Hekate fss0 o enable_mem_mode=1 en exosphere.ini para fusee. No se modifican automáticamente las configuraciones existentes.

Beta: compilaciones ARM y controles PC correctos. El modo de 8 GB, especialmente el perfil Erista adaptado, requiere validación en cada plataforma; no se certifican todas las memorias de sustitución. Los informes chinos largos aún necesitan revisión lingüística.

## v33

- Chino simplificado como octavo idioma en Ajustes y en la configuración inicial.
- Fuente dedicada obligatoria solo para chino. Se conserva la fuente personal para los demás idiomas y el tamaño sigue siendo ajustable.
- Lecture.nyxf es la fuente automática para los idiomas latinos; ruso utiliza la familia integrada. La fuente original de Hekate está disponible en el selector.
- Incluye tres fondos estáticos, cinco iconos Nova y la imagen Launcher facilitada para la tarjeta principal.
- Instalación nueva: francés, Carbon Cyan, opacidad del 75 % en paneles e iconos, texto de 30 px, colores solicitados y logotipo de inicio claro. Se mantiene el ajuste original del reloj.
- Acceso visible a Auto Boot, tiempo de espera, Auto NoGC y Auto HOS Power Off. El brillo enlaza con el ajuste existente, sin duplicar el deslizador.
- Selector Auto Boot paginado con estilo Nova, sin desplazamiento vertical y conservando los índices de Hekate.
- Prueba de memoria independiente con interfaz china y caracteres de 16 × 16; compatibilidad con los siete paquetes anteriores.
- Si la fuente china falta o no es válida, se utiliza inglés sin sustituir la fuente personal. El paquete conserva los ajustes guardados.
