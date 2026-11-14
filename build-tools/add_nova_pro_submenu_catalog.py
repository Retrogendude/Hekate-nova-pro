from pathlib import Path
import json
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui'
rows=r'''Keep BOOT0/BOOT1 together with RAW GPP.~Partitions BOOT0/BOOT1, à conserver avec RAW GPP.~Conserva BOOT0/BOOT1 junto con RAW GPP.~Conserva BOOT0/BOOT1 insieme a RAW GPP.~Храните BOOT0/BOOT1 вместе с RAW GPP.~BOOT0/BOOT1 zusammen mit RAW GPP aufbewahren.~Guarde BOOT0/BOOT1 juntamente com RAW GPP.
Full storage: CAL0, SYSTEM, USER and boot files.~Stockage complet : CAL0, SYSTEM, USER et fichiers de démarrage.~Almacenamiento completo: CAL0, SYSTEM, USER y archivos de arranque.~Memoria completa: CAL0, SYSTEM, USER e file di avvio.~Полное хранилище: CAL0, SYSTEM, USER и загрузочные файлы.~Gesamter Speicher: CAL0, SYSTEM, USER und Startdateien.~Armazenamento completo: CAL0, SYSTEM, USER e ficheiros de arranque.
System partitions in RAW GPP.~Partitions système de RAW GPP.~Particiones del sistema en RAW GPP.~Partizioni di sistema in RAW GPP.~Системные разделы RAW GPP.~Systempartitionen von RAW GPP.~Partições do sistema em RAW GPP.
USER partition. This copy alone is incomplete.~Partition USER. Cette copie seule est incomplète.~Partición USER. Esta copia por sí sola está incompleta.~Partizione USER. Questa copia da sola è incompleta.~Раздел USER. Только эта копия не является полной.~USER-Partition. Diese Kopie allein ist unvollständig.~Partição USER. Esta cópia por si só está incompleta.
Use SD emuMMC instead of the physical eMMC.~Utiliser emuMMC sur la carte SD à la place de l’eMMC physique.~Usar emuMMC en la tarjeta SD en lugar de la eMMC física.~Usa emuMMC sulla scheda SD al posto dell’eMMC fisica.~Использовать emuMMC на SD-карте вместо физической eMMC.~emuMMC auf der SD-Karte statt der physischen eMMC verwenden.~Usar emuMMC no cartão SD em vez da eMMC física.
eMMC / emuMMC: protected read-only access by default.~eMMC / emuMMC : accès protégé en lecture seule par défaut.~eMMC / emuMMC: acceso protegido de solo lectura por defecto.~eMMC / emuMMC: accesso protetto in sola lettura predefinito.~eMMC / emuMMC: по умолчанию защищённый доступ только для чтения.~eMMC / emuMMC: standardmäßig geschützter Lesezugriff.~eMMC / emuMMC: acesso protegido só de leitura por predefinição.
PC or phone connection: read and write.~Connexion au PC ou téléphone : lecture et écriture.~Conexión a PC o teléfono: lectura y escritura.~Collegamento a PC o telefono: lettura e scrittura.~Подключение к ПК или телефону: чтение и запись.~Verbindung mit PC oder Telefon: Lesen und Schreiben.~Ligação ao PC ou telefone: leitura e escrita.
Connect both Joy-Con to use the console as a USB gamepad.~Connectez les deux Joy-Con pour utiliser la console comme manette USB.~Conecta ambos Joy-Con para usar la consola como mando USB.~Collega entrambi i Joy-Con per usare la console come controller USB.~Подключите оба Joy-Con для использования консоли как USB-геймпада.~Beide Joy-Con verbinden, um die Konsole als USB-Gamepad zu nutzen.~Ligue ambos os Joy-Con para usar a consola como comando USB.
Mount this partition over USB.~Monter cette partition par USB.~Montar esta partición por USB.~Monta questa partizione tramite USB.~Подключить этот раздел по USB.~Diese Partition über USB einbinden.~Montar esta partição por USB.
Type: SD partition\nSector: 0x%08X\nNintendo folder: %s~Type : partition SD\nSecteur : 0x%08X\nDossier Nintendo : %s~Tipo: partición SD\nSector: 0x%08X\nCarpeta Nintendo: %s~Tipo: partizione SD\nSettore: 0x%08X\nCartella Nintendo: %s~Тип: раздел SD\nСектор: 0x%08X\nПапка Nintendo: %s~Typ: SD-Partition\nSektor: 0x%08X\nNintendo-Ordner: %s~Tipo: partição SD\nSetor: 0x%08X\nPasta Nintendo: %s
Type: SD files\nFolder: %s\nNintendo folder: %s~Type : fichiers SD\nDossier : %s\nDossier Nintendo : %s~Tipo: archivos SD\nCarpeta: %s\nCarpeta Nintendo: %s~Tipo: file SD\nCartella: %s\nCartella Nintendo: %s~Тип: файлы SD\nПапка: %s\nПапка Nintendo: %s~Typ: SD-Dateien\nOrdner: %s\nNintendo-Ordner: %s~Tipo: ficheiros SD\nPasta: %s\nPasta Nintendo: %s
Choose an emuMMC image on the SD card.~Choisir une image emuMMC sur la carte SD.~Elegir una imagen emuMMC en la tarjeta SD.~Scegli un’immagine emuMMC sulla scheda SD.~Выбрать образ emuMMC на SD-карте.~Ein emuMMC-Abbild auf der SD-Karte wählen.~Escolher uma imagem emuMMC no cartão SD.
Create a copy as files or in an SD partition.~Créer une copie sous forme de fichiers ou dans une partition SD.~Crear una copia como archivos o en una partición SD.~Crea una copia come file o in una partizione SD.~Создать копию в файлах или разделе SD.~Eine Kopie als Dateien oder in einer SD-Partition erstellen.~Criar uma cópia em ficheiros ou numa partição SD.
Migrate a backup or repair an existing emuMMC partition.~Migrer une sauvegarde ou réparer une partition emuMMC existante.~Migrar una copia de seguridad o reparar una partición emuMMC existente.~Migra un backup o ripara una partizione emuMMC esistente.~Перенести резервную копию или восстановить раздел emuMMC.~Ein Backup übertragen oder eine vorhandene emuMMC-Partition reparieren.~Migrar uma cópia de segurança ou reparar uma partição emuMMC existente.
General assessment~Bilan général~Evaluación general~Valutazione generale~Общая оценка~Gesamtprüfung~Avaliação geral
Fuses~Fusibles~Fusibles~Fusibili~Предохранители~Sicherungen~Fusíveis
CPU regulator~Régulateur CPU~Regulador de CPU~Regolatore CPU~Регулятор CPU~CPU-Spannungsregler~Regulador da CPU
PMIC inputs/outputs~Entrées/sorties PMIC~Entradas/salidas PMIC~Ingressi/uscite PMIC~Входы/выходы PMIC~PMIC-Ein-/Ausgänge~Entradas/saídas PMIC
Power supplies~Alimentations~Fuentes de alimentación~Alimentazioni~Источники питания~Spannungsversorgung~Fontes de alimentação
5 V supply~Tension 5 V~Tensión de 5 V~Tensione 5 V~Питание 5 В~5-V-Versorgung~Tensão de 5 V
Battery~Batterie~Batería~Batteria~Батарея~Akku~Bateria
Charger~Chargeur~Cargador~Caricatore~Зарядное устройство~Laderegler~Carregador
Temperatures~Températures~Temperaturas~Temperature~Температуры~Temperaturen~Temperaturas
Fan~Ventilateur~Ventilador~Ventola~Вентилятор~Lüfter~Ventoinha
DRAM memory~Mémoire DRAM~Memoria DRAM~Memoria DRAM~Память DRAM~DRAM-Speicher~Memória DRAM
Clocks~Horloges~Relojes~Clock~Тактовые частоты~Taktraten~Relógios
Storage~Stockage~Almacenamiento~Memoria~Хранилище~Speicher~Armazenamento
Serial numbers~Numéros de série~Números de serie~Numeri di serie~Серийные номера~Seriennummern~Números de série
eMMC health~Santé eMMC~Estado de eMMC~Stato eMMC~Состояние eMMC~eMMC-Zustand~Estado da eMMC
GPT table~Table GPT~Tabla GPT~Tabella GPT~Таблица GPT~GPT-Tabelle~Tabela GPT
BOOT0 startup~Démarrage BOOT0~Arranque de BOOT0~Avvio BOOT0~Загрузка BOOT0~BOOT0-Start~Arranque de BOOT0
SD card contents~Contenu de la carte SD~Contenido de la tarjeta SD~Contenuto della scheda SD~Содержимое SD-карты~Inhalt der SD-Karte~Conteúdo do cartão SD
Storage errors~Erreurs de stockage~Errores de almacenamiento~Errori di memoria~Ошибки хранилища~Speicherfehler~Erros de armazenamento
Game-card reader~Lecteur de jeux~Lector de juegos~Lettore di giochi~Считыватель картриджей~Spielkartenleser~Leitor de jogos
Wi-Fi connection~Connexion Wi-Fi~Conexión Wi-Fi~Connessione Wi-Fi~Подключение Wi-Fi~WLAN-Verbindung~Ligação Wi-Fi
Audio codec~Codec audio~Códec de audio~Codec audio~Аудиокодек~Audio-Codec~Codec de áudio
Audio clocks~Horloges audio~Relojes de audio~Clock audio~Тактовые частоты аудио~Audiotaktraten~Relógios de áudio
Sound test~Test sonore~Prueba de sonido~Test audio~Проверка звука~Klangtest~Teste de som
Display panel~Dalle écran~Panel de pantalla~Pannello schermo~Панель дисплея~Displaypanel~Painel do ecrã
Backlight~Rétroéclairage~Retroiluminación~Retroilluminazione~Подсветка~Hintergrundbeleuchtung~Retroiluminação
Touchscreen~Écran tactile~Pantalla táctil~Touchscreen~Сенсорный экран~Touchscreen~Ecrã tátil
Light sensor~Capteur de luminosité~Sensor de luz~Sensore di luminosità~Датчик освещённости~Helligkeitssensor~Sensor de luminosidade
Buttons~Boutons~Botones~Pulsanti~Кнопки~Tasten~Botões
Reset state~État de réinitialisation~Estado de reinicio~Stato di riavvio~Состояние сброса~Reset-Zustand~Estado de reinicialização
PMC registers~Registres PMC~Registros PMC~Registri PMC~Регистры PMC~PMC-Register~Registos PMC'''
path=root/'localization_nova_pro.json';catalog=json.loads(path.read_text(encoding='utf-8'))
for row in rows.splitlines():
    values=[x.replace('\\n','\n')for x in row.split('~')];assert len(values)==7,row
    catalog[values[0]]=values;catalog[values[1]]=values
path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(f'Nova Pro catalog now contains {len(catalog)} authored keys.')
