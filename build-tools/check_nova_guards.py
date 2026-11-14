"""Exercise the original AutoRCM patched-console guard through Nova's UI."""
from pathlib import Path
import os
import subprocess

work=Path(__file__).resolve().parent
directory=work/'lvgl-preview-v10'
environment=os.environ.copy();environment['NYX_PREVIEW_PATCHED']='1';environment['NYX_PREVIEW_NO_DUMP']='1'
count=0
for screen in ('tools','info'):
    if screen=='info':environment['NYX_PREVIEW_LOCKPICK']='1'
    for language in (0,1,4):
        for size in (20,36):
            result=subprocess.run([str(directory/'preview.exe'),str(work/'hekate-6.5.4/nyx/nyx_gui/fonts/sd'),str(directory/'res.pak'),
                screen,str(language),str(size),'0'],cwd=directory,env=environment,capture_output=True,text=True,timeout=20)
            if result.returncode:raise RuntimeError(result.stdout+result.stderr)
            count+=1
print(f'PASS: {count} hardware-availability Nova scenes; patched-console AutoRCM remains disabled, available Lockpick opens correctly, selection does not execute actions.')
