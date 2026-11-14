from pathlib import Path
import os
import re
import subprocess
import sys
import time

workspace = Path(__file__).resolve().parent.parent
source = workspace / 'work' / 'hekate-nova-pro-v26'
host = workspace / 'work' / 'toolchains' / 'w64devkit' / 'bin'
arm = Path('C:/devkitPro/devkitARM')
tools_environment = workspace / 'work' / 'build-environment'
(tools_environment / 'devkitARM').mkdir(parents=True, exist_ok=True)
# Keep devkitARM's tool definitions, but let native Windows make inherit its
# semicolon-separated PATH instead of the MSYS-only colon path conversion.
base_tools = (arm / 'base_tools').read_text(encoding='utf-8')
base_tools = re.sub(r'^export PATH[^\n]*\n', '', base_tools, flags=re.MULTILINE)
(tools_environment / 'devkitARM' / 'base_tools').write_text(base_tools, encoding='utf-8')
build_path = ';'.join((str(host), str(arm / 'bin'), str(Path(sys.executable).parent), 'C:/Windows/system32', 'C:/Windows'))
args = [str(host / 'make.exe'), '-j4', '-r',
        'DEVKITPRO=' + tools_environment.as_posix(), 'DEVKITARM=C:/devkitPro/devkitARM',
        'SHELL=' + (host / 'sh.exe').as_posix(),
        'NATIVE_CC=' + (host / 'gcc.exe').as_posix(),
        'PYTHON=' + Path(sys.executable).as_posix(),
        'IPLECHO=echo', 'NYXECHO=echo'] + sys.argv[1:]
env = os.environ.copy()
env['PATH'] = build_path
log = workspace / 'work' / 'nova26-build.log'
start = time.time()
print('Building Hekate and Nyx with devkitARM and portable native tools.', flush=True)
with log.open('w', encoding='utf-8') as output:
    result = subprocess.run(args, cwd=source, env=env, stdout=output, stderr=subprocess.STDOUT)
print(f'Build exit code: {result.returncode}; elapsed: {time.time()-start:.1f}s', flush=True)
lines = log.read_text(encoding='utf-8', errors='replace').splitlines()
print('\n'.join(lines[-100:]))
if result.returncode == 0:
    for p in sorted((source / 'output').glob('*')):
        if p.is_file():
            print(f'{p.name}: {p.stat().st_size} bytes')
sys.exit(result.returncode)
