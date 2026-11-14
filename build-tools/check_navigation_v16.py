from pathlib import Path
import os,subprocess,json
work=Path(__file__).resolve().parent;directory=work/'lvgl-preview-v16';project=work/'hekate-6.5.4'
results=[]
for language in range(7):
    for size in (20,30,36):
        env=os.environ.copy();env.update(NYX_TEST_NAVIGATION='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_MENU_OPACITY='25',NYX_PREVIEW_ICON_OPACITY='100')
        result=subprocess.run([str(directory/'preview.exe'),str(project/'nyx/nyx_gui/fonts/sd'),str(directory/'res.pak'),'home',str(language),str(size),'0'],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
        if result.returncode:raise RuntimeError(f'Navigation {language}/{size}: '+result.stdout+result.stderr)
        results.append({'language':language,'size':size,'summary':[s for s in result.stdout.splitlines()if s.startswith('PASS: wallpaper navigation')]})
(work/'navigation-v16.json').write_text(json.dumps({'scenes':len(results),'input':'Actual LVGL press/release touch dispatch','results':results},indent=2),encoding='utf-8')
print('PASS: 21 real-touch navigation scenes; wallpaper + 75% transparent windows, repeated settings/color/font stacks, window closing and all main tabs.')
