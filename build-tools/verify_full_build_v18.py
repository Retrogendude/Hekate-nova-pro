"""Current binary provenance and regressions relevant to the cahier des charges."""
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import hashlib,json,re,subprocess,sys
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';out=project/'output';arm=Path('C:/devkitPro/devkitARM/bin')
scripts=['check_partition_plan_v18.py','check_brightness_v16.py','check_nova_pro_config_v18.py','check_nova_save.py',
 'check_nyx_config_writer_v18.py','check_language_config_v18.py','check_language_actions.py','check_appearance_config_v18.py','check_appearance_actions_v15.py',
 'check_nova_transaction_v18.py','check_nova_v13_list.py','check_theme_actions_v18.py','check_nova_wallpaper_v18.py',
 'check_diag_format.py','check_probe_translations.py','check_unicode_fonts.py','check_gui_readability.py',
 'check_display_present.py','check_packed_hekate.py','check_font_converter_v16.py','check_ram_locale.py','check_report_paging.py',
 'check_payload_reader.py','test_native_backend_v17.py','test_native_verdict_host.py','check_ram_modes_v17.py','check_ram_payload.py',
 'check_animation_v18.py','check_navigation_v18.py','check_boot_v17.py','check_components_v17.py','check_component_export_v17.py','check_reflection_timing_v17.py']
def run(name):
    result=subprocess.run([sys.executable,str(work/name)],capture_output=True,text=True)
    return name,result
logs={};failures=[]
if '--resume' in sys.argv:
    text=(work/'verification-v18.txt').read_text(encoding='utf-8')
    for entry in text.split('\n\n'):
        name,_,body=entry.lstrip().partition('\n')
        if name in scripts and name not in ('check_boot_v17.py','check_packed_hekate.py') and 'PASS:' in body and 'Traceback' not in body and 'FAIL:' not in body:logs[name]=body
with ThreadPoolExecutor(max_workers=4)as pool:
    for future in as_completed([pool.submit(run,name)for name in scripts if name not in logs]):
        name,result=future.result();logs[name]=result.stdout+result.stderr
        print(('PASS 'if not result.returncode else'FAILED ')+name,flush=True)
        if result.returncode:failures.append(name);print(logs[name][-4000:],flush=True)
(work/'verification-v18.txt').write_text('\n\n'.join(name+'\n'+logs[name]for name in scripts),encoding='utf-8')
if failures:raise SystemExit('Failed: '+', '.join(failures))
for target,name in [('build/loader/loader.elf','hekate.bin'),('build/nyx/nyx.elf','nyx.bin')]:
    image=work/('final-v18-'+name);subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(project/target),str(image)],check=True)
    expected=image.read_bytes()+(b'ICTC49'if name=='hekate.bin'else b'')
    assert(out/name).read_bytes()==expected
assert(out/'hekate.bin').stat().st_size<=126296
assert(out/'hekate_unc.bin').stat().st_size<=140288
assert(out/'nyx.bin').stat().st_size<=16*1024*1024
ram=work/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin';assert ram.stat().st_size<=130392
assert(project/'nyx/nyx_gui/diagnostics/ram_handoff.h').read_bytes()==(work/'HWTest-RCM-0.15/ramtest/source/nyx_ram_handoff.h').read_bytes()
symbols=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(project/'build/nyx/nyx.elf')],text=True)
probes=re.findall(r'\{\s*(probe_\w+)\s*,',(project/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text())
original=re.findall(r'\{\s*(probe_\w+)\s*,',(work/'HWTest-RCM-0.15/core.c').read_text());assert probes==original and len(probes)==42
assert all(re.search(r'\bT '+probe+r'$',symbols,re.M)for probe in probes)
for name in ['nyx_component_get','nyx_joycon_test','nyx_touch_test','nyx_screen_patterns','nyx_color_picker_create','nova_splash_stage','nova_partition_validate','nova_partition_simplified','nyx_nova_package_logs','nyx_nova_archive_view']:
    assert re.search(r'\bT '+name+r'$',symbols,re.M)
stub_symbol=re.search(r'^(\w+) [DR] cpu_stub_bin$',symbols,re.M);assert stub_symbol
stub=(project/'build/nyx/cpu_stub.bin').read_bytes();offset=int(stub_symbol[1],16)-0x81000000
assert len(stub)==8128 and(out/'nyx.bin').read_bytes()[offset:offset+len(stub)]==stub
version=re.search(r'#define NYX_NOVA_VERSION "([^"]+)"',(project/'nyx/nyx_gui/nova_version.h').read_text())[1]
ui=json.loads((work/'native-ui-v18.json').read_text());assert not ui['failures']
report={'product':'Hekate Nova Pro','version':version,'build_exit_code':0,'hardware_validation':'not performed',
 'native_ui':ui,'feature_checks':{name:json.loads((work/name).read_text()) for name in ['hands-check-v17.json','components-check-v17.json','components-ui-v17.json','picker-check-v17.json','boot-launch-ui-v17.json','ram-modes-v17.json','boot-check-v17.json','png-check-v18.json','changes-ui-v18.json']},'partition_planner':json.loads((work/'partition-check-v18.json').read_text()),'native_probe_count':42,'cpu_stub_bytes':len(stub),
 'first_start':{'wallpaper':'Nova-Carbon.bmp','window_opacity_percent':25,'window_transparency_percent':75,'existing_preferences':'preserved by existing INI parser'},
 'test_summaries':[line for name in scripts for line in logs[name].splitlines()if line.startswith('PASS:')],
 'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}for p in [out/'hekate.bin',out/'hekate_unc.bin',out/'nyx.bin',ram]},
 'source_sha256':{str(p.relative_to(project)):hashlib.sha256(p.read_bytes()).hexdigest()for p in project.rglob('*')if p.is_file()and not any(part in('build','output','.git','__pycache__')for part in p.relative_to(project).parts)},
 'ram_source_sha256':{str(p.relative_to(work/'HWTest-RCM-0.15')):hashlib.sha256(p.read_bytes()).hexdigest()for p in (work/'HWTest-RCM-0.15').rglob('*')if p.is_file()and not any(part in('build','build-ram','build-gui','output','.git','__pycache__')for part in p.relative_to(work/'HWTest-RCM-0.15').parts)}}
(work/'verification-v18.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print('PASS: final v18 binaries match linked ELF files; 42 probes and RAM service retained; storage, settings, fonts and UI checked.',flush=True)
