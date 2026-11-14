# Historial de Hekate Nova Pro
**Languages:** [Français](CHANGELOG.md) · [English](CHANGELOG.en.md) · **Español** · [Русский](CHANGELOG.ru.md)

Historial importado de los archivos conservados. Las etiquetas describen instantáneas de fuentes, no los commits de desarrollo originales. Las guías detalladas se conservan para cada versión. Los prototipos y las versiones Bluetooth siguen siendo pre-versiones.

## v32 — Nova Pro 32

- Resumen del particionado en una sola línea; valores a la derecha, filas despejadas, asas y botones separados.
- Relanzamiento de un flujo Joy-Con silencioso y después reconexión solo del par afectado, con protección contra los datos UART aún pendientes.
- Estado de radio WL_REG_ON alineado con la referencia HWTest durante la sesión y restaurado al detenerse.
- Compilación y comprobaciones en PC correctas. El uso a 2–3 metros queda por validar en consola; no se garantiza ninguna ganancia de alcance.

[Notas detalladas](docs/releases/v32.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32)

## v31 — Nova Pro 31

- Particionado experto a todo el ancho, resumen inferior y ajustes −/+; tamaños en GB enteros con 29 / 58 GB Full.
- Procesamiento Bluetooth durante el dibujado por software, las copias de imagen y las esperas de visualización.
- Medidas HCI de solo lectura: RSSI relativo, calidad, potencia y contactos fallidos; estadísticas de huecos de informes completas.

[Notas detalladas](docs/releases/v31.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v31)

## v30 — Nova Pro 30

- Al arrancar, carga SD y Bluetooth en segundo plano solo cuando ambos raíles están vacíos.
- Pulsaciones breves conservadas, invalidaciones decorativas acotadas y procesamiento cooperativo en las herramientas largas.
- Auditoría de las lecturas de recursos y de la limpieza; diagnósticos que conservan su uso reservado de los periféricos.

[Notas detalladas](docs/releases/v30.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v30)

## v29 — Nova Pro 29

- Aceptación de los perfiles de emparejamiento Nintendo y estándar ya validados; rechazo de las respuestas SPI truncadas.
- Guardado / carga SD privados de 72 bytes con suma de comprobación, copia de seguridad y reemplazo transaccional.
- Tiempo de espera de conexión de 5,12 s, reintentos espaciados sin parada definitiva al tercer fallo.

[Notas detalladas](docs/releases/v29.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v29)

## v28 — Nova Pro 28

- Presupuesto corto de 250 µs durante la visualización y refresco menos frecuente del panel Bluetooth.
- Diagnóstico de la importación de los emparejamientos, contadores de botones y cadencia de informes.
- Esta versión diagnostica el 0/2 pero todavía no corrige su clasificación; corregida después en v29.

[Notas detalladas](docs/releases/v28.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v28)

## v27 — Nova Pro 27

- Perfil ROM adaptado de HWTest-RCM: paquetes a la tasa base, suspensión / modo activo, QoS y lectura UART prolongada.
- Alimentación, reloj y restauración del estado anterior; diagnósticos de controlador enriquecidos.
- Primeros comentarios de hardware sobre comandos inalámbricos, con latencia y fiabilidad todavía por mejorar.

[Notas detalladas](docs/releases/v27.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v27)

## v26 — Nova Pro 26

- Aceptación de las aperturas HID simultáneas iniciadas por Nova y por el Joy-Con.
- Gestión de canales obsoletos, rechazos tardíos y tiempos de espera, conservando la sesión válida.
- Correcciones verificadas mediante negociaciones HCI/L2CAP/HID simuladas, sin garantía de hardware sobre el alcance.

[Notas detalladas](docs/releases/v26.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v26)

## v25 — Nova Pro 25

- Preferencia por el rol central durante una reconexión entrante; intentos salientes serializados.
- Cifrado activado y estados de conexión mejor seguidos; metadatos de controlador y roles añadidos al informe.

[Notas detalladas](docs/releases/v25.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v25)

## v24 — Nova Pro 24

- Corrección del orden de bytes de las claves importadas y comprobación de la identidad Bluetooth de la consola.
- Importación desde los raíles verificada por la ruta SPI real y comparación con valores de referencia independientes.

[Notas detalladas](docs/releases/v24.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v24)

## v23 — Nova Pro 23

- Reanudación de la conexión del mando según los fallos HID observados; apertura de los canales conocidos sin esperar la descripción SDP.
- Informe anónimo de las negociaciones, autenticaciones y etapas HID; validación de hardware todavía incompleta.

[Notas detalladas](docs/releases/v23.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v23)

## v22 — Nova Pro 22

- Actualización del núcleo a Hekate 6.5.5 / Nyx 1.9.5 y adopción de las correcciones anteriores documentadas, sin reemplazar el tema Nova.
- Corrección del falso aviso Old Nyx GUI found: metadatos en el desplazamiento esperado y guarda de enlace.
- Navegación con el Joy-Con disponible, despertar del cursor oculto, gestión de informes simples y conservación de los ajustes.

