# Hekate Nova Pro v34-alpha4 — préversion

Correction du faux échec emummc.ini causé par les fins de ligne FatFs ; installateur Windows autonome et correction du dossier Payloads.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

Les notes d’origine sont conservées : leurs mentions « non publié » décrivent l’état au moment du développement, avant cette publication.

## Modifications


Correction du faux échec d’enregistrement emuMMC/emummc.ini avant le partitionnement : écriture en octets exacts, sans conversion automatique des fins de ligne. Synchronisation et relecture conservées. Défaut reproduit et correctif testé avec le vrai FatFs Nyx sur FAT32/exFAT virtuels. Écritures courtes et protection en écriture toujours refusées. Interface et fonctions de l’alpha 3 conservées ; essai réel sur console à effectuer.

PC checks passed; physical console validation pending.


## Téléchargements et utilisation

- Pack standard : **hekate-nova-pro-v34-alpha4.zip**.
- Sources complètes, dépendances et outils : **hekate-nova-pro-v34-alpha4-sources.zip**.
- Pack 8GB séparé lorsqu’il existe ; réservé au matériel modifié.
- Changelogs en huit langues et empreintes SHA-256 joints.
- Installateur Windows et son code source joints. Pour alpha4, la révision **1.1** corrige le dossier Payloads ; le premier installateur reste archivé. Pour les alphas suivantes, utiliser l’installateur de la même version.

Installer ensemble le dossier bootloader et le payload de cette version. Conserver les configurations et appairages personnels. Les tests PC ne constituent pas une validation matérielle ; la stabilité TV de l’alpha7 reste à confirmer.

