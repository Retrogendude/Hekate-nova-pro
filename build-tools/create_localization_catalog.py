"""Curated Nyx interface catalog. Ordering: en, fr, es, it, ru, de, pt.

Hardware identifiers, paths and user-supplied boot entries remain intact.
Authored UI translations take precedence over the optional, offline translated
probe commentary. Reviewed electrical explanations take final precedence.
"""
from pathlib import Path
import json
import re
import unicodedata

ROOT = Path(__file__).resolve().parent / "hekate-6.5.4/nyx/nyx_gui"
ROWS = r"""
Home~Accueil~Inicio~Home~Главная~Startseite~Início
Tools~Outils~Herramientas~Strumenti~Утилиты~Werkzeuge~Ferramentas
Console Info~Infos console~Info consola~Info console~О консоли~Konsoleninfo~Info consola
Options~Options~Opciones~Opzioni~Опции~Optionen~Opções
Launch~Démarrer~Iniciar~Avvia~Запуск~Starten~Iniciar
Start~Démarrer~Iniciar~Avvia~Начать~Starten~Iniciar
More Configurations~Autres configurations~Más configuraciones~Altre configurazioni~Другие конфигурации~Weitere Konfigurationen~Mais configurações
More Configs~Autres profils~Más perfiles~Altri profili~Другие профили~Weitere Profile~Mais perfis
Configurations~Configurations~Configuraciones~Configurazioni~Конфигурации~Konfigurationen~Configurações
Payloads~Payloads~Payloads~Payload~Payload~Payloads~Payloads
Diagnostics~Diagnostic~Diagnóstico~Diagnostica~Тесты~Diagnose~Diagnóstico
Nyx Settings~Réglages Nyx~Ajustes de Nyx~Impostazioni Nyx~Настройки Nyx~Nyx-Einstellungen~Definições do Nyx
Close~Fermer~Cerrar~Chiudi~Закрыть~Schließen~Fechar
OK~OK~Aceptar~OK~ОК~OK~OK
Cancel~Annuler~Cancelar~Annulla~Отмена~Abbrechen~Cancelar
Done~Terminé~Listo~Fine~Готово~Fertig~Concluído
Save~Enregistrer~Guardar~Salva~Сохранить~Speichern~Guardar
Save Options~Enregistrer~Guardar opciones~Salva opzioni~Сохранить~Optionen speichern~Guardar opções
Save & Reload~Enregistrer et recharger~Guardar y recargar~Salva e ricarica~Сохранить и перезагрузить~Speichern und neu laden~Guardar e recarregar
Save & reload~Enregistrer et recharger~Guardar y recargar~Salva e ricarica~Сохранить и перезагрузить~Speichern und neu laden~Guardar e recarregar
Reload~Recharger~Recargar~Ricarica~Перезагрузить~Neu laden~Recarregar
Do not reload~Ne pas recharger~No recargar~Non ricaricare~Не перезагружать~Nicht neu laden~Não recarregar
Reboot~Redémarrer~Reiniciar~Riavvia~Перезагрузка~Neustarten~Reiniciar
Reboot (RCM)~Redémarrer (RCM)~Reiniciar (RCM)~Riavvia (RCM)~Перезагрузка (RCM)~Neustarten (RCM)~Reiniciar (RCM)
Power Off~Éteindre~Apagar~Spegni~Выключить~Ausschalten~Desligar
Open~Ouvrir~Abrir~Apri~Открыть~Öffnen~Abrir
Continue~Continuer~Continuar~Continua~Продолжить~Fortfahren~Continuar
Previous~Précédent~Anterior~Precedente~Назад~Zurück~Anterior
Next~Suivant~Siguiente~Successivo~Далее~Weiter~Seguinte
Next Step~Étape suivante~Paso siguiente~Passo successivo~Следующий шаг~Nächster Schritt~Passo seguinte
Yes~Oui~Sí~Sì~Да~Ja~Sim
No~Non~No~No~Нет~Nein~Não
Enable~Activer~Activar~Attiva~Включить~Aktivieren~Ativar
Disable~Désactiver~Desactivar~Disattiva~Отключить~Deaktivieren~Desativar
Enabled~Activé~Activado~Attivo~Включено~Aktiviert~Ativado
Disabled~Désactivé~Desactivado~Disattivato~Отключено~Deaktiviert~Desativado
On~Activé~Activado~Attivo~Вкл.~Ein~Ligado
Off~Désactivé~Desactivado~Disattivato~Выкл.~Aus~Desligado
Normal~Normal~Normal~Normale~Норма~Normal~Normal
Auto~Auto~Auto~Auto~Авто~Auto~Auto
Unknown~Inconnu~Desconocido~Sconosciuto~Неизвестно~Unbekannt~Desconhecido
Error~Erreur~Error~Errore~Ошибка~Fehler~Erro
Failed~Échec~Fallo~Fallito~Сбой~Fehlgeschlagen~Falhou
Pass~Réussi~Correcto~Superato~Пройдено~Bestanden~Aprovado
Please wait...~Veuillez patienter...~Espera...~Attendere...~Подождите...~Bitte warten...~Aguarde...
Done!~Terminé !~¡Listo!~Fatto!~Готово!~Fertig!~Concluído!
Do you want to continue?~Voulez-vous continuer ?~¿Quieres continuar?~Vuoi continuare?~Продолжить?~Möchten Sie fortfahren?~Deseja continuar?
You changed the configuration!~La configuration a été modifiée !~¡Has cambiado la configuración!~Hai modificato la configurazione!~Конфигурация изменена!~Die Konfiguration wurde geändert!~A configuração foi alterada!
Do you want to save it?~Voulez-vous l'enregistrer ?~¿Quieres guardarla?~Vuoi salvarla?~Сохранить изменения?~Möchten Sie sie speichern?~Deseja guardá-la?
Select a payload to launch:~Choisissez un payload à lancer :~Elige un payload para iniciar:~Scegli un payload da avviare:~Выберите payload для запуска:~Payload zum Starten auswählen:~Escolha um payload para iniciar:
hekate - Launch~hekate - Démarrage~hekate - Inicio~hekate - Avvio~hekate - Запуск~hekate - Start~hekate - Iniciar
Choose a system  >~Choisir un système  >~Elegir un sistema  >~Scegli un sistema  >~Выбрать систему  >~System auswählen  >~Escolher um sistema  >
Choose the system to start on your console.~Choisir le système à lancer sur la console.~Elige el sistema que iniciar en la consola.~Scegli il sistema da avviare sulla console.~Выберите систему для запуска на консоли.~System zum Starten auf der Konsole auswählen.~Escolha o sistema a iniciar na consola.
View other boot profiles.~Consulter les autres profils de démarrage.~Consulta los demás perfiles de inicio.~Consulta gli altri profili di avvio.~Просмотр других профилей загрузки.~Weitere Startprofile anzeigen.~Consultar os outros perfis de arranque.
Launch a tool from the microSD card.~Lancer un outil depuis la carte microSD.~Inicia una herramienta desde la tarjeta microSD.~Avvia uno strumento dalla scheda microSD.~Запуск инструмента с карты microSD.~Werkzeug von der microSD-Karte starten.~Iniciar uma ferramenta do cartão microSD.
Manage the system copy on the microSD card.~Gérer la copie du système sur la carte microSD.~Gestiona la copia del sistema en la tarjeta microSD.~Gestisci la copia del sistema sulla scheda microSD.~Управление копией системы на карте microSD.~Systemkopie auf der microSD-Karte verwalten.~Gerir a cópia do sistema no cartão microSD.
Start, manage and check your console.~Démarrer, gérer et vérifier la console.~Inicia, gestiona y comprueba la consola.~Avvia, gestisci e controlla la console.~Запуск, управление и проверка консоли.~Konsole starten, verwalten und prüfen.~Iniciar, gerir e verificar a consola.
Language~Langue~Idioma~Lingua~Язык~Sprache~Idioma
Choose the interface language~Choisir la langue de l'interface~Elige el idioma de la interfaz~Scegli la lingua dell'interfaccia~Выберите язык интерфейса~Sprache der Oberfläche auswählen~Escolher o idioma da interface
Layout~Disposition~Diseño~Disposizione~Макет~Anordnung~Disposição
Home layout~Disposition de l'accueil~Diseño de inicio~Disposizione della home~Макет главной страницы~Anordnung der Startseite~Disposição do início
Choose layout~Choisir la disposition~Elegir diseño~Scegli la disposizione~Выбрать макет~Anordnung auswählen~Escolher disposição
Original~Original~Original~Originale~Исходный~Original~Original
Tiles~Tuiles~Mosaicos~Riquadri~Плитки~Kacheln~Mosaicos
Carousel~Carrousel~Carrusel~Carosello~Карусель~Karussell~Carrossel
Color Theme~Thème de couleurs~Tema de colores~Tema colori~Цветовая тема~Farbschema~Tema de cores
Nyx Color Theme~Thème de couleurs Nyx~Tema de colores de Nyx~Tema colori Nyx~Цветовая тема Nyx~Nyx-Farbschema~Tema de cores do Nyx
Accent color:~Couleur d'accent :~Color de acento:~Colore principale:~Цвет акцента:~Akzentfarbe:~Cor de destaque:
Theme color:~Couleur du thème :~Color del tema:~Colore del tema:~Цвет темы:~Themenfarbe:~Cor do tema:
Grey~Gris~Gris~Grigio~Серый~Grau~Cinzento
Black~Noir~Negro~Nero~Чёрный~Schwarz~Preto
Custom Color~Couleur personnalisée~Color personalizado~Colore personalizzato~Свой цвет~Eigene Farbe~Cor personalizada
Accent sample:~Aperçu de l'accent :~Muestra del acento:~Anteprima colore:~Образец акцента:~Akzentvorschau:~Amostra do destaque:
Home Screen~Écran d'accueil~Pantalla de inicio~Schermata iniziale~Начальный экран~Startbildschirm~Ecrã inicial
Main menu~Menu principal~Menú principal~Menu principale~Главное меню~Hauptmenü~Menu principal
All Configs~Tous les profils~Todos los perfiles~Tutti i profili~Все профили~Alle Profile~Todos os perfis
Select what screen to show on Nyx boot.~Choisir l'écran affiché au démarrage de Nyx.~Elige la pantalla que aparece al iniciar Nyx.~Scegli la schermata all'avvio di Nyx.~Выберите экран при запуске Nyx.~Bildschirm beim Start von Nyx auswählen.~Escolher o ecrã mostrado ao iniciar o Nyx.
Extended Boot Entries~Entrées de démarrage étendues~Entradas de inicio ampliadas~Voci di avvio estese~Расширенные записи загрузки~Erweiterte Starteinträge~Entradas de arranque alargadas
Auto Boot~Démarrage automatique~Inicio automático~Avvio automatico~Автозагрузка~Automatischer Start~Arranque automático
Main configurations~Configurations principales~Configuraciones principales~Configurazioni principali~Основные конфигурации~Hauptkonfigurationen~Configurações principais
Ini folder configurations~Configurations du dossier ini~Configuraciones de la carpeta ini~Configurazioni della cartella ini~Конфигурации из папки ini~Konfigurationen im ini-Ordner~Configurações da pasta ini
Dump Joy-Con BT~Exporter le Bluetooth Joy-Con~Exportar Bluetooth de Joy-Con~Esporta Bluetooth Joy-Con~Экспорт Bluetooth Joy-Con~Joy-Con-Bluetooth exportieren~Exportar Bluetooth Joy-Con
Data Verification~Vérification des données~Verificación de datos~Verifica dati~Проверка данных~Datenprüfung~Verificação de dados
Off (Fastest)~Désactivée (très rapide)~Desactivada (más rápido)~Disattivata (più veloce)~Выкл. (быстрее всего)~Aus (am schnellsten)~Desativada (mais rápido)
Sparse (Fast)~Partielle (rapide)~Parcial (rápida)~Parziale (veloce)~Выборочно (быстро)~Stichproben (schnell)~Parcial (rápida)
Full (Slow)~Complète (lente)~Completa (lenta)~Completa (lenta)~Полная (медленно)~Vollständig (langsam)~Completa (lenta)
Full (Hashes)~Complète (empreintes)~Completa (hashes)~Completa (hash)~Полная (хеши)~Vollständig (Hashes)~Completa (hashes)
Clock (Offset)~Horloge (décalage)~Reloj (desfase)~Orologio (scarto)~Часы (смещение)~Uhr (Versatz)~Relógio (desvio)
Change clock offset manually.~Ajuster le décalage de l'horloge.~Ajusta el desfase del reloj.~Regola lo scarto dell'orologio.~Настроить смещение часов вручную.~Uhrversatz manuell ändern.~Ajustar o desvio do relógio.
Used in all file operations and menu.~Utilisé pour les fichiers et les menus.~Se usa en archivos y menús.~Usato per i file e i menu.~Используется для файлов и меню.~Wird für Dateien und Menüs verwendet.~Usado nos ficheiros e menus.
This doesn't alter the actual HW clock!~Cela ne modifie pas l'horloge matérielle !~¡No cambia el reloj del hardware!~Non modifica l'orologio hardware!~Аппаратные часы не изменяются!~Die Hardware-Uhr wird nicht verändert!~Não altera o relógio do hardware!
Auto Daylight Saving Time~Heure d'été automatique~Horario de verano automático~Ora legale automatica~Автопереход на летнее время~Automatische Sommerzeit~Hora de verão automática
Boot Time Delay~Délai de démarrage~Retardo de inicio~Ritardo di avvio~Задержка загрузки~Startverzögerung~Atraso de arranque
No bootlogo~Sans logo de démarrage~Sin logo de inicio~Senza logo di avvio~Без логотипа загрузки~Ohne Startlogo~Sem logótipo de arranque
1 second~1 seconde~1 segundo~1 secondo~1 секунда~1 Sekunde~1 segundo
2 seconds~2 secondes~2 segundos~2 secondi~2 секунды~2 Sekunden~2 segundos
3 seconds~3 secondes~3 segundos~3 secondi~3 секунды~3 Sekunden~3 segundos
4 seconds~4 secondes~4 segundos~4 secondi~4 секунды~4 Sekunden~4 segundos
5 seconds~5 secondes~5 segundos~5 secondi~5 секунд~5 Sekunden~5 segundos
6 seconds~6 secondes~6 segundos~6 secondi~6 секунд~6 Sekunden~6 segundos
Auto NoGC~NoGC automatique~NoGC automático~NoGC automatico~Авто NoGC~Automatisches NoGC~NoGC automático
Auto HOS Power Off~Extinction HOS automatique~Apagado automático de HOS~Spegnimento HOS automatico~Автовыключение HOS~HOS automatisch ausschalten~Desligar HOS automaticamente
Backlight~Rétroéclairage~Retroiluminación~Retroilluminazione~Подсветка~Hintergrundbeleuchtung~Retroiluminação
Set backlight brightness.~Régler la luminosité du rétroéclairage.~Ajusta el brillo de la pantalla.~Regola la luminosità dello schermo.~Настроить яркость подсветки.~Helligkeit der Beleuchtung einstellen.~Ajustar o brilho da retroiluminação.
Update Reboot 2 Payload~Actualiser Reboot 2 Payload~Actualizar Reboot 2 Payload~Aggiorna Reboot 2 Payload~Обновить Reboot 2 Payload~Reboot 2 Payload aktualisieren~Atualizar Reboot 2 Payload
January~Janvier~Enero~Gennaio~Январь~Januar~Janeiro
February~Février~Febrero~Febbraio~Февраль~Februar~Fevereiro
March~Mars~Marzo~Marzo~Март~März~Março
April~Avril~Abril~Aprile~Апрель~April~Abril
May~Mai~Mayo~Maggio~Май~Mai~Maio
June~Juin~Junio~Giugno~Июнь~Juni~Junho
July~Juillet~Julio~Luglio~Июль~Juli~Julho
August~Août~Agosto~Agosto~Август~August~Agosto
September~Septembre~Septiembre~Settembre~Сентябрь~September~Setembro
October~Octobre~Octubre~Ottobre~Октябрь~Oktober~Outubro
November~Novembre~Noviembre~Novembre~Ноябрь~November~Novembro
December~Décembre~Diciembre~Dicembre~Декабрь~Dezember~Dezembro
Backup & Restore~Sauvegarde et restauration~Copia y restauración~Backup e ripristino~Копирование и восстановление~Sichern und Wiederherstellen~Cópia e restauro
Backup~Sauvegarde~Copia de seguridad~Backup~Резервная копия~Sicherung~Cópia de segurança
Restore~Restauration~Restaurar~Ripristino~Восстановление~Wiederherstellen~Restaurar
Backup eMMC~Sauvegarder l'eMMC~Copiar eMMC~Backup eMMC~Скопировать eMMC~eMMC sichern~Copiar eMMC
Restore eMMC~Restaurer l'eMMC~Restaurar eMMC~Ripristina eMMC~Восстановить eMMC~eMMC wiederherstellen~Restaurar eMMC
Full~Complète~Completa~Completo~Полная~Vollständig~Completa
GPP Partitions~Partitions GPP~Particiones GPP~Partizioni GPP~Разделы GPP~GPP-Partitionen~Partições GPP
SD emuMMC Raw Partition~Partition SD emuMMC brute~Partición SD emuMMC sin formato~Partizione SD emuMMC raw~Raw-раздел SD emuMMC~SD-emuMMC-Raw-Partition~Partição SD emuMMC raw
Allows you to backup the BOOT physical partitions.~Sauvegarde les partitions physiques BOOT.~Copia las particiones físicas BOOT.~Crea il backup delle partizioni fisiche BOOT.~Копирование физических разделов BOOT.~Sichert die physischen BOOT-Partitionen.~Copia as partições físicas BOOT.
They contain the BCT, keys and various package1.~Elles contiennent le BCT, les clés et les package1.~Contienen el BCT, las claves y los package1.~Contengono BCT, chiavi e vari package1.~Они содержат BCT, ключи и файлы package1.~Sie enthalten BCT, Schlüssel und package1-Dateien.~Contêm o BCT, chaves e vários package1.
Allows you to backup the GPP physical partition.~Sauvegarde la partition physique GPP.~Copia la partición física GPP.~Crea il backup della partizione fisica GPP.~Копирование физического раздела GPP.~Sichert die physische GPP-Partition.~Copia a partição física GPP.
Allows you to restore the GPP physical partition.~Restaure la partition physique GPP.~Restaura la partición física GPP.~Ripristina la partizione fisica GPP.~Восстановление физического раздела GPP.~Stellt die physische GPP-Partition wieder her.~Restaura a partição física GPP.
Allows you to restore ALL partitions from RAW GPP~Restaure TOUTES les partitions depuis RAW GPP~Restaura TODAS las particiones desde RAW GPP~Ripristina TUTTE le partizioni da RAW GPP~Восстановить ВСЕ разделы из RAW GPP~Stellt ALLE Partitionen aus RAW GPP wieder her~Restaura TODAS as partições de RAW GPP
Checking for available free space...~Vérification de l'espace libre...~Comprobando el espacio libre...~Controllo dello spazio libero...~Проверка свободного места...~Freien Speicherplatz prüfen...~A verificar o espaço livre...
No single file, checking for part files...~Fichier unique absent, recherche des segments...~Sin archivo único, buscando partes...~File unico assente, ricerca delle parti...~Единый файл не найден, поиск частей...~Keine einzelne Datei, Teildateien suchen...~Sem ficheiro único, a procurar partes...
Only the 1st emuMMC found can be restored!~Seule la première emuMMC trouvée peut être restaurée !~¡Solo se puede restaurar la primera emuMMC encontrada!~Si può ripristinare solo la prima emuMMC trovata!~Можно восстановить только первую найденную emuMMC!~Nur die erste gefundene emuMMC kann wiederhergestellt werden!~Só a primeira emuMMC encontrada pode ser restaurada!
emuMMC Manage~Gestion emuMMC~Gestionar emuMMC~Gestione emuMMC~Управление emuMMC~emuMMC verwalten~Gerir emuMMC
emuMMC Info & Selection~Informations et sélection emuMMC~Información y selección de emuMMC~Informazioni e selezione emuMMC~Сведения и выбор emuMMC~emuMMC-Info und Auswahl~Informação e seleção de emuMMC
emuMMC Tools~Outils emuMMC~Herramientas emuMMC~Strumenti emuMMC~Инструменты emuMMC~emuMMC-Werkzeuge~Ferramentas emuMMC
Create emuMMC~Créer une emuMMC~Crear emuMMC~Crea emuMMC~Создать emuMMC~emuMMC erstellen~Criar emuMMC
Migrate emuMMC~Migrer une emuMMC~Migrar emuMMC~Migra emuMMC~Перенести emuMMC~emuMMC migrieren~Migrar emuMMC
Change emuMMC~Changer d'emuMMC~Cambiar emuMMC~Cambia emuMMC~Сменить emuMMC~emuMMC wechseln~Mudar emuMMC
Create SD File emuMMC~Créer une emuMMC en fichiers SD~Crear emuMMC en archivos SD~Crea emuMMC su file SD~Создать файловую emuMMC на SD~SD-Datei-emuMMC erstellen~Criar emuMMC em ficheiros SD
Create SD Partition emuMMC~Créer une emuMMC en partition SD~Crear emuMMC en partición SD~Crea emuMMC su partizione SD~Создать emuMMC в разделе SD~SD-Partitions-emuMMC erstellen~Criar emuMMC em partição SD
Partition table:~Table des partitions :~Tabla de particiones:~Tabella partizioni:~Таблица разделов:~Partitionstabelle:~Tabela de partições:
SD File~Fichier SD~Archivo SD~File SD~Файл SD~SD-Datei~Ficheiro SD
SD Partition~Partition SD~Partición SD~Partizione SD~Раздел SD~SD-Partition~Partição SD
SD Raw Partitions~Partitions SD brutes~Particiones SD sin formato~Partizioni SD raw~Raw-разделы SD~SD-Raw-Partitionen~Partições SD raw
SD File Based~Fichiers sur SD~Basada en archivos SD~Basata su file SD~На основе файлов SD~Auf SD-Dateien basiert~Baseada em ficheiros SD
No emuMMC found!~Aucune emuMMC trouvée !~¡No se encontró emuMMC!~Nessuna emuMMC trovata!~emuMMC не найдена!~Keine emuMMC gefunden!~Nenhuma emuMMC encontrada!
Fix RAW~Réparer RAW~Reparar RAW~Ripara RAW~Исправить RAW~RAW reparieren~Reparar RAW
emuMMC is disabled and eMMC will be used for boot.~L'emuMMC est désactivée ; le démarrage utilisera l'eMMC.~emuMMC está desactivada; el inicio usará eMMC.~emuMMC è disattivata; verrà avviata eMMC.~emuMMC отключена; загрузка будет с eMMC.~emuMMC ist deaktiviert; zum Start wird eMMC verwendet.~emuMMC desativada; o arranque usará eMMC.
Choose between images created in the emuMMC folder~Choisir une image créée dans le dossier emuMMC~Elige una imagen creada en la carpeta emuMMC~Scegli un'immagine creata nella cartella emuMMC~Выберите образ из папки emuMMC~Abbild aus dem emuMMC-Ordner auswählen~Escolher uma imagem criada na pasta emuMMC
Formatting USER...~Formatage de USER...~Formateando USER...~Formattazione USER...~Форматирование USER...~USER formatieren...~A formatar USER...
Writing new GPT...~Écriture de la nouvelle GPT...~Escribiendo la nueva GPT...~Scrittura della nuova GPT...~Запись новой GPT...~Neue GPT schreiben...~A escrever a nova GPT...
SD Partitions & USB~Partitions SD et USB~Particiones SD y USB~Partizioni SD e USB~Разделы SD и USB~SD-Partitionen und USB~Partições SD e USB
Partition SD Card~Partitionner la carte SD~Particionar tarjeta SD~Partiziona scheda SD~Разметка карты SD~SD-Karte partitionieren~Particionar cartão SD
SD Partitions~Partitions SD~Particiones SD~Partizioni SD~Разделы SD~SD-Partitionen~Partições SD
SD Partition Manager~Gestionnaire de partitions SD~Gestor de particiones SD~Gestore partizioni SD~Управление разделами SD~SD-Partitionsverwaltung~Gestor de partições SD
eMMC Partition Manager~Gestionnaire de partitions eMMC~Gestor de particiones eMMC~Gestore partizioni eMMC~Управление разделами eMMC~eMMC-Partitionsverwaltung~Gestor de partições eMMC
Flash Linux~Installer Linux~Instalar Linux~Installa Linux~Установить Linux~Linux installieren~Instalar Linux
Flash Android~Installer Android~Instalar Android~Installa Android~Установить Android~Android installieren~Instalar Android
Flash Android 11~Installer Android 11~Instalar Android 11~Installa Android 11~Установить Android 11~Android 11 installieren~Instalar Android 11
Flash Android 13+~Installer Android 13+~Instalar Android 13+~Installa Android 13+~Установить Android 13+~Android 13+ installieren~Instalar Android 13+
Delete Installation Files~Supprimer les fichiers d'installation~Eliminar archivos de instalación~Elimina file di installazione~Удалить файлы установки~Installationsdateien löschen~Eliminar ficheiros de instalação
These will be deleted after a successful flash.~Ils seront supprimés après une installation réussie.~Se eliminarán tras una instalación correcta.~Saranno eliminati dopo un'installazione riuscita.~Они будут удалены после успешной установки.~Sie werden nach erfolgreicher Installation gelöscht.~Serão eliminados após uma instalação bem-sucedida.
Please select a partition scheme:~Choisissez un schéma de partitions :~Elige un esquema de particiones:~Scegli uno schema di partizioni:~Выберите схему разделов:~Partitionsschema auswählen:~Escolha um esquema de partições:
Legacy~Classique~Clásico~Classico~Старый~Klassisch~Clássico
Dynamic~Dynamique~Dinámico~Dinamico~Динамический~Dynamisch~Dinâmico
Don't Backup~Ne pas sauvegarder~No hacer copia~Non creare backup~Не копировать~Nicht sichern~Não criar cópia
don't backup~ne pas sauvegarder~no hacer copia~non creare backup~не копировать~nicht sichern~não criar cópia
USB Tools~Outils USB~Herramientas USB~Strumenti USB~Инструменты USB~USB-Werkzeuge~Ferramentas USB
USB Mass Storage~Stockage de masse USB~Almacenamiento masivo USB~Memoria di massa USB~USB-накопитель~USB-Massenspeicher~Armazenamento USB
Read-Only~Lecture seule~Solo lectura~Sola lettura~Только чтение~Schreibgeschützt~Só de leitura
USB Input Devices~Périphériques d'entrée USB~Dispositivos de entrada USB~Dispositivi di input USB~Устройства ввода USB~USB-Eingabegeräte~Dispositivos de entrada USB
Gamepad~Manette~Mando~Gamepad~Геймпад~Gamepad~Comando
Touchpad~Pavé tactile~Panel táctil~Touchpad~Сенсорная панель~Touchpad~Painel tátil
Plug-in the Joy-Con and convert the device~Connecter les Joy-Con pour transformer la console~Conecta los Joy-Con para convertir la consola~Collega i Joy-Con per trasformare la console~Подключите Joy-Con, чтобы превратить консоль~Joy-Con anschließen und die Konsole~Ligue os Joy-Con para transformar a consola
into a gamepad for PC or Phone.~en manette pour ordinateur ou téléphone.~en un mando para PC o teléfono.~in un gamepad per PC o telefono.~в геймпад для ПК или телефона.~als Gamepad für PC oder Telefon nutzen.~num comando para PC ou telemóvel.
Control the PC via the device's touchscreen.~Contrôler l'ordinateur avec l'écran tactile.~Controla el PC con la pantalla táctil.~Controlla il PC con il touchscreen.~Управляйте ПК с сенсорного экрана.~PC über den Touchscreen steuern.~Controlar o PC com o ecrã tátil.
Misc~Divers~Varios~Varie~Прочее~Sonstiges~Diversos
Others~Autres~Otros~Altro~Другое~Weitere~Outros
Main~Principal~Principal~Principale~Основной~Hauptbereich~Principal
Safe~Sécurité~Seguridad~Sicurezza~Безопасность~Sicherheit~Segurança
Fix Archive Bit~Corriger le bit d'archive~Corregir bit de archivo~Correggi bit archivio~Исправить архивный бит~Archivbit korrigieren~Corrigir bit de arquivo
Fix Archive Bit (All folders)~Corriger le bit d'archive (tous les dossiers)~Corregir bit de archivo (todas las carpetas)~Correggi bit archivio (tutte le cartelle)~Исправить архивный бит (все папки)~Archivbit korrigieren (alle Ordner)~Corrigir bit de arquivo (todas as pastas)
Arch bit~Bit d'archive~Bit de archivo~Bit archivio~Архивный бит~Archivbit~Bit de arquivo
Calibrate Touchscreen~Calibrer l'écran tactile~Calibrar pantalla táctil~Calibra touchscreen~Калибровка сенсора~Touchscreen kalibrieren~Calibrar ecrã tátil
Allows you to calibrate the touchscreen module.~Permet de calibrer le module tactile.~Permite calibrar el módulo táctil.~Permette di calibrare il modulo touch.~Калибровка сенсорного модуля.~Ermöglicht die Kalibrierung des Touchscreens.~Permite calibrar o módulo tátil.
Touch~Tactile~Táctil~Touch~Сенсор~Touch~Tátil
SD Card~Carte SD~Tarjeta SD~Scheda SD~Карта SD~SD-Karte~Cartão SD
microSD Card Info~Informations carte microSD~Información de tarjeta microSD~Informazioni scheda microSD~Сведения о карте microSD~microSD-Karteninfo~Informações do cartão microSD
Internal eMMC Info~Informations eMMC interne~Información de eMMC interna~Informazioni eMMC interna~Сведения о встроенной eMMC~Interne eMMC-Info~Informações da eMMC interna
HW & Fuses Info~Informations matériel et fusibles~Info de hardware y fusibles~Info hardware e fusibili~Сведения об оборудовании и фьюзах~Hardware- und Fuse-Info~Info de hardware e fusíveis
SoC & HW Info~Informations SoC et matériel~Info de SoC y hardware~Info SoC e hardware~Сведения о SoC и оборудовании~SoC- und Hardware-Info~Info de SoC e hardware
HW & Fuses~Matériel et fusibles~Hardware y fusibles~Hardware e fusibili~Оборудование и фьюзы~Hardware und Fuses~Hardware e fusíveis
Storage & Battery Info~Informations stockage et batterie~Info de almacenamiento y batería~Info memoria e batteria~Сведения о накопителях и батарее~Speicher- und Akkuinfo~Info de armazenamento e bateria
Battery~Batterie~Batería~Batteria~Батарея~Akku~Bateria
Battery Info~Informations batterie~Información de batería~Informazioni batteria~Сведения о батарее~Akkuinfo~Informações da bateria
View battery and battery charger related info.~Afficher les informations de batterie et de charge.~Ver información de la batería y el cargador.~Mostra informazioni su batteria e caricatore.~Сведения о батарее и зарядном устройстве.~Informationen zu Akku und Ladegerät anzeigen.~Ver informações da bateria e do carregador.
Benchmark~Mesure de vitesse~Prueba de velocidad~Test velocità~Тест скорости~Leistungstest~Teste de velocidade
Additionally you can benchmark read speeds.~Permet aussi de mesurer les vitesses de lecture.~También permite medir la velocidad de lectura.~Permette anche di misurare la velocità di lettura.~Также можно измерить скорость чтения.~Zusätzlich lässt sich die Lesegeschwindigkeit messen.~Também permite medir a velocidade de leitura.
Device Report~Rapport du périphérique~Informe del dispositivo~Rapporto dispositivo~Отчёт устройства~Gerätebericht~Relatório do dispositivo
Dump fuses~Exporter les fusibles~Exportar fusibles~Esporta fusibili~Экспорт фьюзов~Fuses exportieren~Exportar fusíveis
Dump Bootrom~Exporter la Bootrom~Exportar Bootrom~Esporta Bootrom~Экспорт Bootrom~Bootrom exportieren~Exportar Bootrom
Bootrom Info~Informations Bootrom~Información de Bootrom~Informazioni Bootrom~Сведения о Bootrom~Bootrom-Info~Informações da Bootrom
Dump Fuel Regs~Exporter les registres batterie~Exportar registros de batería~Esporta registri batteria~Экспорт регистров батареи~Akkuregister exportieren~Exportar registos da bateria
Dumping to SD card finished!~Exportation sur la carte SD terminée !~¡Exportación a la tarjeta SD terminada!~Esportazione sulla scheda SD completata!~Экспорт на карту SD завершён!~Export auf die SD-Karte abgeschlossen!~Exportação para o cartão SD concluída!
Vendor Registers~Registres fabricant~Registros del fabricante~Registri del produttore~Регистры производителя~Herstellerregister~Registos do fabricante
Not charging~Pas de charge~Sin cargar~Non in carica~Не заряжается~Lädt nicht~Não está a carregar
Pre-charging~Précharge~Precarga~Precarica~Предзаряд~Vorladung~Pré-carga
Fast charging~Charge rapide~Carga rápida~Ricarica rapida~Быстрая зарядка~Schnellladen~Carga rápida
Charge terminated~Charge terminée~Carga terminada~Carica terminata~Зарядка завершена~Laden beendet~Carga terminada
Warm~Chaud~Caliente~Caldo~Тёплая~Warm~Quente
Cool~Frais~Frío~Fresco~Холодная~Kühl~Fria
Connected~Connecté~Conectado~Connesso~Подключено~Verbunden~Ligado
Disconnected~Déconnecté~Desconectado~Disconnesso~Отключено~Getrennt~Desligado
Undefined~Non défini~Sin definir~Non definito~Не задано~Nicht definiert~Não definido
Inactive~Inactif~Inactivo~Inattivo~Неактивно~Inaktiv~Inativo
Temporary~Temporaire~Temporal~Temporaneo~Временно~Temporär~Temporário
Permanent~Permanent~Permanente~Permanente~Постоянно~Dauerhaft~Permanente
Capacity:~Capacité :~Capacidad:~Capacità:~Ёмкость:~Kapazität:~Capacidade:
Capacity (LBA):~Capacité (LBA) :~Capacidad (LBA):~Capacità (LBA):~Ёмкость (LBA):~Kapazität (LBA):~Capacidade (LBA):
Bus Width:~Largeur du bus :~Ancho del bus:~Larghezza bus:~Ширина шины:~Busbreite:~Largura do barramento:
Max Bus Speed:~Vitesse maximale du bus :~Velocidad máxima del bus:~Velocità massima bus:~Макс. скорость шины:~Max. Busgeschwindigkeit:~Velocidade máxima do barramento:
Speed Class:~Classe de vitesse :~Clase de velocidad:~Classe velocità:~Класс скорости:~Geschwindigkeitsklasse:~Classe de velocidade:
Write Protect:~Protection en écriture :~Protección de escritura:~Protezione scrittura:~Защита от записи:~Schreibschutz:~Proteção de escrita:
Vendor Info:~Informations fabricant :~Info del fabricante:~Info produttore:~Сведения о производителе:~Herstellerinfo:~Info do fabricante:
Vendor ID:~Identifiant fabricant :~ID del fabricante:~ID produttore:~ID производителя:~Hersteller-ID:~ID do fabricante:
Model:~Modèle :~Modelo:~Modello:~Модель:~Modell:~Modelo:
Uptime:~Durée de fonctionnement :~Tiempo de actividad:~Tempo di attività:~Время работы:~Betriebszeit:~Tempo de atividade:
Filesystem:~Système de fichiers :~Sistema de archivos:~File system:~Файловая система:~Dateisystem:~Sistema de ficheiros:
Cluster:~Unité d'allocation :~Clúster:~Cluster:~Кластер:~Cluster:~Cluster:
Init fails:~Échecs d'initialisation :~Fallos de inicio:~Errori di inizializzazione:~Сбои инициализации:~Initialisierungsfehler:~Falhas de inicialização:
Capacity now:~Capacité actuelle :~Capacidad actual:~Capacità attuale:~Текущая ёмкость:~Aktuelle Kapazität:~Capacidade atual:
Capacity full:~Capacité pleine :~Capacidad completa:~Capacità piena:~Полная ёмкость:~Volle Kapazität:~Capacidade total:
Capacity (design):~Capacité nominale :~Capacidad nominal:~Capacità nominale:~Проектная ёмкость:~Nennkapazität:~Capacidade nominal:
Current now:~Courant actuel :~Corriente actual:~Corrente attuale:~Текущий ток:~Aktueller Strom:~Corrente atual:
Current average:~Courant moyen :~Corriente media:~Corrente media:~Средний ток:~Mittlerer Strom:~Corrente média:
Voltage now:~Tension actuelle :~Tensión actual:~Tensione attuale:~Текущее напряжение:~Aktuelle Spannung:~Tensão atual:
Voltage open-circuit:~Tension à vide :~Tensión en vacío:~Tensione a vuoto:~Напряжение без нагрузки:~Leerlaufspannung:~Tensão em vazio:
Min voltage reached:~Tension minimale atteinte :~Tensión mínima alcanzada:~Tensione minima raggiunta:~Мин. напряжение:~Niedrigste Spannung:~Tensão mínima atingida:
Max voltage reached:~Tension maximale atteinte :~Tensión máxima alcanzada:~Tensione massima raggiunta:~Макс. напряжение:~Höchste Spannung:~Tensão máxima atingida:
Empty voltage:~Tension de batterie vide :~Tensión de batería vacía:~Tensione batteria scarica:~Напряжение разряда:~Entladespannung:~Tensão da bateria vazia:
Battery temp:~Température batterie :~Temperatura de batería:~Temperatura batteria:~Температура батареи:~Akkutemperatur:~Temperatura da bateria:
Main PMIC:~PMIC principal :~PMIC principal:~PMIC principale:~Основной PMIC:~Haupt-PMIC:~PMIC principal:
Input current limit:~Limite de courant d'entrée :~Límite de corriente de entrada:~Limite corrente in ingresso:~Предел входного тока:~Eingangsstromgrenze:~Limite de corrente de entrada:
System voltage limit:~Limite de tension système :~Límite de tensión del sistema:~Limite tensione sistema:~Предел напряжения системы:~Systemspannungsgrenze:~Limite de tensão do sistema:
Charge current limit:~Limite de courant de charge :~Límite de corriente de carga:~Limite corrente di carica:~Предел зарядного тока:~Ladestromgrenze:~Limite de corrente de carga:
Charge voltage limit:~Limite de tension de charge :~Límite de tensión de carga:~Limite tensione di carica:~Предел зарядного напряжения:~Ladespannungsgrenze:~Limite de tensão de carga:
Charge status:~État de charge :~Estado de carga:~Stato di carica:~Состояние зарядки:~Ladestatus:~Estado de carga:
Temperature status:~État thermique :~Estado térmico:~Stato termico:~Температурный статус:~Temperaturstatus:~Estado térmico:
Connection status:~État de connexion :~Estado de conexión:~Stato connessione:~Состояние подключения:~Verbindungsstatus:~Estado da ligação:
Input Wattage Limit:~Puissance d'entrée maximale :~Potencia máxima de entrada:~Potenza massima in ingresso:~Предел входной мощности:~Eingangsleistungsgrenze:~Potência máxima de entrada:
Can't be read (locked out)~Lecture impossible (verrouillé)~No se puede leer (bloqueado)~Lettura impossibile (bloccato)~Чтение невозможно (заблокировано)~Nicht lesbar (gesperrt)~Leitura impossível (bloqueado)
HWTest overview~Bilan HWTest~Resumen HWTest~Riepilogo HWTest~Сводка HWTest~HWTest-Übersicht~Resumo HWTest
All groups except RAM stress~Tous les groupes, hors stress RAM~Todos los grupos salvo estrés de RAM~Tutti i gruppi tranne stress RAM~Все группы, кроме стресс-теста RAM~Alle Gruppen außer RAM-Stresstest~Todos os grupos exceto stress de RAM
Identity~Identité~Identidad~Identità~Идентификация~Identität~Identidade
SoC, fuses and KFUSE~SoC, fusibles et KFUSE~SoC, fusibles y KFUSE~SoC, fusibili e KFUSE~SoC, фьюзы и KFUSE~SoC, Fuses und KFUSE~SoC, fusíveis e KFUSE
Power~Alimentation~Alimentación~Alimentazione~Питание~Stromversorgung~Alimentação
Battery, charging, voltages, thermal~Batterie, charge, tensions, thermique~Batería, carga, tensiones y temperatura~Batteria, carica, tensioni e temperatura~Батарея, зарядка, напряжения, температура~Akku, Laden, Spannungen, Temperatur~Bateria, carga, tensões e temperatura
Memory / clocks~Mémoire / horloges~Memoria / relojes~Memoria / clock~Память / частоты~Speicher / Takte~Memória / relógios
DRAM identity and clocks~Identité DRAM et horloges~Identificación de DRAM y relojes~Identità DRAM e clock~Идентификация DRAM и частоты~DRAM-Identität und Takte~Identidade DRAM e relógios
Storage~Stockage~Almacenamiento~Memoria di massa~Накопители~Speicher~Armazenamento
SD, eMMC, partitions and reader~SD, eMMC, partitions et lecteur~SD, eMMC, particiones y lector~SD, eMMC, partizioni e lettore~SD, eMMC, разделы и считыватель~SD, eMMC, Partitionen und Leser~SD, eMMC, partições e leitor
Wireless~Sans fil~Inalámbrico~Wireless~Беспроводная связь~Funk~Sem fios
Bluetooth and Wi-Fi / PCIe~Bluetooth et Wi-Fi / PCIe~Bluetooth y Wi-Fi / PCIe~Bluetooth e Wi-Fi / PCIe~Bluetooth и Wi-Fi / PCIe~Bluetooth und Wi-Fi / PCIe~Bluetooth e Wi-Fi / PCIe
Audio~Audio~Audio~Audio~Аудио~Audio~Áudio
Codec and audio clocks~Codec et horloges audio~Códec y relojes de audio~Codec e clock audio~Кодек и аудиочастоты~Codec und Audiotakte~Codec e relógios de áudio
Sound test~Test sonore~Prueba de sonido~Test audio~Проверка звука~Tontest~Teste de som
Speakers and headphone jack~Haut-parleurs et prise casque~Altavoces y toma de auriculares~Altoparlanti e presa cuffie~Динамики и разъём наушников~Lautsprecher und Kopfhörerbuchse~Altifalantes e saída de auscultadores
Display~Écran~Pantalla~Schermo~Экран~Bildschirm~Ecrã
LCD / OLED panel and backlight~Dalle et rétroéclairage LCD / OLED~Panel y retroiluminación LCD / OLED~Pannello e retroilluminazione LCD / OLED~Панель и подсветка LCD / OLED~LCD-/OLED-Panel und Beleuchtung~Painel e retroiluminação LCD / OLED
Inputs~Commandes~Controles~Comandi~Устройства ввода~Eingaben~Controlos
Touch, Joy-Con, buttons, light~Tactile, Joy-Con, boutons, lumière~Táctil, Joy-Con, botones y luz~Touch, Joy-Con, pulsanti e luce~Сенсор, Joy-Con, кнопки, свет~Touch, Joy-Con, Tasten, Licht~Tátil, Joy-Con, botões e luz
Low-level state~État bas niveau~Estado de bajo nivel~Stato a basso livello~Низкоуровневое состояние~Low-Level-Status~Estado de baixo nível
GPIO, UART and boot state~GPIO, UART et état de démarrage~GPIO, UART y estado de inicio~GPIO, UART e stato di avvio~GPIO, UART и состояние загрузки~GPIO, UART und Startstatus~GPIO, UART e estado de arranque
Isolated RAM test~Test RAM isolé~Prueba de RAM aislada~Test RAM isolato~Отдельный тест RAM~Separater RAM-Test~Teste de RAM isolado
Full pass then return to Hekate~Passe complète puis retour Hekate~Pasada completa y regreso a Hekate~Passaggio completo e ritorno a Hekate~Полный проход и возврат в Hekate~Vollständiger Durchlauf, dann zurück zu Hekate~Passagem completa e regresso ao Hekate
WARNING~AVERTISSEMENT~ADVERTENCIA~AVVISO~ПРЕДУПРЕЖДЕНИЕ~WARNUNG~AVISO
FAULT~DÉFAUT~FALLO~GUASTO~НЕИСПРАВНОСТЬ~DEFEKT~FALHA
ERROR~ERREUR~ERROR~ERRORE~ОШИБКА~FEHLER~ERRO
OBSERVATION~OBSERVATION~OBSERVACIÓN~OSSERVAZIONE~НАБЛЮДЕНИЕ~BEOBACHTUNG~OBSERVAÇÃO
NOT RUN~NON EXÉCUTÉ~NO EJECUTADO~NON ESEGUITO~НЕ ВЫПОЛНЕНО~NICHT AUSGEFÜHRT~NÃO EXECUTADO
ABORTED~INTERROMPU~INTERRUMPIDO~INTERROTTO~ПРЕРВАНО~ABGEBROCHEN~INTERROMPIDO
Not enough memory.~Mémoire insuffisante.~Memoria insuficiente.~Memoria insufficiente.~Недостаточно памяти.~Nicht genügend Speicher.~Memória insuficiente.
No report for this selection.~Aucun rapport pour cette sélection.~No hay informe para esta selección.~Nessun rapporto per questa selezione.~Нет отчёта для этого выбора.~Kein Bericht für diese Auswahl.~Sem relatório para esta seleção.
HWTest report~Rapport HWTest~Informe HWTest~Rapporto HWTest~Отчёт HWTest~HWTest-Bericht~Relatório HWTest
Export report~Exporter le rapport~Exportar informe~Esporta rapporto~Экспорт отчёта~Bericht exportieren~Exportar relatório
Diagnostics - Last run~Diagnostic - Dernière exécution~Diagnóstico - Última ejecución~Diagnostica - Ultima esecuzione~Диагностика - Последний запуск~Diagnose - Letzter Durchlauf~Diagnóstico - Última execução
Report~Rapport~Informe~Rapporto~Отчёт~Bericht~Relatório
Export~Exporter~Exportar~Esporta~Экспорт~Exportieren~Exportar
Execution or restoration incomplete.~Exécution ou restauration incomplète.~Ejecución o restauración incompleta.~Esecuzione o ripristino incompleto.~Выполнение или восстановление не завершено.~Ausführung oder Wiederherstellung unvollständig.~Execução ou restauro incompleto.
Report truncated: see the end marker.~Rapport tronqué : consultez le marqueur de fin.~Informe truncado: consulta el marcador final.~Rapporto troncato: consulta il marcatore finale.~Отчёт обрезан: см. отметку в конце.~Bericht gekürzt: Endmarkierung beachten.~Relatório truncado: consulte o marcador final.
Observations without a verdict do not mean a passed test.~Les observations sans verdict ne constituent pas un test réussi.~Las observaciones sin veredicto no indican una prueba superada.~Le osservazioni senza esito non indicano un test superato.~Наблюдения без вердикта не означают успешный тест.~Beobachtungen ohne Ergebnis gelten nicht als bestandener Test.~Observações sem veredicto não significam um teste aprovado.
Touch controls~Commandes tactiles~Controles táctiles~Comandi touch~Сенсорное управление~Touch-Bedienung~Controlos táteis
Diagnostics running~Diagnostic en cours~Diagnóstico en curso~Diagnostica in corso~Диагностика выполняется~Diagnose läuft~Diagnóstico em curso
Preparing checks...~Préparation des contrôles...~Preparando comprobaciones...~Preparazione controlli...~Подготовка проверок...~Prüfungen vorbereiten...~A preparar verificações...
Controls are suspended during hardware measurements.~Les commandes sont suspendues pendant les mesures matérielles.~Los controles se suspenden durante las mediciones de hardware.~I comandi sono sospesi durante le misurazioni hardware.~Управление приостановлено на время аппаратных измерений.~Die Bedienung ist während der Hardware-Messungen gesperrt.~Os controlos ficam suspensos durante as medições de hardware.
Run this group~Exécuter ce groupe~Ejecutar este grupo~Esegui questo gruppo~Запустить эту группу~Diese Gruppe ausführen~Executar este grupo
A sound is played on each output. Check the result by listening.~Un son est émis sur chaque sortie. Vérifiez le résultat à l'écoute.~Se reproduce un sonido en cada salida. Comprueba el resultado escuchando.~Ogni uscita riproduce un suono. Verifica il risultato ascoltando.~На каждом выходе звучит сигнал. Проверьте результат на слух.~Jeder Ausgang gibt einen Ton aus. Ergebnis durch Hören prüfen.~É emitido um som em cada saída. Verifique o resultado ao ouvir.
RAM test~Test RAM~Prueba de RAM~Test RAM~Тест RAM~RAM-Test~Teste de RAM
Full pass and automatic return to Hekate~Passe complète et retour automatique à Hekate~Pasada completa y regreso automático a Hekate~Passaggio completo e ritorno automatico a Hekate~Полный проход и автоматический возврат в Hekate~Vollständiger Durchlauf und automatische Rückkehr zu Hekate~Passagem completa e regresso automático ao Hekate
Start RAM test~Lancer le test RAM~Iniciar prueba de RAM~Avvia test RAM~Запустить тест RAM~RAM-Test starten~Iniciar teste de RAM
Hardware diagnostics~Diagnostic matériel~Diagnóstico de hardware~Diagnostica hardware~Аппаратная диагностика~Hardware-Diagnose~Diagnóstico de hardware
HWTest-RCM: 42 native checks and an isolated RAM service. Choose a category.~HWTest-RCM : 42 contrôles natifs et un service RAM isolé. Choisissez une catégorie.~HWTest-RCM: 42 comprobaciones nativas y un servicio RAM aislado. Elige una categoría.~HWTest-RCM: 42 controlli nativi e un servizio RAM isolato. Scegli una categoria.~HWTest-RCM: 42 встроенные проверки и отдельный сервис RAM. Выберите категорию.~HWTest-RCM: 42 native Prüfungen und ein separater RAM-Dienst. Kategorie auswählen.~HWTest-RCM: 42 verificações nativas e um serviço RAM isolado. Escolha uma categoria.
No diagnostics run.~Aucun diagnostic exécuté.~No se ha ejecutado ningún diagnóstico.~Nessuna diagnostica eseguita.~Диагностика ещё не запускалась.~Noch keine Diagnose ausgeführt.~Nenhum diagnóstico executado.
Fuses~Fusibles~Fusibles~Fusibili~Фьюзы~Fuses~Fusíveis
Power & charging~Alimentation et charge~Alimentación y carga~Alimentazione e carica~Питание и зарядка~Stromversorgung und Laden~Alimentação e carga
Memory & clocks~Mémoire et horloges~Memoria y relojes~Memoria e clock~Память и частоты~Speicher und Takte~Memória e relógios
DRAM memory~Mémoire DRAM~Memoria DRAM~Memoria DRAM~Память DRAM~DRAM-Speicher~Memória DRAM
Battery charger~Chargeur de batterie~Cargador de batería~Caricabatteria~Зарядное устройство~Akkuladegerät~Carregador de bateria
Regulators~Régulateurs~Reguladores~Regolatori~Регуляторы~Spannungsregler~Reguladores
Thermal~Température~Temperatura~Temperatura~Температура~Temperatur~Temperatura
Fan~Ventilateur~Ventilador~Ventola~Вентилятор~Lüfter~Ventoinha
Clocks~Horloges~Relojes~Clock~Частоты~Takte~Relógios
Partitions~Partitions~Particiones~Partizioni~Разделы~Partitionen~Partições
eMMC health~État de l'eMMC~Estado de eMMC~Stato eMMC~Состояние eMMC~eMMC-Zustand~Estado da eMMC
Storage errors~Erreurs de stockage~Errores de almacenamiento~Errori di memoria~Ошибки накопителей~Speicherfehler~Erros de armazenamento
Game card~Carte de jeu~Tarjeta de juego~Scheda di gioco~Игровая карта~Spielkarte~Cartão de jogo
Buttons~Boutons~Botones~Pulsanti~Кнопки~Tasten~Botões
Light sensor~Capteur de lumière~Sensor de luz~Sensore di luce~Датчик освещённости~Lichtsensor~Sensor de luz
Touchscreen~Écran tactile~Pantalla táctil~Touchscreen~Сенсорный экран~Touchscreen~Ecrã tátil
Headphones~Casque~Auriculares~Cuffie~Наушники~Kopfhörer~Auscultadores
Speakers~Haut-parleurs~Altavoces~Altoparlanti~Динамики~Lautsprecher~Altifalantes
Page %d / %d~Page %d / %d~Página %d / %d~Pagina %d / %d~Страница %d / %d~Seite %d / %d~Página %d / %d
Report saved:\n%s~Rapport enregistré :\n%s~Informe guardado:\n%s~Rapporto salvato:\n%s~Отчёт сохранён:\n%s~Bericht gespeichert:\n%s~Relatório guardado:\n%s
Report export failed (code %d).\nCheck the SD card.~Échec de l'export du rapport (code %d).\nVérifiez la carte SD.~Error al exportar el informe (código %d).\nComprueba la tarjeta SD.~Esportazione del rapporto fallita (codice %d).\nControlla la scheda SD.~Не удалось экспортировать отчёт (код %d).\nПроверьте карту SD.~Berichtexport fehlgeschlagen (Code %d).\nSD-Karte prüfen.~Falha ao exportar o relatório (código %d).\nVerifique o cartão SD.
Touch restoration failed.\nUse the Joy-Con or reload Nyx.~La restauration du tactile a échoué.\nUtilisez les Joy-Con ou rechargez Nyx.~No se pudo restaurar el control táctil.\nUsa los Joy-Con o recarga Nyx.~Ripristino touch fallito.\nUsa i Joy-Con o ricarica Nyx.~Не удалось восстановить сенсор.\nИспользуйте Joy-Con или перезагрузите Nyx.~Touch-Wiederherstellung fehlgeschlagen.\nJoy-Con verwenden oder Nyx neu laden.~Falha ao restaurar o controlo tátil.\nUse os Joy-Con ou recarregue o Nyx.
The RAM service could not start (code %d).\nCheck bootloader/sys/hwtest_ram.bin and bootloader/update.bin.~Le service RAM n'a pas pu être lancé (code %d).\nVérifiez bootloader/sys/hwtest_ram.bin et bootloader/update.bin.~No se pudo iniciar el servicio RAM (código %d).\nComprueba bootloader/sys/hwtest_ram.bin y bootloader/update.bin.~Impossibile avviare il servizio RAM (codice %d).\nControlla bootloader/sys/hwtest_ram.bin e bootloader/update.bin.~Не удалось запустить сервис RAM (код %d).\nПроверьте bootloader/sys/hwtest_ram.bin и bootloader/update.bin.~RAM-Dienst konnte nicht starten (Code %d).\nbootloader/sys/hwtest_ram.bin und bootloader/update.bin prüfen.~Não foi possível iniciar o serviço RAM (código %d).\nVerifique bootloader/sys/hwtest_ram.bin e bootloader/update.bin.
Last result: %s\n%d / %d patterns, planned area: %d MiB of %d MiB\n%d error(s), duration %d s, MTC errors: %08X~Dernier résultat : %s\n%d / %d motifs, zone prévue : %d MiB sur %d MiB\n%d erreur(s), durée %d s, erreurs MTC : %08X~Último resultado: %s\n%d / %d patrones, zona prevista: %d MiB de %d MiB\n%d error(es), duración %d s, errores MTC: %08X~Ultimo risultato: %s\n%d / %d schemi, area prevista: %d MiB su %d MiB\n%d errore/i, durata %d s, errori MTC: %08X~Последний результат: %s\n%d / %d шаблонов, область: %d MiB из %d MiB\nОшибок: %d, время %d с, ошибки MTC: %08X~Letztes Ergebnis: %s\n%d / %d Muster, geplanter Bereich: %d MiB von %d MiB\n%d Fehler, Dauer %d s, MTC-Fehler: %08X~Último resultado: %s\n%d / %d padrões, área prevista: %d MiB de %d MiB\n%d erro(s), duração %d s, erros MTC: %08X
%d OK  /  %d warning(s)  /  %d fault(s)  /  %d error(s)  /  %d observation(s)\n%s%s~%d OK  /  %d avert.  /  %d défaut(s)  /  %d erreur(s)  /  %d observation(s)\n%s%s~%d OK  /  %d aviso(s)  /  %d fallo(s)  /  %d error(es)  /  %d observación(es)\n%s%s~%d OK  /  %d avviso/i  /  %d guasto/i  /  %d errore/i  /  %d osservazione/i\n%s%s~%d ОК  /  %d предупрежд.  /  %d неисправн.  /  %d ошибок  /  %d наблюд.\n%s%s~%d OK  /  %d Warnung(en)  /  %d Defekt(e)  /  %d Fehler  /  %d Beobachtung(en)\n%s%s~%d OK  /  %d aviso(s)  /  %d falha(s)  /  %d erro(s)  /  %d observação(ões)\n%s%s
Nyx will close to test available memory with 7 advanced patterns.\nThe Hekate return area and reserved areas remain excluded.\nAt the end, Hekate reloads and displays the result.\nWith faulty hardware or unstable memory, the return may fail.~Nyx sera quitté pour tester la mémoire disponible avec les 7 motifs avancés.\nLa zone de retour Hekate et les zones réservées restent exclues du test.\nÀ la fin, Hekate est rechargé et le résultat est affiché.\nEn cas de panne matérielle ou de mémoire instable, le retour peut échouer.~Nyx se cerrará para probar la memoria disponible con 7 patrones avanzados.\nSe excluyen la zona de retorno a Hekate y las zonas reservadas.\nAl terminar, Hekate se recarga y muestra el resultado.\nCon hardware defectuoso o memoria inestable, el regreso puede fallar.~Nyx si chiuderà per testare la memoria disponibile con 7 schemi avanzati.\nL'area di ritorno a Hekate e le aree riservate restano escluse.\nAl termine, Hekate si ricarica e mostra il risultato.\nCon hardware guasto o memoria instabile, il ritorno può fallire.~Nyx закроется для проверки доступной памяти 7 расширенными шаблонами.\nОбласть возврата в Hekate и резервные области исключены.\nПо завершении Hekate перезагрузится и покажет результат.\nПри неисправном оборудовании или нестабильной памяти возврат может не сработать.~Nyx wird geschlossen, um verfügbaren Speicher mit 7 erweiterten Mustern zu prüfen.\nDer Hekate-Rückkehrbereich und reservierte Bereiche bleiben ausgeschlossen.\nDanach lädt Hekate neu und zeigt das Ergebnis.\nBei Hardwarefehlern oder instabilem Speicher kann die Rückkehr scheitern.~O Nyx será fechado para testar a memória disponível com 7 padrões avançados.\nA área de regresso ao Hekate e as áreas reservadas ficam excluídas.\nNo fim, o Hekate recarrega e mostra o resultado.\nCom hardware avariado ou memória instável, o regresso pode falhar.
"""

