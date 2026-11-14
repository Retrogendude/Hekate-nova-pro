"""Compare unchanged Nova views to v21, excluding the updated version footer."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from PIL import Image,ImageChops
import subprocess,os,json
w=Path(__file__).resolve().parent;d=w/'nova22-visual-tests';d.mkdir(exist_ok=True)
font=w/'test-sd-nova22/nyx/nyx_gui/fonts/sd'
env=os.environ.copy();env.pop('NYX_PREVIEW_NO_DUMP',None)
env.update(NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='25')
jobs=[(screen,tab,lang)for screen,tab in [('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('nyx',2)]for lang in range(7)]
def check(job):
    screen,tab,lang=job;images=[]
    for version,dirname in [('old','lvgl-preview-nova-bt'),('new','lvgl-preview-nova22')]:
        folder=d/f'{screen}-{tab}-{lang}-{version}';folder.mkdir(exist_ok=True);engine=w/dirname
        r=subprocess.run([str(engine/'preview.exe'),str(font),str(engine/'res.pak'),screen,str(lang),'30',str(tab)],cwd=folder,env=env,capture_output=True,text=True,timeout=30)
        assert r.returncode==0,(job,version,r.stderr)
        f=folder/'preview.ppm'
        with Image.open(f)as im:images.append(im.crop((0,0,1280,656)).convert('RGB'))
        f.unlink()
    box=ImageChops.difference(images[0],images[1]).getbbox()
    return {'view':job,'pixel_difference':box}
with ThreadPoolExecutor(max_workers=5)as pool:results=list(pool.map(check,jobs))
assert not any(v['pixel_difference']for v in results),[v for v in results if v['pixel_difference']]
(w/'nova22-visual-check.json').write_text(json.dumps({'identical_views':len(results),'compared_to':'v21 Nova','excluded':'footer containing the changed Hekate version','results':results},indent=2))
print(f'PASS: {len(results)} unchanged Nova views match v21 pixel for pixel above the version footer.')
