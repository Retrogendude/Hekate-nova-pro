from pathlib import Path
import json, re
root = Path(__file__).resolve().parent / 'hekate-nova-pro-v19-tv/nyx/nyx_gui'
phrases = {}
def add(en, fr, es, it, ru, de, pt): phrases[en] = [en, fr, es, it, ru, de, pt]
add('TV / Dock','TV / Dock','TV / Base','TV / Dock','ТВ / Док','TV / Dock','TV / Base')
add('USB-C TV / Dock','TV USB-C / Dock','TV USB-C / Base','TV USB-C / Dock','ТВ USB-C / Док','USB-C TV / Dock','TV USB-C / Base')
add('Start TV','Activer la TV','Activar TV','Attiva TV','Включить ТВ','TV starten','Ativar TV')
add('Stop TV','Arrêter la TV','Detener TV','Ferma TV','Выключить ТВ','TV stoppen','Parar TV')
add('TV controls','Commandes TV','Controles de TV','Comandi TV','Управление ТВ','TV-Steuerung','Controlos de TV')
add('Connect the console directly to the official dock, then start TV output.',
    'Insérez la console directement dans le dock officiel, puis activez la sortie TV.',
    'Coloca la consola directamente en la base oficial y activa la salida de TV.',
    'Inserisci la console direttamente nel dock ufficiale, poi attiva l’uscita TV.',
    'Вставьте консоль прямо в официальный док, затем включите вывод на ТВ.',
    'Die Konsole direkt in das offizielle Dock einsetzen und die TV-Ausgabe starten.',
    'Coloque a consola diretamente na base oficial e ative a saída de TV.')
add('TV output stays active when you return to the menus. VOL- stops it. Restart it manually after undocking.',
    'La sortie TV reste active dans les menus. VOL- l’arrête. Après retrait du dock, réactivez-la manuellement.',
    'La salida de TV sigue activa en los menús. VOL- la detiene. Tras sacar la consola, actívala de nuevo.',
    'L’uscita TV resta attiva nei menu. VOL- la arresta. Dopo aver rimosso la console, riattivala manualmente.',
    'Вывод на ТВ остаётся включённым в меню. VOL- отключает его. После извлечения из дока включите его вручную.',
    'Die TV-Ausgabe bleibt in den Menüs aktiv. VOL- stoppt sie. Nach dem Entnehmen manuell neu starten.',
    'A saída de TV continua ativa nos menus. VOL- desliga-a. Após retirar a consola, ative-a novamente.')
add('TV output stopped.','Sortie TV arrêtée.','Salida de TV detenida.','Uscita TV arrestata.','Вывод на ТВ остановлен.','TV-Ausgabe gestoppt.','Saída de TV parada.')
add('TV disconnected. Output stopped. Start it again after docking.',
    'TV déconnectée. Sortie arrêtée. Réactivez-la après remise sur le dock.',
    'TV desconectada. Salida detenida. Actívala de nuevo tras colocar la consola.',
    'TV scollegata. Uscita arrestata. Riattivala dopo aver inserito la console nel dock.',
    'ТВ отключён. Вывод остановлен. Включите его снова после установки в док.',
    'TV getrennt. Ausgabe gestoppt. Nach dem Andocken erneut starten.',
    'TV desligada. Saída parada. Ative-a de novo após colocar a consola na base.')
add('Dock negotiation timed out. Check power and HDMI, then start again.',
    'Connexion au dock trop longue. Vérifiez l’alimentation et le HDMI, puis réessayez.',
    'Se agotó el tiempo de conexión. Comprueba alimentación y HDMI, e inténtalo de nuevo.',
    'Tempo di connessione scaduto. Verifica alimentazione e HDMI, poi riprova.',
    'Время подключения истекло. Проверьте питание и HDMI, затем повторите.',
    'Zeitlimit beim Andocken. Strom und HDMI prüfen, dann erneut starten.',
    'Tempo de ligação esgotado. Verifique alimentação e HDMI e tente novamente.')
