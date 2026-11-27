"""Exercise real LVGL pointer selection and active-outline pixels, no storage I/O."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json,os,subprocess,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.2';D=W/'lvgl-preview-nova33_2';F=W/'test-sd-nova33_2/nyx/nyx_gui/fonts/sd'
jobs=[(s,l,z,t)for s in ['home','emummc','backup','restore','usb']for l in range(8)for z in [20,30,36]for t in ([0,1]if s in ['backup','restore','usb']else[0])]
def run(job):
 s,l,z,t=job;env=os.environ.copy();env.update(NYX_TEST_SELECTION='1',NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp')
 r=subprocess.run([str(D/'preview.exe'),str(F),str(D/'res.pak'),s,str(l),str(z),str(t)],cwd=D,env=env,capture_output=True,text=True,timeout=45)
 return {'case':job,'passed':r.returncode==0,'output':(r.stdout+r.stderr)[-1800:]}
with ThreadPoolExecutor(max_workers=4)as pool:results=list(pool.map(run,jobs))
failed=[x for x in results if not x['passed']]
files=['frontend/gui_nova.c','frontend/gui_nova_reflection.c','frontend/gui_nova.h','frontend/gui_tools.c','frontend/gui_emmc_tools.c','frontend/gui.c']
report={'passed':not failed,'cases':len(results),'locales':8,'sizes':[20,30,36],'coverage':['pointer hover','unchanged click actions','disabled cards skipped and deselected','cleanup','complete active bottom border pixels','animations on and off'],'physical_console_test':False,'failures':failed,'source_sha256':{n:hashlib.sha256((P/'nyx/nyx_gui'/n).read_bytes()).hexdigest()for n in files}}
(W/'nova33_2-selection-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
for r in failed[:3]:print(r)
assert not failed
print('PASS:',len(results),'actual-LVGL selection/active-outline scenes')
