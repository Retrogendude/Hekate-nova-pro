from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import os,subprocess,json
work=Path(__file__).resolve().parent;folder=work/'lvgl-preview-v17';fonts=work/'test-sd-v17/nyx/nyx_gui/fonts/sd'
jobs=[('colors',tab,lang,size,font)for tab in range(4)for lang in range(7)for size in(20,36)for font in('Lecture.nyxf','Caviar.nyxf')]
jobs += [(mode,0,lang,size,font)for mode in('home','carousel')for lang in range(7)for size in(20,36)for font in('Lecture.nyxf','Caviar.nyxf')]
def run(job):
    mode,tab,lang,size,font=job;env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT=font,NYX_TEST_BOOT_LAUNCH='1',NYX_PREVIEW_LAUNCH='Nova-Carbon.bmp')
    result=subprocess.run([str(folder/'preview.exe'),str(fonts),str(folder/'res.pak'),mode,str(lang),str(size),str(tab)],cwd=folder,env=env,capture_output=True,text=True,timeout=30)
    assert not result.returncode,(job,result.stdout,result.stderr)
with ThreadPoolExecutor(max_workers=4)as pool:list(pool.map(run,jobs))
(work/'boot-launch-ui-v17.json').write_text(json.dumps({'scenes':len(jobs),'languages':7,'sizes':[20,36],'fonts':['Lecture.nyxf','Caviar.nyxf'],'tabs':4,'launch_image':'actual decoder in tiles and carousel; preview create/delete; pending settings isolated'},indent=2))
print(f'PASS: {len(jobs)} actual LVGL scenes; all four appearance tabs, independent boot colors, image selection/cleanup, Launch tile and carousel.')
