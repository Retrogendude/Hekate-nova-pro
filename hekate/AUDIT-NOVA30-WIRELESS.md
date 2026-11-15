# Audit Nova 30 — entrées sans fil, fluidité et budgets
**Languages:** **Français** · [English](AUDIT-NOVA30-WIRELESS.en.md) · [Español](AUDIT-NOVA30-WIRELESS.es.md) · [Русский](AUDIT-NOVA30-WIRELESS.ru.md)

Audit local du 13 novembre 2026. Les essais physiques de la v29 sont déclarés concluants par l’utilisateur. Les résultats ci-dessous proviennent des sources et de tests natifs avec radio/horloge simulées. **Aucune latence de console ou de liaison RF n’a été mesurée.** L’autoreprise au démarrage est traitée séparément par le parent.

Les chemins de sources sont relatifs à `work/hekate-nova-pro-v30/`.

## Résultats et modifications

### 1. Appuis brefs : défaut logiciel reproductible, correction bornée

Avant correction, chaque rapport remplaçait `pad.buttons`. Si deux rapports appui puis relâchement arrivaient avant la prochaine lecture LVGL, le clic disparaissait. Le compteur `buttons_seen` conservait une preuve de l’appui, sans le rendre à l’entrée de l’interface. La période nominale de lecture LVGL est 33 ms (`bdk/libs/lv_conf.h:115`) ; elle ne garantit pas une lecture toutes les 33 ms si une autre tâche prend du temps.

Correction dans `nyx/nyx_gui/bluetooth/nova_bt_protocol.c:7` : les fronts montants sont conservés dans `pending_buttons`, avec un horodatage indépendant par bouton. Les rapports simples `0x3f` et complets `0x30/0x21` empruntent le même mécanisme (`:78`, `:83`). L’état tenu, les sticks, la batterie et le compteur de rapports restent ceux du dernier rapport.

L’API `nova_bt_take_buttons(side)` (`nova_bt_stack.c:299`) rend les boutons actuellement tenus plus les fronts de moins de 100 ms, puis consomme les fronts. La lecture suivante voit donc le relâchement si le dernier rapport est relâché. Chaque bouton expire séparément : un front récent ne rajeunit pas un front ancien. Les âges utilisent la soustraction non signée pour traverser le bouclage de l’horloge. Les fronts sont supprimés par `clear_pad` (`:156`), donc par déconnexion HCI, fermeture HID, stale à 500 ms et Stop. Une lecture stale les consomme aussi sans les rendre.

Intégration requise côté parent : dans chaque itération des deux côtés de `gui_bluetooth_merge`, appeler `uint32_t buttons=nova_bt_take_buttons(i);` avant le contrôle de fraîcheur/priorité rail, puis utiliser `buttons` à la place de `p->buttons` lors de la fusion. Le côté rail doit lui aussi consommer les fronts sans les appliquer, afin de ne pas rejouer un appui lors d’un changement de source. Un seul appel par côté et par lecture de l’interface. Le présent sous-travail ne modifie pas `gui_bluetooth.c` ni `gui.c`.

Coût : 100 octets fixes par Joy-Con (masque et 24 dates), sans allocation. Aucun changement aux clés, au profil BR/sniff/QoS, au page timeout, à la sérialisation des connexions ou aux retries.

Limites : plusieurs appuis du même bouton entre deux lectures deviennent un seul front ; cette correction n’est pas une file de clics. Un rapport resté en attente dans le contrôleur est daté à sa réception par l’hôte : l’ancienneté physique de l’appui ne peut pas être déduite de cette date.

### 2. Budget RX et coût total : rendre l’observation correcte

Le service demeure coopératif. La boucle appelle le polling normal avant `lv_task_handler` (`frontend/gui.c`, boucle GUI), tandis que chaque rectangle d’affichage appelle la variante rapide (`frontend/gui.c:334`). La tâche Bluetooth haute priorité est créée avec une période de 2 ms et ne rafraîchit les statistiques que toutes les 500 ms (`frontend/gui_bluetooth.c`, `tick` et `start`). La priorité ne préempte pas une tâche déjà en cours : `bdk/libs/lvgl/lv_misc/lv_task.c:80` organise des appels coopératifs.

Budgets conservés : `nova_bt_poll()` = RX jusqu’à 8 000 µs, silence de 2 000 µs après réception ; `nova_bt_poll_fast()` = RX jusqu’à 250 µs sans silence (`nova_bt_stack.c:422`). Une ligne vide rend la main immédiatement. Ces nombres bornent une tranche RX, pas le temps radio ni le délai bouton-écran. Un callback de paquet ne peut pas être interrompu en cours de traitement.

Avant correction, le test du budget utilisait une date prise avant `input_byte`, donc avant les callbacks du paquet courant. La même lecture d’horloge est maintenant prise après le traitement (`nova_bt_stack.c:363`). Cela fait compter le callback dans le budget sans ajouter de lecture d’horloge par octet. `rx_budget_hits` compte les tranches qui atteignent leur plafond (`:371`).

L’ancien `rx_max_us` reste un maximum de phase RX. Il ne comprenait pas le TX, les callbacks de boucle, les timers, le profil, les connexions et la fermeture en erreur. Les nouveaux champs (`nova_bt.h:32`) mesurent :

| Champ | Sens |
| --- | --- |
| `poll_max_us` | Maximum d’un appel complet, fermeture en erreur comprise |
| `poll_fast_max_us` | Maximum complet de la variante rapide |
| `poll_normal_max_us` | Maximum complet de la variante normale |
| `poll_gap_max_us` | Plus grand intervalle entre débuts de deux appels au service |
| `rx_budget_hits` | Nombre de tranches RX atteignant leur plafond |

