# Origine de l'intégration TV Nova 19-TV

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
et 26 phrases traduites dans les sept langues disponibles. Les noms d'étapes,
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
