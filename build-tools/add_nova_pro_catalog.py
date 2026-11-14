from pathlib import Path
import json
root=Path(__file__).resolve().parent/'hekate-6.5.4/nyx/nyx_gui'
rows=r'''Themes and colors~Thèmes et couleurs~Temas y colores~Temi e colori~Темы и цвета~Themen und Farben~Temas e cores
Colors~Couleurs~Colores~Colori~Цвета~Farben~Cores
Wallpaper and opacity~Fond et transparence~Fondo y transparencia~Sfondo e trasparenza~Фон и прозрачность~Hintergrund und Transparenz~Fundo e transparência
Background~Fond~Fondo~Sfondo~Фон~Hintergrund~Fundo
Text~Textes~Textos~Testi~Текст~Texte~Textos
Borders~Contours~Bordes~Bordi~Контуры~Konturen~Contornos
Reflection~Reflet~Reflejo~Riflesso~Блик~Lichtreflex~Reflexo
Preview~Aperçu~Vista previa~Anteprima~Предпросмотр~Vorschau~Pré-visualização
Selection reflection~Reflet de sélection~Reflejo de selección~Riflesso di selezione~Блик выделения~Auswahlreflex~Reflexo de seleção
Static wallpaper~Fond d’écran fixe~Fondo de pantalla fijo~Sfondo fisso~Статичный фон~Statisches Hintergrundbild~Imagem de fundo fixa
No wallpaper~Aucun fond d’écran~Sin fondo de pantalla~Nessuno sfondo~Без фонового изображения~Kein Hintergrundbild~Sem imagem de fundo
Menu backgrounds~Fond des menus~Fondo de los menús~Sfondo dei menu~Фон меню~Menühintergrund~Fundo dos menus
Icon backgrounds~Fond des icônes~Fondo de los iconos~Sfondo delle icone~Фон значков~Symbolhintergrund~Fundo dos ícones
Transparency~Transparence~Transparencia~Trasparenza~Прозрачность~Transparenz~Transparência
Reset~Réinitialiser~Restablecer~Ripristina~Сбросить~Zurücksetzen~Repor
Copy BMP images into the wallpapers folder at the root of the SD card.~Copiez vos images BMP dans le dossier wallpapers à la racine de la carte SD.~Copia imágenes BMP en la carpeta wallpapers de la raíz de la tarjeta SD.~Copia le immagini BMP nella cartella wallpapers alla radice della scheda SD.~Скопируйте изображения BMP в папку wallpapers в корне SD-карты.~BMP-Bilder in den Ordner wallpapers im Stammverzeichnis der SD-Karte kopieren.~Copie imagens BMP para a pasta wallpapers na raiz do cartão SD.
24/32-bit BMP · up to 1920 × 1080 · automatic crop~BMP 24/32 bits · 1920 × 1080 maximum · recadrage automatique~BMP de 24/32 bits · hasta 1920 × 1080 · recorte automático~BMP a 24/32 bit · fino a 1920 × 1080 · ritaglio automatico~BMP 24/32 бит · до 1920 × 1080 · автоматическая обрезка~24/32-Bit-BMP · bis 1920 × 1080 · automatischer Zuschnitt~BMP de 24/32 bits · até 1920 × 1080 · recorte automático
0%: transparent · 100%: opaque. Text and symbols stay readable.~0% : transparent · 100% : opaque. Les textes et symboles restent lisibles.~0%: transparente · 100%: opaco. Los textos y símbolos siguen legibles.~0%: trasparente · 100%: opaco. Testi e simboli restano leggibili.~0%: прозрачно · 100%: непрозрачно. Текст и значки остаются читаемыми.~0%: transparent · 100%: deckend. Texte und Symbole bleiben lesbar.~0%: transparente · 100%: opaco. Textos e símbolos mantêm-se legíveis.
Image missing or invalid. Choose an uncompressed BMP image.~Image introuvable ou invalide. Choisissez une image BMP non compressée.~Imagen ausente o no válida. Elige una imagen BMP sin comprimir.~Immagine assente o non valida. Scegli un’immagine BMP non compressa.~Изображение отсутствует или повреждено. Выберите несжатый BMP.~Bild fehlt oder ist ungültig. Ein unkomprimiertes BMP-Bild auswählen.~Imagem ausente ou inválida. Escolha uma imagem BMP sem compressão.
Independent colors, wallpaper and transparency.~Couleurs indépendantes, fond d’écran et transparence.~Colores independientes, fondo de pantalla y transparencia.~Colori indipendenti, sfondo e trasparenza.~Независимые цвета, фон и прозрачность.~Unabhängige Farben, Hintergrundbild und Transparenz.~Cores independentes, imagem de fundo e transparência.
Light reflection along selected borders.~Reflet léger sur les contours actifs.~Reflejo ligero en los bordes seleccionados.~Riflesso leggero sui bordi selezionati.~Лёгкий блик вдоль выделенных контуров.~Dezenter Lichtreflex an ausgewählten Konturen.~Reflexo suave nos contornos selecionados.
Animated reflection~Reflet animé~Reflejo animado~Riflesso animato~Анимированный блик~Animierter Lichtreflex~Reflexo animado
Static border~Contour fixe~Borde fijo~Bordo fisso~Статичный контур~Feste Kontur~Contorno fixo
Selection~Sélection~Selección~Selezione~Выделение~Auswahl~Seleção'''
catalog={}
for line in rows.splitlines():
    values=line.split('~');assert len(values)==7,line
    catalog[values[0]]=values;catalog[values[1]]=values
(root/'localization_nova_pro.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
p=Path(__file__).resolve().parent/'create_localization_catalog.py';s=p.read_text()
needle="    catalog.update(json.loads(nova.read_text(encoding='utf-8')))"
if "localization_nova_pro.json" not in s:s=s.replace(needle,needle+"\nnova_pro = ROOT / 'localization_nova_pro.json'\nif nova_pro.exists():\n    catalog.update(json.loads(nova_pro.read_text(encoding='utf-8')))")
p.write_text(s)
p=root/'frontend/gui_options.c';s=p.read_text(encoding='utf-8').replace('n_cfg.nova_halo ? "Halo" : "Liseré"','n_cfg.nova_halo ? "Reflet animé" : "Contour fixe"').replace('"Souligner l’icône active."','"Reflet léger sur les contours actifs."').replace('"Contour des icônes", n_cfg.nova_halo','"Sélection", n_cfg.nova_halo');p.write_text(s,encoding='utf-8')
print(f'Authored {len(catalog)} Nova Pro localization keys in seven languages.')
