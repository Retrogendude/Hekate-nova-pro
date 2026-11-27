"""Exercise actual LVGL wizard code and the guarded partition bridge on fake media."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor,as_completed
import subprocess,os,json,hashlib
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v34-alpha4';D=W/'lvgl-preview-nova34a4';F=W/'test-sd-nova34a4/nyx/nyx_gui/fonts/sd'
jobs=[('wizard',language,size,step,0,128,1,False)for language in range(8)for size in (20,30,36)for step in range(4)]
jobs += [(scene,language,size,0,0,128,1,False)for scene in ('home','carousel')for language in range(8)for size in (20,30,36)]
jobs += [('wizard',language,30,0,model,capacity,mask,True)for language in (1,4,7)for model in range(4)for capacity in (0,32,64,128)for mask in range(1,8)]
jobs=[(*j,32,32)for j in jobs]
jobs += [('wizard',language,30,0,model,capacity,7,True,linux,android)for language in range(8)for model in range(4)for capacity in (64,128)for linux,android in ((16,16),(16,32),(32,16))]
jobs += [('wizard',language,size,1,0,128,6,False,16,32)for language in range(8)for size in (20,30,36)]
def run(job):
 scene,language,size,step,model,capacity,mask,exercise,linux,android=job;env=os.environ.copy()
 env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_WALLPAPER='Nova-Carbon-Cyan.bmp',NYX_TEST_MODEL=str(model),NYX_TEST_CAPACITY=str(capacity),NYX_WIZARD_SYSTEMS=str(mask),NYX_WIZARD_LINUX=str(linux),NYX_WIZARD_ANDROID=str(android))
 if exercise:env['NYX_TEST_WIZARD']='1'
 r=subprocess.run([str(D/'preview.exe'),str(F),str(D/'res.pak'),scene,str(language),str(size),str(step)],cwd=D,env=env,capture_output=True,text=True,timeout=45)
 return {'case':job,'passed':r.returncode==0,'output':(r.stdout+r.stderr)[-1600:]}
results=[]
with ThreadPoolExecutor(max_workers=4)as pool:
 for f in as_completed([pool.submit(run,j)for j in jobs]):
  r=f.result();results.append(r)
  if not r['passed']:print(r,flush=True)
  if len(results)%100==0:print('Wizard cases',len(results),'/',len(jobs),flush=True)
failed=[x for x in results if not x['passed']]
names=['gui_nova.c','gui_nova.h','gui_install_wizard.inc','gui_partition_plan.c','gui_partition_plan.h','gui_tools_partition_manager.c']
report={'passed':not failed,'cases':len(results),'locales':8,'sizes':[20,30,36],'physical_console_test':False,'destructive_partition_test':False,'automatic_emummc_copy_implemented':True,'coverage':['intro, model/system choice, review, erasure acknowledgement','all 7 system combinations','four hardware models','absent/too-small/sufficient SD','model mismatch','previous/next retains choices','empty selection blocked','exact independent 16/32 GB choices retained and passed to partition bridge','consent labels and revoke','assistant-only bottom-right Next','confirmed plan passed to existing engine without executing it'],'failures':failed,'source_sha256':{n:hashlib.sha256((P/'nyx/nyx_gui/frontend'/n).read_bytes()).hexdigest()for n in names}}
(W/'nova34a4-wizard-check.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');assert not failed
print('PASS:',len(results),'actual-LVGL wizard/home/guarded-handoff cases')
