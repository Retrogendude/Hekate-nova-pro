# Hekate Nova Pro — audit global et optimisations v30
**Languages:** **Français** · [English](AUDIT-NOVA30.en.md) · [Español](AUDIT-NOVA30.es.md) · [Русский](AUDIT-NOVA30.ru.md)

Revue des sources du rendu, de l'ordonnancement, des commandes sans fil, des
entrées, du démarrage, des chargements SD, de la mémoire, des diagnostics et
du mode TV. Les tests utilisent les fonctions réelles avec périphériques
substitués sur PC. Aucun résultat de fréquence d'affichage ou de latence
physique v30 n'est annoncé avant un essai sur console.

## Changements intégrés

| Domaine | Constat | Correction et preuve |
| --- | --- | --- |
| Démarrage | Une session vide obligeait à charger les clés à la main | Après initialisation du pilote, détection physique des rails. Si aucun Joy-Con : chargement du `.bin` validé, puis démarrage en arrière-plan sans fenêtre. Tests absence/1rail/2rails, erreur SD, course de branchement et arrêt manuel. |
| Reflet | La file32zones pouvait déborder avant le rafraîchissement et imposer un écran entier | Pré-calcul du budget global, transitions différées et phase en temps réel.9scénarios corrigés sans débordement périodique ; rendu de trois phases identique à v29. |
| Appuis brefs | Appui et relâchement reçus entre deux lectures UI pouvaient s'annuler | Présentation unique des fronts récents, expiration100ms par bouton, aucune attente ajoutée ; nettoyage à la coupure/Stop et priorité aux rails. Le vrai v29 échoue au test ciblé. |
| Service radio | Le compteur de durée ne couvrait que la partie RX | Nouveaux maxima d'appel complet, de variante rapide/normale et d'intervalle entre appels. Le traitement du paquet courant compte dans le budget RX. |
| Longs outils SD/USB | Certains traitements n'entretiennent pas la boucle GUI complète | Le point d'entretien manuel appelle le polling rapide, hors diagnostics réservant le matériel. Test du polling et de la suspension. |
| Ressources | `res.pak` pouvait dépasser sa région ou être lu partiellement | Bornes avant lecture, taille minimale des ressources référencées, compte réellement lu et fermeture contrôlés. |
| Payloads | Un fichier refusé pouvait rester alloué ; taille vérifiée après allocation | Vérification avant allocation et nettoyage commun ; lectures incomplètes ou échouées refusées.20cas I/O passent,8anciens défauts reproduits avec le vrai code v29. |

## Mesures de rendu sur PC

À33ms entre rafraîchissements sur un cycle de8.48s simulées, les surfaces
redessinées baissent de52,1 % à l'accueil,73,7 % dans Outils et53,4 % dans le
carrousel. Les rafraîchissements complets causés par le reflet passent
respectivement de5/5/3 à0dans ces scènes. Ce sont des surfaces, pas des gains FPS.
Les tests retardent aussi le rafraîchissement à66et100ms et préservent les
invalidations des autres widgets quand la file est déjà remplie.

## Points conservés

Profil radio BR/sniff/QoS, débit UART, contrôle de flux, politique des clés,
fichier de sauvegarde72octets, choix de langue/police/couleurs et style Nova
restent compatibles. Le loader et les modules système ne reçoivent pas les
optimisations GUI. La préparation de la distribution vérifie les tailles et
les en-têtes réels afin de refuser un binaire incompatible.

Le service est coopératif : il fonctionne sur l'accueil et les menus ordinaires
sans panneau Bluetooth ouvert. Les sondes matérielles qui réservent leurs
périphériques conservent leur suspension des commandes ; l'audit ne réactive
pas les tâches pendant les fenêtres de mesure.

## Suite recommandée

| Priorité | Piste identifiée | Travail restant |
| --- | --- | --- |
| Fiabilité | Helper SD commun pouvant retourner succès après écriture incomplète | Corriger son contrat et tester tous les exports/configurations appelants. La sauvegarde Bluetooth dédiée vérifie déjà ces erreurs. |
| Fiabilité | Lecture de Nyx par le bootloader insuffisamment bornée | Ajouter une validation locale compatible avec la faible marge IRAM ; conserver l'en-tête fixe et les limites de payload. |
| Fluidité | Décodage PNG et scans polices lors des reconstructions | Mesurer puis introduire des caches bornés avec invalidation des fichiers remplacés. |
| Démarrage | Délais d'animation indépendants du travail effectué | Recouvrir animation et lectures sans changer les délais électriques, le training mémoire et la rampe d'écran. |
| Diagnostics | Longues phases sans état intermédiaire | Ajouter un rafraîchissement limité entre phases sûres, sans perturber les mesures. |
| TV | Négociation de lien et présentation parfois bloquantes | Découper en états temporisés après mesures sur dock. |

Ces points proviennent de chemins de code identifiés. Ils ne sont pas décrits
comme des pannes matérielles constatées et ne sont pas tous modifiés dans la
v30. Les détails et références figurent dans les trois rapports spécialisés.

## Vérification sur console

Conserver une sauvegarde d'appairage valide. Tester démarrage sans rails,
avec seulement L, avec seulement R et avec les deux Joy-Con fixés. Sans rails,
vérifier la navigation depuis l'accueil et dans les fenêtres sans ouvrir
Bluetooth. Un arrêt manuel doit rester arrêté jusqu'à une action manuelle ou
un nouveau démarrage. Comparer les mêmes parcours et envoyer uniquement le
rapport `last-session.txt`, jamais les fichiers contenant les clés.

Détails : [rendu](AUDIT-NOVA30-GUI.md), [Bluetooth](AUDIT-NOVA30-WIRELESS.md), [cœur et chargements](AUDIT-NOVA30-CORE.md).
