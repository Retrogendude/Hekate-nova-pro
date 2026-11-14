# Hekate Nova Pro — reflet v14

Le reflet blanc traverse toujours la tuile entière, avec une bordure fixe. Cette mise à jour le rend plus large, plus doux sur les bords et plus rapide.

- Largeur de la bande : 129 pixels, contre 45 pixels dans la v13, soit environ trois fois plus large.
- Transparence : profil arrondi continu, blanc léger au centre puis complètement transparent aux deux extrémités. Les faibles opacités restent présentes ; elles ne sont plus coupées par le seuil du remplissage uniforme de LVGL.
- Passage : environ 0,8 seconde, contre 1,2 seconde en v13.
- Repos : environ 2,4 secondes. Le cycle complet dure 3,2 secondes.

Le dessin utilise une seule ligne de dégradé de 516 octets, réutilisée à chaque hauteur. Il ne crée pas d’image de la taille de la tuile et n’alloue rien pendant l’animation. Six petites zones suivent les anciennes et nouvelles positions du reflet. L’effet garde une priorité basse et se met en attente pendant les interactions et les diagnostics. Le réglage Effets de sélection permet toujours de le désactiver.

Seul gui_nova_reflection.c a changé par rapport aux sources livrées en v13. La correction d’accès aux menus avec fond d’écran, le réglage d’heure d’origine avec la langue et les réglages sauvegardés sont conservés.

Les vérifications de cette mise à jour portent sur 99 vues natives avec fond et transparence, 21 séquences de navigation par appuis/relâchements dans les sept langues, et le rendu des pixels du dégradé sur un fond uni puis sur un fond d’écran. Le profil est comparé à une courbe de transparence de référence : largeur, symétrie, transparence aux extrémités, petites opacités et limites de la tuile. Les binaires ARM sont comparés à leur ELF, leurs tailles sont vérifiées et les 42 sondes sont toujours liées. Le démarrage Hekate et le compagnon RAM sont identiques à la v13. Le rapport complet de la v13 est fourni séparément comme historique ; ses 4 407 vues ne sont pas comptées comme des essais rejoués ici.

Les aperçus et le GIF viennent du vrai rendu LVGL. Les mesures de zones redessinées sont faites sur PC, pas en FPS sur la console. **La fluidité sur Switch reste à confirmer sur l’appareil.**

Pour installer, sauvegarder les fichiers actuels, fusionner le dossier bootloader et remplacer bootloader/sys/nyx.bin avec celui de la v14. Utiliser le payload fourni avec la méthode habituelle. Aucun INI n’est inclus ; aucun nouveau logiciel ou SDK n’est nécessaire pour utiliser cette version.
