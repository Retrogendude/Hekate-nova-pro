"""Export the final, actual native UI and its actual moving reflection."""
from pathlib import Path
from PIL import Image
import os,subprocess
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';renderer=work/'lvgl-preview-v11';outputs=work.parent/'outputs'
destination=work/'nova-pro-preview-export';destination.mkdir(exist_ok=True)
modes=[('about',0),('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('appearance',0),('appearance',1),('appearance',2),('nyx',0),('nyx',1),('colors',0),('colors',1),('boot-options',0),('quick-actions',0),('backup',0),('restore',0),('usb',0),('emummc',0),('diagnostic-results',0),('ram-service',0)]+[('diagnostic-group',i)for i in range(1,11)]
names={'home':'accueil','carousel':'carrousel','tools':'outils','info':'informations','options':'reglages','diagnostics':'diagnostic','about':'credits','backup':'sauvegarde','usb':'usb','emummc':'emummc','diagnostic-results':'resultats','colors':'couleurs'}
def render(screen,tab,extra=None):
    env=os.environ.copy()
    for key in list(env):
        if key.startswith(('NYX_PREVIEW_','NYX_RGB_','NYX_CAPTURE_'))or key=='NYX_TRACE':env.pop(key)
    env['NYX_PREVIEW_FONT']='Lecture.nyxf'
    if screen=='info':env['NYX_PREVIEW_LOCKPICK']='1'
    if extra:env.update(extra)
    result=subprocess.run([str(renderer/'preview.exe'),str(project/'nyx/nyx_gui/fonts/sd'),str(renderer/'res.pak'),screen,'1','30',str(tab)],cwd=destination,env=env,capture_output=True,text=True,timeout=40)
    if result.returncode:raise RuntimeError(result.stdout+result.stderr)
    return Image.open(destination/'preview.ppm').convert('RGB')
for screen,tab in modes:
    image=render(screen,tab);image.save(destination/f'preview-v11-{screen}-1-30-{tab}.png')
    if screen in names and tab==0:image.save(outputs/f'apercu-nova-pro-{names[screen]}-v11.png')
    if screen=='colors'and tab==1:image.save(outputs/'apercu-nova-pro-fond-et-transparence-v11.png')
image=render('home',0,{'NYX_PREVIEW_WALLPAPER':'Nova-Carbon.bmp','NYX_PREVIEW_MENU_OPACITY':'50','NYX_PREVIEW_ICON_OPACITY':'25'})
image.save(outputs/'apercu-nova-pro-fond-ecran-v11.png');image.save(destination/'preview-v11-wallpaper.png')
image=render('home',0,{'NYX_RGB_BACKGROUND':'080B11','NYX_RGB_TEXT':'E2E6EE','NYX_RGB_BORDER':'B3BCCB','NYX_RGB_REFLECTION':'FFAE4D'})
image.save(outputs/'apercu-nova-pro-palette-v11.png');image.save(destination/'preview-v11-palette.png')
render('home',0,{'NYX_CAPTURE_REFLECTION':'1'})
raw=destination/'reflection.bgrx';stride=1280*720*4;data=raw.read_bytes();assert len(data)==72*stride
first=Image.frombytes('RGB',(1280,720),data[:stride],'raw','BGRX');palette=first.quantize(colors=256)
frames=[]
for i in range(72):frames.append(Image.frombytes('RGB',(1280,720),data[i*stride:(i+1)*stride],'raw','BGRX').quantize(palette=palette,dither=Image.Dither.NONE))
frames[0].save(outputs/'reflet-nova-pro-v11.gif',save_all=True,append_images=frames[1:],duration=80,loop=0,optimize=True)
assert raw.resolve().parent==destination.resolve();raw.unlink()
print(f'Exported {len(modes)} final native views, wallpaper/palette examples and the actual 72-frame selection reflection.')
