# Hekate Nova Pro26 — correction des ouvertures simultanées des canaux Joy-Con

Base conservée : Hekate6.5.5 / Nyx1.9.5. Interface et réglages Nova conservés.

Ton rapport v25 confirme des authentifications et des activations du chiffrement
réussies, ainsi que les premiers échanges ACL. Le blocage suivant est visible :
Nova et le Joy-Con demandent simultanément l’ouverture du canal manette, puis
Nova refuse la demande entrante avec0004. Aucun rapport de boutons n’arrive.

La v26 accepte la demande entrante lorsqu’une tentative sortante est encore en
attente. Elle conserve la même session manette et isole les réponses tardives de
l’ancienne tentative. Les doublons sont contrôlés et les anciens canaux nettoyés.
La sécurité, le format des clés et le réglage du rôle principal sont conservés.

Le refus a été reproduit sur PC avec le code v25. Les tests v26 passent pour les
ouvertures simultanées, les commandes, les doublons, les réponses tardives et le
nettoyage. Le fonctionnement réel reste à confirmer sur ta console.

## Installer et essayer

1. Copier le contenu du ZIP à la racine de la SD en remplaçant les fichiers du
   dossier **bootloader**. Les fichiers de réglages ne sont pas fournis.
2. Utiliser **hekate_nova_pro_6.5.5_v26.bin** si nécessaire, puis redémarrer
   complètement.
3. Ouvrir **Réglages → TV / Dock → Joy-Con sans fil**. Attacher les Joy-Con et
   choisir **Lire les appairages**.
4. Détacher les Joy-Con, activer le Bluetooth et appuyer sur une touche pour les
   réveiller. Essayer les sticks et les boutons A / ZL / ZR.
5. Choisir **Arrêter Bluetooth** après l’essai. Le rapport reste enregistré dans
   `bootloader/bluetooth/last-session.txt`.

Les compteurs **Rapports**, **ACK** et les commandes effectivement reçues sont
les indicateurs utiles. Après l’arrêt, HID et ENABLED peuvent valoir0 : le
service est coupé. Les compteurs et les derniers événements restent conservés.
SEEN=1 indique qu’un événement a été observé ;00 avec SEEN=0 ne prouve pas un
succès. Les adresses, clés et contenus bruts des paquets ne sont pas enregistrés.

Le prochain démarrage du Bluetooth remet le rapport de session à zéro. Les
clés restent en mémoire ; elles doivent être relues après un redémarrage.
La licence distincte de BTstack et le détail des modifications sont inclus.
