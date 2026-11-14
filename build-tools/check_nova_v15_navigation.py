"""Exercise real LVGL touch dispatch through stacked wallpaper windows."""
from pathlib import Path
import os,subprocess,json
work=Path(__file__).resolve().parent;directory=work/'lvgl-preview-v15';project=work/'hekate-6.5.4'
results=[]
for language in range(7):
 for size in (20,30,36):
  env=os.environ.copy();env.update(NYX_TEST_NAVIGATION='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_MENU_OPACITY='50',NYX_PREVIEW_ICON_OPACITY='25')
  result=subprocess.run([str(directory/'preview.exe'),str(project/'nyx/nyx_gui/fonts/sd'),str(directory/'res.pak'),'home',str(language),str(size),'0'],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
  if result.returncode:raise RuntimeError(f'Navigation lang={language} size={size}: '+result.stdout+result.stderr)
  results.append({'language':language,'size':size,'summary':[s for s in result.stdout.splitlines()if s.startswith('PASS: wallpaper navigation')]})
(work/'nova-v15-navigation.json').write_text(json.dumps({'scenes':len(results),'driver':'Actual LVGL pointer input, press/release; no direct callback invocation for navigation.','results':results},indent=2))
print(f'PASS: {len(results)} wallpaper navigation scenes through actual touch dispatch; each repeats Nyx/color/font window stacks three times and visits every main tab, in seven languages at 20/30/36px.')
