"""Curated electrical terminology and critical explanations after offline MT."""
from pathlib import Path
import json,re
from audit_localization_coverage import literals,FORMATS
WORK=Path(__file__).resolve().parent
ROOT=WORK/'hekate-6.5.4/nyx/nyx_gui'
ROWS=r'''
on~activé~activado~attivo~включено~an~ativado
off~désactivé~desactivado~disattivo~выключено~aus~desativado
PASS~RÉUSSI~CORRECTO~RIUSCITO~УСПЕХ~BESTANDEN~PASSOU
FAIL~ÉCHEC~FALLO~ERRORE~СБОЙ~FEHLER~FALHA
WARN~AVERT.~AVISO~AVVISO~ПРЕДУПР.~WARNUNG~AVISO
asserted~activé~activado~attivo~активен~aktiv~ativo
released~relâché~liberado~rilasciato~снят~freigegeben~libertado
set~positionné~establecido~impostato~установлен~gesetzt~definido
clear~à zéro~a cero~azzerato~сброшен~gelöscht~a zero
driven~piloté~controlado~pilotato~управляется~angesteuert~acionado
tristate~haute impédance~alta impedancia~alta impedenza~высокий импеданс~hochohmig~alta impedância
no pull~sans résistance de tirage~sin resistencia de polarización~senza resistenza di polarizzazione~без подтяжки~ohne Pull-Widerstand~sem resistência de polarização
no error~aucune erreur~sin error~nessun errore~ошибок нет~kein Fehler~sem erro
short~court-circuit~cortocircuito~cortocircuito~короткое замыкание~Kurzschluss~curto-circuito
powered~alimenté~alimentado~alimentato~питание подано~versorgt~alimentado
unpowered~non alimenté~sin alimentación~non alimentato~нет питания~unversorgt~sem alimentação
out of range~hors plage~fuera de rango~fuori intervallo~вне диапазона~außerhalb des Bereichs~fora do intervalo
codec already answering~le codec répond déjà~el códec ya responde~il codec risponde già~кодек уже отвечает~Codec antwortet bereits~o codec já responde
PZ4 drive %s : pad reads %d pinmux %08X~Pilotage PZ4 %s : lecture du pad %d, pinmux %08X~Control PZ4 %s: lectura del pad %d, pinmux %08X~Comando PZ4 %s: lettura pad %d, pinmux %08X~Управление PZ4 %s: значение контакта %d, pinmux %08X~Ansteuerung PZ4 %s: Pad-Wert %d, pinmux %08X~Comando PZ4 %s: leitura do pad %d, pinmux %08X
Codec ID : no answer at 0x1C after %d ms~ID codec : aucune réponse à 0x1C après %d ms~ID de códec: sin respuesta en 0x1C tras %d ms~ID codec: nessuna risposta a 0x1C dopo %d ms~ID кодека: нет ответа по 0x1C за %d ms~Codec-ID: nach %d ms keine Antwort auf 0x1C~ID do codec: sem resposta em 0x1C após %d ms
driven both ways, no answer~deux niveaux de pilotage essayés, aucune réponse~ambos niveles de control probados, sin respuesta~provati entrambi i livelli di comando, nessuna risposta~проверены оба уровня управления, ответа нет~beide Ansteuerpegel getestet, keine Antwort~testados ambos os níveis de comando, sem resposta
enabled by driving HIGH~activé par un pilotage à HIGH~activado con nivel HIGH~attivato pilotando HIGH~включён уровнем HIGH~durch HIGH-Pegel aktiviert~ativado pelo nível HIGH
enabled by driving LOW~activé par un pilotage à LOW~activado con nivel LOW~attivato pilotando LOW~включён уровнем LOW~durch LOW-Pegel aktiviert~ativado pelo nível LOW
not driven (UART/dead)~non piloté (UART ou inactif)~sin control (UART o inactivo)~non pilotato (UART o inattivo)~нет управления (UART или неактивен)~nicht angesteuert (UART oder inaktiv)~sem comando (UART ou inativo)
never low~jamais à LOW~nunca en LOW~mai LOW~уровень LOW не обнаружен~nie LOW~nunca em LOW
answered on a swept MCR~réponse obtenue pendant le balayage MCR~respuesta durante el barrido MCR~risposta durante la scansione MCR~ответ при переборе MCR~Antwort beim MCR-Durchlauf~resposta durante a varredura MCR
no reply on any MCR value~aucune réponse avec les valeurs MCR essayées~sin respuesta con los valores MCR probados~nessuna risposta con i valori MCR provati~нет ответа при проверенных значениях MCR~keine Antwort bei geprüften MCR-Werten~sem resposta com os valores MCR testados
slot empty~port vide~ranura vacía~slot vuoto~слот пуст~Steckplatz leer~ranhura vazia
cartridge seated~cartouche insérée~cartucho insertado~cartuccia inserita~картридж вставлен~Modul eingesetzt~cartucho inserido
N/A : Switch Lite has no ALS (auto-brightness omitted)~Sans objet : la Switch Lite n'a pas d'ALS (luminosité automatique absente)~No aplicable: Switch Lite no tiene ALS (sin brillo automático)~Non applicabile: Switch Lite non ha ALS (luminosità automatica assente)~Неприменимо: у Switch Lite нет ALS (автояркость отсутствует)~Nicht anwendbar: Switch Lite hat keinen ALS (keine Autohelligkeit)~Não aplicável: Switch Lite não tem ALS (sem brilho automático)
- RCM bit (next reboot would re-enter BootROM RCM)~- Bit RCM (le prochain redémarrage retournerait en BootROM RCM)~- Bit RCM (el próximo reinicio volvería a BootROM RCM)~- Bit RCM (il prossimo riavvio tornerebbe a BootROM RCM)~- Бит RCM (следующий перезапуск вернёт в BootROM RCM)~- RCM-Bit (nächster Neustart würde wieder BootROM RCM starten)~- Bit RCM (o próximo reinício regressaria ao BootROM RCM)
- BOOTLOADER bit (CFW bootloader chainload requested)~- Bit BOOTLOADER (chaînage du chargeur CFW demandé)~- Bit BOOTLOADER (solicitado el encadenamiento del cargador CFW)~- Bit BOOTLOADER (richiesto concatenamento del bootloader CFW)~- Бит BOOTLOADER (запрошен переход к загрузчику CFW)~- BOOTLOADER-Bit (CFW-Bootloader-Chainload angefordert)~- Bit BOOTLOADER (pedido encadeamento do carregador CFW)
- bit 18 set: ODM debug-console flag (HOS would enable kgdb)~- Bit 18 positionné : indicateur de console ODM (HOS activerait kgdb)~- Bit 18 activado: indicador de consola ODM (HOS activaría kgdb)~- Bit 18 impostato: console di debug ODM (HOS attiverebbe kgdb)~- Бит 18 установлен: консоль отладки ODM (HOS включит kgdb)~- Bit 18 gesetzt: ODM-Debugkonsole (HOS würde kgdb aktivieren)~- Bit 18 definido: consola de depuração ODM (HOS ativaria kgdb)
PMIC IRQSD : 0x%02X (SD-rail thresholds latched - cross-check NVERC)~PMIC IRQSD : 0x%02X (seuils d'alimentation SD mémorisés ; vérifier NVERC)~PMIC IRQSD: 0x%02X (umbrales de alimentación SD registrados; comprobar NVERC)~PMIC IRQSD: 0x%02X (soglie alimentazione SD registrate; verificare NVERC)~PMIC IRQSD: 0x%02X (пороги питания SD сохранены; сверить NVERC)~PMIC IRQSD: 0x%02X (SD-Versorgungsschwellen gespeichert; NVERC prüfen)~PMIC IRQSD: 0x%02X (limiares de alimentação SD registados; verificar NVERC)
RTC adjusted : not available (no /bootloader/nyx.ini timeoffset)~RTC ajustée : indisponible (pas de timeoffset dans /bootloader/nyx.ini)~RTC ajustado: no disponible (sin timeoffset en /bootloader/nyx.ini)~RTC corretto: non disponibile (manca timeoffset in /bootloader/nyx.ini)~RTC с поправкой: недоступно (нет timeoffset в /bootloader/nyx.ini)~RTC korrigiert: nicht verfügbar (kein timeoffset in /bootloader/nyx.ini)~RTC ajustado: indisponível (sem timeoffset em /bootloader/nyx.ini)
idle anchor : PULL_DOWN (line idles LOW - UART will see permanent BREAK)~Repos : PULL_DOWN (ligne au niveau LOW ; UART voit un BREAK permanent)~Reposo: PULL_DOWN (línea en LOW; UART ve un BREAK permanente)~Riposo: PULL_DOWN (linea LOW; UART vede BREAK permanente)~Покой: PULL_DOWN (линия LOW; UART видит постоянный BREAK)~Ruhepegel: PULL_DOWN (Leitung bleibt LOW; UART sieht dauerhaften BREAK)~Repouso: PULL_DOWN (linha em LOW; UART deteta BREAK permanente)
^- framing/parity/break errors latched - check baud~^- Erreurs de trame/parité/BREAK mémorisées ; vérifier le débit~^- Errores de trama/paridad/BREAK registrados; comprobar velocidad~^- Errori frame/parità/BREAK registrati; verificare baud rate~^- Сохранены ошибки кадра/чётности/BREAK; проверьте скорость~^- Frame-/Paritäts-/BREAK-Fehler gespeichert; Baudrate prüfen~^- Erros de trama/paridade/BREAK registados; verificar velocidade
Verdict : line ACTIVE (%d transitions) - host IS sending~Verdict : ligne ACTIVE (%d transitions) ; l'hôte émet bien~Veredicto: línea ACTIVE (%d transiciones); el host sí transmite~Verdetto: linea ACTIVE (%d transizioni); l'host trasmette~Вердикт: линия ACTIVE (%d переходов); хост передаёт~Ergebnis: Leitung ACTIVE (%d Wechsel); Host sendet~Veredito: linha ACTIVE (%d transições); o anfitrião transmite
Verdict : line stuck LOW - shifter unpowered or shorted to GND~Verdict : ligne bloquée à LOW ; convertisseur de niveau non alimenté ou court-circuit vers GND~Veredicto: línea bloqueada en LOW; conversor de nivel sin alimentación o en corto a GND~Verdetto: linea bloccata LOW; convertitore di livello non alimentato o in corto a GND~Вердикт: линия постоянно LOW; преобразователь уровней без питания или замкнут на GND~Ergebnis: Leitung dauerhaft LOW; Pegelwandler unversorgt oder Kurzschluss nach GND~Veredito: linha presa em LOW; conversor de nível sem alimentação ou em curto a GND
(all rails dormant - normal in RCM, HOS brings them up)~(alimentations au repos ; normal en RCM, HOS les active)~(alimentaciones en reposo; normal en RCM, HOS las activa)~(alimentazioni a riposo; normale in RCM, HOS le attiva)~(питание отключено; нормально в RCM, HOS его включает)~(alle Versorgungen ruhend; in RCM normal, HOS aktiviert sie)~(alimentações em repouso; normal em RCM, HOS ativa-as)
skipped (encrypted, run Lockpick to populate sd:/switch/prod.keys)~ignoré (chiffré ; exécuter Lockpick pour créer sd:/switch/prod.keys)~omitido (cifrado; ejecutar Lockpick para crear sd:/switch/prod.keys)~saltato (cifrato; eseguire Lockpick per creare sd:/switch/prod.keys)~пропущено (зашифровано; запустите Lockpick для создания sd:/switch/prod.keys)~übersprungen (verschlüsselt; Lockpick für sd:/switch/prod.keys ausführen)~ignorado (cifrado; executar Lockpick para criar sd:/switch/prod.keys)
pkg1 build : (BOOT0 sector 0x800 read all-zero - pkg1 absent)~Compilation pkg1 : (secteur BOOT0 0x800 entièrement nul ; pkg1 absent)~Compilación pkg1: (sector BOOT0 0x800 leído a ceros; pkg1 ausente)~Build pkg1: (settore BOOT0 0x800 letto a zero; pkg1 assente)~Сборка pkg1: (сектор BOOT0 0x800 прочитан как нули; pkg1 отсутствует)~pkg1-Build: (BOOT0-Sektor 0x800 nur Nullen; pkg1 fehlt)~Compilação pkg1: (setor BOOT0 0x800 lido a zeros; pkg1 ausente)
ent CRC32 : skipped (entries span %d bytes, only %d in read window)~CRC32 entrées : ignoré (%d octets nécessaires, seulement %d dans la fenêtre lue)~CRC32 entradas: omitido (%d bytes necesarios, solo %d en la ventana leída)~CRC32 voci: saltato (%d byte necessari, solo %d nella finestra letta)~CRC32 записей: пропущено (нужно %d байт, в окне чтения только %d)~Eintrags-CRC32: übersprungen (%d Bytes nötig, nur %d im Lesefenster)~CRC32 entradas: ignorado (%d bytes necessários, apenas %d na janela lida)
Burnt fuses : %d | %d (ODM7 | ODM6)~Fusibles grillés : %d | %d (ODM7 | ODM6)~Fusibles quemados: %d | %d (ODM7 | ODM6)~Fusibili bruciati: %d | %d (ODM7 | ODM6)~Пережжённые предохранители: %d | %d (ODM7 | ODM6)~Gebrannte Sicherungen: %d | %d (ODM7 | ODM6)~Fusíveis queimados: %d | %d (ODM7 | ODM6)
CHRG<->Curr : BQ chrg=%d, gauge=%d mA %s~Charge/courant : BQ chrg=%d, jauge=%d mA %s~Carga/corriente: BQ chrg=%d, medidor=%d mA %s~Carica/corrente: BQ chrg=%d, misuratore=%d mA %s~Заряд/ток: BQ chrg=%d, датчик=%d mA %s~Ladung/Strom: BQ chrg=%d, Messwert=%d mA %s~Carga/corrente: BQ chrg=%d, medidor=%d mA %s
CPU0 : EXCEPTION at stage %d, not a stall~CPU0 : EXCEPTION à l'étape %d, sans blocage~CPU0: EXCEPTION en la etapa %d, sin bloqueo~CPU0: EXCEPTION nella fase %d, senza blocco~CPU0: EXCEPTION на этапе %d, без зависания~CPU0: EXCEPTION in Phase %d, kein Stillstand~CPU0: EXCEPTION na fase %d, sem bloqueio
CPU0 : stalled (attempt %d/3, hb=%d) - resetting AFI~CPU0 : bloqué (essai %d/3, hb=%d) ; réinitialisation AFI~CPU0: bloqueado (intento %d/3, hb=%d); reiniciando AFI~CPU0: bloccato (tentativo %d/3, hb=%d); reset AFI~CPU0: зависание (попытка %d/3, hb=%d); сброс AFI~CPU0: Stillstand (Versuch %d/3, hb=%d); AFI zurücksetzen~CPU0: bloqueado (tentativa %d/3, hb=%d); reiniciar AFI
GPIO PV1 SIG : %s (LCD_BL_EN, low on some Mariko panels)~GPIO PV1 SIG : %s (LCD_BL_EN, niveau bas sur certains écrans Mariko)~GPIO PV1 SIG: %s (LCD_BL_EN, nivel bajo en algunas pantallas Mariko)~GPIO PV1 SIG: %s (LCD_BL_EN, livello basso su alcuni schermi Mariko)~GPIO PV1 SIG: %s (LCD_BL_EN, низкий уровень на части экранов Mariko)~GPIO PV1 SIG: %s (LCD_BL_EN, bei manchen Mariko-Displays LOW)~GPIO PV1 SIG: %s (LCD_BL_EN, nível baixo em alguns ecrãs Mariko)
Result : ADMAIF FIFO stayed full for >5 ms~Résultat : ADMAIF FIFO est resté plein pendant >5 ms~Resultado: ADMAIF FIFO permaneció lleno durante >5 ms~Risultato: ADMAIF FIFO rimasta piena per >5 ms~Результат: ADMAIF FIFO оставался заполненным >5 ms~Ergebnis: ADMAIF FIFO blieb >5 ms voll~Resultado: ADMAIF FIFO permaneceu cheio por >5 ms
UART_LCR : 0x%02X (DLAB=%d, word=%d)~UART_LCR : 0x%02X (DLAB=%d, mot=%d)~UART_LCR: 0x%02X (DLAB=%d, palabra=%d)~UART_LCR: 0x%02X (DLAB=%d, parola=%d)~UART_LCR: 0x%02X (DLAB=%d, слово=%d)~UART_LCR: 0x%02X (DLAB=%d, Wort=%d)~UART_LCR: 0x%02X (DLAB=%d, palavra=%d)
eMMC HS400 : mode=%d, bus=%d-bit %s~eMMC HS400 : mode=%d, bus de %d bits %s~eMMC HS400: modo=%d, bus de %d bits %s~eMMC HS400: modo=%d, bus a %d bit %s~eMMC HS400: режим=%d, шина %d бит %s~eMMC HS400: Modus=%d, Bus=%d Bit %s~eMMC HS400: modo=%d, barramento de %d bits %s
transport rdy : chip RTS_N %s (MSR=%02X, junk=%d)~Transport prêt : RTS_N puce %s (MSR=%02X, octets parasites=%d)~Transporte listo: RTS_N del chip %s (MSR=%02X, bytes sobrantes=%d)~Trasporto pronto: RTS_N chip %s (MSR=%02X, byte spuri=%d)~Готовность: RTS_N чипа %s (MSR=%02X, лишних байт=%d)~Transport bereit: Chip RTS_N %s (MSR=%02X, Restbytes=%d)~Transporte pronto: RTS_N do chip %s (MSR=%02X, bytes espúrios=%d)
**Atmosphere**~**Atmosphere**~**Atmosphere**~**Atmosphere**~**Atmosphere**~**Atmosphere**~**Atmosphere**
Verdict : line idle HIGH - shifter alive, host quiet~Verdict : ligne au repos HIGH ; convertisseur de niveau actif, hôte silencieux~Veredicto: línea en reposo HIGH; conversor de nivel activo, host sin transmitir~Verdetto: linea a riposo HIGH; convertitore di livello attivo, host silenzioso~Вердикт: линия в покое HIGH; преобразователь уровней работает, хост молчит~Ergebnis: Leitung in Ruhe HIGH; Pegelwandler aktiv, Host sendet nicht~Veredito: linha em repouso HIGH; conversor de nível ativo, anfitrião sem transmitir
Verdict : PWM0 driving BL_PWM @ duty %d - backlight lit~Verdict : PWM0 pilote BL_PWM, rapport cyclique %d ; rétroéclairage allumé~Veredicto: PWM0 controla BL_PWM, ciclo de trabajo %d; retroiluminación encendida~Verdetto: PWM0 pilota BL_PWM, ciclo utile %d; retroilluminazione accesa~Вердикт: PWM0 управляет BL_PWM, заполнение %d; подсветка включена~Ergebnis: PWM0 steuert BL_PWM, Tastgrad %d; Hintergrundbeleuchtung an~Veredito: PWM0 controla BL_PWM, ciclo de trabalho %d; retroiluminação ligada
Tach pad : CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Broche tachymètre : CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Pin del tacómetro: CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Pin tachimetro: CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Контакт тахометра: CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Tacho-Pin: CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d~Pino do tacómetro: CAM1_PWDN=%04X E_INPUT=%d TRI=%d pull=%d lvl=%d
Tach line : %d transitions, %d low samples in %d ms~Tachymètre : %d transitions, %d échantillons bas en %d ms~Tacómetro: %d transiciones, %d muestras bajas en %d ms~Tachimetro: %d transizioni, %d campioni bassi in %d ms~Тахометр: %d переходов, %d низких отсчётов за %d ms~Tachosignal: %d Wechsel, %d LOW-Abtastungen in %d ms~Tacómetro: %d transições, %d amostras baixas em %d ms
Fan verdict : SPINNING (draws %d mA) but tach unreadable~Ventilateur : EN ROTATION (%d mA consommés), tachymètre illisible~Ventilador: GIRANDO (consume %d mA), tacómetro ilegible~Ventola: IN ROTAZIONE (assorbe %d mA), tachimetro illeggibile~Вентилятор: ВРАЩАЕТСЯ (ток %d mA), тахометр не читается~Lüfter: DREHT (Strom %d mA), Tachosignal nicht lesbar~Ventoinha: A GIRAR (consome %d mA), tacómetro ilegível
idle anchor : internal PULL_UP (line idles HIGH on its own)~Repos : PULL_UP interne (la ligne reste d'elle-même à HIGH)~Reposo: PULL_UP interno (la línea permanece en HIGH por sí sola)~Riposo: PULL_UP interno (la linea resta da sola HIGH)~Покой: внутренний PULL_UP (линия сама остаётся HIGH)~Ruhepegel: interner PULL_UP (Leitung bleibt von selbst HIGH)~Repouso: PULL_UP interno (a linha permanece em HIGH por si)
Major.Minor : %d.%02d~Version majeure.mineure : %d.%02d~Versión mayor.menor: %d.%02d~Versione maggiore.minore: %d.%02d~Версия основная.дополнительная: %d.%02d~Haupt-/Unterversion: %d.%02d~Versão principal.secundária: %d.%02d
No handoff : bring-up incomplete~Aucun transfert : initialisation incomplète~Sin transferencia: inicialización incompleta~Nessun passaggio: inizializzazione incompleta~Нет передачи управления: инициализация не завершена~Keine Übergabe: Initialisierung unvollständig~Sem transferência: inicialização incompleta
X / Y coord : %d / %d~Coordonnées X / Y : %d / %d~Coordenadas X / Y: %d / %d~Coordinate X / Y: %d / %d~Координаты X / Y: %d / %d~X-/Y-Koordinaten: %d / %d~Coordenadas X / Y: %d / %d
PRODINFO GPT : LBA %08X..%08X is not a usable range~PRODINFO GPT : plage LBA %08X..%08X inutilisable~PRODINFO GPT: rango LBA %08X..%08X no utilizable~PRODINFO GPT: intervallo LBA %08X..%08X non utilizzabile~PRODINFO GPT: диапазон LBA %08X..%08X недоступен~PRODINFO GPT: LBA-Bereich %08X..%08X nicht nutzbar~PRODINFO GPT: intervalo LBA %08X..%08X inutilizável
CPU RAM repair never completed~Réparation RAM CPU jamais terminée~La reparación RAM CPU no terminó~Riparazione RAM CPU mai completata~Восстановление RAM CPU не завершено~CPU-RAM-Reparatur nicht abgeschlossen~A reparação RAM CPU não terminou
LCD reset (act-low)~Réinitialisation LCD (active à LOW)~Reinicio LCD (activo en LOW)~Reset LCD (attivo LOW)~Сброс LCD (активен LOW)~LCD-Reset (aktiv bei LOW)~Reinício LCD (ativo em LOW)
long-press = hard reset~Appui long = réinitialisation matérielle~Pulsación larga = reinicio físico~Pressione lunga = reset hardware~Долгое нажатие = аппаратный сброс~Langes Drücken = Hardware-Reset~Pressão longa = reinício físico
no documented flags set~Aucun indicateur documenté activé~Ningún indicador documentado activo~Nessun indicatore documentato attivo~Нет установленных документированных флагов~Keine dokumentierten Flags gesetzt~Nenhum indicador documentado ativo
not charging~Pas de charge en cours~No se está cargando~Non in carica~Не заряжается~Lädt nicht~Não está a carregar
nothing latched~Aucun état mémorisé~Ningún estado registrado~Nessuno stato memorizzato~Нет сохранённых состояний~Keine Zustände gespeichert~Nenhum estado registado
skipped (gibberish, prod.keys not for this device)~Ignoré (données incohérentes ; prod.keys ne correspond pas à cette console)~Omitido (datos incoherentes; prod.keys no corresponde a esta consola)~Saltato (dati incoerenti; prod.keys non appartiene a questa console)~Пропущено (некорректные данные; prod.keys от другой консоли)~Übersprungen (unlesbare Daten; prod.keys gehört zu anderer Konsole)~Ignorado (dados incoerentes; prod.keys não corresponde a esta consola)
version file absent, contents/ exists~Fichier de version absent, dossier contents/ présent~Falta el archivo de versión; existe contents/~File di versione assente; contents/ presente~Нет файла версии; папка contents/ существует~Versionsdatei fehlt; contents/ vorhanden~Ficheiro de versão ausente; contents/ existe

BUCK_STAT : 0x%02X (live status; informational)~BUCK_STAT : 0x%02X (état actuel ; à titre informatif)~BUCK_STAT: 0x%02X (estado actual; informativo)~BUCK_STAT: 0x%02X (stato attuale; informativo)~BUCK_STAT: 0x%02X (текущее состояние; справочно)~BUCK_STAT: 0x%02X (aktueller Status; zur Information)~BUCK_STAT: 0x%02X (estado atual; informativo)
TOPSYS_STAT : 0x%02X (live status; informational)~TOPSYS_STAT : 0x%02X (état actuel ; à titre informatif)~TOPSYS_STAT: 0x%02X (estado actual; informativo)~TOPSYS_STAT: 0x%02X (stato attuale; informativo)~TOPSYS_STAT: 0x%02X (текущее состояние; справочно)~TOPSYS_STAT: 0x%02X (aktueller Status; zur Information)~TOPSYS_STAT: 0x%02X (estado atual; informativo)
PCIE partition: %s (was %s, bit %d = %d)~Partition PCIE : %s (précédemment %s, bit %d = %d)~Partición PCIE: %s (antes %s, bit %d = %d)~Partizione PCIE: %s (prima %s, bit %d = %d)~Раздел PCIE: %s (ранее %s, бит %d = %d)~PCIE-Partition: %s (vorher %s, Bit %d = %d)~Partição PCIE: %s (antes %s, bit %d = %d)
hdr CRC32 : 0x%08X (stored 0x%08X) %s~CRC32 en-tête : 0x%08X (stocké 0x%08X) %s~CRC32 cabecera: 0x%08X (guardado 0x%08X) %s~CRC32 intestazione: 0x%08X (salvato 0x%08X) %s~CRC32 заголовка: 0x%08X (сохранён 0x%08X) %s~Header-CRC32: 0x%08X (gespeichert 0x%08X) %s~CRC32 cabeçalho: 0x%08X (guardado 0x%08X) %s
BIS keys : loaded but invalid (wrong unit's prod.keys?)~Clés BIS : chargées mais invalides (prod.keys d'une autre console ?)~Claves BIS: cargadas pero inválidas (¿prod.keys de otra consola?)~Chiavi BIS: caricate ma non valide (prod.keys di un'altra console?)~Ключи BIS: загружены, но неверны (prod.keys от другой консоли?)~BIS-Schlüssel: geladen, aber ungültig (prod.keys einer anderen Konsole?)~Chaves BIS: carregadas mas inválidas (prod.keys de outra consola?)
hekate : update.bin absent~hekate : update.bin absent~hekate: falta update.bin~hekate: update.bin assente~hekate: update.bin отсутствует~hekate: update.bin fehlt~hekate: update.bin ausente
hekate : update.bin magic mismatch (0x%08X)~hekate : signature de update.bin incorrecte (0x%08X)~hekate: firma incorrecta de update.bin (0x%08X)~hekate: firma update.bin errata (0x%08X)~hekate: неверная сигнатура update.bin (0x%08X)~hekate: ungültige update.bin-Kennung (0x%08X)~hekate: assinatura de update.bin incorreta (0x%08X)
atmosphere : %s (HATS manifest)~atmosphere : %s (manifeste HATS)~atmosphere: %s (manifiesto HATS)~atmosphere: %s (manifesto HATS)~atmosphere: %s (манифест HATS)~atmosphere: %s (HATS-Manifest)~atmosphere: %s (manifesto HATS)
atmosphere : %s (HATS pack)~atmosphere : %s (paquet HATS)~atmosphere: %s (paquete HATS)~atmosphere: %s (pacchetto HATS)~atmosphere: %s (пакет HATS)~atmosphere: %s (HATS-Paket)~atmosphere: %s (pacote HATS)
eMMC init failures : %d~Échecs d'initialisation eMMC : %d~Fallos de inicialización eMMC: %d~Errori di inizializzazione eMMC: %d~Сбои инициализации eMMC: %d~eMMC-Initialisierungsfehler: %d~Falhas de inicialização eMMC: %d
eMMC R/W retries : n/a (BDK counts these against SD)~Nouveaux essais eMMC L/E : indisponibles (comptabilisés avec SD par BDK)~Reintentos eMMC L/E: no disponibles (BDK los cuenta con SD)~Tentativi eMMC L/S: non disponibili (BDK li conta con SD)~Повторы чтения/записи eMMC: недоступны (BDK учитывает их вместе с SD)~eMMC-Lese-/Schreibwiederholungen: n/v (BDK zählt diese zu SD)~Repetições eMMC L/E: indisponíveis (BDK conta-as com SD)
Result : XUSB_PADCTL stuck in reset, refusing to read~Résultat : XUSB_PADCTL bloqué en réinitialisation ; lecture refusée~Resultado: XUSB_PADCTL bloqueado en reinicio; lectura rechazada~Risultato: XUSB_PADCTL bloccato in reset; lettura rifiutata~Результат: XUSB_PADCTL остаётся в сбросе; чтение отклонено~Ergebnis: XUSB_PADCTL bleibt im Reset; Lesen verweigert~Resultado: XUSB_PADCTL bloqueado em reinício; leitura recusada
Chip %d dens : Unknown (MR8 0x%02X)~Densité puce %d : inconnue (MR8 0x%02X)~Densidad chip %d: desconocida (MR8 0x%02X)~Densità chip %d: sconosciuta (MR8 0x%02X)~Плотность чипа %d: неизвестна (MR8 0x%02X)~Dichte Chip %d: unbekannt (MR8 0x%02X)~Densidade chip %d: desconhecida (MR8 0x%02X)

loaded from sd:/switch/prod.keys~Chargé depuis sd:/switch/prod.keys~Cargado desde sd:/switch/prod.keys~Caricato da sd:/switch/prod.keys~Загружено из sd:/switch/prod.keys~Aus sd:/switch/prod.keys geladen~Carregado de sd:/switch/prod.keys
no sd:/switch/prod.keys (run Lockpick first)~sd:/switch/prod.keys absent (exécuter Lockpick d'abord)~Falta sd:/switch/prod.keys (ejecutar Lockpick primero)~Manca sd:/switch/prod.keys (eseguire prima Lockpick)~Нет sd:/switch/prod.keys (сначала запустите Lockpick)~sd:/switch/prod.keys fehlt (zuerst Lockpick ausführen)~Falta sd:/switch/prod.keys (executar primeiro o Lockpick)
Hekate intent: %d (%s)~Intention de démarrage Hekate : %d (%s)~Intención de inicio Hekate: %d (%s)~Intento di avvio Hekate: %d (%s)~Режим запуска Hekate: %d (%s)~Hekate-Startabsicht: %d (%s)~Intenção de arranque Hekate: %d (%s)
Hekate intent: none (normal boot, no override pending)~Intention Hekate : aucune (démarrage normal, aucun remplacement demandé)~Intención Hekate: ninguna (inicio normal, sin cambios pendientes)~Intento Hekate: nessuno (avvio normale, nessuna modifica richiesta)~Режим Hekate: обычный запуск, переопределений нет~Hekate-Startabsicht: keine (normaler Start, keine Vorgabe ausstehend)~Intenção Hekate: nenhuma (arranque normal, sem substituição pendente)
RTC adjusted : %04d-%02d-%02d %02d:%02d:%02d (nyx.ini offset %s%d s%s)~RTC ajustée : %04d-%02d-%02d %02d:%02d:%02d (décalage nyx.ini %s%d s%s)~RTC ajustado: %04d-%02d-%02d %02d:%02d:%02d (desfase nyx.ini %s%d s%s)~RTC corretto: %04d-%02d-%02d %02d:%02d:%02d (scarto nyx.ini %s%d s%s)~RTC с поправкой: %04d-%02d-%02d %02d:%02d:%02d (сдвиг nyx.ini %s%d s%s)~RTC korrigiert: %04d-%02d-%02d %02d:%02d:%02d (nyx.ini-Versatz %s%d s%s)~RTC ajustado: %04d-%02d-%02d %02d:%02d:%02d (desvio nyx.ini %s%d s%s)

(STILL PARKED)~(TOUJOURS EN ATTENTE)~(SIGUE EN ESPERA)~(ANCORA IN ATTESA)~(ВСЁ ЕЩЁ ОЖИДАЕТ)~(WEITERHIN IN WARTEPOSITION)~(AINDA EM ESPERA)
BUSY~OCCUPÉ~OCUPADO~OCCUPATO~ЗАНЯТ~BELEGT~OCUPADO
WRONG~INCORRECT~INCORRECTO~ERRATO~НЕВЕРНО~FALSCH~INCORRETO
(WRONG)~(INCORRECT)~(INCORRECTO)~(ERRATO)~(НЕВЕРНО)~(FALSCH)~(INCORRETO)
APERTURE LIVE~FENÊTRE ACTIVE~VENTANA ACTIVA~FINESTRA ATTIVA~ОКНО АКТИВНО~FENSTER AKTIV~JANELA ATIVA
ATTACHED~CONNECTÉ~CONECTADO~COLLEGATO~ПОДКЛЮЧЕНО~VERBUNDEN~LIGADO
DISABLED~DÉSACTIVÉ~DESACTIVADO~DISATTIVATO~ОТКЛЮЧЕНО~DEAKTIVIERT~DESATIVADO
ENABLED, LOCKED~ACTIVÉ, VERROUILLÉ~ACTIVADO, BLOQUEADO~ATTIVO, BLOCCATO~ВКЛЮЧЕНО, ЗАБЛОКИРОВАНО~AKTIVIERT, GESPERRT~ATIVADO, BLOQUEADO
ENABLED, NO-LOCK~ACTIVÉ, NON VERROUILLÉ~ACTIVADO, SIN BLOQUEO~ATTIVO, NON BLOCCATO~ВКЛЮЧЕНО, НЕ ЗАБЛОКИРОВАНО~AKTIVIERT, NICHT GESPERRT~ATIVADO, SEM BLOQUEIO
FAILED~ÉCHEC~FALLO~ERRORE~СБОЙ~FEHLER~FALHA
INIT FAILED~ÉCHEC INITIALISATION~FALLO DE INICIO~ERRORE INIZIALIZZAZIONE~СБОЙ ИНИЦИАЛИЗАЦИИ~INITIALISIERUNG FEHLGESCHLAGEN~FALHA DE INICIALIZAÇÃO
INPUT-DISABLED~ENTRÉE DÉSACTIVÉE~ENTRADA DESACTIVADA~INGRESSO DISATTIVATO~ВХОД ОТКЛЮЧЁН~EINGANG DEAKTIVIERT~ENTRADA DESATIVADA
INPUTS CLAMPED~ENTRÉES BLOQUÉES~ENTRADAS BLOQUEADAS~INGRESSI BLOCCATI~ВХОДЫ ЗАФИКСИРОВАНЫ~EINGÄNGE FIXIERT~ENTRADAS BLOQUEADAS
LOCKED -> 120 MHz OUT0~VERROUILLÉ -> 120 MHz OUT0~BLOQUEADO -> 120 MHz OUT0~BLOCCATO -> 120 MHz OUT0~ЗАФИКСИРОВАН -> 120 MHz OUT0~EINGERASTET -> 120 MHz OUT0~BLOQUEADO -> 120 MHz OUT0
MISMATCH~INCOHÉRENCE~NO COINCIDE~NON CORRISPONDE~НЕСОВПАДЕНИЕ~ABWEICHUNG~DIVERGÊNCIA
NO LOCK~NON VERROUILLÉ~SIN BLOQUEO~NON BLOCCATO~НЕТ СИНХРОНИЗАЦИИ~NICHT EINGERASTET~SEM BLOQUEIO
PRESSED~APPUYÉ~PULSADO~PREMUTO~НАЖАТО~GEDRÜCKT~PREMIDO
STILL ASSERTED~TOUJOURS ACTIVÉ~SIGUE ACTIVO~ANCORA ATTIVO~ВСЁ ЕЩЁ АКТИВЕН~WEITERHIN AKTIV~AINDA ATIVO
UNEXPECTED DEVICE~PÉRIPHÉRIQUE INATTENDU~DISPOSITIVO INESPERADO~DISPOSITIVO INATTESO~НЕОЖИДАННОЕ УСТРОЙСТВО~UNERWARTETES GERÄT~DISPOSITIVO INESPERADO
UNGATE FAILED~ÉCHEC ACTIVATION HORLOGE~FALLO AL ACTIVAR EL RELOJ~ERRORE ATTIVAZIONE CLOCK~СБОЙ ВКЛЮЧЕНИЯ ТАКТИРОВАНИЯ~TAKTFREIGABE FEHLGESCHLAGEN~FALHA AO ATIVAR O RELÓGIO
emmc_initialize FAILED~ÉCHEC de emmc_initialize~FALLO de emmc_initialize~ERRORE emmc_initialize~СБОЙ emmc_initialize~emmc_initialize FEHLGESCHLAGEN~FALHA emmc_initialize
FAN : %s~VENTILATEUR : %s~VENTILADOR: %s~VENTOLA: %s~ВЕНТИЛЯТОР: %s~LÜFTER: %s~VENTOINHA: %s
FAULT (%s) : 0x%02X (%s)~DÉFAUT (%s) : 0x%02X (%s)~FALLO (%s): 0x%02X (%s)~GUASTO (%s): 0x%02X (%s)~НЕИСПРАВНОСТЬ (%s): 0x%02X (%s)~STÖRUNG (%s): 0x%02X (%s)~FALHA (%s): 0x%02X (%s)
MCR=%02X : TX gated, MSR=%02X~MCR=%02X : TX bloqué, MSR=%02X~MCR=%02X: TX bloqueado, MSR=%02X~MCR=%02X: TX bloccato, MSR=%02X~MCR=%02X: TX заблокирован, MSR=%02X~MCR=%02X: TX gesperrt, MSR=%02X~MCR=%02X: TX bloqueado, MSR=%02X

'''
def normalize(text):
    return re.sub(r'\s*:\s*', ': ', re.sub(r'\s+', ' ', text.strip()))

