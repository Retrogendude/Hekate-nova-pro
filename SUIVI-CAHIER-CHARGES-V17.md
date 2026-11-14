# Hekate Nova Pro 17 — suivi des corrections

Base Hekate 6.5.4 / Nyx 1.9.4. Cette version implémente les huit points de la dernière liste et les fonctions de diagnostic console demandées. Les fonctions de diagnostic depuis un PC restent pour une étape ultérieure.

| Point | Réalisation |
|---|---|
| 1. Démarrage | Groupe logo centré horizontalement ; barre remontée près du titre. Mises à jour toutes les 20 ms pendant les interrogations du temps par les pilotes. Progression plafonnée à l’étape en cours. Animation suspendue pour le dernier entraînement mémoire et arrêtée avant le transfert Hekate/Nyx. |
| 2. Navigation et reflet | Espacement des icônes, libellés et cadres augmenté. Reflet blanc de 240 ms, puis pauses alternées de 3 et 5 secondes. Bordure bleu/violet conservée. |
| 3. Image Launch | BMP facultatif limité à la tuile de lancement, dans les dispositions tuiles et carrousel. Choix indépendant du fond d’écran ; image recadrée aux dimensions de la tuile et décodée une fois à sa création. Libellés et actions conservés au-dessus. |
| 4. Trait parasite | Suppression du petit trait vertical indépendant dans les encadrés d’information d’Outils, Infos et Diagnostic. Cadre normal conservé. |
| 5. Sélecteur | Rectangle teinte/saturation, curseur de luminosité, HEX/RGB et aperçu à droite. Couleur active correcte ; modifications cumulées conservées entre cibles et onglets avant sauvegarde. Présélections supprimées. |
| 6. Sélection | Contour visible pendant l’appui et sur les choix activés, y compris réglages inférieurs et commandes supérieures. |
| 7. Couleurs de démarrage | Logo et barre réglables séparément dans l’onglet Démarrage. Lecture des couleurs par Hekate et Nyx, dans la même configuration sauvegardée. Valeurs absentes ou invalides : turquoise Nova. |
| 8. Diagnostic console | Bilan par composant et pistes ; Joy-Con/Lite interactifs ; grille tactile ; RAM rapide/avancée/personnalisée isolée ; mires manuelles. Détails ci-dessous. |

## Diagnostic et comptes rendus

Le test complet ouvre le bilan de quinze composants/ensembles. Les anomalies sont présentées d’abord, puis les contrôles réussis et les couvertures incomplètes. Chaque composant propose son périmètre, les preuves enregistrées et l’accès aux rapports d’origine. Les six états sont PASS, avertissement, anomalie, erreur de test, partiel et non testé.

L’agrégation utilise les résultats structurés et les prérequis réellement disponibles. Une absence de test ou d’initialisation ne devient pas un PASS. Les reprises de lecture communes à la SD et à l’eMMC ne désignent pas automatiquement la SD comme défectueuse. Les liens CPU0/Wi-Fi et USB-PD/chargeur sont signalés comme recoupements, sans imposer le remplacement d’une puce.

Les boutons Joy-Con/Lite sont mémorisés ; sticks, repos, amplitude, décalage et pertes/reconnexions sont mesurés. La grille tactile couvre les 144 cases du panneau entier, y compris sous les consignes ; les zones non parcourues restent non testées. Les cinq mires permettent une déclaration visuelle par couleur, distincte d’une détection automatique.

Le service RAM consomme un choix rapide, avancé ou personnalisé non vide et retourne son mode réel, ses masques sélectionnés/achevés, sa durée, ses erreurs et sa couverture. Les 4 premiers MiB réservés restent exclus. La zone prévue et le préfixe effectivement relu avec au moins un motif sont distingués ; une interruption ne valide pas les motifs inachevés.

L’export conserve le compte rendu d’origine et ajoute le bilan, les pistes et les derniers contrôles complémentaires capturés pendant cette session. Les observations sont conservées en mémoire jusqu’au redémarrage ; les exporter permet de les garder. Les mires et la grille ne remplacent pas les contrôles électriques ou matériels correspondants.

## Vérifications et limites

Les rapports JSON joints décrivent les scénarios réellement exécutés : moteur LVGL natif avec polices et traductions, limites d’affichage, sauvegardes et échecs d’écriture, composants, export, minuterie de démarrage et contrat du service RAM. Les binaires sont comparés aux exécutables liés, et le décodeur réel vérifie les images compressées. Les 42 contrôles HWTest d’origine sont conservés.

Le coût du reflet est vérifié en surfaces redessinées : aucune image complète pour les scènes de référence, moins d’un huitième de l’écran par mise à jour et aucun rafraîchissement d’animation lors d’une pause de l’interface. Cette mesure sur PC ne constitue pas une mesure de FPS sur Switch.

La progression du démarrage est coopérative : elle continue pendant les attentes qui interrogent les temporisateurs. Une phase de calcul ou d’entraînement sans interrogation ne peut pas être animée par ce mécanisme. Aucun délai de chargement matériel de cinq secondes n’a été mesuré ou promis.

La validation physique sur Switch reste à effectuer : démarrage, couleurs, fluidité, entrées, mesures et retour RAM sur les modèles concernés. Aucune opération de formatage, partitionnement ou test matériel n’a été exécutée sur une console pendant la préparation.

Le paquet ne contient aucun `nyx.ini` ou `hekate_ipl.ini` : les préférences existantes sont conservées. Les valeurs par défaut restent Carbon, couleurs Nova et fenêtres transparentes à 75 %. Les archives v16 restent intactes.