[Notas detalladas](docs/releases/v22.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v22)

## v21 — Nova Pro 21 — Bluetooth

- Primer servicio Bluetooth personal para los Joy-Con separados, con importación de los emparejamientos y navegación.
- Bandeado Solo lectura de Herramientas USB reformateado; fondo del panel Créditos restaurado.
- Pestaña Changelog añadida a los créditos. Versión histórica experimental, seguida de varias correcciones Bluetooth.

[Notas detalladas](docs/releases/v21.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v21)

## v20 — Nova Pro 20 — Dock

- Detección automática del dock y conmutación de pantalla después del arranque.
- Ajustes solo TV o TV + pantalla de la consola; controlador TV conservado entre los menús.
- Conexión de referencia: consola compatible insertada directamente en el dock oficial; la Switch Lite no proporciona salida TV.

[Notas detalladas](docs/releases/v20.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v20)

## v19 — Nova Pro 19 — TV

- Primera integración TV manual en Nova, mantenida entre los menús.
- Referencia de hardware comunicada: prototipo TV01, OLED directamente en un dock oficial, imagen estable y parada VOL−.
- No presentar esta observación del prototipo como una validación de todas las consolas ni de todas las versiones TV.

[Notas detalladas](docs/releases/v19.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v19)

## tv-experiment-02 — Ensayo TV autónomo 02

- Programa TV autónomo de referencia sobre base Hekate 6.5.5; distinto de las versiones Nova.
- Ensayo en dock oficial; conservar los límites y el procedimiento VOL− descritos en la guía original.

[Notas detalladas](docs/releases/tv-experiment-02.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-02)

## tv-experiment-01 — Ensayo TV autónomo 01

- Programa TV autónomo de referencia sobre base Hekate 6.5.5; distinto de las versiones Nova.
- Ensayo en dock oficial; conservar los límites y el procedimiento VOL− descritos en la guía original.

[Notas detalladas](docs/releases/tv-experiment-01.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-01)

## v18 — Nova Pro 18

- Créditos limitados a las pestañas superiores, autores y licencias conservados.
- Iconos PNG transparentes desde la carpeta icone, imagen Launch y compatibilidad BMP.
- Contornos eliminados de las Opciones del sistema; V1/V2 agrupadas y capacidad FAT32 calculada en los perfiles compatibles.
- Mínimo / Recomendado para Linux y Android, tamaños en GB en modo experto. La salida TV sigue siendo un estudio en esta etapa.

[Notas detalladas](docs/releases/v18.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v18)

## v17 — Nova Pro 17

- Paleta rectangular, vista previa persistente entre los destinos y colores de arranque independientes.
- Personalización del mosaico Launch, centrado del logotipo y progreso de carga.
- Balance de quince componentes, cobertura real de las pruebas y pistas de investigación; exportación de las comprobaciones complementarias.
- Pruebas interactivas Joy-Con/Lite, sticks, rejilla táctil, mires de pantalla y prueba de RAM con modos rápido / avanzado / personalizado.

[Notas detalladas](docs/releases/v17.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v17)

## v16 — Nova Pro 16

- Fondo Carbon y colores Nova por defecto; ventanas al 75 % de transparencia en el primer arranque.
- Particionado compacto simplificado / experto: perfiles de consola, emuMMC completa, Linux y Android independientes.
- Brillo manual a la derecha del reloj; copia de seguridad/restauración, USB y registros redistribuidos.
- Borde de selección azul/violeta y reflejo acelerado; animaciones de fondo siempre eliminadas.

[Notas detalladas](docs/releases/v16.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v16)

## v15 — Nova Pro 15

- Legibilidad de los informes, alineación eMMC y paginación de las tablas; traducción de las copias de seguridad y de los registros.
- Vista previa acumulativa de los colores, introducción progresiva y reflejo de 0,48 segundos.
- Columnas emuMMC alineadas, selección de archivos por páginas y submenús recentrados.
- Conversor de fuentes corregido para los acentos y Caviar. Archivo de fuentes restaurado desde la instantánea v15 conservada; 23 huellas de fuentes modificadas concuerdan, quedando excluidos los dos archivos de payload generados.

[Notas detalladas](docs/releases/v15.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v15)

## v14 — Nova Pro 14

- Reflejo ampliado a 129 píxeles con degradado transparente en los extremos.
- Transición acelerada a aproximadamente 0,8 segundos, siempre sobre el mosaico completo.

[Notas detalladas](docs/releases/v14.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v14)

## v13 — Nova Pro 13

- Corrección del orden de las ventanas y de la navegación táctil cuando un fondo de pantalla está activo.
- Reflejo blanco diagonal sobre todo el mosaico seleccionado, contornos fijos.

