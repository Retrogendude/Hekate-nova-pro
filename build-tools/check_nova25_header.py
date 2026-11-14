"""Check the actual loader contract and exercise the linker placement guard."""
from pathlib import Path
import subprocess,json,re,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v25';d=w/'nova25-tests'
arm=Path('C:/devkitPro/devkitARM/bin');cc=arm/'arm-none-eabi-gcc.exe'
nyx=(p/'output/nyx.bin').read_bytes();assert nyx[0x98:0x9f]==b'GCTC195'
for name in ('hekate.bin','hekate_unc.bin'):assert (p/'output'/name).read_bytes()[0x118:0x11f]==b'ICTC655'
old=(w/'hekate-nova-pro-v21-bt/output/nyx.bin').read_bytes()
assert old[0x98:0x9c]!=b'GCTC' and old[0xb0:0xb4]==b'GCTC'
objects=[str(f.resolve())for f in (p/'build/nyx').glob('*.o')]
common=[str(cc),'-march=armv4t','-mtune=arm7tdmi','-mthumb-interwork','-nostartfiles','-Wl,--nmagic,--gc-sections',
    '-Wl,--defsym=NYX_LOAD_ADDR=0x81000000','-Wl,-u,nova_header_intruder','-T',str((p/'nyx/nyx_gui/link.ld').resolve())]
for section,should_pass in [('.text._start',True),('.text.nyx_entry',False)]:
    (d/'intruder.S').write_text('.section '+section+',"ax"\n.arm\n.global nova_header_intruder\nnova_header_intruder:\n bx lr\n')
    subprocess.run([str(cc),'-march=armv4t','-c',str(d/'intruder.S'),'-o',str(d/'intruder.o')],check=True)
    r=subprocess.run([*common,*objects,str(d/'intruder.o'),'-lgcc','-o',str(d/'intruder.elf')],capture_output=True,text=True)
    if should_pass:
        assert r.returncode==0,r.stderr
        symbols=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(d/'intruder.elf')],text=True)
        assert re.search(r'^81000098 T ipl_ver$',symbols,re.M)
    else:assert r.returncode!=0 and 'Nyx version header must stay at offset 0x98'in r.stderr,r.stderr
result={'passed':True,'nyx_metadata_offset':'0x98','nyx_version':'1.9.5','loader_metadata_offset':'0x118','loader_version':'6.5.5',
    'old_defect_reproduced':True,'extra_c_callback_does_not_shift_header':True,'invalid_entry_growth_rejected_by_linker':True,
    'source_sha256':{n:hashlib.sha256((p/n).read_bytes()).hexdigest()for n in ('nyx/nyx_gui/start.S','nyx/nyx_gui/link.ld')}}
(w/'nova25-header-check.json').write_text(json.dumps(result,indent=2))
print('PASS: Nyx/loader binary versions and fixed offsets; old defect reproduced; linker rejects shifted headers.')
