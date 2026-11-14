# Hekate Nova Pro — version 11

Cette version applique les douze modifications consignées dans MODIFICATIONS-NOVA-PRO.md. La maquette Nova reste la base, avec les changements expressément demandés : commandes directes, reflet, suppression des badges, nouvelles interactions et personnalisation.

Les onglets occupent la barre supérieure ; les fenêtres restent entre 76 et 656 pixels, au-dessus du bandeau d'état. Les commandes de la barre supérieure sont indisponibles pendant l'ouverture d'une fenêtre, puis retrouvent leur état à sa fermeture. Les cartes gardent des angles de 8 pixels, une palette anthracite et des pictogrammes fins. Les pages de sauvegarde/restauration, USB, emuMMC, diagnostic, apparence et crédits reprennent ces composants. Les rapports techniques restent consultables dans leur fenêtre Nova.

Le premier appui sur une tuile d'Outils, Infos ou Diagnostic la sélectionne. Le second appui sur cette même tuile ouvre sa fonction. Une tuile présélectionnée demande également deux appuis après l'ouverture du menu. Les actions spéciales par appui long, les confirmations et les restrictions des fonctions d'origine sont conservées.

Le reflet est dessiné toutes les 80 ms sur de petits segments des contours visibles. Seules les zones correspondant à ses anciennes et nouvelles positions sont rafraîchies. Les surfaces masquées sont ignorées ; le reflet s'interrompt pendant les diagnostics et les interactions. Les vagues, particules et aurores ont été retirées du code et de l'interface. Le champ historique nova_halo commande désormais le reflet.

Le fond, le texte, les contours et le reflet possèdent quatre couleurs indépendantes. Deux réglages d'opacité concernent uniquement les fonds des menus et des cadres d'icônes. L'éditeur présente un aperçu, des valeurs RGB et des échantillons. Fermer annule les choix ; Enregistrer et recharger les applique. Les réglages de langue et de police restent distincts.

Les fonds d'écran sont fixes, au format BMP 24/32 bits non compressé, dans /wallpapers. Le décodeur vérifie les dimensions, les tailles, l'orientation et les accès au fichier. Le recadrage et la mise à l'échelle ont lieu au chargement, sans accès SD pendant le dessin. Une seule ligne source et l'image finale sont allouées. L'image finale en 1280 × 720 occupe environ 3,5 Mio uniquement lorsqu'un fond est choisi. Le convertisseur fourni prépare les images PNG/JPG personnelles ; Nova-Carbon est livré prêt à choisir.

Le démarrage présente le monogramme original Nova Pro et le nom Hekate Nova Pro. La nouvelle animation utilise une barre et trois points ; le délai de démarrage et l'annulation par VOL− sont conservés. Le nouveau payload fourni doit être lancé pour bénéficier de cette présentation. Le chargeur est reconstruit lorsque les données Hekate compressées changent ; la vérification finale décompresse le véritable payload et compare son contenu au binaire actuel.

La compilation et les vérifications portent sur le logiciel. Les rendus utilisent le véritable LVGL avec les actions matérielles remplacées par des valeurs de démonstration. Ils ne constituent pas une mesure de fluidité sur console. La validation physique sur Switch reste nécessaire, notamment pour les diagnostics et le retour du service RAM.
