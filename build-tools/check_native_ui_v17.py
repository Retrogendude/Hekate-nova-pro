"""Bounded native LVGL scenes, real fonts, translations and interactions."""
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import json,os,subprocess,shutil,time
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';directory=work/'lvgl-preview-v17';exe=directory/'preview.exe'
font_root=work/'test-sd-v17/nyx/nyx_gui/fonts/sd';font_root.mkdir(parents=True,exist_ok=True)
wallpapers=work/'test-sd-v17/wallpapers';wallpapers.mkdir(exist_ok=True)
shutil.copyfile(project/'wallpapers/Nova-Carbon.bmp',wallpapers/'Nova-Carbon.bmp')
for font in (project/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copyfile(font,font_root/font.name)
shutil.copyfile(work.parent/'outputs/Caviar.nyxf',font_root/'Caviar.nyxf')
screens=[('partition-simple',0),('partition-simple',4),('partition-simple',8),('partition-expert',0),('partition-emmc',0),('backup',0),('restore',0),('usb',0),('archive',0),('package',0),('package',1),('nyx',0),('nyx',1),('about',0),('home',0),('colors',1),('colors',0),('colors',2),('colors',3),('carousel',0),('tools',0),('info',0),('diagnostics',0),('options',0),('emummc',0),('emummc-change',9)]
fonts=['Lecture.nyxf','Lecture-renforcee.nyxf','Lecture-fixe.nyxf','Caviar.nyxf']
jobs=[(screen,language,size,tab,font,{})for screen,tab in screens for language in range(7)for size in (20,30,32,36)for font in fonts]
# Low capacity, each real console model, independent system presets, actual
# slider callbacks, invalid-model blocking, Android total and 2x emuMMC.
jobs += [('partition-simple',1,32,option,'Lecture.nyxf',{'NYX_TEST_MODEL':str(model),'NYX_TEST_CAPACITY':str(capacity),'NYX_TEST_PARTITION_ACTIONS':'1'})for model in range(4)for capacity in (16,32,64,128,256)for option in range(9)]
# First-start Carbon + 75% transparency and cumulative palette navigation.
jobs += [(screen,language,32,tab,'Caviar.nyxf',{'NYX_PREVIEW_WALLPAPER':'Nova-Carbon.bmp','NYX_PREVIEW_MENU_OPACITY':'25'})for screen,tab in screens for language in (0,1,4)]
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
report={'native_scenes':len(jobs),'fonts':fonts,'languages':7,'sizes':[20,30,32,36],'partition_action_scenes':180,'first_start_carbon_scenes':len(screens)*3,'failures':failed,'elapsed_seconds':round(time.time()-start,1),'hardware_io':'substituted; no console or SD operations performed'}
(work/'native-ui-v17.json').write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding='utf-8')
if failed:raise SystemExit(f'{len(failed)} native scene failures; see native-ui-v17.json')
print(f'PASS: {len(jobs)} real-LVGL scenes; four font files, seven languages, bounded partition/settings/backup/USB/archive/package/credits views and first-start Carbon.',flush=True)
