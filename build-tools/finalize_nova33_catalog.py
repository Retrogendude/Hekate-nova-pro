from pathlib import Path
import json,re
import unicodedata
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33/nyx/nyx_gui'
catalog=json.loads((P/'localization_catalog.json').read_text(encoding='utf-8'))
# Fresh English messages have explicit translations for the seven existing locales.
new=[
['Chinese font (required)','Police chinoise imposée','Fuente china obligatoria','Carattere cinese obbligatorio','Обязательный китайский шрифт','Chinesische Pflichtschrift','Fonte chinesa obrigatória','中文专用字体'],
['Personal fonts disabled for Chinese.','Police personnalisée indisponible en chinois.','Fuentes personales desactivadas en chino.','Caratteri personali disattivati in cinese.','Для китайского используется специальный шрифт.','Eigene Schriften sind für Chinesisch deaktiviert.','Fontes pessoais desativadas em chinês.','中文模式使用专用字体。'],
['Hekate original','Hekate d’origine','Hekate original','Hekate originale','Исходный Hekate','Hekate-Original','Hekate original','Hekate 原版字体'],
['Boot options','Options de démarrage','Opciones de arranque','Opzioni di avvio','Параметры загрузки','Startoptionen','Opções de arranque','启动选项'],
['Backlight is available in Nyx Settings > System.','Luminosité : Réglages Nyx > Système.','Brillo: Ajustes de Nyx > Sistema.','Luminosità: Impostazioni Nyx > Sistema.','Яркость: Настройки Nyx > Система.','Helligkeit: Nyx-Einstellungen > System.','Brilho: Definições do Nyx > Sistema.','屏幕亮度位于 Nyx 设置 > 系统。'],
['Auto Boot / Boot Delay / Auto NoGC / Auto HOS Power Off','Démarrage auto / Délai / Auto NoGC / Extinction HOS','Inicio automático / Espera / Auto NoGC / Apagado HOS','Avvio automatico / Ritardo / Auto NoGC / Spegnimento HOS','Автозагрузка / Задержка / Auto NoGC / Выключение HOS','Autostart / Wartezeit / Auto NoGC / HOS-Abschaltung','Arranque automático / Atraso / Auto NoGC / Desligar HOS','自动启动 / 启动延时 / Auto NoGC / HOS 自动关机'],
['Chinese font missing or invalid. English restored for this session.','Police chinoise absente ou invalide. Anglais rétabli pour cette session.','Fuente china ausente o no válida. Inglés restaurado para esta sesión.','Carattere cinese assente o non valido. Inglese ripristinato per questa sessione.','Китайский шрифт отсутствует или повреждён. В этом сеансе используется английский.','Chinesische Schrift fehlt oder ist ungültig. Englisch für diese Sitzung wiederhergestellt.','Fonte chinesa ausente ou inválida. Inglês reposto nesta sessão.','中文字体缺失或无效。本次会话已切换为英语。'],
]
curated=r'''Home~主页
Tools~工具
Infos~信息
Your console, simply.~轻松管理你的主机。
Start, manage and check.~启动、管理与检测。
Retrouver les autres profils de démarrage.~查看其他启动配置。
Ouvrir un outil depuis la carte microSD.~从 microSD 卡启动工具。
Gérer la copie du système sur la carte SD.~管理 SD 卡上的系统副本。
Explorer les contrôles matériels.~查看硬件诊断项目。
Votre console, simplement.~轻松管理你的主机。
Démarrer, gérer et vérifier.~启动、管理与检测。
Bienvenue dans Hekate~欢迎使用 Hekate
Sauvegarde eMMC~备份 eMMC
Créer une sauvegarde du stockage.~创建内部存储备份。
Restauration eMMC~恢复 eMMC
Restaurer une sauvegarde existante.~恢复已有备份。
Partitions SD~SD 卡分区
Préparer les partitions de la carte SD.~设置 SD 卡分区。
Outils USB~USB 工具
Accéder aux fonctions de stockage USB.~使用 USB 存储功能。
Bit d’archive~归档属性
Réparer les attributs des fichiers.~修复文件的归档属性。
Écran tactile~触摸屏
Ajuster la calibration du tactile.~校准触摸屏。
Consulter et modifier le mode AutoRCM.~查看和更改 AutoRCM 模式。
Extraire les éléments du système.~提取系统组件。
Matériel et fusibles~硬件与熔丝
Identité de la console et mémoire.~查看主机与内存信息。
Informations sur le stockage interne.~查看内部存储信息。
Informations sur la carte mémoire.~查看存储卡信息。
Batterie~电池
Charge, températures et alimentation.~查看电量、温度与供电状态。
Informations et extraction.~查看信息并导出数据。
Données des clés HDCP.~HDCP 密钥数据。
Outil présent sur la carte SD.~使用 SD 卡上的工具。
Bilan HWTest~HWTest 总览
Vue d’ensemble des groupes.~查看各诊断分组。
Identité~主机信息
SoC, fusibles et KFUSE.~SoC、熔丝与 KFUSE。
Alimentation~供电
Batterie, charge et températures.~电池、充电与温度。
Mémoire~内存
DRAM et horloges.~DRAM 与时钟。
Stockage~存储
SD, eMMC et lecteur.~SD 卡、eMMC 与读卡器。
Sans fil~无线
Bluetooth et Wi-Fi.~蓝牙与 Wi-Fi。
Codec et horloges audio.~音频编解码器与时钟。
Test sonore~声音测试
Haut-parleurs et prise casque.~扬声器与耳机接口。
Écran~屏幕
Dalle et rétroéclairage.~显示面板与背光。
Commandes~控制器
Tactile, Joy-Con et boutons.~触摸屏、Joy-Con 与按键。
Bas niveau~底层硬件
GPIO, UART et démarrage.~GPIO、UART 与启动。
Test RAM isolé~独立内存测试
Test séparé, retour à Hekate.~运行独立测试，完成后返回 Hekate。
Options système~系统选项
Apparence et lecture~外观与阅读
Thèmes et couleurs~主题与颜色
Couleurs indépendantes, fond d’écran et transparence.~分别调整颜色、壁纸与透明度。
Sélection~选中效果
Reflet blanc diagonal~白色斜向反光
Sans reflet~关闭反光
Effets de sélection~选中效果
Reflet blanc discret sur la tuile sélectionnée.~为选中的磁贴显示柔和的白色反光。
Vérification des sauvegardes~备份校验
Contrôler les données écrites.~校验写入的数据。
Advanced test~高级测试
Quick test~快速测试
Custom test~自定义测试
Run all seven RAM patterns.~运行全部七种内存测试。
Console Info~主机信息
Options~选项
Launch~启动
Start~开始
More Configurations~其他启动配置
More Configs~其他配置
Configurations~启动配置
Payloads~载荷
Diagnostics~诊断
Diagnostic~诊断
Nyx Settings~Nyx 设置
Close~关闭
OK~确定
Cancel~取消
Done~完成
Save~保存
Save Options~保存选项
Save & Reload~保存并重新加载
Save & reload~保存并重新加载
Reload~重新加载
Do not reload~暂不重新加载
Reboot~重启
Reboot (RCM)~重启至 RCM
Power Off~关机
Open~打开
Continue~继续
Previous~上一页
Next~下一页
Next Step~下一步
Yes~是
No~否
Enable~启用
Disable~禁用
Enabled~已启用
Disabled~已禁用
Active~已启用
Inactive~已禁用
Save & Reload (R)~保存并重新加载 (R)
Save & reload (R)~保存并重新加载 (R)
On~开
Off~关
Normal~正常
Auto~自动
Unknown~未知
Error~错误
Failed~失败
Pass~通过
Warning~警告
WARNING~警告
Your real DRAM ID does not match your RAM!~实际 DRAM ID 与已安装的内存不匹配！
Density and rank differ!~内存密度和 Rank 配置不同！
Choosing to boot that way will cause~以此方式启动会导致
performance degradation or even crashes.~性能下降，甚至系统崩溃。
Do you really want to boot via OFW method?~确定仍要使用 OFW 方式启动吗？
This operation is irreversible!~此操作不可逆！
The data will be lost!~数据将丢失！
Warning: Do you really want to continue?!~警告：确定要继续吗？！
Do you really want~确定要
auto-erase~自动擦除
secure-erase~安全擦除
Average Erases SLC:~SLC 平均擦除次数：
Average Erases MLC:~MLC 平均擦除次数：
Average Erases Hybrid:~混合区域平均擦除次数：
Maximum Erases SLC:~SLC 最大擦除次数：
Maximum Erases MLC:~MLC 最大擦除次数：
Minimum Erases SLC:~SLC 最小擦除次数：
Minimum Erases MLC:~MLC 最小擦除次数：
Maximum Erases EUDA:~EUDA 最大擦除次数：
Minimum Erases EUDA:~EUDA 最小擦除次数：
Average Erases EUDA:~EUDA 平均擦除次数：
FAIL~失败
PASS~通过
Please wait...~请稍候…
Done!~已完成！
Do you want to continue?~是否继续？
You changed the configuration!~配置已更改！
Do you want to save it?~是否保存更改？
Select a payload to launch:~选择要启动的载荷：
Language~语言
Interface language~界面语言
Choose the interface language~选择界面语言
Chinese (Simplified)~简体中文
Appearance~外观
Appearance & language~外观与语言
Font~字体
Default~默认
Default font~默认字体
SD font loaded~已加载字体
Text size~文字大小
%d Go~%d GB
%d Go Full~%d GB Full
2 x %d Go~2 x %d GB
2 x %d Go Full~2 x %d GB Full
Reading~阅读
Layout~布局
Home layout~主页布局
Choose layout~选择布局
Original~原始
Tiles~磁贴
Carousel~轮播
Color Theme~配色主题
Theme~主题
Color~颜色
Colors~颜色
Background~背景
Wallpaper~壁纸
Transparency~透明度
Selection~选中效果
Interface~界面
System~系统
Clock (Offset)~日期与时间
Backlight~屏幕亮度
Auto Boot~自动启动
Boot Time Delay~启动延时（秒）
Auto NoGC~自动保护游戏卡槽
Auto HOS Power Off~HOS 自动关机
Update Reboot 2 Payload~更新重启载荷
Main configurations~主要启动配置
Ini folder configurations~其他配置文件
Home Screen~启动页面
Main menu~主菜单
All Configs~全部配置
Data Verification~数据校验
Sparse~抽样
Full~完整
Hashes~哈希
TV only~仅电视
TV + console~电视与主机
TV display~显示方式
Automatic dock detection~自动检测底座
Wireless Joy-Con~无线 Joy-Con
Start TV~开启电视输出
Stop TV~停止电视输出
Start Bluetooth~开启蓝牙
Stop Bluetooth~停止蓝牙
Read pairings~读取配对信息
Credits~鸣谢
Changelog~更新日志
About~关于
Temperature~温度
Battery~电池
Information~信息
Storage~存储
Memory~内存
SD Card~SD 卡
SD Partition Manager~SD 卡分区管理
Simple mode~简易模式
Expert mode~专家模式
Repair MBR/GPT hybrid~修复混合 MBR/GPT
Notes~说明
Capacity~容量
Summary~汇总
Format~格式化
Backup~备份
Restore~恢复
Test~测试
Tests~测试
Test complete~测试完成
Save report~保存报告
Results~结果
Choose a system~选择系统
Start, manage and check your console.~启动、管理并检查主机。
Choose the system to start on your console.~选择要在主机上启动的系统。
View other boot profiles.~查看其他启动配置。
Launch a tool from the microSD card.~从 microSD 卡启动工具。
Manage the system copy on the microSD card.~管理 microSD 卡上的系统副本。
Choose your language, reading comfort and background in one place.~设置界面语言、文字和背景。
Save & reload applies all choices. Closing cancels the pending changes.~保存并重新加载后应用全部选项。关闭窗口将取消未保存的更改。
To add a font, copy a Nyx font file into /fonts on the SD card, then reload Nyx.~添加字体：将 Nyx 字体文件复制到 SD 卡根目录的 /fonts 文件夹，然后重新加载 Nyx。
The selected font is missing or invalid. Choose another font.~所选字体缺失或无效，请选择其他字体。
SD card unavailable. Nothing was saved.~无法访问 SD 卡，未保存任何更改。
Nyx is missing. Nothing was saved.~找不到 Nyx，未保存任何更改。
Could not save settings. Check the SD card and retry.~保存失败，请检查 SD 卡后重试。
Could not save appearance settings. Check the SD card and try again.~外观设置保存失败，请检查 SD 卡后重试。
Settings saved.~设置已保存。
Settings saved. Reload Nyx to apply the changes.~设置已保存。重新加载 Nyx 后应用更改。
Set backlight brightness.~调整屏幕亮度。
Choose which boot entry or payload to automatically boot when injecting.~选择注入载荷后自动启动的配置或载荷。
Check the fuses and automatically apply the game-card protection patch when needed.~检查熔丝状态，并在需要时自动应用游戏卡槽保护补丁。
Automatically power off on the next payload injection after shutting down from HOS.~从 HOS 关机后，在下一次注入载荷时自动关机。
When FSS0 is used, check and update the reboot-to-payload image to Hekate.~使用 FSS0 时检查并将重启载荷更新为 Hekate。
Font unavailable or invalid. Default font restored.~字体缺失或无效，已使用默认字体。
Not enough memory. Default font restored.~内存不足，已使用默认字体。
'''
manual=dict(line.split('~',1) for line in curated.strip().splitlines())
manual.update(json.loads((W/'nova33_ram_zh.json').read_text(encoding='utf-8')))
version_key='v33 · Languages and defaults\nSimplified Chinese with its required font.\nBundled wallpapers, icons and clearer boot options.'
catalog[version_key]=[
 version_key,
 'v33 · Langues et réglages par défaut\nChinois simplifié avec sa police dédiée.\nFonds, icônes et options de démarrage plus visibles.',
 'v33 · Idiomas y ajustes predeterminados\nChino simplificado con su fuente obligatoria.\nFondos, iconos y opciones de arranque más visibles.',
 'v33 · Lingue e impostazioni predefinite\nCinese semplificato con carattere dedicato.\nSfondi, icone e opzioni di avvio più visibili.',
 'v33 · Языки и настройки по умолчанию\nУпрощённый китайский со специальным шрифтом.\nОбои, значки и заметные параметры загрузки.',
 'v33 · Sprachen und Vorgaben\nVereinfachtes Chinesisch mit eigener Schrift.\nHintergründe, Symbole und deutlichere Startoptionen.',
 'v33 · Idiomas e predefinições\nChinês simplificado com fonte dedicada.\nFundos, ícones e opções de arranque mais visíveis.'
]
manual[version_key]='v33 · 语言与默认设置\n简体中文及其专用字体。\n内置壁纸、图标与更清晰的启动选项。'
manual['No configurations found.']='未找到启动配置。'
manual['Could not read boot configurations. Check the SD card and retry.']='无法读取启动配置，请检查 SD 卡后重试。'
manual['#FF8000 Warning#\n\n#FFDD00 Your real DRAM ID does not match your RAM!#\n#FFDD00 Density and rank differ!#\n\n#FFDD00 Choosing to boot that way will cause #\n#FFDD00 performance degradation or even crashes.#\n\nDo you really want to boot via OFW method?#']='#FF8000 警告#\n\n#FFDD00 实际 DRAM ID 与已安装的内存不匹配！#\n#FFDD00 内存密度和 Rank 配置不同！#\n\n#FFDD00 以此方式启动会导致性能下降，#\n#FFDD00 甚至系统崩溃。#\n\n确定仍要使用 OFW 方式启动吗？#'
draft=json.loads((P/'localization_zh_draft.json').read_text(encoding='utf-8'))
for row in new:catalog[row[0]]=row[:7];manual[row[0]]=row[7]
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for key,v in catalog.items():
 zh=unicodedata.normalize('NFC',manual.get(key,manual.get(v[0],draft.get(key,v[0]))))
 assert fmt.findall(zh)==fmt.findall(key),(key,zh)
 catalog[key]=v[:7]+[zh]
