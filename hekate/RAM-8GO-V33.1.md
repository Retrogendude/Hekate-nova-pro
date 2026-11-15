# RAM 8 Go — Hekate Nova Pro v33.1 bêta

## Choisir le bon pack

Le pack **standard** est destiné aux consoles normales de 4 Go. Le pack **8GB expérimental** est destiné exclusivement aux consoles dont la mémoire a été physiquement remplacée pour atteindre 8 Go : V1/Erista ou V2/Lite/OLED/Mariko. Un logiciel ne transforme pas 4 Go en 8 Go.

Installer ensemble le payload et le dossier bootloader du même pack. Le payload de la variante 8GB porte un nom explicite ; son bootloader/update.bin porte le même drapeau que le payload injecté. Aucun fusible matériel n’est programmé par cette option.

## Atmosphère

Atmosphère 1.11.0 ou plus récent est nécessaire pour les modes mémoire supérieurs à 4 Go. Pour un démarrage Hekate via fss0, ajouter **memmode=1** à l’entrée Atmosphère existante dans bootloader/hekate_ipl.ini. Ne pas créer une deuxième entrée identique et conserver vos chemins et options emuMMC.

Pour un démarrage via fusee, conserver votre section [exosphere] dans exosphere.ini et y ajouter **enable_mem_mode=1**. Le pack ne remplace aucun de ces fichiers et n’inclut pas Atmosphère. Ne pas utiliser la méthode OFW directe lorsqu’un avertissement Hekate signale une incompatibilité entre l’identifiant DRAM et la RAM installée.

## Diagnostic séparé

Le diagnostic reprend le drapeau 8 Go avant de réinitialiser la mémoire, lit la capacité du contrôleur et sélectionne les profils selon la plateforme et la capacité. Une incohérence après entraînement arrête le test avec une erreur, sans annoncer PASS. Le retour à Nova conserve le mode initial, même si le fichier de retour sur SD avait un autre drapeau.

Le balayage utilise des adresses physiques 64 bits et une fenêtre SMMU de 4 Mio pour accéder aussi à la RAM au-delà de 4 Gio d’adresse. Sur 8 192 Mio, **8 176 Mio** sont testables ; **16 Mio** sont protégés pour les données de retour, les tables SMMU et le framebuffer. Les pages du framebuffer recouvrent deux fenêtres. Le compte rendu indique séparément la capacité, la zone testable et la zone vérifiée. Le test rapide ne réalise que le motif d’adresse ; le test avancé réalise les sept motifs, dont PRBS et rétention. Une interruption n’est pas un résultat PASS.

## État de validation

Les compilations ARM, la transmission du drapeau, les profils de sélection, les adresses jusqu’à 0x280000000, les sept motifs sur des fenêtres représentatives, les erreurs injectées et les rapports ont été contrôlés sur PC. Cela ne valide pas électriquement une console ni ses puces de remplacement. **Le support 8 Go reste expérimental sur chaque modèle.**

Le profil Mariko est celui des tables 8 Go amont. Erista n’a pas de table Atmosphère 8 Go distincte : le pont sélectionne le profil Samsung compatible 6 Go employé par l’ajustement DRAM amont, tout en conservant la géométrie 8 Go initialisée par Hekate et en contrôlant sa capacité après entraînement. Ce choix nécessite une validation réelle spécifique à Erista ; ne pas présenter ce pack comme certifié pour toutes les puces RAM.

## Références

- [Hekate : memmode et payloads RAM 8GB](https://github.com/CTCaer/hekate/releases/tag/v6.5.2)
- [Atmosphère 1.11.0 : modes mémoire au-delà de 4 Go](https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.11.0)
- [Ajustement DRAM et modes mémoire Atmosphère](https://github.com/Atmosphere-NX/Atmosphere/pull/2748)
- [HWTest-RCM de Creased](https://github.com/Creased/HWTest-RCM)
