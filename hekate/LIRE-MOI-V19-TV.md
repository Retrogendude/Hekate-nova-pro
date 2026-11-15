# Hekate Nova Pro 19-TV — première intégration de la sortie TV
**Languages:** **Français** · [English](LIRE-MOI-V19-TV.en.md) · [Español](LIRE-MOI-V19-TV.es.md) · [Русский](LIRE-MOI-V19-TV.ru.md)

Cette version reprend Nova Pro v18 et ajoute une sortie TV manuelle qui reste
active dans les menus. Base Hekate 6.5.4 / Nyx 1.9.4, interface Nova préservée.
Le prototype autonome 01, basé sur 6.5.5, reste disponible séparément.

## Ce qui a été validé

Tu as confirmé une image stable et l'arrêt par VOL- avec le prototype 01 sur
ta Switch OLED insérée directement dans un dock officiel. Les échecs rapportés
utilisaient un câble entre la console et le dock. Ils ne sont donc pas considérés
comme une défaillance confirmée du pilote en branchement direct. Le type exact
de ce câble et son branchement n'ont pas été identifiés.

Cette intégration Nova est nouvelle : elle doit encore être essayée sur console.
Ses tests PC ne remplacent pas cet essai. Le pilote vidéo et ses paramètres
restent identiques à ceux du prototype 01 ; les transitions dans Nova sont nouvelles.

## Installation

1. Sauvegarde ton dossier `bootloader` actuel, ou utilise une carte SD d'essai.
2. Copie à la racine le dossier `bootloader` fourni dans cette archive.
   Les fichiers de configuration personnels `hekate_ipl.ini` et `nyx.ini`
   ne sont pas fournis, pour conserver tes préférences.
3. Injecte `hekate_nova_pro_6.5.4_v19_tv.bin`. Vérifie dans les crédits que la
   version Nova indiquée est **19-TV**. Copier seulement le payload sans les
   fichiers de `bootloader/sys` ne met pas à jour Nyx.

## Activer la TV et revenir aux menus

1. Raccorde le dock officiel à son alimentation et au téléviseur en HDMI.
2. Ouvre **Réglages Nyx → TV / Dock → TV USB-C / Dock**.
   Le bouton de réglages rapide en haut à droite ouvre les réglages Nyx.
3. Choisis **Activer la TV** et insère directement la console dans le dock.
   Tu peux lancer hors du dock : tu disposes de 45 secondes pour la mettre en place.
4. Après connexion, ferme cette fenêtre. La sortie TV reste active sur l'accueil,
   les onglets et les fenêtres de Nova. Le mode est 1280 × 720 à 60 Hz.
5. **VOL-** arrête la sortie TV à tout moment entre les phases d'initialisation.
   Le bouton **Arrêter la TV** est aussi disponible en rouvrant ses réglages.
6. Retirer la console ou perdre le signal du dock arrête la session. Pour la
   reprendre, replace la console et utilise de nouveau **Activer la TV**.

L'écran de la console reste allumé. Les commandes Nyx existantes sont conservées,
dont le pointeur avec les Joy-Con sur les rails lorsque cette option est active.
Cette version n'ajoute pas la connexion Bluetooth des manettes détachées.

## Arrêt avant une autre fonction matérielle

Nova coupe la sortie TV avant un lancement de système ou de payload, un
rechargement, un arrêt/redémarrage, un diagnostic matériel actif ou un usage
USB de type stockage/manette. Le tampon TV est libéré après l'arrêt du moteur
d'affichage. La fréquence du menu est rétablie après la négociation du dock.

Un diagnostic actif peut donc revenir sur l'écran de la console. Le résultat
reste accessible dans Nova ; tu peux ensuite réactiver la TV manuellement.
Le test de mémoire garde son payload séparé et son retour existant à Hekate.

L'audio HDMI, l'activation automatique au démarrage et le branchement avec
câble intermédiaire ne sont pas validés par cette livraison.

## Rapports

Les rapports sont écrits sur la SD dans `bootloader/tv-test/` :

- `last-run.txt` : dernier essai ;
- `run-001.txt`, `run-002.txt`, etc. : copie distincte de chaque essai, jusqu'à 999.

Une erreur de ma part dans les prototypes 01/02 empêchait l'écriture normale
des rapports : le résultat de `sd_mount()` était interprété à l'envers.
Cette version utilise le contrat réel du BDK, vérifié séparément sur PC.
Il n'est donc pas nécessaire de rechercher un journal manquant du prototype 01.

Pour valider cette intégration, vérifie l'image à l'accueil après fermeture de
la fenêtre TV, l'ouverture de quelques sous-menus, VOL-, puis une reconnexion.
En cas de problème, joins le rapport de l'essai avec ce qui apparaît à l'écran.

## Provenance et retour

L'archive de sources contient les fichiers exacts utilisés, les tests, leurs
résultats, la provenance des pilotes et la licence Hekate. Voir
`documentation/TV-SOURCES.md` avant une éventuelle redistribution publique.

Nova v18 et les prototypes 01/02 sont conservés dans leurs archives précédentes.
Pour revenir en arrière, restaure le dossier `bootloader` sauvegardé et lance
le payload correspondant. Aucune modification de ton dossier source `Desktop/nd5`
n'a été effectuée pendant ce développement.
