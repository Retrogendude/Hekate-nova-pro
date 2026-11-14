from pathlib import Path
import subprocess,os,json,hashlib
w=Path('work').resolve();p=w/'hekate-nova-pro-v31';d=w/'lvgl-preview-nova31';cc=w/'toolchains/w64devkit/bin/gcc.exe';t=w/'nova31-display-tests'
s=(w/'hekate-nova-pro-v30/bdk/libs/lvgl/lv_draw/lv_draw_vbasic.c').read_text();s+='\nvoid lv_draw_set_service_cb(void (*callback)(void)){(void)callback;}\n'
(t/'vbasic-before.c').write_text(s)
# The original rasterizer uses sibling relative includes; retain its original path.
source=p/'bdk/libs/lvgl/lv_draw/vbasic_before_test.c';source.write_text(s)
try:
 flags=['-std=gnu11','-O1','-include','stdint.h','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(d),'-I'+str(p/'bdk'),'-I'+str(p/'nyx/nyx_gui')]
 r=subprocess.run([str(cc),*flags,'-c',str(source),'-o',str(t/'vbasic-before.o')],capture_output=True,text=True);assert r.returncode==0,r.stderr
finally:source.unlink()
objs=[str(f)for f in d.glob('*.o')if f.name!='lv_draw_vbasic.o'];subprocess.run([str(cc),*objs,str(t/'vbasic-before.o'),'-o',str(t/'raster-before.exe')],check=True)
checks=[]
for screen,tab in [('home',0),('partition-expert',0),('partition-emmc',0),('nyx',2),('about',4),('bluetooth',0)]:
 hashes=[]
 for exe in [t/'raster-before.exe',d/'preview.exe']:
  env=os.environ.copy();env.update(NYX_TEST_MODEL='3',NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='25')
  r=subprocess.run([str(exe),str(w/'test-sd-nova31/nyx/nyx_gui/fonts/sd'),str(d/'res.pak'),screen,'1','30',str(tab)],cwd=d,capture_output=True,text=True,env=env,timeout=30);assert r.returncode==0,(screen,r.stdout,r.stderr)
  hashes.append(hashlib.sha256((d/'preview.ppm').read_bytes()).hexdigest())
 assert hashes[0]==hashes[1],(screen,hashes);checks.append({'screen':screen,'sha256':hashes[0]})
report={'passed':True,'pixel_exact_frames':checks,'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ['bdk/libs/lvgl/lv_draw/lv_draw_vbasic.c','bdk/libs/lvgl/lv_draw/lv_draw_vbasic.h']},'reference':'v30 rasterizer, same current UI/font/theme, service setter substituted only in baseline'}
(w/'nova31-raster-check.json').write_text(json.dumps(report,indent=2));print('PASS: six pixel-identical UI frames with actual draw servicing enabled, including wallpaper, alpha, full-width sliders, credits and fonts')
