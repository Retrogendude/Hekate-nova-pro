# Hekate 6.5.4 — interface console, prototype 2

L'accueil Nyx utilise maintenant quatre grandes tuiles colorées : Démarrer,
Configurations, Payloads et emuMMC. Chaque tuile présente un titre et une
description. Deux raccourcis donnent accès au Diagnostic et aux réglages Nyx.
Les boutons de rechargement, redémarrage et arrêt conservent leurs actions et
leurs dialogues existants.

L'onglet Diagnostic reprend ces couleurs et propose trois choix : bilan des
contrôles intégrés, écran, identification mémoire. Les résultats affichent
le nombre d'échecs et d'avertissements ainsi que les détails des observations.
Les tests disponibles restent ceux du premier prototype ; cette version
n'ajoute pas les tests audio, radio, stockage ou de RAM complète.

## Périmètre

- Accueil et Diagnostic remaniés ; navigation principale raccourcie et
  partiellement traduite. Les fenêtres secondaires existantes sont conservées.
- Disposition native pour l'écran 1280 × 720 ; aucune image ni police
  supplémentaire à charger sur la carte SD.
- Tuiles opaques, coins arrondis et indication visuelle à l'appui. Le fond
  personnalisé reste visible autour des tuiles.
- Libellés français sans accents dans les sources natives : les polices
  embarquées actuelles ne couvrent que l'ASCII. La maquette de conception
  peut afficher des accents et des icônes différents des symboles natifs.
- Ordre interne des onglets préservé : Options = 4, Diagnostic = 5.
- Le choix entre configurations principales et supplémentaires utilise un
  identifiant explicite, au lieu de dépendre du texte affiché sur le bouton.

## Correction associée au diagnostic

Quatre extractions du second octet des registres mémoire dans
`bdk/mem/sdram.c` comportaient un problème de priorité des opérateurs :
`mrr & 0xFF00 >> 8`. Elles utilisent maintenant `(mrr & 0xFF00) >> 8`.
Cette correction permet de distinguer les deux canaux au lieu de réutiliser
le premier octet pour les deux. Les huit expressions d'extraction ont été
vérifiées avec quatre mots contenant des octets distincts, soit 32 cas réussis.

## Vérifications et limites

Les appels de navigation, les indices des six onglets, les dimensions des
tuiles et les largeurs des titres ont été examinés à partir des sources et
des métriques des polices embarquées. La syntaxe JavaScript de la maquette
interactive a été vérifiée. Le navigateur n'a pas autorisé son ouverture
depuis un fichier local ; aucun rendu navigateur n'a été validé ici.

La chaîne de compilation ARM n'est pas installée dans cet environnement.
Aucune compilation, aucun binaire et aucun essai matériel ne sont fournis.
La maquette présente une direction visuelle ; elle ne lance aucun test et
n'est pas une capture du programme fonctionnant sur console.

Avant installation, compiler Hekate et Nyx avec devkitARM suivant les
Makefiles du projet, puis vérifier sur console l'accès aux six onglets,
les actions de chaque tuile, les fenêtres de résultats, les appels répétés,
la lecture DRAM sur Erista/Mariko et les panneaux LCD/OLED.

Voir également `DIAGNOSTIC-INTEGRATION.md` pour le périmètre et la provenance
des sondes HWTest-RCM.
