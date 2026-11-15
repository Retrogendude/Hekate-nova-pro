# Hekate Nova Pro — correctif v13
**Languages:** **Français** · [English](CORRECTIONS-NOVA-PRO-V13.en.md) · [Español](CORRECTIONS-NOVA-PRO-V13.es.md) · [Русский](CORRECTIONS-NOVA-PRO-V13.ru.md)

Cette version corrige le blocage des menus après activation d’un fond d’écran et remplace le reflet du symbole par un reflet sur toute la tuile sélectionnée.

## Fond d’écran et accès aux fenêtres

Le défaut a été reproduit avant correction dans le vrai code LVGL : après ouverture de la fenêtre Couleurs avec un fond actif, le premier élément de la liste des enfants avait un prédécesseur. Le contenu de la fenêtre n’était plus accessible par le parcours utilisé pour trouver le bouton touché, alors que le dessin pouvait encore le parcourir dans l’autre sens. Les anciens essais d’images ne vérifiaient pas cet invariant.

La fonction lv_ll_move_before met maintenant aussi à jour la tête de liste lorsqu’un élément passe devant le premier. C’est le cas lors du placement du contenu d’une fenêtre devant son fond. Les fonds restent protégés, sans interaction et derrière le contenu. La correction porte sur l’ordre interne des objets et leur accès au toucher.

Les contrôles ajoutés vérifient la cohérence des deux sens du parcours sur tout l’arbre des fenêtres. Un test de la fonction C réelle compare 316 646 déplacements à un modèle indépendant. D’autres essais envoient de véritables appuis et relâchements au pilote d’entrée LVGL : ouverture de Nyx, Couleurs, choix d’une couleur, fermeture, ouverture d’Apparence, fermeture puis changement de chacun des onglets principaux. Chaque séquence est répétée trois fois, dans les sept langues, en 20, 30 et 36 pixels.

## Reflet sur la tuile entière

Une bande blanche diagonale traverse toute la surface de la tuile sélectionnée : fond, pictogramme et texte. Elle ne tourne pas sur les contours ; la bordure reste fixe. Elle s’applique aux tuiles de l’accueil, au carrousel et aux sélections des grilles Outils, Infos et Diagnostic.

Le passage dure environ 1,2 seconde, suivi d’environ 2,4 secondes de repos. Le blanc reste léger, avec plusieurs niveaux de transparence. Le dessin est effectué directement en petites bandes, sans image transparente de la taille de la tuile et sans allocation pendant l’animation. Quatre zones suivent l’ancienne et la nouvelle position du reflet pour éviter de redessiner systématiquement toute la tuile ou tout l’écran. L’effet se met en attente pendant les interactions et les diagnostics. Le bouton Effets de sélection permet de le couper.

## Installation et conservation des réglages

Fusionner le dossier bootloader de l’archive avec celui de la SD après sauvegarde. Utiliser le payload hekate_nova_pro_6.5.4_v13.bin fourni. Le nouveau fichier bootloader/sys/nyx.bin contient les corrections ; ne pas conserver celui de la v12. Aucun fichier nyx.ini ou hekate_ipl.ini n’est livré. Les couleurs, le fond, la langue et les polices déjà configurés sont conservés. Le réglage d’heure d’origine avec la langue ajoutée reste inchangé. Il n’est pas nécessaire d’installer un autre programme pour utiliser cette version.

## Validation

Compilation devkitARM et vérifications du code réel sur PC, avec les polices, les fonds et les transparences. Les essais incluent l’accès aux boutons, les ouvertures et fermetures de fenêtres superposées, les sauvegardes, les limites des binaires et les 42 sondes de diagnostic. Le démarrage Hekate et le compagnon RAM restent ceux de la v12 ; leurs binaires sont comparés pendant la préparation de la livraison.

Les résultats détaillés sont dans verification/. Les aperçus et le GIF sont issus du rendu LVGL réel. **Aucun essai de cette version sur Switch ni mesure de FPS de console n’a été effectué ici.** Vérifier sur l’appareil l’accès aux réglages avec le fond actif et la fluidité du reflet.
