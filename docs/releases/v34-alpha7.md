# Hekate Nova Pro v34-alpha7 — préversion

Validation alternative par captures CRC fraîches quand SINK_STATUS reste nul, avec source active et voies stables. Ne pas envoyer Exit DP pour une TV simplement détectée. Stabilité TV à confirmer sur console.

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

Les notes d’origine sont conservées : leurs mentions « non publié » décrivent l’état au moment du développement, avant cette publication.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé. Alpha6 : sortie contrôlée du mode DP hérité avant identification ; repli du seul lien vidéo vers HBR/RBR après perte de synchronisation, avec contrôle frais de toutes les voies. 429 assertions du pilote et 54 scénarios du service réussis sur PC ; essai console requis. Alpha7 : éviter la coupure d’un flux reçu lorsque SINK_STATUS reste nul. Confirmation alternative par compteur CRC frais, source active, compteurs d’erreurs valides à zéro et cinq lectures de voies alignées. Contrôle des erreurs AUX et du redémarrage du compteur. Ne plus envoyer Exit DP pour SIGNAL_ON seul. Stabilité sur console à confirmer.


## Téléchargements et utilisation

- Pack standard : **hekate-nova-pro-v34-alpha7.zip**.
- Sources complètes, dépendances et outils : **hekate-nova-pro-v34-alpha7-sources.zip**.
- Pack 8GB séparé lorsqu’il existe ; réservé au matériel modifié.
- Changelogs en huit langues et empreintes SHA-256 joints.
- Installateur Windows et son code source joints. Pour alpha4, la révision **1.1** corrige le dossier Payloads ; le premier installateur reste archivé. Pour les alphas suivantes, utiliser l’installateur de la même version.

Installer ensemble le dossier bootloader et le payload de cette version. Conserver les configurations et appairages personnels. Les tests PC ne constituent pas une validation matérielle ; la stabilité TV de l’alpha7 reste à confirmer.

[Notice d’utilisation](https://github.com/Retrogendude/Hekate-nova-pro/blob/v34-alpha7/docs/NOTICE-UTILISATION.md) · [Sources parcourables](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha7/hekate) · [Changelogs](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha7/docs/changelogs)

<details><summary>Notice livrée avec cette version</summary>

Hekate Nova Pro v34 alpha 7 — maintien d'une image réellement reçue

Retours alpha6 run-025 à run-029 : l'image apparaît puis disparaît, activation régulière. HBR2 perd l'égalisation. En HBR/RBR, les voies sont alignées, les compteurs d'erreurs valides sont à zéro et le récepteur compte des captures CRC en RBR, mais SINK_STATUS reste nul. L'alpha6 coupait alors la sortie : son contrôle de synchronisation était trop strict pour ce dock.

- Validation alternative par des captures CRC fraîches du récepteur si SINK_STATUS reste nul. Exiger l'avancement du compteur après activation de la capture, toutes les lectures/écritures AUX réussies, une source active, des compteurs d'erreurs valides à zéro et cinq lectures consécutives de voies alignées (80 ms). Une ancienne valeur CRC non nulle ne suffit pas.
- Le début du compteur est lu après activation, pour éviter de prendre sa remise à zéro pour des images reçues. La capture est arrêtée après le contrôle et un échec de cet arrêt invalide la preuve.
- La présence d'une TV seule (SIGNAL_ON, cas 4182 des run-026/028) ne déclenche plus Exit DP lorsque DP_EN est déjà nul. Cela évite la commande refusée par le contrôleur pour un mode déjà arrêté.
- Conservation du débit initial puis des replis HBR/RBR, du nettoyage d'un vrai mode DP actif et des arrêts au retrait/commande manuelle. Service TV pendant les opérations longues et journaux différés conservés.

Compilation ARM, en-têtes, 54 scénarios du service, 957 assertions du pilote et intégration aux boucles longues vérifiés sur PC. Les contrôles de copie/partitionnement/FatFs de l'alpha6 restent applicables aux sources identiques, vérifiées par empreintes. Aucun essai HDMI ni écriture sur une SD physique ici. Le retour d'image alpha6 est celui rapporté par l'utilisateur ; la stabilité alpha7 reste à confirmer.

Installer le pack complet et employer son payload alpha7. Garder les réglages existants et Standard pour une console non modifiée en RAM. Tester d'abord la permanence de l'image au démarrage et après retrait/remise sur dock officiel. Les nouveaux journaux doivent indiquer 34-alpha7 ; si la validation alternative est utilisée, ils contiennent 'video stream confirmed by fresh sink CRC frames and stable lanes'. En cas de coupure, transmettre les derniers run-*.txt et last-run.txt de bootloader/tv-test. Valider l'affichage avant de tester les opérations longues. Sauvegarder les données SD avant partitionnement.

Le pack 8GB expérimental reste destiné aux consoles physiquement modifiées. Aucun upload GitHub de cette alpha.

</details>
