# Hekate Nova Pro25 — essai de reconnexion des Joy-Con
**Languages:** **Français** · [English](LIRE-MOI-V25.en.md) · [Español](LIRE-MOI-V25.es.md) · [Русский](LIRE-MOI-V25.ru.md)

Base conservée : Hekate6.5.5 / Nyx1.9.5. Interface et réglages Nova conservés.

Le rapport v24 confirme une authentification réussie pour une connexion, puis
une perte de liaison pendant l’activation du chiffrement (08). La correction
du format des clés de la v24 est conservée. Il reste un problème de connexion.

Cette version demande que la console assure le rôle principal quand un Joy-Con
initie la reconnexion, et lance les tentatives sortantes successivement.
Ces changements sont vérifiés sur PC. Leur efficacité sur la console reste
à confirmer : le rapport seul ne permet pas d’attribuer avec certitude la
coupure au rôle Bluetooth.

## Installer et essayer

1. Copier le contenu du ZIP à la racine de la SD, en remplaçant les fichiers
   du dossier **bootloader**. Les fichiers de réglages ne sont pas fournis.
2. Utiliser **hekate_nova_pro_6.5.5_v25.bin** si nécessaire, puis redémarrer
   complètement la console.
3. Ouvrir **Réglages → TV / Dock → Joy-Con sans fil**. Attacher les Joy-Con,
   choisir **Lire les appairages**, puis les détacher et activer Bluetooth.
4. Appuyer sur une touche pour les réveiller et essayer les boutons/sticks.
5. Choisir **Arrêter Bluetooth** après l’essai. Le rapport se trouve dans
   `bootloader/bluetooth/last-session.txt`.

Ne pas utiliser uniquement l’état des LED comme critère : les compteurs de
rapports et le fonctionnement des commandes sont les indicateurs utiles.
Les clés restent en mémoire et doivent être relues après chaque redémarrage.

## Nouveau rapport

- AUTH=00 SEEN=1 : un événement confirme une authentification réussie.
- ENC=00 SEEN=1 ENABLED=1 ou2 : chiffrement activé tant que la liaison est active.
- ENC=08 SEEN=1 : la liaison a expiré pendant cette étape.
- DISC : raison de déconnexion conservée.
- ROLE=00 : rôle principal connu de la pile ;01 : secondaire ;FF : inconnu.
  Les changements de rôle reçus apparaissent dans la trace13 ; la politique
  demandée à l’acceptation apparaît dans la trace14.
- HOST=1 : identité du contrôleur conforme à celle des appairages importés.
- SEEN=0 : aucun événement correspondant observé ;00 seul ne prouve pas un succès.

Les derniers résultats restent disponibles après Arrêter, même si l’on appuie
une seconde fois. Le prochain démarrage du Bluetooth ouvre une nouvelle session.
Le rapport inclut la version du contrôleur, sans adresses, clés ni paquets bruts.

La sécurité du lien est conservée. Aucun nouvel appairage et aucune écriture de
clé ne sont effectués. Les précédents correctifs Nyx, dock et diagnostics sont
conservés. La licence propre de BTstack est incluse dans la documentation.
