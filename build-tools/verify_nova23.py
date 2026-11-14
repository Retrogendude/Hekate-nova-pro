from pathlib import Path
import subprocess,json,re,hashlib,sys
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v23';base=w/'hekate-nova-pro-v22';arm=Path('C:/devkitPro/devkitARM/bin')
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
def sources():
 return {f.relative_to(p).as_posix():sha(f)for f in p.rglob('*')if f.is_file()
    and not any(t in ('build','output','.git','__pycache__')for t in f.relative_to(p).parts)
    and f.relative_to(p).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
before=sources()
expected={'nyx/Makefile','nyx/nyx_gui/localization_catalog.inc','nyx/nyx_gui/localization_catalog.json','nyx/nyx_gui/nova_version.h',
 'nyx/nyx_gui/bluetooth/btstack_config.h','nyx/nyx_gui/bluetooth/nova_bt.h','nyx/nyx_gui/bluetooth/nova_bt_stack.c',
 'nyx/nyx_gui/frontend/gui_bluetooth.c','nyx/nyx_gui/frontend/gui_nova.c','nyx/nyx_gui/libs/btstack/UPSTREAM.txt','nyx/nyx_gui/libs/btstack/classic/hid_host.c'}
changed={f for f,h in before.items()if(base/f).exists()and sha(base/f)!=h}
assert changed==expected,changed^expected
for f in ('nyx/nyx_gui/frontend/gui.c','nyx/nyx_gui/bluetooth/nova_bt_protocol.c','nyx/nyx_gui/bluetooth/nova_bt_radio.c',
 'bdk/display/tv/dpout.c','bdk/display/tv/usbpd.c','nyx/nyx_gui/diagnostics/ram_handoff.h'):
 assert sha(p/f)==sha(base/f),f
lib=p/'nyx/nyx_gui/libs/btstack'
for f in lib.rglob('*'):
 if f.suffix not in ('.c','.h'):continue
 rel=f.relative_to(lib)
 if rel.as_posix()=='classic/hid_host.c':continue
 assert sha(f)==sha(w/'btstack-upstream/src'/rel),rel
names=('nova23-bt-check.json','nova23-negotiation-check.json','nova23-report-check.json','nova23-radio-check.json',
 'nova23-input-check.json','nova23-navigation-check.json','nova23-tv-lifecycle-check.json','native-ui-nova23.json','nova23-visual-check.json')
reports={n:json.loads((w/n).read_text(encoding='utf-8'))for n in names}
neg=reports['nova23-negotiation-check.json'];assert neg['passed']and len(neg['logs'])==4
assert neg['hid_host_sha256']==sha(lib/'classic/hid_host.c')
for f,h in neg['source_sha256'].items():assert before['nyx/nyx_gui/'+f]==h
assert reports['nova23-report-check.json']['passed']and reports['nova23-report-check.json']['cases']==11
assert reports['nova23-report-check.json']['source_sha256']==sha(p/'nyx/nyx_gui/frontend/gui_bluetooth.c')
assert reports['native-ui-nova23.json']['native_scenes']==462 and not reports['native-ui-nova23.json']['failures']
assert reports['nova23-navigation-check.json']['passed_scenes']==28
assert reports['nova23-visual-check.json']['identical_views']==49
for f,h in reports['native-ui-nova23.json']['source_sha256'].items():assert before[f]==h
print('Nova23 sources and full-stack negotiation evidence verified. Final ARM build.',flush=True)
r=subprocess.run([sys.executable,str(w/'run_build_nova23.py'),'-B','-r'],capture_output=True,text=True)
print('\n'.join(r.stdout.splitlines()[-8:]),flush=True)
if r.returncode:print(r.stdout+r.stderr);raise SystemExit(r.returncode)
assert sources()==before,'Source changed during build'
subprocess.run([sys.executable,str(w/'check_nova23_header.py')],check=True)
reports['nova23-header-check.json']=json.loads((w/'nova23-header-check.json').read_text())
out=p/'output'
for n in ('hekate.bin','hekate_unc.bin','libsys_lp0.bso','libsys_minerva.bso'):assert sha(out/n)==sha(base/'output'/n),n
for elf,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
 tmp=w/('linked-nova23-'+name)
 subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(p/elf),str(tmp)],check=True)
 assert tmp.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')==(out/name).read_bytes()
nm=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(p/'build/nyx/nyx.elf')],text=True)
assert re.search(r'^81000098 T ipl_ver$',nm,re.M)
for symbol in ('gui_bluetooth_merge','nova_bt_trace_get','nova_bt_poll','hid_host_connect','hci_init','hw_display_external_end'):
 assert re.search(r'\bT '+symbol+r'$',nm,re.M),symbol
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(p/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
assert len(probes)==42 and all(re.search(r'\bT '+f+r'$',nm,re.M)for f in probes)
info={'product':'Hekate Nova Pro','version':'23','base':'Hekate6.5.5 / Nyx1.9.5','build_exit_code':0,
 'source_sha256':before,'changed_existing_sources':sorted(changed),'binaries':{f.name:{'size':f.stat().st_size,'sha256':sha(f)}for f in out.iterdir()if f.is_file()},
 'tests':reports,'native_probes':42,'physical_validation':False,
 'connection_change':'Known HID PSM0x11 then0x13; security level2 unchanged; no mandatory SDP descriptor query',
 'evidence':'Virtual peer exercises actual HCI authentication and L2CAP/HID signals; does not prove the console radio works',
 'report':'Manual Stop writes bounded protocol metadata only; no addresses, keys or packet payloads',
 'license_note':'BTstack separate personal/noncommercial license retained; modified hid_host.c documented'}
(w/'verification-nova23.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: Nova23 final ARM binaries, header, retained bootloader, full-stack negotiation and test evidence match.')
