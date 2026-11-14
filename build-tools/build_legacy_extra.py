"""Human-authored translations for console information and partition tools.

Rows contain whole UI phrases or complete report field labels. Composition
only adds the original color/layout/format decorations around those phrases.
Product identifiers, paths, sizes, register names and instruction mnemonics
are deliberately preserved.
"""
import json
import re
from pathlib import Path
WORK=Path(__file__).resolve().parent
ROOT=WORK/'hekate-6.5.4/nyx/nyx_gui'
ROWS=r'''
Status:~État :~Estado:~Stato:~Состояние:~Status:~Estado:
Error:~Erreur :~Error:~Errore:~Ошибка:~Fehler:~Erro:
Warning:~Attention :~Advertencia:~Attenzione:~Внимание:~Warnung:~Aviso:
Success:~Réussite :~Éxito:~Operazione riuscita:~Успешно:~Erfolg:~Sucesso:
Dump~Exporter~Exportar~Esporta~Сохранить~Sichern~Exportar
- Init~- Initialisation~- Inicialización~- Inizializzazione~- Инициализация~- Initialisierung~- Inicialização
- Low~- Faible~- Bajo~- Basso~- Низкий~- Niedrig~- Baixo
Failed to dump to~Échec de l'export vers~No se pudo volcar en~Esportazione non riuscita in~Не удалось сохранить в~Speichern fehlgeschlagen unter~Falha ao exportar para
CAL0 Info~Informations CAL0~Información CAL0~Informazioni CAL0~Сведения CAL0~CAL0-Informationen~Informações CAL0
CAL0 Version:~Version CAL0 :~Versión CAL0:~Versione CAL0:~Версия CAL0:~CAL0-Version:~Versão CAL0:
Update Count:~Nombre de mises à jour :~Número de actualizaciones:~Numero aggiornamenti:~Число обновлений:~Aktualisierungen:~Número de atualizações:
Serial Number:~Numéro de série :~Número de serie:~Numero di serie:~Серийный номер:~Seriennummer:~Número de série:
WLAN MAC:~MAC Wi-Fi :~MAC Wi-Fi:~MAC Wi-Fi:~MAC Wi-Fi:~WLAN-MAC:~MAC Wi-Fi:
Bluetooth MAC:~MAC Bluetooth :~MAC Bluetooth:~MAC Bluetooth:~MAC Bluetooth:~Bluetooth-MAC:~MAC Bluetooth:
Battery LOT:~Lot de batterie :~Lote de batería:~Lotto batteria:~Партия батареи:~Batteriecharge:~Lote da bateria:
LCD Vendor:~Fabricant LCD :~Fabricante LCD:~Produttore LCD:~Производитель LCD:~LCD-Hersteller:~Fabricante LCD:
Touch Vendor:~Fabricant tactile :~Fabricante táctil:~Produttore touch:~Производитель сенсора:~Touch-Hersteller:~Fabricante tátil:
IMU Type/Mount:~Type/montage IMU :~Tipo/montaje IMU:~Tipo/montaggio IMU:~Тип/установка IMU:~IMU-Typ/Montage:~Tipo/montagem IMU:
Stick L/R Type:~Type des sticks G/D :~Tipo de palancas I/D:~Tipo stick S/D:~Тип стиков Л/П:~Sticktyp L/R:~Tipo de manípulos E/D:
SHA256 Hash Match:~Correspondance SHA256 :~Coincidencia SHA256:~Corrispondenza SHA256:~Совпадение SHA256:~SHA256 stimmt überein:~Correspondência SHA256:
Wafer~Plaquette~Oblea~Wafer~Пластина~Wafer~Lâmina
SoC:~SoC :~SoC:~SoC:~SoC:~SoC:~SoC:
SKU:~SKU :~SKU:~SKU:~SKU:~SKU:~SKU:
DRAM ID:~Identifiant DRAM :~Identificador DRAM:~Identificativo DRAM:~Идентификатор DRAM:~DRAM-Kennung:~Identificador DRAM:
Burnt Fuses (ODM 7/6):~Fusibles gravés (ODM 7/6) :~Fusibles quemados (ODM 7/6):~Fusibili bruciati (ODM 7/6):~Прожжённые предохранители (ODM 7/6):~Gebrannte Sicherungen (ODM 7/6):~Fusíveis gravados (ODM 7/6):
ODM Fields (4/6/7):~Champs ODM (4/6/7) :~Campos ODM (4/6/7):~Campi ODM (4/6/7):~Поля ODM (4/6/7):~ODM-Felder (4/6/7):~Campos ODM (4/6/7):
Secure Boot Key (SBK):~Clé de démarrage sécurisé (SBK) :~Clave de arranque seguro (SBK):~Chiave di avvio sicuro (SBK):~Ключ безопасной загрузки (SBK):~Sicherer Startschlüssel (SBK):~Chave de arranque seguro (SBK):
Device Key (DK):~Clé de l'appareil (DK) :~Clave del dispositivo (DK):~Chiave del dispositivo (DK):~Ключ устройства (DK):~Geräteschlüssel (DK):~Chave do dispositivo (DK):
Public Key (PK SHA256):~Clé publique (PK SHA256) :~Clave pública (PK SHA256):~Chiave pubblica (PK SHA256):~Открытый ключ (PK SHA256):~Öffentlicher Schlüssel (PK SHA256):~Chave pública (PK SHA256):
HOS Keygen Revision:~Révision de génération des clés HOS :~Revisión de generación de claves HOS:~Revisione generazione chiavi HOS:~Ревизия генерации ключей HOS:~HOS-Schlüsselerzeugungsrevision:~Revisão da geração de chaves HOS:
USB Controller (BROM):~Contrôleur USB (BROM) :~Controlador USB (BROM):~Controller USB (BROM):~Контроллер USB (BROM):~USB-Controller (BROM):~Controlador USB (BROM):
Final Test Revision:~Révision du test final :~Revisión de prueba final:~Revisione test finale:~Ревизия итогового теста:~Endprüfungsrevision:~Revisão do teste final:
Chip Probing Revision:~Révision de test de la puce :~Revisión de sondeo del chip:~Revisione verifica chip:~Ревизия проверки чипа:~Chip-Prüfungsrevision:~Revisão da sondagem do chip:
BootROM Revision:~Révision BootROM :~Revisión BootROM:~Revisione BootROM:~Ревизия BootROM:~BootROM-Revision:~Revisão BootROM:
CPU/GPU/SoC Speedo:~Speedo CPU/GPU/SoC :~Speedo CPU/GPU/SoC:~Speedo CPU/GPU/SoC:~Speedo CPU/GPU/SoC:~CPU/GPU/SoC-Speedo:~Speedo CPU/GPU/SoC:
CPU/GPU/SoC IDDQ:~IDDQ CPU/GPU/SoC :~IDDQ CPU/GPU/SoC:~IDDQ CPU/GPU/SoC:~IDDQ CPU/GPU/SoC:~CPU/GPU/SoC-IDDQ:~IDDQ CPU/GPU/SoC:
CPU Speedo 1:~Speedo CPU 1 :~Speedo CPU 1:~Speedo CPU 1:~Speedo CPU 1:~CPU-Speedo 1:~Speedo CPU 1:
SoC Speedo 2:~Speedo SoC 2 :~Speedo SoC 2:~Speedo SoC 2:~Speedo SoC 2:~SoC-Speedo 2:~Speedo SoC 2:
Product Code:~Code produit :~Código de producto:~Codice prodotto:~Код продукта:~Produktcode:~Código do produto:
Vendor Code:~Code fabricant :~Código de fabricante:~Codice produttore:~Код производителя:~Herstellercode:~Código do fabricante:
FAB/LOT Code:~Code usine/lot :~Código de fábrica/lote:~Codice fabbrica/lotto:~Код фабрики/партии:~Fabrik-/Chargencode:~Código de fábrica/lote:
Wafer ID:~Identifiant de plaquette :~Identificador de oblea:~Identificativo wafer:~Идентификатор пластины:~Wafer-Kennung:~Identificador da lâmina:
X Coordinate:~Coordonnée X :~Coordenada X:~Coordinata X:~Координата X:~X-Koordinate:~Coordenada X:
Y Coordinate:~Coordonnée Y :~Coordenada Y:~Coordinata Y:~Координата Y:~Y-Koordinate:~Coordenada Y:
Uptime:~Durée de fonctionnement :~Tiempo de actividad:~Tempo di attività:~Время работы:~Betriebszeit:~Tempo de atividade:
Forced DRAM Config 8GB~Configuration DRAM forcée à 8 Go~Configuración DRAM forzada a 8 GB~Configurazione DRAM forzata a 8 GB~Принудительная настройка DRAM 8 ГБ~Erzwungene DRAM-Konfiguration 8 GB~Configuração DRAM forçada para 8 GB
Overburnt~Trop de fusibles gravés~Demasiados fusibles quemados~Troppi fusibili bruciati~Избыточно прожжены~Zu viele Sicherungen gebrannt~Fusíveis gravados em excesso
Dev~Développement~Desarrollo~Sviluppo~Разработка~Entwicklung~Desenvolvimento
Retail~Commercial~Comercial~Commerciale~Розница~Handelsversion~Comercial
Vendor:~Fabricant :~Fabricante:~Produttore:~Производитель:~Hersteller:~Fabricante:
Rev ID:~Identifiant de révision :~Identificador de revisión:~Identificativo revisione:~Идентификатор ревизии:~Revisionskennung:~Identificador de revisão:
Density:~Capacité :~Densidad:~Densità:~Плотность:~Dichte:~Densidade:
OEM Clone 6.2"~Clone OEM 6,2"~Clon OEM 6,2"~Clone OEM 6,2"~OEM-клон 6,2"~OEM-Nachbau 6,2"~Clone OEM 6,2"
OEM Clone 5.5"~Clone OEM 5,5"~Clon OEM 5,5"~Clone OEM 5,5"~OEM-клон 5,5"~OEM-Nachbau 5,5"~Clone OEM 5,5"
OEM Clone~Clone OEM~Clon OEM~Clone OEM~OEM-клон~OEM-Nachbau~Clone OEM
LCD OEM Clone 7"~Clone LCD OEM 7"~Clon LCD OEM 7"~Clone LCD OEM 7"~OEM-клон LCD 7"~OEM-LCD-Nachbau 7"~Clone LCD OEM 7"
OEM Clone TSP~Clone TSP OEM~Clon TSP OEM~Clone TSP OEM~OEM-клон TSP~OEM-TSP-Nachbau~Clone TSP OEM
Clone~Clone~Clon~Clone~Клон~Nachbau~Clone
Contact me~Me contacter~Contáctame~Contattami~Связаться со мной~Kontakt aufnehmen~Contactar-me
FTB ver:~Version FTB :~Versión FTB:~Versione FTB:~Версия FTB:~FTB-Version:~Versão FTB:
FW rev:~Révision du micrologiciel :~Revisión del firmware:~Revisione firmware:~Ревизия прошивки:~Firmware-Revision:~Revisão do firmware:
HW rev:~Révision matérielle :~Revisión de hardware:~Revisione hardware:~Ревизия оборудования:~Hardware-Revision:~Revisão do hardware:
Paired~Appairé~Vinculado~Associato~Сопряжено~Gekoppelt~Emparelhado
This unit is exploitable~Cette console est vulnérable~Esta consola es vulnerable~Questa console è vulnerabile~Эта консоль уязвима~Diese Konsole ist anfällig~Esta consola é vulnerável
to the RCM bug!~à la faille RCM !~al fallo RCM.~alla falla RCM.~к уязвимости RCM.~für den RCM-Fehler.~à falha RCM.
This unit is patched~Cette console est protégée~Esta consola está protegida~Questa console è protetta~Эта консоль защищена~Diese Konsole ist geschützt~Esta consola está protegida
Ipatches:~Correctifs Ipatch :~Parches Ipatch:~Patch Ipatch:~Исправления Ipatch:~Ipatch-Korrekturen:~Correções Ipatch:
Address~Adresse~Dirección~Indirizzo~Адрес~Adresse~Endereço
Val~Valeur~Valor~Valore~Значение~Wert~Valor
Instruction~Instruction~Instrucción~Istruzione~Инструкция~Anweisung~Instrução
Device report~Rapport de l'appareil~Informe del dispositivo~Rapporto dispositivo~Отчёт устройства~Gerätebericht~Relatório do dispositivo
Average Erases SLC:~Effacements SLC moyens :~Borrados SLC medios:~Cancellazioni SLC medie:~Средние стирания SLC:~Mittlere SLC-Löschungen:~Apagamentos SLC médios:
Average Erases MLC:~Effacements MLC moyens :~Borrados MLC medios:~Cancellazioni MLC medie:~Средние стирания MLC:~Mittlere MLC-Löschungen:~Apagamentos MLC médios:
Read Reclaims SLC:~Récupérations en lecture SLC :~Recuperaciones de lectura SLC:~Recuperi in lettura SLC:~Восстановления чтения SLC:~SLC-Lesewiederherstellungen:~Recuperações de leitura SLC:
Read Reclaims MLC:~Récupérations en lecture MLC :~Recuperaciones de lectura MLC:~Recuperi in lettura MLC:~Восстановления чтения MLC:~MLC-Lesewiederherstellungen:~Recuperações de leitura MLC:
Bad Blocks Factory:~Blocs défectueux d'usine :~Bloques defectuosos de fábrica:~Blocchi difettosi di fabbrica:~Заводские плохие блоки:~Ab Werk fehlerhafte Blöcke:~Blocos defeituosos de fábrica:
Bad Blocks SYS:~Blocs SYS défectueux :~Bloques SYS defectuosos:~Blocchi SYS difettosi:~Плохие блоки SYS:~Fehlerhafte SYS-Blöcke:~Blocos SYS defeituosos:
Bad Blocks SLC:~Blocs SLC défectueux :~Bloques SLC defectuosos:~Blocchi SLC difettosi:~Плохие блоки SLC:~Fehlerhafte SLC-Blöcke:~Blocos SLC defeituosos:
Bad Blocks MLC:~Blocs MLC défectueux :~Bloques MLC defectuosos:~Blocchi MLC difettosi:~Плохие блоки MLC:~Fehlerhafte MLC-Blöcke:~Blocos MLC defeituosos:
FW Updates:~Mises à jour du micrologiciel :~Actualizaciones de firmware:~Aggiornamenti firmware:~Обновления прошивки:~Firmware-Aktualisierungen:~Atualizações do firmware:
FW Buildtime:~Date de compilation du micrologiciel :~Fecha de compilación del firmware:~Data compilazione firmware:~Дата сборки прошивки:~Firmware-Erstellungszeit:~Data de compilação do firmware:
Total Writes:~Écritures totales :~Escrituras totales:~Scritture totali:~Всего записано:~Gesamtschreibmenge:~Escritas totais:
Total Writes SLC:~Écritures SLC totales :~Escrituras SLC totales:~Scritture SLC totali:~Всего записано SLC:~SLC-Gesamtschreibmenge:~Escritas SLC totais:
Total Writes MLC:~Écritures MLC totales :~Escrituras MLC totales:~Scritture MLC totali:~Всего записано MLC:~MLC-Gesamtschreibmenge:~Escritas MLC totais:
BigFile limit status:~État de la limite BigFile :~Estado del límite BigFile:~Stato limite BigFile:~Состояние ограничения BigFile:~BigFile-Grenzstatus:~Estado do limite BigFile:
Average Erases Hybrid:~Effacements hybrides moyens :~Borrados híbridos medios:~Cancellazioni ibride medie:~Средние гибридные стирания:~Mittlere Hybrid-Löschungen:~Apagamentos híbridos médios:
Advanced Health Status~État de santé avancé~Estado de salud avanzado~Stato di salute avanzato~Расширенное состояние ресурса~Erweiterter Zustandsbericht~Estado de saúde avançado
Power ups:~Mises sous tension :~Encendidos:~Accensioni:~Включения:~Einschaltvorgänge:~Ligações:
Maximum Erases SLC:~Effacements SLC maximaux :~Borrados SLC máximos:~Cancellazioni SLC massime:~Максимум стираний SLC:~Maximale SLC-Löschungen:~Apagamentos SLC máximos:
Maximum Erases MLC:~Effacements MLC maximaux :~Borrados MLC máximos:~Cancellazioni MLC massime:~Максимум стираний MLC:~Maximale MLC-Löschungen:~Apagamentos MLC máximos:
Minimum Erases SLC:~Effacements SLC minimaux :~Borrados SLC mínimos:~Cancellazioni SLC minime:~Минимум стираний SLC:~Minimale SLC-Löschungen:~Apagamentos SLC mínimos:
Minimum Erases MLC:~Effacements MLC minimaux :~Borrados MLC mínimos:~Cancellazioni MLC minime:~Минимум стираний MLC:~Minimale MLC-Löschungen:~Apagamentos MLC mínimos:
Maximum Erases EUDA:~Effacements EUDA maximaux :~Borrados EUDA máximos:~Cancellazioni EUDA massime:~Максимум стираний EUDA:~Maximale EUDA-Löschungen:~Apagamentos EUDA máximos:
Minimum Erases EUDA:~Effacements EUDA minimaux :~Borrados EUDA mínimos:~Cancellazioni EUDA minime:~Минимум стираний EUDA:~Minimale EUDA-Löschungen:~Apagamentos EUDA mínimos:
Average Erases EUDA:~Effacements EUDA moyens :~Borrados EUDA medios:~Cancellazioni EUDA medie:~Средние стирания EUDA:~Mittlere EUDA-Löschungen:~Apagamentos EUDA médios:
Read Reclaims EUDA:~Récupérations en lecture EUDA :~Recuperaciones de lectura EUDA:~Recuperi in lettura EUDA:~Восстановления чтения EUDA:~EUDA-Lesewiederherstellungen:~Recuperações de leitura EUDA:
Bad Blocks EUDA:~Blocs EUDA défectueux :~Bloques EUDA defectuosos:~Blocchi EUDA difettosi:~Плохие блоки EUDA:~Fehlerhafte EUDA-Blöcke:~Blocos EUDA defeituosos:
Uncorrectable ECC:~ECC non corrigeables :~ECC no corregibles:~ECC non correggibili:~Неисправимые ECC:~Nicht korrigierbare ECC:~ECC não corrigíveis:
Temperature Now:~Température actuelle :~Temperatura actual:~Temperatura attuale:~Текущая температура:~Aktuelle Temperatur:~Temperatura atual:
Temperature Max:~Température maximale :~Temperatura máxima:~Temperatura massima:~Максимальная температура:~Höchsttemperatur:~Temperatura máxima:
Health Level EUDA:~État de santé EUDA :~Nivel de salud EUDA:~Livello salute EUDA:~Состояние ресурса EUDA:~EUDA-Zustand:~Nível de saúde EUDA:
Health Level MLC:~État de santé MLC :~Nivel de salud MLC:~Livello salute MLC:~Состояние ресурса MLC:~MLC-Zustand:~Nível de saúde MLC:
Vendor/Reserved Registers~Registres fabricant/réservés~Registros del fabricante/reservados~Registri produttore/riservati~Регистры производителя/резервные~Hersteller-/Reservierte Register~Registos do fabricante/reservados
SCR Vendor:~SCR fabricant :~SCR del fabricante:~SCR produttore:~SCR производителя:~Hersteller-SCR:~SCR do fabricante:
Has data!~Contient des données !~Contiene datos.~Contiene dati.~Есть данные.~Enthält Daten!~Contém dados!
Empty~Vide~Vacío~Vuoto~Пусто~Leer~Vazio
[Raw Reads] Abort: VOL- & VOL+~[Lectures brutes] Annuler : VOL- et VOL+~[Lecturas sin procesar] Cancelar: VOL- y VOL+~[Letture dirette] Annulla: VOL- e VOL+~[Прямое чтение] Отмена: VOL- и VOL+~[Rohdaten lesen] Abbruch: VOL- und VOL+~[Leituras diretas] Cancelar: VOL- e VOL+
[Raw Reads]~[Lectures brutes]~[Lecturas sin procesar]~[Letture dirette]~[Прямое чтение]~[Rohdaten lesen]~[Leituras diretas]
Rate:~Débit :~Velocidad:~Velocità:~Скорость:~Rate:~Velocidade:
SEQ 16MB~SEQ 16 Mo~SEQ 16 MB~SEQ 16 MB~SEQ 16 МБ~SEQ 16 MB~SEQ 16 MB
SEQ  4KB~SEQ  4 Ko~SEQ  4 KB~SEQ  4 KB~SEQ  4 КБ~SEQ  4 KB~SEQ  4 KB
RND  4KB~RND  4 Ko~RND  4 KB~RND  4 KB~RND  4 КБ~RND  4 KB~RND  4 KB
Manual~Manuel~Manual~Manuale~Вручную~Manuell~Manual
Minor~Mineure~Menor~Minore~Младшая~Nebenrevision~Menor
Vendor ID:~Identifiant fabricant :~Identificador de fabricante:~Identificativo produttore:~Идентификатор производителя:~Herstellerkennung:~Identificador do fabricante:
Model:~Modèle :~Modelo:~Modello:~Модель:~Modell:~Modelo:
Prod Rev:~Révision produit :~Revisión del producto:~Revisione prodotto:~Ревизия продукта:~Produktrevision:~Revisão do produto:
S/N:~N° de série :~N.º de serie:~N. di serie:~Серийный №:~Seriennr.:~N.º de série:
Month/Year:~Mois/année :~Mes/año:~Mese/anno:~Месяц/год:~Monat/Jahr:~Mês/ano:
Cmd Classes:~Classes de commandes :~Clases de comandos:~Classi comandi:~Классы команд:~Befehlsklassen:~Classes de comandos:
Max Bus Rate:~Débit maximal du bus :~Velocidad máxima del bus:~Velocità massima bus:~Максимальная скорость шины:~Maximale Busrate:~Velocidade máxima do barramento:
Current Rate:~Débit actuel :~Velocidad actual:~Velocità attuale:~Текущая скорость:~Aktuelle Rate:~Velocidade atual:
Enhanced Area:~Zone améliorée :~Área mejorada:~Area migliorata:~Улучшенная область:~Erweiterter Bereich:~Área melhorada:
Write Cache:~Cache d'écriture :~Caché de escritura:~Cache scrittura:~Кэш записи:~Schreibcache:~Cache de escrita:
Maintenance:~Maintenance :~Mantenimiento:~Manutenzione:~Обслуживание:~Wartung:~Manutenção:
Estimated Life:~Durée de vie estimée :~Vida útil estimada:~Durata stimata:~Оценка срока службы:~Geschätzte Lebensdauer:~Vida útil estimada:
Reserved Used:~Réserve utilisée :~Reserva utilizada:~Riserva utilizzata:~Использовано резерва:~Verbrauchte Reserve:~Reserva utilizada:
eMMC Physical Partitions:~Partitions physiques eMMC :~Particiones físicas eMMC:~Partizioni fisiche eMMC:~Физические разделы eMMC:~Physische eMMC-Partitionen:~Partições físicas eMMC:
Size:~Taille :~Tamaño:~Dimensione:~Размер:~Größe:~Tamanho:
Sectors:~Secteurs :~Sectores:~Settori:~Секторы:~Sektoren:~Setores:
GPP (eMMC USER) Partition Table:~Table de partitions GPP (eMMC USER) :~Tabla de particiones GPP (eMMC USER):~Tabella partizioni GPP (eMMC USER):~Таблица разделов GPP (eMMC USER):~GPP-Partitionstabelle (eMMC USER):~Tabela de partições GPP (eMMC USER):
Idx Name                            Size     Offset    Sectors~Idx Nom                             Taille   Position  Secteurs~Índ Nombre                          Tamaño   Posición  Sectores~Ind Nome                            Dimens.  Offset    Settori~Инд Имя                             Размер   Смещение  Секторы~Idx Name                            Größe    Position  Sektoren~Índ Nome                            Tamanho  Posição   Setores
Card ID~Identifiant de carte~Identificador de tarjeta~Identificativo scheda~Идентификатор карты~Kartenkennung~Identificador do cartão
OEM ID:~Identifiant OEM :~Identificador OEM:~Identificativo OEM:~Идентификатор OEM:~OEM-Kennung:~Identificador OEM:
Max Power:~Puissance maximale :~Potencia máxima:~Potenza massima:~Максимальная мощность:~Maximale Leistung:~Potência máxima:
Initial bus:~Bus initial :~Bus inicial:~Bus iniziale:~Исходная шина:~Anfänglicher Bus:~Barramento inicial:
Card-Specific Data~Données propres à la carte~Datos específicos de la tarjeta~Dati specifici della scheda~Данные конкретной карты~Kartenspezifische Daten~Dados específicos do cartão
Capacity:~Capacité :~Capacidad:~Capacità:~Ёмкость:~Kapazität:~Capacidade:
Capacity (LBA):~Capacité (LBA) :~Capacidad (LBA):~Capacità (LBA):~Ёмкость (LBA):~Kapazität (LBA):~Capacidade (LBA):
Bus Width:~Largeur du bus :~Ancho del bus:~Larghezza bus:~Ширина шины:~Busbreite:~Largura do barramento:
Max Bus Speed:~Vitesse maximale du bus :~Velocidad máxima del bus:~Velocità massima bus:~Максимальная скорость шины:~Maximale Busgeschwindigkeit:~Velocidade máxima do barramento:
Speed Class:~Classe de vitesse :~Clase de velocidad:~Classe di velocità:~Класс скорости:~Geschwindigkeitsklasse:~Classe de velocidade:
UHS Classes:~Classes UHS :~Clases UHS:~Classi UHS:~Классы UHS:~UHS-Klassen:~Classes UHS:
Write Protect:~Protection en écriture :~Protección de escritura:~Protezione scrittura:~Защита записи:~Schreibschutz:~Proteção de escrita:
Vendor Info:~Informations fabricant :~Información del fabricante:~Informazioni produttore:~Сведения производителя:~Herstellerinformationen:~Informações do fabricante:
SDMMC1 Errors:~Erreurs SDMMC1 :~Errores SDMMC1:~Errori SDMMC1:~Ошибки SDMMC1:~SDMMC1-Fehler:~Erros SDMMC1:
SDMMC4 Errors:~Erreurs SDMMC4 :~Errores SDMMC4:~Errori SDMMC4:~Ошибки SDMMC4:~SDMMC4-Fehler:~Erros SDMMC4:
Init fails:~Échecs d'initialisation :~Fallos de inicialización:~Inizializzazioni fallite:~Сбои инициализации:~Initialisierungsfehler:~Falhas de inicialização:
Read/Write fails:~Échecs de lecture/écriture :~Fallos de lectura/escritura:~Operazioni lettura/scrittura fallite:~Сбои чтения/записи:~Lese-/Schreibfehlschläge:~Falhas de leitura/escrita:
Read/Write errors:~Erreurs de lecture/écriture :~Errores de lectura/escritura:~Errori lettura/scrittura:~Ошибки чтения/записи:~Lese-/Schreibfehler:~Erros de leitura/escrita:
Found FAT FS:~Système FAT détecté :~Sistema FAT detectado:~File system FAT rilevato:~Обнаружена FAT:~FAT-Dateisystem gefunden:~Sistema FAT encontrado:
Filesystem:~Système de fichiers :~Sistema de archivos:~File system:~Файловая система:~Dateisystem:~Sistema de ficheiros:
Cluster:~Unité d'allocation :~Unidad de asignación:~Unità di allocazione:~Кластер:~Zuordnungseinheit:~Unidade de alocação:
Size free/total:~Espace libre/total :~Espacio libre/total:~Spazio libero/totale:~Свободно/всего:~Frei/Gesamt:~Espaço livre/total:
Init~Initialisation~Inicialización~Inizializzazione~Инициализация~Initialisierung~Inicialização
Low~Faible~Bajo~Basso~Низкий~Niedrig~Baixo
Error!~Erreur !~Error.~Errore.~Ошибка!~Fehler!~Erro!
Unknown OTP~OTP inconnu~OTP desconocida~OTP sconosciuta~Неизвестная OTP~Unbekannte OTP~OTP desconhecida
Battery Charger IC Info:~Informations du circuit de charge :~Información del circuito de carga:~Informazioni circuito di carica:~Сведения о микросхеме зарядки:~Informationen zum Lade-IC:~Informações do circuito de carga:
Input current limit:~Limite de courant d'entrée :~Límite de corriente de entrada:~Limite corrente in ingresso:~Предел входного тока:~Eingangsstromgrenze:~Limite da corrente de entrada:
System voltage limit:~Limite de tension système :~Límite de tensión del sistema:~Limite tensione sistema:~Предел напряжения системы:~Systemspannungsgrenze:~Limite da tensão do sistema:
Charge current limit:~Limite de courant de charge :~Límite de corriente de carga:~Limite corrente di carica:~Предел зарядного тока:~Ladestromgrenze:~Limite da corrente de carga:
Charge voltage limit:~Limite de tension de charge :~Límite de tensión de carga:~Limite tensione di carica:~Предел зарядного напряжения:~Ladespannungsgrenze:~Limite da tensão de carga:
Charge status:~État de charge :~Estado de carga:~Stato carica:~Состояние зарядки:~Ladestatus:~Estado de carga:
Temperature status:~État thermique :~Estado térmico:~Stato termico:~Температурное состояние:~Temperaturstatus:~Estado térmico:
USB-PD IC Info:~Informations du circuit USB-PD :~Información del circuito USB-PD:~Informazioni circuito USB-PD:~Сведения о микросхеме USB-PD:~Informationen zum USB-PD-IC:~Informações do circuito USB-PD:
Connection status:~État de connexion :~Estado de conexión:~Stato connessione:~Состояние подключения:~Verbindungsstatus:~Estado da ligação:
Input Wattage Limit:~Limite de puissance d'entrée :~Límite de potencia de entrada:~Limite potenza in ingresso:~Предел входной мощности:~Eingangsleistungsgrenze:~Limite da potência de entrada:
USB-PD Profiles:~Profils USB-PD :~Perfiles USB-PD:~Profili USB-PD:~Профили USB-PD:~USB-PD-Profile:~Perfis USB-PD:
Non PD~Sans PD~Sin PD~Senza PD~Без PD~Ohne PD~Sem PD
Delete Installation Files~Supprimer les fichiers d'installation~Eliminar archivos de instalación~Elimina file di installazione~Удалить файлы установки~Installationsdateien löschen~Eliminar ficheiros de instalação
Flashing Linux...~Écriture de Linux...~Escribiendo Linux...~Scrittura di Linux...~Запись Linux...~Linux wird geschrieben...~A escrever Linux...
Failed to open 1st part!~Impossible d'ouvrir la première partie !~No se pudo abrir la primera parte.~Impossibile aprire la prima parte.~Не удалось открыть первую часть!~Erster Teil konnte nicht geöffnet werden!~Não foi possível abrir a primeira parte!
Failed to open part %d~Impossible d'ouvrir la partie %d~No se pudo abrir la parte %d~Impossibile aprire la parte %d~Не удалось открыть часть %d~Teil %d konnte nicht geöffnet werden~Não foi possível abrir a parte %d
Reading from SD!~Lecture de la carte SD !~Lectura de la tarjeta SD.~Lettura dalla scheda SD.~Чтение с SD!~Lesen von SD!~Leitura do cartão SD!
Writing to SD!~Écriture sur la carte SD !~Escritura en la tarjeta SD.~Scrittura sulla scheda SD.~Запись на SD!~Schreiben auf SD!~Escrita no cartão SD!
Searching for files and partitions...~Recherche des fichiers et partitions...~Buscando archivos y particiones...~Ricerca di file e partizioni...~Поиск файлов и разделов...~Dateien und Partitionen werden gesucht...~A procurar ficheiros e partições...
Installation files not found!~Fichiers d'installation introuvables !~No se encontraron archivos de instalación.~File di installazione non trovati.~Файлы установки не найдены!~Installationsdateien nicht gefunden!~Ficheiros de instalação não encontrados!
No partition found!~Aucune partition trouvée !~No se encontró ninguna partición.~Nessuna partizione trovata.~Разделы не найдены!~Keine Partition gefunden!~Nenhuma partição encontrada!
The image is not aligned to 4 MiB!~L'image n'est pas alignée sur 4 Mio !~La imagen no está alineada a 4 MiB.~L'immagine non è allineata a 4 MiB.~Образ не выровнен по 4 МиБ!~Das Image ist nicht auf 4 MiB ausgerichtet!~A imagem não está alinhada a 4 MiB!
The image is bigger than the partition!~L'image est plus grande que la partition !~La imagen supera el tamaño de la partición.~L'immagine è più grande della partizione.~Образ больше раздела!~Das Image ist größer als die Partition!~A imagem é maior do que a partição!
Found installation files and partition.~Fichiers d'installation et partition trouvés.~Archivos de instalación y partición encontrados.~File di installazione e partizione trovati.~Файлы установки и раздел найдены.~Installationsdateien und Partition gefunden.~Ficheiros de instalação e partição encontrados.
Offset:~Position :~Posición:~Offset:~Смещение:~Position:~Posição:
Image size:~Taille de l'image :~Tamaño de imagen:~Dimensione immagine:~Размер образа:~Imagegröße:~Tamanho da imagem:
No Android GPT was found!~Aucune GPT Android trouvée !~No se encontró ninguna GPT de Android.~Nessuna GPT Android trovata.~GPT Android не найдена!~Keine Android-GPT gefunden!~Nenhuma GPT Android encontrada!
Kernel image not found!~Image du noyau introuvable !~No se encontró la imagen del núcleo.~Immagine del kernel non trovata.~Образ ядра не найден!~Kernel-Image nicht gefunden!~Imagem do kernel não encontrada!
Kernel image too big!~Image du noyau trop grande !~La imagen del núcleo es demasiado grande.~Immagine del kernel troppo grande.~Образ ядра слишком большой!~Kernel-Image zu groß!~Imagem do kernel demasiado grande!
Kernel image flashed!~Image du noyau écrite !~Imagen del núcleo escrita.~Immagine del kernel scritta.~Образ ядра записан!~Kernel-Image geschrieben!~Imagem do kernel escrita!
Kernel partition not found!~Partition du noyau introuvable !~No se encontró la partición del núcleo.~Partizione del kernel non trovata.~Раздел ядра не найден!~Kernel-Partition nicht gefunden!~Partição do kernel não encontrada!
Recovery image not found!~Image de récupération introuvable !~No se encontró la imagen de recuperación.~Immagine di ripristino non trovata.~Образ восстановления не найден!~Recovery-Image nicht gefunden!~Imagem de recuperação não encontrada!
Recovery image too big!~Image de récupération trop grande !~La imagen de recuperación es demasiado grande.~Immagine di ripristino troppo grande.~Образ восстановления слишком большой!~Recovery-Image zu groß!~Imagem de recuperação demasiado grande!
Recovery image flashed!~Image de récupération écrite !~Imagen de recuperación escrita.~Immagine di ripristino scritta.~Образ восстановления записан!~Recovery-Image geschrieben!~Imagem de recuperação escrita!
Recovery partition not found!~Partition de récupération introuvable !~No se encontró la partición de recuperación.~Partizione di ripristino non trovata.~Раздел восстановления не найден!~Recovery-Partition nicht gefunden!~Partição de recuperação não encontrada!
DTB image not found!~Image DTB introuvable !~No se encontró la imagen DTB.~Immagine DTB non trovata.~Образ DTB не найден!~DTB-Image nicht gefunden!~Imagem DTB não encontrada!
DTB image too big!~Image DTB trop grande !~La imagen DTB es demasiado grande.~Immagine DTB troppo grande.~Образ DTB слишком большой!~DTB-Image zu groß!~Imagem DTB demasiado grande!
DTB image flashed!~Image DTB écrite !~Imagen DTB escrita.~Immagine DTB scritta.~Образ DTB записан!~DTB-Image geschrieben!~Imagem DTB escrita!
DTB partition not found!~Partition DTB introuvable !~No se encontró la partición DTB.~Partizione DTB non trovata.~Раздел DTB не найден!~DTB-Partition nicht gefunden!~Partição DTB não encontrada!
Do you want to reboot into Recovery\nto finish Android installation?~Redémarrer en récupération\npour terminer l'installation d'Android ?~¿Reiniciar en recuperación\npara terminar la instalación de Android?~Riavviare in ripristino\nper completare l'installazione Android?~Перезагрузить в режим восстановления\nдля завершения установки Android?~In Recovery neu starten,\num die Android-Installation abzuschließen?~Reiniciar em recuperação\npara concluir a instalação do Android?
Initializing Ramdisk...~Initialisation du disque RAM...~Inicializando el disco RAM...~Inizializzazione disco RAM...~Инициализация RAM-диска...~RAM-Disk wird initialisiert...~A inicializar o disco RAM...
Failed to initialize Ramdisk!~Impossible d'initialiser le disque RAM !~No se pudo inicializar el disco RAM.~Impossibile inizializzare il disco RAM.~Не удалось инициализировать RAM-диск!~RAM-Disk konnte nicht initialisiert werden!~Não foi possível inicializar o disco RAM!
Backing up files...~Sauvegarde des fichiers...~Guardando copia de los archivos...~Backup dei file...~Резервное копирование файлов...~Dateien werden gesichert...~A criar cópia dos ficheiros...
Failed to back up files!~Impossible de sauvegarder les fichiers !~No se pudo guardar la copia de los archivos.~Backup dei file non riuscito.~Не удалось сохранить резервную копию файлов!~Dateisicherung fehlgeschlagen!~Não foi possível criar a cópia dos ficheiros!
Bootloader folder exceeds 1.2GB or corrupt!~Dossier bootloader corrompu ou supérieur à 1,2 Go !~La carpeta bootloader supera 1,2 GB o está dañada.~La cartella bootloader supera 1,2 GB o è danneggiata.~Папка bootloader повреждена или превышает 1,2 ГБ!~Ordner bootloader ist beschädigt oder größer als 1,2 GB!~A pasta bootloader excede 1,2 GB ou está danificada!
Formatting FAT32 partition...~Formatage de la partition FAT32...~Formateando la partición FAT32...~Formattazione partizione FAT32...~Форматирование раздела FAT32...~FAT32-Partition wird formatiert...~A formatar a partição FAT32...
Failed to format disk (%d)!~Impossible de formater le disque (%d) !~No se pudo formatear el disco (%d).~Impossibile formattare il disco (%d).~Не удалось отформатировать диск (%d)!~Datenträger konnte nicht formatiert werden (%d)!~Não foi possível formatar o disco (%d)!
Restoring files...~Restauration des fichiers...~Restaurando archivos...~Ripristino dei file...~Восстановление файлов...~Dateien werden wiederhergestellt...~A restaurar os ficheiros...
Failed to restore files!~Impossible de restaurer les fichiers !~No se pudieron restaurar los archivos.~Ripristino dei file non riuscito.~Не удалось восстановить файлы!~Dateiwiederherstellung fehlgeschlagen!~Não foi possível restaurar os ficheiros!
Restored files but the operation failed!~Fichiers restaurés, mais l'opération a échoué !~Archivos restaurados, pero la operación falló.~File ripristinati, ma operazione non riuscita.~Файлы восстановлены, но операция завершилась неудачей!~Dateien wiederhergestellt, aber Vorgang fehlgeschlagen!~Ficheiros restaurados, mas a operação falhou!
Flashing partition table...~Écriture de la table de partitions...~Escribiendo la tabla de particiones...~Scrittura della tabella partizioni...~Запись таблицы разделов...~Partitionstabelle wird geschrieben...~A escrever a tabela de partições...
Done!~Terminé !~Terminado.~Completato.~Готово!~Fertig!~Concluído!
Initializing...~Initialisation...~Inicializando...~Inizializzazione...~Инициализация...~Initialisierung...~A inicializar...
For formatting USER partition,~Pour formater la partition USER,~Para formatear la partición USER,~Per formattare la partizione USER,~Для форматирования раздела USER~Zum Formatieren der USER-Partition~Para formatar a partição USER,
BIS keys are needed!~les clés BIS sont nécessaires !~se necesitan las claves BIS.~sono necessarie le chiavi BIS.~нужны ключи BIS!~werden BIS-Schlüssel benötigt!~são necessárias as chaves BIS!
Formatting USER partition...~Formatage de la partition USER...~Formateando la partición USER...~Formattazione partizione USER...~Форматирование раздела USER...~USER-Partition wird formatiert...~A formatar a partição USER...
Flash Linux~Installer Linux~Instalar Linux~Installa Linux~Установить Linux~Linux installieren~Instalar Linux
Flash Android~Installer Android~Instalar Android~Installa Android~Установить Android~Android installieren~Instalar Android
Legacy~Ancien format~Heredado~Tradizionale~Старая схема~Klassisch~Legado
Dynamic~Dynamique~Dinámico~Dinamico~Динамическая~Dynamisch~Dinâmico
Don't Backup~Ne pas sauvegarder~No hacer copia~Non fare backup~Не создавать копию~Nicht sichern~Não criar cópia
The SD Card files will be backed up automatically!~Les fichiers de la carte SD seront sauvegardés automatiquement !~Los archivos de la tarjeta SD se copiarán automáticamente.~I file della scheda SD saranno salvati automaticamente.~Файлы SD-карты будут автоматически сохранены!~Die SD-Dateien werden automatisch gesichert!~Os ficheiros do cartão SD serão copiados automaticamente!
Any other partition will be wiped!~Toutes les autres partitions seront effacées !~Se borrarán todas las demás particiones.~Tutte le altre partizioni saranno cancellate.~Все остальные разделы будут стёрты!~Alle anderen Partitionen werden gelöscht!~Todas as outras partições serão apagadas!
Total files:~Nombre de fichiers :~Archivos totales:~File totali:~Всего файлов:~Dateien insgesamt:~Total de ficheiros:
Total size:~Taille totale :~Tamaño total:~Dimensione totale:~Общий размер:~Gesamtgröße:~Tamanho total:
No valid GPT was found!~Aucune GPT valide trouvée !~No se encontró ninguna GPT válida.~Nessuna GPT valida trovata.~Корректная GPT не найдена!~Keine gültige GPT gefunden!~Nenhuma GPT válida encontrada!
The Hybrid MBR needs no change!~Le MBR hybride ne nécessite aucune modification !~El MBR híbrido no necesita cambios.~L'MBR ibrido non richiede modifiche.~Гибридная MBR не требует изменений!~Der Hybrid-MBR muss nicht geändert werden!~O MBR híbrido não necessita de alterações!
The following need to be corrected:~Les éléments suivants doivent être corrigés :~Se deben corregir los siguientes elementos:~Occorre correggere quanto segue:~Необходимо исправить следующее:~Folgendes muss korrigiert werden:~É necessário corrigir o seguinte:
- MBR attributes~- Attributs MBR~- Atributos MBR~- Attributi MBR~- Атрибуты MBR~- MBR-Attribute~- Atributos MBR
- emuMMC GPT Partition address and size~- Adresse et taille de partition GPT emuMMC~- Dirección y tamaño de partición GPT emuMMC~- Indirizzo e dimensione partizione GPT emuMMC~- Адрес и размер раздела GPT emuMMC~- Adresse und Größe der emuMMC-GPT-Partition~- Endereço e tamanho da partição GPT emuMMC
- GPT OOB/Empty Partitions (removal)~- Partitions GPT hors limites/vides (suppression)~- Particiones GPT fuera de límites/vacías (eliminar)~- Partizioni GPT fuori limite/vuote (rimozione)~- Разделы GPT вне границ/пустые (удаление)~- GPT-Partitionen außerhalb der Grenzen/leer (Entfernung)~- Partições GPT fora dos limites/vazias (remoção)
Choose new partition layout:~Choisir la nouvelle disposition des partitions :~Elige la nueva distribución de particiones:~Scegli la nuova disposizione delle partizioni:~Выберите новую схему разделов:~Neues Partitionslayout wählen:~Escolher a nova disposição das partições:
HOS (FAT32):~HOS (FAT32) :~HOS (FAT32):~HOS (FAT32):~HOS (FAT32):~HOS (FAT32):~HOS (FAT32):
eMMC (USER):~eMMC (USER) :~eMMC (USER):~eMMC (USER):~eMMC (USER):~eMMC (USER):~eMMC (USER):
emuMMC (RAW):~emuMMC (RAW) :~emuMMC (RAW):~emuMMC (RAW):~emuMMC (RAW):~emuMMC (RAW):~emuMMC (RAW):
Linux (EXT4):~Linux (EXT4) :~Linux (EXT4):~Linux (EXT4):~Linux (EXT4):~Linux (EXT4):~Linux (EXT4):
Android (USER):~Android (USER) :~Android (USER):~Android (USER):~Android (USER):~Android (USER):~Android (USER):
%s Benchmark~Test de performances %s~Prueba de rendimiento %s~Test prestazioni %s~Тест скорости %s~%s-Leistungstest~Teste de desempenho %s
(Module 0 | 1):~(Module 0 | 1) :~(Módulo 0 | 1):~(Modulo 0 | 1):~(Модуль 0 | 1):~(Modul 0 | 1):~(Módulo 0 | 1):
 + Manual~ + Manuel~ + Manual~ + Manuale~ + Вручную~ + Manuell~ + Manual
Start:~Début :~Inicio:~Inizio:~Начало:~Anfang:~Início:
Type:~Type :~Tipo:~Tipo:~Тип:~Typ:~Tipo:
Warning: Do you really want to continue?!~Attention : voulez-vous vraiment continuer ?!~Advertencia: ¿seguro que quieres continuar?~Attenzione: continuare davvero?~Внимание: вы действительно хотите продолжить?!~Warnung: Wirklich fortfahren?!~Aviso: pretende mesmo continuar?!
Please select a partition scheme:~Choisissez un schéma de partitions :~Selecciona un esquema de particiones:~Scegli uno schema di partizioni:~Выберите схему разделов:~Partitionsschema auswählen:~Escolha um esquema de partições:
Android 13+ (Preferred)~Android 13+ (recommandé)~Android 13+ (recomendado)~Android 13+ (consigliato)~Android 13+ (рекомендуется)~Android 13+ (empfohlen)~Android 13+ (recomendado)
Press #FF8000 POWER# to Continue.~Appuyez sur #FF8000 POWER# pour continuer.~Pulsa #FF8000 POWER# para continuar.~Premi #FF8000 POWER# per continuare.~Нажмите #FF8000 POWER# для продолжения.~Zum Fortfahren #FF8000 POWER# drücken.~Prima #FF8000 POWER# para continuar.
Press #FF8000 VOL# to abort.~Appuyez sur #FF8000 VOL# pour annuler.~Pulsa #FF8000 VOL# para cancelar.~Premi #FF8000 VOL# per annullare.~Нажмите #FF8000 VOL# для отмены.~Zum Abbrechen #FF8000 VOL# drücken.~Prima #FF8000 VOL# para cancelar.
Remove the SD card and check that is OK.~Retirez la carte SD et vérifiez son état.~Retira la tarjeta SD y comprueba su estado.~Rimuovi la scheda SD e verifica che funzioni.~Извлеките SD-карту и проверьте её исправность.~SD-Karte entfernen und auf Fehler prüfen.~Retire o cartão SD e verifique o seu estado.
If not, format it, reinsert it and~Sinon, formatez-la, réinsérez-la et~Si falla, formatéala, vuelve a insertarla y~Se necessario, formattala, reinseriscila e~При неисправности отформатируйте, вставьте и~Bei Fehlern formatieren, wieder einsetzen und~Se não estiver bem, formate-o, volte a inseri-lo e
press #FF8000 POWER#!~appuyez sur #FF8000 POWER# !~pulsa #FF8000 POWER#.~premi #FF8000 POWER#.~нажмите #FF8000 POWER#!~#FF8000 POWER# drücken!~prima #FF8000 POWER#!
Choose #FFDD00 new# partition layout:~Choisir la #FFDD00 nouvelle# disposition des partitions :~Elige la #FFDD00 nueva# distribución de particiones:~Scegli la #FFDD00 nuova# disposizione delle partizioni:~Выберите #FFDD00 новую# схему разделов:~#FFDD00 Neues# Partitionslayout wählen:~Escolher a #FFDD00 nova# disposição das partições:
This will flash #C7EA46 Kernel#, #C7EA46 DTB# and #C7EA46 Recovery# if found.~Les images #C7EA46 Kernel#, #C7EA46 DTB# et #C7EA46 Recovery# seront écrites si elles sont trouvées.~Se escribirán las imágenes #C7EA46 Kernel#, #C7EA46 DTB# y #C7EA46 Recovery# que se encuentren.~Saranno scritte le immagini #C7EA46 Kernel#, #C7EA46 DTB# e #C7EA46 Recovery# se presenti.~Будут записаны найденные образы #C7EA46 Kernel#, #C7EA46 DTB# и #C7EA46 Recovery#.~Gefundene Images für #C7EA46 Kernel#, #C7EA46 DTB# und #C7EA46 Recovery# werden geschrieben.~Serão escritas as imagens #C7EA46 Kernel#, #C7EA46 DTB# e #C7EA46 Recovery# que forem encontradas.
These will be deleted after a successful flash.~Elles seront supprimées après une écriture réussie.~Se eliminarán después de escribirlas correctamente.~Saranno eliminate dopo una scrittura riuscita.~После успешной записи они будут удалены.~Nach erfolgreichem Schreiben werden sie gelöscht.~Serão eliminadas após uma escrita bem-sucedida.
This will launch Lockpick RCM.~Lockpick RCM va être lancé.~Se iniciará Lockpick RCM.~Verrà avviato Lockpick RCM.~Будет запущен Lockpick RCM.~Lockpick RCM wird gestartet.~O Lockpick RCM será iniciado.
To return back from lockpick use~Pour revenir depuis Lockpick, utilisez~Para volver desde Lockpick, usa~Per tornare da Lockpick, usa~Для возврата из Lockpick используйте~Zur Rückkehr aus Lockpick die Option~Para regressar do Lockpick, utilize
Reboot to hekate~Reboot to hekate~Reboot to hekate~Reboot to hekate~Reboot to hekate~Reboot to hekate~Reboot to hekate
View Ipatches and dump the unpatched and patched versions~Afficher les correctifs Ipatch et exporter les versions d'origine et corrigée~Ver los parches Ipatch y exportar las versiones original y corregida~Visualizza le patch Ipatch ed esporta le versioni originale e corretta~Просмотр Ipatch и сохранение исходной и исправленной версий~Ipatches anzeigen und ursprüngliche sowie korrigierte Versionen sichern~Ver correções Ipatch e exportar as versões original e corrigida
of BootROM.~de BootROM.~de BootROM.~di BootROM.~BootROM.~des BootROM.~de BootROM.
Or dump every single key via #C7EA46 Lockpick RCM#.~Ou exporter toutes les clés avec #C7EA46 Lockpick RCM#.~O exportar todas las claves con #C7EA46 Lockpick RCM#.~Oppure esporta tutte le chiavi con #C7EA46 Lockpick RCM#.~Или сохраните все ключи через #C7EA46 Lockpick RCM#.~Oder alle Schlüssel mit #C7EA46 Lockpick RCM# sichern.~Ou exportar todas as chaves com #C7EA46 Lockpick RCM#.
of BootROM. Or dump every single key via #C7EA46 Lockpick RCM#.~de BootROM. Ou exporter toutes les clés avec #C7EA46 Lockpick RCM#.~de BootROM. O exportar todas las claves con #C7EA46 Lockpick RCM#.~di BootROM. Oppure esporta tutte le chiavi con #C7EA46 Lockpick RCM#.~BootROM. Или сохраните все ключи через #C7EA46 Lockpick RCM#.~des BootROM. Oder alle Schlüssel mit #C7EA46 Lockpick RCM# sichern.~de BootROM. Ou exportar todas as chaves com #C7EA46 Lockpick RCM#.
#FFDD00 bootloader/payloads/Lockpick_RCM.bin is missing or old!#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin est absent ou ancien !#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin falta o es antiguo.#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin manca o è obsoleto.#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin отсутствует или устарел!#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin fehlt oder ist veraltet!#~#FFDD00 bootloader/payloads/Lockpick_RCM.bin está ausente ou desatualizado!#
View and dump the cached #C7EA46 Fuses# and #C7EA46 KFuses#.~Afficher et exporter les #C7EA46 fusibles# et #C7EA46 KFuses# en cache.~Ver y exportar los #C7EA46 fusibles# y #C7EA46 KFuses# almacenados.~Visualizza ed esporta #C7EA46 fusibili# e #C7EA46 KFuses# memorizzati.~Просмотр и сохранение кэшированных #C7EA46 предохранителей# и #C7EA46 KFuses#.~Zwischengespeicherte #C7EA46 Fuses# und #C7EA46 KFuses# anzeigen und sichern.~Ver e exportar os #C7EA46 fusíveis# e #C7EA46 KFuses# em cache.
Fuses contain info about the SoC/SKU and KFuses HDCP keys.~Les fusibles contiennent les informations SoC/SKU et les KFuses les clés HDCP.~Los fusibles contienen información del SoC/SKU y los KFuses las claves HDCP.~I fusibili contengono informazioni SoC/SKU e i KFuses le chiavi HDCP.~Предохранители содержат данные SoC/SKU, а KFuses — ключи HDCP.~Fuses enthalten SoC-/SKU-Daten und KFuses die HDCP-Schlüssel.~Os fusíveis contêm dados do SoC/SKU e os KFuses as chaves HDCP.
You can also see info about #C7EA46 DRAM#, #C7EA46 Screen# and #C7EA46 Touch panel#.~Consulter aussi les informations sur la #C7EA46 DRAM#, l'#C7EA46 écran# et la #C7EA46 dalle tactile#.~También puedes ver información de la #C7EA46 DRAM#, la #C7EA46 pantalla# y el #C7EA46 panel táctil#.~Puoi anche consultare informazioni su #C7EA46 DRAM#, #C7EA46 schermo# e #C7EA46 pannello touch#.~Также доступны сведения о #C7EA46 DRAM#, #C7EA46 экране# и #C7EA46 сенсорной панели#.~Auch Informationen zu #C7EA46 DRAM#, #C7EA46 Bildschirm# und #C7EA46 Touchpanel# sind verfügbar.~Também pode consultar informações da #C7EA46 DRAM#, do #C7EA46 ecrã# e do #C7EA46 painel tátil#.
View info about the eMMC or microSD and their partition list.~Consulter les informations de l'eMMC ou de la microSD et leurs partitions.~Ver información de la eMMC o microSD y sus particiones.~Visualizza informazioni su eMMC o microSD e le relative partizioni.~Просмотр сведений об eMMC или microSD и их разделах.~Informationen zu eMMC oder microSD und deren Partitionen anzeigen.~Ver informações da eMMC ou microSD e das suas partições.
Additionally you can benchmark read speeds.~Vous pouvez aussi mesurer les débits de lecture.~También puedes medir la velocidad de lectura.~Puoi anche misurare la velocità di lettura.~Также можно измерить скорость чтения.~Zusätzlich kann die Lesegeschwindigkeit gemessen werden.~Também pode medir a velocidade de leitura.
Your eMMC is initialized in a slower mode,~Votre eMMC fonctionne dans un mode plus lent,~La eMMC se ha iniciado en un modo más lento,~L'eMMC è inizializzata in una modalità più lenta,~eMMC инициализирована в замедленном режиме,~Die eMMC wurde in einem langsameren Modus initialisiert,~A eMMC foi inicializada num modo mais lento,
or init/read/write errors occurred!~ou des erreurs d'initialisation/lecture/écriture ont eu lieu !~o se produjeron errores de inicio/lectura/escritura.~oppure si sono verificati errori di avvio/lettura/scrittura.~либо произошли ошибки инициализации/чтения/записи!~oder es traten Initialisierungs-/Lese-/Schreibfehler auf!~ou ocorreram erros de inicialização/leitura/escrita!
This might mean hardware issues!~Cela peut signaler un problème matériel !~Esto puede indicar problemas de hardware.~Ciò può indicare problemi hardware.~Это может указывать на неисправность оборудования!~Dies kann auf Hardwareprobleme hinweisen!~Isto pode indicar problemas de hardware!
Bus Speed:~Vitesse du bus :~Velocidad del bus:~Velocità bus:~Скорость шины:~Busgeschwindigkeit:~Velocidade do barramento:
Note 1: Only up to #C7EA46 1.2GB# can be backed up. If more, you will be asked to back them manually at the next step.~Note 1 : la sauvegarde est limitée à #C7EA46 1,2 Go#. Au-delà, une sauvegarde manuelle sera demandée à l'étape suivante.~Nota 1: solo se pueden copiar hasta #C7EA46 1,2 GB#. Si hay más, se pedirá una copia manual en el siguiente paso.~Nota 1: è possibile salvare al massimo #C7EA46 1,2 GB#. Oltre questo limite verrà richiesto un backup manuale al passaggio successivo.~Примечание 1: можно сохранить не более #C7EA46 1,2 ГБ#. При превышении на следующем шаге потребуется ручная копия.~Hinweis 1: Es können höchstens #C7EA46 1,2 GB# gesichert werden. Bei mehr Daten wird im nächsten Schritt zur manuellen Sicherung aufgefordert.~Nota 1: apenas é possível copiar até #C7EA46 1,2 GB#. Acima disso, será pedida uma cópia manual no passo seguinte.
Note 2: Resized emuMMC formats the USER partition. A save data manager can be used to move them over.~Note 2 : redimensionner l'emuMMC formate la partition USER. Un gestionnaire de sauvegardes peut transférer les données.~Nota 2: redimensionar la emuMMC formatea la partición USER. Usa un gestor de partidas para trasladar los datos.~Nota 2: ridimensionare l'emuMMC formatta la partizione USER. Un gestore dei salvataggi può trasferire i dati.~Примечание 2: изменение размера emuMMC форматирует раздел USER. Данные можно перенести менеджером сохранений.~Hinweis 2: Eine Größenänderung der emuMMC formatiert USER. Spielstände können mit einem Speicherdatenmanager übertragen werden.~Nota 2: redimensionar a emuMMC formata a partição USER. Pode usar um gestor de gravações para transferir os dados.
Note 3: The #C7EA46 Flash Linux# and #C7EA46 Flash Android# will flash files if suitable partitions and installer files are found.~Note 3 : #C7EA46 Installer Linux# et #C7EA46 Installer Android# écrivent les fichiers si les partitions et fichiers d'installation adéquats sont trouvés.~Nota 3: #C7EA46 Instalar Linux# e #C7EA46 Instalar Android# escriben los archivos si se encuentran particiones y archivos de instalación adecuados.~Nota 3: #C7EA46 Installa Linux# e #C7EA46 Installa Android# scrivono i file se vengono trovati partizioni e file di installazione adatti.~Примечание 3: #C7EA46 Установить Linux# и #C7EA46 Установить Android# запишут файлы при наличии подходящих разделов и установочных файлов.~Hinweis 3: #C7EA46 Linux installieren# und #C7EA46 Android installieren# schreiben die Dateien, wenn geeignete Partitionen und Installationsdateien gefunden werden.~Nota 3: #C7EA46 Instalar Linux# e #C7EA46 Instalar Android# escrevem os ficheiros se forem encontradas partições e ficheiros de instalação adequados.
Note 1: Any partition existing after the selected ones gets removed from the table.~Note 1 : toutes les partitions situées après celles sélectionnées sont retirées de la table.~Nota 1: se eliminarán de la tabla todas las particiones situadas después de las seleccionadas.~Nota 1: tutte le partizioni successive a quelle selezionate vengono rimosse dalla tabella.~Примечание 1: все разделы после выбранных будут удалены из таблицы.~Hinweis 1: Alle Partitionen nach den ausgewählten werden aus der Tabelle entfernt.~Nota 1: todas as partições após as selecionadas serão removidas da tabela.
Note 2: The HOS USER partition gets formatted. A save data manager can be used to move them over.~Note 2 : la partition HOS USER est formatée. Un gestionnaire de sauvegardes peut transférer les données.~Nota 2: se formateará la partición HOS USER. Usa un gestor de partidas para trasladar los datos.~Nota 2: la partizione HOS USER viene formattata. Un gestore dei salvataggi può trasferire i dati.~Примечание 2: раздел HOS USER будет отформатирован. Данные можно перенести менеджером сохранений.~Hinweis 2: Die Partition HOS USER wird formatiert. Spielstände können mit einem Speicherdatenmanager übertragen werden.~Nota 2: a partição HOS USER será formatada. Pode usar um gestor de gravações para transferir os dados.
'''

