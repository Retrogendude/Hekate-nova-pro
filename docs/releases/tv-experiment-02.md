# tv-experiment-02 — Essai TV autonome 02

- Programme TV autonome de référence sur base Hekate 6.5.5 ; distinct des versions Nova.
- Essai sur dock officiel ; conserver les limites et la procédure VOL− décrites dans la notice originale.

**Nature de la livraison :** préversion compilée, sources et fichiers d’origine conservés.

## Fichiers et empreintes

| Fichier | Octets | SHA-256 |
|---|---:|---|
| [hekate-tv-experiment-02-sources.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-02/hekate-tv-experiment-02-sources.zip) | 1,692,017 | cbe08ee9441b68a086e29cc358dac1d681500b2d70ef12325fbbd37dcb99468a |
| [hekate-tv-experiment-02.zip](https://github.com/Retrogendude/Hekate-nova-pro/releases/download/tv-experiment-02/hekate-tv-experiment-02.zip) | 912,982 | 3098248e4d29e1fa175a5fc06cb557d34839638cff7c54fdfe67cc3814cc8ceb |

## Notice historique complète

Les instructions et chemins ci-dessous concernent l’archive de cette version. Ils ne décrivent pas automatiquement la dernière version. Les limitations et validations restent celles indiquées à l’époque.

# Hekate — prototype TV 02

Prototype de fiabilisation de la sortie TV intégré à ta base Hekate 6.5.5 / Nyx 1.9.5.
Il utilise l'interface classique de cette base, distincte de Nova Pro v18.
Les diagnostics BM92T36 existants sont conservés.

Le prototype 01 a affiché une image stable sur une Switch OLED avec dock
officiel, et VOL− a fonctionné. Des essais en échec ont ensuite été signalés,
après VOL− comme après redémarrage complet. La sortie TV reste intermittente.

Cette version 02 corrige deux défauts reproduits dans le code : la perte
d'une confirmation de commande pendant que le contrôleur est occupé, et
la remise à zéro du compteur lors des nouvelles demandes au hub du dock.
Les rapports sont horodatés et séparés par essai. Le protocole matériel et
les paramètres du lien vidéo sont conservés. Il n'y a aucun reset ni nouvelle
tentative complète automatique. Ces corrections ne prouvent pas encore
qu'elles résolvent les échecs observés sur la console.

Ce prototype est compilé et vérifié sur PC. L'image, sa stabilité et les
branchements/rebranchements doivent encore être validés sur console.

## Installation et lancement

1. Utiliser une carte SD d'essai, ou sauvegarder intégralement son dossier
   `bootloader` avant de copier les fichiers du prototype à la racine.
   Le paquet ne remplace ni `hekate_ipl.ini`, ni `nyx.ini`, ni `update.bin`.
2. Injecter **hekate_tv_experiment_02_6.5.5.bin**. Ce fichier doit être lancé
   explicitement ; le payload Nova Pro v18 ne charge pas cette version de Nyx.
3. Hors du dock, ouvrir **Console Info → Battery → I2C Diagnostics →
   Test TV (experimental)**.
4. Raccorder le dock officiel à son alimentation et à la TV en HDMI.
   Allumer la TV et choisir l'entrée HDMI correspondante.
5. Appuyer sur **Demarrer**, puis placer la console dans le dock dans les
   45 secondes. On peut aussi commencer avec la console déjà branchée au dock
   si le bouton est accessible.
6. Observer la TV. Le mode demandé est **1280 × 720 à 60 Hz**. Le texte
   « configuree (a verifier) » signifie que l'initialisation logicielle a abouti ;
   il faut confirmer visuellement l'image sur la TV.
7. **VOL-**, **Arreter** ou **Close** coupe la session. L'écran de la console
   reste actif. Un débranchement détecté coupe aussi la sortie.

## Rapport à récupérer après l'essai

Sur la SD : **bootloader/tv-test/last-run.txt**.
Le journal contient les identifiants, étapes de négociation, registres, lecture
du récepteur DisplayPort et diagnostic du lien vidéo. Une copie distincte de chaque essai est conservée sous `run-001.txt`,
`run-002.txt`, etc. Le fichier `last-run.txt` reste un raccourci vers le
dernier essai. Ouvrir puis fermer la fenêtre sans démarrer préserve les rapports.
La fermeture du test sauvegarde les dernières lignes. Si la SD est absente,
pleine ou illisible, le test peut fonctionner sans rapport enregistré.

Pour continuer le développement, fournir le message d’erreur affiché et les fichiers `run-NNN.txt`
d’un essai réussi et d’un échec, avec : modèle de console
(V1/V2/OLED), modèle de dock, présence ou absence d'image, stabilité de l'image,
et comportement après arrêt/retrait. Un écran TV noir ne suffit pas à désigner
un composant défectueux.

## Portée de cette étape

- Négociation d'alimentation USB-C / mode dock / DisplayPort sur demande.
- Lecture AUX, DPCD et EDID ; entraînement du lien ; seconde tête d'affichage.
- Recopie de la fenêtre de test, sans rotation de l'image de la TV.
- Aucun reset automatique du BM92T36. Switch Lite exclue.
- Arrêt explicite, limite de 45 s sur la négociation, journal borné à 32 Kio.
- Libération du tampon après arrêt du moteur vidéo ; restauration des réglages
  LDO7/LDO8, du pad HPD et de la fréquence BPMP sauvegardés.

La phase d'initialisation vidéo est synchrone, avec ses propres attentes bornées.
Pendant cette phase, les boutons sont traités quand elle rend la main ; les
45 secondes ne constituent pas un délai maximal absolu de cette phase.
La négociation peut laisser le contrat d'alimentation actif après l'arrêt du
test ; un retrait du dock termine ce contrat. La restauration complète de
tous les états du port et les transitions vers les OS restent à valider.

Le miroir s'arrête en quittant cette fenêtre. La sortie TV permanente dans
tous les menus Nova, l'audio HDMI, la navigation sans tactile et le démarrage
automatique en mode dock ne sont pas encore livrés.

Pour revenir à Nova Pro v18, restaurer le dossier `bootloader` sauvegardé puis
relancer son payload. Aucun fichier du dossier source `Desktop/nd5` n'a été changé.

Voir `documentation/TV-SOURCES.md` et `documentation/VERIFICATION-TV-02.json`
pour la provenance et les vérifications de cette compilation.

