# Origine de l'intégration TV Nova 20-Dock

Base Nova Pro v18 : copie séparée de `work/hekate-6.5.4`, sans remplacement de
la version déjà livrée. Pilote vidéo `dpout.c/.h` identique au prototype TV01.
Le pilote `usbpd.c/.h` reprend les deux correctifs de protocole TV02, vérifiés
par une régression sur la confirmation des commandes et le budget du hub.

Origine HWTest-RCM / Creased :
https://github.com/Creased/HWTest-RCM/tree/3731bfb471515a5f83709aa496c9cdb0a9aeff83

Références du pilote amont : pilotes BM92TXX et Tegra NVIDIA de CTCaer /
switchroot L4T, et pilote DisplayPort Tegra210 de coreboot.
Commentaires et tables de réglage d'origine conservés. Pas de retouche des
tensions, niveaux de pilotage, taux de lien ou chronogrammes pour ce port.

Changements Nova : session indépendante de la fenêtre de réglages, recopie des
lignes modifiées de son tampon 1280x720, arrêt avant fin matérielle/USB/diagnostic,
fréquence de menu restaurée après connexion, nouvelles vues dans le thème Nova
et 36 phrases TV/dock traduites dans les sept langues disponibles. Les noms d'étapes,
registres et détails bruts du pilote restent des informations de diagnostic.

Réparation distincte de l'enregistrement SD : utilisation de `sd_mount() == 0`
pour reconnaître le succès, avec test du vrai corps de cette fonction.
Les prototypes précédents utilisaient un test inversé et un mock incorrect.

Validation physique rapportée : prototype TV01, OLED insérée directement dans
un dock officiel, image stable et arrêt VOL- fonctionnel. Les essais utilisant
un câble console/dock ne servent pas à qualifier le branchement direct.
La validation physique de ce nouveau port Nova reste à effectuer.

Hekate et l'intégration suivent les licences fournies dans le projet.
Le dépôt HWTest-RCM consulté ne précise pas de licence autonome pour ces
nouveaux fichiers : clarification nécessaire avant une diffusion publique de
l'ensemble. Livraison personnelle de développement, avec provenance conservée.

## Détection automatique dans 20-Dock

Préférences `tv_auto` (défaut0) et `tv_keep_screen` (défaut1) ajoutées en fin
de la configuration Nyx pour préserver les anciens champs. Chargement strict,
écriture atomique par le mécanisme existant. Activation avec Enregistrer.

Le détecteur passif lit STATUS1, l'offre PDO et l'identifiant du contrôleur
quand aucun câble n'est signalé. Il ne lit pas ALERT_STATUS et n'envoie aucune
commande. Il utilise la signature d'alimentation déjà reconnue par ce pilote
pour un dock Nintendo ; l'identification/négociation complète vient ensuite.
Deux échantillons positifs sont requis après un délai initial de2secondes.
Une tentative par insertion détectée ; les erreurs ou un arrêt manuel ne
déclenchent pas de répétition immédiate. Les erreurs de lecture espacent les
sondages à2puis5secondes et ne sont pas assimilées à un retrait.

Le service suspend sa surveillance pendant USB/diagnostics et rétablit celle-ci
à leur retour, sans annuler un arrêt manuel préalable. Le choix TV seule
éteint le rétroéclairage après initialisation vidéo, et le rétablit à l'arrêt
avec la luminosité courante. La sortie vers un autre système évite un flash
de rétroéclairage juste avant la fin du pilote LCD.

Aucun service Bluetooth nouveau n'est intégré : voir l'étude Joy-Con dédiée.
La nouvelle détection automatique et la commutation d'écran restent à valider
sur console, séparément du succès du prototype01 en dock direct.

## Alpha6 — reprise d'état et de lien vidéo (27 novembre 2026)

Les rapports Nova alpha5 run-021 à run-024 montrent des échecs d'identification
avec DP=4182 ; un départ de DP=4000 aboutit à l'identification, puis perd
l'égalisation après entraînement HBR2 (77/81 devient 11/80). L'utilisateur
confirme une LED brève mais aucune image. Cette corrélation motive les essais
suivants ; elle ne prouve pas à elle seule la cause de tous les échecs.

Le service quitte un ancien état DP avec la commande 0x3B3B existante, attend
son acquittement et le délai de stabilisation. L'indication SIGNAL_ON peut
rester haute avec une TV connectée : elle n'est pas exigée basse après sortie.
Les définitions BM92T et la fonction Exit DP sont documentées par le pilote
amont : https://lkml.rescloud.iu.edu/hypermail/linux/kernel/2605.1/03297.html

Le lien conserve son débit initial ; si le flux ne synchronise pas après
entraînement, il est arrêté proprement et repris aux débits standards
inférieurs HBR puis RBR, sans recréer la session PD entre ces essais. Le budget
720p60 et les paramètres TU sont recalculés à chaque débit. La confirmation
lit ensemble les voies, l'alignement et la synchronisation ; ni l'entraînement
ni un bit de synchronisation ancien ne suffisent. Référence des contrôles
de voies et du budget : https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/tegra/dp.c

Ces changements ne modifient pas les tensions ou les tables de réglage des
pads et n'ajoutent aucun reset PD automatique. Tests PC sur les fonctions
réelles avec registres simulés et compilation ARM réussis ; HDMI à valider.

## Alpha7 — ne pas couper une image reçue (27 novembre 2026)

Retour utilisateur alpha6 : activation régulière, image qui apparaît puis
disparaît. Dans run-025/027/029, le lien RBR a des voies alignées (77/81),
des compteurs de symboles valides à zéro (8000/8000) et trois captures CRC
rapportées, mais SINK_STATUS reste nul. L'alpha6 détruisait alors la session.
Le verdict exigeant ce bit seul était trop strict pour ce convertisseur.

Une validation alternative exige maintenant des captures CRC nouvelles,
une source active et cinq lectures alignées consécutives, sans erreurs de
symboles. La capture lit son compteur de départ après activation ; toutes
les lectures/écritures AUX, y compris l'arrêt, doivent réussir. Ni une CRC
ancienne, ni un compteur inchangé ou illisible ne valident le flux.

Le mécanisme de lecture d'une CRC nouvelle par changement du compteur suit
le principe de drm_dp_aux_get_crc ; son utilisation comme preuve alternative
avec les gardes ci-dessus est une adaptation Nova, à valider sur console :
https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/display/drm_dp_helper.c

Correction supplémentaire : SIGNAL_ON seul peut indiquer une TV connectée,
pas un mode DP actif. run-026/028 refusent Exit DP avec DP=4182. La préparation
n'envoie donc Exit DP que si DP_EN est réellement haut, et conserve l'attente
d'acquittement pour ce cas. Les niveaux électriques restent inchangés.
