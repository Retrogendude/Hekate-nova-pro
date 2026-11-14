"""Convert a personal image to a Nyx BMP: python convert_wallpaper.py image.png [output.bmp]."""
from pathlib import Path
import argparse
import unicodedata
from PIL import Image,ImageOps
parser=argparse.ArgumentParser(description='Convertir une image en fond BMP Nova Pro (1280 x 720).')
parser.add_argument('source',type=Path)
parser.add_argument('destination',type=Path,nargs='?')
args=parser.parse_args()
name=unicodedata.normalize('NFKD',args.source.stem).encode('ascii','ignore').decode('ascii')
name=''.join(c for c in name if c.isalnum() or c in ' _-').strip()[:54] or 'Fond'
target=args.destination or args.source.with_name(name+'-nova.bmp')
if target.resolve()==args.source.resolve():parser.error('Le fichier de sortie doit être différent de la source.')
with Image.open(args.source)as original:
    image=ImageOps.fit(ImageOps.exif_transpose(original).convert('RGB'),(1280,720),method=Image.Resampling.LANCZOS)
    image.save(target,format='BMP')
print(target.resolve())
