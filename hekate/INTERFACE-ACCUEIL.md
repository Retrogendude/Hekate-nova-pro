# Hekate 6.5.4 — deux dispositions d'accueil, prototype 5

Cette version intègre deux dispositions de l'accueil Nyx, sélectionnables
dans les réglages. Les fonctions Hekate et le nouvel onglet Diagnostic restent
accessibles dans les deux modes.

## Choisir la disposition

1. Ouvrir les réglages Nyx depuis l'accueil.
2. Utiliser le bouton « Disposition » en haut des réglages Nyx.
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
supplémentaires à déposer sur la carte SD. Les libellés français des sources
utilisent l'ASCII compatible avec les polices embarquées.

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
141A19, les panneaux 202B27, l'accent 93E1AE et les angles arrondis.

Le Diagnostic contient désormais toute la table des sondes HWTest, l'export
et le service RAM avec retour à Hekate. Voir DIAGNOSTIC-INTEGRATION.md.

## Validation et limites

La compilation complète a réussi le 4 novembre 2026 avec devkitARM GCC 16.1.0.
Hekate, Nyx et les modules ont été générés. Les tailles du payload et de
Hekate non compressé respectent les limites du Makefile. Les fichiers binaires
ont été comparés à leur ELF lié. Le programme Wi-Fi AArch64 et le service
RAM séparé ont également été construits. La sauvegarde des réglages a passé 69 cas C
exécutés avec une carte SD simulée, y compris les erreurs d'écriture.

Aucun essai sur console n'a encore été effectué. La maquette interactive
présente le choix dans les réglages et ne lit ni n'écrit une carte SD réelle.

Pour la validation matérielle, vérifier les deux dispositions sur l'écran 1280 × 720,
les raccourcis, les réglages, la conservation du choix après rechargement,
l'annulation, les erreurs d'écriture et la navigation tactile/Joy-Con.

Voir `DIAGNOSTIC-INTEGRATION.md` pour la provenance et le périmètre des sondes.