review={}
lookup={}
for line in ROWS.strip().splitlines():
    if not line.strip(): continue
    row=line.split('~');assert len(row)==7,row
    assert all(FORMATS.findall(row[0])==FORMATS.findall(v) for v in row)
    review[row[0]]=row;lookup[normalize(row[0])]=row
for path in (ROOT/'diagnostics/vendor').glob('*.c'):
    for source,line in literals(path):
        row=lookup.get(normalize(source))
        if row:
            before=source[:len(source)-len(source.lstrip())];after=source[len(source.rstrip()):]
            review[source]=[source]+[before+value+after for value in row[1:]]
for source in ['force open','sense open','FPC force open','FPC sense open','key force open','key sense open']:
    pin='FORCE' if 'force' in source else 'SENSE'
    group='FPC ' if source.startswith('FPC') else ''
    row=[source,f'circuit {group}{pin} ouvert',f'circuito {group}{pin} abierto',f'circuito {group}{pin} aperto',f'обрыв {group}{pin}',f'{group}{pin} unterbrochen',f'circuito {group}{pin} aberto']
    if source.startswith('key'):
        row=[source,f'circuit {pin} des touches ouvert',f'circuito {pin} de teclas abierto',f'circuito {pin} dei tasti aperto',f'обрыв {pin} кнопок',f'{pin} der Tasten unterbrochen',f'circuito {pin} das teclas aberto']
    review[source]=row
