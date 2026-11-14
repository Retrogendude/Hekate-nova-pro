# Hekate Nova Pro 18 — état des cinq demandes

| Point | État |
|---|---|
| 1. Crédits | Réalisé : quatre onglets supérieurs, pagination du bas supprimée, auteurs et licences conservés ; logiciels libres répartis sur deux colonnes. |
| 2. PNG et dossier `icone` | Réalisé : dossier livré à la racine, cinq pictogrammes d’accueil remplaçables par PNG transparents, image de Launch sélectionnable dans ce dossier. Compatibilité des anciens BMP conservée. |
| 3. Options système | Réalisé : suppression des contours dans la colonne de gauche, en conservant l’état et le fonctionnement des interrupteurs. |
| 4. Partitionnement | Réalisé : V1/V2 regroupées, aperçu FAT32 corrigé pour les profils calculables, mentions Minimum/Recommandé, valeurs Go en mode expert et capacité MiB/Go. Les confirmations et contrôles bloquant un partitionnement incompatible restent présents. |
| 5. TV via dock officiel | Étude préparée. Aucun affichage TV fonctionnel n’est annoncé. Le portage des pilotes et la validation physique restent à réaliser ; voir [le plan de développement](DEVELOPPEMENT-TV-DOCK.md). |

## Images

Les petits pictogrammes utilisent `icone/launch.png`, `configs.png`, `payloads.png`, `emummc.png` et `diagnostic.png`. Ils conservent leurs proportions et leur alpha. Fichier absent ou invalide : icône Nova d’origine. Recharger Nyx après changement des fichiers.

L’image de la grande tuile Launch se choisit dans le menu du thème, indépendamment du fond d’écran. Elle est recadrée pour couvrir la tuile. Les textes et les actions restent au-dessus. La prise en charge PNG est ajoutée à Nyx ; le fond d’écran global conserve son chargement BMP depuis `wallpapers`.

Limites : source 1920 × 1080, PNG de 8 MiB maximum, allocation interne limitée à 20 MiB, sortie limitée à 1280 × 720. Le décodage se fait une fois lors de la création de l’image. Le décodeur LodePNG est distribué avec son attribution, sa licence et les empreintes de ses sources officielles.

## Calcul de l’espace FAT32

Le calcul de la capacité est désormais conservé lorsqu’un autre modèle est prévisualisé. Un modèle incompatible continue de bloquer la validation ; V1 et V2 sont considérées comme une même famille. En cas d’espace réellement insuffisant, l’interface ne fabrique pas une capacité restante positive. Les opérations de formatage et d’installation ne sont pas modifiées.

## Vérification

Les comptes rendus joints décrivent les tests sur PC : transparence et formats PNG, rendu natif LVGL dans les sept langues et plusieurs polices, onglets de crédits, calculateur de partitions, sécurité des préférences et commandes de sauvegarde. Les binaires sont comparés aux exécutables liés et aux empreintes des sources.

La validation sur Switch reste nécessaire. Aucun formatage ni test sur une carte SD physique n’a été effectué pendant cette préparation. Les archives v17 restent intactes.
