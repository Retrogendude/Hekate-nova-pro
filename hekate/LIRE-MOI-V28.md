# Hekate Nova Pro 28 — réactivité et diagnostic des appairages

Cette version réduit les attentes Bluetooth répétées pendant le dessin des
menus : la lecture série utilise un budget court de 250 microsecondes dans
les appels d’affichage et du minuteur de la fenêtre. La boucle principale
conserve la lecture prolongée nécessaire au profil radio HWTest-RCM.
Le panneau de statistiques se rafraîchit deux fois par seconde.
Cela vise les ralentissements signalés en v27 ; le résultat sur console
reste à mesurer. Il ne s’agit pas d’une garantie de fréquence d’affichage.

Le menu indique maintenant la catégorie d’échec de l’importation : rails
indisponibles, données non lues, données Nintendo non reconnues par le lecteur,
ou contrôles de compatibilité non satisfaits. Un échec 0/2 peut être enregistré
sans lancer le Bluetooth. Les validations et le format des clés restent ceux
de la v27 : **la cause du 0/2 observé sur console n’est pas encore corrigée**.

Le rapport ajoute des indicateurs d’importation et, pour chaque Joy-Con,
les boutons observés depuis le début de la session, les dates des premiers et
derniers rapports et leur cadence moyenne. Il ne contient ni adresse, ni clé,
ni contenu brut des paquets. « Buttons seen » conserve un appui même si le
bouton a déjà été relâché. Le bouton A du Joy-Con droit correspond au bit 000008.
La cadence moyenne porte sur les rapports décodés, y compris les réponses
de commande, et ne mesure pas la latence d’un appui.

Installation et essai pour le problème actuel :

1. Extraire le ZIP à la racine de la SD en remplaçant les fichiers du dossier
   bootloader. Utiliser hekate_nova_pro_6.5.5_v28.bin si nécessaire, puis redémarrer.
2. Fixer les deux Joy-Con et choisir « Lire les appairages ».
3. Si le résultat est encore 0/2, relever le nouveau message et choisir
   « Arrêter Bluetooth », même sans l’avoir démarré. Transmettre uniquement
   bootloader/bluetooth/last-session.txt.
4. Si l’importation réussit, détacher les Joy-Con, démarrer Bluetooth, déplacer
   le curseur avec L et appuyer plusieurs fois sur A du R. Arrêter ensuite
   Bluetooth et récupérer le rapport avant toute nouvelle session.

Les menus, le profil radio, la gestion des connexions simultanées, les
diagnostics et le mode TV sont conservés. Aucun réglage utilisateur n’est
fourni dans le ZIP. Aucun nouvel appairage ni changement de clés n’est ajouté.
Base : Hekate 6.5.5 / Nyx 1.9.5. Licences et provenance incluses.

Les vérifications sur PC et la compilation ne remplacent pas l’essai matériel.
En v27, un déplacement avec L a été signalé avec une forte latence, sans
validation avec A du R. Les derniers rapports fournis montrent une connexion
non établie, puis un import à 0/2 : ils ne décrivent pas cette session active.
