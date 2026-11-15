# Audit Nova Pro v30 — cœur, démarrage, stockage et diagnostics
**Languages:** **Français** · [English](AUDIT-NOVA30-CORE.en.md) · [Español](AUDIT-NOVA30-CORE.es.md) · [Русский](AUDIT-NOVA30-CORE.ru.md)

Audit statique en lecture seule du 13 novembre 2026, sur `work/hekate-nova-pro-v30`. Aucun fichier source modifié. Les références de lignes décrivent la version lue pendant l’audit ; le parent travaille simultanément sur le démarrage. GUI générale et Bluetooth font l’objet d’audits distincts.

Les résultats matériels v29 annoncés concluants sont conservés comme information fournie, sans prétendre les reproduire. Aucun gain de FPS, de débit SD/USB ou de temps matériel n’a été mesuré ici. Les durées ci-dessous viennent des délais et boucles explicites du code.

## Priorités

| Priorité | Sujet | Effet démontrable | Portée conseillée |
|---|---|---|---|
| P1 | Tailles des fichiers binaires et `res.pak` | Accès hors fichier ou hors région réservée sur fichier invalide | Valider avant allocation/lecture/exécution |
| P1 | Écriture SD signalée réussie malgré échec | Exports incomplets présentés comme enregistrés | Vérifier compte écrit et fermeture |
| P1 | Limites de taille de build non bloquantes | Un binaire hors capacité peut être livré | Faire échouer le build, sans grossir le loader |
| P2 | Fuite du payload refusé dans Nyx | Consommation répétée du tas, bornage absent | Taille avant allocation, nettoyage unique |
| P2 | 1,1 s d’animation ajoutée au démarrage | Attente logicielle imposée, même travail déjà achevé | Animation sur le travail, conserver apparence et délais matériels |
| P2 | Longs diagnostics sans rafraîchissement périodique | Impression de gel pendant plusieurs secondes | Heartbeat d’affichage uniquement, cadencé |
| P2 | Étapes TV encore bloquantes | Tâche GUI retenue pendant attente/négociation | États temporisés, progression et annulation entre phases sûres |
| P3 | Allocations de traduction après troncature rapport | Travail sans résultat conservé | Éviter la traduction lorsque l’écriture est déjà impossible |

P1 désigne ici une fiabilité prioritaire, pas un incident matériel constaté. Les changements BDK communs ont une surface de régression plus grande que les corrections locales ; les isoler et vérifier chaque appelant.

## 1. Fichiers de démarrage : borner et vérifier avant utilisation — P1

**Preuves.** `bootloader/main.c:585` lit `bootloader/sys/nyx.bin` avec `sd_file_read(..., NULL)`. La lecture du mot de version à `main.c:596` suppose au moins `NYX_VER_OFF + 4` octets (`NYX_VER_OFF = 0x9C`, ligne 580). L’adresse chargée devient ensuite une entrée exécutée, lignes 656–657. `bdk/storage/sd.c:258–272` alloue toute la taille indiquée par le fichier, sans borne métier ; `f_read` ne vérifie pas le compte lu et `f_close` est ignoré.

`nyx/nyx_gui/nyx.c:400–413` lit toute la taille de `bootloader/sys/res.pak` directement à `NYX_RES_ADDR`. La région réservée ne fait que `NYX_RES_SZ = SZ_16M` (`bdk/memory_map.h:82–83`). Une taille excessive écrit au-delà de cette région. Une taille insuffisante peut laisser des ressources absentes ou anciennes sans que cette fonction échoue.

**Correction testable.** Demander la taille de Nyx, rejeter une image trop petite et imposer une borne cohérente avec sa région/format avant lecture et avant saut. Pour `res.pak`, vérifier taille attendue ou au minimum bornes du format, puis compte effectivement lu et retour de fermeture. Ne pas considérer une simple comparaison de version comme validation du format. Utiliser des helpers locaux ou étendre sobrement la lecture bornée déjà présente : `_read_service_payload` dans `nyx.c:168–198` valide la taille avant allocation et vérifie lecture complète et fermeture. Éviter de dupliquer une infrastructure lourde dans le loader proche de sa limite.

**Validation.** Fichiers absents, vides, 1 octet, `0x9F`, taille valide, limite exacte, limite + 1 ; lecture courte avec retour `FR_OK`, erreur de lecture/fermeture ; sentinelles avant/après la région res. Le résultat attendu est un refus explicite avant accès version, saut ou débordement. Vérifier ensuite un démarrage matériel avec les fichiers v29/v30 valides.

**Limite.** Audit du contrôle de taille et du contrat d’entrée, pas vérification cryptographique de Nyx ni du format complet de `res.pak`.

## 2. `sd_save_to_file` retourne un faux succès — P1

