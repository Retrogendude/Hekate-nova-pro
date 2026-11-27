"""Real LVGL scenes across eight locales and supported font sizes."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor,as_completed
import os,subprocess,json,hashlib,shutil,time
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha4';D=W/'lvgl-preview-nova34a4'
F=W/'test-sd-nova34a4/nyx/nyx_gui/fonts/sd'
for p in (P/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copy2(p,F/p.name)
modes=[('home',0),('carousel',0),('tools',0),('info',0),('options',0),('diagnostics',0),('boot-options',0),('appearance',0),('appearance',1),('appearance',2),('nyx',0),('nyx',1),('nyx',2),('about',0),('about',4),('usb',0),('colors',0),('ram-service',0),('partition-expert',0)]
modes += [('backup',0),('backup',1),('restore',0),('restore',1),('about',3),('colors',3)]
modes += [('setup',0),('boot-selector-view',0)]
jobs=[(mode,lang,size,tab,'')for mode,tab in modes for lang in range(8)for size in (20,30,36)]
jobs += [(mode,7,size,tab,'')for mode,tab in [('home',0),('appearance',1),('boot-options',0),('options',0)]for size in (22,24,26,28,32,34)]
jobs += [(mode,lang,30,0,'Lecture-fixe.nyxf')for mode in ('font-policy','boot-selector')for lang in (1,7)]
jobs += [('home',lang,size,0,'Hekate-Original.nyxf')for lang in (1,4)for size in (20,30,36)]
jobs += [('factory-defaults',1,30,0,'')]
def run(job):
 mode,lang,size,tab,font=job;env=os.environ.copy();env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp')
 if font:env['NYX_PREVIEW_FONT']=font
 r=subprocess.run([str((D/'preview.exe').resolve()),str(F.resolve()),str((D/'res.pak').resolve()),mode,str(lang),str(size),str(tab)],cwd=D,env=env,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=45)
 return {'scene':list(job),'passed':r.returncode==0,'output':(r.stdout+r.stderr)[-4000:]}
results=[];started=time.time()
with ThreadPoolExecutor(max_workers=4)as pool:
 for f in as_completed([pool.submit(run,j)for j in jobs]):
  value=f.result();results.append(value)
  if not value['passed']:print(json.dumps(value,ensure_ascii=False),flush=True)
  if len(results)%50==0:print('Native scenes',len(results),'/',len(jobs),flush=True)
failures=[x for x in results if not x['passed']]
files=['nyx/nyx_gui/fonts/font_manager.c','nyx/nyx_gui/frontend/gui_options.c','nyx/nyx_gui/frontend/gui_appearance.c','nyx/nyx_gui/frontend/gui_nova.c','nyx/nyx_gui/frontend/gui_nova_theme.c','nyx/nyx_gui/localization_catalog.inc']
report={'passed':not failures,'native_scenes':len(results),'failed_scenes':len(failures),'seconds':round(time.time()-started,2),'locales':8,'font_sizes':[20,22,24,26,28,30,32,34,36],'physical_console_test':False,'failures':failures,'source_sha256':{n:hashlib.sha256((P/n).read_bytes()).hexdigest()for n in files}}
(W/'nova34a4-ui-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
assert not failures,str(len(failures))+' native scene failures'
print('PASS:',len(results),'native LVGL scenes including Chinese locking, original fonts and paged boot selection')