ROWS += r"""
Raw state~État brut~Estado sin procesar~Stato grezzo~Необработанное состояние~Rohstatus~Estado bruto
Partition Manager~Gestionnaire de partitions~Gestor de particiones~Gestore partizioni~Управление разделами~Partitionsverwaltung~Gestor de partições
Linux Flasher~Installation de Linux~Instalador de Linux~Installazione Linux~Установка Linux~Linux-Installation~Instalador de Linux
Android Flasher~Installation d'Android~Instalador de Android~Installazione Android~Установка Android~Android-Installation~Instalador de Android
Android Partitioning~Partitionnement Android~Particionado de Android~Partizionamento Android~Разметка для Android~Android-Partitionierung~Particionamento Android
Warning: This will partition the SD Card!~Attention : la carte SD va être repartitionnée !~¡Atención: se va a particionar la tarjeta SD!~Attenzione: la scheda SD verrà ripartizionata!~Внимание: будет изменена разметка карты SD!~Achtung: Die SD-Karte wird neu partitioniert!~Atenção: o cartão SD será reparticionado!
Warning: This will partition the eMMC!~Attention : l'eMMC va être repartitionnée !~¡Atención: se va a particionar eMMC!~Attenzione: eMMC verrà ripartizionata!~Внимание: будет изменена разметка eMMC!~Achtung: Die eMMC wird neu partitioniert!~Atenção: a eMMC será reparticionada!
Your files will be backed up and restored!~Vos fichiers seront sauvegardés puis restaurés !~¡Tus archivos se copiarán y restaurarán!~I file verranno salvati e ripristinati!~Ваши файлы будут скопированы и восстановлены!~Ihre Dateien werden gesichert und wiederhergestellt!~Os ficheiros serão copiados e restaurados!
Your files will be wiped!~Vos fichiers seront effacés !~¡Tus archivos se borrarán!~I file verranno cancellati!~Ваши файлы будут удалены!~Ihre Dateien werden gelöscht!~Os ficheiros serão apagados!
Any other partition will be wiped!~Toute autre partition sera effacée !~¡Se borrarán las demás particiones!~Ogni altra partizione verrà cancellata!~Все остальные разделы будут удалены!~Alle anderen Partitionen werden gelöscht!~As outras partições serão apagadas!
Any other partition will be also wiped!~Toute autre partition sera aussi effacée !~¡También se borrarán las demás particiones!~Anche ogni altra partizione verrà cancellata!~Все остальные разделы также будут удалены!~Alle anderen Partitionen werden ebenfalls gelöscht!~As outras partições também serão apagadas!
Use USB UMS to copy them over!~Utilisez USB UMS pour copier vos fichiers !~¡Usa USB UMS para copiar tus archivos!~Usa USB UMS per copiare i file!~Используйте USB UMS для копирования файлов!~Dateien mit USB UMS kopieren!~Use USB UMS para copiar os ficheiros!
The USER partition will also be formatted!~La partition USER sera aussi formatée !~¡También se formateará la partición USER!~Anche la partizione USER verrà formattata!~Раздел USER также будет отформатирован!~Die USER-Partition wird ebenfalls formatiert!~A partição USER também será formatada!
The SD Card files will be backed up automatically!~Les fichiers de la carte SD seront sauvegardés automatiquement !~¡Los archivos de la tarjeta SD se copiarán automáticamente!~I file della scheda SD verranno salvati automaticamente!~Файлы карты SD будут скопированы автоматически!~Die Dateien der SD-Karte werden automatisch gesichert!~Os ficheiros do cartão SD serão copiados automaticamente!
The SD Card cannot be backed up automatically!~La carte SD ne peut pas être sauvegardée automatiquement !~¡No se puede copiar la tarjeta SD automáticamente!~Impossibile salvare automaticamente la scheda SD!~Невозможно автоматически скопировать карту SD!~Die SD-Karte kann nicht automatisch gesichert werden!~Não é possível copiar o cartão SD automaticamente!
You will be asked to back up your files later via UMS.~La sauvegarde des fichiers par UMS vous sera proposée ensuite.~Más tarde se pedirá copiar los archivos mediante UMS.~In seguito verrà chiesto di salvare i file tramite UMS.~Позже будет предложено скопировать файлы через UMS.~Sie werden später zur Dateisicherung über UMS aufgefordert.~Será pedido para copiar os ficheiros por UMS mais tarde.
Analyzing SD card usage. This might take a while...~Analyse de la carte SD. Cela peut prendre du temps...~Analizando la tarjeta SD. Puede tardar un poco...~Analisi della scheda SD. Potrebbe richiedere tempo...~Анализ карты SD. Это может занять некоторое время...~SD-Kartennutzung wird analysiert. Dies kann dauern...~A analisar o cartão SD. Pode demorar algum tempo...
Don't touch the screen!~Ne touchez pas l'écran !~¡No toques la pantalla!~Non toccare lo schermo!~Не касайтесь экрана!~Den Bildschirm nicht berühren!~Não toque no ecrã!
Warning: Only run this if you really have issues!~Attention : n'exécutez ceci qu'en cas de problème !~¡Atención: ejecútalo solo si tienes problemas!~Attenzione: esegui solo in caso di problemi!~Внимание: запускайте только при наличии проблем!~Achtung: Nur bei tatsächlichen Problemen ausführen!~Atenção: execute apenas se houver problemas!
The touchscreen calibration finished!~Le calibrage de l'écran tactile est terminé !~¡La calibración táctil ha terminado!~Calibrazione del touchscreen completata!~Калибровка сенсора завершена!~Touchscreen-Kalibrierung abgeschlossen!~A calibração do ecrã tátil terminou!
The touchscreen calibration failed!~Le calibrage de l'écran tactile a échoué !~¡Falló la calibración táctil!~Calibrazione del touchscreen fallita!~Калибровка сенсора не удалась!~Touchscreen-Kalibrierung fehlgeschlagen!~A calibração do ecrã tátil falhou!
Error mounting SD Card!~Impossible de monter la carte SD !~¡Error al montar la tarjeta SD!~Impossibile montare la scheda SD!~Ошибка подключения карты SD!~SD-Karte konnte nicht eingebunden werden!~Erro ao montar o cartão SD!
No emuMMC found active!~Aucune emuMMC active trouvée !~¡No hay emuMMC activa!~Nessuna emuMMC attiva trovata!~Активная emuMMC не найдена!~Keine aktive emuMMC gefunden!~Nenhuma emuMMC ativa encontrada!
Active emuMMC is not partition based!~L'emuMMC active n'est pas basée sur une partition !~¡La emuMMC activa no está basada en partición!~L'emuMMC attiva non è basata su partizione!~Активная emuMMC не размещена в разделе!~Aktive emuMMC ist nicht partitionsbasiert!~A emuMMC ativa não se baseia numa partição!
Dump package1/2~Exporter package1/2~Exportar package1/2~Esporta package1/2~Экспорт package1/2~package1/2 exportieren~Exportar package1/2
"""

