# Changelog Hekate Nova Pro

# Hekate Nova Pro v34-alpha7 — préversion

Validation alternative par captures CRC fraîches quand SINK_STATUS reste nul, avec source active et voies stables. Ne pas envoyer Exit DP pour une TV simplement détectée. Stabilité TV à confirmer sur console.

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé. Alpha6 : sortie contrôlée du mode DP hérité avant identification ; repli du seul lien vidéo vers HBR/RBR après perte de synchronisation, avec contrôle frais de toutes les voies. 429 assertions du pilote et 54 scénarios du service réussis sur PC ; essai console requis. Alpha7 : éviter la coupure d’un flux reçu lorsque SINK_STATUS reste nul. Confirmation alternative par compteur CRC frais, source active, compteurs d’erreurs valides à zéro et cinq lectures de voies alignées. Contrôle des erreurs AUX et du redémarrage du compteur. Ne plus envoyer Exit DP pour SIGNAL_ON seul. Stabilité sur console à confirmer.




# Hekate Nova Pro v34-alpha6 — préversion

Replis vidéo HBR/RBR et reprise de connexion. L’image a été rapportée puis coupée par la validation logicielle ; cette limite est traitée en alpha7.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé. Alpha6 : sortie contrôlée du mode DP hérité avant identification ; repli du seul lien vidéo vers HBR/RBR après perte de synchronisation, avec contrôle frais de toutes les voies. 429 assertions du pilote et 54 scénarios du service réussis sur PC ; essai console requis.




# Hekate Nova Pro v34-alpha5 — préversion

Service TV pendant les opérations longues, journaux différés et tentatives limitées. Version TV historique : son contrôle de synchronisation trop strict est corrigé en alpha7.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Maintien du service TV pendant les opérations longues, rafraîchissement après chaque image terminée, journaux TV différés pour éviter tout accès SD réentrant. Attentes de boutons coopératives. Réglages et arrêts matériels conservés. Validation matérielle à effectuer. Jusqu’à trois tentatives espacées après échec de connexion ; aucun reset PD forcé et arrêt manuel prioritaire. Confirmation fraîche de synchronisation vidéo (attente bornée à 1 s) ; échec explicite sans synchronisation/AUX/HPD. Numéro de journal corrigé.




# Hekate Nova Pro v34-alpha4 — préversion

Correction du faux échec emummc.ini causé par les fins de ligne FatFs ; installateur Windows autonome et correction du dossier Payloads.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Correction du faux échec d’enregistrement emuMMC/emummc.ini avant le partitionnement : écriture en octets exacts, sans conversion automatique des fins de ligne. Synchronisation et relecture conservées. Défaut reproduit et correctif testé avec le vrai FatFs Nyx sur FAT32/exFAT virtuels. Écritures courtes et protection en écriture toujours refusées. Interface et fonctions de l’alpha 3 conservées ; essai réel sur console à effectuer.

PC checks passed; physical console validation pending.




# Hekate Nova Pro v34-alpha3 — préversion

Choix Linux/Android 16 Go minimum ou 32 Go recommandé, accord d’effacement, aide et erreurs de stockage détaillées. Signalement des cartes suspectes selon l’indicateur Hekate existant.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Choix Linux et Android indépendants : 16 Go minimum / 32 Go recommandé, sans réduction automatique. Aide : emuMMC 29 Go V1/V2/Lite ou 58 Go OLED et renvoi vers le mode expert. Accord explicite « J’accepte / Je n’accepte pas ». Suivant en bas à droite uniquement dans l’assistant. Erreurs de stockage détaillées ; blocage sur l’indicateur SD suspecte de Hekate (fabricant 0x00). Cet indicateur ne détecte pas toutes les fausses capacités. Copie emuMMC vérifiée conservée. Linux/Android : partitions seulement.

PC checks passed; physical console validation pending.




# Hekate Nova Pro v34-alpha2 — préversion

