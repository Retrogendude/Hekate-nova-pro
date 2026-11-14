"""End-to-end real decoder/merge/pointer/LVGL tests, plus v21 regressions."""
from pathlib import Path
import subprocess,os,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v23';d=w/'lvgl-preview-nova23';t=w/'nova23-tests'
cc=w/'toolchains/w64devkit/bin/gcc.exe';nyx=p/'nyx/nyx_gui'
c=(d/'preview.c').read_text(encoding='utf-8')
old=(w/'hekate-nova-pro-v21-bt/nyx/nyx_gui/frontend/gui.c').read_text()
old=old[old.index('static bool _jc_virt_mouse_read('):old.index('typedef struct _system_maintenance_tasks_t')]
a=c.index('static bool _jc_virt_mouse_read(');b=c.index('static unsigned bt_test_clicks;',a)
(t/'old_navigation.c').write_text(c[:a]+old+c[b:],encoding='utf-8')
flags=['-std=gnu11','-O1','-include','stdint.h','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(d),'-I'+str(p/'bdk'),'-I'+str(nyx)]
r=subprocess.run([str(cc),*flags,'-c',str(t/'old_navigation.c'),'-o',str(t/'old_navigation.o')],capture_output=True,text=True)
assert r.returncode==0,r.stderr
objs=[str(f)for f in d.glob('*.o')if f.name!='preview.o']
subprocess.run([str(cc),*objs,str(t/'old_navigation.o'),'-o',str(t/'old_navigation.exe')],check=True)
font=w/'test-sd-nova23/nyx/nyx_gui/fonts/sd'
env=os.environ.copy();env.update(NYX_TEST_BT_INPUT='1',NYX_PREVIEW_NO_DUMP='1')
base=[]
for right,expected in [(False,'calibration_step==JC_CAL_MAX_STEPS'),(True,'!jc_drv_ctx.cursor_hidden')]:
    environment=env.copy()
    if right:environment['NYX_TEST_BT_PREFER_RIGHT']='1'
    r=subprocess.run([str(t/'old_navigation.exe'),str(font),str(d/'res.pak'),'bluetooth','1','30','0'],cwd=d,env=environment,capture_output=True,text=True,timeout=20)
    assert r.returncode!=0 and expected in r.stderr,(r.returncode,r.stdout,r.stderr)
    base.append(r.stderr.strip())
passed=0
for language in range(7):
    for size in (20,36):
        for right in (False,True):
            environment=env.copy()
            if right:environment['NYX_TEST_BT_PREFER_RIGHT']='1'
            r=subprocess.run([str(d/'preview.exe'),str(font),str(d/'res.pak'),'bluetooth',str(language),str(size),'0'],cwd=d,env=environment,capture_output=True,text=True,timeout=20)
            assert r.returncode==0 and 'PASS Bluetooth report ->' in r.stdout,(r.stdout,r.stderr)
            passed+=1
files=['frontend/gui.c','frontend/gui_bluetooth.c','bluetooth/nova_bt_protocol.c','bluetooth/nova_bt.h']
report={'passed_scenes':passed,'regressions_fail_on_v21':base,'real_code':'decoder, merge, pointer callback and LVGL widgets; radio substituted',
    'source_sha256':{f:hashlib.sha256((nyx/f).read_bytes()).hexdigest()for f in files}}
(w/'nova23-navigation-check.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(f'PASS: {passed} end-to-end wireless navigation scenes; missing-half and hidden-cursor regressions reproduced on v21.')
