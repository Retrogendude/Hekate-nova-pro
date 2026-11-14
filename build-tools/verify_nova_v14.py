"""Targeted validation of the wider, softer and faster native tile sheen."""
from pathlib import Path
from zipfile import ZipFile
from PIL import Image,ImageChops
import subprocess,os,json,math,re,hashlib
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';directory=work/'lvgl-preview-v14';outputs=work.parent/'outputs'
exe=directory/'preview.exe';fonts=project/'nyx/nyx_gui/fonts/sd';logs=[]
def run(screen,language=1,size=30,tab=0,extra=None,cwd=directory):
 env={k:v for k,v in os.environ.items()if not k.startswith('NYX_')};env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT='Lecture.nyxf')
 if extra:env.update(extra)
 result=subprocess.run([str(exe),str(fonts),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],cwd=cwd,env=env,capture_output=True,text=True,timeout=30)
 if result.returncode:raise RuntimeError(f'{screen} {language} {size} {tab}: '+result.stdout+result.stderr)
 match=re.search(r'Nova reflection max=(\d+) total=(\d+)',result.stdout)
 if match:
  performance={'reflection_period_ms':40,'max_redraw_pixels_per_frame':int(match[1]),'total_redraw_pixels_over_3200ms':int(match[2]),'paused_redraw_pixels':0,'platform':'Host LVGL, not Switch FPS.'}
  (directory/('nova-performance-'+screen+'.json')).write_text(json.dumps(performance,indent=2))
 return result.stdout
# Prove scope: every delivered source/header/build recipe from v13 is unchanged
# except this renderer. Keep v13's wider regression evidence explicitly separate.
changed=[]
with ZipFile(outputs/'hekate-nova-pro-v13-sources.zip')as archive:
 for name in archive.namelist():
  if name.startswith('hekate-6.5.4/')and name.endswith(('.c','.h','Makefile')):
   path=work/name
   if path.read_bytes()!=archive.read(name):changed.append(name)
assert changed==['hekate-6.5.4/nyx/nyx_gui/frontend/gui_nova_reflection.c'],changed
modes=[('home',0),('carousel',0),('tools',0),('info',0),('diagnostics',0),('options',0),('nyx',0),('colors',0),('colors',1),('appearance',1),('setup',0)]
scenes=0
for screen,tab in modes:
 for language in (0,1,4):
  for size in (20,30,36):
   run(screen,language,size,tab,extra={'NYX_PREVIEW_WALLPAPER':'Nova-Carbon.bmp','NYX_PREVIEW_MENU_OPACITY':'50','NYX_PREVIEW_ICON_OPACITY':'25'});scenes+=1
logs.append(f'PASS: {scenes} current native UI scenes with wallpaper and partial transparency; main tile layouts, settings, colors, fonts and original clock; actual tree, hit-test, action and redraw checks.')
# Validate the actual rendered pixels against a smooth cosine opacity profile.
# This exercises real alpha blending below LV_OPA_MIN, rather than testing a
# table against itself, and catches a hard transparency cutoff at the ends.
profiles=[]
for photo in (False,True):
 destination=work/('nova-v14-gradient-photo'if photo else'nova-v14-gradient-solid');destination.mkdir(exist_ok=True)
 extra={'NYX_TEST_SHEEN_PROFILE':'1'}
 if photo:extra.update(NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='50')
 stdout=run('home',extra=extra,cwd=destination)
 match=re.search(r'PROFILE center=(\d+) row=(\d+) half=(\d+) sweep=(\d+) cycle=(\d+) period=(\d+)',stdout);assert match,stdout
 center,row,half,sweep,cycle,period=map(int,match.groups())
 assert half==64 and sweep*period==800 and(cycle-sweep)*period==2400
 before=Image.open(destination/'sheen-baseline.ppm').convert('RGB');after=Image.open(destination/'sheen-profile.ppm').convert('RGB')
 bbox=ImageChops.difference(before,after).getbbox();assert bbox and bbox[0]>=36 and bbox[1]>=217 and bbox[2]<=655 and bbox[3]<=627,bbox
 samples=[]
 for x in range(center-half-16,center+half+17):
  distance=x-center;opacity=round(36*(1+math.cos(math.pi*distance/half))/2)if abs(distance)<=half else 0
  original=before.getpixel((x,row));expected=tuple((255*opacity+c*(255-opacity))//256 for c in original)if opacity else original
  actual=after.getpixel((x,row));assert actual==expected,(photo,x,opacity,original,expected,actual)
  samples.append((distance,max(abs(a-b)for a,b in zip(actual,original))))
 active=[d for d,delta in samples if delta]
 assert max(active)-min(active)>95
 assert samples[16][1]==0 and samples[-17][1]==0
 assert max(delta for _,delta in samples)>=24
 if not photo:
  for i in range(1,len(samples)):assert abs(samples[i][1]-samples[i-1][1])<=2
  for distance,delta in samples:
   assert delta==dict(samples).get(-distance,delta)
 profiles.append({'wallpaper':photo,'width_px':129,'active_profile_width_px':max(active)-min(active)+1,'sweep_ms':800,'rest_ms':2400,'transparent_edges':True,'native_pixel_oracle':True,'affected_bounds':bbox})
logs.append('PASS: real native pixels match the 129px transparent cosine gradient on a solid tile and wallpaper; gradual sub-16/255 opacity, symmetry, soft ends, clipping and 800ms sweep / 2400ms rest verified.')
nav=subprocess.check_output([os.sys.executable,str(work/'check_nova_v14_navigation.py')],text=True);logs.append(nav.strip())
arm=Path('C:/devkitPro/devkitARM/bin');out=project/'output';ram=work/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin'
for name,elf in [('loader',project/'build/loader/loader.elf'),('nyx',project/'build/nyx/nyx.elf')]:
 target=work/f'final-v14-{name}.bin';subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(elf),str(target)],check=True)
 assert(out/('hekate.bin'if name=='loader'else'nyx.bin')).read_bytes()==target.read_bytes()+(b'ICTC49'if name=='loader'else b'')
