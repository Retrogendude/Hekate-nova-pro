"""Original lightweight static Nova wallpaper, supplied as a ready-to-use BMP."""
from pathlib import Path
from PIL import Image,ImageDraw
import math
work=Path(__file__).resolve().parent
folder=work/'hekate-6.5.4/wallpapers';folder.mkdir(exist_ok=True)
image=Image.new('RGB',(1280,720));pixels=image.load()
for y in range(720):
    for x in range(1280):
        glow=max(0,1-math.hypot((x-1070)/850,(y-220)/650))**2
        pixels[x,y]=(int(11+glow*5),int(16+glow*22),int(24+glow*34))
draw=ImageDraw.Draw(image)
for x in range(-720,1560,120):draw.line((x,720,x+720,0),fill=(22,33,47),width=1)
for width,shade in [(1,(35,63,80)),(1,(26,44,60))]:
    draw.polygon([(820,155),(1045,25),(1270,155),(1270,415),(1045,545),(820,415)],outline=shade,width=width)
draw.line([(880,365),(880,214),(1150,365),(1150,214)],fill=(31,66,84),width=3)
image.save(folder/'Nova-Carbon.bmp')
image.save(work/'nova-wallpaper.png')
(folder/'LIRE-MOI.txt').write_text('FONDS D ECRAN NOVA PRO\n\nCopier les fichiers BMP dans wallpapers a la racine de la carte SD.\nChoisir ensuite Reglages Nyx > Themes et couleurs > Fond et transparence.\n\nFormats : BMP non compresse, 24 ou 32 bits, jusqu a 1920 x 1080.\nLes images sont recadrees et mises a l echelle une seule fois au chargement.\nUtiliser un nom simple (lettres sans accents, chiffres, espaces, - ou _).\nJusqu a 32 fonds sont presentes dans le selecteur. Aucun fond anime.\n\nNova-Carbon est une creation originale incluse dans ce projet sous GPL-2.0-or-later.\n',encoding='utf-8')
print('Created the original static Nova-Carbon wallpaper and SD folder instructions.')
