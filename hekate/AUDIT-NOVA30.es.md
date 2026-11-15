# Hekate Nova Pro — auditoría global y optimizaciones v30
**Languages:** [Français](AUDIT-NOVA30.md) · [English](AUDIT-NOVA30.en.md) · **Español** · [Русский](AUDIT-NOVA30.ru.md)

Revisión de las fuentes de la representación, de la planificación, de los comandos inalámbricos, de las
entradas, del arranque, de las cargas SD, de la memoria, de los diagnósticos y
del modo TV. Las pruebas usan las funciones reales con periféricos
sustituidos en PC. No se anuncia ningún resultado de frecuencia de representación ni de latencia
física v30 antes de una prueba en consola.

## Cambios integrados

| Ámbito | Constatación | Corrección y prueba |
| --- | --- | --- |
| Arranque | Una sesión vacía obligaba a cargar las claves a mano | Tras la inicialización del driver, detección física de los raíles. Si no hay Joy-Con: carga del `.bin` validado, y luego arranque en segundo plano sin ventana. Pruebas de ausencia/1 raíl/2 raíles, error SD, carrera de fijación y detención manual. |
| Reflejo | La cola de 32 zonas podía desbordarse antes del refresco e imponer una pantalla completa | Precálculo del presupuesto global, transiciones diferidas y fase en tiempo real. 9 escenarios corregidos sin desbordamiento periódico; representación de tres fases idéntica a v29. |
| Pulsaciones breves | La pulsación y la suelta recibidas entre dos lecturas UI podían anularse | Presentación única de los flancos recientes, caducidad de 100 ms por botón, ninguna espera añadida; limpieza en el corte/Stop y prioridad a los raíles. La v29 real falla la prueba específica. |
| Servicio radio | El contador de duración solo cubría la parte RX | Nuevos máximos de llamada completa, de variante rápida/normal y de intervalo entre llamadas. El procesamiento del paquete actual cuenta en el presupuesto RX. |
| Herramientas SD/USB largas | Algunos procesamientos no mantienen el bucle GUI completo | El punto de mantenimiento manual llama al polling rápido, fuera de los diagnósticos que reservan el hardware. Prueba del polling y de la suspensión. |
| Recursos | `res.pak` podía superar su región o leerse parcialmente | Límites antes de la lectura, tamaño mínimo de los recursos referenciados, número realmente leído y cierre controlados. |
| Payloads | Un archivo rechazado podía quedarse asignado; tamaño verificado tras la asignación | Verificación antes de la asignación y limpieza común; lecturas incompletas o fallidas rechazadas. 20 casos I/O pasan, 8 defectos antiguos reproducidos con el código v29 real. |

## Mediciones de representación en PC

Con 33 ms entre refrescos en un ciclo simulado de 8,48 s, las superficies
redibujadas bajan del 52,1 % en la pantalla de inicio, del 73,7 % en Herramientas y del 53,4 % en el
carrusel. Los refrescos completos causados por el reflejo pasan
respectivamente de 5/5/3 a 0 en estas escenas. Son superficies, no ganancias de FPS.
Las pruebas también retrasan el refresco a 66 y 100 ms y preservan las
invalidaciones de los demás widgets cuando la cola ya está llena.

## Puntos conservados

Perfil de radio BR/sniff/QoS, velocidad UART, control de flujo, política de claves,
archivo de copia de seguridad de 72 bytes, elección de idioma/fuente/colores y estilo Nova
siguen siendo compatibles. El loader y los módulos del sistema no reciben las
optimizaciones GUI. La preparación de la distribución verifica los tamaños y
las cabeceras reales para rechazar un binario incompatible.

El servicio es cooperativo: funciona en la pantalla de inicio y los menús ordinarios
sin panel Bluetooth abierto. Las sondas de hardware que reservan sus
periféricos conservan su suspensión de los comandos; la auditoría no reactiva
las tareas durante las ventanas de medición.

## Continuación recomendada

| Prioridad | Vía identificada | Trabajo restante |
| --- | --- | --- |
| Fiabilidad | Helper SD común que puede devolver éxito tras una escritura incompleta | Corregir su contrato y probar todas las exportaciones/configuraciones llamadoras. La copia de seguridad Bluetooth dedicada ya verifica estos errores. |
| Fiabilidad | Lectura de Nyx por el bootloader insuficientemente limitada | Añadir una validación local compatible con el escaso margen IRAM; conservar la cabecera fija y los límites de payload. |
| Fluidez | Decodificación PNG y escaneos de fuentes durante las reconstrucciones | Medir y luego introducir cachés limitadas con invalidación de los archivos sustituidos. |
| Arranque | Retardos de animación independientes del trabajo realizado | Cubrir animación y lecturas sin cambiar los retardos eléctricos, el training de memoria y la rampa de pantalla. |
| Diagnósticos | Fases largas sin estado intermedio | Añadir un refresco limitado entre fases seguras, sin perturbar las mediciones. |
| TV | Negociación de enlace y presentación a veces bloqueantes | Dividir en estados temporizados tras mediciones en la dock. |

Estos puntos provienen de rutas de código identificadas. No se describen
como fallos de hardware constatados y no todos están modificados en
la v30. Los detalles y referencias figuran en los tres informes especializados.

## Verificación en consola

Conservar una copia de seguridad de emparejamiento válida. Probar el arranque sin raíles,
con solo L, con solo R y con los dos Joy-Con fijados. Sin raíles,
verificar la navegación desde la pantalla de inicio y en las ventanas sin abrir
Bluetooth. Una detención manual debe seguir detenida hasta una acción manual o
un nuevo arranque. Comparar los mismos recorridos y enviar únicamente el
informe `last-session.txt`, nunca los archivos que contienen las claves.

Detalles: [representación](AUDIT-NOVA30-GUI.md), [Bluetooth](AUDIT-NOVA30-WIRELESS.md), [núcleo y cargas](AUDIT-NOVA30-CORE.md).
