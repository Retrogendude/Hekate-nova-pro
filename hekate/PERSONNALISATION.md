# Apparence, langues et lecture — version 8

Depuis l'accueil, ouvrir **Réglages Nyx → Apparence**. Le même écran regroupe
la langue, la police, la taille du texte, la disposition de l'accueil et les
animations de fond. **Enregistrer et recharger** applique les choix. Fermer
la fenêtre annule les choix provisoires.

## Polices et tailles

Neuf tailles principales sont proposées, de 20 à 36 pixels. Les légendes
utilisent une taille proportionnée et les tableaux conservent une police
à largeur fixe. Les titres trop longs défilent sans recouvrir les commandes. Les choix très
longs peuvent employer la taille de légende de la même police pour rester
lisibles dans leur bouton.

Trois polices prêtes à utiliser sont fournies dans `fonts/`, à copier à la
racine de la carte SD : Lecture, Lecture renforcée et Lecture fixe. Le menu
relit ce dossier au prochain chargement de Nyx. Jusqu'à seize polices SD
peuvent être ajoutées. Un fichier invalide utilise la police intégrée.

Pour une police personnelle TTF ou OTF, utiliser le convertisseur fourni
dans `outils-polices/`, puis copier le fichier `.nyxf` obtenu dans `fonts/`.
Les TTF/OTF ne sont pas lus directement par la console. Python 3 et Pillow
sont nécessaires uniquement sur l'ordinateur pour cette conversion.
Les instructions détaillées sont dans `documentation/POLICES.md`.

## Arrière-plans et fluidité

Le fond est uniforme, noir/gris. Les cartes sont transparentes et leurs
contours gris ou blancs ont des angles légèrement arrondis. Le fond SD
personnalisé n'est plus dessiné dans cette présentation ; son fichier est
conservé. Les pastilles de couleur ont un remplissage propre, distinct des
menus transparents. La première pastille correspond à l'accent gris.

Les choix d'animation sont Désactivé, Vagues, Particules et Aurore, avec
vitesse et intensité. Les effets sont concentrés dans une bande inférieure
de 64 pixels. Ils se figent pendant l'utilisation du tactile/Joy-Con et
reprennent après 900 ms de repos ; les diagnostics les suspendent aussi.
Une seule présentation de l'image est effectuée par rafraîchissement LVGL,
même lorsque plusieurs petites zones changent. Les recherches de glyphes
et les traductions ont été optimisées sans modifier les mesures matérielles.

## Dispositions

Les deux accueils Tuiles et Carrousel restent disponibles. Outils,
Informations, Options et Diagnostic tiennent dans la zone entre la
navigation haute et la barre d'état, sans défilement vertical des pages
principales. Les aides longues sont accessibles par « ? ». Les titres
peuvent revenir à la ligne et les contrôles gardent leurs identifiants.

Réglages Nyx comprend deux onglets internes, Interface et Système.
Apparence comprend Langue, Police, Disposition et Effet. La police se
sélectionne avec les flèches ; les autres choix courants sont visibles.
Les rapports détaillés et les listes de fichiers peuvent toujours défiler.

## Langues et données

Les sept langues sont l'anglais, le français, l'espagnol, l'italien, le russe,
l'allemand et le portugais. Le catalogue comprend les libellés, messages,
aides et avertissements de l'interface ainsi que les textes de diagnostic
répertoriés. Les mesures, noms de composants, registres, chemins et noms
d'entrées de démarrage restent identifiables et ne sont pas renommés.
Les commentaires détaillés des sondes sont désormais présents dans les
sept langues. Leur complément a été traduit hors ligne, avec des contrôles
sur les valeurs et les chemins ainsi que des corrections de terminologie.
Une relecture linguistique des descriptions automatiques reste à faire ;
le périmètre exact est décrit dans `TRADUCTIONS.md`.

Le test RAM lancé depuis Nyx reçoit un petit pack de textes et de glyphes
dans la zone de mémoire déjà exclue du test. Le pack est vérifié avant
utilisation. S'il manque ou s'il est corrompu, le service conserve son
affichage de secours en anglais. Le démarrage autonome et les messages
précoces de récupération restent disponibles sans ressources de langue.

Les réglages sont ajoutés à `bootloader/nyx.ini`. Aucun fichier de réglages
ni aucune entrée de démarrage n'est fourni dans l'archive. Fusionner les
dossiers `bootloader/` et `fonts/` avec la carte SD existante.

## Vérification

La compilation et les contrôles sur ordinateur sont fournis dans l'archive.
Les vérifications graphiques couvrent 1 365 scènes LVGL : les sept langues,
les neuf tailles principales et des contrôles des trois polices fournies.
Ils vérifient notamment la configuration, l'annulation, les erreurs de
lecture, les formats de police, les textes UTF-8 et les commandes conservées.
Ils ne remplacent pas l'essai sur console : vérifier la lecture à 20 et
36 pixels, les sept langues, les effets et la navigation tactile/Joy-Con.
