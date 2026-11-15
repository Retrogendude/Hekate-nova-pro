from pathlib import Path
import os,subprocess
from PIL import Image,ImageDraw
w=Path(__file__).resolve().parent;d=w/'lvgl-preview-nova33_1';f=w/'test-sd-nova33_1/nyx/nyx_gui/fonts/sd'
out=w.parent/'outputs/preview-nova33.1';out.mkdir(exist_ok=True)
env=os.environ.copy();env['NYX_PREVIEW_WALLPAPER']='Nova-Carbon-Cyan.bmp';env.pop('NYX_PREVIEW_NO_DUMP',None)
jobs=[('accueil','home',1,0),('options','options',1,0),('credits','about',1,3),('sauvegarde-emummc','backup',1,1),('restauration-emummc','restore',1,1),('chinois','home',7,0),('changelog','about',1,4)]
for name,scene,lang,tab in jobs:
 r=subprocess.run([str(d/'preview.exe'),str(f),str(d/'res.pak'),scene,str(lang),'30',str(tab)],cwd=d,env=env,capture_output=True,text=True,timeout=45)
 assert r.returncode==0,r.stderr
 Image.open(d/'preview.ppm').save(out/(name+'.png'))
canvas=Image.new('RGB',(1280,760),'#10151d');draw=ImageDraw.Draw(canvas)
for i,name in enumerate(('accueil','options','credits','sauvegarde-emummc')):
 im=Image.open(out/(name+'.png'));im.thumbnail((632,355));x=(i%2)*640+4;y=(i//2)*380+22
 canvas.paste(im,(x,y));draw.text((x+8,y-18),name,fill='#55c9f1')
canvas.save(out/'apercu-v33.1.png')
print('Saved seven real-code host previews and overview')