Assistant agrandi ; partitionnement suivi de la copie et de la relecture intégrale de BOOT0, BOOT1 et de l’emuMMC, avec activation conditionnelle.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Bouton Assistant d’installation automatisé agrandi avec icône ; confirmation obligatoire de sauvegarde et avertissement d’effacement ; après un partitionnement réussi, copie complète emuMMC avec relecture avant activation. Vérifications de capacité, modèle, identité SD et cible ; arrêt sur erreur. Linux/Android : partitions seulement. Tests PC réussis ; validation physique à effectuer.

PC checks passed; physical console validation pending.




# Hekate Nova Pro v34-alpha1 — préversion

Première ébauche de l’assistant : présentation, modèle/OS et plan de partitionnement. La copie emuMMC automatique n’est pas encore raccordée.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

## Modifications


Premier assistant sur l’accueil : présentation, modèle et choix indépendants emuMMC/Linux/Android, récapitulatif et passage protégé vers le partitionneur. Tailles automatiques avec validation de capacité et de modèle. Copie emuMMC automatique et installation des OS non raccordées dans cette alpha.

PC checks passed; physical console validation pending.




# Hekate Nova Pro v33.3 — préversion

Halo à deux couleurs avec aperçu et sauvegarde, et corrections des contours de la v33.2.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

## Modifications


- Le choix Fond est remplacé par Halo dans Interface > Thèmes et couleurs > Couleurs.
- Couleur 1 et Couleur 2 permettent de personnaliser indépendamment les deux couleurs du contour sélectionné. L’aperçu affiche le dégradé sans appliquer les changements avant sauvegarde.
- Les deux valeurs sont conservées entre les onglets, enregistrées dans nyx.ini et utilisées après rechargement. Bleu/violet reste le réglage par défaut ; le fond existant est conservé.

PC: 98 configuration checks + 24 Halo scenes + 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.2.


# Hekate Nova Pro v33.2 beta

- Contours lumineux de sélection sur toutes les tuiles de l’accueil, les trois actions Gestion emuMMC, les quatre actions Sauvegarde, les trois actions Restauration et les huit actions Outils USB.
- Le contour suit le curseur Joy-Con ou la sélection tactile ; les boutons conservent leurs actions et les cases désactivées perdent leur contour.
- Contour extérieur complet pour Partition SD emuMMC brute et Lecture seule lorsqu’ils sont activés, y compris le bord inférieur auparavant incomplet. Les contours d’état restent visibles sans animations.

PC: 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.1.




# Hekate Nova Pro v33.2 — préversion

Contours de sélection complets sur les tuiles et options actives, dont Sauvegarde, Restauration et Outils USB. Version historique remplacée par la v33.3.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

## Modifications


- Contours lumineux de sélection sur toutes les tuiles de l’accueil, les trois actions Gestion emuMMC, les quatre actions Sauvegarde, les trois actions Restauration et les huit actions Outils USB.
- Le contour suit le curseur Joy-Con ou la sélection tactile ; les boutons conservent leurs actions et les cases désactivées perdent leur contour.
- Contour extérieur complet pour Partition SD emuMMC brute et Lecture seule lorsqu’ils sont activés, y compris le bord inférieur auparavant incomplet. Les contours d’état restent visibles sans animations.

PC: 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.1.




# Hekate Nova Pro v33 — préversion

Chinois simplifié, police dédiée, ressources et options de démarrage visibles. Première bêta historique, remplacée par les versions suivantes.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

## Modifications


