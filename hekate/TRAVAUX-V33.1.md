# v33.1

- Tuile Démarrer : petite icône /icone/launch.png indépendante du grand fond /icone/Launcher.png, remplaçable depuis Nova ; aperçu cohérent.
- Crédits : attribution de HWTest-RCM 0.15 à Creased (Baptiste MOINE), dans Logiciels libres.
- Apparence et options : retrait du fond des lignes Vérification des sauvegardes et Effets de sélection ; contour du panneau Options système harmonisé.
- Sauvegarde et restauration emuMMC : contour actif repositionné sur le panneau extérieur, sans double cadre décalé ; comportement des boutons conservé.
- Deux packs distincts : standard et expérimental RAM 8 Go pour consoles physiquement modifiées (Erista V1 et Mariko V2/Lite/OLED). Le second applique le drapeau logiciel officiel Hekate, sans programmer de fusibles.
- Diagnostic RAM : conservation du mode 8 Go au lancement et au retour, sélection des profils selon la plateforme et la capacité, arrêt en cas de capacité incohérente après entraînement.
- Vérification des adresses hautes avec calculs 64 bits et compte rendu de la capacité réelle ; 8 176 Mio testables sur 8 192 Mio, les 16 Mio de travail restant protégés.
- Atmosphère 1.11.0 minimum pour les modes mémoire au-delà de 4 Go : memmode=1 dans une entrée Hekate utilisant fss0, ou enable_mem_mode=1 dans exosphere.ini pour fusee. Les configurations existantes ne sont pas modifiées automatiquement.

Bêta : compilations ARM et contrôles PC réussis. Le mode 8 Go, notamment le profil Erista adapté, attend une validation matérielle sur chaque plateforme ; aucune compatibilité avec toutes les puces de remplacement n’est certifiée. Les longs rapports chinois conservent une traduction automatique à relire.