catalog = {}
for lineno, line in enumerate(ROWS.splitlines(), 1):
    if not line.strip():
        continue
    values = [value.replace(r"\n", "\n") for value in line.split("~")]
    if len(values) != 7:
        raise ValueError(f"Line {lineno}: expected 7 values, got {len(values)}")
    key = values[0]
    if key in catalog and catalog[key] != values:
        raise ValueError(f"Conflicting duplicate: {key}")
    catalog[key] = values

def ascii_fr(text):
    return "".join(c for c in unicodedata.normalize("NFD", text)
                   if unicodedata.category(c) != "Mn").replace("œ", "oe").replace("Œ", "OE")

# Existing frontend code uses French without accents. Keep exact source aliases
# alongside canonical English, so the runtime can translate either generation.
for values in list(catalog.values()):
    catalog.setdefault(values[1], values)
    catalog.setdefault(ascii_fr(values[1]), values)

ALIASES = {
    "Save options": "Save Options",
    "Enregistrer les options": "Save Options",
    "Reglages Nyx": "Nyx Settings",
    "Choisir le systeme a lancer sur ta console.": "Choose the system to start on your console.",
    "Consulter les autres profils de demarrage.": "View other boot profiles.",
    "Demarrer, gerer et verifier ta console.": "Start, manage and check your console.",
    "Choisir un systeme  >": "Choose a system  >",
    "Enregistrer & recharger": "Save & Reload",
    "Disposition de l'accueil": "Home layout",
    "Bilan HWTest": "HWTest overview",
    "Memoire / horloges": "Memory / clocks",
    "Memoire DRAM": "DRAM memory",
    "Batterie, charge, tensions, thermique": "Battery, charging, voltages, thermal",
    "Dalle et retroeclairage LCD / OLED": "LCD / OLED panel and backlight",
    "Passe complete puis retour Hekate": "Full pass then return to Hekate",
    "NON EXECUTE": "NOT RUN",
    "Diagnostic - Derniere execution": "Diagnostics - Last run",
    "Execution ou restauration incomplete. ": "Execution or restoration incomplete.",
    "La restauration du tactile a echoue.\nUtilisez les Joy-Con ou rechargez Nyx.": "Touch restoration failed.\nUse the Joy-Con or reload Nyx.",
    "Les observations sans verdict ne constituent pas un test reussi.": "Observations without a verdict do not mean a passed test.",
    "Les commandes sont suspendues pendant les mesures materielles.": "Controls are suspended during hardware measurements.",
    "Un son est emis sur chaque sortie. Verifiez le resultat a l'ecoute.": "A sound is played on each output. Check the result by listening.",
    "HWTest-RCM : 42 controles natifs et un service RAM isole. Choisissez une categorie.": "HWTest-RCM: 42 native checks and an isolated RAM service. Choose a category.",
    "Le service RAM n'a pas pu etre lance (code %d).\nVerifiez bootloader/sys/hwtest_ram.bin et bootloader/update.bin.": "The RAM service could not start (code %d).\nCheck bootloader/sys/hwtest_ram.bin and bootloader/update.bin.",
    "Dernier resultat : %s\n%d / %d motifs, zone prevue : %d MiB sur %d MiB\n%d erreur(s), duree %d s, erreurs MTC : %08X": "Last result: %s\n%d / %d patterns, planned area: %d MiB of %d MiB\n%d error(s), duration %d s, MTC errors: %08X",
    "%d OK  /  %d avert.  /  %d defaut(s)  /  %d erreur(s)  /  %d observation(s)\n%s%s": "%d OK  /  %d warning(s)  /  %d fault(s)  /  %d error(s)  /  %d observation(s)\n%s%s",
    "Nyx sera quitte pour tester la memoire disponible avec les 7 motifs avances.\nLa zone de retour Hekate et les zones reservees restent exclues du test.\nA la fin, Hekate est recharge et le resultat est affiche.\nEn cas de panne materielle ou de memoire instable, le retour peut echouer.": "Nyx will close to test available memory with 7 advanced patterns.\nThe Hekate return area and reserved areas remain excluded.\nAt the end, Hekate reloads and displays the result.\nWith faulty hardware or unstable memory, the return may fail.",
    "Aucun diagnostic execute.\n": "No diagnostics run.",
    "Part 1": "Part 1", "Part 2": "Part 2", "Part 3": "Part 3",
}
for number in range(1, 4):
    english = f"Part {number}"
    catalog[english] = [english, f"Partie {number}", f"Parte {number}", f"Parte {number}", f"Часть {number}", f"Teil {number}", f"Parte {number}"]