- Ajout du chinois simplifié comme huitième langue, dans les réglages et au premier démarrage.
- Police chinoise dédiée et imposée uniquement pour cette langue. Le choix de police personnel est conservé et rétabli pour les autres langues ; la taille reste réglable.
- Lecture.nyxf devient la police automatique des langues latines ; le russe utilise par défaut la famille intégrée. La police Hekate d’origine est disponible dans le sélecteur.
- Trois fonds fixes et cinq icônes Nova sont inclus, ainsi que le fichier Launcher fourni pour la tuile principale.
- Nouveaux réglages d’installation : français, Carbon Cyan, opacités des fenêtres et icônes à 75 %, texte de 30 px, couleurs fournies et logo de démarrage clair. L’horloge d’origine reste à régler sur la console.
- Accès explicite aux options de démarrage : Auto Boot, délai, Auto NoGC et Auto HOS Power Off. Le lien de luminosité rejoint le réglage existant, sans deuxième curseur.
- Sélecteur Auto Boot paginé dans le thème Nova, sans liste verticale ; les indices des configurations principales et supplémentaires restent ceux de Hekate.
- Test mémoire séparé : interface chinoise et glyphes 16 × 16, compatibilité conservée avec les sept packs de langues précédents.
- Repli lisible en anglais si la ressource chinoise est absente ou invalide, sans remplacer la police personnelle. Les réglages déjà enregistrés ne sont pas écrasés par le pack.

Bêta à essayer sur console. Les contrôles PC ne remplacent pas cet essai. Les libellés principaux, les avertissements ciblés et le test mémoire chinois ont été revus ; les longs rapports comprennent encore une traduction automatique à faire relire. La publication GitHub attend le retour du testeur.





# Hekate Nova Pro v33.3 beta

- Le choix Fond est remplacé par Halo dans Interface > Thèmes et couleurs > Couleurs.
- Couleur 1 et Couleur 2 permettent de personnaliser indépendamment les deux couleurs du contour sélectionné. L’aperçu affiche le dégradé sans appliquer les changements avant sauvegarde.
- Les deux valeurs sont conservées entre les onglets, enregistrées dans nyx.ini et utilisées après rechargement. Bleu/violet reste le réglage par défaut ; le fond existant est conservé.

PC: 98 configuration checks + 24 Halo scenes + 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.2.


# Hekate Nova Pro v33.2 beta

- Contours lumineux de sélection sur toutes les tuiles de l’accueil, les trois actions Gestion emuMMC, les quatre actions Sauvegarde, les trois actions Restauration et les huit actions Outils USB.
- Le contour suit le curseur Joy-Con ou la sélection tactile ; les boutons conservent leurs actions et les cases désactivées perdent leur contour.
- Contour extérieur complet pour Partition SD emuMMC brute et Lecture seule lorsqu’ils sont activés, y compris le bord inférieur auparavant incomplet. Les contours d’état restent visibles sans animations.

PC: 192 selection/outline cases + 683 regression scenes; ARM build and binary metadata verified. Console test pending. RAM 8GB remains experimental, as in v33.1.



# Hekate Nova Pro v33.1 beta

- Tuile Démarrer : petite icône /icone/launch.png indépendante du grand fond /icone/Launcher.png, remplaçable depuis Nova ; aperçu cohérent.
- Crédits : attribution de HWTest-RCM 0.15 à Creased (Baptiste MOINE), dans Logiciels libres.
- Apparence et options : retrait du fond des lignes Vérification des sauvegardes et Effets de sélection ; contour du panneau Options système harmonisé.
- Sauvegarde et restauration emuMMC : contour actif repositionné sur le panneau extérieur, sans double cadre décalé ; comportement des boutons conservé.
- Deux packs distincts : standard et expérimental RAM 8 Go pour consoles physiquement modifiées (Erista V1 et Mariko V2/Lite/OLED). Le second applique le drapeau logiciel officiel Hekate, sans programmer de fusibles.
- Diagnostic RAM : conservation du mode 8 Go au lancement et au retour, sélection des profils selon la plateforme et la capacité, arrêt en cas de capacité incohérente après entraînement.
- Vérification des adresses hautes avec calculs 64 bits et compte rendu de la capacité réelle ; 8 176 Mio testables sur 8 192 Mio, les 16 Mio de travail restant protégés.
- Atmosphère 1.11.0 minimum pour les modes mémoire au-delà de 4 Go : memmode=1 dans une entrée Hekate utilisant fss0, ou enable_mem_mode=1 dans exosphere.ini pour fusee. Les configurations existantes ne sont pas modifiées automatiquement.