**Preuves.** Après ouverture réussie, `bdk/storage/sd.c:291–294` ignore `f_write`, le nombre d’octets écrits et `f_close`, puis retourne zéro. Des appelants utilisent ce zéro comme succès : dumps `nyx/nyx_gui/frontend/gui_info.c:84,108,138`, captures `frontend/gui.c:300`, extraction `frontend/gui_tools.c:1006,1152`, export Joy-Con `frontend/gui_options.c:758`, journal fatal `bootloader/hos/secmon_exo.c:418`.

**Impact.** Carte pleine, retrait de carte ou erreur différée lors de la fermeture : un fichier partiel peut être annoncé enregistré. Il ne s’agit pas d’un gain de vitesse ; cela évite des reprises inutiles et rend les échecs exploitables.

**Correction testable.** Recueillir `UINT written`, vérifier `FRESULT` et `written == size`, toujours fermer un fichier ouvert, propager l’erreur de fermeture lorsque l’écriture a réussi. Choisir un retour d’erreur existant cohérent pour l’écriture partielle. Ne pas modifier implicitement les politiques d’écrasement ou d’effacement des exports.

**Validation.** Substitutions FatFS : écriture complète, partielle + `FR_OK`, erreur de write, erreur de close, ouverture refusée ; chaque chemin ferme exactement une fois si nécessaire et seuls les fichiers complets fermés avec succès retournent zéro. Le diagnostic natif donne déjà un bon exemple local dans `nyx_diagnostics.c:346–357`, qui vérifie compte écrit et fermeture.

## 3. Limites du binaire : les rendre bloquantes — P1

**Preuves.** `Makefile:108` et `112` affichent seulement un avertissement au-delà de respectivement 140288 et 126296 octets. Les commandes réussissent malgré un dépassement. Le parent annonce actuellement 139273 octets bruts : marge déclarée de 1015 octets, valeur non reconstruite indépendamment pendant cet audit.

**Correction testable.** Faire échouer la génération/livraison sur dépassement des deux limites, conserver leurs valeurs existantes, contrôler également la taille des artefacts livrés. Cette garde ne consomme aucun octet du payload. Évaluer la taille après chaque correction BDK compilée dans Hekate ; privilégier les corrections locales au code générique volumineux.

**Validation.** Vérifier limite − 1, limite, limite + 1 : les deux premiers cas passent, le dernier échoue réellement. Refaire une compilation incrémentale et propre. Le loader possède déjà la dépendance explicite de `loader.o` aux deux headers de payload et à `Versions.inc` (`loader/Makefile:67–71`) : la conserver.

**À préserver.** `loader/loader.c:80–105` relocalise puis décompresse deux blocs embarqués de taille de compilation connue. Aucun bénéfice concret ne justifie ici d’ajouter cache, nouveau codec ou rendu supplémentaire à cette étape. Aucun coût de décompression n’a été mesuré.

## 4. Fuite et allocation avant validation d’un payload Nyx — P2

**Preuves.** `nyx/nyx_gui/nyx.c:133–145` charge un payload avec `sd_file_read`, puis refuse une taille supérieure à `0x30000`. Le chemin `out` (`nyx.c:164–168`) démonte la carte mais ne libère pas `buf`. La version bootloader possède, elle, `free(buf)` à `bootloader/main.c:213`. Un payload surdimensionné peut ainsi consommer sa taille à chaque nouvelle tentative dans Nyx.

**Correction testable.** Vérifier la taille du payload avant allocation, conserver un pointeur initialisé à `NULL` dans toute la fonction et un nettoyage unique pour chaque sortie qui revient à Nyx. Une image de zéro octet doit être refusée avant lancement. Tenir compte des sorties avant même que `buf` ait été déclaré dans la fonction actuelle.

**Validation.** Cent refus successifs du même payload > `0x30000` : occupation du tas stable et zéro saut d’exécution ; succès puis retour simulé ; chemin sans nom de fichier ; fichier absent et lecture échouée. Le compteur doit distinguer mémoire vivante et mémoire seulement réservée par le tas.

**Contexte du tas.** `bdk/mem/heap.c:39–112` recherche linéairement un bloc libre puis étend le tas sans borne maximale ; `calloc` multiplie sans garde (`heap.c:179–181`). La carte mémoire réserve pourtant 256 MiB (`memory_map.h:52–53`). Les tests `if (!malloc(...))` ne fournissent donc pas un contrôle fiable d’épuisement. Prioriser les tailles d’entrées et la fuite locale. Un bornage global du tas demanderait un changement de contrat et l’audit des appels `zalloc`/`calloc`, qui font `memset` sans vérifier un retour nul (`heap.c:172–181`) : ce n’est pas une modification isolée à effectuer à la hâte.

## 5. Démarrage : recouvrir l’animation avec les travaux — P2

