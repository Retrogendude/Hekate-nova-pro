# Hekate 6.5.4 — interface originale, prototype 3

Cette variante reprend l'accueil original de l'archive utilisateur : quatre
boutons Launch, More Configs, Payloads et emuMMC, les paramètres Nyx en bas
à gauche et les boutons d'alimentation en bas à droite. Les titres Home,
Tools, Console Info et Options sont ceux d'origine.

Le nouvel onglet Diagnostic reste à la suite d'Options. Sa présentation suit
le thème Nyx : boutons natifs, descriptions à droite et séparateurs fins.
Il reprend les couleurs du thème actif et prend en compte le fond personnalisé.
Les sondes disponibles restent l'identification d'écran, l'état du
rétroéclairage et l'identification DRAM ; aucun test ne part à l'ouverture.

Le routage entre Launch et More Configs utilise toujours des identifiants
explicites. Ce sont les deux seules additions au code de construction de
l'accueil original. Les indices des onglets sont préservés, Options = 4 et
Diagnostic = 5. Les détails du rapport et la correction de lecture des
canaux DRAM de la variante précédente sont conservés.

## Validation et limites

Le code de construction de l'accueil a été comparé à l'archive originale :
il est identique après retrait des deux affectations d'identifiants.
Les points d'entrée Diagnostic, les indices d'onglets et la syntaxe de la
maquette interactive ont été contrôlés. La maquette est une approximation
interactive du thème original, avec des symboles de remplacement ; ce n'est
pas une capture prise sur une console et elle ne lance aucun test matériel.

Aucune chaîne de compilation ARM n'est disponible dans l'environnement.
Ce livrable contient les sources uniquement, sans binaire. La compilation et
les essais sur console restent à effectuer, notamment la lisibilité des six
onglets, les boutons du diagnostic et les mesures Erista/Mariko et LCD/OLED.
Le rendu de la maquette n'a pas été vérifié dans un navigateur.

Voir `DIAGNOSTIC-INTEGRATION.md` pour le périmètre des sondes et leur provenance.
