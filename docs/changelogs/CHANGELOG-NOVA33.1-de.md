# Hekate Nova Pro v33.1 beta

- Start-Kachel: kleines Symbol /icone/launch.png unabhängig vom großen Hintergrund /icone/Launcher.png; in Nova mit passender Vorschau austauschbar.
- Mitwirkende: HWTest-RCM 0.15 von Creased (Baptiste MOINE), unter Freie Software.
- Darstellung und Optionen: Hintergründe der Zeilen Sicherungsprüfung und Auswahleffekte entfernt; Rahmen der Systemoptionen vereinheitlicht.
- emuMMC-Sicherung und Wiederherstellung: aktiver Rahmen auf dem äußeren Panel, ohne versetzten Doppelrahmen; Tastenverhalten bleibt erhalten.
- Separate Standard- und experimentelle 8-GB-Pakete für physisch aufgerüstete Konsolen: Erista V1 und Mariko V2/Lite/OLED. Offizielles Hekate-Softwareflag ohne Programmierung von Fuses.
- RAM-Diagnose erhält den 8-GB-Modus bei Start und Rückkehr, wählt Trainingsprofile nach Plattform und Kapazität und stoppt bei Kapazitätsänderung nach dem Training.
- 64-Bit-Berechnung hoher Adressen und Bericht der tatsächlichen Kapazität: 8176 MiB von 8192 MiB prüfbar; 16 MiB für Arbeitsdaten geschützt.
- Atmosphère ab 1.11.0 für Speichermodi über 4 GB: memmode=1 in einem Hekate-fss0-Eintrag oder enable_mem_mode=1 in exosphere.ini für fusee. Vorhandene Konfigurationen werden nicht automatisch geändert.

Beta: ARM-Builds und PC-Prüfungen erfolgreich. Der 8-GB-Modus, insbesondere das angepasste Erista-Profil, benötigt Hardwaretests je Plattform; nicht alle Ersatzchips sind zertifiziert. Lange chinesische Berichte benötigen sprachliche Prüfung.

## v33

- Vereinfachtes Chinesisch als achte Sprache in den Einstellungen und bei der Ersteinrichtung.
- Nur Chinesisch verwendet eine festgelegte eigene Schrift. Die persönliche Schrift bleibt für andere Sprachen gespeichert; die Textgröße bleibt einstellbar.
- Lecture.nyxf ist die automatische Schrift für lateinische Sprachen; Russisch nutzt standardmäßig die integrierte Schrift. Die ursprüngliche Hekate-Schrift ist auswählbar.
- Drei statische Hintergründe, fünf Nova-Symbole und das bereitgestellte Launcher-Bild für die Hauptkachel sind enthalten.
- Vorgaben für neue Installationen: Französisch, Carbon Cyan, 75% Deckkraft für Fenster und Symbole, 30px Text, gewünschte Farben und helles Startlogo. Die ursprüngliche Uhreinstellung bleibt erhalten.
- Deutlicher Zugang zu Auto Boot, Startverzögerung, Auto NoGC und Auto HOS Power Off. Helligkeit öffnet den vorhandenen Regler ohne Duplikat.
- Seitenweiser Auto-Boot-Selektor im Nova-Stil ohne vertikale Liste und mit unveränderten Hekate-Konfigurationsindizes.
- Separater Speichertest mit chinesischer Oberfläche und 16 × 16 Zeichen; kompatibel mit den bisherigen sieben Sprachpaketen.
- Bei fehlender oder ungültiger chinesischer Schrift wird vorübergehend Englisch verwendet. Persönliche Schrift und gespeicherte Einstellungen bleiben erhalten.
