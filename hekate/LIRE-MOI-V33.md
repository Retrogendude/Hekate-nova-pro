# Hekate Nova Pro v33 — bêta de test

Base : Hekate 6.5.5 / Nyx 1.9.5. Cette archive est destinée au prochain essai sur console ; la publication GitHub attend le retour de cet essai.

## Installation et configurations existantes

Extraire le contenu du pack à la racine de la carte SD. Remplacer ensemble les fichiers fournis dans `bootloader`, y compris `bootloader/sys/nyx.bin` et `bootloader/update.bin`. Utiliser le nouveau payload `hekate_nova_pro_6.5.5_v33_beta.bin` lorsque l'injection du payload est nécessaire.

L'archive ne fournit ni `bootloader/nyx.ini` ni `bootloader/hekate_ipl.ini` : les réglages et entrées de démarrage déjà présents restent conservés. Conserver également une copie de vos icônes personnelles avant de remplacer les PNG portant les mêmes noms.

Les nouvelles valeurs par défaut s'appliquent lorsqu'il n'existe pas de fichier `bootloader/nyx.ini`. Pour essayer cette situation sur une installation existante, renommer temporairement ce fichier sur le PC, par exemple en `nyx.ini.avant-v33`, puis démarrer Nova. Le dialogue d'origine permet de régler la date, l'heure et la langue. Ne pas recopier la valeur `timeoffset` d'une autre console.

## Réglages par défaut

Français, disposition en tuiles, fond `Nova-Carbon-Cyan.bmp`, taille de texte de 30 pixels. L'opacité des fenêtres et des icônes est de 75 %.

| Élément | Couleur |
| --- | --- |
| Fond | `10151D` |
| Texte | `F2F5FA` |
| Contours | `334050` |
| Reflet et barre de chargement | `55C9F1` |
| Logo de l'accueil | `55C9F1` |
| Icônes intégrées et logo de démarrage | `F2F2F2` |
| Sélection | `1C2937` |

Les animations de fond restent désactivées. La détection automatique du dock reste désactivée et le mode TV conserve l'écran de la console. Les autres paramètres fournis sont repris dans les valeurs initiales ; les réglages existants ont priorité.

## Langues et polices

Les huit langues sont l'anglais, le français, l'espagnol, l'italien, le russe, l'allemand, le portugais et le chinois simplifié.

Le choix « Par défaut » utilise automatiquement `Lecture.nyxf` pour les langues latines et la famille intégrée pour le russe. « Hekate d'origine » sélectionne directement la police intégrée ; aucun fichier supplémentaire n'est requis pour ce choix. Les trois variantes Lecture restent incluses dans `/fonts`.

Le chinois utilise obligatoirement `bootloader/sys/Nova-Chinese-Simplified.nyxf`. La sélection de polices personnelles est grisée pour cette seule langue. La taille reste réglable de 20 à 36 pixels. La police personnelle est conservée séparément et redevient applicable lorsque l'on quitte le chinois. Les choix en attente peuvent être annulés en fermant la fenêtre.

La ressource chinoise ne doit pas être déplacée dans `/fonts`. Si elle est absente ou invalide, Nova utilise l'anglais pour garder une interface lisible. Il faut recopier la ressource depuis le pack pour retrouver le chinois. Le fichier contient les caractères de l'interface ; il ne garantit pas l'affichage de tous les noms de fichiers chinois possibles.

Les titres, petits textes et rapports utilisent des variantes adaptées. Les rapports chinois utilisent des colonnes latines fixes avec des cellules plus larges pour les caractères chinois. Les libellés principaux, des avertissements ciblés et les messages du test mémoire ont été revus. Les longs rapports de diagnostic contiennent encore de la traduction automatique et doivent faire l'objet d'une relecture linguistique.

## Fonds et icônes

Trois nouveaux fonds fixes sont disponibles dans `/wallpapers` : Carbon Cyan, Nébuleuse Violette et Obsidienne. Les sélectionner dans Réglages Nyx > Thèmes et couleurs > Fond et transparence, puis enregistrer et recharger.

Le dossier `/icone` contient cinq icônes Nova et `Launcher.png`, conservé tel que fourni. Le réglage initial `launch_image=Launcher.png` utilise ce fichier comme icône de la tuile principale, sans le placer devant son contenu. Si le fichier est absent, l'icône `launch.png` sert de remplacement.

Les PNG personnels `launch.png`, `configs.png`, `payloads.png`, `emummc.png` et `diagnostic.png` restent pris en charge. Leurs couleurs et leur transparence propres sont conservées. L'image Launcher fournie contient déjà une opacité uniforme de 20 % ; elle peut donc sembler discrète sur un fond sombre.

## Options de démarrage

Dans Réglages > Options système, utiliser le bouton visible « Options de démarrage ». Cette page regroupe Auto Boot, le délai, Auto NoGC, Auto HOS Power Off et la mise à jour du payload de redémarrage.

Auto Boot ouvre un sélecteur paginé de configurations principales ou supplémentaires. Choisir une configuration active le démarrage automatique ; « Désactiver » l'arrête. Les indices enregistrés restent ceux des fichiers de configuration Hekate. Le délai va de 0 à 6 secondes. VOL− pendant ce délai permet de revenir au menu Hekate.

Le bouton Luminosité ouvre le réglage déjà présent dans Réglages Nyx > Système. Il n'y a pas de deuxième curseur. Enregistrer les changements avec le bouton de sauvegarde.

## Test mémoire séparé et contrôles

Le test mémoire garde son lancement séparé et son retour à Hekate. Il dispose maintenant d'un pack chinois à caractères 16 × 16. Les sept anciens packs de langues restent compatibles ; la logique des tests mémoire et de l'entraînement n'a pas été changée.

Les compilations ARM, le chargement des polices, les choix de langues et l'interface sont contrôlés sur PC. Ces contrôles ne valident ni le fonctionnement matériel du test mémoire, ni la portée Bluetooth, ni les opérations de stockage sur une console réelle. Les changements Bluetooth et de partitionnement de la v32 sont conservés.

Les notes de version sont fournies en huit langues dans `documentation/changelogs`. Les licences Hekate, Nyx, HWTest, Noto, Lucide et BTstack restent applicables et sont conservées dans le pack ou les sources.
