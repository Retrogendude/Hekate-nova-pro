# Construction Windows — prototype personnalisé

Compilation complète réussie le 4 novembre 2026 à partir des Makefiles du projet.

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
110 204 octets (limite 126 296), Nyx environ 1 Mo avec les polices Unicode et
le catalogue de langues (taille exacte dans BUILD-INFO.json). Le service RAM séparé fait 75 548 octets (limite 130 392) et son image
décompressée 174 092 octets. Les limites sont vérifiées
sur les fichiers finaux, y compris le suffixe de six octets du payload.

Les sorties ARM sont des ELF32 ARM EABI5, avec points d'entrée 0x40007000
(chargeur) et 0x81000000 (Nyx). Les fichiers binaires du chargeur et de Nyx
ont été vérifiés contre les ELF liés ; le payload comporte le suffixe ICTC49.

Le compilateur a émis des avertissements non bloquants pour des variables
inutilisées de LVGL, une ancienne fonction de boutons devenue inutilisée et
les segments de liaison RWX des programmes embarqués. GNU Make signale aussi
la réinitialisation du parallélisme dans les sous-constructions.

La sauvegarde des réglages a passé 76 cas C sur les fonctions réelles extraites
des sources, avec des doublures de la carte SD : modes 0/1, valeurs invalides,
conservation des anciens réglages et échecs de montage, ouverture, écriture et
fermeture. Cela ne remplace pas les essais matériels.

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
