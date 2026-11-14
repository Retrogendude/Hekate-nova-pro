# Hekate Nova Pro 24 — correction de l’import des clés Joy-Con
**Languages:** **Français** · [English](LIRE-MOI-V24.en.md) · [Español](LIRE-MOI-V24.es.md) · [Русский](LIRE-MOI-V24.ru.md)

Base : Hekate 6.5.5 / Nyx 1.9.5. Interface Nova et réglages conservés.

Le rapport v23 montrait un refus d’authentification 05 et aucun échange ACL.
Une erreur a été identifiée dans Nova : une seconde inversion des octets de
la clé déjà préparée par Hekate. Elle est supprimée dans cette version.
La comparaison avec le chemin d’import de Switchroot est décrite dans
AUTH-SOURCES-V24.md. Le type de clé et la sécurité sont conservés.

La compilation et les tests logiciels passent. Le fonctionnement des commandes
sans fil doit encore être confirmé sur une console.

## Installation et essai

1. Copier le contenu de l’archive à la racine de la SD en remplaçant les fichiers
   du dossier **bootloader**. Les fichiers de réglages ne sont pas fournis.
2. Utiliser le payload **hekate_nova_pro_6.5.5_v24.bin** si nécessaire.
3. Redémarrer complètement la console. Dans Nyx, ouvrir **Réglages → TV / Dock →
   Joy-Con sans fil**.
4. Attacher les deux Joy-Con et choisir **Lire les appairages**.
5. Détacher les Joy-Con, choisir **Activer Bluetooth**, puis appuyer sur une touche.
6. Vérifier que HID et Rapports augmentent, puis essayer le stick et A / ZL / ZR.
7. Choisir **Arrêter Bluetooth** pour enregistrer le nouveau rapport :
   `bootloader/bluetooth/last-session.txt`.

Il faut relire les appairages après le redémarrage : les clés restent uniquement
en mémoire. Il n’est pas nécessaire de supprimer les réglages ni les appairages
existants. Le service ne crée pas de nouvel appairage.

Le rapport ajoute **HOST=1** lorsque l’identité Bluetooth relue correspond à
celle attendue. **HOST=2 / erreur 9** signale une identité différente et interrompt
la connexion. **AUTH=05** indique un refus d’authentification ; **AUTH=00** n’est
une réussite que si un événement d’authentification a effectivement eu lieu
(visible dans la trace). HID et Rapports restent les indicateurs des commandes.
Les clés, adresses et contenus bruts des paquets ne sont jamais journalisés.

Les correctifs précédents de l’en-tête Nyx, du dock et des diagnostics sont
conservés. BTstack garde sa licence personnelle/non commerciale distincte,
incluse dans la documentation du paquet.
