# Hekate Nova Pro 28 — reactividad y diagnóstico de los emparejamientos
**Languages:** [Français](LIRE-MOI-V28.md) · [English](LIRE-MOI-V28.en.md) · **Español** · [Русский](LIRE-MOI-V28.ru.md)

Esta versión reduce las esperas Bluetooth repetidas durante el dibujado de los
menús: la lectura serie usa un presupuesto corto de 250 microsegundos en
las llamadas de visualización y del temporizador de la ventana. El bucle principal
conserva la lectura prolongada necesaria para el perfil de radio HWTest-RCM.
El panel de estadísticas se refresca dos veces por segundo.
Esto apunta a las ralentizaciones señaladas en v27; el resultado en consola
sigue por medir. No es una garantía de frecuencia de visualización.

El menú ahora indica la categoría de fallo de la importación: raíles
no disponibles, datos no leídos, datos Nintendo no reconocidos por el lector,
o controles de compatibilidad no satisfechos. Un fallo 0/2 puede registrarse
sin iniciar Bluetooth. Las validaciones y el formato de las claves siguen siendo los
de v27: **la causa del 0/2 observado en consola aún no está corregida**.

El informe añade indicadores de importación y, para cada Joy-Con,
los botones observados desde el inicio de la sesión, las fechas del primer y
último informe y su cadencia media. No contiene ni dirección, ni clave,
ni contenido bruto de los paquetes. «Buttons seen» conserva una pulsación incluso si el
botón ya se ha soltado. El botón A del Joy-Con derecho corresponde al bit 000008.
La cadencia media abarca los informes decodificados, incluidas las respuestas
de comando, y no mide la latencia de una pulsación.

Instalación y prueba para el problema actual:

1. Extraer el ZIP en la raíz de la SD sustituyendo los archivos de la carpeta
   bootloader. Usar hekate_nova_pro_6.5.5_v28.bin si es necesario, y luego reiniciar.
2. Fijar los dos Joy-Con y elegir «Leer los emparejamientos».
3. Si el resultado sigue siendo 0/2, anotar el nuevo mensaje y elegir
   «Detener Bluetooth», incluso sin haberlo iniciado. Transmitir únicamente
   bootloader/bluetooth/last-session.txt.
4. Si la importación se realiza correctamente, desprender los Joy-Con, iniciar Bluetooth, mover
   el cursor con L y pulsar varias veces A del R. Luego detener
   Bluetooth y recuperar el informe antes de cualquier nueva sesión.

Se conservan los menús, el perfil de radio, la gestión de las conexiones simultáneas, los
diagnósticos y el modo TV. Ningún ajuste de usuario se
incluye en el ZIP. No se añade ningún nuevo emparejamiento ni cambio de claves.
Base: Hekate 6.5.5 / Nyx 1.9.5. Licencias y procedencia incluidas.

Las comprobaciones en PC y la compilación no sustituyen la prueba de hardware.
En v27, se señaló un movimiento con L con una fuerte latencia, sin
validación con A del R. Los últimos informes incluidos muestran una conexión
no establecida, y luego una importación a 0/2: no describen esta sesión activa.
