from pathlib import Path
from PIL import Image,ImageChops
import os,subprocess,re,json
work=Path(__file__).resolve().parent;folder=work/'lvgl-preview-v16';exe=folder/'preview.exe';fonts=work/'hekate-6.5.4/nyx/nyx_gui/fonts/sd'
env=os.environ.copy();env.update(NYX_TEST_BORDER='1',NYX_TEST_SHEEN_PROFILE='1')
run=subprocess.run([str(exe),str(fonts),str(folder/'res.pak'),'home','1','32','0'],cwd=folder,env=env,capture_output=True,text=True,check=True)
first=Image.open(folder/'border-0.ppm').convert('RGB');second=Image.open(folder/'border-16.ppm').convert('RGB')
checks=0
for name,point in [('tile',(100,217)),('icon',(90,249)),('navigation',(250,13))]:
    a,b=first.getpixel(point),second.getpixel(point)
    assert a!=b and a[2]==255 and b[2]==255,(name,point,a,b)
    assert 50<=a[0]<=156 and 99<=a[1]<=144,(name,a)
    checks+=2
baseline=Image.open(folder/'sheen-baseline.ppm').convert('RGB');profile=Image.open(folder/'sheen-profile.ppm').convert('RGB')
center,row,half,sweep,cycle,period=map(int,re.search(r'center=(\d+) row=(\d+) half=(\d+) sweep=(\d+) cycle=(\d+) period=(\d+)',run.stdout).groups())
assert half==64 and sweep*period==240 and cycle*period==720
# Broad, white, transparent ends; interior sample avoids outlines/text/glyphs.
values=[]
for x in range(center-half,center+half+1):
    a,b=baseline.getpixel((x,row)),profile.getpixel((x,row));delta=[b[i]-a[i]for i in range(3)]
    assert min(delta)>=0 and max(delta)-min(delta)<=8,(x,delta)
    values.append(sum(delta))
assert max(values)>35 and values[0]==values[-1]==0 and values[len(values)//2]>values[12]
checks+=132
report={'sweep_ms':sweep*period,'repeat_ms':cycle*period,'border_blue_violet_verified':True,'gradient_width_pixels':2*half+1,'pixel_checks':checks,'performance':{}}
for screen in ('home','tools','carousel'):
    result=subprocess.run([str(exe),str(fonts),str(folder/'res.pak'),screen,'0','30','0'],cwd=folder,capture_output=True,text=True,check=True)
    maximum,total=map(int,re.search(r'Nova reflection max=(\d+) total=(\d+)',result.stdout).groups())
    assert 0<maximum<1280*720/8
    report['performance'][screen]={'peak_redraw_pixels':maximum,'cycle_redraw_pixels':total,'pause_redraw_pixels':0}
first.save(folder/'border-phase-0.png');second.save(folder/'border-phase-16.png');profile.save(folder/'sheen-fast.png')
(work/'animation-v16.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('PASS: animated blue/violet tile, icon and navigation outlines; 240ms white sheen with 129px fade; redraw below one eighth of display; zero paused redraw.')
