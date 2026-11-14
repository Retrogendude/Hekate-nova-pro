"""Build and record the precise TV prototype sources/binaries being delivered."""
from pathlib import Path
import hashlib, json, subprocess, sys, time
work = Path(__file__).resolve().parent
project = work / 'hekate-tv-02'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()

def tracked():
    result = {}
    for p in project.rglob('*'):
        if not p.is_file(): continue
        rel = p.relative_to(project)
        if any(x in ('build', 'output', '.git', '__pycache__') for x in rel.parts): continue
        if rel.as_posix() in ('loader/payload_00.h', 'loader/payload_01.h'): continue
        result[rel.as_posix()] = sha(p)
    return result

subprocess.run([sys.executable, str(work / 'check_tv_lifecycle_02.py')], check=True)
subprocess.run([sys.executable, str(work / 'check_tv_policy_02.py')], check=True)
subprocess.run([sys.executable, str(work / 'check_tv_protocol.py')], check=True)
before = tracked()
start = time.time()
result = subprocess.run([sys.executable, str(work / 'run_build_tv02.py'), '-B', '-r'],
                        text=True, capture_output=True)
print('\n'.join(result.stdout.splitlines()[-8:]))
if result.returncode:
    print(result.stderr)
    raise SystemExit(result.returncode)
assert before == tracked(), 'Sources changed during build'
assert (project / 'output/hekate_unc.bin').stat().st_size <= 140288
nm = subprocess.check_output(['C:/devkitPro/devkitARM/bin/arm-none-eabi-nm.exe',
                              str(project / 'build/nyx/nyx.elf')], text=True)
for symbol in ('gui_tv_open', 'gui_tv_dirty', 'dock_step', 'dpout_video_start', 'dpout_aux_end'):
    assert any(line.endswith(' ' + symbol) for line in nm.splitlines()), symbol
assert 'error:' not in (work / 'hekate-tv-02-build.log').read_text(errors='replace')
binaries = {p.name: {'size': p.stat().st_size, 'sha256': sha(p)}
            for p in (project / 'output').iterdir() if p.is_file()}
tests = {name: json.loads((work / ('tv02-' + name + '-check.json')).read_text())
         for name in ('lifecycle', 'policy')}
tests['protocol'] = json.loads((work / 'tv-protocol-check.json').read_text())
assert tests['protocol']['source_sha256'] == before['bdk/display/tv/usbpd.c']
assert tests['lifecycle']['source_sha256'] == before['nyx/nyx_gui/frontend/gui_tv.c']
for name, digest in tests['policy']['source_sha256'].items():
    assert digest == before['bdk/display/tv/' + name]
info = {'experiment': 'TV 02', 'base': 'Hekate 6.5.5 / Nyx 1.9.5 (nd5 copy)',
        'build_exit_code': result.returncode, 'elapsed_seconds': round(time.time() - start, 1),
        'hardware_validation': False, 'source_sha256': before,
        'binaries': binaries, 'tests': tests}
(work / 'verification-tv-02.json').write_text(json.dumps(info, indent=2))
print('PASS: exact-source full build, linked TV entry points and binary hashes.')
