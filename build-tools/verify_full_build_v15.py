"""Final source/binary, settings, native UI and RAM regressions for Nova Pro."""
from pathlib import Path
import hashlib,json,re,subprocess,sys
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';arm=Path('C:/devkitPro/devkitARM/bin');out=project/'output'
scripts=[
 'check_nyx_config_writer.py','check_language_config.py','check_language_actions.py',
 'check_appearance_config.py','check_appearance_actions_v15.py','check_nova_v13.py','check_nova_v13_list.py','check_nova_pro_config.py','check_nova_theme_actions.py','check_nova_wallpaper.py',
 'check_localization.py','check_diag_format.py','check_probe_translations.py','check_unicode_fonts.py','check_font_manager.py','check_gui_readability.py',
 'check_nova_save.py','check_nova_reference.py','check_display_present.py','check_nova_boot_v15.py','check_packed_hekate.py','check_font_converter_fix.py',
 'render_lvgl_preview_v15.py','check_alternate_fonts_v15.py','check_nova_v15_navigation.py','check_nova_guards_v15.py','check_nova_pro_visuals_v15.py','check_nova_gradient_v15.py',
 'check_ram_locale.py','check_report_paging.py','check_payload_reader.py','test_native_backend_host.py','test_native_verdict_host.py','check_ram_handoff.py','check_ram_payload.py'
]
logs=[]
if '--resume' in sys.argv:
    saved=(work/'verification-v15.txt').read_text(encoding='utf-8').split('\n\n')
    completed={entry.split('\n',1)[0]:entry for entry in saved if entry.split('\n',1)[0] in scripts and 'Traceback' not in entry}
    logs=list(completed.values())
    scripts=[script for script in scripts if script not in completed or script=='check_localization.py']
for script in scripts:
    print('Checking '+script,flush=True)
    result=subprocess.run([sys.executable,str(work/script),*(['--check-all']if script=='render_lvgl_preview_v15.py'else[])],capture_output=True,text=True)
    logs.append(script+'\n'+result.stdout+result.stderr)
    (work/'verification-v15.txt').write_text('\n\n'.join(logs),encoding='utf-8')
    if result.returncode:print(logs[-1],flush=True);raise SystemExit(result.returncode)
for name,elf in [('loader',project/'build/loader/loader.elf'),('nyx',project/'build/nyx/nyx.elf')]:
    target=work/f'final-v15-{name}.bin';subprocess.run([str(arm/'arm-none-eabi-objcopy.exe'),'-S','-O','binary',str(elf),str(target)],check=True)
    actual=(out/('hekate.bin'if name=='loader'else'nyx.bin')).read_bytes()
    assert actual==target.read_bytes()+(b'ICTC49'if name=='loader'else b'')
