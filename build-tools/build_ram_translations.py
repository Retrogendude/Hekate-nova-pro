"""Authored translations for every user-facing isolated RAM-service string."""
from pathlib import Path
import json
import re
from audit_localization_coverage import literals, technical
WORK=Path(__file__).resolve().parent
ROOT=WORK/'hekate-6.5.4/nyx/nyx_gui'
ROWS=r'''
Power-cycle before trusting memory state after a HW timeout.~Après un délai matériel dépassé, éteindre puis rallumer avant de se fier à la mémoire.~Tras un tiempo de espera de hardware agotado, apaga y enciende antes de confiar en la memoria.~Dopo un timeout hardware, spegnere e riaccendere prima di fidarsi della memoria.~После аппаратного тайм-аута выключите и включите консоль перед проверкой памяти.~Nach Hardware-Zeitüberschreitung vor weiteren Speichertests aus- und einschalten.~Após um tempo limite de hardware, desligue e ligue antes de confiar na memória.
- EMC/DLL status transition timed out~- Délai dépassé pour la transition EMC/DLL~- Tiempo agotado en la transición EMC/DLL~- Timeout della transizione EMC/DLL~- Тайм-аут перехода EMC/DLL~- Zeitüberschreitung beim EMC/DLL-Wechsel~- Tempo limite na transição EMC/DLL
- invalid/corrupt timing table~- Table de temporisation invalide ou corrompue~- Tabla de tiempos no válida o dañada~- Tabella dei tempi non valida o danneggiata~- Таблица таймингов недействительна или повреждена~- Ungültige/beschädigte Timing-Tabelle~- Tabela de temporização inválida ou danificada
- memory PLL did not lock~- La PLL mémoire ne s'est pas verrouillée~- La PLL de memoria no se sincronizó~- Il PLL della memoria non si è agganciato~- PLL памяти не синхронизировалась~- Speicher-PLL hat nicht eingerastet~- A PLL da memória não sincronizou
- unexpected EMC entry clock; no training attempted~- Horloge EMC initiale inattendue ; aucune calibration tentée~- Reloj EMC inicial inesperado; no se intentó el entrenamiento~- Clock EMC iniziale inatteso; calibrazione non tentata~- Неожиданная начальная частота EMC; настройка не выполнялась~- Unerwarteter EMC-Ausgangstakt; kein Training versucht~- Relógio EMC inicial inesperado; treino não iniciado
- unsupported DRAM ID/table~- Identifiant ou table DRAM non pris en charge~- ID o tabla DRAM no compatible~- ID o tabella DRAM non supportati~- ID или таблица DRAM не поддерживается~- DRAM-ID/Tabelle nicht unterstützt~- ID ou tabela DRAM não suportado
+ %d additional failing words~+ %d mots supplémentaires en erreur~+ %d palabras adicionales con errores~+ %d parole aggiuntive errate~+ %d дополнительных ошибочных слов~+ %d weitere fehlerhafte Wörter~+ %d palavras adicionais com erro
1600 MHz training: FAIL~Calibration à 1600 MHz : ÉCHEC~Entrenamiento a 1600 MHz: FALLO~Calibrazione a 1600 MHz: ERRORE~Настройка 1600 MHz: СБОЙ~1600-MHz-Training: FEHLER~Treino a 1600 MHz: FALHA
55 pattern~Motif 55~Patrón 55~Schema 55~Шаблон 55~55-Muster~Padrão 55
AA pattern~Motif AA~Patrón AA~Schema AA~Шаблон AA~AA-Muster~Padrão AA
Address pattern~Motif d'adresse~Patrón de dirección~Schema indirizzo~Адресный шаблон~Adressmuster~Padrão de endereço
Advanced test~Test avancé~Prueba avanzada~Test avanzato~Расширенный тест~Erweiterter Test~Teste avançado
All-one~Tout à 1~Todo a 1~Tutto a 1~Все единицы~Nur Einsen~Tudo em 1
All-zero~Tout à 0~Todo a 0~Tutto a 0~Все нули~Nur Nullen~Tudo em 0
Back~Retour~Volver~Indietro~Назад~Zurück~Voltar
Choose individual RAM patterns to run.~Choisir les motifs RAM à exécuter.~Elige los patrones de RAM que se ejecutarán.~Scegliere gli schemi RAM da eseguire.~Выберите отдельные шаблоны проверки RAM.~Einzelne RAM-Testmuster auswählen.~Escolha os padrões de RAM a executar.
Custom RAM test~Test RAM personnalisé~Prueba RAM personalizada~Test RAM personalizzato~Выборочная проверка RAM~Benutzerdefinierter RAM-Test~Teste RAM personalizado
Custom test~Test personnalisé~Prueba personalizada~Test personalizzato~Выборочный тест~Benutzerdefinierter Test~Teste personalizado
DRAM result : %s~Résultat DRAM : %s~Resultado DRAM: %s~Risultato DRAM: %s~Результат DRAM: %s~DRAM-Ergebnis: %s~Resultado DRAM: %s
DRAM size : %d MiB~Taille DRAM : %d MiB~Tamaño DRAM: %d MiB~Dimensione DRAM: %d MiB~Объём DRAM: %d MiB~DRAM-Größe: %d MiB~Tamanho DRAM: %d MiB
DRAM training failed~Échec de calibration DRAM~Falló el entrenamiento DRAM~Calibrazione DRAM fallita~Ошибка настройки DRAM~DRAM-Training fehlgeschlagen~Falha no treino DRAM
Data-bus preflight~Vérification du bus de données~Comprobación previa del bus de datos~Verifica preliminare del bus dati~Предварительная проверка шины данных~Vorprüfung des Datenbusses~Verificação prévia do barramento de dados
Final result~Résultat final~Resultado final~Risultato finale~Итоговый результат~Endergebnis~Resultado final
Inverse address~Adresse inversée~Dirección inversa~Indirizzo inverso~Инверсный адрес~Invertierte Adresse~Endereço inverso
MTC flags : %08X~Indicateurs MTC : %08X~Indicadores MTC: %08X~Indicatori MTC: %08X~Флаги MTC: %08X~MTC-Flags: %08X~Indicadores MTC: %08X
One full Address pattern write/verify pass.~Une passe complète d'écriture/vérification du motif d'adresse.~Una pasada completa de escritura y verificación del patrón de dirección.~Un passaggio completo di scrittura/verifica dello schema indirizzo.~Полный проход записи и проверки адресного шаблона.~Ein vollständiger Schreib-/Prüfdurchlauf mit Adressmuster.~Uma passagem completa de escrita/verificação do padrão de endereço.
POWER toggles~POWER active ou désactive~POWER activa o desactiva~POWER attiva o disattiva~POWER переключает~POWER schaltet um~POWER ativa ou desativa
POWER: menu VOL+: restart console VOL-: power off~POWER : menu  VOL+ : redémarrer  VOL- : éteindre~POWER: menú  VOL+: reiniciar  VOL-: apagar~POWER: menu  VOL+: riavvia  VOL-: spegni~POWER: меню  VOL+: перезапуск  VOL-: выключение~POWER: Menü  VOL+: Neustart  VOL-: Ausschalten~POWER: menu  VOL+: reiniciar  VOL-: desligar
PRBS + retention~PRBS + rétention~PRBS + retención~PRBS + ritenzione~PRBS + удержание~PRBS + Datenerhalt~PRBS + retenção
Pass %d/%d %s %s errors %d~Passe %d/%d %s %s erreurs %d~Pasada %d/%d %s %s errores %d~Passaggio %d/%d %s %s errori %d~Проход %d/%d %s %s ошибок %d~Durchlauf %d/%d %s %s Fehler %d~Passagem %d/%d %s %s erros %d
Platform : %s~Plateforme : %s~Plataforma: %s~Piattaforma: %s~Платформа: %s~Plattform: %s~Plataforma: %s
Power off~Éteindre~Apagar~Spegni~Выключить~Ausschalten~Desligar
Powering off...~Extinction...~Apagando...~Spegnimento...~Выключение...~Ausschalten...~A desligar...
Protected Hekate return image was modified during training.~L'image protégée de retour Hekate a été modifiée pendant la calibration.~La imagen protegida de retorno a Hekate cambió durante el entrenamiento.~L'immagine protetta di ritorno a Hekate è cambiata durante la calibrazione.~Защищённый образ возврата Hekate изменился во время настройки.~Geschütztes Hekate-Rückkehrabbild wurde beim Training verändert.~A imagem protegida de regresso ao Hekate mudou durante o treino.
Quick test~Test rapide~Prueba rápida~Test rapido~Быстрый тест~Schnelltest~Teste rápido
RAM test menu~Menu du test RAM~Menú de prueba RAM~Menu test RAM~Меню проверки RAM~RAM-Testmenü~Menu de teste RAM
Restart console~Redémarrer la console~Reiniciar consola~Riavvia console~Перезапустить консоль~Konsole neu starten~Reiniciar consola
Restart through the console's normal boot path.~Redémarrer avec le démarrage normal de la console.~Reinicia mediante el arranque normal de la consola.~Riavviare con il normale avvio della console.~Перезапуск обычным способом загрузки консоли.~Neustart über den normalen Startpfad der Konsole.~Reinicie pelo arranque normal da consola.
Restarting console...~Redémarrage de la console...~Reiniciando consola...~Riavvio console...~Перезапуск консоли...~Konsole wird neu gestartet...~A reiniciar a consola...
Return image invalid; refusing to execute corrupted code.~Image de retour invalide : le code corrompu ne sera pas exécuté.~Imagen de retorno no válida; no se ejecutará código dañado.~Immagine di ritorno non valida; codice danneggiato non eseguito.~Образ возврата недействителен; повреждённый код не будет запущен.~Ungültiges Rückkehrabbild; beschädigter Code wird nicht ausgeführt.~Imagem de regresso inválida; o código danificado não será executado.
Run all seven RAM patterns.~Exécuter les sept motifs RAM.~Ejecutar los siete patrones de RAM.~Eseguire tutti i sette schemi RAM.~Выполнить все семь шаблонов RAM.~Alle sieben RAM-Muster ausführen.~Executar os sete padrões de RAM.
Run selected checks~Exécuter la sélection~Ejecutar selección~Esegui selezione~Запустить выбранные проверки~Ausgewählte Tests starten~Executar seleção
SMMU setup : FAIL~Configuration SMMU : ÉCHEC~Configuración SMMU: FALLO~Configurazione SMMU: ERRORE~Настройка SMMU: СБОЙ~SMMU-Einrichtung: FEHLER~Configuração SMMU: FALHA
SMMU window : PASS~Fenêtre SMMU : OK~Ventana SMMU: OK~Finestra SMMU: OK~Окно SMMU: ОК~SMMU-Fenster: OK~Janela SMMU: OK
Select option~Choisir une option~Elegir opción~Scegli opzione~Выбрать пункт~Option auswählen~Escolher opção
Shut down the console cleanly.~Éteindre proprement la console.~Apagar la consola de forma segura.~Spegnere correttamente la console.~Корректно выключить консоль.~Konsole ordnungsgemäß ausschalten.~Desligar corretamente a consola.
Target : 1600 MHz~Cible : 1600 MHz~Objetivo: 1600 MHz~Obiettivo: 1600 MHz~Цель: 1600 MHz~Ziel: 1600 MHz~Objetivo: 1600 MHz
Test stopped; restart the console manually.~Test arrêté : redémarrer la console manuellement.~Prueba detenida; reinicia la consola manualmente.~Test arrestato; riavviare manualmente la console.~Тест остановлен; перезапустите консоль вручную.~Test angehalten; Konsole manuell neu starten.~Teste parado; reinicie a consola manualmente.
Test termine. Retour automatique a Hekate...~Test terminé. Retour automatique à Hekate...~Prueba terminada. Volviendo a Hekate...~Test terminato. Ritorno automatico a Hekate...~Тест завершён. Автоматический возврат в Hekate...~Test beendet. Automatische Rückkehr zu Hekate...~Teste terminado. Regresso automático ao Hekate...
Toggle / select~Activer / sélectionner~Activar / seleccionar~Attiva / seleziona~Переключить / выбрать~Umschalten / auswählen~Ativar / selecionar
Training : PASS~Calibration : OK~Entrenamiento: OK~Calibrazione: OK~Настройка: ОК~Training: OK~Treino: OK
Training : kept from the previous run~Calibration : conservée de l'exécution précédente~Entrenamiento: conservado de la ejecución anterior~Calibrazione: mantenuta dalla prova precedente~Настройка: сохранена с прошлого запуска~Training: vom vorherigen Durchlauf beibehalten~Treino: mantido da execução anterior
Training + data bus~Calibration + bus de données~Entrenamiento + bus de datos~Calibrazione + bus dati~Настройка + шина данных~Training + Datenbus~Treino + barramento de dados
Training DRAM~Calibration de la DRAM~Entrenando DRAM~Calibrazione DRAM~Настройка DRAM~DRAM-Training~Treino da DRAM
Training...~Calibration...~Entrenando...~Calibrazione...~Настройка...~Training...~A treinar...
Unsupported Tegra HIDREV %08X (major %X); no RAM training will run.~Tegra HIDREV %08X (major %X) non pris en charge ; aucune calibration RAM.~Tegra HIDREV %08X (major %X) no compatible; no se entrenará la RAM.~Tegra HIDREV %08X (major %X) non supportato; RAM non calibrata.~Tegra HIDREV %08X (major %X) не поддерживается; RAM не настраивается.~Tegra HIDREV %08X (major %X) nicht unterstützt; kein RAM-Training.~Tegra HIDREV %08X (major %X) não suportado; RAM não será treinada.
Unsupported platform~Plateforme non prise en charge~Plataforma no compatible~Piattaforma non supportata~Платформа не поддерживается~Plattform nicht unterstützt~Plataforma não suportada
VOL+: restart console VOL-: power off~VOL+ : redémarrer  VOL- : éteindre~VOL+: reiniciar  VOL-: apagar~VOL+: riavvia  VOL-: spegni~VOL+: перезапуск  VOL-: выключение~VOL+: Neustart  VOL-: Ausschalten~VOL+: reiniciar  VOL-: desligar
VOL-: Up VOL+: Down PWR: %s~VOL- : haut  VOL+ : bas  PWR : %s~VOL-: arriba  VOL+: abajo  PWR: %s~VOL-: su  VOL+: giù  PWR: %s~VOL-: вверх  VOL+: вниз  PWR: %s~VOL-: hoch  VOL+: runter  PWR: %s~VOL-: cima  VOL+: baixo  PWR: %s
[hekate] Diagnostic RAM~[hekate] Diagnostic RAM~[hekate] Diagnóstico RAM~[hekate] Diagnostica RAM~[hekate] Проверка RAM~[hekate] RAM-Diagnose~[hekate] Diagnóstico RAM
always run first~toujours exécutés en premier~siempre se ejecutan primero~eseguiti sempre per primi~всегда выполняются первыми~werden immer zuerst ausgeführt~são sempre executados primeiro
bad @ %08X%08X exp %08X got %08X (%s)~erreur @ %08X%08X attendu %08X lu %08X (%s)~error @ %08X%08X esperado %08X leído %08X (%s)~errore @ %08X%08X atteso %08X letto %08X (%s)~ошибка @ %08X%08X ожидалось %08X получено %08X (%s)~Fehler @ %08X%08X erwartet %08X gelesen %08X (%s)~erro @ %08X%08X esperado %08X lido %08X (%s)
elapsed : %d s~durée : %d s~duración: %d s~durata: %d s~время: %d s~Dauer: %d s~duração: %d s
error flags : %08X~indicateurs d'erreur : %08X~indicadores de error: %08X~indicatori errore: %08X~флаги ошибок: %08X~Fehlerflags: %08X~indicadores de erro: %08X
errors seen : %d~erreurs observées : %d~errores detectados: %d~errori rilevati: %d~обнаружено ошибок: %d~erkannte Fehler: %d~erros detetados: %d
mode : %s~mode : %s~modo: %s~modalità: %s~режим: %s~Modus: %s~modo: %s
pass %s: %s, +%d errors, %ds~passe %s : %s, +%d erreurs, %ds~pasada %s: %s, +%d errores, %ds~passaggio %s: %s, +%d errori, %ds~проход %s: %s, +%d ошибок, %ds~Durchlauf %s: %s, +%d Fehler, %ds~passagem %s: %s, +%d erros, %ds
passes done : %d / %d~passes terminées : %d / %d~pasadas terminadas: %d / %d~passaggi completati: %d / %d~завершено проходов: %d / %d~Durchläufe fertig: %d / %d~passagens concluídas: %d / %d
pattern checks~contrôles des motifs~comprobaciones de patrones~controlli degli schemi~проверки шаблонов~Musterprüfungen~verificações dos padrões
phase prog : %d / %d ms~progression : %d / %d ms~progreso: %d / %d ms~avanzamento: %d / %d ms~ход этапа: %d / %d ms~Fortschritt: %d / %d ms~progresso: %d / %d ms
phase prog : %d / %d regions (%d MiB)~progression : %d / %d zones (%d MiB)~progreso: %d / %d zonas (%d MiB)~avanzamento: %d / %d aree (%d MiB)~ход этапа: %d / %d областей (%d MiB)~Fortschritt: %d / %d Bereiche (%d MiB)~progresso: %d / %d áreas (%d MiB)
platform : %s~plateforme : %s~plataforma: %s~piattaforma: %s~платформа: %s~Plattform: %s~plataforma: %s
quick~rapide~rápido~rapido~быстрый~schnell~rápido
advanced~avancé~avanzado~avanzato~расширенный~erweitert~avançado
custom~personnalisé~personalizado~personalizzato~выборочный~benutzerdefiniert~personalizado
retention~rétention~retención~ritenzione~удержание~Datenerhalt~retenção
scope/pass : %d / %d regions (%d MiB)~zones/passe : %d / %d zones (%d MiB)~ámbito/pasada: %d / %d zonas (%d MiB)~ambito/passaggio: %d / %d aree (%d MiB)~область/проход: %d / %d областей (%d MiB)~Bereich/Durchlauf: %d / %d Bereiche (%d MiB)~âmbito/passagem: %d / %d áreas (%d MiB)
stopped on : pass %d/%d, %s, %s~arrêt sur : passe %d/%d, %s, %s~detenido en: pasada %d/%d, %s, %s~arresto su: passaggio %d/%d, %s, %s~остановка: проход %d/%d, %s, %s~angehalten bei: Durchlauf %d/%d, %s, %s~parou em: passagem %d/%d, %s, %s
training mask : %08X~masque de calibration : %08X~máscara de entrenamiento: %08X~maschera calibrazione: %08X~маска настройки: %08X~Trainingsmaske: %08X~máscara de treino: %08X
verify~vérification~verificación~verifica~проверка~Prüfung~verificação
walking data bits: %s~test des bits de données : %s~prueba de bits de datos: %s~test dei bit dati: %s~проверка битов данных: %s~Datenbit-Test: %s~teste de bits de dados: %s
write~écriture~escritura~scrittura~запись~Schreiben~escrita
'''
rows={}
for line in ROWS.strip().splitlines():
    parts=line.split('~'); assert len(parts)==7,parts
    rows[parts[0].casefold()]=parts
extra={}
missing=[]
for raw,line in literals(WORK/'HWTest-RCM-0.15/ramtest/source/ramtest_main.c'):
    key=re.sub(r'\s+',' ',raw.strip())
    if not key: continue
    row=rows.get(key.casefold())
    if row:
        leading=raw[:len(raw)-len(raw.lstrip())]
        trailing=raw[len(raw.rstrip()):]
        values=[raw]+[leading+text+trailing for text in row[1:]]
        if key.startswith('Test termine.'):
            values[0]='Test finished. Returning automatically to Hekate...'
        extra[raw]=values
    elif raw.startswith('[ram] '):
        # Structured UART telemetry keys remain stable for external readers.
        extra[raw]=[raw]*7
for key,row in rows.items():
    extra.setdefault(row[0],row)
(ROOT/'localization_ram_extra.json').write_text(json.dumps(extra,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Authored RAM translations:',len(extra),'exact keys')
