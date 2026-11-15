# Hekate Nova Pro 22
**Languages:** **Français** · [English](LIRE-MOI-V22.en.md) · [Español](LIRE-MOI-V22.es.md) · [Русский](LIRE-MOI-V22.ru.md)

Base fonctionnelle actualisée : Hekate 6.5.5 / Nyx 1.9.5, avec le style Nova.
Cette version corrige l’avertissement Nyx erroné et plusieurs blocages possibles
des commandes sans fil. La liaison Bluetooth reste expérimentale.

## Installer

1. Conserve une copie de ton dossier bootloader actuel.
2. Copie le dossier bootloader fourni à la racine de la carte SD et accepte
   le remplacement des fichiers. Il n’est pas nécessaire de supprimer le dossier.
3. Injecte hekate_nova_pro_6.5.5_v22.bin, ou utilise le bootloader/update.bin fourni.
4. Vérifie « 22 » dans les crédits et « Hekate 6.5.5 / Nyx 1.9.5 ».

Les fichiers personnels nyx.ini et hekate_ipl.ini ne sont pas fournis dans ce
paquet. Tes réglages, thèmes, fonds, polices et configurations de démarrage sont
conservés. Remplacer seulement le payload sans mettre à jour nyx.bin ne suffit pas.

## Avertissement « Old Nyx GUI found »

La v21 livrée plaçait les métadonnées Nyx à une mauvaise position dans le binaire.
Cela déclenchait un faux avertissement même lorsque tous les bons fichiers étaient
présents. L’entrée de Nyx est maintenant isolée, et la compilation est refusée si
le placement des métadonnées change. Le contrôle de compatibilité de Hekate reste
actif : un réel mélange de versions peut toujours déclencher cet avertissement.

## Mise à jour Hekate / Nyx

Les changements officiels relatifs à HOS 23.5.0 sont repris dans le cœur de Nova,
avec le correctif NOGC pour FAT32 publié après le tag 6.5.5. Les panneaux, icônes,
couleurs et dispositions Nova restent ceux de la version précédente.

Le module emuMMC et le composant L4T sc7exit_b01.bin proviennent du paquet officiel
6.5.5 actualisé dont l’empreinte a été contrôlée. Nyx et le payload restent les
binaires Nova compilés pour cette version, et ne sont pas remplacés par l’interface
officielle. Les ressources Nova et les diagnostics sont conservés.

Références du portage :
- [Publication Hekate 6.5.5 / Nyx 1.9.5](https://github.com/CTCaer/hekate/releases/tag/v6.5.5)
- Code du tag : 9593272c54f8dc127a5a25f91e1ac90e1be6205d.
- Correctif FAT32 final : 8e1392df4569ac501a115867056b13c361429e9e.

## Joy-Con sans fil

Dans Réglages Nyx → TV / Dock → Joy-Con sans fil :

1. Attache les Joy-Con déjà appairés à cette console, puis choisis Lire les appairages.
2. Vérifie le nombre importé, détache les Joy-Con et choisis Activer Bluetooth.
3. Réveille les Joy-Con avec un bouton, laisse les sticks au repos un instant,
   puis déplace le stick et vérifie A / ZL / ZR.

Les clés restent en mémoire pendant la session. Elles ne sont pas écrites sur SD
et les appairages stockés dans les Joy-Con ne sont pas modifiés. Le service reste
désactivé à chaque démarrage. Il est arrêté avant les diagnostics, le mode USB et
le lancement d’un autre système ; relance-le manuellement après un diagnostic.

Corrections de navigation :
- Si le côté choisi est absent, le stick de l’autre Joy-Con connecté peut piloter
  le curseur. Le réglage enregistré du côté préféré est conservé.
- A / ZL / ZR réveillent le curseur masqué et restent utilisables pendant le centrage.
- Les rapports simples 0x3F sont décodés, en plus de 0x21 et 0x30. La confirmation
  du mode complet est suivie, et les commandes anciennes sont relâchées.

Les compteurs HCI seuls ne prouvent pas que les boutons sont reçus. Une ligne
d’état supplémentaire permet désormais de distinguer :

| Champ | Lecture |
| --- | --- |
| Rapports Joy-Con | Nombre de rapports réellement décodés, gauche / droit |
| ID | Type de rapport, généralement 30 en mode complet, 3F en mode simple |
| HID | 1 lorsque la connexion manette est ouverte |
| ACK | 1 lorsque le mode complet est confirmé ou qu’un rapport 30 est reçu |
| ERR | Dernier code d’échec d’ouverture de la connexion, 00 en l’absence d’erreur enregistrée |
| BTN | Boutons décodés ; la valeur doit changer pendant un appui |
| XY | Valeurs des sticks ; environ 2048,2048 au repos en mode complet |

Si les commandes ne réagissent toujours pas, relève cette ligne en maintenant A,
puis en déplaçant le stick. Cela permet de séparer un problème de connexion, de
décodage et de navigation. Aucun échec du prototype ne constitue à lui seul un
diagnostic de panne matérielle.

## Vérifications et limites

La compilation ARM, les positions et versions des en-têtes, les vrais chemins
de décodage / fusion / pointeur jusqu’au clic LVGL, les menus et les régressions
TV sont contrôlés sur PC avec entrées matérielles substituées. Deux blocages de
navigation ont été reproduits sur le code v21 avant de vérifier les corrections.
Le programme de vérification refuse également une entrée Nyx qui décale l’en-tête.

Une validation physique de cette v22 sur ta console reste nécessaire : les tests
PC ne prouvent pas une connexion radio, un démarrage HOS ou une sortie HDMI réelle.
Les réglages du pilote vidéo et la détection du dock restent ceux de Nova v20/v21.

## Licences

Cette version conserve les crédits et licences des composants. BTstack est soumis
à la licence personnelle et non commerciale BlueKitchen jointe au paquet ; tous
les composants ne sont donc pas présentés comme étant sous GPL. Aucun micrologiciel
Bluetooth propriétaire n’est fourni. Voir aussi les documents des versions précédentes.

Référence du format simple Joy-Con : [pilote SDL Switch](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_switch.c),
pour les orientations gauche/droite en utilisation verticale. Aucun code SDL n’est
incorporé à la pile ; le décodeur Nova utilise les formats documentés.
