"""Build the isolated HWTest-RCM RAM service using local ARM and host tools."""
from pathlib import Path
import os
import subprocess
import sys
import time

workspace = Path(__file__).resolve().parent.parent
source = workspace / 'work' / 'HWTest-RCM-nova33' / 'ramtest'
host = workspace / 'work' / 'toolchains' / 'w64devkit' / 'bin'
arm = Path('C:/devkitPro/devkitARM')
# The upstream recipe replaces this generated directory. Validate the final
# resolved target before allowing its recursive cleanup, including symlinks.
generated = (source / '..' / 'build-ram' / 'atmosphere').resolve()
assert workspace.resolve() in generated.parents
assert generated.name == 'atmosphere' and generated.parent.name == 'build-ram'
env = os.environ.copy()
env['PATH'] = ';'.join((str(host), str(arm / 'bin'), 'C:/Python315',
                       'C:/Windows/system32', 'C:/Windows'))
args = [str(host / 'make.exe'), '-j4', 'DEVKITARM=C:/devkitPro/devkitARM',
        'SHELL=' + (host / 'sh.exe').as_posix(), 'PYTHON=C:/Python315/python.exe',
        'GIT=C:/Users/Retrogen/.cache/codex-runtimes/codex-primary-runtime/dependencies/native/git/cmd/git.exe'] + sys.argv[1:]
log = workspace / 'work' / 'nova33-ram-build.log'
start = time.time()
with log.open('w', encoding='utf-8') as output:
    result = subprocess.run(args, cwd=source, env=env, stdout=output,
                            stderr=subprocess.STDOUT)
print(f'RAM build exit: {result.returncode}; elapsed {time.time()-start:.1f}s')
print('\n'.join(log.read_text(encoding='utf-8', errors='replace').splitlines()[-100:]))
sys.exit(result.returncode)
