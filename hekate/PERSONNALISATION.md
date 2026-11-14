# Apparence, langues et lecture — Nova, version 10

Ouvrir **Réglages** dans la navigation haute. Les codes de langue, la ligne
Police et les tailles proposées ouvrent directement le sélecteur concerné.
Le titre **Apparence et lecture** ouvre les réglages Nyx détaillés ; choisir
ensuite **Apparence** pour les autres choix. Le même écran regroupe
la langue, la police, la taille du texte, la disposition de l'accueil et les
animations de fond. **Enregistrer et recharger** applique les choix. Fermer
la fenêtre annule les choix provisoires.

## Polices et tailles

Neuf tailles principales sont proposées, de 20 à 36 pixels. Les légendes
utilisent une taille proportionnée et les tableaux conservent une police
à largeur fixe. Nova conserve des rôles distincts pour les titres, les commandes
et les légendes, proportionnés à la taille choisie et chargés une seule fois.
Les cartes utilisent un retour à la ligne ou des points de suspension ; le
bandeau de détail et l'aide donnent le texte complet. Les choix très
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

Le fond de référence est `#10151D`, les cartes `#19212D`, les contours
`#334050` et l'accent `#55C9F1`. Nova reprend les icônes Lucide de la maquette,
leurs traits fins, les cartes remplies et les angles de 8 pixels.
Le halo de 18 pixels reste fixe
autour de l'icône sélectionnée : aucun minuteur ne l'anime. Les cartes
affichent des fonds pleins, avec des icônes détourées. Le fond SD
personnalisé n'est plus dessiné dans cette présentation ; son fichier est
conservé. Les pastilles de couleur ont un remplissage propre, distinct des
cartes Nova. La première pastille correspond à l'accent bleu par défaut.
**Effets de sélection** active ou désactive le halo ; enregistrer les options
pour conserver ce choix.

Les choix d'animation sont Désactivé, Vagues, Particules et Aurore, avec
vitesse et intensité. Les effets sont concentrés dans une bande inférieure
de 64 pixels. Ils se figent pendant l'utilisation du tactile/Joy-Con et
reprennent après 900 ms de repos ; les diagnostics les suspendent aussi.
Une seule présentation de l'image est effectuée par rafraîchissement LVGL,
même lorsque plusieurs petites zones changent. Les recherches de glyphes
et les traductions ont été optimisées sans modifier les mesures matérielles.

## Dispositions

L'accueil Tuiles reprend la grande carte Démarrer et les quatre accès de la
maquette. Le Carrousel reste disponible. Les pages principales utilisent la
navigation haute et une zone de 1 280 × 580 pixels au-dessus de la barre d'état.
Outils et Infos ont quatre colonnes sur deux rangées ; Diagnostic présente
douze cartes sur trois rangées, sans défilement vertical.
Toucher une carte sélectionne son détail. La flèche du bandeau inférieur ouvre
la fonction ; toucher le texte du bandeau affiche l'aide complète.
Les mots trop larges utilisent d'abord la taille de légende de la police
choisie. Les options à deux états ont un interrupteur et un état écrit ;
Auto Boot continue d'ouvrir le choix de l'entrée de démarrage. Il se trouve
avec les autres options de démarrage derrière le titre **Options système**.
Le logo HEKATE ouvre les commandes rapides et les réglages Nyx ; un appui long
ouvre les crédits.

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
Les vérifications graphiques couvrent 1 890 scènes LVGL : les sept langues,
les neuf tailles principales, les trois polices fournies et la police intégrée.
Douze scènes supplémentaires vérifient les gardes selon la disponibilité du
matériel ou d'un outil. Les pixels des icônes détourées sont contrôlés.
Ils vérifient notamment la configuration, l'annulation, les erreurs de
lecture, les formats de police, les textes UTF-8 et les commandes conservées.
Ils ne remplacent pas l'essai sur console : vérifier la lecture à 20 et
36 pixels, les sept langues, les effets et la navigation tactile/Joy-Con.
