# Hekate Nova Pro v34-alpha6 — préversion

Replis vidéo HBR/RBR et reprise de connexion. L’image a été rapportée puis coupée par la validation logicielle ; cette limite est traitée en alpha7.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

Les notes d’origine sont conservées : leurs mentions « non publié » décrivent l’état au moment du développement, avant cette publication.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé. Alpha6 : sortie contrôlée du mode DP hérité avant identification ; repli du seul lien vidéo vers HBR/RBR après perte de synchronisation, avec contrôle frais de toutes les voies. 429 assertions du pilote et 54 scénarios du service réussis sur PC ; essai console requis.


## Téléchargements et utilisation

- Pack standard : **hekate-nova-pro-v34-alpha6.zip**.
- Sources complètes, dépendances et outils : **hekate-nova-pro-v34-alpha6-sources.zip**.
- Pack 8GB séparé lorsqu’il existe ; réservé au matériel modifié.
- Changelogs en huit langues et empreintes SHA-256 joints.
- Installateur Windows et son code source joints. Pour alpha4, la révision **1.1** corrige le dossier Payloads ; le premier installateur reste archivé. Pour les alphas suivantes, utiliser l’installateur de la même version.

Installer ensemble le dossier bootloader et le payload de cette version. Conserver les configurations et appairages personnels. Les tests PC ne constituent pas une validation matérielle ; la stabilité TV de l’alpha7 reste à confirmer.

[Notice d’utilisation](https://github.com/Retrogendude/Hekate-nova-pro/blob/v34-alpha6/docs/NOTICE-UTILISATION.md) · [Sources parcourables](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha6/hekate) · [Changelogs](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha6/docs/changelogs)

<details><summary>Notice livrée avec cette version</summary>

Hekate Nova Pro v34 alpha 6 — correctif dock / reprise vidéo

Rapports utilisateur run-021 à run-024 : les tentatives sans réponse à Discover Identity démarrent avec un ancien état DP actif (4182). La tentative partie de 4000 identifie le dock, puis le lien HBR2 perd son égalisation après le démarrage du flux (77/81 devient 11/80). La TV reste sans signal ; l'extinction de la LED résulte de l'arrêt du pilote après échec. Ces observations orientent les corrections, sans établir une validation matérielle de celles-ci.

- Nettoyage d'un mode DisplayPort hérité avant de redemander l'identité : commande Exit DP existante, attente de fin puis délai de stabilisation habituel. Aucun reset du contrôleur PD ni changement des tensions.
- Le premier lien conserve le réglage précédent. S'il s'entraîne mais ne synchronise pas la vidéo, reprise du seul lien vidéo à 2,7 puis 1,62 Gbit/s, dans les limites du récepteur. La session du dock est conservée pendant ces replis ; pas de boucle infinie.
- Vérification fraîche de la récupération d'horloge, de l'égalisation, du verrouillage des symboles, de l'alignement et de la synchronisation du flux ; un ancien bit de succès ne suffit pas.
- Maintien des correctifs alpha5 : service TV pendant les opérations longues, image transmise après composition, journaux différés pour éviter les accès SD concurrents, jusqu'à trois sessions espacées, arrêts de sécurité et commandes manuelles conservés.

Contrôles PC : 54 scénarios du service TV ; 429 assertions sur les fonctions réelles du pilote (376 synchronisation, 30 replis, 16 mode hérité, 7 calculs 720p60), intégration aux maintenances/attentes de boutons ; 3664 assertions de partitionnement, 246 de copie, 147 avec FatFs réel sur FAT32/exFAT virtuels. Compilation ARM et en-têtes vérifiés. Aucun test HDMI ou écriture sur carte physique ici.

Installer bootloader et utiliser le payload de ce pack ; conserver ses réglages existants. Activer la détection TV automatique dans Nyx si souhaité. Tester d'abord un démarrage directement sur dock officiel, puis un retrait/remise, avant toute opération longue. En cas d'échec, transmettre last-run.txt et l'historique run-*.txt de bootloader/tv-test : le journal doit indiquer 34-alpha6 et mentionner les replis de débit lorsqu'ils ont lieu. Ne pas partitionner une SD sans sauvegarde de ses données.

Le pack 8GB expérimental est uniquement destiné aux consoles physiquement modifiées en RAM. Aucun upload GitHub de cette alpha ; validation sur console requise.

</details>
