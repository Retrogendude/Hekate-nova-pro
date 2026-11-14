# Diagnostic matériel — intégration native, prototype 5

Les 42 entrées de la table HWTest-RCM 0.15 sont intégrées dans Nyx : identité,
fusibles et KFUSE ; alimentation, charge, batterie et thermique ; mémoire et
horloges ; SD, eMMC, partitions, GPT, PRODINFO et lecteur de cartouches ;
Bluetooth, Wi-Fi et complexe CPU ; codec, horloges et sorties audio ; écran
et rétroéclairage ; tactile, lumière ambiante, Joy-Con et boutons ; GPIO,
UART, cause de redémarrage et registres PMC ; synthèse et recoupements.

L'onglet Diagnostic propose un bilan, dix catégories et le service RAM isolé.
Chaque catégorie permet une exécution collective ou sonde par sonde. Les
résultats distinguent OK, avertissement, défaut, erreur, observation et non
exécuté. Une absence de mesure ou de prérequis ne devient pas un succès.
Les rapports sont consultables par pages puis exportables sur la carte SD
sous `bootloader/diagnostics/hwtest_XXXXXXXX.txt`, sans relancer les mesures.
La capture est bornée à 256 Kio avec un marqueur explicite en cas de troncature.

## Adaptation à Nyx

Les sondes sont présentes sous `nyx/nyx_gui/diagnostics/vendor/`. Le programme
principal, l'interface et la journalisation sur écran/UART du payload original
ne sont pas utilisés : un adaptateur natif capture le rapport et publie la
progression dans les fenêtres Nyx. Le pilote des commandes et les tâches de
maintenance sont suspendus pendant les mesures puis rétablis. Les données,
états GPIO/pads, ventilateur, audio, PCIe, partition eMMC et emplacements de clés
modifiés sont restaurés lorsque leur accès est disponible. La réactivation du
tactile utilise le chemin sans auto-calibration persistante.

Le Wi-Fi comprend réellement le programme AArch64 CPU0 requis : le build échoue
si ce programme n'est pas disponible. Il réserve A0000000..A003FFFF, en dehors
du tas de Nyx et des images écran. Les messages de diagnostic ne sont jamais
envoyés sur l'UART-B utilisé par le Joy-Con droit.

## Prérequis et interprétation

Certaines mesures dépendent du modèle ou de composants présents. Les lectures
PRODINFO déchiffrées nécessitent `switch/prod.keys` de cette console et des
emplacements SE lisibles/modifiables ; sinon elles sont signalées comme non
réalisées ou avec avertissement, sans écraser les clés verrouillées de Nyx.
Le test sonore joue les sorties successivement et doit être vérifié à l'écoute.
Un verdict du transport audio ne prouve pas que le haut-parleur est audible.

## RAM séparée et retour à Hekate

`bootloader/sys/hwtest_ram.bin` est un programme séparé : Nyx est quitté afin de
ne pas conserver une interface dans la mémoire soumise aux tests. Le service
exécute une seule séquence des sept motifs avancés sur toutes les zones
accessibles et non réservées, puis recharge Hekate. Le retour force le menu et
une nouvelle instance de Nyx affiche le résultat conservé, y compris après
annulation ou échec du test. Les premiers 4 Mio, dont l'image de retour et son
en-tête, restent exclus. Une image de retour corrompue n'est jamais exécutée ;
une panne matérielle ou un blocage du contrôleur peut empêcher tout retour.

La taille des deux images est contrôlée avant allocation, et les lectures
courtes ou échouées sont rejetées. L'image de retour
vient de `bootloader/update.bin`, est reconnue comme Hekate, bornée à 128 Kio et
contrôlée par empreinte avant utilisation. Le service RAM est borné à 130392
octets. Le contrat partagé et sa copie sont vérifiés identiques.

## Validation

Construction ARM Hekate/Nyx, programme AArch64 Wi-Fi et service RAM réussie.
Tests C sur fonctions réelles : sauvegarde de configuration, lectures bornées,
capture/export et verdict partiel, contrat de retour RAM. La décompression du
service est vérifiée par le décodeur C réel et les binaires sont comparés aux
ELF liés. Aucun essai sur console ni validation physique des mesures n'a encore
été effectué.

## Provenance

HWTest-RCM : archive utilisateur 0.15, commit
778e25b66a11c5cdd65c11a69ef8083aff1cd9f7, dépôt Creased/HWTest-RCM.
Hekate : archive utilisateur 6.5.4, commit
45cd81ed08e30a26ba380958c7e20b55c342834b. Atmosphère du service RAM : commit
cb4b882e3b176480ac57a1161a85ff175c3f162c, dépendance exacte de HWTest.
Les commentaires de provenance, la licence Hekate et la licence Atmosphère
sont conservés. Les adaptations sont décrites dans diagnostics/PROVENANCE.md
et dans le compagnon HWTest-RCM, ramtest/NYX-RETURN.md et DEPENDENCIES.json.
