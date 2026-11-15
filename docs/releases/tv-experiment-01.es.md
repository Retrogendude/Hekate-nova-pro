# tv-experiment-01 — Ensayo TV autónomo 01
**Languages:** [Français](tv-experiment-01.md) · [English](tv-experiment-01.en.md) · **Español** · [Русский](tv-experiment-01.ru.md)

- Programa TV autónomo de referencia sobre base Hekate 6.5.5; distinto de las versiones Nova.
- Ensayo en dock oficial; conservar los límites y el procedimiento VOL− descritos en la nota original.

**Naturaleza de la entrega:** preversión compilada, fuentes y archivos originales conservados.

## Archivos y huellas

| Archivo | Bytes | SHA-256 |
|---|---:|---|
| [hekate-tv-experiment-01-sources.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-01/hekate-tv-experiment-01-sources.zip) | 1,690,037 | e8bc2344798462014b5236979c013f36dbe583e7ec70b7cc2a3573ace816e476 |
| [hekate-tv-experiment-01.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-01/hekate-tv-experiment-01.zip) | 912,066 | 5bbcf52e99cbae1be19dc4b2092e84c178c153ae7757a1b7eb1ac6a387fa9dfc |

## Nota histórica completa

Las instrucciones y rutas siguientes se refieren al archivo de esta versión. No describen automáticamente la última versión. Las limitaciones y validaciones siguen siendo las indicadas en su momento.

# Hekate — prototipo TV 01

Primer ensayo de salida TV integrado en tu base Hekate 6.5.5 / Nyx 1.9.5.
Utiliza la interfaz clásica de esta base, distinta de Nova Pro v18.
Se conservan los diagnósticos BM92T36 existentes.

Este prototipo está compilado y verificado en PC. La imagen, su estabilidad y las
conexiones/reconexiones aún deben validarse en consola.

## Instalación y lanzamiento

1. Utilizar una tarjeta SD de prueba, o guardar íntegramente su carpeta
   `bootloader` antes de copiar los archivos del prototipo en la raíz.
   El paquete no sustituye ni `hekate_ipl.ini`, ni `nyx.ini`, ni `update.bin`.
2. Inyectar **hekate_tv_experiment_01_6.5.5.bin**. Este archivo debe lanzarse
   explícitamente; el payload Nova Pro v18 no carga esta versión de Nyx.
3. Fuera del dock, abrir **Console Info → Battery → I2C Diagnostics →
   Test TV (experimental)**.
4. Conectar el dock oficial a su alimentación y a la TV por HDMI.
   Encender la TV y elegir la entrada HDMI correspondiente.
5. Pulsar **Iniciar**, y luego colocar la consola en el dock en los
   45 segundos. También se puede empezar con la consola ya conectada al dock
   si el botón es accesible.
6. Observar la TV. El modo solicitado es **1280 × 720 a 60 Hz**. El texto
   «configurada (a verificar)» significa que la inicialización software ha concluido;
   hay que confirmar visualmente la imagen en la TV.
7. **VOL-**, **Detener** o **Cerrar** corta la sesión. La pantalla de la consola
   sigue activa. Una desconexión detectada también corta la salida.

## Informe a recuperar tras el ensayo

En la SD: **bootloader/tv-test/last-run.txt**.
El registro contiene los identificadores, etapas de negociación, registros, lectura
del receptor DisplayPort y diagnóstico del enlace de vídeo. Se sustituye en cada
nuevo ensayo; abrir y luego cerrar la ventana sin iniciar preserva el informe.
El cierre de la prueba guarda las últimas líneas. Si la SD está ausente,
llena o ilegible, la prueba puede funcionar sin informe guardado.

Para continuar el desarrollo, proporcionar este archivo con: modelo de consola
(V1/V2/OLED), modelo de dock, presencia o ausencia de imagen, estabilidad de la imagen,
y comportamiento tras parada/extracción. Una pantalla TV negra no basta para designar
un componente defectuoso.

## Alcance de esta etapa

- Negociación de alimentación USB-C / modo dock / DisplayPort a demanda.
- Lectura AUX, DPCD y EDID; entrenamiento del enlace; segunda cabeza de visualización.
- Copia de la ventana de prueba, sin rotación de la imagen de la TV.
- Ningún reinicio automático del BM92T36. Switch Lite excluida.
- Parada explícita, límite de 45 s en la negociación, registro acotado a 32 KiB.
- Liberación del búfer tras la parada del motor de vídeo; restauración de los ajustes
  LDO7/LDO8, del pad HPD y de la frecuencia BPMP guardados.

La fase de inicialización de vídeo es sincrónica, con sus propias esperas acotadas.
Durante esta fase, los botones se procesan cuando devuelve el control; los
45 segundos no constituyen un plazo máximo absoluto de esta fase.
La negociación puede dejar el contrato de alimentación activo tras la parada de la
prueba; una extracción del dock termina este contrato. La restauración completa de
todos los estados del puerto y las transiciones a los OS quedan por validar.

El espejo se detiene al salir de esta ventana. La salida TV permanente en
todos los menús Nova, el audio HDMI, la navegación sin táctil y el arranque
automático en modo dock aún no se entregan.

Para volver a Nova Pro v18, restaurar la carpeta `bootloader` guardada y luego
relanzar su payload. No se ha cambiado ningún archivo de la carpeta fuente `Desktop/nd5`.

Ver `documentation/TV-SOURCES.md` y `documentation/VERIFICATION-TV-01.json`
para la procedencia y las verificaciones de esta compilación.
