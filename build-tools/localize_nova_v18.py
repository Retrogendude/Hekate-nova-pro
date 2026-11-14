from pathlib import Path
import json
r=Path(__file__).parent/'hekate-6.5.4/nyx/nyx_gui';c={}
def add(fr,en,es,it,ru,de,pt):c[fr]=[en,fr,es,it,ru,de,pt]
add('Minimum','Minimum','Mínimo','Minimo','Минимум','Minimum','Mínimo')
add('Recommandé','Recommended','Recomendado','Consigliato','Рекомендуется','Empfohlen','Recomendado')
add('Capacité : %d MiB (%d,%02d Go) · réserves : %d MiB','Capacity: %d MiB (%d.%02d GiB) · reserved: %d MiB','Capacidad: %d MiB (%d,%02d GiB) · reserva: %d MiB','Capacità: %d MiB (%d,%02d GiB) · riserva: %d MiB','Ёмкость: %d МиБ (%d,%02d ГиБ) · резерв: %d МиБ','Kapazität: %d MiB (%d,%02d GiB) · Reserve: %d MiB','Capacidade: %d MiB (%d,%02d GiB) · reserva: %d MiB')
add('%d,%02d Go','%d.%02d GiB','%d,%02d GiB','%d,%02d GiB','%d,%02d ГиБ','%d,%02d GiB','%d,%02d GiB')
add('Choisissez un PNG transparent du dossier icone. L’image couvre uniquement la tuile Launch. Les anciens BMP restent disponibles.','Choose a transparent PNG from icone. It covers only the Launch tile. Existing BMP images remain available.','Elija un PNG transparente de icone. Solo cubre la tarjeta de inicio. Los BMP anteriores siguen disponibles.','Scegli un PNG trasparente da icone. Copre solo il riquadro di avvio. I vecchi BMP restano disponibili.','Выберите прозрачный PNG из icone. Он покроет только плитку запуска. Прежние BMP остаются доступны.','Transparentes PNG aus icone wählen. Es füllt nur die Startkachel. Bisherige BMP-Bilder bleiben verfügbar.','Escolha um PNG transparente de icone. Cobre apenas o mosaico de início. Os BMP anteriores continuam disponíveis.')
add('Image introuvable ou invalide. Utilisez un PNG dans icone ou un BMP non compressé dans wallpapers.','Image missing or invalid. Use a PNG in icone or an uncompressed BMP in wallpapers.','Imagen inexistente o inválida. Use PNG en icone o BMP sin comprimir en wallpapers.','Immagine mancante o non valida. Usa PNG in icone o BMP non compresso in wallpapers.','Изображение отсутствует или некорректно. Используйте PNG в icone или несжатый BMP в wallpapers.','Bild fehlt oder ist ungültig. PNG in icone oder unkomprimiertes BMP in wallpapers verwenden.','Imagem ausente ou inválida. Use PNG em icone ou BMP sem compressão em wallpapers.')
(r/'localization_nova_v18.json').write_text(json.dumps(c,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
p=Path(__file__).parent/'create_localization_catalog.py';s=p.read_text();marker='# Preserve literal formatting/color controls around fully authored phrases.'
if 'nova_v18 =' not in s:s=s.replace(marker,"nova_v18 = ROOT / 'localization_nova_v18.json'\nif nova_v18.exists():\n    catalog.update(json.loads(nova_v18.read_text(encoding='utf-8')))\n\n"+marker)
p.write_text(s,encoding='utf-8')
print('v18 labels translated in seven languages')