Bêta : compilations ARM et contrôles PC réussis. Le mode 8 Go, notamment le profil Erista adapté, attend une validation matérielle sur chaque plateforme ; aucune compatibilité avec toutes les puces de remplacement n’est certifiée. Les longs rapports chinois conservent une traduction automatique à relire.

## v33

- Ajout du chinois simplifié comme huitième langue, dans les réglages et au premier démarrage.
- Police chinoise dédiée et imposée uniquement pour cette langue. Le choix de police personnel est conservé et rétabli pour les autres langues ; la taille reste réglable.
- Lecture.nyxf devient la police automatique des langues latines ; le russe utilise par défaut la famille intégrée. La police Hekate d’origine est disponible dans le sélecteur.
- Trois fonds fixes et cinq icônes Nova sont inclus, ainsi que le fichier Launcher fourni pour la tuile principale.
- Nouveaux réglages d’installation : français, Carbon Cyan, opacités des fenêtres et icônes à 75 %, texte de 30 px, couleurs fournies et logo de démarrage clair. L’horloge d’origine reste à régler sur la console.
- Accès explicite aux options de démarrage : Auto Boot, délai, Auto NoGC et Auto HOS Power Off. Le lien de luminosité rejoint le réglage existant, sans deuxième curseur.
- Sélecteur Auto Boot paginé dans le thème Nova, sans liste verticale ; les indices des configurations principales et supplémentaires restent ceux de Hekate.
- Test mémoire séparé : interface chinoise et glyphes 16 × 16, compatibilité conservée avec les sept packs de langues précédents.
- Repli lisible en anglais si la ressource chinoise est absente ou invalide, sans remplacer la police personnelle. Les réglages déjà enregistrés ne sont pas écrasés par le pack.



Historique importé depuis les archives conservées. Les tags décrivent des instantanés de sources, pas les commits de développement d’origine. Les notices détaillées sont conservées pour chaque version. Les prototypes et versions Bluetooth restent des préversions.

## v32 — Nova Pro 32

- Récapitulatif du partitionnement sur une seule ligne ; valeurs à droite, rangées aérées, poignées et boutons séparés.
- Relance d’un flux Joy-Con silencieux puis reconnexion du seul pair concerné, avec protection contre les données UART encore en attente.
- État radio WL_REG_ON aligné sur la référence HWTest pendant la session et restauré à l’arrêt.
- Compilation et contrôles PC réussis. L’usage à 2–3 mètres reste à valider sur console ; aucun gain de portée n’est garanti.

[Notes détaillées](docs/releases/v32.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v32)

## v31 — Nova Pro 31

- Partitionnement expert pleine largeur, récapitulatif inférieur et réglages −/+ ; tailles Go entières avec 29 / 58 Go Full.
- Traitement Bluetooth pendant le dessin logiciel, les copies d’image et les attentes d’affichage.
- Mesures HCI en lecture seule : RSSI relatif, qualité, puissance et contacts échoués ; statistiques de trous de rapports complets.

[Notes détaillées](docs/releases/v31.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v31)

## v30 — Nova Pro 30

- Au démarrage, chargement SD et Bluetooth en arrière-plan seulement lorsque les deux rails sont vides.
- Appuis brefs conservés, invalidations décoratives bornées et traitement coopératif dans les outils longs.
- Audit des lectures de ressources et du nettoyage ; diagnostics conservant leur usage réservé des périphériques.

[Notes détaillées](docs/releases/v30.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v30)

## v29 — Nova Pro 29

- Acceptation des profils d’appairage Nintendo et standard déjà validés ; rejet des réponses SPI tronquées.
- Sauvegarde / chargement SD privés de 72 octets avec somme de contrôle, copie de secours et remplacement transactionnel.
- Délai de connexion de 5,12 s, nouvelles tentatives espacées sans arrêt définitif au troisième échec.

