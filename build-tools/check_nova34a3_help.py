"""Actual LVGL help popup and suspected-fake-card stop, in eight languages."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,os,json
W=Path(__file__).resolve().parent;D=W/'lvgl-preview-nova34a3';F=W/'test-sd-nova34a3/nyx/nyx_gui/fonts/sd'
def run(job):
 kind,lang,size=job;e=os.environ.copy();e.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp',NYX_WIZARD_SYSTEMS='7')
 e['NYX_TEST_WIZARD_HELP' if kind=='help' else 'NYX_TEST_FAKE_SD']='1'
 r=subprocess.run([str(D/'preview.exe'),str(F),str(D/'res.pak'),'wizard',str(lang),str(size),'1'if kind=='help'else'2'],cwd=D,env=e,capture_output=True,text=True,timeout=30)
 return {'case':job,'passed':not r.returncode,'output':(r.stdout+r.stderr)[-1400:]}
jobs=[(kind,lang,size)for kind in ('help','fake')for lang in range(8)for size in (20,30,36)]
with ThreadPoolExecutor(max_workers=4)as pool:results=list(pool.map(run,jobs))
failed=[r for r in results if not r['passed']]
(W/'nova34a3-help-check.json').write_text(json.dumps({'passed':not failed,'cases':len(results),'real_lvgl':True,'hardware_test':False,'coverage':['help popup','Hekate manufacturer-ID 0 suspect flag blocks Next','8 locales, 3 text sizes'],'failures':failed},ensure_ascii=False,indent=2))
for f in failed:print(f)
assert not failed
print('PASS:',len(results),'help and suspect-card-block scenes')