add('Stage: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Étape : %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Etapa: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Fase: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Этап: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Schritt: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s',
    'Etapa: %s\nSTATUS1 %04X  STATUS2 %04X  DP %04X\n\n%s')
add('Failed at %s\n%s','Échec à l’étape %s\n%s','Error en %s\n%s','Errore in %s\n%s','Ошибка на этапе %s\n%s','Fehler bei %s\n%s','Falha em %s\n%s')
add('No TV signal after negotiation. Check the dock and HDMI connection.',
    'Aucun signal TV après connexion. Vérifiez le dock et le câble HDMI.',
    'Sin señal de TV tras conectar. Comprueba la base y la conexión HDMI.',
    'Nessun segnale TV dopo la connessione. Verifica dock e HDMI.',
    'Нет сигнала ТВ после подключения. Проверьте док и HDMI.',
    'Kein TV-Signal nach dem Andocken. Dock und HDMI-Verbindung prüfen.',
    'Sem sinal de TV após ligar. Verifique a base e a ligação HDMI.')
add('AUX/DPCD failed (%d): %s','Échec AUX/DPCD (%d) : %s','Error AUX/DPCD (%d): %s','Errore AUX/DPCD (%d): %s','Ошибка AUX/DPCD (%d): %s','AUX/DPCD-Fehler (%d): %s','Falha AUX/DPCD (%d): %s')
add('1280x720 / 60 Hz: %s\nTV: %s %s\nLink: %d lanes, %s\n%s\n\nCheck the image on your TV. Close this window to use Nova.\nStop TV or VOL- ends TV output.',
    '1280x720 / 60 Hz : %s\nTV : %s %s\nLien : %d voies, %s\n%s\n\nVérifiez l’image sur la TV. Fermez cette fenêtre pour utiliser Nova.\nArrêter la TV ou VOL- coupe la sortie.',
    '1280x720 / 60 Hz: %s\nTV: %s %s\nEnlace: %d vías, %s\n%s\n\nComprueba la imagen en la TV. Cierra esta ventana para usar Nova.\nDetener TV o VOL- desactiva la salida.',
    '1280x720 / 60 Hz: %s\nTV: %s %s\nCollegamento: %d linee, %s\n%s\n\nControlla l’immagine sulla TV. Chiudi la finestra per usare Nova.\nFerma TV o VOL- interrompe l’uscita.',
    '1280x720 / 60 Гц: %s\nТВ: %s %s\nЛиния: %d каналов, %s\n%s\n\nПроверьте изображение на ТВ. Закройте окно для работы с Nova.\nКнопка выключения ТВ или VOL- отключает вывод.',
    '1280x720 / 60 Hz: %s\nTV: %s %s\nVerbindung: %d Lanes, %s\n%s\n\nBild am TV prüfen. Fenster schließen, um Nova zu verwenden.\nTV stoppen oder VOL- beendet die Ausgabe.',
    '1280x720 / 60 Hz: %s\nTV: %s %s\nLigação: %d vias, %s\n%s\n\nVerifique a imagem na TV. Feche esta janela para usar o Nova.\nParar TV ou VOL- desliga a saída.')
add('Configured - check TV','Configurée — vérifier la TV','Configurada — comprobar TV','Configurata — controlla la TV','Настроен — проверьте ТВ','Konfiguriert — TV prüfen','Configurada — verificar TV')
add('TV service could not start. Close this window and try again.',
    'Le service TV n’a pas pu démarrer. Fermez cette fenêtre et réessayez.',
    'No se pudo iniciar el servicio de TV. Cierra esta ventana y reinténtalo.',
    'Impossibile avviare il servizio TV. Chiudi la finestra e riprova.',
    'Не удалось запустить службу ТВ. Закройте окно и повторите.',
    'TV-Dienst konnte nicht starten. Fenster schließen und erneut versuchen.',
    'Não foi possível iniciar o serviço de TV. Feche a janela e tente novamente.')