[Notes détaillées](docs/releases/v29.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v29)

## v28 — Nova Pro 28

- Budget court de 250 µs pendant l’affichage et rafraîchissement moins fréquent du panneau Bluetooth.
- Diagnostic de l’import des appairages, compteurs de boutons et cadence de rapports.
- Cette version diagnostique le 0/2 mais ne corrige pas encore sa classification ; corrigée ensuite en v29.

[Notes détaillées](docs/releases/v28.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v28)

## v27 — Nova Pro 27

- Profil ROM adapté de HWTest-RCM : paquets au débit de base, veille / mode actif, QoS et lecture UART prolongée.
- Alimentation, horloge et restauration de l’état précédent ; diagnostics de contrôleur enrichis.
- Premiers retours matériels de commandes sans fil, avec latence et fiabilité restant à améliorer.

[Notes détaillées](docs/releases/v27.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v27)

## v26 — Nova Pro 26

- Acceptation des ouvertures HID simultanées initiées par Nova et par le Joy-Con.
- Gestion des canaux obsolètes, refus tardifs et délais, avec conservation de la session valide.
- Corrections vérifiées par des négociations HCI/L2CAP/HID simulées, sans garantie matérielle de portée.

[Notes détaillées](docs/releases/v26.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v26)

## v25 — Nova Pro 25

- Préférence pour le rôle central lors d’une reconnexion entrante ; tentatives sortantes sérialisées.
- Activation du chiffrement et états de connexion mieux suivis ; métadonnées de contrôleur et rôles ajoutés au rapport.

[Notes détaillées](docs/releases/v25.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v25)

## v24 — Nova Pro 24

- Correction de l’ordre des octets des clés importées et contrôle de l’identité Bluetooth de la console.
- Import depuis les rails vérifié par le chemin SPI réel et comparaison à des valeurs de référence indépendantes.

[Notes détaillées](docs/releases/v24.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v24)

## v23 — Nova Pro 23

- Reprise de la connexion manette d’après les échecs HID observés ; ouverture des canaux connus sans attendre la description SDP.
- Rapport anonyme des négociations, authentifications et étapes HID ; validation matérielle encore incomplète.

[Notes détaillées](docs/releases/v23.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v23)

## v22 — Nova Pro 22

- Mise à jour du cœur vers Hekate 6.5.5 / Nyx 1.9.5 et reprise des corrections amont documentées, sans remplacer le thème Nova.
- Correction du faux avertissement Old Nyx GUI found : métadonnées à l’offset attendu et garde de liaison.
- Navigation avec le Joy-Con disponible, réveil du curseur masqué, gestion des rapports simples et conservation des réglages.

[Notes détaillées](docs/releases/v22.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v22)

## v21 — Nova Pro 21 — Bluetooth

- Premier service Bluetooth personnel pour les Joy-Con détachés, avec import des appairages et navigation.
- Bandeau Lecture seule d’Outils USB remis en forme ; fond du panneau Crédits restauré.
- Onglet Changelog ajouté aux crédits. Version historique expérimentale, suivie de plusieurs corrections Bluetooth.

[Notes détaillées](docs/releases/v21.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v21)

## v20 — Nova Pro 20 — Dock

- Détection automatique du dock et basculement d’affichage après démarrage.
- Réglages TV seule ou TV + écran console ; pilote TV conservé entre les menus.
- Branchement de référence : console compatible directement insérée dans le dock officiel ; la Switch Lite ne fournit pas de sortie TV.

[Notes détaillées](docs/releases/v20.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v20)

## v19 — Nova Pro 19 — TV

- Première intégration TV manuelle dans Nova, maintenue entre les menus.
- Référence matérielle rapportée : prototype TV01, OLED directement dans un dock officiel, image stable et arrêt VOL−.
- Ne pas présenter cette observation du prototype comme une validation de toutes les consoles ou de toutes les versions TV.

[Notes détaillées](docs/releases/v19.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v19)