[Notas detalladas](docs/releases/v13.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v13)

## v12 — Nova Pro 12

- Orden de visualización del fondo de pantalla corregido; seis colores independientes, incluidos logotipo, iconos y selección.
- Acceso a los créditos mediante el logotipo, temperatura coloreada e interruptores acordes con el tema.
- Ajuste de fecha/hora de Hekate original conservado y enriquecido con la elección de idioma en el primer arranque.

[Notas detalladas](docs/releases/v12.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v12)

## v11 — Nova Pro 11

- Batería a la derecha; cinco pestañas a la izquierda y cuatro acciones rápidas a la derecha.
- Estilo Nova extendido a los submenús, eliminación de las banderolas, selección en dos pulsaciones y nuevo monograma.
- Colores independientes, fondo de pantalla BMP, opacidad de los menús e iconos, arranque Hekate Nova Pro.
- Eliminación de las olas, partículas y otras animaciones de fondo; efectos limitados a la selección.

[Notas detalladas](docs/releases/v11.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v11)

## v10 — Nova

- Adopción del renderizado Nova: pestañas superiores, tarjetas antracita/azul e iconos Lucide finos.
- Gran tarjeta de arranque y cuatro accesos secundarios; rejillas Herramientas / Información y doce categorías Diagnóstico.
- Bandeado de detalle de la selección e inicio Mosaicos / Carrusel conservado.

[Notas detalladas](docs/releases/v10.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v10)

## v9 — Studio

- Presentación Studio con navegación lateral, fondo antracita, acentos violetas y ángulos de seis píxeles.
- Halo de selección fijo sin refresco continuo; interruptores ON/OFF traducidos y menús encuadrados entre los bandeados.

[Notas detalladas](docs/releases/v9.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v9)

## v8 — Interfaz compacta y fluidez

- Eliminación de las grandes listas verticales en favor de pestañas compactas y selectores visibles.
- Contenido comprendido entre la navegación superior y la barra de estado inferior; ajustes Interfaz / Sistema y Apariencia repartidos.
- Tarjetas oscuras transparentes, contornos menos redondeados y ayudas largas accesibles mediante botones.

[Notas detalladas](docs/releases/v8.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v8)

## v7 — Personalización

- Menú Apariencia: idioma, fuente SD en formato NYXF, tamaño de texto, disposición y efectos de fondo.
- Traducción ampliada de los menús y textos técnicos; controles de apariencia agrupados.
- Las animaciones de fondo de esta versión se retiraron más tarde para aligerar el renderizado.

[Notas detalladas](docs/releases/v7.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v7)

## v6 — Transparencia e idiomas — prototipo 6

- Estilo moderno transparente en los menús, controles y tarjetas, con icono Diagnóstico.
- Siete idiomas: inglés, francés, español, italiano, ruso, alemán y portugués; acentos y cirílico.
- Mosaicos / Carrusel, 42 diagnósticos nativos y prueba de RAM aislada con restitución del resultado al volver.

[Notas detalladas](docs/releases/v6.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v6)

## v5 — Diagnósticos completos — prototipo 5

- Integración de las 42 entradas nativas HWTest-RCM, categorías, ejecución individual e informes exportables.
- Servicio de prueba de memoria distinto con regreso a Hekate; conservación de las sondas y de los comentarios de procedencia.
- El archivo de fuentes está disponible; no se ha encontrado ningún paquete binario específico de v5 entre las entregas conservadas.

[Notas detalladas](docs/releases/v5.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v5)

## v4 — Mosaicos y carrusel — prototipo 4

- Dos inicios Mosaicos / Carrusel seleccionables desde los ajustes Nyx.
- El archivo custom-sources pertenece también a esta etapa; sus notas ligeramente diferentes se conservan como documento histórico. Solo fuentes.

[Notas detalladas](docs/releases/v4.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v4)

## v3 — Interfaz original — prototipo 3

- Vuelta a la disposición de la interfaz original, con acceso Diagnóstico.
- Corrección de lectura DRAM conservada. Solo fuentes.

[Notas detalladas](docs/releases/v3.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v3)

## v2 — Interfaz consola — prototipo 2

- Inicio de tipo consola con tarjetas grandes y acceso a las funciones principales.
- Corrección de lectura del segundo byte de los canales DRAM; diagnósticos conservados. Solo fuentes.

[Notas detalladas](docs/releases/v2.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v2)

## v1 — Diagnóstico nativo — prototipo 1

- Nueva pestaña Diagnóstico en Nyx; identificación de pantalla, retroiluminación y DRAM.
- Veredictos PASS/WARN/FAIL y adaptador Nyx para las sondas HWTest. Solo fuentes; la prueba DRAM identifica la memoria sin recorrer todas sus celdas.

[Notas detalladas](docs/releases/v1.md) · [Descargas](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v1)
