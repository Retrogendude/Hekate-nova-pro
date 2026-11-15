# Origine de l'intégration TV Nova 20-Dock
**Languages:** **Français** · [English](TV-SOURCES.en.md) · [Español](TV-SOURCES.es.md) · [Русский](TV-SOURCES.ru.md)

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
