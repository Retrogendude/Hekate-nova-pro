# Hekate Nova Pro — corrections v15

Les onze points de la liste ont été traités dans les sources et intégrés à la compilation. Les vérifications ci-dessous sont réalisées sur PC avec le moteur LVGL du projet. Les accès au matériel sont remplacés par des données de test dans ces aperçus ; ils ne constituent pas une sauvegarde, un partitionnement ou un diagnostic exécuté sur une Switch.

1. **Police des rapports corrigée.** Le redimensionnement utilisait la largeur de la cellule fixe comme largeur du bitmap de chaque caractère. Les glyphes étroits étaient lus avec un mauvais pas, ce qui reproduisait les caractères brouillés des captures. Le lecteur utilise maintenant la largeur réelle du bitmap. Les glyphes sont centrés dans une cellule fixe, y compris à la taille native de 20 px.
2. **Informations eMMC réalignées.** Chaque libellé et sa valeur partagent une hauteur de ligne calculée selon leur texte. Les propriétés et les partitions sont réparties sur deux onglets. Les partitions utilisent des pages Précédent/Suivant ; le tableau ne s'arrête plus après les vingt premières lignes.
3. **Sauvegarde et restauration traduites.** Le titre, l'espace libre, la taille, le dossier et le fichier passent par le catalogue des sept langues. Les fragments ajoutés au journal passent aussi par la traduction : fin, échec, annulation et erreurs. Les chemins, noms de fichiers, nombres et adresses sont conservés.
4. **Caractères techniques vérifiés.** Les chiffres, lettres ASCII et lettres cyrilliques sont contrôlés à partir des vrais bitmaps. Les accents et les sept langues sont vérifiés dans les vues techniques. La famille choisie pour les menus reste indépendante de la police fixe des rapports.
5. **Vues des captures reproduites.** Les informations eMMC, la progression de sauvegarde et l'avertissement MBR sont inclus dans les essais de langues, tailles et polices, ainsi que dans les essais de fond d'écran et de transparence. Les données affichées dans les aperçus sont simulées. Le journal reste dans une zone bornée ; les longues tables sont paginées.
6. **Aperçu des couleurs cumulatif.** Les couleurs provisoires du fond, des textes, des contours, du logo, des icônes et de la sélection restent présentes ensemble. Le logo et l'icône de réglages ont chacun leur couleur. Changer de cible ou d'onglet ne réinitialise plus l'aperçu. Enregistrer applique le brouillon ; fermer sans enregistrer conserve les réglages précédents.
7. **Reflet accéléré.** La bande traverse toute la tuile en environ **0,48 seconde**, contre 0,8 seconde en v14. Elle garde sa largeur de 129 px, ses extrémités transparentes et sa bordure fixe. Dix bandes de redessin bornent le travail. La pause de 2,4 secondes et la suspension pendant les interactions et diagnostics sont conservées. Aucune allocation n'est faite à chaque image.
8. **Chargement progressif.** La barre avance jusqu'à 100 % en six étapes animées, avec 1,8 seconde d'animation au total, en plus du temps réel de chargement. Les accès bloquants à la carte SD peuvent encore ajouter une pause entre ces étapes. Le délai d'autodémarrage, l'annulation par VOL− et le réglage initial d'heure d'origine avec choix de langue sont conservés.
9. **Colonnes emuMMC alignées.** Les panneaux de gestion partagent leurs limites haute et basse. Leur disposition ne dépend plus du placement automatique de l'ancienne fenêtre.
10. **Choix des fichiers emuMMC sans liste verticale.** Deux colonnes de même taille, titres centrés et quatre choix par page remplacent la liste déroulante. Précédent/Suivant donne accès aux choix suivants. Le choix sauvegarde toujours le chemin du dossier d'origine ; les partitions RAW indisponibles restent désactivées.
11. **Zone utile des sous-menus.** Les fenêtres utilisent l'espace entre le menu supérieur et le bandeau inférieur. Les boîtes centrées sont recentrées dans cette zone, avec adaptation de largeur et, si nécessaire, de la taille du texte. Les rapports eMMC, la progression et les sélecteurs corrigés ont des dimensions bornées.

## Vérifications

La matrice comprend 40 vues ou onglets, les sept langues et neuf tailles de 20 à 36 px avec Lecture, puis les trois autres familles intégrée, Lecture fixe et Lecture renforcée aux tailles 20, 30 et 36 px. Des essais supplémentaires couvrent le fond d'écran, trois niveaux de transparence, une palette personnalisée, les commandes accessibles et la navigation par appuis/relâchements.

Les contrôles portent aussi sur la conservation du brouillon de couleurs, l'accès à toutes les pages des rapports et des fichiers emuMMC, les liens de fenêtres LVGL, l'enregistrement des réglages, le chargement des polices et le convertisseur Caviar corrigé. Le profil de transparence du reflet est comparé aux pixels réellement dessinés. Les contrôles de démarrage vérifient le remplissage monotone, la rotation d'écran, le délai et l'annulation.

Les binaires sont comparés aux exécutables ELF et aux limites de taille. Les 42 sondes sont toujours présentes, ainsi que le service RAM séparé et son mécanisme de retour. Les journaux et les nombres exacts sont inclus dans `verification/`.

**À vérifier sur la console :** démarrage à froid, lisibilité des trois vues sur les données réelles, ouverture des réglages avec le fond choisi, fluidité du reflet et navigation tactile. Aucune mesure de FPS sur Switch ni opération de stockage réelle n'a été effectuée ici.

## Installation

Sauvegarder les fichiers actuels de la carte SD. Fusionner le dossier `bootloader` fourni, puis utiliser le payload `hekate_nova_pro_6.5.4_v15.bin` avec la méthode habituelle. `bootloader/update.bin` contient ce même payload. Remplacer Nyx et le payload est nécessaire pour disposer de toutes les modifications du chargement.

Les dossiers `fonts`, `wallpapers` et les outils sont inclus. Aucun `nyx.ini` ni `hekate_ipl.ini` n'est fourni : les réglages existants ne sont pas remplacés. Aucun nouveau logiciel n'est nécessaire pour utiliser le paquet compilé.
