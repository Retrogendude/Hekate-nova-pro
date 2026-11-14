# Hekate Nova Pro 20-Dock
**Languages:** **Français** · [English](LIRE-MOI-V20-DOCK.en.md) · [Español](LIRE-MOI-V20-DOCK.es.md) · [Русский](LIRE-MOI-V20-DOCK.ru.md)

Cette version ajoute la détection automatique du dock aux fonctions TV de
Nova19-TV. Le branchement de référence est une Switch OLED insérée directement
dans un dock officiel Nintendo, avec alimentation et HDMI raccordés.
L'activation automatique de cette version reste à essayer sur console.

## Installer

Sauvegarde ton dossier `bootloader`, ou utilise une carte SD d'essai. Copie le
dossier `bootloader` fourni à la racine, puis lance
`hekate_nova_pro_6.5.4_v20_dock.bin`. Vérifie **20-Dock** dans les crédits.
Le paquet conserve tes fichiers personnels `nyx.ini` et `hekate_ipl.ini`.
Nyx est dans `bootloader/sys/nyx.bin` : remplacer seulement le payload ne suffit pas.

## Activation automatique : ON / OFF

Dans **Réglages Nyx → TV / Dock**, active **Détection automatique du dock**,
puis utilise **Enregistrer** pour garder ton choix après redémarrage.

- **ON** : lorsque Nyx démarre déjà sur le dock, ou lorsque tu poses ensuite
  la console dessus, la connexion TV démarre automatiquement après une courte
  vérification de présence. La TV doit être allumée et sur la bonne entrée HDMI.
- **OFF** : aucune détection automatique en arrière-plan. Le bouton **Activer
  la TV** reste disponible. Désactiver l'automatisme ne coupe pas une connexion
  TV déjà lancée : utilise **Arrêter la TV** ou **VOL-** pour cela.

La nouvelle option est désactivée par défaut afin de conserver le comportement
des configurations précédentes. Une fois enregistrée, elle est relue à chaque
démarrage de Nyx. Elle ne modifie pas les fonctions TV d'un système déjà lancé.

La détection attend environ deux secondes après l'initialisation de l'interface,
puis confirme la présence du dock sur deux lectures espacées. La négociation
et l'initialisation de la vidéo prennent ensuite leur propre temps.

## Écran de la console

Le réglage **Affichage TV** est également dans le même onglet :

- **TV + console** conserve les deux écrans ; c'est le réglage initial.
- **TV seule** éteint l'écran de la console après l'initialisation réussie de la TV.
  Il se rallume lorsque la sortie TV est arrêtée ou débranchée.

Utilise **Enregistrer** pour conserver ce choix. La luminosité réglée dans Nyx
est conservée pour le retour à l'écran de la console. Elle n'est pas écrasée
par le passage sur TV.

## Arrêt, retrait et nouvelles tentatives

**VOL-** ou **Arrêter la TV** coupe la sortie. L'automatisme ne la rallume pas
immédiatement : retire la console puis remets-la sur le dock pour réarmer la
détection, ou utilise **Activer la TV** manuellement.

Une tentative automatique qui échoue n'est pas répétée en boucle. Si la TV ou
le HDMI n'était pas prêt, vérifie le branchement puis relance manuellement ou
retire/repose la console. Une erreur de lecture du contrôleur n'est pas
interprétée comme une déconnexion.

Le détecteur reconnaît l'offre d'alimentation habituelle des docks Nintendo,
puis le pilote effectue la négociation complète. Les autres chargeurs/adaptateurs
ne constituent pas le branchement de référence ; l'activation manuelle reste
disponible. La Switch Lite n'est pas prise en charge pour la sortie TV.

Les diagnostics matériels et les fonctions USB exclusives suspendent le service
TV. Leur retour rétablit la surveillance automatique ; une sortie TV qui était
active peut être réactivée. L'automatisme respecte un arrêt manuel précédent.
Un lancement de système, de payload, un arrêt ou un redémarrage termine le service.

## Rapports et validation

Les rapports sont dans `bootloader/tv-test/last-run.txt` et `run-NNN.txt`.
Pour l'essai sur console, vérifier :

1. ON enregistré, redémarrage de Nyx avec console sur le dock ;
2. démarrage hors du dock puis pose sur le dock ;
3. arrêt avec VOL-, absence de relance immédiate, puis retrait/reconnexion ;
4. retour à l'écran console et conservation des choix après redémarrage.

La compilation et les tests PC vérifient le code, les préférences et les cas
d'erreur. Ils ne remplacent pas ces essais physiques. Le pilote vidéo et ses
réglages sont conservés depuis le prototype01 testé en insertion directe.
La sortie HDMI audio et les câbles intermédiaires ne sont pas validés ici.

## Joy-Con sans fil

Ils ne sont pas activés dans cette version. Le code actuel teste la radio,
mais il manque le service Bluetooth de connexion et de commandes des manettes.
Voir `documentation/ETUDE-JOYCONS-SANS-FIL.md` pour l'analyse et les références.
Les Joy-Con sur rails conservent leur prise en charge existante.

Les archives Nova19-TV et du prototype01 sont conservées pour revenir à la
version précédente. Voir `documentation/TV-SOURCES.md` pour la provenance.
