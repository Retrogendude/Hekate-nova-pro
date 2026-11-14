"""Authored translations for the Nova reference, preserving its French copy."""
from pathlib import Path
import json

work=Path(__file__).resolve().parent
root=work/'hekate-6.5.4/nyx/nyx_gui'
rows=r'''
Your console, made simple.~Votre console, simplement.~Tu consola, sin complicaciones.~La tua console, semplicemente.~Ваша консоль. Всё просто.~Deine Konsole. Ganz einfach.~A tua consola, sem complicações.
Welcome to Hekate~Bienvenue dans Hekate~Bienvenido a Hekate~Benvenuto in Hekate~Добро пожаловать в Hekate~Willkommen bei Hekate~Bem-vindo ao Hekate
Start, manage and check.~Démarrer, gérer et vérifier.~Iniciar, gestionar y comprobar.~Avviare, gestire e verificare.~Запуск, управление и проверка.~Starten, verwalten und prüfen.~Iniciar, gerir e verificar.
Tools and maintenance.~Outils et maintenance.~Herramientas y mantenimiento.~Strumenti e manutenzione.~Утилиты и обслуживание.~Werkzeuge und Wartung.~Ferramentas e manutenção.
Storage, maintenance and connections.~Stockage, maintenance et connexions.~Almacenamiento, mantenimiento y conexiones.~Archiviazione, manutenzione e connessioni.~Хранилище, обслуживание и подключения.~Speicher, Wartung und Verbindungen.~Armazenamento, manutenção e ligações.
Console information.~Informations console.~Información de la consola.~Informazioni sulla console.~Информация о консоли.~Konsoleninformationen.~Informações da consola.
Hardware, storage and power.~Matériel, stockage et alimentation.~Hardware, almacenamiento y alimentación.~Hardware, archiviazione e alimentazione.~Оборудование, хранилище и питание.~Hardware, Speicher und Stromversorgung.~Hardware, armazenamento e alimentação.
Hardware diagnostics.~Diagnostic matériel.~Diagnóstico de hardware.~Diagnostica hardware.~Диагностика оборудования.~Hardwarediagnose.~Diagnóstico de hardware.
42 native checks and a separate RAM service.~42 contrôles natifs et un service RAM isolé.~42 comprobaciones nativas y un servicio RAM separado.~42 controlli nativi e un servizio RAM separato.~42 встроенные проверки и отдельный тест ОЗУ.~42 native Prüfungen und ein separater RAM-Dienst.~42 verificações nativas e um serviço RAM separado.
Appearance and options.~Apparence et options.~Apariencia y opciones.~Aspetto e opzioni.~Оформление и параметры.~Darstellung und Optionen.~Aspeto e opções.
Reading, options and selection effects.~Lecture, options et effets de sélection.~Lectura, opciones y efectos de selección.~Lettura, opzioni ed effetti di selezione.~Читаемость, параметры и эффекты выбора.~Lesbarkeit, Optionen und Auswahleffekte.~Leitura, opções e efeitos de seleção.
Info~Infos~Información~Info~Сведения~Infos~Informações
Settings~Réglages~Ajustes~Impostazioni~Настройки~Einstellungen~Definições
8 tools~8 outils~8 herramientas~8 strumenti~8 утилит~8 Werkzeuge~8 ferramentas
42 checks~42 contrôles~42 comprobaciones~42 controlli~42 проверки~42 Prüfungen~42 verificações
Comfort and clarity~Confort et clarté~Comodidad y claridad~Comfort e chiarezza~Удобство и ясность~Komfort und Klarheit~Conforto e clareza
Find your other boot profiles.~Retrouver les autres profils de démarrage.~Encontrar los demás perfiles de arranque.~Ritrova gli altri profili di avvio.~Другие профили загрузки.~Weitere Startprofile finden.~Encontrar os outros perfis de arranque.
Open a tool from the microSD card.~Ouvrir un outil depuis la carte microSD.~Abrir una herramienta desde la tarjeta microSD.~Apri uno strumento dalla scheda microSD.~Открыть утилиту с карты microSD.~Ein Werkzeug von der microSD-Karte öffnen.~Abrir uma ferramenta do cartão microSD.
Manage the system copy on the SD card.~Gérer la copie du système sur la carte SD.~Gestionar la copia del sistema en la tarjeta SD.~Gestisci la copia del sistema sulla scheda SD.~Управление копией системы на SD-карте.~Systemkopie auf der SD-Karte verwalten.~Gerir a cópia do sistema no cartão SD.
Explore the hardware checks.~Explorer les contrôles matériels.~Explorar las comprobaciones de hardware.~Esplora i controlli hardware.~Обзор проверок оборудования.~Hardwareprüfungen erkunden.~Explorar as verificações de hardware.
Choose a system~Choisir un système~Elegir un sistema~Scegli un sistema~Выбрать систему~System auswählen~Escolher um sistema
eMMC backup~Sauvegarde eMMC~Copia de eMMC~Backup eMMC~Резервная копия eMMC~eMMC sichern~Cópia da eMMC
Create a storage backup.~Créer une sauvegarde du stockage.~Crear una copia del almacenamiento.~Crea un backup dell'archiviazione.~Создать резервную копию хранилища.~Eine Sicherung des Speichers erstellen.~Criar uma cópia do armazenamento.
eMMC restore~Restauration eMMC~Restauración de eMMC~Ripristino eMMC~Восстановление eMMC~eMMC wiederherstellen~Restauro da eMMC
Restore an existing backup.~Restaurer une sauvegarde existante.~Restaurar una copia existente.~Ripristina un backup esistente.~Восстановить имеющуюся копию.~Eine vorhandene Sicherung wiederherstellen.~Restaurar uma cópia existente.
SD partitions~Partitions SD~Particiones SD~Partizioni SD~Разделы SD~SD-Partitionen~Partições SD
Prepare the SD card partitions.~Préparer les partitions de la carte SD.~Preparar las particiones de la tarjeta SD.~Prepara le partizioni della scheda SD.~Подготовить разделы SD-карты.~Partitionen der SD-Karte vorbereiten.~Preparar as partições do cartão SD.
Access USB storage functions.~Accéder aux fonctions de stockage USB.~Acceder a las funciones de almacenamiento USB.~Accedi alle funzioni di archiviazione USB.~Функции USB-накопителя.~Auf USB-Speicherfunktionen zugreifen.~Aceder às funções de armazenamento USB.
Archive bit~Bit d’archive~Bit de archivo~Bit archivio~Атрибут архива~Archivbit~Atributo de arquivo
Repair file attributes.~Réparer les attributs des fichiers.~Reparar los atributos de los archivos.~Correggi gli attributi dei file.~Исправить атрибуты файлов.~Dateiattribute reparieren.~Reparar os atributos dos ficheiros.
Adjust touch calibration.~Ajuster la calibration du tactile.~Ajustar la calibración táctil.~Regola la calibrazione del touch.~Настроить калибровку сенсорного экрана.~Touchscreen-Kalibrierung anpassen.~Ajustar a calibração tátil.
View and change AutoRCM mode.~Consulter et modifier le mode AutoRCM.~Consultar y cambiar el modo AutoRCM.~Visualizza e modifica la modalità AutoRCM.~Просмотр и изменение режима AutoRCM.~AutoRCM-Modus anzeigen und ändern.~Consultar e alterar o modo AutoRCM.
Extract system components.~Extraire les éléments du système.~Extraer los componentes del sistema.~Estrai i componenti del sistema.~Извлечь компоненты системы.~Systemkomponenten extrahieren.~Extrair os componentes do sistema.
Hardware and fuses~Matériel et fusibles~Hardware y fusibles~Hardware e fusibili~Оборудование и фьюзы~Hardware und Fuses~Hardware e fusíveis
Console identity and memory.~Identité de la console et mémoire.~Identidad de la consola y memoria.~Identità della console e memoria.~Идентификация консоли и память.~Konsolenkennung und Speicher.~Identidade da consola e memória.
Internal storage information.~Informations sur le stockage interne.~Información del almacenamiento interno.~Informazioni sull'archiviazione interna.~Сведения о внутреннем хранилище.~Informationen zum internen Speicher.~Informações do armazenamento interno.
Memory card information.~Informations sur la carte mémoire.~Información de la tarjeta de memoria.~Informazioni sulla scheda di memoria.~Сведения о карте памяти.~Informationen zur Speicherkarte.~Informações do cartão de memória.
Charge, temperatures and power.~Charge, températures et alimentation.~Carga, temperaturas y alimentación.~Carica, temperature e alimentazione.~Заряд, температуры и питание.~Ladung, Temperaturen und Stromversorgung.~Carga, temperaturas e alimentação.
Information and extraction.~Informations et extraction.~Información y extracción.~Informazioni ed estrazione.~Сведения и извлечение.~Informationen und Extraktion.~Informações e extração.
HDCP key data.~Données des clés HDCP.~Datos de claves HDCP.~Dati delle chiavi HDCP.~Данные ключей HDCP.~HDCP-Schlüsseldaten.~Dados das chaves HDCP.
Tool on the SD card.~Outil présent sur la carte SD.~Herramienta en la tarjeta SD.~Strumento sulla scheda SD.~Утилита на SD-карте.~Werkzeug auf der SD-Karte.~Ferramenta no cartão SD.
Group overview.~Vue d’ensemble des groupes.~Vista general de los grupos.~Panoramica dei gruppi.~Обзор групп.~Übersicht der Gruppen.~Visão geral dos grupos.
SoC, fuses and KFUSE.~SoC, fusibles et KFUSE.~SoC, fusibles y KFUSE.~SoC, fusibili e KFUSE.~SoC, фьюзы и KFUSE.~SoC, Fuses und KFUSE.~SoC, fusíveis e KFUSE.
Battery, charging and temperatures.~Batterie, charge et températures.~Batería, carga y temperaturas.~Batteria, carica e temperature.~Батарея, зарядка и температуры.~Akku, Ladung und Temperaturen.~Bateria, carga e temperaturas.
Memory~Mémoire~Memoria~Memoria~Память~Speicher~Memória
DRAM and clocks.~DRAM et horloges.~DRAM y relojes.~DRAM e clock.~DRAM и частоты.~DRAM und Takte.~DRAM e frequências.
SD, eMMC and reader.~SD, eMMC et lecteur.~SD, eMMC y lector.~SD, eMMC e lettore.~SD, eMMC и считыватель.~SD, eMMC und Leser.~SD, eMMC e leitor.
Bluetooth and Wi-Fi.~Bluetooth et Wi-Fi.~Bluetooth y Wi-Fi.~Bluetooth e Wi-Fi.~Bluetooth и Wi-Fi.~Bluetooth und WLAN.~Bluetooth e Wi-Fi.
Codec and audio clocks.~Codec et horloges audio.~Códec y relojes de audio.~Codec e clock audio.~Кодек и тактовые сигналы аудио.~Codec und Audiotakte.~Codec e frequências de áudio.
Speakers and headphone jack.~Haut-parleurs et prise casque.~Altavoces y toma de auriculares.~Altoparlanti e jack cuffie.~Динамики и разъём наушников.~Lautsprecher und Kopfhöreranschluss.~Altifalantes e entrada de auscultadores.
Panel and backlight.~Dalle et rétroéclairage.~Panel y retroiluminación.~Pannello e retroilluminazione.~Панель и подсветка.~Display und Hintergrundbeleuchtung.~Painel e retroiluminação.
Touch, Joy-Con and buttons.~Tactile, Joy-Con et boutons.~Pantalla táctil, Joy-Con y botones.~Touch, Joy-Con e pulsanti.~Сенсорный экран, Joy-Con и кнопки.~Touchscreen, Joy-Con und Tasten.~Ecrã tátil, Joy-Con e botões.
Low-level~Bas niveau~Bajo nivel~Basso livello~Низкий уровень~Systemnah~Baixo nível
GPIO, UART and boot state.~GPIO, UART et démarrage.~GPIO, UART y arranque.~GPIO, UART e avvio.~GPIO, UART и загрузка.~GPIO, UART und Startzustand.~GPIO, UART e arranque.
Separate test, return to Hekate.~Test séparé, retour à Hekate.~Prueba separada, vuelta a Hekate.~Test separato, ritorno a Hekate.~Отдельный тест, возврат в Hekate.~Separater Test, Rückkehr zu Hekate.~Teste separado, regresso ao Hekate.
System options~Options système~Opciones del sistema~Opzioni di sistema~Параметры системы~Systemoptionen~Opções do sistema
Appearance and reading~Apparence et lecture~Apariencia y lectura~Aspetto e lettura~Оформление и читаемость~Darstellung und Lesbarkeit~Aspeto e leitura
Game-card reader protection.~Protection du lecteur de cartes.~Protección del lector de cartuchos.~Protezione del lettore di schede gioco.~Защита считывателя игровых карт.~Schutz des Spielkartenlesers.~Proteção do leitor de cartões de jogo.
HOS shutdown~Extinction HOS~Apagado de HOS~Spegnimento HOS~Выключение HOS~HOS ausschalten~Encerramento HOS
Automatic power-off after HOS.~Arrêt automatique après HOS.~Apagado automático después de HOS.~Spegnimento automatico dopo HOS.~Автовыключение после HOS.~Automatisches Ausschalten nach HOS.~Encerramento automático após HOS.
Backup verification~Vérification des sauvegardes~Verificación de copias~Verifica dei backup~Проверка резервных копий~Sicherungen prüfen~Verificação de cópias
Check written data.~Contrôler les données écrites.~Comprobar los datos escritos.~Controlla i dati scritti.~Проверять записанные данные.~Geschriebene Daten prüfen.~Verificar os dados escritos.
Selection effects~Effets de sélection~Efectos de selección~Effetti di selezione~Эффекты выбора~Auswahleffekte~Efeitos de seleção
Highlight the active icon.~Souligner l’icône active.~Resaltar el icono activo.~Evidenzia l'icona attiva.~Подсветка выбранного значка.~Das aktive Symbol hervorheben.~Realçar o ícone ativo.
Icon outline~Contour des icônes~Contorno de iconos~Contorno delle icone~Контур значков~Symbolumrandung~Contorno dos ícones
Halo~Halo~Halo~Alone~Свечение~Leuchten~Halo
Outline~Liseré~Borde~Bordo~Контур~Umrandung~Contorno
Active~Actif~Activo~Attivo~Вкл.~Aktiv~Ativo
Inactive~Inactif~Inactivo~Inattivo~Выкл.~Inaktiv~Inativo
Settings saved.~Réglages enregistrés.~Ajustes guardados.~Impostazioni salvate.~Настройки сохранены.~Einstellungen gespeichert.~Definições guardadas.
No tests started.~Aucun test lancé.~Ninguna prueba iniciada.~Nessun test avviato.~Тесты не запущены.~Keine Tests gestartet.~Nenhum teste iniciado.
Could not save settings. Check the SD card and retry.~Échec de l’enregistrement. Vérifiez la carte SD et réessayez.~No se pudieron guardar los ajustes. Comprueba la tarjeta SD y vuelve a intentarlo.~Impossibile salvare le impostazioni. Controlla la scheda SD e riprova.~Не удалось сохранить настройки. Проверьте SD-карту и повторите попытку.~Speichern fehlgeschlagen. SD-Karte prüfen und erneut versuchen.~Não foi possível guardar as definições. Verifique o cartão SD e tente novamente.
Boot options were saved. Nyx settings could not be saved. Check the SD card and retry.~Les options de démarrage sont enregistrées. Échec de l’enregistrement des réglages Nyx. Vérifiez la carte SD et réessayez.~Se guardaron las opciones de arranque, pero no los ajustes de Nyx. Comprueba la tarjeta SD y vuelve a intentarlo.~Opzioni di avvio salvate, ma impostazioni Nyx non salvate. Controlla la scheda SD e riprova.~Параметры загрузки сохранены. Не удалось сохранить настройки Nyx. Проверьте SD-карту и повторите попытку.~Startoptionen gespeichert. Nyx-Einstellungen konnten nicht gespeichert werden. SD-Karte prüfen und erneut versuchen.~Opções de arranque guardadas. Não foi possível guardar as definições do Nyx. Verifique o cartão SD e tente novamente.
'''
catalog={}
for line in rows.strip().splitlines():
    values=line.split('~');assert len(values)==7,line
    catalog[values[0]]=values;catalog[values[1]]=values
reference=json.loads((work/'nova-reference.json').read_text())
existing=json.loads((root/'localization_catalog.json').read_text(encoding='utf-8'))
texts={x for collection in reference['items'].values() for item in collection for x in item[1:3]}
texts.update(x for pair in reference['titles'].values() for x in pair)
texts.update(row[1] for row in reference['navigation'])
proper={'AutoRCM','BootROM','KFuses','Lockpick','Package1 / 2','eMMC','emuMMC','microSD'}
for text in sorted(texts):
    if text in catalog:continue
    if text in proper:catalog[text]=[text]*7
    else:
        assert text in existing, 'Missing authored Nova translation: '+text
        values=existing[text].copy();values[1]=text;catalog[text]=values
(root/'localization_nova.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Authored Nova catalog:',len(catalog),'keys; all reference captions covered in seven languages.')