for key, canonical in ALIASES.items():
    catalog[key] = catalog[canonical]

# Authored follow-up phrases are stored with the source so a translator can
# update them without editing the Python builder. They never rewrite paths.
probe_extra = ROOT / 'localization_probe_extra.json'
if probe_extra.exists():
    for key, values in json.loads(probe_extra.read_text(encoding='utf-8')).items():
        catalog.setdefault(key, values)
extra = ROOT / 'localization_extra.json'
if extra.exists():
    catalog.update(json.loads(extra.read_text(encoding='utf-8')))
legacy_extra = ROOT / 'localization_legacy_extra.json'
if legacy_extra.exists():
    catalog.update(json.loads(legacy_extra.read_text(encoding='utf-8')))
ram_extra = ROOT / 'localization_ram_extra.json'
if ram_extra.exists():
    catalog.update(json.loads(ram_extra.read_text(encoding='utf-8')))
final_extra = ROOT / 'localization_final_extra.json'
if final_extra.exists():
    catalog.update(json.loads(final_extra.read_text(encoding='utf-8')))
probe_review = ROOT / 'localization_probe_review.json'
if probe_review.exists():
    catalog.update(json.loads(probe_review.read_text(encoding='utf-8')))
nova = ROOT / 'localization_nova.json'
if nova.exists():
    catalog.update(json.loads(nova.read_text(encoding='utf-8')))
