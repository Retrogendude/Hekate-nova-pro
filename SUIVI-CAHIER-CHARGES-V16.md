# Hekate Nova Pro 16 — suivi du cahier des charges

Version basée sur Hekate 6.5.4 / Nyx 1.9.4. La maquette reste en pause : les changements ci-dessous sont intégrés dans le code de Nyx.

## Réalisé

| Demande | Réalisation |
|---|---|
| Conserver le fonctionnement existant | Fonctions de création MBR/GPT, copie, sauvegarde/restauration, installation Linux/Android et correction des attributs conservées. Comparaison automatique avec les sources livrées en v15. |
| Crédits et version | Version Nova 16 issue du code ; Retrogen pour la conception et l’adaptation, ChatGPT pour l’assistance ; crédits Hekate/Nyx et licences conservés dans des onglets lisibles. |
| Sauvegarde eMMC | Disposition 2 × 2 alignée : BOOT0/BOOT1 et SYS à gauche, RAW GPP et USER à droite. |
| Restauration eMMC | BOOT0/BOOT1 et ALL à gauche, RAW GPP à droite ; mêmes actions qu’auparavant. |
| Gestionnaire de partitions | Zone fixe entre la barre supérieure et la barre inférieure ; commandes essentielles accessibles ; graphique proportionnel aux allocations réelles. |
| Mode simplifié | Détection et choix V1/V2/Lite/OLED. Vérification de la correspondance avec la console. Réservation complète existante : 29 Go arrondis pour V1/V2/Lite, 58 Go arrondis pour OLED. |
| Linux et Android | Activations indépendantes ; 16 Go minimal ou 32 Go recommandé par système. Un système désactivé occupe 0. Les 16/32 Go Android comprennent ses partitions système. |
| Vérification des tailles | Minimum FAT32, réserves et alignements conservés. Dispositions incompatibles bloquées. Taille de l’eMMC contrôlée ; mode simplifié refusé si elle est indisponible ou agrandie. |
| Mode expert | Réglages manuels conservés, dont emuMMC redimensionnée ou double, Linux, Android et gestion eMMC interne. Outils d’installation existants conservés. |
| Confirmation | Nouveau récapitulatif des allocations avant les confirmations d’effacement d’origine. Nouvelle vérification de disponibilité et de capacité avant la procédure de formatage. |
| Outils USB | Marges rétablies, bloc lecture seule dégagé et cartes contenues dans la page. |
| Bit d’archive | Présentation centrée, chemin borné à une ligne défilante, résultats et erreurs conservés ensemble. Parcours des dossiers inchangé. |
| Package 1/2 | Onglets Principal/Sécurité avec journaux complets et défilement interne lorsque nécessaire. Bouton de fermeture accessible après l’opération. |
| Onglets des réglages | Fond sélectionné distinct, soulignement et contraste renforcés. |
| Luminosité | Dans Réglages Nyx → Système, à droite de l’horloge. Plage et réglage matériel existants réutilisés. Sauvegarde commune des options Hekate et Nyx. Aucun contrôle ajouté à la barre inférieure. |
| Première installation | Fond `Nova-Carbon.bmp`, couleurs Nova, fenêtres transparentes à 75 % : valeur interne d’opacité de 25 %. Réglage date/heure d’origine et choix de langue conservés. |
| Préférences existantes | Chargement des préférences sauvegardées conservé. Le paquet ne contient aucun `nyx.ini` ni `hekate_ipl.ini`. « Réinitialiser » dans le menu du thème rétablit les nouveaux choix Carbon/Nova. |
| Dernier ajout : bordure animée | Dégradé bleu/violet animé sur la tuile, l’icône et le menu principal sélectionnés. Désactivable avec le réglage des effets de sélection. |
| Dernier ajout : reflet rapide | Reflet blanc diagonal sur la tuile complète, largeur douce de 129 pixels ; passage de 240 ms, cycle de 720 ms. Aucun retour des vagues ou autres animations de fond. |

Les valeurs « Go » des choix simplifiés correspondent aux allocations binaires du code ; le récapitulatif affiche les MiB exacts. L’emuMMC complète réserve **29 856 MiB** ou **59 680 MiB**, au lieu de tronquer ces tailles à 29/58 GiB. Le partitionnement réserve l’espace : la création de l’emuMMC et l’installation des systèmes utilisent ensuite leurs outils dédiés.

## Vérifications effectuées sur PC

- 141 902 assertions sur le calculateur de partitions compilé en C, dont 5 000 dispositions aléatoires, les limites de capacité et les fonctions de stockage conservées.
- 2 020 scènes avec le véritable moteur LVGL : sept langues, quatre fichiers de police (dont Caviar), tailles 20/30/32/36, dispositions, commandes et fond Carbon avec transparence.
- Navigation tactile réelle dans LVGL : 21 scénarios avec empilement et fermeture des fenêtres de réglages, couleurs et polices sur le fond d’écran.
- Contrôle des pixels de la bordure bleu/violet et du dégradé blanc ; surfaces redessinées inférieures à un huitième de l’écran par image dans les trois scènes de référence ; aucun rafraîchissement d’animation pendant une pause.
- Tests du réglage de luminosité, de la sauvegarde et de ses échecs, des préférences, des traductions, du premier réglage de l’horloge, des polices et du service mémoire.
- Compilation ARM et vérification que les binaires correspondent aux exécutables liés ; les 42 contrôles HWTest et le service mémoire isolé sont conservés.

## Validation restante sur console

Le contrôle du rendu et des interactions sur PC ne mesure pas les FPS réels sur Switch. Il reste à vérifier sur matériel la fluidité des nouveaux effets, la luminosité effective et les opérations de stockage. Aucun partitionnement, formatage ou test matériel n’a été exécuté sur une console pendant cette préparation.

Les propositions supplémentaires de diagnostic console puis PC restent dans la liste séparée des évolutions futures ; elles ne sont pas déclarées réalisées par ce cahier des charges.
