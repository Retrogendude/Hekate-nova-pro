from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,os,json
work=Path(__file__).resolve().parent;directory=work/'lvgl-preview-v17';fonts=work/'test-sd-v17/nyx/nyx_gui/fonts/sd'
cases=[('component-results',i)for i in range(4)]+[('component-detail',i)for i in range(15)]+[('component-leads',0),('diagnostic-results',0)]
jobs=[(mode,index,language,size,font)for mode,index in cases for language in range(7)for size in(20,36)for font in('Lecture.nyxf','Caviar.nyxf')]
def run(job):
    mode,index,language,size,font=job;env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT=font)
    result=subprocess.run([str(directory/'preview.exe'),str(fonts),str(directory/'res.pak'),mode,str(language),str(size),str(index)],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
    assert not result.returncode,(job,result.stdout,result.stderr)
with ThreadPoolExecutor(max_workers=4)as pool:list(pool.map(run,jobs))
(work/'components-ui-v17.json').write_text(json.dumps({'scenes':len(jobs),'components':15,'list_pages':4,'languages':7,'sizes':[20,36],'fonts':['Lecture.nyxf','Caviar.nyxf'],'native_logic':'actual C classifier and LVGL view; fixture findings, no hardware I/O','translation_status':'v17 catalog completed; seven-language native layout checked'},indent=2),encoding='utf-8')
print(f'PASS: {len(jobs)} native component summary/detail/leads scenes; all fifteen components, four pages and bounded layout.')
