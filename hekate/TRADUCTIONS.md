# État des traductions — version 8

Le catalogue comporte sept valeurs par clé : anglais, français, espagnol,
italien, russe, allemand et portugais. Les menus principaux, réglages,
cartes Outils/Informations, confirmations répertoriées et l'écran Apparence
utilisent ce catalogue. Le test RAM lancé depuis Nyx dispose de sept packs
complets pour ses 108 chaînes source, y compris ses commandes et erreurs.

Le catalogue contient 3 692 clés, avec les sept langues pour chaque clé.
Les 1 268 chaînes source répertoriées dans les 42 sondes natives sont
couvertes, y compris leurs commentaires détaillés. Les compléments de
rapports ont été traduits hors ligne avec Argos/CTranslate2, puis contrôlés
pour préserver les paramètres, nombres, chemins et notations techniques.
201 entrées corrigées prennent la priorité sur la traduction automatique.
Les menus utilisent les traductions rédigées pour l'interface.

La couverture du catalogue ne garantit pas la qualité linguistique de
chaque commentaire automatique. Les termes électriques critiques ont été
revus, mais une relecture des longues descriptions par des locuteurs natifs
reste utile. La provenance et les empreintes des modèles figurent dans
`verification/provenance-traductions-techniques.json`. Aucun modèle n'est
nécessaire sur la console et aucune traduction en ligne n'y est effectuée.

Les noms de composants, registres, valeurs, unités, chemins et noms choisis
par l'utilisateur ne sont pas renommés. Les messages précoces du chargeur
et les affichages de secours restent en anglais. Le service RAM utilise
également l'anglais lorsque son pack est absent ou invalide. La traduction
de Nyx ne couvre donc pas l'ensemble des écrans précoces de récupération
du chargeur, avant le chargement de l'interface.

L'inventaire `verification/couverture-traductions.json` énumère les chaînes
restantes. Il comprend aussi les octets de contrôle des boutons, des
identifiants internes, des noms de composants et des fichiers : son
compteur n'est pas un taux de traduction de l'interface.
Les textes sont conservés en UTF-8. Les contrôles vérifient l'ordre des
paramètres numériques, les accents, le cyrillique, les icônes, les couleurs,
les durées de vie des menus et les découpes de pages multilingues.
