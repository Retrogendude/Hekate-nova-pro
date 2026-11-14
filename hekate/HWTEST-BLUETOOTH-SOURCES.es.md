# Nova27 — adaptación del host GUI Bluetooth de HWTest-RCM
**Languages:** [Français](HWTEST-BLUETOOTH-SOURCES.md) · [English](HWTEST-BLUETOOTH-SOURCES.en.md) · **Español** · [Русский](HWTEST-BLUETOOTH-SOURCES.ru.md)

Referencia: Creased/HWTest-RCM, commit
`74b66aab2352ae35912744fcd5441b07a0146307`, inspeccionado el 2026-11-13.

- [Implementación del host GUI](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/gui/gui_bt.c)
- [Documentación Bluetooth](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/docs/bluetooth.md)
- [Diagnóstico de radio](https://github.com/Creased/HWTest-RCM/blob/74b66aab2352ae35912744fcd5441b07a0146307/probe_bt.c)

Los `probe_bt.c` y `probe_wifi.c` actuales coinciden con las fuentes de diagnóstico
existentes de Nova tras normalizar los finales de línea. El host GUI antes pasado por alto es
la referencia adicional relevante. Soporta un solo controlador y realiza el
emparejamiento. Nova conserva su servicio BTstack de dos interlocutores, las claves importadas y el nivel de seguridad
2; su procedimiento de emparejamiento no se sustituye.

Las notas de hardware de la referencia informan de entradas perdidas del tamaño EDR con la ROM de la radio,
desconexiones relacionadas con sniff, y huecos de reinicio UART superiores a 300 us. Su código
selecciona paquetes de velocidad básica, permite inicialmente los cambios de rol/sniff, pide
QoS de baja latencia y devuelve los enlaces sniff al modo activo. El bucle de recepción permite
2 ms de silencio con un límite de llamada de 8 ms y una marca de agua FIFO de 8 bytes. Su arranque habilita
LDO1/LDO7 y las salidas de reloj de referencia del PMIC. Estos hechos guían el port de Nova.

Nova27 expresa este perfil a través de las API BTstack existentes y un pequeño planificador
de comandos pendientes. Los tipos BR se seleccionan para las llamadas salientes y se piden de nuevo
por cada enlace establecido, incluidas las conexiones entrantes. Los eventos de tipo de paquete recibido y de modo
se registran por separado del envío de comandos. Los parámetros QoS siguen
la referencia. No se introduce ningún firmware propietario ni ningún nuevo emparejamiento.

La bomba de recepción de Nova además devuelve inmediatamente en una línea inicialmente vacía
y tiene un límite de 2048 bytes. El servicio de dos controladores conserva la corrección HID cruzado
de v26. Los ajustes de tensión de los raíles se conservan; el desmontaje restaura solo los bits de control del PMIC
propios y el nivel de reset Wi-Fi anterior, sin escrituras en PH0.

Las nuevas pruebas modelan la pérdida de entrada EDR documentada y las pausas UART temporizadas. La pila
v26 real falla esas regresiones; v27 recibe informes completos, aplica BR/QoS,
gestiona la recuperación de sniff y lee a través de pausas de 700 us. Se comprueban el presupuesto de bomba de
8 ms y los metadatos de configuración de múltiples comandos. Las pruebas de registros de plataforma cubren
alimentaciones/relojes ya habilitados, la restauración y cambios de bits del PMIC no relacionados.
Estas pruebas en PC sustituyen el hardware; ni ellas ni los resultados de banco de upstream
establecen un funcionamiento correcto en la consola del usuario.

El JSON de auditoría registra los hashes de los archivos upstream y el resultado de la comparación. El código fuente de la GUI
upstream se referencia en lugar de empaquetarse como un segundo backend sin modificar.
Los avisos existentes de BTstack y HWTest permanecen en el proyecto.
