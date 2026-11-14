# Version courante — Hekate Nova Pro 11 (6 novembre 2026)

Les douze modifications et leur fonctionnement sont décrits dans [NOVA-PRO-V11.md](NOVA-PRO-V11.md). La compilation et l'ensemble des vérifications v11 sont validés. Binaire Hekate : 110036 octets ; Nyx : 1924293 octets ; service RAM : 77052 octets. Le contenu compressé du payload final correspond au nouveau Hekate décompressé ; les dépendances du chargeur sont corrigées pour les compilations incrémentales.

Les contrôles comprennent 4 260 scènes LVGL (polices, langues, dimensions, disponibilités matérielles, palettes et transparences), les réglages, les erreurs SD, le chargement d'images, l'animation de démarrage et les contrats du service RAM. Les sondes matérielles ne sont pas exécutées sur la console pendant ces contrôles PC.

Les animations de fond ont été retirées. Les mesures du reflet concernent des surfaces redessinées par LVGL sur ordinateur, sans valeur de FPS Switch. Essais physiques de cette version encore à effectuer.

---
## Historique — versions précédentes

# Nova — prototype 10, 6 novembre 2026

La présentation Nova est implémentée dans les composants LVGL natifs :
navigation haute, cartes pleines, bandeau de détail, interrupteurs avec état
écrit et icônes Lucide 1.8.0 identiques aux chemins SVG de la maquette.
Les 32 cartes reprennent leurs textes français, avec un catalogue de sept
langues. La palette, le rayon 8 pixels et le halo fixe de 18 pixels viennent
de cette référence. Le contenu occupe 1 280 × 580 pixels, sous un en-tête
de 76 pixels et au-dessus de la barre d'état à 656 pixels.

Les masques alpha des icônes sont précalculés en trois tailles ; leur décodage
LVGL est activé. Les rôles typographiques sont mis en cache depuis les fichiers
NYXF ou la police intégrée. Les callbacks de cartes conservent leurs identifiants
et leurs gardes ; la sélection est distincte de l'ouverture de l'action.
Les options de démarrage, les commandes rapides et l'interface Apparence
restent accessibles depuis les titres de Réglages et le logo HEKATE.

Les contrôles natifs sur ordinateur couvrent 1 890 scènes et douze cas
supplémentaires de disponibilité. Ils vérifient aussi les pixels des glyphes
Lucide, les états des interrupteurs, l'annulation des choix et les erreurs
d'enregistrement. Le halo invalide au plus sa zone locale et n'entretient
aucun rafraîchissement au repos. Ce résultat ne mesure pas les FPS matériels.
Voir verification/reference-nova.json et verification/performance-halo-nova.json.

Les notes suivantes décrivent les versions précédentes. BUILD-INFO.json
donne les tailles, empreintes et résultats de la construction courante.

# Studio — prototype 9 (historique)

La version actuelle ajoute une navigation latérale native de 224 pixels,
un en-tête de 64 pixels et une zone principale de 1 056 × 592 pixels.
Les indices d'onglets et les actions existantes sont conservés. Le thème
anthracite/violet et les angles de 6 pixels sont partagés avec les fenêtres
détaillées. Les icônes sont celles des ressources officielles du projet.
Le halo de 28 pixels utilise une ombre locale sur l'icône sélectionnée,
sans tâche d'animation ni allocation par image. Les interrupteurs lisent
l'état réel du bouton, y compris après fermeture du choix Auto Boot.

Le rendu LVGL sur ordinateur vérifie la navigation, les interrupteurs,
les dimensions dans les sept langues et les trois polices fournies.
Le contrôle du halo borne le dessin lors d'un changement et vérifie
l'absence de rafraîchissement au repos, effets de fond mis en pause.
Les mesures courantes figurent dans verification/performance-halo-studio.json.
Les contrôles matériels et le service RAM n'ont pas été modifiés.

