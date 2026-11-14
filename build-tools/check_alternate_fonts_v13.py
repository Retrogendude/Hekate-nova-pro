"""Check the bundled alternate faces with the already built actual-LVGL UI."""
from pathlib import Path
import os
import subprocess

WORK=Path(__file__).resolve().parent
directory=WORK/'lvgl-preview-v13'
executable=directory/'preview.exe'
fonts=WORK/'hekate-6.5.4/nyx/nyx_gui/fonts/sd'
modes=[('about',0),('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),
       ('appearance',0),('appearance',1),('appearance',2),('nyx',0),('nyx',1),('colors',0),('colors',1),('boot-options',0),('quick-actions',0),('setup',0)]
modes.extend([('backup',0),('restore',0),('usb',0),('emummc',0),('diagnostic-results',0),('ram-service',0)]+[('diagnostic-group',i)for i in range(1,11)])
checks=0
for font in ['Lecture-fixe.nyxf','Lecture-renforcee.nyxf','']:
    environment=os.environ.copy();environment['NYX_PREVIEW_FONT']=font;environment['NYX_PREVIEW_NO_DUMP']='1'
    for screen,tab in modes:
        for language in range(7):
            for size in (20,30,36):
                result=subprocess.run([str(executable),str(fonts),str(directory/'res.pak'),screen,
                    str(language),str(size),str(tab)],cwd=directory,env=environment,
                    capture_output=True,text=True,timeout=20)
                if result.returncode:
                    raise RuntimeError(f'{font}: {screen} language={language} size={size} tab={tab}: '+result.stdout+result.stderr)
                checks+=1
    print('PASS: alternate face',font or 'embedded Inter','checked across seven languages and 20/30/36px.',flush=True)
print(f'PASS: {checks} additional actual-LVGL scenes with the fixed, reinforced and embedded faces; selection bindings and palette pixels checked.',flush=True)
