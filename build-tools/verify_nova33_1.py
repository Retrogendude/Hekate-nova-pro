from pathlib import Path
import json,hashlib,struct,re
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.1';OLD=W/'hekate-nova-pro-v32'
sha=lambda data:hashlib.sha256(data).hexdigest()
old=json.loads((OLD/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'));new=json.loads((P/'nyx/nyx_gui/localization_catalog.json').read_text(encoding='utf-8'))
assert all(new[k][:7]==v for k,v in old.items()),'Existing translations changed'
assert all(len(v)==8 for v in new.values())
fmt=re.compile(r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hlLzjt])?[diuoxXfFeEgGaAcspn%])')
for k,v in new.items():assert all(fmt.findall(x)==fmt.findall(k)for x in v),k
f=P/'nyx/nyx_gui/fonts/sd/Nova-Chinese-Simplified.nyxf';data=f.read_bytes();assert data[:4]==b'NYXF'
assert struct.unpack_from('<I',data,8)[0]==len(data)<16*1024*1024
def fnv(b):
 h=2166136261
 for x in b:h=((h^x)*16777619)&0xffffffff
 return h
assert fnv(data[:28])==struct.unpack_from('<I',data,28)[0]
sections=struct.unpack_from('<I',data,12)[0];assert sections==12
assert fnv(data[32:32+sections*32])==struct.unpack_from('<I',data,24)[0]
required={ord(c)for v in new.values()for c in v[7]if ord(c)>=32}
for i in range(sections):
 height,count,start,bitmap,size,end,check,reserved=struct.unpack_from('<8I',data,32+i*32)
 assert count<=2048 and not reserved and bitmap==start+count*12 and end-bitmap==size and end-start<=2*1024*1024
 assert fnv(data[start:end])==check
 cps={struct.unpack_from('<I',data,start+j*12)[0]for j in range(count)}
 assert required<=cps,(height,required-cps)
for file in ['nyx.bin','hekate.bin','hekate_unc.bin']:
 b=(P/'output'/file).read_bytes();offset,expected=(0x98,b'GCTC195')if file=='nyx.bin'else(0x118,b'ICTC655');assert b[offset:offset+7]==expected
assert (P/'output/hekate.bin').stat().st_size<=130392
assert (W/'HWTest-RCM-nova33_1/build-ram/hwtest_ram.bin').stat().st_size<=130392
ram8=json.loads((W/'nova33_1-8gb-check.json').read_text());assert ram8['passed']
tests={}
for name in ['nova33_1-font-check.json','nova33_1-ui-check.json','nova33_1-header-check.json']:
 t=json.loads((W/name).read_text());assert t['passed'],name;tests[name]=t
for n,h in tests['nova33_1-ui-check.json']['source_sha256'].items():assert sha((P/n).read_bytes())==h,(n,'UI evidence stale')
assert tests['nova33_1-ui-check.json']['native_scenes']==683
assert not json.loads((W/'nova33_1-ram-ui.json').read_text())['missing_translations']
options=(P/'nyx/nyx_gui/frontend/gui_options.c').read_text(encoding='utf-8')
assert options.count('lv_slider_set_action(brightness, _slider_brightness_action)')==1
assert options.count('_settings_toggle(card, SYMBOL_CHIP" Auto NoGC"')==1
assert options.count('_settings_toggle(card, SYMBOL_POWER" Auto HOS Power Off"')==1
assert 'lv_list_create'not in options[options.index('/* Paged boot entries'):options.index('static lv_res_t _autoboot_hide_delay_action(')]
unchanged=[]
for prefix in ['nyx/nyx_gui/bluetooth','nyx/nyx_gui/libs/btstack','nyx/nyx_gui/diagnostics/vendor','bdk/storage','bdk/mem']:
 for p in (OLD/prefix).rglob('*'):
  if p.is_file():n=p.relative_to(OLD).as_posix();assert (P/n).read_bytes()==p.read_bytes(),n;unchanged.append(n)
for n in ['nyx/nyx_gui/frontend/gui_bluetooth.c','nyx/nyx_gui/frontend/gui_partition_nova.inc','nyx/nyx_gui/frontend/gui_tools_partition_manager.c']:
 assert (P/n).read_bytes()==(OLD/n).read_bytes();unchanged.append(n)
source={p.relative_to(P).as_posix():sha(p.read_bytes())for p in P.rglob('*')if p.is_file()and not any(s in ('build','output','.git','__pycache__')for s in p.relative_to(P).parts)and p.relative_to(P).as_posix()not in ('loader/payload_00.h','loader/payload_01.h')}
changed=[n for n in source if(OLD/n).is_file()and sha((OLD/n).read_bytes())!=source[n]]
info={'version':'33.1-beta','passed':True,'base':'Hekate 6.5.5 / Nyx 1.9.5','languages':8,'catalog_entries':len(new),'existing_seven_translations_preserved':True,'chinese_font_bytes':len(data),'chinese_font_glyphs':count,'complete_chinese_catalog_glyph_coverage':True,'ram8_checks':ram8,'native_ui_scenes':tests['nova33_1-ui-check.json']['native_scenes'],'font_check':tests['nova33_1-font-check.json']['result'],'ram_locale_check':'63477 actual-C assertions across eight packs; see test script and log','physical_console_test':False,'linguistic_review':'Main labels, selected warnings and RAM UI reviewed; long reports retain machine translations pending linguistic review','github_publication':'authorized beta publication; hardware 8GB validation pending','changed_existing_sources':changed,'unchanged_low_level_and_wireless_files':len(unchanged),'tests':list(tests)+['check_nova33_1_ram_locale.py'],'source_sha256':source,'binaries':{n:{'bytes':(P/'output'/n).stat().st_size,'sha256':sha((P/'output'/n).read_bytes())}for n in ['nyx.bin','hekate.bin','hekate_unc.bin','libsys_lp0.bso','libsys_minerva.bso']},'ram_payload':{'bytes':(W/'HWTest-RCM-nova33_1/build-ram/hwtest_ram.bin').stat().st_size,'sha256':sha((W/'HWTest-RCM-nova33_1/build-ram/hwtest_ram.bin').read_bytes())}}
(W/'verification-nova33_1.json').write_text(json.dumps(info,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: v33 binaries, complete Chinese glyph coverage, preserved translations, 683 native scenes, no duplicated boot controls and',len(unchanged),'unchanged low-level/wireless files')
