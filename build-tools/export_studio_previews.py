"""Export scenes from the verified native LVGL renderer, with hardware stubs."""
from pathlib import Path
import subprocess
from PIL import Image

work = Path(__file__).resolve().parent
nyx = work/'hekate-6.5.4/nyx/nyx_gui'
renderer = work/'lvgl-preview-v9'
destination = work/'studio-preview-export'
destination.mkdir(exist_ok=True)
outputs = work.parent/'outputs'
jobs = [('tools',1,20,0),('tools',4,36,0),('info',1,20,0),
        ('options',1,20,0),('options',4,36,0),('diagnostics',4,36,0),
        ('appearance',1,30,0),('appearance',1,30,1),('appearance',1,30,3),
        ('nyx',4,36,0),('nyx',4,36,1),('colors',1,30,0),
        ('home',1,30,0),('carousel',4,36,0),
        ('tools',1,30,0),('options',1,30,0),('diagnostics',1,30,0),('carousel',1,30,0)]
names = {'home':'accueil','tools':'outils','options':'options',
         'diagnostics':'diagnostic','carousel':'carrousel'}
for screen,language,size,tab in jobs:
    result = subprocess.run([str(renderer/'preview.exe'),str(nyx/'fonts/sd'),
        str(renderer/'res.pak'),screen,str(language),str(size),str(tab)],
        cwd=destination,capture_output=True,text=True,check=True,timeout=20)
    scene = Image.open(destination/'preview.ppm')
    scene.save(work/f'preview-v9-{screen}-{language}-{size}-{tab}.png')
    if language==1 and size==30 and screen in names:
        scene.save(outputs/f'apercu-studio-{names[screen]}-v9.png')
print(f'Exported {len(jobs)} actual-LVGL scenes and five Studio previews.')
