# Hekate 6.5.4 — deux dispositions d'accueil, prototype 4

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

Les contrôles Diagnostic disponibles restent l'identification écran,
l'état du rétroéclairage et l'identification DRAM. L'audio, les radios,
le stockage et le test complet de RAM ne sont pas intégrés. La correction
de lecture des canaux DRAM du prototype précédent est conservée.

## Validation et limites

Cette livraison contient les sources uniquement. La chaîne de compilation
ARM n'est pas installée dans l'environnement utilisé ; aucun binaire compilé
ni essai sur console n'est fourni. La maquette interactive présente le choix
dans les réglages et ne lit ni n'écrit une carte SD réelle.

Avant installation, compiler Hekate et Nyx avec la chaîne prévue par leurs
Makefiles, puis vérifier les deux dispositions sur l'écran 1280 × 720,
les raccourcis, les réglages, la conservation du choix après rechargement,
l'annulation, les erreurs d'écriture et la navigation tactile/Joy-Con.

Voir `DIAGNOSTIC-INTEGRATION.md` pour la provenance et le périmètre des sondes.
