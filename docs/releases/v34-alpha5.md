# Hekate Nova Pro v34-alpha5 — préversion

Service TV pendant les opérations longues, journaux différés et tentatives limitées. Version TV historique : son contrôle de synchronisation trop strict est corrigé en alpha7.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

Les notes d’origine sont conservées : leurs mentions « non publié » décrivent l’état au moment du développement, avant cette publication.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé.


## Téléchargements et utilisation

- Pack standard : **hekate-nova-pro-v34-alpha5.zip**.
- Sources complètes, dépendances et outils : **hekate-nova-pro-v34-alpha5-sources.zip**.
- Pack 8GB séparé lorsqu’il existe ; réservé au matériel modifié.
- Changelogs en huit langues et empreintes SHA-256 joints.
- Installateur Windows et son code source joints. Pour alpha4, la révision **1.1** corrige le dossier Payloads ; le premier installateur reste archivé. Pour les alphas suivantes, utiliser l’installateur de la même version.

Installer ensemble le dossier bootloader et le payload de cette version. Conserver les configurations et appairages personnels. Les tests PC ne constituent pas une validation matérielle ; la stabilité TV de l’alpha7 reste à confirmer.

[Notice d’utilisation](https://github.com/Retrogendude/Hekate-nova-pro/blob/v34-alpha5/docs/NOTICE-UTILISATION.md) · [Sources parcourables](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha5/hekate) · [Changelogs](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha5/docs/changelogs)

<details><summary>Notice livrée avec cette version</summary>

Hekate Nova Pro v34 alpha 5 — maintien de la sortie TV

Le mode TV est maintenant servi pendant les opérations synchrones : partitionnement SD, copie/vérification emuMMC, sauvegarde/restauration et attentes de confirmation par boutons. L’image complète est transmise après le rafraîchissement de l’interface ; les boucles longues font avancer le service dock sans relancer tout le planificateur graphique.

Les rapports TV sont différés pendant ces boucles : aucun montage, démontage ou fichier de journal TV n’est écrit en concurrence avec le partitionnement ou la copie. Les journaux en attente sont traités au retour dans la boucle normale de Nyx.

Utilisation : activer Démarrage TV/détection automatique dans Réglages Nyx, ou démarrer la TV manuellement avant l’opération ; placer la console directement dans le dock officiel alimenté et relié en HDMI. Le choix de conserver l’écran console reste respecté. Après un échec de négociation ou de lien vidéo, jusqu’à trois tentatives au total sont possibles, espacées de 1,5 puis 3 secondes après nettoyage. Aucun reset forcé du contrôleur PD ; arrêt manuel, défaut matériel signalé ou transfert de contrôle annulent les relances. Les arrêts de sécurité au retrait du dock, les transferts de contrôle USB/payload/OS et l’isolation des diagnostics matériels restent conservés. Un changement de payload, un redémarrage ou une TV débranchée n’assurent pas un affichage continu.

Installer le dossier bootloader et utiliser le payload de ce pack. Les réglages personnels sont conservés. Le payload est aussi livré dans bootloader/payloads pour apparaître dans la liste. Le pack 8GB reste réservé aux consoles physiquement modifiées.

Validation PC : 54 scénarios du service TV dont rafraîchissement manuel, fermeture de la fenêtre, dock retiré, rapports différés sans accès SD, fin de négociation, VOL−, remise en route normale et débordement du compteur temps ; fonctions réelles de maintenance/présentation/attente de boutons testées ; 3664 assertions de transaction/tables, 246 de copie/activation et 147 avec FatFs réel sur FAT32/exFAT virtuels. Compilation ARM et en-têtes binaires vérifiés. Aucun test d’affichage HDMI ou de partitionnement sur Switch effectué ici : essai sur OLED/dock officiel à effectuer sur une carte sauvegardée.

Rapports utilisateur tests 12/13 : échec d’identification du dock dans un cas ; lien entraîné mais flux vidéo non synchronisé dans l’autre. Nova attend maintenant jusqu’à une seconde une confirmation fraîche du récepteur après démarrage du flux ; absence de synchronisation, erreur AUX ou retrait HPD ne sont plus annoncés comme une configuration réussie. Ces échecs passent par le nettoyage et les tentatives limitées. Neuf scénarios supplémentaires de synchronisation sont vérifiés sur PC. Le numéro du journal TV reflète désormais la version Nova réelle. Ce correctif reste à confirmer sur la TV concernée.

</details>
