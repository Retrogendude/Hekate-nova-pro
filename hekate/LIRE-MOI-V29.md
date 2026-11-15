# Hekate Nova Pro 29 — appairages, sauvegarde SD et reconnexion
**Languages:** **Français** · [English](LIRE-MOI-V29.en.md) · [Español](LIRE-MOI-V29.es.md) · [Русский](LIRE-MOI-V29.ru.md)

Le formatage de la carte SD n'est pas une correction du problème observé.
Les données d'appairage sont lues dans les Joy-Con. La v28 a reçu les deux
manettes sur une OLED, tandis que R a perdu sa connexion sur l'autre.
Le passage par Nintendo a rétabli l'import2/2 ; un nouvel essai a ensuite
reproduit0/2 sur la seconde OLED. La cause exacte sur matériel reste à vérifier.

Cette version accepte les deux profils d'hôte déjà validés par le lecteur BDK :
Nintendo et standard. La v28 exigeait la classification Nintendo même pour un
enregistrement standard reconnu comme valide. Le nouveau chemin conserve le
contrôle de somme, l'entrée active, les clés non nulles, les côtés et la
compatibilité des deux hôtes. Les réponses SPI tronquées ne remplissent plus
le cache. L'export historique Hekate conserve sa classification Nintendo.

Le délai d'une tentative de connexion passe de15.36s à5.12s. Les tentatives
continuent, espacées, tant que le service Bluetooth est activé ; elles ne
s'arrêtent plus au troisième échec. Une seule tentative sortante reste active
à la fois. Une connexion rétablie remet le compteur de reprise à zéro.
Cela améliore la reprise après réveil ou coupure ; le réveil par bouton normal
et l'absence de coupures sur les deux consoles doivent encore être testés.
Le débit radio, le chiffrement, la gestion des canaux et les touches sont conservés.

Deux nouvelles actions figurent dans le menu Joy-Con sans fil :

- **Sauvegarder les appairages** : après une lecture valide, Bluetooth arrêté.
  Un fichier de72octets contient les données nécessaires des deux manettes,
  dans `bootloader/bluetooth/joycon-pairing.bin`. Il ne s'agit pas d'une copie
  complète de la mémoire des Joy-Con. Les clés sont enregistrées en clair :
  conserver ce fichier privé et ne pas l'envoyer avec les rapports.
- **Charger les appairages** : recharge explicitement ce fichier dans la
  session après un redémarrage. Une erreur de format ou de somme le fait
  rejeter, en conservant les clés déjà présentes en mémoire.

La sauvegarde utilise un fichier temporaire puis conserve la version précédente
dans `joycon-pairing.bak`. Le chargement essaie cette copie si le fichier principal
est absent ou endommagé. Le fichier `.bak` contient lui aussi des clés privées.
Une sauvegarde avec une seule manette valide reste une sauvegarde1/2.
« Effacer les clés » efface la session en mémoire, pas les fichiers de sauvegarde.
Les fichiers sont à utiliser avec les manettes associées. Pour comparer les
deux consoles, lire et sauvegarder les appairages correspondant à chaque essai.
Si un nouvel appairage est réalisé dans le système Nintendo, relire puis
sauvegarder les données mises à jour avant de réutiliser la sauvegarde SD.

Installation et contrôle :

1. Extraire le ZIP à la racine de la SD en remplaçant `bootloader`, sans formater.
   Utiliser `hekate_nova_pro_6.5.5_v29.bin` si nécessaire, puis redémarrer.
2. Avec les Joy-Con fixés, lire les appairages. Si2/2, les sauvegarder avant
   d'activer le Bluetooth. Si0/2 persiste, arrêter et récupérer le rapport.
3. Détacher les manettes, activer Bluetooth et essayer d'abord un bouton normal
   de chaque manette pour les réveiller. Tester le déplacement et A/B.
4. Arrêter Bluetooth et récupérer immédiatement `bootloader/bluetooth/last-session.txt`,
   sans effacer les clés ni relire les appairages. Ne pas joindre les fichiers
   d'appairage `.bin` ou `.bak`.
5. Pour vérifier la persistance : redémarrer Nova, charger les appairages depuis
   la SD, puis refaire le test. Aucune lecture des rails n'est nécessaire pour
   ce chargement si la sauvegarde est valide.

Le rapport ajoute `recognized` (profil valide reconnu) et `source` (1rails,
2SD). Lors d'un chargement SD, la classification Nintendo n'est pas renseignée ;
les indicateurs de rails ne représentent pas une nouvelle lecture physique.
Le type de trace20 indique le délai de tentative demandé, en unités Bluetooth.
Aucune clé, adresse ou donnée brute n'est ajoutée au rapport.

Compilation et vérifications sur PC ne remplacent pas la validation matérielle.
La reprise après délai dépassé est testée avec une radio simulée ; elle ne
prouve pas que la cause des coupures de R a disparu. Aucun effacement ni
nouvel appairage n'est envoyé aux Joy-Con. Base : Hekate6.5.5 / Nyx1.9.5.
Les réglages utilisateur ne sont pas inclus dans le ZIP. Licences conservées.
