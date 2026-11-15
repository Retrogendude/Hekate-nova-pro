# Hekate Nova Pro 19-TV — première intégration de la sortie TV
**Languages:** [Français](LIRE-MOI-V19-TV.md) · [English](LIRE-MOI-V19-TV.en.md) · **Español** · [Русский](LIRE-MOI-V19-TV.ru.md)

Cette version reprend Nova Pro v18 et ajoute une sortie TV manuelle qui reste
active dans les menus. Base Hekate 6.5.4 / Nyx 1.9.4, interface Nova préservée.
Le prototype autonome 01, basé sur 6.5.5, reste disponible séparément.

## Lo que se ha validado

Has confirmado una imagen estable y la parada por VOL- con el prototipo 01 en
tu Switch OLED insertada directamente en una dock oficial. Los fallos reportados
usaban un cable entre la consola y la dock. Por lo tanto, no se consideran
un fallo confirmado del driver en conexión directa. El tipo exacto
de ese cable y su conexión no se han identificado.

Esta integración Nova es nueva: todavía debe probarse en la consola.
Sus pruebas en PC no sustituyen esa prueba. El driver de vídeo y sus parámetros
siguen idénticos a los del prototipo 01; las transiciones en Nova son nuevas.

## Instalación

1. Haz una copia de seguridad de tu carpeta `bootloader` actual, o utiliza una tarjeta SD de prueba.
2. Copia en la raíz la carpeta `bootloader` incluida en este archivo.
   Los archivos de configuración personales `hekate_ipl.ini` y `nyx.ini`
   no se incluyen, para conservar tus preferencias.
3. Inyecta `hekate_nova_pro_6.5.4_v19_tv.bin`. Comprueba en los créditos que la
   versión Nova indicada es **19-TV**. Copiar solo el payload sin los
   archivos de `bootloader/sys` no actualiza Nyx.

## Activar la TV y volver a los menús

1. Conecta la dock oficial a su alimentación y al televisor por HDMI.
2. Abre **Ajustes de Nyx → TV / Dock → TV USB-C / Dock**.
   El botón de ajustes rápidos de arriba a la derecha abre los ajustes de Nyx.
3. Elige **Activar la TV** e inserta la consola directamente en la dock.
   Puedes lanzarlo fuera de la dock: dispones de 45 segundos para colocarla.
4. Tras la conexión, cierra esta ventana. La salida TV permanece activa en la pantalla
   de inicio, las pestañas y las ventanas de Nova. El modo es 1280 × 720 a 60 Hz.
5. **VOL-** detiene la salida TV en cualquier momento entre las fases de inicialización.
   El botón **Detener la TV** también está disponible al volver a abrir sus ajustes.
6. Retirar la consola o perder la señal de la dock detiene la sesión. Para
   reanudarla, vuelve a colocar la consola y usa de nuevo **Activar la TV**.

La pantalla de la consola permanece encendida. Se conservan los controles de Nyx
existentes, incluido el puntero con los Joy-Con en los raíles cuando esa opción está activa.
Esta versión no añade la conexión Bluetooth de los mandos desplegados.

## Detención antes de otra función de hardware

Nova corta la salida TV antes de un lanzamiento de sistema o de payload, de una
recarga, de un apagado/reinicio, de un diagnóstico de hardware activo o de un uso
USB de tipo almacenamiento/mando. El búfer TV se libera tras la detención del motor
de visualización. La frecuencia del menú se restablece tras la negociación de la dock.

Por lo tanto, un diagnóstico activo puede volver a la pantalla de la consola. El resultado
sigue accesible en Nova; después puedes reactivar la TV manualmente.
La prueba de memoria conserva su payload separado y su retorno existente a Hekate.

El audio HDMI, la activación automática al arranque y la conexión con
cable intermedio no se validan en esta entrega.

## Informes

Los informes se escriben en la tarjeta SD en `bootloader/tv-test/`:

- `last-run.txt`: última prueba;
- `run-001.txt`, `run-002.txt`, etc.: copia distinta de cada prueba, hasta 999.

Un error mío en los prototipos 01/02 impedía la escritura normal
de los informes: el resultado de `sd_mount()` se interpretaba al revés.
Esta versión usa el contrato real del BDK, verificado por separado en PC.
Por lo tanto, no es necesario buscar un registro ausente del prototipo 01.

Para validar esta integración, comprueba la imagen en la pantalla de inicio tras cerrar
la ventana TV, la apertura de algunos submenús, VOL-, y luego una reconexión.
En caso de problema, adjunta el informe de la prueba junto con lo que aparece en la pantalla.

## Procedencia y comentarios

El archivo de fuentes contiene los archivos exactos utilizados, las pruebas, sus
resultados, la procedencia de los drivers y la licencia de Hekate. Consulta
`documentation/TV-SOURCES.md` antes de una posible redistribución pública.

Nova v18 y los prototipos 01/02 se conservan en sus archivos anteriores.
Para volver atrás, restaura la carpeta `bootloader` respaldada y lanza
el payload correspondiente. No se ha realizado ninguna modificación de tu carpeta de origen `Desktop/nd5`
durante este desarrollo.