## tv-experiment-02 — Essai TV autonome 02

- Programme TV autonome de référence sur base Hekate 6.5.5 ; distinct des versions Nova.
- Essai sur dock officiel ; conserver les limites et la procédure VOL− décrites dans la notice originale.

[Notes détaillées](docs/releases/tv-experiment-02.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-02)

## tv-experiment-01 — Essai TV autonome 01

- Programme TV autonome de référence sur base Hekate 6.5.5 ; distinct des versions Nova.
- Essai sur dock officiel ; conserver les limites et la procédure VOL− décrites dans la notice originale.

[Notes détaillées](docs/releases/tv-experiment-01.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/tv-experiment-01)

## v18 — Nova Pro 18

- Crédits limités aux onglets supérieurs, auteurs et licences conservés.
- Icônes PNG transparentes depuis le dossier icone, image Launch et compatibilité BMP.
- Contours retirés d’Options système ; V1/V2 regroupées et capacité FAT32 calculée dans les profils compatibles.
- Minimum / Recommandé pour Linux et Android, tailles Go en mode expert. La sortie TV est encore une étude à ce stade.

[Notes détaillées](docs/releases/v18.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v18)

## v17 — Nova Pro 17

- Palette rectangulaire, aperçu persistant entre les cibles et couleurs de démarrage indépendantes.
- Personnalisation de la tuile Launch, centrage du logo et progression de chargement.
- Bilan de quinze composants, couverture réelle des tests et pistes de recherche ; export des contrôles complémentaires.
- Tests interactifs Joy-Con/Lite, sticks, grille tactile, mires écran et test RAM à modes rapide / avancé / personnalisé.

[Notes détaillées](docs/releases/v17.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v17)

## v16 — Nova Pro 16

- Fond Carbon et couleurs Nova par défaut ; fenêtres à 75 % de transparence au premier lancement.
- Partitionnement compact simplifié / expert : profils console, emuMMC complète, Linux et Android indépendants.
- Luminosité manuelle à droite de l’horloge ; sauvegarde/restauration, USB et journaux remis en page.
- Bordure de sélection bleu/violet et reflet accéléré ; animations de fond toujours supprimées.

[Notes détaillées](docs/releases/v16.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v16)

## v15 — Nova Pro 15

- Lisibilité des rapports, alignement eMMC et pagination des tables ; traduction des sauvegardes et journaux.
- Aperçu cumulatif des couleurs, introduction progressive et reflet de 0,48 seconde.
- Colonnes emuMMC alignées, sélection des fichiers par pages et sous-menus recentrés.
- Convertisseur de polices corrigé pour les accents et Caviar. Archive source restaurée depuis le snapshot v15 conservé ; 23 empreintes de sources modifiées concordent, les deux fichiers payload générés étant exclus.

[Notes détaillées](docs/releases/v15.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v15)

## v14 — Nova Pro 14

- Reflet élargi à 129 pixels avec dégradé transparent aux extrémités.
- Passage accéléré à environ 0,8 seconde, toujours sur la tuile complète.

[Notes détaillées](docs/releases/v14.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v14)

## v13 — Nova Pro 13

- Correction de l’ordre des fenêtres et de la navigation tactile lorsqu’un fond d’écran est actif.
- Reflet blanc diagonal sur toute la tuile sélectionnée, contours fixes.

[Notes détaillées](docs/releases/v13.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v13)

## v12 — Nova Pro 12

- Ordre d’affichage du fond d’écran corrigé ; six couleurs indépendantes, dont logo, icônes et sélection.
- Accès aux crédits par le logo, température colorée et interrupteurs assortis au thème.
- Réglage date/heure Hekate d’origine conservé et enrichi du choix de langue au premier démarrage.

[Notes détaillées](docs/releases/v12.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v12)

## v11 — Nova Pro 11

