# Présentation compacte — version 8

La version actuelle conserve Tuiles et Carrousel dans un thème noir/gris
uniforme, avec cartes transparentes et contours gris/blancs peu arrondis.
Les onglets principaux n'utilisent plus de défilement vertical. Réglages Nyx
et Apparence sont répartis en onglets internes ; les aides sont derrière « ? ».
Voir PERSONNALISATION.md pour l'utilisation et BUILD-INFO.json pour les tests.
Les notes ci-dessous décrivent les étapes précédentes de l'interface.

# Hekate 6.5.4 — interface et langues, prototype 7

Cette version intègre deux dispositions de l'accueil Nyx, sélectionnables
dans les réglages. Les fonctions Hekate et le nouvel onglet Diagnostic restent
accessibles dans les deux modes.

## Choisir la disposition

1. Ouvrir les réglages Nyx depuis l'accueil.
2. Ouvrir « Apparence », puis la section « Disposition de l'accueil ».
3. Choisir « Tuiles » ou « Carrousel ».
4. Utiliser « Enregistrer et recharger » pour appliquer le choix.

Fermer le sélecteur sans enregistrer conserve la disposition précédente.
Le choix est stocké dans la section `[config]` de `bootloader/nyx.ini` :

```ini
home_layout=0
```

`0` sélectionne les tuiles et `1` le carrousel. Un réglage absent ou invalide
utilise les tuiles. Le réglage existant `homescreen` reste indépendant :
pour afficher l'accueil au démarrage, choisir « Main menu » dans « Home Screen ».

## Les deux modes

**Tuiles** : une grande commande de démarrage et quatre accès aux configurations
supplémentaires, payloads, gestion emuMMC et Diagnostic.

**Carrousel** : des cartes se déplacent dans une zone horizontale. Les flèches
ou une carte visible changent la sélection ; « Ouvrir » lance uniquement la
fonction choisie. Une sélection ne lance jamais de payload automatiquement.
Le mode ne promet pas une navigation directe à la croix directionnelle :
les Joy-Con conservent le fonctionnement de pointeur déjà présent dans Nyx.

Les deux modes utilisent des formes et des polices natives, sans images
supplémentaires à déposer sur la carte SD. Une icône de puce identifie
Diagnostic dans la barre d'onglets et sur sa carte d'accueil.

## Langues

Ouvrir « Nyx Settings → Appearance », choisir une langue puis « Save & reload ».
Le menu propose English, Français, Español, Italiano, Русский, Deutsch et
Português. Fermer annule la sélection. Les modifications déjà présentes dans
les réglages Nyx sont également enregistrées par cette commande.

Le champ `language` de `bootloader/nyx.ini` vaut respectivement 0 à 6 dans cet
ordre. Une valeur absente ou invalide utilise l'anglais. Le choix de langue
est indépendant de `home_layout`.

Les menus, commandes principales, catégories de diagnostic, messages et
avertissements répertoriés utilisent un catalogue de sept langues. Les noms
de démarrage, chemins et rapports techniques restent tels quels. Certaines
informations techniques restent en anglais. Le programme RAM séparé reçoit
des ressources dans la langue choisie, avec un affichage de secours anglais.
Les accents et le cyrillique sont pris en charge par les polices Unicode
embarquées Noto Sans, sous licence SIL OFL fournie avec les sources.

## Sauvegarde et diagnostic

Le sélecteur vérifie la présence de Nyx avant le rechargement. Une erreur de
sauvegarde doit afficher un message et empêcher le rechargement. Les erreurs
d'écriture et de fermeture du fichier de configuration sont désormais prises
en compte par le rédacteur de configuration Nyx.

Le thème de l'accueil est également appliqué aux composants des menus et
sous-menus : fenêtres, panneaux, boutons, listes, réglages, sélecteurs,
interrupteurs, boîtes de dialogue et barre d'onglets. Les couleurs d'alerte
et les textes numériques à police fixe sont conservés. Les fonds et teintes
personnalisés restent pris en compte. Le thème par défaut utilise le fond
141A19, l'accent 93E1AE et les angles arrondis. Les cartes, disques d'icônes,
panneaux, boutons et sélections ont un fond transparent ; contours et texte
signalent la sélection. Les fenêtres superposées, dialogues, clavier et
listes déroulées gardent un fond sombre pour éviter les textes superposés.
Les indicateurs et barres de progression conservent leur couleur.

Le Diagnostic contient désormais toute la table des sondes HWTest, l'export
et le service RAM avec retour à Hekate. Voir DIAGNOSTIC-INTEGRATION.md.

## Validation et limites

La compilation complète a réussi avec devkitARM GCC 16.1.0.
Hekate, Nyx et les modules ont été générés. Les tailles du payload et de
Hekate non compressé respectent les limites du Makefile. Les fichiers binaires
ont été comparés à leur ELF lié. Le programme Wi-Fi AArch64 et le service
RAM séparé ont également été construits. La sauvegarde des réglages a passé 96 cas C
exécutés avec une carte SD simulée, y compris les erreurs d'écriture.

Aucun essai sur console n'a encore été effectué. Les traductions, polices et
leur gestion mémoire ont été contrôlées sur ordinateur avec le code C réel.

Pour la validation matérielle, vérifier les deux dispositions sur l'écran 1280 × 720,
les raccourcis, les réglages, la conservation du choix après rechargement,
l'annulation, les erreurs d'écriture et la navigation tactile/Joy-Con.

Voir `DIAGNOSTIC-INTEGRATION.md` pour la provenance et le périmètre des sondes.

Voir `PERSONNALISATION.md` pour les polices SD, les tailles et les effets ;
`TRADUCTIONS.md` décrit les limites actuelles de la traduction des rapports.
