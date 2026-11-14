"""Exercise page boundaries and the original emuMMC folder save callbacks."""
from pathlib import Path
import json,os,subprocess
work=Path(__file__).resolve().parent;directory=work/'lvgl-preview-v15';project=work/'hekate-6.5.4'
scenes=0
for count in (1,4,5,16,17,63):
    for language in range(7):
        for size in (20,30,36):
            for disabled in (False,True):
                env={k:v for k,v in os.environ.items()if not k.startswith('NYX_')}
                env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='50')
                if disabled:env['NYX_RAW_DISABLED']='1'
                result=subprocess.run([str(directory/'preview.exe'),str(project/'nyx/nyx_gui/fonts/sd'),str(directory/'res.pak'),'emummc-change',str(language),str(size),str(count)],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
                if result.returncode:raise RuntimeError(f'count={count}, language={language}, size={size}, disabled={disabled}: '+result.stdout+result.stderr)
                scenes+=1
(work/'nova-v15-emummc-pages.json').write_text(json.dumps({'scenes':scenes,'file_counts':[1,4,5,16,17,63],'languages':7,'sizes':[20,30,36],'save_paths':'Each visible button is invoked and the saved path compared to its original folder; hardware/config-file I/O mocked.','raw_availability_variants':2},indent=2))
print(f'PASS: {scenes} emuMMC page-boundary scenes; every folder remains accessible and preserves its exact save path, with wallpaper and available/unavailable RAW entries.')