catalog={}
for row in ROWS.strip().splitlines():
    values=[value.replace(r'\n','\n').replace(r'\"','"') for value in row.split('~')]
    assert len(values)==7,(len(values),row)
    assert values[0] not in catalog,values[0]
    catalog[values[0]]=values

for number in range(4):
    source=f'Partition {number} - Type:'
    catalog[source]=[source,f'Partition {number} - Type :',f'Partición {number} - Tipo:',
        f'Partizione {number} - Tipo:',f'Раздел {number} - Тип:',f'Partition {number} - Typ:',f'Partição {number} - Tipo:']

# The patched and vulnerable RCM notices need different prepositions; author
# the whole notices rather than sharing the ambiguous second English line.
source='\n\n#96FF00 This unit is exploitable#\n#96FF00 to the RCM bug!#'
catalog[source]=[source,
    '\n\n#96FF00 Cette console est vulnérable#\n#96FF00 à la faille RCM !#',
    '\n\n#96FF00 Esta consola es vulnerable#\n#96FF00 al fallo RCM.#',
    '\n\n#96FF00 Questa console è vulnerabile#\n#96FF00 alla falla RCM.#',
    '\n\n#96FF00 Эта консоль уязвима#\n#96FF00 к ошибке RCM!#',
    '\n\n#96FF00 Diese Konsole ist anfällig#\n#96FF00 für den RCM-Fehler!#',
    '\n\n#96FF00 Esta consola é vulnerável#\n#96FF00 à falha RCM!#']
