# Hekate Nova Pro 29 — emparejamientos, copia de seguridad SD y reconexión
**Languages:** [Français](LIRE-MOI-V29.md) · [English](LIRE-MOI-V29.en.md) · **Español** · [Русский](LIRE-MOI-V29.ru.md)

Formatear la tarjeta SD no es una corrección del problema observado.
Los datos de emparejamiento se leen en los Joy-Con. La v28 recibió los dos
mandos en una OLED, mientras que el R perdió su conexión en la otra.
El paso por Nintendo restableció la importación 2/2; una nueva prueba luego
reprodujo 0/2 en la segunda OLED. La causa exacta en hardware queda por verificar.

Esta versión acepta los dos perfiles de host ya validados por el lector BDK:
Nintendo y estándar. La v28 exigía la clasificación Nintendo incluso para un
registro estándar reconocido como válido. La nueva ruta conserva la comprobación
de suma, la entrada activa, las claves no nulas, los lados y la
compatibilidad de los dos hosts. Las respuestas SPI truncadas ya no llenan
la caché. La exportación histórica de Hekate conserva su clasificación Nintendo.

El tiempo de espera de un intento de conexión pasa de 15,36 s a 5,12 s. Los intentos
continúan, espaciados, mientras el servicio Bluetooth está activado; ya no
se detienen en el tercer fallo. Solo un intento saliente permanece activo
a la vez. Una conexión restablecida vuelve a poner el contador de reintentos a cero.
Esto mejora la recuperación tras el despertar o un corte; el despertar con un botón normal
y la ausencia de cortes en las dos consolas todavía deben probarse.
Se conservan la velocidad de radio, el cifrado, la gestión de canales y las teclas.

Dos nuevas acciones figuran en el menú Joy-Con inalámbricos:

- **Guardar los emparejamientos**: tras una lectura válida, Bluetooth detenido.
  Un archivo de 72 bytes contiene los datos necesarios de los dos mandos,
  en `bootloader/bluetooth/joycon-pairing.bin`. No es una copia
  completa de la memoria de los Joy-Con. Las claves se guardan en claro:
  mantener este archivo privado y no enviarlo con los informes.
- **Cargar los emparejamientos**: recarga explícitamente este archivo en la
  sesión tras un reinicio. Un error de formato o de suma lo hace
  rechazar, conservando las claves ya presentes en memoria.

La copia de seguridad usa un archivo temporal y luego conserva la versión anterior
en `joycon-pairing.bak`. La carga intenta esta copia si el archivo principal
falta o está dañado. El archivo `.bak` también contiene claves privadas.
Una copia de seguridad con un solo mando válido sigue siendo una copia 1/2.
«Borrar las claves» borra la sesión en memoria, no los archivos de copia de seguridad.
Los archivos deben usarse con los mandos asociados. Para comparar las
dos consolas, leer y guardar los emparejamientos correspondientes a cada prueba.
Si se realiza un nuevo emparejamiento en el sistema Nintendo, volver a leer y luego
guardar los datos actualizados antes de reutilizar la copia de seguridad SD.

Instalación y control:

1. Extraer el ZIP en la raíz de la SD sustituyendo `bootloader`, sin formatear.
   Usar `hekate_nova_pro_6.5.5_v29.bin` si es necesario, y luego reiniciar.
2. Con los Joy-Con fijados, leer los emparejamientos. Si es 2/2, guardarlos antes
   de activar Bluetooth. Si persiste 0/2, detener y recuperar el informe.
3. Desprender los mandos, activar Bluetooth y probar primero un botón normal
   de cada mando para despertarlos. Probar el movimiento y A/B.
4. Detener Bluetooth y recuperar inmediatamente `bootloader/bluetooth/last-session.txt`,
   sin borrar las claves ni volver a leer los emparejamientos. No adjuntar los archivos
   de emparejamiento `.bin` o `.bak`.
5. Para comprobar la persistencia: reiniciar Nova, cargar los emparejamientos desde
   la SD, y luego repetir la prueba. Para esta carga no es necesaria ninguna lectura
   de los raíles si la copia de seguridad es válida.

El informe añade `recognized` (perfil válido reconocido) y `source` (1 raíles,
2 SD). Durante una carga SD, la clasificación Nintendo no se rellena;
los indicadores de raíles no representan una nueva lectura física.
El tipo de traza 20 indica el tiempo de espera del intento solicitado, en unidades Bluetooth.
No se añade ninguna clave, dirección o dato bruto al informe.

La compilación y las comprobaciones en PC no sustituyen la validación de hardware.
La recuperación tras un tiempo de espera agotado se prueba con una radio simulada; no
prueba que la causa de los cortes de R haya desaparecido. No se envía ningún borrado ni
nuevo emparejamiento a los Joy-Con. Base: Hekate 6.5.5 / Nyx 1.9.5.
Los ajustes de usuario no se incluyen en el ZIP. Licencias conservadas.