[Notice d’utilisation](https://github.com/Retrogendude/Hekate-nova-pro/blob/v34-alpha4/docs/NOTICE-UTILISATION.md) · [Sources parcourables](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha4/hekate) · [Changelogs](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha4/docs/changelogs)

<details><summary>Notice livrée avec cette version</summary>

CORRECTIF V34 ALPHA 4 — BLOCAGE AVANT PARTITIONNEMENT

La v34 alpha 3 pouvait afficher une impossibilité d’enregistrer emuMMC/emummc.ini sur une carte pourtant accessible. La conversion automatique des fins de ligne par la fonction d’écriture de texte ne correspondait pas à la comparaison de relecture : le contrôle échouait à tort avant le formatage.

La configuration est désormais écrite en octets exacts, avec contrôle du nombre d’octets, synchronisation, fermeture puis relecture identique avant remplacement. Les protections contre les erreurs réelles restent actives.

Installer en fusionnant le dossier bootloader fourni et utiliser le payload de ce pack. Garder ses configurations et fichiers personnels. Il n’est pas nécessaire d’effacer emummc.ini, de supprimer ses sauvegardes ou de formater manuellement pour installer ce correctif.

Validation ciblée : 147 assertions avec le vrai code FatFs et la configuration de Nyx, sur des disques virtuels FAT32 et exFAT. Le défaut de la v34 alpha 3 est reproduit, puis sa correction contrôlée : première désactivation, désactivations répétées, activation brute et par fichiers, remplacement d’un ancien fichier CRLF et refus réel d’écriture protégée. 246 assertions de copie/activation avec erreurs et écritures courtes simulées ; 3664 de transaction/tables et 76 de transfert. Les contrôles de l’interface et du calcul des tailles sont repris de l’alpha 3 car ces sources sont inchangées ; ils ne constituent pas une nouvelle validation sur console.

Aucun disque physique n’a été écrit par les tests. L’essai sur Switch reste à effectuer avec une SD entièrement sauvegardée avant toute confirmation de partitionnement.

Hekate Nova Pro v34 alpha 4 — assistant d’installation automatisé

Nouveautés
Le bouton Accueil est agrandi et porte le texte « Assistant d’installation automatisé » avec une icône. Il existe dans les vues tuiles et carrousel.
L’assistant enchaîne désormais le partitionnement réussi et, si elle a été sélectionnée, la création d’une emuMMC complète. BOOT0, BOOT1 et la zone principale de la mémoire interne sont copiés puis relus pour comparaison avant activation.

AVANT TOUT PARTITIONNEMENT
Sauvegarder sur un PC tous les fichiers utiles de la carte SD. Le partitionnement est destructif : toute donnée non sauvegardée peut être perdue. La sauvegarde limitée effectuée par le partitionneur Hekate ne remplace pas une sauvegarde personnelle complète.

Parcours
1. Depuis Accueil, ouvrir Assistant d’installation automatisé et lire la présentation.
2. Choisir le modèle réel de la console : Switch V1/V2, Lite ou OLED ; sélectionner emuMMC/emuNAND, Linux et/ou Android. Pour Linux et Android, cocher séparément 16 Go Minimum ou 32 Go Recommandé (32 Go par défaut). Une seule taille par système ; les choix restent mémorisés lors des retours entre étapes.
3. Lire le récapitulatif. Les tailles sont calculées avec la capacité réelle : emuMMC complète de 29 Go environ sur V1/V2/Lite ou 58 Go sur OLED, réserves techniques et au moins 2 Go de FAT32. Linux et Android utilisent exactement les tailles 16/32 Go sélectionnées. Aucun passage silencieux à 16 Go : si le plan ne tient pas, choisir soi-même 16 Go, retirer un système ou utiliser une carte plus grande. Une carte absente, une capacité insuffisante, un modèle incorrect ou une mémoire interne incompatible bloquent la poursuite.
4. Lire la mise en garde puis donner son accord avec « J’accepte le partitionnement de cette carte SD. ». L’état indique « Je n’accepte pas » / « J’accepte ». Suivant reste bloqué avant accord et redevient bloqué si cet accord est retiré. Cet accord autorise la suite ; il ne certifie pas qu’une sauvegarde a été faite.
5. Le partitionneur existant reçoit le plan préparé. Dans l’assistant, Suivant se trouve en bas à droite, y compris sur cet écran final. Les menus manuels simplifié et expert conservent leur disposition. Choisir Suivant et conserver la confirmation matérielle Hekate : après le délai de sécurité, POWER confirme et VOL annule. Jusqu’à cette confirmation, aucun partitionnement n’est lancé.
6. Après réussite du formatage, de la restauration des fichiers et de l’écriture vérifiée des tables, la copie emuMMC démarre automatiquement si elle a été choisie. Ne retirer ni la SD ni l’alimentation. La relecture intégrale prend du temps. L’emuMMC n’est activée qu’après réussite de la copie, de la vérification et de l’enregistrement de ses métadonnées.
7. En cas d’erreur ou d’annulation pendant le partitionnement ou la copie, la suite automatique s’arrête. Après le début de l’opération, l’emuMMC reste désactivée jusqu’à une copie complètement réussie. Lire les messages de l’écran. Il n’y a pas de reprise automatique après interruption.

Le processus vérifie l’identité de la carte, sa capacité, le modèle, la capacité interne et la partition cible à plusieurs étapes. Remplacer la carte impose de revoir le plan et de reconfirmer la sauvegarde. Aucun fichier personnel de configuration ou d’appairage n’est fourni par ce pack.

Portée
Cette alpha automatise la préparation de la carte et la copie emuMMC. Linux et Android sont seulement réservés dans la table de partitions : leurs fichiers et leur installation restent nécessaires. Aucun téléchargement ni installation automatique de ces OS n’est annoncé.

Installation du pack de test
Conserver vos nyx.ini, hekate_ipl.ini, appairages, fonds et icônes. Fusionner le dossier bootloader fourni avec celui de la carte, puis utiliser le payload de ce même pack. Ne pas effacer les configurations pour installer cette alpha.
Pack standard : consoles à mémoire normale. Pack 8GB : uniquement consoles physiquement modifiées, expérimental ; ce changement ne valide pas le matériel 8 Go.

Validation et limites
Compilation ARM et en-têtes Hekate 6.5.5 / Nyx 1.9.5 vérifiés. 696 cas de parcours et accueil en huit langues, 48 scènes d’aide et de blocage sur carte suspecte, 683 scènes de régression, 3539 assertions du calcul et des cibles, 3664 assertions de transaction/écriture des tables, 76 assertions de transfert et 246 assertions de copie/activation avec erreurs simulées réussis sur PC.
Les interfaces sont rendues depuis le code LVGL. Les entrées/sorties des tests sont simulées ; aucun effacement ou transfert réel sur une Switch n’a été exécuté ici. Faire d’abord un essai avec une carte intégralement sauvegardée. Cette alpha locale n’est pas publiée sur GitHub.

Aide et tailles personnalisées
L’icône Aide affiche les tailles de l’emuMMC complète : 29 Go pour V1/V2 et Lite, 58 Go pour OLED (valeurs arrondies, réserves conservées). Elle explique les choix Linux/Android et indique le chemin pour une taille personnalisée : fermer l’assistant, puis Outils > Gestionnaire de partitions SD > Mode expert. Ce renvoi ne lance aucune opération automatique.

Erreurs de stockage
Un échec du contrôle indique sa cause : initialisation SD, identité ou capacité de carte différente, modèle ou plan changé, source interne inaccessible ou de taille différente, échec de sauvegarde de emuMMC/emummc.ini. Si l’échec survient avant le formatage, le message précise que le partitionnement n’a pas commencé. Lire ce détail avant toute nouvelle tentative.

Carte signalée suspecte
Hekate affiche déjà « Fake » pour l’identifiant fabricant SD 0x00. L’assistant utilise ce même indicateur et bloque l’installation automatique avant l’effacement, avec une explication. Cela signale une suspicion ; il ne s’agit pas d’un test complet de capacité réelle ni d’une garantie d’authenticité. Les cartes de fausse capacité qui annoncent un identifiant ordinaire peuvent échapper à ce contrôle. Aucun nouveau test destructif de capacité n’est ajouté.

</details>