- Batterie à droite ; cinq onglets à gauche et quatre actions rapides à droite.
- Style Nova étendu aux sous-menus, suppression des banderoles, sélection en deux appuis et nouveau monogramme.
- Couleurs indépendantes, fond d’écran BMP, opacité des menus et icônes, démarrage Hekate Nova Pro.
- Suppression des vagues, particules et autres animations de fond ; effets limités à la sélection.

[Notes détaillées](docs/releases/v11.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v11)

## v10 — Nova

- Adoption du rendu Nova : onglets supérieurs, cartes anthracite/bleu et icônes Lucide fines.
- Grande carte de démarrage et quatre accès secondaires ; grilles Outils / Infos et douze catégories Diagnostic.
- Bandeau de détail de la sélection et accueil Tuiles / Carrousel conservé.

[Notes détaillées](docs/releases/v10.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v10)

## v9 — Studio

- Présentation Studio avec navigation latérale, fond anthracite, accents violets et angles de six pixels.
- Halo de sélection fixe sans rafraîchissement continu ; interrupteurs ON/OFF traduits et menus cadrés entre les bandeaux.

[Notes détaillées](docs/releases/v9.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v9)

## v8 — Interface compacte et fluidité

- Suppression des grandes listes verticales au profit d’onglets compacts et de sélecteurs visibles.
- Contenu contenu entre la navigation haute et la barre d’état basse ; réglages Interface / Système et Apparence répartis.
- Cartes sombres transparentes, contours moins arrondis et aides longues accessibles par boutons.

[Notes détaillées](docs/releases/v8.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v8)

## v7 — Personnalisation

- Menu Apparence : langue, police SD au format NYXF, taille de texte, disposition et effets de fond.
- Traduction étendue des menus et textes techniques ; contrôles d’apparence regroupés.
- Les animations de fond de cette version ont été retirées plus tard pour alléger le rendu.

[Notes détaillées](docs/releases/v7.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v7)

## v6 — Transparence et langues — prototype 6

- Style moderne transparent dans les menus, contrôles et cartes, avec icône Diagnostic.
- Sept langues : anglais, français, espagnol, italien, russe, allemand et portugais ; accents et cyrillique.
- Tuiles / Carrousel, 42 diagnostics natifs et test RAM isolé avec restitution du résultat au retour.

[Notes détaillées](docs/releases/v6.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v6)

## v5 — Diagnostics complets — prototype 5

- Intégration des 42 entrées natives HWTest-RCM, catégories, exécution individuelle et rapports exportables.
- Service de test mémoire distinct avec retour vers Hekate ; conservation des sondes et commentaires de provenance.
- L’archive de sources est disponible ; aucun paquet binaire spécifique v5 n’a été retrouvé parmi les livraisons conservées.

[Notes détaillées](docs/releases/v5.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v5)

## v4 — Tuiles et carrousel — prototype 4

- Deux accueils Tuiles / Carrousel sélectionnables depuis les réglages Nyx.
- L’archive custom-sources appartient également à cette étape ; ses notes légèrement différentes sont conservées comme document historique. Sources uniquement.

[Notes détaillées](docs/releases/v4.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v4)

## v3 — Interface originale — prototype 3

- Retour à la disposition de l’interface originale, avec accès Diagnostic.
- Correction de lecture DRAM conservée. Sources uniquement.

[Notes détaillées](docs/releases/v3.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v3)

## v2 — Interface console — prototype 2

- Accueil de type console avec grandes cartes et accès aux fonctions principales.
- Correction de lecture du second octet des canaux DRAM ; diagnostics conservés. Sources uniquement.

[Notes détaillées](docs/releases/v2.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v2)

## v1 — Diagnostic natif — prototype 1

- Nouvel onglet Diagnostic dans Nyx ; identification écran, rétroéclairage et DRAM.
- Verdicts PASS/WARN/FAIL et adaptateur Nyx pour les sondes HWTest. Sources uniquement ; le test DRAM identifie la mémoire sans en parcourir toutes les cellules.

[Notes détaillées](docs/releases/v1.md) · [Téléchargements](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v1)
