# Hekate Nova Pro v33.1 beta

- Riquadro Avvia: icona piccola /icone/launch.png indipendente dallo sfondo grande /icone/Launcher.png, sostituibile in Nova con anteprima coerente.
- Crediti: HWTest-RCM 0.15 attribuito a Creased (Baptiste MOINE) in Software libero.
- Aspetto e opzioni: rimossi gli sfondi delle righe Verifica backup ed Effetti selezione; uniformato il bordo di Opzioni di sistema.
- Backup e ripristino emuMMC: bordo attivo sul pannello esterno, senza doppia cornice spostata; comportamento dei pulsanti preservato.
- Pacchetti separati standard e sperimentale da 8 GB per console modificate fisicamente: Erista V1 e Mariko V2/Lite/OLED. Il secondo usa il flag software ufficiale Hekate, senza programmare fusibili.
- Diagnostica RAM: conserva la modalità 8 GB all’avvio e al ritorno, seleziona i profili per piattaforma e capacità e si arresta se la capacità cambia dopo l’addestramento.
- Indirizzi alti con calcoli a 64 bit e rapporto della capacità reale: 8.176 MiB verificabili su 8.192 MiB, con 16 MiB protetti per i dati di lavoro.
- Atmosphère 1.11.0 o successivo per modalità oltre 4 GB: memmode=1 in una voce Hekate fss0 oppure enable_mem_mode=1 in exosphere.ini per fusee. Le configurazioni esistenti non vengono modificate automaticamente.

Beta: compilazioni ARM e controlli PC superati. La modalità 8 GB, soprattutto il profilo Erista adattato, richiede verifica su ogni piattaforma; non sono certificati tutti i chip sostitutivi. I lunghi rapporti cinesi richiedono revisione linguistica.

## v33

- Cinese semplificato come ottava lingua nelle impostazioni e al primo avvio.
- Carattere dedicato obbligatorio solo in cinese. Il carattere personale viene conservato per le altre lingue e la dimensione resta regolabile.
- Lecture.nyxf è il carattere automatico per le lingue latine; il russo usa la famiglia integrata. Il carattere originale Hekate è disponibile nel selettore.
- Inclusi tre sfondi statici, cinque icone Nova e l’immagine Launcher fornita per il riquadro principale.
- Nuova installazione: francese, Carbon Cyan, opacità di pannelli e icone al 75%, testo di 30 px, colori richiesti e logo iniziale chiaro. Rimane la regolazione originale dell’orologio.
- Accesso evidente ad Auto Boot, ritardo, Auto NoGC e Auto HOS Power Off. La luminosità rimanda al controllo esistente senza duplicarlo.
- Selettore Auto Boot a pagine in stile Nova, senza scorrimento verticale e con gli indici originali di Hekate.
- Test di memoria separato con interfaccia cinese e glifi 16 × 16; compatibilità con i sette pacchetti precedenti.
- Ripristino temporaneo dell’inglese se la risorsa cinese manca o non è valida, senza sostituire il carattere personale. Le impostazioni salvate restano intatte.