**Preuves.** `nova_splash_animate` (`bdk/display/nova_splash.c:46–57`) effectue toutes ses étapes et attentes même lorsque `from == to`. Il peut précisément reprendre `from` au progrès déjà atteint, ligne 48, tout en conservant la durée totale. Le chemin normal ajoute des animations de 300 + 200 ms dans `bootloader/main.c:590,655` puis 300 + 300 ms dans `nyx/nyx_gui/nyx.c:575,619`, soit 1100 ms de durée demandée, hors coût du rendu et granularité des temporisations.

**Correction testable.** Conserver le splash, ses couleurs et ses étapes mais l’animer pendant les lectures/initialisations. Terminer immédiatement ou avec un bref rattrapage borné lorsque le travail de l’étape est réellement terminé. Le mécanisme de plafond existant (`nova_splash.c:29–31`) évite d’annoncer un travail achevé trop tôt : le préserver. Les nouvelles phases de chargement `.bin` du parent doivent utiliser la même logique.

**Validation.** Mesurer séparément lecture Nyx, configuration, ressources et première image interactive ; carte rapide et lente ; splash personnalisé ; entrée Nyx après retour RAM. Une substitution du timer peut vérifier qu’une étape déjà achevée n’attend plus 300 ms. Les 1100 ms sont un budget logiciel identifiable, pas un gain mural garanti de 1100 ms sur console.

**À préserver impérativement.** L’attente de cycle d’alimentation SD (`main.c:652–653`), la rampe rétroéclairage, le training Minerva et `nova_splash_pause()` avant le training (`main.c:645–647`). Ne pas accélérer les délais électriques pour gagner du temps apparent. Les hooks de timer exécutent du rendu depuis `get_tmr_ms/us` (`bdk/soc/timer.c:39–51`) ; ne pas les étendre sans borne ni les réactiver pendant des mesures sensibles.

## 6. Diagnostics : progression régulière sans perturber les mesures — P2

**Preuves.** L’adaptateur `nyx_diagnostics.c:132` définit `msleep_poll(ms)` comme un simple `msleep(ms)`, contrairement au sens apparent du nom. Le callback d’affichage n’est appelé que par `status_set` / `status_step` (`nyx_diagnostics.c:133–143`) et à l’entrée de chaque sonde (`:261`). Le callback GUI rafraîchit immédiatement (`frontend/gui_diagnostics.c:261–269`) ; il n’y a pas de heartbeat pendant les attentes.

Exemples : ventilateur 600 ms de kick, 2000 ms de spin-up puis la mesure BDK pouvant attendre 2 s (`diagnostics/vendor/probe_power.c:806,813,974–993`, `bdk/thermal/fan.c:86–96`) et mesure propre de 1 s. Le Wi-Fi attend jusqu’à 40000 itérations de 500 µs par tentative, avec jusqu’à trois tentatives (`vendor/probe_wifi.c:501–541`). Les branches de sortie peuvent raccourcir ces plafonds ; il ne faut pas annoncer systématiquement 60 s.

**Correction testable.** Ajouter un rafraîchissement d’état cadencé à faible fréquence aux longues attentes et aux limites sûres des phases, avec nom de phase/temps écoulé. Respecter les périodes d’échantillonnage sans rendu ; ne pas raccourcir les fenêtres de mesure pour embellir la durée. `manual_system_maintenance` sait déjà se limiter au rafraîchissement lorsque `diagnostics_suspended` est vrai (`frontend/gui.c:784–791`) : un point d’intégration possible à examiner sans importer le frontend dans le code vendor.

**Risques.** Ne pas lancer `lv_task_handler()` ou réactiver sans distinction tactile, rails, Bluetooth, SD ou tâches matérielles depuis une sonde. Les diagnostics possèdent snapshot/restauration et suspension des entrées ; toute annulation future doit traverser le même nettoyage. Le texte utilisateur annonce actuellement explicitement les commandes suspendues (`gui_diagnostics.c:284`) : l’absence d’interactivité n’est donc pas décrite comme un bouton Annuler cassé.

**Validation.** Timer simulé : nombre de rafraîchissements borné, aucun accès aux périphériques suspendus, même durée/ordre des mesures et même verdict ; nettoyage/restauration exécuté une fois après succès et erreur. Puis comparaison des résultats matériels avec la référence v29.

## 7. TV : conserver les délais, rendre les étapes coopératives — P2

**Preuves.** La négociation est majoritairement une machine à états, mais `bdk/display/tv/usbpd.c:957–958` attend encore synchroniquement la fin d’un délai de 100 ms, et `:1032` attend 250 ms pour `ND_BUSY`. La tâche GUI appelle `dock_step` puis, lorsque le dock est prêt, `dpout_aux_init`, `dpout_probe_sink` et `dpout_video_start` synchroniquement (`frontend/gui_tv.c:329,354–365`). Les boucles de training DisplayPort, replis de débit et essais de marge possèdent plusieurs attentes (`bdk/display/tv/dpout.c:1817–1860`). La limite de session de 45 s n’est évaluée qu’à l’entrée de `_tick` (`gui_tv.c:321`), pas pendant ces appels.

