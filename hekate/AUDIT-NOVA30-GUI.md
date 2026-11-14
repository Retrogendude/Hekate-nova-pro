# Audit graphique Nova30 — 13 novembre 2026
**Languages:** **Français** · [English](AUDIT-NOVA30-GUI.en.md) · [Español](AUDIT-NOVA30-GUI.es.md) · [Русский](AUDIT-NOVA30-GUI.ru.md)

Le reflet sélectionné produisait des invalidations toutes les20ms, alors que
le rafraîchissement LVGL intervient toutes les33ms. La file contient32zones.
Deux phases pouvaient s'accumuler avant leur consommation et provoquer le
repli LVGL sur l'écran entier. Le test de rendu précédent vidait cette file
avec `lv_refr_now()` après chaque phase ; il ne reproduisait pas ce cas.

La correction dans `frontend/gui_nova_reflection.c` calcule le nombre de zones
d'une transition avant insertion. Si elles ne tiennent pas dans la file
globale, la transition décorative attend ; les invalidations des autres objets
restent intactes. La prochaine transition rejoint la phase de l'horloge réelle,
sans rejouer une liste d'états intermédiaires. Les cibles sont parcourues en
alternance. Le gradient blanc, la bordure et les dimensions restent identiques.

`check_nova30_reflection_timing.py` utilise les vrais objets et dessins LVGL,
une copie instrumentée de `lv_refr.c` pour compter les débordements, et des
rafraîchissements33/66/100ms. Il compare le vrai effet v29 au nouvel effet,
sans changer le LVGL distribué. Les pixels aux phases0/5/16 sont identiques.
Après mise en pause stabilisée, aucun pixel décoratif n'est redessiné.

Résultats sur un cycle de8.48s simulées, rafraîchissement33ms :

| Scène | Pixels redessinés v29 | Pixels redessinés v30 | Réduction | Écrans entiers v29 → v30 |
| --- | ---: | ---: | ---: | ---: |
| Accueil | 8 073 993 | 3 868 461 | 52,1 % | 5 → 0 |
| Outils | 6 106 329 | 1 605 560 | 73,7 % | 5 → 0 |
| Carrousel | 4 946 025 | 2 306 984 | 53,4 % | 3 → 0 |

Les9cas corrigés (3scènes × 3cadences) n'ont aucun débordement périodique,
avec un pic de28zones. Avec30invalidations utilisateur déjà présentes,
l'effet conserve ces30zones et n'en provoque pas la suppression par débordement.
Une ouverture de fenêtre, une pause ou un autre producteur peut toujours
demander un dessin complet ; cette correction cible le producteur périodique.

Ces nombres sont des surfaces dessinées sur PC, pas des FPS ni une mesure de
temps sur Switch. Les phases intermédiaires peuvent être sautées lorsque le
rafraîchissement est occupé. La cadence matérielle et les réglages visuels de
Nova sont conservés.

Autres pistes à mesurer ensuite : décodage des PNG personnalisés et scans de
polices pendant les reconstructions, coût des grandes surfaces transparentes,
rafraîchissement des statistiques et temps de présentation TV. Un cache borné
peut aider, mais il doit conserver le remplacement des fichiers SD et la
libération des ressources lors des changements de langue/police/thème.

La v30 conserve les tests d'interface avec3polices,7langues et2tailles,
incluant le nouveau message de démarrage automatique.
