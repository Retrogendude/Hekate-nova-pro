"""Real partition controls, layout and integer labels; storage I/O is substituted."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,os,json,hashlib,shutil,time
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32';d=w/'lvgl-preview-nova32';font=w/'test-sd-nova32/nyx/nyx_gui/fonts/sd'
font.mkdir(parents=True,exist_ok=True)
for f in (w/'test-sd-nova30/nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copyfile(f,font/f.name)
jobs=[(mode,model,lang,size,face,128)for mode in ['partition-simple','partition-expert','partition-emmc']for model in range(4)for lang in range(7)for size in [20,36]for face in ['Lecture.nyxf','Lecture-fixe.nyxf','Caviar.nyxf']]
jobs +=[(mode,model,1,30,'Lecture.nyxf',capacity)for mode in ['partition-expert','partition-emmc']for model in range(4)for capacity in [16,32,64,256,955,1900]]
jobs +=[('partition-expert',model,lang,size,face,955)for model in range(4)for lang in range(7)for size in [20,36]for face in ['Lecture.nyxf','Lecture-fixe.nyxf','Caviar.nyxf']]
wallpapers=w/'test-sd-nova32/wallpapers';wallpapers.mkdir(parents=True,exist_ok=True);shutil.copyfile(p/'wallpapers/Nova-Carbon.bmp',wallpapers/'Nova-Carbon.bmp')
def run(job):
 mode,model,lang,size,face,capacity=job;env=os.environ.copy();env.update(NYX_TEST_MODEL=str(model),NYX_TEST_PARTITION_ACTIONS='1',NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT=face,NYX_TEST_CAPACITY=str(capacity),NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='25')
 r=subprocess.run([str(d/'preview.exe'),str(font),str(d/'res.pak'),mode,str(lang),str(size),'0'],cwd=d,env=env,capture_output=True,text=True,timeout=30)
 return None if r.returncode==0 else {'job':job,'exit':r.returncode,'output':r.stdout+r.stderr}
start=time.time()
with ThreadPoolExecutor(max_workers=6)as pool:failed=[r for r in pool.map(run,jobs)if r]
report={'passed':not failed,'native_scenes':len(jobs),'failures':failed,'languages':7,'fonts':3,'sizes':[20,36],'controls':'real callbacks: all 45 emu stops, Linux/Android minima and 1-GB +/-; stock 29/58 Full and double Full; capacity clamp through 1900 GB; 955-GB user layout; same-row summary centers; +/- knob/value separation; model mismatch, modes and bounds','storage_io':'substituted; no partition, format, flash or card writes executed','elapsed_seconds':round(time.time()-start,1),'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['nyx/nyx_gui/frontend/gui_partition_nova.inc','nyx/nyx_gui/frontend/gui_tools_partition_manager.c','nyx/nyx_gui/localization_catalog.inc']}}
(w/'nova32-partition-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
assert not failed,json.dumps(failed[:4],ensure_ascii=False)
print('PASS',len(jobs),'native partition scenes: full-width geometry, no scroll, real +/- and stock sizes; no storage operations')
