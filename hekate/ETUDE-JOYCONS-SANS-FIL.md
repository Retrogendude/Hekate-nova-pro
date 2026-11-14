> Historique v20 : cette étude précède le prototype 21-BT. Pour les fonctions désormais intégrées et leurs limites, consulter LIRE-MOI-V21-BT.md.

# Joy-Con sans fil directement dans Hekate : état et piste de développement
**Languages:** **Français** · [English](ETUDE-JOYCONS-SANS-FIL.en.md) · [Español](ETUDE-JOYCONS-SANS-FIL.es.md) · [Русский](ETUDE-JOYCONS-SANS-FIL.ru.md)

La fonction est envisageable, mais elle n'est pas disponible dans cette
livraison. Le travail nécessaire dépasse l'ajout d'un interrupteur Bluetooth.
Aucun bouton de connexion sans fil factice n'a été ajouté.

## Ce que le projet contient déjà

- Le pilote `bdk/input/joycon.c` utilise les UART B/C pour les Joy-Con sur les
  rails. Les messages « HCI » envoyés aux manettes ne constituent pas une pile
  Bluetooth fonctionnant sur la console.
- Les réglages Nyx savent exporter des informations d'appairage Joy-Con.
  Cet export sert notamment à d'autres systèmes et n'active pas le sans-fil
  dans Nyx. [Source Hekate](https://github.com/CTCaer/hekate/blob/master/bdk/input/joycon.c).
- Le test matériel local `diagnostics/vendor/probe_bt.c` sait vérifier que la
  radio BCM/CYW4356 répond à des commandes HCI par UART-D. Il effectue une
  préparation matérielle puis un diagnostic, avec restauration par le cadre
  des tests. Il ne gère ni connexion de manette, ni réception HID continue.
  Ce test ne doit pas devenir une tâche répétée pour piloter les Joy-Con.

## Ce qu'il faut encore développer et valider

1. Démarrer la radio une seule fois pour une session, traiter son éventuel
   micrologiciel d'initialisation, puis recevoir les événements Bluetooth sans
   bloquer le menu. Vérifier en parallèle la coexistence avec l'affichage TV.
2. Gérer les connexions Bluetooth Classic, les canaux HID, les informations
   d'appairage et les reconnexions. Préserver les associations existantes et
   ne jamais placer leurs clés dans les journaux de diagnostic.
3. Décoder les boutons et joysticks des deux Joy-Con, appliquer leurs
   calibrations, puis alimenter les commandes Nyx existantes. Les rapports
   Bluetooth Joy-Con sont documentés, ce qui fournit une base de travail.
   [Notes techniques de dekuNukem](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/bluetooth_hid_notes.md).
4. Vérifier les déconnexions, le retour aux commandes sur rails, les diagnostics,
   la mémoire disponible et le passage vers un système ou un autre payload.
   L'arrêt du service doit libérer ses ressources avant ce passage.

Une piste est [BTstack](https://github.com/bluekitchen/btstack), qui propose
un fonctionnement embarqué sans système d'exploitation, un transport H4 UART
et un exemple d'hôte HID. Son intégration sur la Switch reste à faire ; il ne
suffit pas de compiler la bibliothèque. Voir aussi sa
[documentation des puces](https://github.com/bluekitchen/btstack/blob/master/chipset/README.md)
et ses [conditions de licence](https://github.com/bluekitchen/btstack/blob/master/LICENSE),
à examiner avant son adoption ou une redistribution du projet.

## Conclusion pour cette version

La détection automatique du dock est implémentée indépendamment du Bluetooth.
La commande de Nyx avec des Joy-Con détachés reste un développement distinct.
La première preuve à obtenir serait une connexion stable d'une manette avec
réception réelle de ses boutons, avant de présenter la fonction comme utilisable.
Un résultat « radio HCI : PASS » ne constitue pas cette preuve.