nova_pro = ROOT / 'localization_nova_pro.json'
if nova_pro.exists():
    catalog.update(json.loads(nova_pro.read_text(encoding='utf-8')))

nova_v12 = ROOT / 'localization_nova_v12.json'
if nova_v12.exists():
    catalog.update(json.loads(nova_v12.read_text(encoding='utf-8')))

nova_v13 = ROOT / 'localization_nova_v13.json'
if nova_v13.exists():
    catalog.update(json.loads(nova_v13.read_text(encoding='utf-8')))

nova_v15 = ROOT / 'localization_nova_v15.json'
if nova_v15.exists():
    catalog.update(json.loads(nova_v15.read_text(encoding='utf-8')))

# Preserve literal formatting/color controls around fully authored phrases.
from audit_localization_coverage import expand_catalog
composed_count = expand_catalog(catalog, ROOT.parents[1])
print(f'Composed {composed_count} decorated/combined source strings from authored phrases')

FORMAT = re.compile(r"%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcspn]|%)")
for key, values in catalog.items():
    expected = FORMAT.findall(key)
    assert len(values) == 7 and all(isinstance(x, str) and x for x in values), key
    for language, value in enumerate(values):
        if FORMAT.findall(value) != expected:
            raise ValueError(f"Placeholder mismatch for language {language}: {key!r}")
