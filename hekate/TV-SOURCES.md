# Provenance du prototype TV 01

Base de travail : copie de `C:/Users/Retrogen/Desktop/nd5`, Hekate 6.5.5 /
Nyx 1.9.5. Les ajouts BM92T36 et la fenêtre I2C de cette base sont conservés.
Le dossier original et Nova Pro v18 n'ont pas été modifiés.

Le pilote expérimental reprend `dpout.c/.h` et `usbpd.c/.h` de
[HWTest-RCM, Creased](https://github.com/Creased/HWTest-RCM/tree/3731bfb471515a5f83709aa496c9cdb0a9aeff83),
révision `3731bfb471515a5f83709aa496c9cdb0a9aeff83`.
Ses références sont les pilotes de CTCaer / switchroot L4T, les pilotes NVIDIA
Tegra et coreboot. Les commentaires et tables d'origine sont conservés.
La documentation amont décrit des essais sur dock Nintendo ; cela ne valide
pas cette adaptation à Nyx sur une console physique.

Modifications locales du pilote :

- remplacement des dépendances HWTest par les en-têtes BDK ;
- arrêt des horloges AUX possédées par la session ;
- délais I2C/AUX valides au débordement du compteur ;
- nettoyage des PLL après démarrage incomplet et suivi des attaches partielles ;
- refus de modifier des PLL vidéo déjà actives ;
- vérification des erreurs de lecture des identifiants ;
- rejet des PDO non fixes, prévention d'un calcul négatif du courant disponible.

Intégration Nyx : `frontend/gui_tv.c/.h`, entrée dans la fenêtre I2C,
copie des lignes modifiées après le rafraîchissement LVGL, bouton VOL- d'arrêt,
journal SD, fin de session lors de la destruction de la fenêtre.

Les ressources SD proviennent de la
[publication officielle Hekate 6.5.5 / Nyx 1.9.5](https://github.com/CTCaer/hekate/releases/tag/v6.5.5).
SHA-256 de son archive :
`e3c6613fc9e099d00683063855812a6059247da6e9d75f2caa203f9e78e5df56`.
Les composants Hekate, Nyx, Minerva et LP0 sont remplacés par notre compilation.

Hekate et le code d'intégration suivent la GPL-2.0 fournie dans `LICENSE`.
Le dépôt HWTest-RCM consulté n'indique pas de licence autonome pour ces nouveaux
fichiers : sa clarification reste nécessaire avant une diffusion publique de
la version intégrée. Cette livraison est un prototype personnel de développement.

Les sources amont originales, leurs empreintes et les différences locales sont
jointes à l'archive de sources, pour conserver leur provenance vérifiable.