Les notes du prototype 8 ci-dessous décrivent les optimisations conservées
et la présentation précédente. BUILD-INFO.json décrit la construction actuelle.

# Correctifs du prototype 8 (historique)

Les onglets principaux sont compacts et bornés par les barres haute/basse.
Le thème emploie un fond uni et des contours gris/blancs de rayon 8 pixels.
Les couleurs des pastilles sont rendues explicitement ; leurs styles sont
réutilisés plutôt que réalloués à chaque ouverture.

Le pilote attend la fin de l'image précédente avant la première écriture,
puis lance une seule rotation VIC à la fin du rafraîchissement LVGL.
Les polices Unicode triées utilisent une recherche binaire ; les faces SD
ont un accès ASCII direct. La recherche des motifs de traduction est indexée
et la recherche de fragments ne crée plus de copie temporaire de clé.
Les offsets d'animation et textes dynamiques identiques ne redessinent plus
leurs labels sans changement. Aucune fréquence matérielle n'a été augmentée.

Les effets n'utilisent plus vingt objets mobiles. Leur travail de dessin
est borné à une bande de 64 pixels ou à seize petites zones, et se suspend
pendant les gestes. Les mesures de `verification/performance-fonds.json`
sont celles du rendu logiciel LVGL sur ordinateur, et non des FPS de console.
Les contrôles d'affichage vérifient les présentations groupées, les contrôles
visibles et les pastilles. Les essais physiques de cette version restent à
faire. Les confirmations et les opérations matérielles sont conservées.

# Construction Windows — prototype personnalisé

Prototype 9 construit le 5 novembre 2026 à partir des Makefiles du projet.
Les notes des prototypes antérieurs ci-dessous décrivent leur état historique.
Les tailles courantes et résultats détaillés sont dans BUILD-INFO.json.

- Cible ARM : devkitARM GCC 16.1.0 installé par devkitPro.
- Outils Windows : GCC 16.2.0, GNU Make 4.4.1 et shell BusyBox de w64devkit 2.11.0.
- L'archive w64devkit a été obtenue depuis le dépôt officiel skeeto/w64devkit et
  son SHA-256 a été vérifié contre les métadonnées de publication GitHub.
- MSYS2 n'a pas pu créer son canal de signaux dans l'environnement d'exécution.
  Le build a utilisé les outils Windows portables et une copie locale de
  `base_tools` sans la réécriture de PATH spécifique à MSYS2. Les Makefiles,
  options de compilation ARM et scripts de liaison du projet sont conservés.
- Aucun logiciel supplémentaire n'a été installé dans devkitPro par le build.

Résultats : Hekate non compressé 134 529 octets (limite 140 288), payload final
110 204 octets (limite 126 296). Nyx comprend désormais le moteur de polices SD,
les effets et le catalogue étendu. Les tailles courantes de Nyx et du service
RAM sont consignées dans BUILD-INFO.json (service : limite 130 392). Les limites sont vérifiées
sur les fichiers finaux, y compris le suffixe de six octets du payload.

Les sorties ARM sont des ELF32 ARM EABI5, avec points d'entrée 0x40007000
(chargeur) et 0x81000000 (Nyx). Les fichiers binaires du chargeur et de Nyx
ont été vérifiés contre les ELF liés ; le payload comporte le suffixe ICTC49.

Le compilateur a émis des avertissements non bloquants pour des variables
inutilisées de LVGL, une ancienne fonction de boutons devenue inutilisée et
les segments de liaison RWX des programmes embarqués. GNU Make signale aussi
la réinitialisation du parallélisme dans les sous-constructions.

La sauvegarde des réglages a passé 96 cas C sur les fonctions réelles extraites
des sources, avec des doublures de la carte SD : modes 0/1, valeurs invalides,
conservation des anciens réglages et échecs de montage, ouverture, écriture et
fermeture. Cela ne remplace pas les essais matériels.