ROOT.mkdir(parents=True, exist_ok=True)
target = ROOT / "localization_catalog.json"
target.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(f"Wrote {target}: {len(catalog)} exact keys, {len(set(tuple(v) for v in catalog.values()))} unique translation groups")

# C string syntax matches JSON string escaping for this UTF-8 catalog. Keep
# actual Unicode in the source; the firmware toolchain compiles UTF-8 bytes.
def c_string(value):
    return json.dumps(value, ensure_ascii=False)

lines = ["/* Generated by work/create_localization_catalog.py. UTF-8; en fr es it ru de pt. */",
         "static const locale_entry locale_catalog[] = {"]
for key, values in sorted(catalog.items()):
    lines.append("    {" + c_string(key) + ", {" + ", ".join(map(c_string, values)) + "}},")
lines.append("};")
pattern_rows=[i for i,(key,_) in enumerate(sorted(catalog.items())) if '%' in key and len(re.findall(r'[A-Za-z]',key))>=5]
assert len(catalog)<65536
lines.append('static const uint16_t locale_pattern_rows[] = {'+', '.join(map(str,pattern_rows))+'};')
first=[sorted(catalog)[i].encode('utf-8')[0] for i in pattern_rows]
starts=[sum(ch<code for ch in first) for code in range(257)]
lines.append('static const uint16_t locale_pattern_start[257] = {'+', '.join(map(str,starts))+'};')
inc = ROOT / "localization_catalog.inc"
inc.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"Wrote {inc}")
