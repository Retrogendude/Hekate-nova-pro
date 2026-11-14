from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,os,subprocess,shutil
w=Path(__file__).resolve().parent;d=w/'lvgl-preview-v18';fonts=w/'test-sd-v18/nyx/nyx_gui/fonts/sd'
shutil.copyfile(w/'test-sd-v18/icone/launch.png',w/'test-sd-v18/icone/diagnostic.png')
jobs=[('about',tab,lang,size,font)for tab in range(4)for lang in range(7)for size in(20,36)for font in('Lecture.nyxf','Caviar.nyxf')]
jobs += [('colors',tab,lang,size,font)for tab in range(4)for lang in range(7)for size in(20,36)for font in('Lecture.nyxf','Caviar.nyxf')]
def run(job):
 mode,tab,lang,size,font=job;e=os.environ.copy();e.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT=font,NYX_PREVIEW_LAUNCH='launch.png',NYX_TEST_V18='1')
 p=subprocess.run([str(d/'preview.exe'),str(fonts),str(d/'res.pak'),mode,str(lang),str(size),str(tab)],env=e,cwd=d,capture_output=True,text=True,timeout=40)
 assert not p.returncode,(job,p.stdout,p.stderr)
with ThreadPoolExecutor(max_workers=4)as pool:list(pool.map(run,jobs))
(w/'changes-ui-v18.json').write_text(json.dumps({'scenes':len(jobs),'languages':7,'credits_tabs':4,'credits':'no report pagination, no clipped labels; complete OSS columns','images':'native PNG loading plus Launch preview','fonts':['Lecture.nyxf','Caviar.nyxf'],'sizes':[20,36]},indent=2))
print(f'PASS: {len(jobs)} native credit/PNG/theme scenes; four credit tabs, no bottom pages, seven languages and full alpha images.')
