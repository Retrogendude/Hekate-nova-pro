# Hekate Nova Pro — bêta v31

Base : Hekate 6.5.5 / Nyx 1.9.5. Cette archive contient le dossier bootloader complet et le payload correspondant.

## Menu expert de partitionnement

- Trois réglages sur toute la largeur, récapitulatif horizontal en bas.
- Curseurs plus longs et boutons −/+ avec répétition sur appui maintenu.
- Tailles affichées en Go entiers, sans décimales. Repères « 29 Go Full » et « 58 Go Full », également pour les doubles emuMMC.
- Pas de modification des tailles réelles, réserves ou alignements : les Go affichés sont arrondis à l'entier inférieur, comme les repères de Hekate. Android affiche son allocation totale, système inclus. Les paliers emuMMC de Hekate sont conservés ; Linux et Android utilisent les pas de 1 Go après leur minimum.
- Les confirmations d'effacement existantes restent actives. La modification du menu ne lance aucun partitionnement.
- Thème Nova conservé, y compris le fond d'écran et la transparence.

## Bluetooth et fluidité

Le traitement Bluetooth continue pendant les longs dessins logiciels, les copies d'image et les attentes de rotation d'affichage. Le passage depuis le dessin est limité à un budget de réception de 250 µs, au plus une fois toutes les 2 ms ; le traitement d'un paquet entier peut dépasser ce budget. Il ne lance ni autre menu, ni tâche graphique, ni écriture sur la SD.

Le fonctionnement de v30 est conservé : démarrage automatique depuis le fichier d'appairage enregistré quand les deux rails sont vides au démarrage, sans ouvrir le menu Bluetooth. La présence d'un Joy-Con sur un rail empêche cet automatisme. Un arrêt manuel reste respecté jusqu'au prochain démarrage. Les diagnostics qui réservent les périphériques conservent leur fonctionnement dédié.

Cette version ajoute des mesures de liaison en lecture seule, espacées (au maximum deux requêtes par seconde au total). Aucun réglage de puissance, EDR, vitesse UART, appairage ou chiffrement n'a été changé. Aucun firmware HCD supplémentaire n'est chargé.

**La portée au-delà de 80 cm n'est pas encore validée sur console.** Le rapport v30 montre un maximum de 118,5 ms entre deux passages de traitement côté hôte et une interruption de réception plus longue à droite ; il ne prouve pas une panne radio ni la cause exacte de la distance. Cette bêta améliore les possibilités de traitement et permet de mesurer ce qui reste.

## Installation

1. Console arrêtée, conserver une copie de votre dossier bootloader et de vos fichiers personnels.
2. Fusionner le dossier bootloader de cette archive avec celui de la SD en remplaçant les fichiers fournis. Garder ensemble le nouveau Nyx et le payload correspondant.
3. Utiliser hekate_nova_pro_6.5.5_v31.bin si votre méthode de démarrage utilise un payload externe. bootloader/update.bin est également fourni.
4. Conserver bootloader/bluetooth/joycon-pairing.bin ou sa sauvegarde .bak : cette mise à jour ne change pas leur format. Ne pas effacer les clés ni nettoyer toute la SD pour cet essai.

Aucun fichier privé d'appairage et aucun fichier de configuration personnel ne sont inclus dans l'archive.

## Essai conseillé

À position de console identique, tester une session à proximité puis une autre au-delà de 80 cm. Dans chaque session, déplacer les sticks et presser A/B pendant environ 20 secondes. Choisir « Arrêter Bluetooth » et copier immédiatement bootloader/bluetooth/last-session.txt avant de relancer ou relire les appairages. Conserver les deux rapports sous des noms distincts.

Envoyer uniquement ces rapports texte ; les .bin/.bak d'appairage contiennent des clés privées.

Le rapport ajoute :
- Capture_ms et nombre de passages pendant le dessin.
- Full_reports, âge du dernier rapport complet, maximum entre rapports complets et compteurs de trous >50 / >250 ms. Les accusés de réception ne sont pas comptés comme rapports complets. Les statistiques de session ne constituent pas une mesure de latence bouton-écran.
- RF kind=0 : RSSI BR/EDR relatif en dB ; 0 signifie dans la plage du contrôleur, pas un signal nul. Ce n'est pas une mesure absolue en dBm.
- kind=1 : qualité fournisseur de 0 à 255, pas un pourcentage.
- kind=2/3 : puissance d'émission actuelle / maximale en dBm, seulement lue.
- kind=4 : compteur de contacts échoués consécutifs du contrôleur, pas un cumul de tous les paquets perdus.
- Pour chaque mesure : nombre d'échantillons, âge, dernière valeur et minimum/maximum de session. n/a signifie aucune mesure valide. Des valeurs anciennes ne décrivent pas nécessairement la liaison au moment de l'export.
- Erreurs de mesure : FD = valeur de puissance hors plage, FE = réponse absente, FF = réponse mal formée ou mauvais identifiant. Une mesure non prise en charge ou sans réponse est désactivée pour cette connexion ; elle n'arrête pas les commandes. Ces codes sont indépendants des erreurs d'authentification.

## Vérification

Compilation ARM et contrôles automatiques documentés dans VERIFICATION-V31.json. Les essais PC couvrent le rendu réel LVGL, les vrais callbacks du partitionnement, les échanges HCI/L2CAP/HID simulés et les chemins d'appairage existants. Les périphériques et la SD sont remplacés dans ces essais : aucune opération de partitionnement réelle et aucune mesure de portée physique n'ont été effectuées.