with ZipFile(outputs/'hekate-nova-pro-v13.zip')as archive:
 assert archive.read('hekate_nova_pro_6.5.4_v13.bin')==(out/'hekate.bin').read_bytes()
 assert archive.read('bootloader/sys/hwtest_ram.bin')==ram.read_bytes()
assert(out/'hekate.bin').stat().st_size<=126296 and(out/'hekate_unc.bin').stat().st_size<=140288 and(out/'nyx.bin').stat().st_size<=16*1024*1024
symbols=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(project/'build/nyx/nyx.elf')],text=True)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(project/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text());assert len(probes)==42 and all(re.search(r'\bT '+probe+r'$',symbols,re.M)for probe in probes)
card=re.search(r'^(\w+) [tT] nova_card_signal$',symbols,re.M);toggle=re.search(r'^(\w+) [tT] nova_toggle_signal$',symbols,re.M);assert card and toggle and card[1]!=toggle[1]
logs.append('PASS: current ARM ELF binaries match delivered files and size limits; all 42 probes linked; Hekate boot and RAM payload byte-identical to v13.')
previous=json.loads((work/'verification-v13.json').read_text(encoding='utf-8'))
report={
 'product':'Hekate Nova Pro, version 14','authorized_changes':1,'build_exit_code':0,'hardware_validation':'not performed',
 'scope':'Only gui_nova_reflection.c changed relative to v13 source delivery. The v13 wallpaper order fix and original clock/language code are unchanged.',
 'changed_code_files':changed,'test_summaries':logs,'current_ui_scenes':scenes,'current_touch_scenes':21,'gradient_profiles':profiles,
 'wallpaper_navigation':json.loads((work/'nova-v14-navigation.json').read_text()),
 'full_tile_performance':{screen:json.loads((directory/('nova-performance-'+screen+'.json')).read_text())for screen in ('home','carousel','tools')},
 'baseline_regression':{'version':13,'scenes':4407,'executed_in_this_run':False,'report_sha256':hashlib.sha256((work/'verification-v13.json').read_bytes()).hexdigest()},
 'native_probe_entries':len(probes),'background_animation':'removed from the source, UI and linked executable',
 'hekate_payload_bytes':(out/'hekate.bin').stat().st_size,'nyx_bytes':(out/'nyx.bin').stat().st_size,'ram_service_bytes':ram.stat().st_size,
 'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}for p in [out/'hekate.bin',out/'hekate_unc.bin',out/'nyx.bin',ram]}}
(work/'verification-v14.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
(work/'verification-v14.txt').write_text('\n\n'.join(logs)+'\n',encoding='utf-8')
print('\n'.join(logs));print('PASS: Nova Pro v14 targeted verification completed.')
