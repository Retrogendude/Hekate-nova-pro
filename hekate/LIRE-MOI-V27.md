# Hekate Nova Pro27 — profil Bluetooth adapté de HWTest-RCM

La v27 reprend les réglages du service de manettes sans fil de HWTest-RCM :
débit radio de base, gestion de veille, rythme de transmission et lecture série
prolongée. Le démarrage utilise aussi ses réglages d’alimentation et d’horloge,
avec restauration de l’état précédent à l’arrêt.

L’interface Nova, les deux Joy-Con, les clés importées, la sécurité et les
correctifs de connexion précédents sont conservés. Les diagnostics Wi-Fi et
Bluetooth déjà intégrés restent en place. Base : Hekate6.5.5 / Nyx1.9.5.

Compilation et tests logiciels réussis. Le fonctionnement sur console reste
à confirmer. Le détail des adaptations figure dans HWTEST-BLUETOOTH-SOURCES.md.

1. Copier le ZIP à la racine de la SD en remplaçant les fichiers **bootloader**.
2. Utiliser **hekate_nova_pro_6.5.5_v27.bin** si nécessaire, puis redémarrer.
3. Attacher les Joy-Con et choisir **Lire les appairages** dans leur menu sans fil.
4. Les détacher, activer Bluetooth, appuyer sur une touche et tester les commandes.
5. Choisir **Arrêter Bluetooth** après l’essai pour enregistrer
   `bootloader/bluetooth/last-session.txt`.

Le rapport ajoute BR_ONLY, les changements de mode et des statistiques de
réception. Les configurations regroupées sont désormais toutes détaillées,
sans adresses, clés ni données brutes. Après Stop, HID/ENABLED peuvent valoir0 ;
les compteurs de rapports et les derniers résultats restent conservés.

Les réglages utilisateur ne sont pas fournis dans le ZIP. Les clés restent
en mémoire et doivent être relues après le redémarrage. Les licences existantes,
dont celle de BTstack, sont incluses.