for source in ['force-GND short','sense-GND short','force-VCM short','sense-VCM short','force-force short','sense-sense short','force-sense short']:
    pins=source[:-6].upper()
    review[source]=[source,f'court-circuit {pins}',f'cortocircuito {pins}',f'cortocircuito {pins}',f'замыкание {pins}',f'Kurzschluss {pins}',f'curto-circuito {pins}']
for source in ['Sharp LQ055T1SW10','ins','pre','prv','oemid']:
    review[source]=[source]*7
for path in (ROOT/'diagnostics/vendor').glob('*.c'):
    for source,line in literals(path):
        core=source.strip()
        if core.startswith(('plle ', 'PCC3 : fn=X', 'PHY (CPU0)    : plle=', 'hekate       : v')) or core in ['atmosphere   : %s','atmosphere   : %s (%s)','emummc.ini   : %s','[eMMC GPT]', '"atmosphere"']:
            review[source]=[source]*7
        if re.fullmatch(r'\s*(?:plle|pllx|pllu|pllp|xio|prv|oemid)\s*:?[ 0-9A-Fa-f%Xx]+:?\s*', source) or source.strip().startswith('PCC3 : fn=X'):
            review[source]=[source]*7
(ROOT/'localization_probe_review.json').write_text(json.dumps(review,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Curated electrical terms and explanations:',len(review),'exact keys')
