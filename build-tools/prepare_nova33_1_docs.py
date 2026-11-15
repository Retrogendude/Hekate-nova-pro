from pathlib import Path
import json
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.1';O=W.parent/'outputs'
old=json.loads((W/'nova33_release_notes.json').read_text(encoding='utf-8'))
updates={
'fr':[
'Tuile Démarrer : petite icône /icone/launch.png indépendante du grand fond /icone/Launcher.png, remplaçable depuis Nova ; aperçu cohérent.',
'Crédits : attribution de HWTest-RCM 0.15 à Creased (Baptiste MOINE), dans Logiciels libres.',
'Apparence et options : retrait du fond des lignes Vérification des sauvegardes et Effets de sélection ; contour du panneau Options système harmonisé.',
'Sauvegarde et restauration emuMMC : contour actif repositionné sur le panneau extérieur, sans double cadre décalé ; comportement des boutons conservé.',
'Deux packs distincts : standard et expérimental RAM 8 Go pour consoles physiquement modifiées (Erista V1 et Mariko V2/Lite/OLED). Le second applique le drapeau logiciel officiel Hekate, sans programmer de fusibles.',
'Diagnostic RAM : conservation du mode 8 Go au lancement et au retour, sélection des profils selon la plateforme et la capacité, arrêt en cas de capacité incohérente après entraînement.',
'Vérification des adresses hautes avec calculs 64 bits et compte rendu de la capacité réelle ; 8 176 Mio testables sur 8 192 Mio, les 16 Mio de travail restant protégés.',
'Atmosphère 1.11.0 minimum pour les modes mémoire au-delà de 4 Go : memmode=1 dans une entrée Hekate utilisant fss0, ou enable_mem_mode=1 dans exosphere.ini pour fusee. Les configurations existantes ne sont pas modifiées automatiquement.'
],
'en':[
'Launch tile: the small /icone/launch.png icon is independent of the large /icone/Launcher.png background; replace the latter in Nova with a matching preview.',
'Credits: HWTest-RCM 0.15 is attributed to Creased (Baptiste MOINE) under Free software.',
'Appearance and options: removed the row backgrounds for backup verification and selection effects; matched the System options panel outline.',
'emuMMC backup and restore: the active outline belongs to the outer panel, removing the offset double frame while preserving button behavior.',
'Separate standard and experimental 8GB packs for physically upgraded consoles: Erista V1 and Mariko V2/Lite/OLED. The latter uses the official Hekate software flag and does not program fuses.',
'RAM diagnostic preserves the 8GB mode on entry and return, selects training profiles by platform and capacity, and stops if capacity changes after training.',
'High addresses use 64-bit calculations and results report actual capacity: 8,176 MiB testable out of 8,192 MiB, with 16 MiB protected for working data.',
'Atmosphère 1.11.0 or newer is required for memory modes above 4GB: memmode=1 in an fss0 Hekate entry, or enable_mem_mode=1 in exosphere.ini for fusee. Existing configurations are not edited automatically.'
],
'es':[
'Baldosa Iniciar: icono pequeño /icone/launch.png independiente del fondo grande /icone/Launcher.png, sustituible en Nova con vista previa coherente.',
'Créditos: HWTest-RCM 0.15 atribuido a Creased (Baptiste MOINE) en Software libre.',
'Apariencia y opciones: eliminados los fondos de Verificación de copias y Efectos de selección; borde de Opciones del sistema armonizado.',
'Copia y restauración emuMMC: el borde activo se sitúa en el panel exterior, sin doble marco desplazado; se conserva el comportamiento de los botones.',
'Paquetes separados estándar y experimental de 8 GB para consolas modificadas físicamente: Erista V1 y Mariko V2/Lite/OLED. Se utiliza la marca de software oficial de Hekate sin programar fusibles.',
'Diagnóstico RAM: conserva el modo de 8 GB al iniciar y volver, selecciona perfiles por plataforma y capacidad y detiene la prueba si la capacidad cambia tras el entrenamiento.',
'Direcciones altas calculadas en 64 bits e informe de capacidad real: 8.176 MiB comprobables de 8.192 MiB; 16 MiB protegidos para datos de trabajo.',
'Atmosphère 1.11.0 o posterior para modos superiores a 4 GB: memmode=1 en una entrada Hekate fss0 o enable_mem_mode=1 en exosphere.ini para fusee. No se modifican automáticamente las configuraciones existentes.'
],
'it':[
'Riquadro Avvia: icona piccola /icone/launch.png indipendente dallo sfondo grande /icone/Launcher.png, sostituibile in Nova con anteprima coerente.',
'Crediti: HWTest-RCM 0.15 attribuito a Creased (Baptiste MOINE) in Software libero.',
'Aspetto e opzioni: rimossi gli sfondi delle righe Verifica backup ed Effetti selezione; uniformato il bordo di Opzioni di sistema.',
'Backup e ripristino emuMMC: bordo attivo sul pannello esterno, senza doppia cornice spostata; comportamento dei pulsanti preservato.',
'Pacchetti separati standard e sperimentale da 8 GB per console modificate fisicamente: Erista V1 e Mariko V2/Lite/OLED. Il secondo usa il flag software ufficiale Hekate, senza programmare fusibili.',
'Diagnostica RAM: conserva la modalità 8 GB all’avvio e al ritorno, seleziona i profili per piattaforma e capacità e si arresta se la capacità cambia dopo l’addestramento.',
'Indirizzi alti con calcoli a 64 bit e rapporto della capacità reale: 8.176 MiB verificabili su 8.192 MiB, con 16 MiB protetti per i dati di lavoro.',
'Atmosphère 1.11.0 o successivo per modalità oltre 4 GB: memmode=1 in una voce Hekate fss0 oppure enable_mem_mode=1 in exosphere.ini per fusee. Le configurazioni esistenti non vengono modificate automaticamente.'
],
'ru':[
'Плитка запуска: маленький значок /icone/launch.png независим от большого фона /icone/Launcher.png; фон меняется в Nova с соответствующим предпросмотром.',
'Авторы: HWTest-RCM 0.15 — Creased (Baptiste MOINE), в разделе свободного ПО.',
'Внешний вид и параметры: убраны фоны строк проверки резервных копий и эффектов выбора; рамка системных параметров приведена к общему стилю.',
'Резервное копирование и восстановление emuMMC: активная рамка на внешней панели без смещённого двойного контура; логика кнопок сохранена.',
'Отдельные стандартный и экспериментальный пакеты 8 ГБ для физически модифицированных консолей Erista V1 и Mariko V2/Lite/OLED. Используется официальный программный флаг Hekate без записи предохранителей.',
'Диагностика ОЗУ сохраняет режим 8 ГБ при запуске и возврате, выбирает профиль по платформе и объёму и прекращает тест при изменении объёма после тренировки.',
'64-битные вычисления верхних адресов и отчёт о реальном объёме: проверяется 8176 МиБ из 8192 МиБ, 16 МиБ защищены для рабочих данных.',
'Для режимов свыше 4 ГБ требуется Atmosphère 1.11.0 или новее: memmode=1 в записи Hekate fss0 либо enable_mem_mode=1 в exosphere.ini для fusee. Существующие конфигурации автоматически не меняются.'
],
'de':[
'Start-Kachel: kleines Symbol /icone/launch.png unabhängig vom großen Hintergrund /icone/Launcher.png; in Nova mit passender Vorschau austauschbar.',
'Mitwirkende: HWTest-RCM 0.15 von Creased (Baptiste MOINE), unter Freie Software.',
'Darstellung und Optionen: Hintergründe der Zeilen Sicherungsprüfung und Auswahleffekte entfernt; Rahmen der Systemoptionen vereinheitlicht.',
'emuMMC-Sicherung und Wiederherstellung: aktiver Rahmen auf dem äußeren Panel, ohne versetzten Doppelrahmen; Tastenverhalten bleibt erhalten.',
'Separate Standard- und experimentelle 8-GB-Pakete für physisch aufgerüstete Konsolen: Erista V1 und Mariko V2/Lite/OLED. Offizielles Hekate-Softwareflag ohne Programmierung von Fuses.',
'RAM-Diagnose erhält den 8-GB-Modus bei Start und Rückkehr, wählt Trainingsprofile nach Plattform und Kapazität und stoppt bei Kapazitätsänderung nach dem Training.',
'64-Bit-Berechnung hoher Adressen und Bericht der tatsächlichen Kapazität: 8176 MiB von 8192 MiB prüfbar; 16 MiB für Arbeitsdaten geschützt.',
'Atmosphère ab 1.11.0 für Speichermodi über 4 GB: memmode=1 in einem Hekate-fss0-Eintrag oder enable_mem_mode=1 in exosphere.ini für fusee. Vorhandene Konfigurationen werden nicht automatisch geändert.'
],
'pt':[
'Bloco Iniciar: ícone pequeno /icone/launch.png independente do fundo grande /icone/Launcher.png, substituível no Nova com pré-visualização correspondente.',
'Créditos: HWTest-RCM 0.15 atribuído a Creased (Baptiste MOINE), em Software livre.',
'Aspeto e opções: removidos os fundos das linhas Verificação das cópias e Efeitos de seleção; contorno de Opções do sistema uniformizado.',
'Cópia e restauro emuMMC: contorno ativo no painel exterior, sem moldura dupla deslocada; comportamento dos botões preservado.',
'Pacotes separados padrão e experimental de 8 GB para consolas fisicamente modificadas: Erista V1 e Mariko V2/Lite/OLED. Utiliza o sinalizador oficial de software Hekate, sem programar fusíveis.',
'Diagnóstico RAM: mantém o modo de 8 GB ao iniciar e regressar, escolhe perfis por plataforma e capacidade e para se a capacidade mudar após o treino.',
'Endereços altos com cálculos de 64 bits e relatório da capacidade real: 8176 MiB testáveis de 8192 MiB; 16 MiB protegidos para dados de trabalho.',
'Atmosphère 1.11.0 ou posterior para modos acima de 4 GB: memmode=1 numa entrada Hekate fss0 ou enable_mem_mode=1 em exosphere.ini para fusee. As configurações existentes não são alteradas automaticamente.'
],
'zh-CN':[
'启动磁贴：左上角的小图标 /icone/launch.png 与大背景 /icone/Launcher.png 独立，背景可在 Nova 中更换，预览与实际一致。',
'鸣谢：在自由软件页面列出 HWTest-RCM 0.15 及其开发者 Creased（Baptiste MOINE）。',
'外观与选项：移除备份验证及选择效果行的底色，统一系统选项面板的边框。',
'emuMMC 备份与恢复：活动边框位于外层面板，消除错位双框，保留按钮行为。',
'分别提供标准包与实验性 8GB 包，后者仅用于已物理扩容的 Erista V1 和 Mariko V2/Lite/OLED。使用 Hekate 官方软件标志，不写入熔丝。',
'内存检测：启动和返回时保留 8GB 模式，按平台和容量选择训练配置；训练后容量不一致则停止检测。',
'采用 64 位计算访问高位地址，并报告实际容量：8192 MiB 中可测试 8176 MiB，另外 16 MiB 用于受保护的工作数据。',
'超过 4GB 的内存模式需要 Atmosphère 1.11.0 或更新版本：Hekate fss0 启动项使用 memmode=1，fusee 则在 exosphere.ini 中使用 enable_mem_mode=1。不会自动修改现有配置。'
]}
validation={
'fr':'Bêta : compilations ARM et contrôles PC réussis. Le mode 8 Go, notamment le profil Erista adapté, attend une validation matérielle sur chaque plateforme ; aucune compatibilité avec toutes les puces de remplacement n’est certifiée. Les longs rapports chinois conservent une traduction automatique à relire.',
'en':'Beta: ARM builds and PC checks passed. The 8GB mode, especially the adapted Erista profile, still requires hardware validation on each platform; compatibility with every replacement chip is not certified. Long Chinese reports retain machine translations pending review.',
'es':'Beta: compilaciones ARM y controles PC correctos. El modo de 8 GB, especialmente el perfil Erista adaptado, requiere validación en cada plataforma; no se certifican todas las memorias de sustitución. Los informes chinos largos aún necesitan revisión lingüística.',
'it':'Beta: compilazioni ARM e controlli PC superati. La modalità 8 GB, soprattutto il profilo Erista adattato, richiede verifica su ogni piattaforma; non sono certificati tutti i chip sostitutivi. I lunghi rapporti cinesi richiedono revisione linguistica.',
'ru':'Бета: сборки ARM и проверки на ПК пройдены. Режим 8 ГБ, особенно адаптированный профиль Erista, требует проверки на каждой платформе; совместимость со всеми заменяемыми чипами не подтверждена. Длинные китайские отчёты требуют языковой проверки.',
'de':'Beta: ARM-Builds und PC-Prüfungen erfolgreich. Der 8-GB-Modus, insbesondere das angepasste Erista-Profil, benötigt Hardwaretests je Plattform; nicht alle Ersatzchips sind zertifiziert. Lange chinesische Berichte benötigen sprachliche Prüfung.',
'pt':'Beta: compilações ARM e verificações PC concluídas. O modo de 8 GB, sobretudo o perfil Erista adaptado, exige validação em cada plataforma; não se certificam todos os chips de substituição. Relatórios chineses longos precisam de revisão linguística.',
'zh-CN':'测试版：ARM 编译与电脑端检查已通过。8GB 模式，尤其是调整后的 Erista 配置，仍需在各平台实机验证；不保证所有替换内存芯片均兼容。较长的中文报告仍含待校对的机器翻译。'
}
notes={}
for code,items in updates.items():
 notes[code]={'title':'Hekate Nova Pro v33.1 beta','items':items,'inherited_v33':old[code]['items'],'validation':validation[code]}
 text='# '+notes[code]['title']+'\n\n'+'\n'.join('- '+v for v in items)+'\n\n'+validation[code]+'\n\n## v33\n\n'+'\n'.join('- '+v for v in old[code]['items'])+'\n'
 (O/('CHANGELOG-NOVA33.1-'+code+'.md')).write_text(text,encoding='utf-8')