L’instrumentation ajoute une lecture d’horloge par appel, après toutes les phases (`nova_bt_stack.c:415`), et aucune attente. Elle ne prétend pas fournir un percentile ni un intervalle entre lectures LVGL. Il faut ajouter ces mesures UI au besoin avant d’attribuer les à-coups au radio. Les compteurs doivent être exposés dans le rapport SD par le parent.

### 3. Débit UART et RTS : marge limitée, réglages physiques à conserver

Le radio utilise 115 200 bauds, 8 bits et un stop (`nova_bt_radio.c:67`), le contrôle de flux matériel RTS/CTS (`:80`) et le seuil RX 8 octets repris du profil HWTest (`:18`). La voie Nova lit directement le FIFO et rend immédiatement `-1` s’il est vide (`:101`) ; elle n’utilise pas le `uart_recv` générique du BDK, qui attend jusqu’à 250 µs après chaque octet (`bdk/soc/uart.c:98`). Remplacer cette voie par le helper BDK réintroduirait une attente inadaptée au redessin.

À 115 200 bauds en 8N1, la capacité théorique est 11 520 octets/s. Le temps filaire d’une trame de N octets est `N × 10 / 115200` secondes : 59 octets occupent 5,12 ms ; 63 octets occupent 5,47 ms. Ces tailles illustrent le calcul, elles ne constituent pas une mesure des rapports de la console. À titre de scénario, 110 trames/s de 59 octets utilisent environ 56 % du débit brut, avant événements HCI, signaux et pauses RTS. Le flux peut donc rapprocher plusieurs rapports et occuper la tranche RX de 8 ms, tout en restant sous le débit nominal.

Les pauses RTS protègent le FIFO pendant les tâches graphiques mais peuvent augmenter le retard des rapports en attente. Le commentaire HWTest fait état d’une reprise UART supérieure à 300 µs (`nova_bt_stack.c:356`) ; il s’agit d’un résultat antérieur au présent audit. Les tests existants simulent des trous de reprise de 700 µs. **Ne pas abaisser arbitrairement 8 ms / 2 ms, supprimer RTS/CTS, changer le seuil FIFO ou augmenter le baudrate dans cette itération.** Comparer d’abord `poll_gap_max_us`, `rx_budget_hits`, erreurs UART et régularité des rapports sur la console.

### 4. Allocations et statistiques : optimisation secondaire testable

Les objets BTstack sont en pools fixes : 2 connexions HCI, 8 canaux L2CAP et 2 connexions HID (`bluetooth/btstack_config.h:9`, `libs/btstack/btstack_memory.c:140`, `:257`, `:844`). Aucun malloc n’a été ajouté au chemin de réception ou au latch.

Le panneau de statistiques formate un texte contenant des compteurs et positions toutes les 500 ms puis appelle `lv_label_set_text` et ajuste la taille. Le garde identique de `bdk/libs/lvgl/lv_objx/lv_label.c:150` évite les travaux si le texte ne change pas, mais les compteurs changent pendant le streaming. Le chemin modifié libère/alloue et recalcule le label (`:169` à `:183`). Cela constitue une source de redessins occasionnels, sans preuve qu’elle soit perceptible sur console. Optimisation future testable : figer les détails avancés hors de leur panneau, conserver les informations utiles visibles et comparer les durées de frame panneau ouvert/fermé. Le Stop explicite écrit le rapport SD ; ce coût ponctuel ne doit pas être confondu avec celui du streaming.

## Vérification native

`python work/check_nova30_wireless.py` passe contre les sources v30 réelles, radio et horloge substituées. Le script compile aussi les sources v29 réelles pour démontrer les deux échecs ciblés :

- v29 perd l’appui 1 000 ms / relâchement 1 017 ms avant une lecture 1 033 ms ; v30 le rend une fois puis rend le relâchement.
- v29 ne compte pas un callback de boucle de 3 000 µs dans `rx_max_us` ; v30 le compte dans le maximum de poll complet.
- v30 vérifie l’état tenu, l’expiration indépendante de deux boutons, le stale, un vrai événement de déconnexion HCI, le nettoyage périodique stale, Stop/redémarrage, rapports simples/complets, côté gauche/droit et bouclage d’horloge.
- v30 vérifie exactement quatre lectures d’horloge dans un poll vide instrumenté, sans attente ajoutée ; un callback de paquet simulé de 600 µs atteint bien le plafond rapide de 250 µs, ce qui montre aussi qu’un callback n’est pas préemptible.
- La suite native H4/BTstack existante est réutilisée et passe : initialisation réelle de 57 commandes HCI, parsing, clés, entrée, reprise et échéances. Seules ses assertions de coût avec horloge virtuelle sont ajustées de 270 à 280 µs (et 27 000 à 28 000 pour cent appels), soit une lecture d’horloge virtuelle supplémentaire par appel.

Les tests `input` existants utilisant une extraction du merge et un faux statut devront déclarer un stub `nova_bt_take_buttons` lorsque le parent intègre l’API. Pour tester le latch de bout en bout, relier l’implémentation réelle plutôt qu’un stub qui renvoie seulement les boutons courants. Les suites de négociation/radio physiques restent nécessaires après intégration ; ce rapport n’annonce pas leur exécution sur une console.

Validation intégrée par le parent : consommation dans le merge avant priorité rails, tests de boutons brefs dans ce merge, maxima exposés au rapport SD et génération nova30-wireless-check.json. Les budgets radio physiques restent conservés.
