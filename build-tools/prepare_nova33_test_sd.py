"""Prepare isolated host-test files; never write to a physical card."""
from pathlib import Path
import shutil
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33';SD=W/'test-sd-nova33'
assert W.resolve()in SD.resolve().parents
fonts=SD/'nyx/nyx_gui/fonts/sd';fonts.mkdir(parents=True,exist_ok=True)
for p in (P/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'):shutil.copy2(p,fonts/p.name)
for name in ('icone','wallpapers'):
 source=W.parent/'outputs/nova-pack-visuel-01'/name
 if not source.exists():source=W/'assets'/name
 assert source.is_dir(),source
 shutil.copytree(source,SD/name,dirs_exist_ok=True)
launcher=W/'assets/icone/Launcher.png'
if launcher.exists():shutil.copy2(launcher,SD/'icone/Launcher.png')
assert(SD/'icone/Launcher.png').exists(),'Restore the supplied Launcher.png into assets/icone.'
internal=SD/'bootloader/sys';internal.mkdir(parents=True,exist_ok=True)
shutil.copy2(fonts/'Nova-Chinese-Simplified.nyxf',internal/'Nova-Chinese-Simplified.nyxf')
print('Prepared isolated Nova v33 host-test assets.')
