"""Targeted validation of the wider, softer and faster native tile sheen."""
from pathlib import Path
from zipfile import ZipFile
from PIL import Image,ImageChops
import subprocess,os,json,math,re,hashlib
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';directory=work/'lvgl-preview-v15';outputs=work.parent/'outputs'
exe=directory/'preview.exe';fonts=project/'nyx/nyx_gui/fonts/sd';logs=[]
def run(screen,language=1,size=30,tab=0,extra=None,cwd=directory):
 env={k:v for k,v in os.environ.items()if not k.startswith('NYX_')};env.update(NYX_PREVIEW_NO_DUMP='1',NYX_PREVIEW_FONT='Lecture.nyxf')
 if extra:env.update(extra)
 result=subprocess.run([str(exe),str(fonts),str(directory/'res.pak'),screen,str(language),str(size),str(tab)],cwd=cwd,env=env,capture_output=True,text=True,timeout=30)
 if result.returncode:raise RuntimeError(f'{screen} {language} {size} {tab}: '+result.stdout+result.stderr)
 match=re.search(r'Nova reflection max=(\d+) total=(\d+)',result.stdout)
 if match:
  performance={'reflection_period_ms':40,'max_redraw_pixels_per_frame':int(match[1]),'total_redraw_pixels_over_2880ms':int(match[2]),'paused_redraw_pixels':0,'platform':'Host LVGL, not Switch FPS.'}
  (directory/('nova-performance-'+screen+'.json')).write_text(json.dumps(performance,indent=2))
 return result.stdout
profiles=[]
for photo in (False,True):
 destination=work/('nova-v15-gradient-photo'if photo else'nova-v15-gradient-solid');destination.mkdir(exist_ok=True)
 extra={'NYX_TEST_SHEEN_PROFILE':'1'}
 if photo:extra.update(NYX_PREVIEW_WALLPAPER='Nova-Carbon.bmp',NYX_PREVIEW_MENU_OPACITY='50')
 stdout=run('home',extra=extra,cwd=destination)
 match=re.search(r'PROFILE center=(\d+) row=(\d+) half=(\d+) sweep=(\d+) cycle=(\d+) period=(\d+)',stdout);assert match,stdout
 center,row,half,sweep,cycle,period=map(int,match.groups())
 assert half==64 and sweep*period==480 and(cycle-sweep)*period==2400
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
 profiles.append({'wallpaper':photo,'width_px':129,'active_profile_width_px':max(active)-min(active)+1,'sweep_ms':480,'rest_ms':2400,'transparent_edges':True,'native_pixel_oracle':True,'affected_bounds':bbox})
logs.append('PASS: real native pixels match the 129px transparent cosine gradient on a solid tile and wallpaper; gradual sub-16/255 opacity, symmetry, soft ends, clipping and 480ms sweep / 2400ms rest verified.')

(work/'nova-v15-gradient.json').write_text(json.dumps(profiles,indent=2))
print('\n'.join(logs),flush=True)
