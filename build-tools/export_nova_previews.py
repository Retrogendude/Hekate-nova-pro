"""Export scenes from the verified native LVGL renderer, with hardware stubs."""
from pathlib import Path
import os
import subprocess
from PIL import Image

work = Path(__file__).resolve().parent
nyx = work/'hekate-6.5.4/nyx/nyx_gui'
renderer = work/'lvgl-preview-v10'
destination = work/'nova-preview-export'
destination.mkdir(exist_ok=True)
outputs = work.parent/'outputs'
jobs = [('tools',1,20,0),('tools',4,36,0),('info',1,20,0),
        ('options',1,20,0),('options',4,36,0),('diagnostics',4,36,0),
        ('appearance',1,30,0),('appearance',1,30,1),('appearance',1,30,3),
        ('nyx',4,36,0),('nyx',4,36,1),('colors',1,30,0),
        ('home',1,30,0),('carousel',4,36,0),
        ('tools',1,30,0),('info',1,30,0),('options',1,30,0),('diagnostics',1,30,0),('carousel',1,30,0),('boot-options',1,30,0),('quick-actions',1,30,0)]
names = {'home':'accueil','tools':'outils','info':'informations','options':'reglages',
         'diagnostics':'diagnostic','carousel':'carrousel'}
for screen,language,size,tab in jobs:
    environment=os.environ.copy()
    environment.pop('NYX_PREVIEW_NO_DUMP',None);environment.pop('NYX_TRACE',None)
    environment['NYX_PREVIEW_FONT']='Lecture.nyxf'
    if screen=='info':environment['NYX_PREVIEW_LOCKPICK']='1'
    else:environment.pop('NYX_PREVIEW_LOCKPICK',None)
    result = subprocess.run([str(renderer/'preview.exe'),str(nyx/'fonts/sd'),
        str(renderer/'res.pak'),screen,str(language),str(size),str(tab)],
        cwd=destination,env=environment,capture_output=True,text=True,check=True,timeout=20)
    scene = Image.open(destination/'preview.ppm')
    scene.save(work/f'preview-v10-{screen}-{language}-{size}-{tab}.png')
    if language==1 and size==30 and screen in names:
        scene.save(outputs/f'apercu-nova-{names[screen]}-v10.png')
print(f'Exported {len(jobs)} actual-LVGL scenes and six Nova previews.')
