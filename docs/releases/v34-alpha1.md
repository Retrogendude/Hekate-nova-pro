# Hekate Nova Pro v34-alpha1 — préversion

Première ébauche de l’assistant : présentation, modèle/OS et plan de partitionnement. La copie emuMMC automatique n’est pas encore raccordée.

**Version historique.** Les versions suivantes sont disponibles dans [les releases](https://github.com/Retrogendude/Hekate-nova-pro/releases). Pour les derniers travaux TV, consulter [v34-alpha7](https://github.com/Retrogendude/Hekate-nova-pro/releases/tag/v34-alpha7).

L’assistant peut lancer un partitionnement destructif après confirmation : sauvegarder les données SD. Linux et Android ne sont pas installés automatiquement ; seules leurs partitions sont réservées. Le pack 8GB est exclusivement destiné aux consoles physiquement modifiées et reste expérimental.

Les notes d’origine sont conservées : leurs mentions « non publié » décrivent l’état au moment du développement, avant cette publication.

## Modifications


Premier assistant sur l’accueil : présentation, modèle et choix indépendants emuMMC/Linux/Android, récapitulatif et passage protégé vers le partitionneur. Tailles automatiques avec validation de capacité et de modèle. Copie emuMMC automatique et installation des OS non raccordées dans cette alpha.

PC checks passed; physical console validation pending.


## Téléchargements et utilisation

- Pack standard : **hekate-nova-pro-v34-alpha1.zip**.
- Sources complètes, dépendances et outils : **hekate-nova-pro-v34-alpha1-sources.zip**.
- Pack 8GB séparé lorsqu’il existe ; réservé au matériel modifié.
- Changelogs en huit langues et empreintes SHA-256 joints.

Installer ensemble le dossier bootloader et le payload de cette version. Conserver les configurations et appairages personnels. Les tests PC ne constituent pas une validation matérielle ; la stabilité TV de l’alpha7 reste à confirmer.

[Notice d’utilisation](https://github.com/Retrogendude/Hekate-nova-pro/blob/v34-alpha1/docs/NOTICE-UTILISATION.md) · [Sources parcourables](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha1/hekate) · [Changelogs](https://github.com/Retrogendude/Hekate-nova-pro/tree/v34-alpha1/docs/changelogs)

<details><summary>Notice livrée avec cette version</summary>

Hekate Nova Pro v34 alpha 1 — premier assistant d’installation

Depuis Accueil, choisir Assistant d’installation. Le bouton existe dans les dispositions tuiles et carrousel.

1. Lire la présentation du processus, puis Suivant.
2. Choisir uniquement le modèle (Switch V1/V2, Lite ou OLED) et les éléments souhaités : emuMMC/emuNAND, Linux, Android. Plusieurs choix sont possibles.
3. Suivant affiche un récapitulatif calculé avec la capacité réelle de la carte SD. Précédent conserve les choix. Sans système choisi, sans carte, avec un modèle incorrect ou une capacité insuffisante, la poursuite est bloquée.

La disposition réserve une emuMMC complète quand elle est sélectionnée : environ 29 Go pour V1/V2/Lite, 58 Go pour OLED. Linux et Android reçoivent automatiquement les tailles existantes recommandées de 32 Go, ou les préréglages de 16 Go si la capacité ne suffit pas. Les réserves techniques et au moins 2 Go de FAT32 sont conservés. Le choix Linux ou Android seul n’impose pas de partition emuMMC.

PORTÉE DE CETTE ÉBAUCHE
L’introduction, les choix et le récapitulatif n’effacent rien. Le bouton Suivant du récapitulatif ouvre cependant le VRAI partitionneur Hekate avec les tailles préparées. Il garde ses confirmations avant effacement. Pour tester seulement l’interface, revenir ou fermer avant de confirmer l’effacement. Sauvegarder les fichiers personnels avant tout partitionnement réel.

La copie automatique de la mémoire interne vers l’emuMMC n’est PAS encore raccordée. Les partitions Linux/Android ne constituent pas une installation de ces systèmes : leurs fichiers d’installation restent nécessaires. Cette alpha ne prétend donc pas achever une installation complète. Le chaînage et la reprise du processus feront l’objet de la prochaine étape après validation du parcours.

INSTALLATION DU PACK
Conserver vos fichiers nyx.ini, hekate_ipl.ini, vos appairages et vos images personnels. Fusionner le dossier bootloader fourni avec celui de la SD et utiliser le payload correspondant. Aucun fichier de configuration personnel n’est livré. Le pack standard est destiné aux consoles normales ; le pack 8GB reste exclusivement destiné aux consoles physiquement modifiées et expérimental, comme la v33.3.

CONTRÔLES
Compilation ARM et en-têtes Hekate/Nyx vérifiés. 456 cas de parcours, accueil et transmission protégée au partitionneur ; 683 scènes de régression ; 2061 assertions du planificateur C. Les huit langues ont des libellés pour l’assistant. Aucun test d’effacement réel ou de copie automatique n’a été exécuté sur console. Les aperçus sont des rendus du code sur PC.

</details>
