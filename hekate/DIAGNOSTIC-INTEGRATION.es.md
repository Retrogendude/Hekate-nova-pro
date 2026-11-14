# Presentación actual — Nova Pro 11
**Languages:** [Français](DIAGNOSTIC-INTEGRATION.md) · [English](DIAGNOSTIC-INTEGRATION.en.md) · **Español** · [Русский](DIAGNOSTIC-INTEGRATION.ru.md)

Las doce categorías usan la selección y luego la apertura en dos pulsaciones. Las sondas y resultados se presentan en tarjetas paginadas, con nombres legibles y acceso a los informes. Las 42 funciones de medición, sus protecciones y el servicio RAM aislado siguen siendo los de la integración descrita abajo. El retorno RAM y los límites de los payloads se controlan en la verificación v11; las pruebas físicas siguen siendo necesarias.

---
## Integración de hardware y funcionamiento

# Diagnóstico de hardware — integración nativa, prototipo 5

Las 42 entradas de la tabla HWTest-RCM 0.15 están integradas en Nyx: identidad,
fusibles y KFUSE; alimentación, carga, batería y térmica; memoria y
relojes; SD, eMMC, particiones, GPT, PRODINFO y lector de cartuchos;
Bluetooth, Wi-Fi y complejo CPU; códec, relojes y salidas de audio; pantalla
y retroiluminación; táctil, luz ambiental, Joy-Con y botones; GPIO,
UART, causa de reinicio y registros PMC; síntesis y cruces de datos.

La pestaña Diagnóstico ofrece un balance, diez categorías y el servicio RAM aislado.
Cada categoría permite una ejecución colectiva o sonda por sonda. Los
resultados distinguen OK, aviso, defecto, error, observación y no
ejecutado. Una ausencia de medición o de requisito previo no se convierte en un éxito.
Los informes pueden consultarse por páginas y luego exportarse a la tarjeta SD
en `bootloader/diagnostics/hwtest_XXXXXXXX.txt`, sin relanzar las mediciones.
La captura está limitada a 256 KiB con un marcador explícito en caso de truncamiento.

## Adaptación a Nyx

Las sondas están presentes en `nyx/nyx_gui/diagnostics/vendor/`. El programa
principal, la interfaz y el registro en pantalla/UART del payload original
no se usan: un adaptador nativo captura el informe y publica el
progreso en las ventanas Nyx. El driver de los comandos y las tareas de
mantenimiento se suspenden durante las mediciones y luego se restablecen. Los datos,
estados GPIO/pads, ventilador, audio, PCIe, partición eMMC y emplazamientos de claves
modificados se restauran cuando su acceso está disponible. La reactivación del
táctil usa la vía sin autocalibración persistente.

El Wi-Fi comprende realmente el programa AArch64 CPU0 necesario: la build falla
si este programa no está disponible. Reserva A0000000..A003FFFF, fuera
del heap de Nyx y de las imágenes de pantalla. Los mensajes de diagnóstico nunca
se envían por el UART-B usado por el Joy-Con derecho.

## Requisitos e interpretación

Algunas mediciones dependen del modelo o de componentes presentes. Las lecturas
PRODINFO descifradas requieren `switch/prod.keys` de esta consola y emplazamientos SE
legibles/modificables; si no, se señalan como no
realizadas o con aviso, sin sobrescribir las claves bloqueadas de Nyx.
La prueba sonora reproduce las salidas sucesivamente y debe verificarse escuchando.
Un veredicto del transporte de audio no prueba que el altavoz sea audible.

## RAM separada y retorno a Hekate

`bootloader/sys/hwtest_ram.bin` es un programa separado: se sale de Nyx para
no conservar una interfaz en la memoria sometida a las pruebas. El servicio
ejecuta una sola secuencia de los siete patrones avanzados sobre todas las zonas
accesibles y no reservadas, y luego recarga Hekate. El retorno fuerza el menú y
una nueva instancia de Nyx muestra el resultado conservado, incluido tras
la cancelación o el fallo de la prueba. Los primeros 4 MiB, incluida la imagen de retorno y su
cabecera, permanecen excluidos. Una imagen de retorno corrupta nunca se ejecuta;
un fallo de hardware o un bloqueo del controlador puede impedir cualquier retorno.

El tamaño de las dos imágenes se controla antes de la asignación, y las lecturas
cortas o fallidas se rechazan. La imagen de retorno
viene de `bootloader/update.bin`, se reconoce como Hekate, está limitada a 128 KiB y se
controla por huella antes de su uso. El servicio RAM está limitado a 130392
bytes. El contrato compartido y su copia se verifican idénticos.

## Validación

Construcción ARM Hekate/Nyx, programa AArch64 Wi-Fi y servicio RAM correctos.
Pruebas C sobre funciones reales: copia de seguridad de configuración, lecturas limitadas,
captura/exportación y veredicto parcial, contrato de retorno RAM. La descompresión del
servicio se verifica por el decodificador C real y los binarios se comparan con los
ELF enlazados. Todavía no se ha realizado ninguna prueba en consola ni validación física
de las mediciones.

## Procedencia

HWTest-RCM: archivo de usuario 0.15, commit
778e25b66a11c5cdd65c11a69ef8083aff1cd9f7, repositorio Creased/HWTest-RCM.
Hekate: archivo de usuario 6.5.4, commit
45cd81ed08e30a26ba380958c7e20b55c342834b. Atmosphère del servicio RAM: commit
cb4b882e3b176480ac57a1161a85ff175c3f162c, dependencia exacta de HWTest.
Se conservan los comentarios de procedencia, la licencia Hekate y la licencia Atmosphère.
Las adaptaciones se describen en diagnostics/PROVENANCE.md
y en el compañero HWTest-RCM, ramtest/NYX-RETURN.md y DEPENDENCIES.json.