source='\n\n#FF8000 This unit is patched#\n#FF8000 to the RCM bug!#'
catalog[source]=[source,
    '\n\n#FF8000 Cette console est protégée#\n#FF8000 contre la faille RCM !#',
    '\n\n#FF8000 Esta consola está protegida#\n#FF8000 contra el fallo RCM.#',
    '\n\n#FF8000 Questa console è protetta#\n#FF8000 contro la falla RCM.#',
    '\n\n#FF8000 Эта консоль защищена#\n#FF8000 от ошибки RCM!#',
    '\n\n#FF8000 Diese Konsole ist gegen#\n#FF8000 den RCM-Fehler geschützt!#',
    '\n\n#FF8000 Esta consola está protegida#\n#FF8000 contra a falha RCM!#']

# Add the complete original literal when authored labels/phrases can be
# composed around its original layout. No generic word translation is used.
source_catalog=json.loads((ROOT/'localization_catalog.json').read_text(encoding='utf-8'))
phrases={**source_catalog,**catalog}
keys=sorted(phrases,key=len,reverse=True)
inventory=json.loads((WORK/'legacy-extra-candidates.json').read_text(encoding='utf-8'))
original_literals={re.sub(r'^[\x80-\x9f]', '', item['text']) for item in inventory}
def decorate(text,lang):
    if text in phrases and (text in catalog or text not in original_literals):return phrases[text][lang]
    core=text.strip()
    if core in catalog:
        lead=len(text)-len(text.lstrip());trail=len(text)-len(text.rstrip())
        return text[:lead]+catalog[core][lang]+(text[len(text)-trail:] if trail else '')
    if '\n' in text:return '\n'.join(decorate(line,lang) for line in text.split('\n'))
    # Preserve control bytes, color tags and printf conversions exactly.
    protected=re.compile(r'#[0-9A-Fa-f]{6} |#|[\x80-\x9f]')
    pieces=protected.split(text); decorations=protected.findall(text)
    output=[]
    for i,piece in enumerate(pieces):
        if piece:
            stripped=piece.strip()
            if stripped in phrases:
                lead=len(piece)-len(piece.lstrip());trail=len(piece)-len(piece.rstrip())
                piece=piece[:lead]+phrases[stripped][lang]+(piece[len(piece)-trail:] if trail else '')
            else:
                # Each newline is a presentation boundary, not a word.
                lines=[]
                for line in piece.split('\n'):
                    core=line.strip();lead=len(line)-len(line.lstrip());trail=len(line)-len(line.rstrip())
                    translated=phrases.get(core)
                    if translated:line=line[:lead]+translated[lang]+(line[len(line)-trail:] if trail else '')
                    else:
                        # Report field labels can precede values/units. Match
                        # only complete authored labels ending with a colon.
                        labels=[key for key in keys if key.endswith(':') and key in line]
                        tokenized=[];position=0
                        while position<len(line):
                            matches=[(line.find(key,position),key) for key in labels if line.find(key,position)>=0]
                            if not matches:tokenized.append(line[position:]);break
                            at,key=min(matches,key=lambda item:(item[0],-len(item[1])))
                            tokenized.append(line[position:at]);tokenized.append(phrases[key][lang]);position=at+len(key)
                        line=''.join(tokenized)
                    lines.append(line)
                piece='\n'.join(lines)
        output.append(piece)
        if i<len(decorations):output.append(decorations[i])
    return ''.join(output)

for item in inventory:
    source=re.sub(r'^[\x80-\x9f]', '', item['text'])
    if source in catalog:continue
    translated=[source]+[decorate(source,lang) for lang in range(1,7)]
    if any(value!=source for value in translated[1:]):catalog[source]=translated

FORMAT=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcspn]|%)')
for key,row in catalog.items():
    for lang,text in enumerate(row):
        assert FORMAT.findall(text)==FORMAT.findall(key),(lang,key,text)
target=ROOT/'localization_legacy_extra.json'
target.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Wrote',target,len(catalog),'authored/decorated keys')
