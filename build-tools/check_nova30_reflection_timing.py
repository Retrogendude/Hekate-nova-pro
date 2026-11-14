"""Native LVGL regression: real scene builders, firmware refresh cadence.

The established V29 preview supplies only its hardware substitutions and scene
builders. The production V30 decoration and Nova builders are injected afresh.
An instrumented PRIVATE copy of lv_refr.c counts FIFO overflows; production LVGL
and the existing renderer are never edited. A V29 decoration is the before-fix
control. Outputs are host redraw counts, never console frame-rate estimates.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import os
import re
import subprocess

work = Path(__file__).resolve().parent
project = work / 'hekate-nova-pro-v30'
old_project = work / 'hekate-nova-pro-v29'
preview = work / 'lvgl-preview-nova29'
out = work / 'lvgl-reflection-nova30'
out.mkdir(exist_ok=True)
cc = work / 'toolchains/w64devkit/bin/gcc.exe'
nyx = project / 'nyx/nyx_gui'
lvgl = project / 'bdk/libs/lvgl'


def stripped(path):
    source=path.read_text(encoding='utf-8')
    if path.name=='gui_nova.c':
        source=source.replace('lv_obj_t *label = lv_label_create(parent, NULL);','if(getenv("NYX_TRACE"))fprintf(stderr,"NOVA TEXT: %s\\n",text);\n    lv_obj_t *label = lv_label_create(parent, NULL);')
        for asset in ['nova_reference.inc','nova_icons.inc']:
            source=source.replace('#include "nova_assets/'+asset+'"',(path.parent/'nova_assets'/asset).read_text(encoding='utf-8'))
    return re.sub(r'^#include[^\n]*\n', '', source, flags=re.M)


config = (preview / 'lv_conf.h').read_text()
(out / 'lv_conf.h').write_text(config)
refr = (lvgl / 'lv_core/lv_refr.c').read_text()
refr = refr.replace('static uint32_t px_num;', '''static uint32_t px_num;
unsigned audit_overflows, audit_peak, audit_refresh_ms = 33;
''')
refr = refr.replace('lv_task_create(lv_refr_task, LV_REFR_PERIOD,',
                    'lv_task_create(lv_refr_task, audit_refresh_ms,')
needle = '''inv_buf_p = 0;
            lv_area_copy(&inv_buf[inv_buf_p].area, &scr_area);'''
assert refr.count(needle) == 1
refr = refr.replace(needle, '''audit_overflows++;
            inv_buf_p = 0;
            lv_area_copy(&inv_buf[inv_buf_p].area, &scr_area);''')
refr = refr.replace('inv_buf_p ++;', 'inv_buf_p ++; if(inv_buf_p > audit_peak) audit_peak = inv_buf_p;')
(out / 'lv_refr.c').write_text(refr)

test = r'''
if(!strcmp(argv[3],"tools")||!strcmp(argv[3],"home")||!strcmp(argv[3],"carousel")){
 /* Compare deterministic pixels at identical phases, independently of timing. */
 for(unsigned pass=0;pass<3;pass++){
  unsigned phase=pass==0?0:pass==1?5:16;
  for(unsigned i=0;i<REFLECTION_TARGETS;i++)if(reflection_targets[i].target){
   reflection_target *target=&reflection_targets[i];target->phase=phase;
   target->painted=reflection_sweeping(phase)&&lv_obj_get_width(target->target)>=100&&lv_obj_get_height(target->target)>=64;
   if(target->painted)reflection_bounds(target);lv_obj_invalidate(target->overlay);
  }
  lv_refr_now();unsigned hash=2166136261u;
  for(unsigned i=0;i<LV_HOR_RES*LV_VER_RES;i++){hash^=screen_pixels[i].full;hash*=16777619u;}
  printf("PIXELS phase=%u hash=%08x\n",phase,hash);
 }
 lv_refr_now();audit_overflows=0;audit_peak=0;
 /* Existing widget invalidations must survive a crowded effect callback. */
 for(unsigned i=0;i<30;i++){lv_area_t marker={2*i,700,2*i,700};lv_inv_area(&marker);}
 reflection_frame(NULL);
 printf("CROWDED queue=%u overflow=%u\n",lv_refr_get_buf_size(),audit_overflows);
 lv_refr_now();audit_overflows=0;audit_peak=0;rendered_pixels=0;
 audit_max_pixels=audit_frames=audit_full_frames=0;audit_previous_pixels=0;
 lv_refr_set_monitor_cb(audit_monitor);
 unsigned start=lv_tick_get();
 for(unsigned ms=0;ms<REFLECTION_CYCLE*REFLECTION_PERIOD;ms++){
  lv_tick_inc(1);lv_task_handler();
 }
 unsigned elapsed=lv_tick_elaps(start);
 printf("TIMING refresh=%u elapsed=%u phase=%u overflow=%u peak=%u frames=%u full_frames=%u max_pixels=%lu total_pixels=%lu\n",
  audit_refresh_ms,elapsed,reflection_phase,audit_overflows,audit_peak,audit_frames,audit_full_frames,audit_max_pixels,rendered_pixels);
 /* Pausing must settle, then cease all decorative redraws. */
 nyx_background_pause(true);lv_refr_now();rendered_pixels=0;
 for(unsigned ms=0;ms<250;ms++){lv_tick_inc(1);lv_task_handler();}
 printf("PAUSED pixels=%lu\n",rendered_pixels);
 printf("PASS UI %s\n",argv[3]);return 0;
}
'''
metrics = r'''
extern unsigned audit_overflows,audit_peak,audit_refresh_ms;
static unsigned long audit_max_pixels,audit_previous_pixels;
static unsigned audit_frames,audit_full_frames;
static void audit_monitor(uint32_t elapsed,uint32_t pixels){
 (void)elapsed;(void)pixels;unsigned long n=rendered_pixels-audit_previous_pixels;
 audit_previous_pixels=rendered_pixels;audit_frames++;if(n>audit_max_pixels)audit_max_pixels=n;
 if(n>=LV_HOR_RES*LV_VER_RES)audit_full_frames++;
}
'''
base = (preview / 'preview.c').read_text(encoding='utf-8')
start = base.index('if((!strcmp(argv[3],"tools")')
end = base.index('if(getenv("NYX_TEST_HANDS"))', start)
base = base[:start] + test + base[end:]
main = 'int main(int argc,char**argv){'
assert base.count(main) == 1
base = base.replace(main, metrics + '\n' + main + '\naudit_refresh_ms=atoi(getenv("NOVA_AUDIT_REFRESH_MS"));')
for name in ['gui_nova.c', 'gui_nova_reflection.c']:
    old = stripped(old_project / 'nyx/nyx_gui/frontend' / name)
    assert base.count(old) == 1, name
    if name == 'gui_nova.c':
        base = base.replace(old, stripped(nyx / 'frontend' / name))
    else:
        old_reflection = old

flags = ['-std=gnu11', '-O1', '-include', 'stdint.h', '-DLV_CONF_INCLUDE_SIMPLE',
         '-I' + str(out), '-I' + str(project / 'bdk'), '-I' + str(nyx),
         '-I' + str(lvgl / 'lv_core')]
sources = [p for p in lvgl.rglob('*.c') if p.name != 'lv_refr.c'] + [out / 'lv_refr.c',
          nyx / 'fonts/unicode_fonts.c', nyx / 'frontend/nova_image.c', nyx / 'libs/png/lodepng_nyx.c']


def compile_one(source):
    obj = out / (source.stem + '.o')
    result = subprocess.run([str(cc), *flags, '-c', str(source), '-o', str(obj)], capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(str(source) + '\n' + result.stderr)
    return obj


with ThreadPoolExecutor(max_workers=6) as pool:
    objects = list(pool.map(compile_one, sources))
results = []
for variant in ['before', 'fixed']:
    source = out / (variant + '.c')
    source.write_text(base.replace(old_reflection, old_reflection if variant == 'before' else stripped(nyx / 'frontend/gui_nova_reflection.c')), encoding='utf-8')
    obj = compile_one(source)
    exe = out / (variant + '.exe')
    subprocess.run([str(cc), *map(str, objects), str(obj), '-o', str(exe)], check=True)
    for scene in ['home', 'tools', 'carousel']:
        for refresh in [33, 66, 100]:
            env = os.environ.copy()
            env.update(NYX_PREVIEW_NO_DUMP='1', NOVA_AUDIT_REFRESH_MS=str(refresh))
            run = subprocess.run([str(exe), str(nyx / 'fonts/sd'), str(preview / 'res.pak'), scene, '0', '30', '0'],
                                 cwd=out, env=env, capture_output=True, text=True, timeout=60)
            if run.returncode:
                raise RuntimeError(run.stdout + run.stderr)
            timing = re.search(r'TIMING (.+)', run.stdout)
            assert timing, run.stdout
            row = dict(re.findall(r'(\w+)=(\d+)', timing[1]))
            row = {k: int(v) for k, v in row.items()}
            row.update(variant=variant, scene=scene,
                       pixels=re.findall(r'PIXELS phase=(\d+) hash=(\w+)', run.stdout),
                       crowded=dict((k, int(v)) for k, v in re.findall(r'(queue|overflow)=(\d+)', re.search(r'CROWDED (.+)', run.stdout)[1])))
            assert 'PAUSED pixels=0' in run.stdout, run.stdout
            if variant == 'before':
                assert row['overflow'] > 0, row
            else:
                assert row['overflow'] == row['full_frames'] == 0 and row['peak'] <= 31, row
                assert row['crowded'] == {'queue': 30, 'overflow': 0}, row
                assert row['phase'] >= row['elapsed'] // 20, row
                prior = next(r for r in results if r['variant'] == 'before' and r['scene'] == scene and r['refresh'] == refresh)
                assert prior['pixels'] == row['pixels'], (prior, row)
            results.append(row)
            print(json.dumps(row), flush=True)
report = {'platform': 'native host LVGL; substituted hardware; no console FPS claim', 'results': results,
          'production_source_sha256': hashlib.sha256((nyx / 'frontend/gui_nova_reflection.c').read_bytes()).hexdigest()}
(out / 'results.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
report['passed']=True
(work/'nova30-reflection-check.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('PASS: old overflow reproduced; fixed FIFO bounded at 33/66/100 ms; same phase pixels and clean pause.')
