from pathlib import Path
import subprocess, concurrent.futures, os, json
root=Path(__file__).resolve().parent/'hekate-6.5.4'
cwd=root/'nyx'
flags=(root/'build/nyx/.cflags').read_text().strip()
# This stamp is the exact previously working Nyx command line. Its two header
# defines require quoted C tokens, rather than shell-literal single quotes.
import shlex
args=shlex.split(flags)
args=[a for a in args if not a.startswith(('-DGFX_INC=', '-DFFCFG_INC='))]
args+=['-DGFX_INC="../nyx/nyx_gui/gfx/gfx.h"', '-DFFCFG_INC="../nyx/nyx_gui/libs/fatfs/ffconf.h"']
args += ['-I../bdk','-DHAVE_CPU_STUB=1']
gcc='C:/devkitPro/devkitARM/bin/arm-none-eabi-gcc.exe'
out=Path(__file__).resolve().parent/'backend-check';out.mkdir(exist_ok=True)
sources=list((cwd/'nyx_gui/diagnostics/vendor').glob('*.c'))
sources=[p for p in sources if p.stem!='core']+[cwd/'nyx_gui/diagnostics/nyx_diagnostics.c',cwd/'nyx_gui/diagnostics/hwtest_pages.c']
def compile_one(p):
    command=[gcc]+args+['-c',str(p.relative_to(cwd)),'-o',str(out/(p.stem+'.o'))]
    r=subprocess.run(command,cwd=cwd,capture_output=True,text=True)
    return p.stem,r.returncode,r.stderr
failed=0
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    for name,code,log in pool.map(compile_one,sources):
        if code:
            failed+=1;print(name,'FAILED\n',log)
        else:print(name,'OK',log)
print('Failed:',failed,'/',len(sources))
raise SystemExit(bool(failed))
