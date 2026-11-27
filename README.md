# Hekate Nova Pro

Personnalisation expérimentale de Hekate / Nyx : interface Nova, diagnostics matériels, personnalisation et prise en charge du dock et des Joy-Con sans fil. Projet non officiel ; les auteurs des composants d’origine conservent leurs droits et leur attribution.

## Télécharger

**[Préversion v34-alpha5 : fichiers et sources](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha5)** · [Toutes les versions](https://github.com/Retrogendude/Hekate-nova-pro/releases) · [Changelog complet](CHANGELOG.md) · [Notice d’utilisation](docs/NOTICE-UTILISATION.md)

Service TV pendant les opérations longues, journaux différés et tentatives limitées. Version TV historique : son contrôle de synchronisation trop strict est corrigé en alpha7. Les essais PC et retours console sont distingués dans les notes de chaque version.

![Halo à deux couleurs — rendu du code sur PC](docs/images/preview-v33.3/colors.png)

[Corrections v33.3 et v33.2](docs/changelogs/CHANGELOG-NOVA33.3-fr.md) · [Guide RAM 8 Go](docs/RAM-8GO-V33.1.md) · [Notice actualisée](docs/NOTICE-UTILISATION.md)


[Historique des nouvelles releases](docs/RELEASES-V33-V34.md) · [Changelogs en huit langues](docs/changelogs)

## Fonctions

- Accueil Nova en tuiles ou carrousel, réglages répartis en onglets et fenêtres adaptées aux bandeaux.
- Huit langues : anglais, français, espagnol, italien, russe, allemand, portugais et chinois simplifié ; polices SD et taille de texte réglable.
- Couleurs indépendantes, palette avec aperçu, fond d’écran, transparence, icônes PNG et reflet de sélection désactivable.
- Diagnostics natifs issus de HWTest-RCM : 42 entrées, rapports exportables, bilan par composant, contrôles interactifs et test RAM séparé avec retour à Hekate.
- Partitionnement simplifié / expert et assistant automatisé : modèle et choix des partitions, contrôle des capacités, copie/relecture emuMMC après partitionnement. Linux/Android : réservation des partitions ; leur installation reste distincte.
- Sortie TV via dock sur consoles compatibles : TV seule ou TV + écran console. La Switch Lite n’a pas de sortie TV.
- Joy-Con Bluetooth : lecture des appairages, sauvegarde privée SD, chargement automatique sans Joy-Con sur les rails et traitement en arrière-plan. Connexion et portée restent expérimentales.

## Installation

**[Ouvrir la notice complète](docs/NOTICE-UTILISATION.md)** : installation, premiers réglages, fonds d’écran, polices, icônes, diagnostics, dock et Joy-Con sans fil.

Télécharger le paquet compilé de la version choisie dans Releases. Console arrêtée, conserver une copie de bootloader puis fusionner les dossiers fournis avec la racine de la SD en remplaçant uniquement les fichiers livrés. Garder les configurations et appairages personnels. Utiliser le payload correspondant à la version, ou le bootloader/update.bin fourni selon la méthode de démarrage.

Les instructions propres à chaque version, ses limites et ses vérifications figurent dans docs/releases et dans son archive. Les versions v1 à v5 sont conservées sous forme de sources uniquement dans les fichiers disponibles ici.

L’installateur Windows copie le pack sur la lettre SD choisie ; il ne partitionne pas et n’injecte pas le payload. Les réglages et appairages existants sont conservés. Le partitionnement depuis Nova efface les données non sauvegardées.

## Sources et construction

Le dossier hekate contient les sources Hekate / Nyx de l’instantané sélectionné. Les archives de sources d’origine restent jointes à chaque release, avec les scripts de compilation et de vérification associés. Elles conservent leur disposition historique et constituent la référence pour reproduire une ancienne livraison.

Pour la v33.1, voir [BUILDING-V33.1.txt](BUILDING-V33.1.txt) : le diagnostic séparé utilise HWTest-RCM-nova33_1 et ses dépendances publiques HWTest-RCM-0.15. Dans le dépôt : entrer dans hekate puis utiliser les Makefiles du projet. Les outils requis comprennent devkitARM, devkitA64, GNU Make, Python 3 et un compilateur C natif pour les composants qui le nécessitent. Le compagnon HWTest-RCM-0.15 et les outils complémentaires sont conservés lorsqu’ils étaient présents dans l’archive. Voir BUILDING.txt et la notice de la version pour les particularités Windows.

Les répertoires de travail locaux, SDK et données de console ne sont pas importés. Les archives compilées sont des assets de Releases, pas des fichiers ZIP ajoutés à chaque commit Git.

## Historique et état des essais

Les instantanés v1 à v32 et deux essais TV autonomes restent conservés. La v33.1 publie les évolutions v33 et leurs correctifs dans une seule bêta. Les tags et commits ont été reconstitués à partir des archives lors de leur publication ; ils ne prétendent pas restituer les dates ou les commits originaux du développement. Une sauvegarde des sources v15 a été retrouvée et comparée aux empreintes des sources modifiées conservées dans les preuves de cette version ; les deux fichiers payload générés ne font pas partie de cette archive de sources.

Les aperçus natifs, tests à périphériques simulés et retours sur console sont distingués dans les notices. Un verdict PASS ne couvre que les contrôles réellement exécutés. Les résultats RAM, tactiles et écran ont leurs propres limites détaillées dans la documentation.

## Origines, crédits et licences

- [Hekate / Nyx — CTCaer](https://github.com/CTCaer/hekate) et contributeurs : licence originale conservée dans hekate/LICENSE ; README amont conservé dans hekate/README.md.
- [HWTest-RCM — Creased](https://github.com/Creased/HWTest-RCM) : sondes, compagnon et commentaires de provenance conservés dans les fichiers concernés. Voir hekate/nyx/nyx_gui/diagnostics/PROVENANCE.md.
- [BTstack — BlueKitchen](https://github.com/bluekitchen/btstack) : licence séparée conservée dans hekate/nyx/nyx_gui/libs/btstack/LICENSE, comprenant une restriction d’usage personnel et non commercial. Cette composante n’est pas présentée comme couverte par la seule licence GPL de Hekate.
- LodePNG, icônes, polices et autres composants conservent leurs propres notices dans les sources et les paquets.

Aucune affiliation avec Nintendo, CTCaer, Creased ou BlueKitchen n’est revendiquée. Les licences propres aux composants et les notices d’origine restent applicables.

## Rapports TV

La v34-alpha5 reste une préversion. Tester d’abord l’image permanente au démarrage puis après retrait/remise sur dock officiel. En cas de coupure, joindre les derniers `bootloader/tv-test/run-*.txt` et `last-run.txt`. L’alpha7 peut confirmer le flux par captures CRC fraîches lorsque l’indicateur de synchronisation du dock reste nul ; cela reste à valider sur console.

## Rapports Bluetooth

Pour signaler un problème, arrêter le service Bluetooth après l’essai et conserver bootloader/bluetooth/last-session.txt. Indiquer la version, le modèle, la distance et l’utilisation ou non d’un dock. **Ne pas publier les fichiers joycon-pairing.bin / .bak, prod.keys, title.keys ou des dumps personnels.**