**Correction testable.** Transformer les attentes de 100/250 ms en états avec échéance, sans changer leur durée. Capturer une seule valeur `elapsed` au lieu de recalculer entre condition et soustraction à `usbpd.c:957–958`. Découper lecture sink, training et marge en phases bornées seulement si des pauses sûres sont identifiées ; afficher l’étape et vérifier débranchement/annulation entre ces phases. Commencer par les états PD simples, moins risqués que le séquencement SOR/PLL.

**Validation.** Horloge simulée : même ordre de commandes, pas d’appel avant son délai, `ND_BUSY` jusqu’à sa limite, retrait HPD, retour en état propre, restauration horloge et rails. La stabilité vidéo, la négociation de docks variés et les timings PLL restent des tests console. Ne pas proposer de gain FPS TV : sortie configurée 720p/60 Hz et cadence effective de l’interface sont des mesures différentes.

## 8. Rapport diagnostic : éviter le travail après troncature — P3

**Preuves.** `append` abandonne si le rapport est muet ou tronqué (`nyx_diagnostics.c:77–90`). Pourtant `_log_emit` traduit et alloue toujours la ligne avant de l’appeler (`:98–101`). Après saturation du rapport de 256 KiB (`:29`), des sondes bavardes continuent ainsi les recherches de traduction et allocations libérées sans texte ajouté. La traduction peut construire récursivement une nouvelle chaîne (`localization.c:235–251`).

**Correction testable.** Conserver en premier l’enregistrement de gravité dans `_log_emit`, puis court-circuiter uniquement la traduction/append quand aucune sortie n’est consommable (`muted`, `truncated` ou rapport absent selon contrat). Ne pas sauter les découvertes/verdicts. Éviter un cache de toutes les lignes : les valeurs variables auraient peu de réutilisation et augmenteraient le tas.

**Validation.** Rapport saturé : même marqueur de troncature, mêmes résultats et compteur de findings, zéro traduction supplémentaire attendue après troncature ; cas non tronqué identique dans les sept langues. Gain CPU non chiffré avant mesure.

## USB, SD et mémoire : décisions conservatrices

Le service UMS possède déjà entretien périodique et training (`bdk/usb/usb_gadget_ums.c:1794–1810,1914`), sélection de contrôleur Erista/Mariko et buffers fixes. Aucun élément lu ne justifie de grossir les transferts ou d’augmenter les fréquences arbitrairement. Les écritures UMS et caches demandent de vrais tests hôte/carte ; les substitutions PC ne démontrent ni débit ni intégrité après retrait brutal.

Les dates UMS utilisent des comparaisons absolues `timer < time` (`:1801,1806`) plutôt qu’un calcul de temps écoulé. Une longue session traversant le débordement du compteur millisecondes peut retarder l’entretien. C’est une amélioration de robustesse secondaire : échéances sûres au rollover et tests près de `UINT32_MAX`, sans raccourcir les intervalles existants.

Le SD conserve ses replis de vitesse et retries (`bdk/storage/sd.c:93–179`) ; retirer ces replis ou augmenter le mode maximal au nom des performances compromettrait les cartes marginales. Une optimisation utile doit séparer temps d’initialisation, débit séquentiel, petits fichiers et temps de rendu ; un débit de lecture global ne prouve pas une amélioration du démarrage.

## Validation et limites de cet audit

Lecture des sources et de la chaîne de compilation uniquement ; aucun test matériel, benchmark, compilation ou exécution des substituts v29 réalisé par cet audit. Les scripts `work/check_nova29_*.py` inspectés concernent la référence v29 et plusieurs ciblent explicitement le répertoire `hekate-nova-pro-v29`. Les lancer inchangés ne validerait pas les modifications v30. Pour toute correction future, cibler explicitement le bon arbre et consigner les hashes de sources testés.

Ordre recommandé : gardes de build ; refus des fichiers invalides et contrat d’écriture SD ; fuite locale ; recouvrement des animations ; heartbeat diagnostics ; états TV simples. Réserver l’évolution globale du tas et le découpage du training vidéo à des changements indépendants. Pour chaque étape, conserver fonctionnalités Nova, ressources personnalisées, langues, comportement de retour RAM et résultats matériels v29.

Validation post-audit : corrections locales de nyx.c intégrées et check_nova30_core_io.py exécuté par le parent ; 20cas v30 passent et 8régressions v29 réelles sont reproduites. Pas de modification des helpers BDK communs ni du loader dans cette version.
