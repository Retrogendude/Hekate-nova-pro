# Sortie TV de Hekate Nova Pro — dock officiel

Périmètre confirmé : Nintendo Switch V1/V2 et OLED, dock officiel. La Switch Lite est exclue. Objectif initial proposé : interface Nova en 1280 × 720 à 60 Hz, avec retour à l’écran intégré au retrait du dock.

**État : étude du code effectuée ; la sortie TV n’est pas implémentée dans la v18.** Aucun réglage présenté comme fonctionnel n’est ajouté pour l’instant.

## Éléments présents dans les sources

- `bdk/display/di.c` initialise l’affichage interne et ses fenêtres ; `bdk/display/vic.c` fournit des fonctions de composition.
- `bdk/power/bm92t36.c` lit l’identification et les informations de la source USB-PD. Il définit notamment le registre `DP_STATUS`, mais n’implémente pas la négociation du mode vidéo avec le dock.
- `bdk/soc/t210.h` définit les adresses SOR1 et DPAUX1 ; ces définitions ne constituent pas un pilote vidéo externe.
- Les contrôles USB-PD existants permettent de capturer connexion, alimentation et informations du contrôleur. Un contrat d’alimentation valide ne prouve pas que la liaison vidéo est établie.

## Développement à réaliser

1. Capturer les états du contrôleur USB-PD avec et sans dock officiel sur V1/V2 puis OLED. Identifier précisément les états de négociation et les transitions de branchement. Conserver des délais bornés et le retour à l’affichage interne en cas d’échec.
2. Implémenter les échanges de négociation DisplayPort avec le dock et la configuration des voies correspondantes. Vérifier les commandes spécifiques au contrôleur de la Switch à partir d’une documentation ou d’un pilote compatible ; ne pas deviner les valeurs des registres.
3. Porter les opérations DPAUX nécessaires : détection de l’écran, lecture EDID/DPCD et gestion de connexion. Mettre en place les horloges SOR1, la négociation du lien et les paramètres du mode 720p.
4. Ajouter une présentation de l’image Nyx en orientation paysage pour la sortie externe, avec buffers dédiés et rotation/composition contrôlée. Conserver le moteur de menus et éviter de redessiner tout l’écran lorsque seule une tuile change.
5. Traiter le retrait du dock, l’échec de négociation, la veille et le lancement d’un payload ou du système. Restaurer les périphériques avant de passer la main.
6. Définir la commande de l’interface une fois la console dans le dock : Joy-Con détachés ou périphérique d’entrée USB. Les pilotes USB actuellement utilisés par les outils UMS/HID sont des fonctions périphériques et ne suffisent pas à lire une souris branchée au dock.

La validation doit inclure au moins un modèle LCD et un OLED, deux téléviseurs, des branchements répétés et le retour à l’écran interne. Les tests du moteur graphique sur PC ne peuvent pas valider la négociation électrique et le lien vidéo du dock.

## Références techniques primaires consultées

- [Hekate, dépôt d’origine](https://github.com/CTCaer/hekate).
- [Pilote Tegra SOR de Linux](https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/tegra/sor.c) et [pilote DPAUX](https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/tegra/dpaux.c) : références pour la structure des opérations, à adapter à l’environnement autonome de Hekate.
- [Pilote USB-C DisplayPort Alternate Mode de Linux](https://github.com/torvalds/linux/blob/master/drivers/usb/typec/altmodes/displayport.c) : négociation du mode vidéo et états de connexion.
- [Nintendo : modes de jeu et connexion au téléviseur](https://www.nintendo.com/sg/support/switch/playmode/index.html).

Ces références expliquent les blocs nécessaires ; elles ne prouvent pas qu’un portage compilé fonctionnerait directement sur la Switch ou son dock.
