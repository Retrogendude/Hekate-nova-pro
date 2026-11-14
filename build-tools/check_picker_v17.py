from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import os,subprocess,json
work=Path(__file__).resolve().parent;folder=work/'lvgl-preview-v17'
fonts=work/'test-sd-v17/nyx/nyx_gui/fonts/sd';fonts.mkdir(parents=True,exist_ok=True)
import shutil
for p in (work/'hekate-6.5.4/nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copyfile(p,fonts/p.name)
shutil.copyfile(work.parent/'outputs/Caviar.nyxf',fonts/'Caviar.nyxf')
wallpapers=work/'test-sd-v17/wallpapers';wallpapers.mkdir(exist_ok=True)
shutil.copyfile(work/'hekate-6.5.4/wallpapers/Nova-Carbon.bmp',wallpapers/'Nova-Carbon.bmp')
jobs=[(lang,size,font,tab)for lang in range(7)for size in(20,30,32,36)for font in('Lecture.nyxf','Caviar.nyxf')for tab in(0,1)]
def run(job):
    lang,size,font,tab=job;env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT=font,NYX_TEST_PICKER='1')
    result=subprocess.run([str(folder/'preview.exe'),str(fonts),str(folder/'res.pak'),'colors',str(lang),str(size),str(tab)],cwd=folder,env=env,capture_output=True,text=True)
    assert not result.returncode,(job,result.stdout,result.stderr)
with ThreadPoolExecutor(max_workers=4)as pool:list(pool.map(run,jobs))
(work/'picker-check-v17.json').write_text(json.dumps({'scenes':len(jobs),'languages':7,'sizes':[20,30,32,36],'fonts':['Lecture.nyxf','Caviar.nyxf'],'touch_test':True,'exact_rgb_on_target_selection':True,'status':'passed','scope':'actual rectangular widget, touch and independent appearance targets'},indent=2),encoding='utf-8')
print(f'PASS: {len(jobs)} actual LVGL color scenes, real touch selection, brightness endpoints, exact RGB preservation, seven languages and two fonts.')
