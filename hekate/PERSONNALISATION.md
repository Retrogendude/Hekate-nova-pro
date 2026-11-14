# Apparence, langues et lecture — prototype 7

Depuis l'accueil, ouvrir **Réglages Nyx → Apparence**. Le même écran regroupe
la langue, la police, la taille du texte, la disposition de l'accueil et les
animations de fond. **Enregistrer et recharger** applique les choix. Fermer
la fenêtre annule les choix provisoires.

## Polices et tailles

Neuf tailles principales sont proposées, de 20 à 36 pixels. Les légendes
utilisent une taille proportionnée et les tableaux conservent une police
à largeur fixe. Les titres trop longs défilent sans recouvrir les commandes.

Trois polices prêtes à utiliser sont fournies dans `fonts/`, à copier à la
racine de la carte SD : Lecture, Lecture renforcée et Lecture fixe. Le menu
relit ce dossier au prochain chargement de Nyx. Jusqu'à seize polices SD
peuvent être ajoutées. Un fichier invalide utilise la police intégrée.

Pour une police personnelle TTF ou OTF, utiliser le convertisseur fourni
dans `outils-polices/`, puis copier le fichier `.nyxf` obtenu dans `fonts/`.
Les TTF/OTF ne sont pas lus directement par la console. Python 3 et Pillow
sont nécessaires uniquement sur l'ordinateur pour cette conversion.
Les instructions détaillées sont dans `documentation/POLICES.md`.

## Arrière-plans

Les choix sont Désactivé, Vagues, Particules et Aurore. La vitesse et
l'intensité se règlent séparément. Les effets sont désactivés par défaut.
Ils se suspendent pendant les mesures matérielles et ne modifient pas les
résultats. Le rendu utilise des formes natives ; aucun fichier vidéo n'est
nécessaire. Les fenêtres gardent une base sombre pour préserver la lecture.

## Dispositions

Les deux accueils Tuiles et Carrousel restent disponibles. Les menus
Outils, Informations, Options et Réglages Nyx sont réorganisés en cartes
avec défilement. Les aides reviennent à la ligne et leur hauteur est
mesurée. Diagnostic utilise deux colonnes avec les grandes tailles.
Les opérations et leurs confirmations conservent leurs fonctions.

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
Ils vérifient notamment la configuration, l'annulation, les erreurs de
lecture, les formats de police, les textes UTF-8 et les commandes conservées.
Ils ne remplacent pas l'essai sur console : vérifier la lecture à 20 et
36 pixels, les sept langues, les effets et la navigation tactile/Joy-Con.