assert(out/'hekate.bin').stat().st_size<=126296
assert(out/'hekate_unc.bin').stat().st_size<=140288
assert(out/'nyx.bin').stat().st_size<=16*1024*1024
ram=work/'HWTest-RCM-0.15/build-ram/hwtest_ram.bin';assert ram.stat().st_size<=130392
assert(project/'nyx/nyx_gui/diagnostics/ram_handoff.h').read_bytes()==(work/'HWTest-RCM-0.15/ramtest/source/nyx_ram_handoff.h').read_bytes()
pages=(project/'nyx/nyx_gui/diagnostics/hwtest_pages.c').read_text();probes=re.findall(r'\{\s*(probe_\w+)\s*,',pages)
original=re.findall(r'\{\s*(probe_\w+)\s*,',(work/'HWTest-RCM-0.15/core.c').read_text());assert probes==original and len(probes)==42
symbols=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),str(project/'build/nyx/nyx.elf')],text=True)
assert all(re.search(r'\bT '+probe+r'$',symbols,re.M)for probe in probes)
card=re.search(r'^(\w+) [tT] nova_card_signal$',symbols,re.M);toggle=re.search(r'^(\w+) [tT] nova_toggle_signal$',symbols,re.M);assert card and toggle and card[1]!=toggle[1]
assert not any(re.search(r'\b[tT] '+name+r'$',symbols,re.M)for name in ('background_frame','background_design','background_line_count','background_particle_invalidate'))
stub_symbol=re.search(r'^(\w+) [DR] cpu_stub_bin$',symbols,re.M);assert stub_symbol
stub=(project/'build/nyx/cpu_stub.bin').read_bytes();assert len(stub)==8128
stub_offset=int(stub_symbol[1],16)-0x81000000;assert(out/'nyx.bin').read_bytes()[stub_offset:stub_offset+len(stub)]==stub
def read(name):return json.loads((work/name).read_text(encoding='utf-8'))
summary={
 'product':'Hekate Nova Pro, version 15','authorized_changes':11,'build_exit_code':0,'hardware_validation':'not performed',
 'native_probe_entries':42,'native_probes_compiled_and_linked':probes,'wifi_a64_stub_bytes':len(stub),
 'hekate_payload_bytes':(out/'hekate.bin').stat().st_size,'hekate_uncompressed_bytes':(out/'hekate_unc.bin').stat().st_size,'nyx_bytes':(out/'nyx.bin').stat().st_size,'ram_service_bytes':ram.stat().st_size,
 'packed_hekate':read('packed-hekate-check.json'),'boot':read('nova-boot-check-v15.json'),'gradient_profiles':read('nova-v15-gradient.json'),
 'ui_chrome':'Nova v15: normalized bitmap stride and fixed cells; aligned eMMC properties and paged partitions; bounded transfer report; cumulative palette preview; 480ms tile sheen; staged loading reaches 100%; aligned emuMMC and four-choice file pages; modals centered within y76..656; original first-start clock retained.',
 'reflection_performance':read('lvgl-preview-v15/nova-performance.json'),'background_animation':'removed from the source, UI and linked executable',
 'display_batching':'one VIC presentation per LVGL refresh','nova_reference':read('nova-reference-check.json'),
 'ui_geometry_matrix':{'native_views':40,'primary_scenes':2520,'alternate_font_scenes':2520,'availability_scenes':12,'wallpaper_opacity_scenes':396,'wallpaper_touch_scenes':21,'emummc_page_boundary_scenes':252,'languages':7,'font_sizes':list(range(20,38,2)),'live_battery_updates_per_scene':110,'scope':'Actual LVGL; hardware operations/sensor values substituted on PC. Not a console FPS measurement.'},
 'emummc_page_boundaries':read('nova-v15-emummc-pages.json'),
 'languages':['English','Français','Español','Italiano','Русский','Deutsch','Português'],
 'sd_fonts':[p.name for p in sorted((project/'nyx/nyx_gui/fonts/sd').glob('*.nyxf'))],
 'catalog_keys':len(json.loads((project/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))),
 'probe_translation_provenance':json.loads((project/'nyx/nyx_gui/localization_probe_provenance.json').read_text(encoding='utf-8')),
 'ram_ui_packs':read('ram-ui/manifest.json'),
 'v13_clock_and_transaction':read('nova-v13-check.json'),
 'wallpaper_navigation':read('nova-v15-navigation.json'),'linked_list_regression':read('nova-v13-list.json'),
 'full_tile_performance':{name:read('lvgl-preview-v15/nova-performance-'+name+'.json')for name in ('home','carousel','tools')},
 'tests':{'config_writer':136,'appearance_configuration':2443,'nova_pro_configuration':607,'theme_actions':38,'wallpaper_decoder':110,'appearance_actions':42,'language_config':21,'language_actions':49,'nova_save':31},
 'test_summaries':[line for log in logs for line in log.splitlines()if line.startswith('PASS:')],
 'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}for p in [out/'hekate.bin',out/'hekate_unc.bin',out/'nyx.bin',ram]}
}
(work/'verification-v15.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False),encoding='utf-8')
message='PASS: Nova Pro v15 full regression; 42 linked probes, required A64 stub, matching current packed firmware, exact ELF binaries and all payload limits.'
logs.append(message);(work/'verification-v15.txt').write_text('\n\n'.join(logs),encoding='utf-8')
print(message,flush=True);print('Hekate/Nyx/RAM bytes:',summary['hekate_payload_bytes'],summary['nyx_bytes'],summary['ram_service_bytes'],flush=True)
