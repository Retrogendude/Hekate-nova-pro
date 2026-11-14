"""Exercise static wallpaper and both surface opacity controls in native LVGL."""
from pathlib import Path
import os,subprocess
work=Path(__file__).resolve().parent;directory=work/'lvgl-preview-v12';exe=directory/'preview.exe'
fonts=work/'hekate-6.5.4/nyx/nyx_gui/fonts/sd'
count=0
for screen,tab in [('home',0),('tools',0),('options',0),('backup',0),('colors',0),('colors',1)]:
    for language in (0,1,4):
        for size in (20,30,36):
            for menu,icon in [(100,100),(50,25),(0,0)]:
                env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY=str(menu),NYX_PREVIEW_ICON_OPACITY=str(icon))
                r=subprocess.run([str(exe),str(fonts),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],env=env,cwd=directory,capture_output=True,text=True,timeout=20)
                if r.returncode:raise RuntimeError(f'{screen}/{tab} language={language} size={size} opacity={menu}/{icon}: '+r.stdout+r.stderr)
                count+=1
for screen,tab in [('home',0),('tools',0),('options',0),('backup',0),('colors',0),('colors',1)]:
    for language in (0,1,4):
        for size in (20,30,36):
            env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_RGB_BACKGROUND='080B11',NYX_RGB_TEXT='E2E6EE',NYX_RGB_BORDER='B3BCCB',NYX_RGB_REFLECTION='FFAE4D',NYX_RGB_LOGO='D49AF1',NYX_RGB_ICONS='FFAE4D',NYX_RGB_SELECTION='302846',NYX_PREVIEW_MENU_OPACITY='75',NYX_PREVIEW_ICON_OPACITY='50')
            env.pop('NYX_PREVIEW_WALLPAPER',None)
            r=subprocess.run([str(exe),str(fonts),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],env=env,cwd=directory,capture_output=True,text=True,timeout=20)
            if r.returncode:raise RuntimeError(f'Custom palette {screen}/{tab} language={language} size={size}: '+r.stdout+r.stderr)
            count+=1
print(f'PASS: {count} additional native-LVGL wallpaper/opacity/palette scenes; opaque/partial/transparent menu and icon backgrounds, independent colors, three languages, three sizes, actual BMP and original glyphs.',flush=True)