Le prototype 7 avait ajouté des contrôles sur les polices SD, les actions du menu
Apparence, les effets et les dispositions. Des rendus ont été produits avec
le vrai LVGL sur ordinateur pour les cartes et l'écran Apparence, à 36 pixels
en français et en russe. Les sélecteurs ont été corrigés après inspection.
Les ressources du test RAM sont validées avec son vrai moteur et son dessin
de framebuffer ; elles restent dans les premiers 4 Mio déjà exclus du test.
Voir PERSONNALISATION.md et TRADUCTIONS.md pour le périmètre actuel.

Le catalogue final comporte 3 688 clés. Les compléments des rapports des
sondes ont été traduits hors ligne avec des modèles Argos/CTranslate2 ;
les menus rédigés et 201 entrées revues gardent la priorité. Les contrôles
automatiques vérifient les 1 268 chaînes source des sondes, l'ordre des
paramètres, les notations numériques et les chemins. La qualité linguistique
de tous les commentaires automatiques n'a pas été validée par des locuteurs
natifs. Les modèles de traduction ne font pas partie du paquet SD.

Le paquet contient les ressources précompilées inchangées de la publication
officielle Hekate 6.5.4 / Nyx 1.9.4, ainsi que les binaires personnalisés
nouvellement construits. Leur provenance et leurs empreintes figurent dans
BUILD-INFO.json à côté des instructions du paquet binaire.

Pour reconstruire dans un environnement devkitPro/MSYS2 fonctionnel avec GCC
natif disponible, la commande standard reste `make -j4` à la racine du projet.

## Intégration complète du prototype 5

Le programme AArch64 Wi-Fi de 8 128 octets est inclus et requis à la compilation.
La table native contient les 42 entrées HWTest-RCM. Le service RAM utilise la
révision exacte d'Atmosphère indiquée dans DEPENDENCIES.json et conserve une
image de retour Hekate dans la zone déjà exclue des premiers 4 Mio de DRAM.

Vérifications complémentaires sur le code réel : 108 assertions du lecteur
borné de payloads ; capture bornée et dix cas d'export ; six cas de synthèse
avec clés manquantes, succès partiel, avertissements et défauts ; 28 cas du
contrat RAM/retour. Le décodeur C du chargeur reconstitue tous les octets de
l'image RAM décompressée. Aucune de ces vérifications ne simule les mesures
physiques des périphériques de la console.

Les sources du compagnon RAM et ses dépendances nécessaires sont fournies
avec les sources de Hekate. Sur un système disposant de devkitARM, devkitA64,
Python 3, Git, GCC natif et Make, construire Hekate à sa racine, puis le service
RAM dans HWTest-RCM-0.15/ramtest. Les scripts Windows sous build-tools sont des
références pour l'environnement portable utilisé dans cette session.

## Interface et langues du prototype 6

Fonds transparents sur les composants de navigation ; icône Diagnostic dans
l'onglet et sur l'accueil. Sept langues, accents et cyrillique : les nouvelles
polices sont des bitmaps 4 bpp de 707 caractères aux hauteurs 20 et 30 pixels.
La police Noto Sans et sa licence SIL OFL sont fournies avec leur générateur.
Le passage en UTF-8 conserve les symboles Hekate et corrige les découpes de
texte utilisant auparavant les offsets d'octets comme nombres de caractères.

La sauvegarde/restauration eMMC utilise des identifiants de boutons stables,
indépendants des libellés traduits. Les matrices de boutons conservent leurs
octets de contrôle et libèrent leurs traductions à la fermeture. Les noms
utilisateur, chemins et rapports techniques ne sont pas traduits. Le service
RAM séparé garde sa propre interface en anglais.

Les contrôles complémentaires exécutent le code C de configuration, des
actions du menu Langue et du moteur de traduction, avec erreurs d'allocation
et nettoyage des menus. Les polices sont contrôlées avec le vrai lecteur
LVGL ; leur aperçu décodé a été inspecté sur ordinateur. Ces vérifications
ne remplacent pas la validation visuelle et matérielle sur une Switch.