for key,zh in manual.items():
 if key not in catalog:catalog[key]=[key]*7+[zh]
(P/'localization_catalog.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(P/'localization_zh_reviewed.json').write_text(json.dumps(manual,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
lines=['/* Nova v33 UTF-8 catalog: en fr es it ru de pt zh-CN. */','static const locale_entry locale_catalog[] = {']
quote=lambda s:json.dumps(s,ensure_ascii=False)
for key,v in sorted(catalog.items()):lines.append('    {'+quote(key)+', {'+', '.join(map(quote,v))+'}},')
lines.append('};')
rows=[i for i,(key,_)in enumerate(sorted(catalog.items())) if '%'in key and len(re.findall('[A-Za-z]',key))>=5]
lines.append('static const uint16_t locale_pattern_rows[] = {'+', '.join(map(str,rows))+'};')
first=[sorted(catalog)[i].encode()[0]for i in rows]
starts=[sum(ch<code for ch in first)for code in range(257)]
lines.append('static const uint16_t locale_pattern_start[257] = {'+', '.join(map(str,starts))+'};')
(P/'localization_catalog.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
characters=sorted({ord(c)for v in catalog.values()for c in v[7]if ord(c)>=32})
(W/'nova33-zh-characters.json').write_text(json.dumps(characters))
print('Catalog',len(catalog),'manual entries',len(manual),'Chinese characters',len(characters),'Han',sum(0x3400<=x<=0x9fff for x in characters))
