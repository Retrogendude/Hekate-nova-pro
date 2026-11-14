# Hekate 6.5.4 — onglet Diagnostic, prototype 1

Cette version de travail ajoute un onglet Diagnostic dans Nyx. Les tests tournent directement dans Nyx, sans lancer HWTest-RCM séparément.

Intégré depuis HWTest-RCM 0.15 : identification écran, état du rétroéclairage LCD/OLED, identification DRAM et comparaison des canaux. Résultats textuels PASS/WARN/FAIL. Les sources des sondes sont conservées dans frontend/diagnostics, avec un adaptateur Nyx spécifique.

Non intégré : diagnostics alimentation, stockage, audio, Wi-Fi/Bluetooth, entrées, test complet de RAM, export de rapport. Le test DRAM présent identifie la mémoire ; il ne vérifie pas toutes ses cellules.

L'onglet est ajouté après Options afin de conserver les indices des onglets existants. Aucun test ne s'exécute à l'ouverture de l'onglet.

Compilation : utiliser la procédure Hekate avec devkitARM, puis compiler Nyx via son Makefile (depuis nyx : make). Aucun binaire compilé n'est fourni. Les outils de compilation ARM ne sont pas disponibles dans l'environnement utilisé ; ni la compilation, ni l'affichage sur console, ni les mesures matérielles n'ont été validés. Ce prototype est destiné à une revue et à une compilation avant toute installation.

Points à vérifier sur matériel : largeur des six onglets, ouverture/fermeture des résultats, exécutions répétées, diagnostics LCD/OLED, identification DRAM Erista/Mariko. Les verdicts sont des observations des registres et ne constituent pas une validation complète du matériel.

Provenance : archives utilisateur hekate-6.5.4.zip et HWTest-RCM-0.15.zip. Licence Hekate conservée dans LICENSE. Les commentaires de provenance des sondes HWTest-RCM sont conservés. L'archive HWTest-RCM fournie ne contient pas de licence racine identifiée ; vérifier les droits de redistribution de ses sondes avant diffusion publique.
