from pathlib import Path
import subprocess,time,json
w=Path.cwd();d=w/'work/nova-installer-1.3';b=d/'build';t=w/'work/toolchains/w64devkit/bin';root=w/'work'/('ni11-tests-'+str(time.time_ns())[-9:]);root.mkdir();(root/'links').mkdir();(root/'sentinel').mkdir();(root/'sentinel/keep.txt').write_text('EXTERNAL SENTINEL')
cmd="New-Item -ItemType Junction -Path '"+str(root/'links/bootloader')+"' -Target '"+str(root/'sentinel')+"' | Out-Null"
subprocess.run(['powershell.exe','-NoProfile','-Command',cmd],check=True)
args=[str(t/'gcc.exe'),'-std=gnu11','-O2','-static','-municode','-DINSTALLER_TEST','-I'+str(d),str(d/'install.c'),str(d/'test.c'),str(b/'resources.o'),'-o',str(b/'test.exe'),'-lsetupapi','-ladvapi32']
r=subprocess.run(args,capture_output=True,text=True);assert r.returncode==0,r.stderr
r=subprocess.run([str(b/'test.exe'),str(root)],capture_output=True,text=True,timeout=120);print(r.stdout+r.stderr);assert not r.returncode
(b/'test-result.json').write_text(json.dumps({'passed':True,'result':r.stdout.strip(),'fixture_root':str(root),'physical_sd_written':False},indent=2))