add('Switch Lite does not support TV output.','La Switch Lite ne prend pas en charge la sortie TV.','Switch Lite no admite salida de TV.','Switch Lite non supporta l’uscita TV.','Switch Lite не поддерживает вывод на ТВ.','Switch Lite unterstützt keine TV-Ausgabe.','A Switch Lite não suporta saída de TV.')
add('BM92T36 did not respond. No automatic reset was performed.',
    'Le BM92T36 n’a pas répondu. Aucun redémarrage automatique du contrôleur n’a été effectué.',
    'El BM92T36 no respondió. No se ha reiniciado el controlador automáticamente.',
    'BM92T36 non risponde. Nessun ripristino automatico del controller eseguito.',
    'BM92T36 не ответил. Автоматический сброс контроллера не выполнялся.',
    'BM92T36 antwortet nicht. Kein automatischer Controller-Reset durchgeführt.',
    'O BM92T36 não respondeu. Não foi efetuada reposição automática do controlador.')
add('Video power state could not be read. TV start cancelled.',
    'Impossible de lire l’état des alimentations vidéo. Activation TV annulée.',
    'No se pudo leer la alimentación de vídeo. Activación cancelada.',
    'Impossibile leggere lo stato dell’alimentazione video. Avvio TV annullato.',
    'Не удалось прочитать состояние питания видео. Запуск ТВ отменён.',
    'Video-Stromstatus konnte nicht gelesen werden. TV-Start abgebrochen.',
    'Não foi possível ler o estado da alimentação de vídeo. Ativação cancelada.')
add('Not enough memory for TV output.','Mémoire insuffisante pour la sortie TV.','Memoria insuficiente para la salida de TV.','Memoria insufficiente per l’uscita TV.','Недостаточно памяти для вывода на ТВ.','Nicht genug Speicher für die TV-Ausgabe.','Memória insuficiente para a saída de TV.')
add('Connecting to the dock... Stop TV or VOL- cancels.',
    'Connexion au dock… Arrêter la TV ou VOL- pour annuler.',
    'Conectando a la base… Detener TV o VOL- para cancelar.',
    'Connessione al dock… Ferma TV o VOL- per annullare.',
    'Подключение к доку… Выключить ТВ или VOL- для отмены.',
    'Verbindung zum Dock… TV stoppen oder VOL- zum Abbrechen.',
    'A ligar à base… Parar TV ou VOL- para cancelar.')
add('TV output is off. Start TV connects to the dock.',
    'La sortie TV est désactivée. Activer la TV lance la connexion au dock.',
    'La salida de TV está desactivada. Activar TV conecta a la base.',
    'L’uscita TV è disattivata. Attiva TV avvia la connessione al dock.',
    'Вывод на ТВ выключен. Включить ТВ запускает подключение к доку.',
    'TV-Ausgabe ist aus. TV starten verbindet mit dem Dock.',
    'A saída de TV está desligada. Ativar TV inicia a ligação à base.')
add('TV stopped. Report: bootloader/tv-test/last-run.txt',
    'TV arrêtée. Rapport : bootloader/tv-test/last-run.txt',
    'TV detenida. Informe: bootloader/tv-test/last-run.txt',
    'TV arrestata. Rapporto: bootloader/tv-test/last-run.txt',
    'ТВ остановлен. Отчёт: bootloader/tv-test/last-run.txt',
    'TV gestoppt. Bericht: bootloader/tv-test/last-run.txt',
    'TV parada. Relatório: bootloader/tv-test/last-run.txt')
add('TV stopped. Report could not be saved to SD.',
    'TV arrêtée. Impossible d’enregistrer le rapport sur la SD.',
    'TV detenida. No se pudo guardar el informe en la SD.',
    'TV arrestata. Impossibile salvare il rapporto sulla SD.',
    'ТВ остановлен. Не удалось сохранить отчёт на SD.',
    'TV gestoppt. Bericht konnte nicht auf SD gespeichert werden.',
    'TV parada. Não foi possível guardar o relatório no SD.')