(W/'nova33_1_release_notes.json').write_text(json.dumps(notes,ensure_ascii=False,indent=2),encoding='utf-8')
readme=(P/'LIRE-MOI-V33.md').read_text(encoding='utf-8')
readme=readme.replace('v33 — bêta de test','v33.1 — bêta').replace('Cette archive est destinée au prochain essai sur console ; la publication GitHub attend le retour de cet essai.','Version publiée en bêta ; le mode 8 Go et le diagnostic associé attendent une validation sur consoles modifiées.').replace('hekate_nova_pro_6.5.5_v33_beta.bin','hekate_nova_pro_6.5.5_v33.1_beta.bin')
readme=readme.replace('Les sept anciens packs de langues restent compatibles ; la logique des tests mémoire et de l’entraînement n’a pas été changée.','Les huit packs de langues sont inclus. Le mode 8 Go est transmis au diagnostic et conservé au retour ; les profils et la capacité sont contrôlés avant le test.')
readme+='\n## Corrections v33.1\n\n'+'\n'.join('- '+v for v in updates['fr'][:4])+'\n\nVoir aussi [le guide RAM 8 Go](RAM-8GO-V33.1.md).\n'
(P/'LIRE-MOI-V33.1.md').write_text(readme,encoding='utf-8')
(P/'LIRE-MOI-V33.md').write_text('# Documentation\n\nVoir [la notice v33.1](LIRE-MOI-V33.1.md).\n',encoding='utf-8')
ram='''# RAM 8 Go — Hekate Nova Pro v33.1 bêta

## Choisir le bon pack

Le pack **standard** est destiné aux consoles normales de 4 Go. Le pack **8GB expérimental** est destiné exclusivement aux consoles dont la mémoire a été physiquement remplacée pour atteindre 8 Go : V1/Erista ou V2/Lite/OLED/Mariko. Un logiciel ne transforme pas 4 Go en 8 Go.

Installer ensemble le payload et le dossier bootloader du même pack. Le payload de la variante 8GB porte un nom explicite ; son bootloader/update.bin porte le même drapeau que le payload injecté. Aucun fusible matériel n’est programmé par cette option.

## Atmosphère

Atmosphère 1.11.0 ou plus récent est nécessaire pour les modes mémoire supérieurs à 4 Go. Pour un démarrage Hekate via fss0, ajouter **memmode=1** à l’entrée Atmosphère existante dans bootloader/hekate_ipl.ini. Ne pas créer une deuxième entrée identique et conserver vos chemins et options emuMMC.

Pour un démarrage via fusee, conserver votre section [exosphere] dans exosphere.ini et y ajouter **enable_mem_mode=1**. Le pack ne remplace aucun de ces fichiers et n’inclut pas Atmosphère. Ne pas utiliser la méthode OFW directe lorsqu’un avertissement Hekate signale une incompatibilité entre l’identifiant DRAM et la RAM installée.

## Diagnostic séparé

Le diagnostic reprend le drapeau 8 Go avant de réinitialiser la mémoire, lit la capacité du contrôleur et sélectionne les profils selon la plateforme et la capacité. Une incohérence après entraînement arrête le test avec une erreur, sans annoncer PASS. Le retour à Nova conserve le mode initial, même si le fichier de retour sur SD avait un autre drapeau.

Le balayage utilise des adresses physiques 64 bits et une fenêtre SMMU de 4 Mio pour accéder aussi à la RAM au-delà de 4 Gio d’adresse. Sur 8 192 Mio, **8 176 Mio** sont testables ; **16 Mio** sont protégés pour les données de retour, les tables SMMU et le framebuffer. Les pages du framebuffer recouvrent deux fenêtres. Le compte rendu indique séparément la capacité, la zone testable et la zone vérifiée. Le test rapide ne réalise que le motif d’adresse ; le test avancé réalise les sept motifs, dont PRBS et rétention. Une interruption n’est pas un résultat PASS.

## État de validation

Les compilations ARM, la transmission du drapeau, les profils de sélection, les adresses jusqu’à 0x280000000, les sept motifs sur des fenêtres représentatives, les erreurs injectées et les rapports ont été contrôlés sur PC. Cela ne valide pas électriquement une console ni ses puces de remplacement. **Le support 8 Go reste expérimental sur chaque modèle.**

Le profil Mariko est celui des tables 8 Go amont. Erista n’a pas de table Atmosphère 8 Go distincte : le pont sélectionne le profil Samsung compatible 6 Go employé par l’ajustement DRAM amont, tout en conservant la géométrie 8 Go initialisée par Hekate et en contrôlant sa capacité après entraînement. Ce choix nécessite une validation réelle spécifique à Erista ; ne pas présenter ce pack comme certifié pour toutes les puces RAM.

## Références

- [Hekate : memmode et payloads RAM 8GB](https://github.com/CTCaer/hekate/releases/tag/v6.5.2)
- [Atmosphère 1.11.0 : modes mémoire au-delà de 4 Go](https://github.com/Atmosphere-NX/Atmosphere/releases/tag/1.11.0)
- [Ajustement DRAM et modes mémoire Atmosphère](https://github.com/Atmosphere-NX/Atmosphere/pull/2748)
- [HWTest-RCM de Creased](https://github.com/Creased/HWTest-RCM)
'''
(P/'RAM-8GO-V33.1.md').write_text(ram,encoding='utf-8')
(P/'TRAVAUX-V33.1.md').write_text('# v33.1\n\n'+ '\n'.join('- '+v for v in updates['fr'])+'\n\n'+validation['fr']+'\n',encoding='utf-8')
print('Prepared documentation and 8 localized changelogs')
