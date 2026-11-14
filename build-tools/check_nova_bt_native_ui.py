"""Bounded native LVGL scenes, real fonts, translations and interactions."""
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import json,os,subprocess,shutil,time,hashlib
work=Path(__file__).resolve().parent;project=work/'hekate-nova-pro-v21-bt';directory=work/'lvgl-preview-nova-bt';exe=directory/'preview.exe'
font_root=work/'test-sd-nova-bt/nyx/nyx_gui/fonts/sd';font_root.mkdir(parents=True,exist_ok=True)
wallpapers=work/'test-sd-nova-bt/wallpapers';wallpapers.mkdir(exist_ok=True)
shutil.copyfile(project/'wallpapers/Nova-Carbon.bmp',wallpapers/'Nova-Carbon.bmp')
for font in (project/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copyfile(font,font_root/font.name)
shutil.copyfile(work.parent/'outputs/Caviar.nyxf',font_root/'Caviar.nyxf')
screens=[('about',i)for i in range(5)]+[('usb',0)]+[('bluetooth',i)for i in range(4)]+[('nyx',2)]
fonts=['Lecture.nyxf','Lecture-fixe.nyxf','Caviar.nyxf']
jobs=[(screen,language,size,tab,font,{'NYX_PREVIEW_WALLPAPER':'Nova-Carbon.bmp','NYX_PREVIEW_MENU_OPACITY':'25'})for screen,tab in screens for language in range(7)for size in (20,36)for font in fonts]

def run(job):
    screen,language,size,tab,font,extra=job;environment=os.environ.copy()
    environment.update({'NYX_PREVIEW_NO_DUMP':'1','NYX_PREVIEW_FONT':font});environment.update(extra)
    result=subprocess.run([str(exe),str(font_root),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],cwd=directory,capture_output=True,text=True,env=environment,timeout=30)
    if result.returncode:return {'job':job,'exit_code':result.returncode,'output':result.stdout+result.stderr}
    return None
start=time.time();failed=[];completed=0
with ThreadPoolExecutor(max_workers=6)as pool:
    futures=[pool.submit(run,job)for job in jobs]
    for future in as_completed(futures):
        failure=future.result();completed+=1
        if failure:
            failed.append(failure)
            if len(failed)<=5:print('FAILED '+json.dumps(failure,ensure_ascii=False),flush=True)
        if completed%400==0:print(f'Checked {completed}/{len(jobs)} native scenes.',flush=True)
report={'native_scenes':len(jobs),'fonts':fonts,'languages':7,'sizes':[20,36],'changed_ui_views':11,'failures':failed,'elapsed_seconds':round(time.time()-start,1),'hardware_io':'substituted; no console or SD operations performed'}
report['source_sha256']={f:hashlib.sha256((project/f).read_bytes()).hexdigest() for f in ('nyx/nyx_gui/frontend/gui_nova.c','nyx/nyx_gui/frontend/gui_tools.c','nyx/nyx_gui/frontend/gui_options.c','nyx/nyx_gui/frontend/gui_bluetooth.c','nyx/nyx_gui/frontend/nova_assets/nova_icons.inc','nyx/nyx_gui/localization_catalog.inc')}
(work/'native-ui-nova-bt.json').write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
if failed:raise SystemExit(f'{len(failed)} native scene failures; see native-ui-nova-bt.json')
print(f'PASS: {len(jobs)} real-LVGL scenes; three font files, seven languages, credits/changelog, USB, Bluetooth and TV settings.',flush=True)