add('Use the console directly in the official dock.\nConnect dock power and HDMI, then select Start TV.\n\nYou can start outside the dock, then dock within 45 seconds.\nClose this window to keep using Nova on your TV.\nVOL- stops TV output. The console screen stays on.',
    'Insérez la console directement dans le dock officiel.\nBranchez son alimentation et le HDMI, puis activez la TV.\n\nVous pouvez démarrer hors du dock, puis l’insérer sous 45 secondes.\nFermez cette fenêtre pour utiliser Nova sur votre TV.\nVOL- coupe la sortie TV. L’écran de la console reste allumé.',
    'Coloca la consola directamente en la base oficial.\nConecta alimentación y HDMI, y activa la TV.\n\nPuedes iniciar fuera de la base e insertar la consola antes de 45 segundos.\nCierra esta ventana para usar Nova en la TV.\nVOL- detiene la salida. La pantalla de la consola sigue encendida.',
    'Inserisci la console direttamente nel dock ufficiale.\nCollega alimentazione e HDMI, poi attiva la TV.\n\nPuoi avviare fuori dal dock e inserire la console entro 45 secondi.\nChiudi questa finestra per usare Nova sulla TV.\nVOL- interrompe l’uscita. Lo schermo della console resta acceso.',
    'Вставьте консоль прямо в официальный док.\nПодключите питание и HDMI, затем включите ТВ.\n\nМожно начать вне дока и вставить консоль в течение 45 секунд.\nЗакройте это окно для работы с Nova на ТВ.\nVOL- отключает вывод. Экран консоли остаётся включённым.',
    'Die Konsole direkt in das offizielle Dock einsetzen.\nStrom und HDMI anschließen, dann TV starten.\n\nStart außerhalb des Docks möglich: innerhalb von 45 Sekunden andocken.\nFenster schließen, um Nova am TV zu verwenden.\nVOL- stoppt die TV-Ausgabe. Der Konsolenbildschirm bleibt an.',
    'Coloque a consola diretamente na base oficial.\nLigue alimentação e HDMI e ative a TV.\n\nPode iniciar fora da base e colocar a consola em até 45 segundos.\nFeche esta janela para usar o Nova na TV.\nVOL- desliga a saída. O ecrã da consola continua ligado.')

catalog = root / 'localization_catalog.inc'
lines = {}
for line in catalog.read_text(encoding='utf-8').splitlines():
    if line.startswith('    {"'):
        key = json.JSONDecoder().raw_decode(line.strip()[1:])[0]
        lines[key] = line
for key, values in phrases.items():
    assert len(values) == 7
    lines[key] = '    {' + json.dumps(key, ensure_ascii=False) + ', {' + ', '.join(json.dumps(v, ensure_ascii=False) for v in values) + '}},'
keys = sorted(lines)
patterns = [i for i,key in enumerate(keys) if '%' in key and len(re.findall(r'[A-Za-z]',key)) >= 5]
first = [keys[i].encode('utf-8')[0] for i in patterns]
starts = [sum(ch < code for ch in first) for code in range(257)]
tail = 'static const uint16_t locale_pattern_rows[] = {' + ', '.join(map(str,patterns)) + '};\n'
tail += 'static const uint16_t locale_pattern_start[257] = {' + ', '.join(map(str,starts)) + '};\n'
catalog.write_text('/* Nova catalog, including TV v19. en fr es it ru de pt. */\nstatic const locale_entry locale_catalog[] = {\n' + '\n'.join(lines[key] for key in keys) + '\n};\n' + tail, encoding='utf-8')
data = json.loads((root / 'localization_catalog.json').read_text(encoding='utf-8'))
data.update(phrases)
(root / 'localization_catalog.json').write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')
(root / 'localization_tv_v19.json').write_text(json.dumps(phrases, ensure_ascii=False, indent=2), encoding='utf-8')
print(f'PASS: {len(phrases)} TV phrases in seven languages.')
