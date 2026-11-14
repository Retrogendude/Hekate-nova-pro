# Origen de la integración TV Nova 20-Dock
**Languages:** [Français](TV-SOURCES.md) · [English](TV-SOURCES.en.md) · **Español** · [Русский](TV-SOURCES.ru.md)

Base Nova Pro v18: copia separada de `work/hekate-6.5.4`, sin sustitución de
la versión ya entregada. Driver de vídeo `dpout.c/.h` idéntico al prototipo TV01.
El driver `usbpd.c/.h` retoma las dos correcciones del protocolo TV02, verificadas
por una regresión sobre la confirmación de los comandos y el presupuesto del hub.

Origen HWTest-RCM / Creased:
https://github.com/Creased/HWTest-RCM/tree/3731bfb471515a5f83709aa496c9cdb0a9aeff83

Referencias del driver upstream: drivers BM92TXX y Tegra NVIDIA de CTCaer /
switchroot L4T, y driver DisplayPort Tegra210 de coreboot.
Comentarios y tablas de ajuste originales conservados. Sin retoque de las
tensiones, niveles de pilotaje, tasas de enlace ni cronogramas para este port.

Cambios Nova: sesión independiente de la ventana de ajustes, copia de las
líneas modificadas de su búfer 1280x720, detención antes del fin de hardware/USB/diagnóstico,
frecuencia de menú restaurada tras la conexión, nuevas vistas en el tema Nova
y 36 frases TV/dock traducidas en los siete idiomas disponibles. Los nombres de etapas,
registros y detalles brutos del driver siguen siendo información de diagnóstico.

Corrección distinta de la grabación SD: uso de `sd_mount() == 0`
para reconocer el éxito, con prueba del verdadero cuerpo de esta función.
Los prototipos anteriores usaban una prueba invertida y un mock incorrecto.

Validación física reportada: prototipo TV01, OLED insertada directamente en
una dock oficial, imagen estable y parada VOL- funcional. Las pruebas que usan
un cable consola/dock no sirven para calificar la conexión directa.
La validación física de este nuevo port Nova queda por realizar.

Hekate y la integración siguen las licencias incluidas en el proyecto.
El repositorio HWTest-RCM consultado no precisa ninguna licencia autónoma para estos
nuevos archivos: aclaración necesaria antes de una difusión pública del
conjunto. Entrega personal de desarrollo, con procedencia conservada.

## Detección automática en 20-Dock

Preferencias `tv_auto` (por defecto 0) y `tv_keep_screen` (por defecto 1) añadidas al final
de la configuración de Nyx para conservar los antiguos campos. Carga estricta,
escritura atómica por el mecanismo existente. Activación con Guardar.

El detector pasivo lee STATUS1, la oferta PDO y el identificador del controlador
cuando no se señala ningún cable. No lee ALERT_STATUS y no envía ningún
comando. Usa la firma de alimentación ya reconocida por este driver
para una dock Nintendo; la identificación/negociación completa viene después.
Se requieren dos muestras positivas tras un retardo inicial de 2 segundos.
Un intento por inserción detectada; los errores o una detención manual no
desencadenan una repetición inmediata. Los errores de lectura espacian los
sondeos a 2 y luego 5 segundos y no se asimilan a una retirada.

El servicio suspende su supervisión durante USB/diagnósticos y la restablece
en su retorno, sin cancelar una detención manual previa. La elección Solo TV
apaga la retroiluminación tras la inicialización de vídeo, y la restablece en la detención
con el brillo actual. La salida hacia otro sistema evita un destello
de retroiluminación justo antes del fin del driver LCD.

No se integra ningún servicio Bluetooth nuevo: ver el estudio Joy-Con dedicado.
La nueva detección automática y la conmutación de pantalla quedan por validar
en consola, por separado del éxito del prototipo01 en dock directa.
