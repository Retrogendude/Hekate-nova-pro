"""Measure old/new animation invalidation using the real LVGL software renderer.

CPU timings are host-only observations, not console FPS. Pixel/region counts
exercise LVGL's actual invalidation FIFO, merging, drawing and display flush.
The v7 background fixture is embedded so the regression remains reproducible.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import json
import re
import subprocess
from zipfile import ZipFile

WORK=Path(__file__).resolve().parent
PROJECT=WORK/'hekate-6.5.4'
DIRECTORY=WORK/'background-performance-check'
DIRECTORY.mkdir(exist_ok=True)
with ZipFile(WORK/'hekate_ctcaer_6.5.4_Nyx_1.9.4.zip') as resources:
    (DIRECTORY/'res.pak').write_bytes(resources.read('bootloader/sys/res.pak'))
LVGL=PROJECT/'bdk/libs/lvgl'
LEGACY='/* Unified language/readability controls and restrained native backgrounds.\n * SPDX-License-Identifier: GPL-2.0-or-later */\n#include "gui.h"\n#include "gui_appearance.h"\n#include "../config.h"\n#include "../fonts/font_manager.h"\n#include <libs/lvgl/lv_misc/lv_math.h>\n#include <string.h>\n\n/* Background drawing owns fixed storage and one low-priority 5fps task.\n * Integer sine and a handful of short polylines keep the BPMP available for\n * input. No framebuffer allocations or per-frame heap calls are required. */\n#define FX_LINES 20\n#define FX_POINTS 49\nstatic struct {\n    lv_obj_t *holder, *lines[FX_LINES];\n    lv_style_t styles[FX_LINES];\n    lv_point_t points[FX_LINES][FX_POINTS];\n    lv_task_t *task;\n    u32 phase, effect, speed, opacity, pause_depth;\n    lv_signal_func_t original_signal;\n} background;\n\nstatic u32 bounded(u32 value, u32 low, u32 high, u32 fallback)\n{\n    return value >= low && value <= high ? value : fallback;\n}\n\nstatic void background_frame(void *unused)\n{\n    (void)unused;\n    if (!background.holder || background.pause_depth || !background.effect) return;\n    /* Keep a monotonic phase so particles do not jump back every 360 steps;\n     * the sine paths reduce their angle independently. */\n    background.phase += background.speed * 2;\n    u32 count = background.effect == 2 ? 20 : (background.effect == 3 ? 7 : 4);\n    for (u32 line = 0; line < count; ++line) {\n        u32 point_count = background.effect == 2 ? 2 : FX_POINTS;\n        if (background.effect == 2) {\n            u32 x = (line * 137 + background.phase * (line % 3 + 1)) % (LV_HOR_RES - 16);\n            u32 y = (line * 89 + 35 + background.phase / 2) % (LV_VER_RES - 16);\n            background.points[line][0] = (lv_point_t){x, y};\n            background.points[line][1] = (lv_point_t){x + 4 + line % 4, y + 2};\n        } else {\n            for (u32 point = 0; point < FX_POINTS; ++point) {\n                int x = point * LV_HOR_RES / (FX_POINTS - 1);\n                int angle = (point * (background.effect == 3 ? 8 : 6) + background.phase + line * 19) % 360;\n                int amplitude = background.effect == 3 ? 62 : 26;\n                int baseline = background.effect == 3 ? 300 + line * 34 : 385 + line * 72;\n                int y = baseline + lv_trigo_sin(angle) * amplitude / LV_TRIGO_SIN_MAX;\n                background.points[line][point] = (lv_point_t){x, y};\n            }\n        }\n        /* Limit invalidation to the actual drawing. In particular, twenty\n         * moving particles must not each invalidate a full 1280x720 canvas. */\n        int left = LV_HOR_RES, top = LV_VER_RES, right = 0, bottom = 0;\n        for (u32 point = 0; point < point_count; ++point) {\n            lv_point_t p = background.points[line][point];\n            if (p.x < left) left = p.x;\n            if (p.x > right) right = p.x;\n            if (p.y < top) top = p.y;\n            if (p.y > bottom) bottom = p.y;\n        }\n        int padding = background.effect == 3 ? 8 : 3;\n        for (u32 point = 0; point < point_count; ++point) {\n            background.points[line][point].x -= left - padding;\n            background.points[line][point].y -= top - padding;\n        }\n        lv_obj_set_pos(background.lines[line], left - padding, top - padding);\n        lv_obj_set_size(background.lines[line], right - left + 2 * padding + 1, bottom - top + 2 * padding + 1);\n        lv_line_set_points(background.lines[line], background.points[line], point_count);\n    }\n}\n\nstatic lv_res_t background_signal(lv_obj_t *object, lv_signal_t signal, void *param)\n{\n    lv_res_t result = background.original_signal(object, signal, param);\n    if (signal == LV_SIGNAL_CLEANUP) {\n        if (background.task) lv_task_del(background.task);\n        background.task = NULL; background.holder = NULL;\n    }\n    return result;\n}\n\nvoid nyx_background_create(lv_obj_t *screen)\n{\n    if (background.holder) lv_obj_del(background.holder);\n    memset(&background, 0, sizeof(background));\n    background.effect = n_cfg.background_effect < 4 ? n_cfg.background_effect : 0;\n    background.speed = bounded(n_cfg.background_speed, 1, 3, 2);\n    background.opacity = bounded(n_cfg.background_opacity, 10, 50, 20);\n    if (!background.effect) return;\n    background.holder = lv_obj_create(screen, NULL);\n    lv_obj_set_style(background.holder, &lv_style_transp);\n    lv_obj_set_size(background.holder, LV_HOR_RES, LV_VER_RES);\n    lv_obj_set_pos(background.holder, 0, 0);\n    lv_obj_set_click(background.holder, false);\n    background.original_signal = lv_obj_get_signal_func(background.holder);\n    lv_obj_set_signal_func(background.holder, background_signal);\n    u32 count = background.effect == 2 ? 20 : (background.effect == 3 ? 7 : 4);\n    for (u32 i = 0; i < count; ++i) {\n        lv_style_copy(&background.styles[i], &lv_style_plain);\n        background.styles[i].body.empty = 1;\n        background.styles[i].body.border.width = 0;\n        background.styles[i].line.width = background.effect == 3 ? 12 : 2;\n        background.styles[i].line.rounded = 1;\n        background.styles[i].line.opa = background.opacity * 255 / 100;\n        lv_color_t accent = lv_theme_get_current()->btn.pr->text.color;\n        background.styles[i].line.color = background.effect == 3 ?\n            lv_color_mix(accent, LV_COLOR_HEX(0x5878D8), 80 + i * 20) : accent;\n        background.lines[i] = lv_line_create(background.holder, NULL);\n        lv_line_set_style(background.lines[i], &background.styles[i]);\n        lv_line_set_auto_size(background.lines[i], false);\n        lv_obj_set_size(background.lines[i], LV_HOR_RES, LV_VER_RES);\n        lv_obj_set_pos(background.lines[i], 0, 0);\n        lv_obj_set_click(background.lines[i], false);\n    }\n    background_frame(NULL);\n    background.task = lv_task_create(background_frame, 200, LV_TASK_PRIO_LOWEST, NULL);\n}\n\nvoid nyx_background_pause(bool pause)\n{\n    if (pause) ++background.pause_depth;\n    else if (background.pause_depth) --background.pause_depth;\n    if (background.task)\n        lv_task_set_prio(background.task, background.pause_depth ? LV_TASK_PRIO_OFF : LV_TASK_PRIO_LOWEST);\n}\n\n'
current=(PROJECT/'nyx/nyx_gui/frontend/gui_appearance.c').read_text(encoding='utf-8')
current=current[:current.index('static nyx_config appearance_pending;')]
config=(PROJECT/'bdk/libs/lv_conf.h').read_text()
config=config.replace('#include <soc/timer.h>','#include <utils/types.h>').replace('#include <memory_map.h>','')
config=config.replace('#define LV_MEM_CUSTOM         0','#define LV_MEM_CUSTOM         1').replace('#  define LV_MEM_CUSTOM_INCLUDE <mem/heap.h>','#  define LV_MEM_CUSTOM_INCLUDE <stdlib.h>')
config=config.replace('#define LV_VDB_ADR          NYX_LV_VDB_ADR','#define LV_VDB_ADR          0').replace('#define LV_TICK_CUSTOM               1','#define LV_TICK_CUSTOM               0')
(DIRECTORY/'lv_conf.h').write_text(config)
common=r'''
#include <windows.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libs/lvgl/lvgl.h>
#include <libs/lvgl/lv_core/lv_refr.h>
#include <libs/lvgl/lv_misc/lv_math.h>
static struct { u32 background_effect,background_speed,background_opacity; } n_cfg;
'''
harness=r'''
static lv_color_t pixels[LV_HOR_RES*LV_VER_RES];
static u32 refreshed,flushes,refreshes;
static u32 monitor_pixels;
static void flush(int32_t x1,int32_t y1,int32_t x2,int32_t y2,const lv_color_t *data) {
    assert(x1>=0&&y1>=0&&x2<LV_HOR_RES&&y2<LV_VER_RES&&x2>=x1&&y2>=y1);
    ++flushes;
    for(int32_t y=y1;y<=y2;y++) {
        memcpy(pixels+y*LV_HOR_RES+x1,data,(x2-x1+1)*sizeof *data);
        data+=x2-x1+1;
    }
}
static void flush_done(int32_t x1,int32_t y1,int32_t x2,int32_t y2,const lv_color_t *data) {
    flush(x1,y1,x2,y2,data);lv_flush_ready();
}
static void monitor(u32 ms,u32 count) { (void)ms;monitor_pixels+=count;++refreshes; }
int main(int argc,char **argv) {
    assert(argc==3);
    FILE *resource=fopen(argv[2],"rb");assert(resource);fseek(resource,0,SEEK_END);long bytes=ftell(resource);rewind(resource);
    void *memory=VirtualAlloc((void*)0xEE000000,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(memory==(void*)0xEE000000);
    assert(fread(memory,1,bytes,resource)==(size_t)bytes);fclose(resource);
    u32 effect=atoi(argv[1]);assert(effect>=1&&effect<=3);
    n_cfg.background_effect=effect;n_cfg.background_speed=2;n_cfg.background_opacity=30;
    lv_init();lv_disp_drv_t driver;lv_disp_drv_init(&driver);driver.disp_flush=flush_done;lv_disp_drv_register(&driver);
    lv_refr_set_monitor_cb(monitor);
    lv_theme_t *theme=lv_theme_hekate_init(0x141A19,0,&interui_20);lv_theme_set_current(theme);
    static lv_style_t base,card,footer;
    lv_style_copy(&base,&lv_style_plain);base.body.main_color=base.body.grad_color=LV_COLOR_HEX(0x141A19);
    lv_obj_set_style(lv_scr_act(),&base);nyx_background_create(lv_scr_act());
    lv_style_copy(&card,&lv_style_plain);card.body.main_color=card.body.grad_color=LV_COLOR_HEX(0x222B2A);card.body.opa=200;card.body.radius=12;
    for(int row=0;row<4;row++)for(int column=0;column<2;column++) {
        lv_obj_t *tile=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_style(tile,&card);
        lv_obj_set_pos(tile,30+column*630,55+row*145);lv_obj_set_size(tile,590,125);
        lv_obj_t *label=lv_label_create(tile,NULL);lv_label_set_text(label,"Memory diagnostics and console information");lv_obj_set_pos(label,20,18);
        lv_obj_t *detail=lv_label_create(tile,NULL);lv_label_set_text(detail,"Read hardware values and inspect the report.");lv_obj_set_pos(detail,20,66);
    }
    lv_style_copy(&footer,&card);footer.body.opa=180;footer.body.radius=0;
    lv_obj_t *bar=lv_obj_create(lv_scr_act(),NULL);lv_obj_set_style(bar,&footer);lv_obj_set_pos(bar,0,656);lv_obj_set_size(bar,1280,64);
    lv_obj_t *status=lv_label_create(bar,NULL);lv_label_set_text(status,"Hekate 6.5.4    Nyx 1.9.4                          Battery 92%    16:58");lv_obj_set_pos(status,25,22);
    lv_refr_now();assert(lv_refr_get_buf_size()==0);
    for(int frame=0;frame<8;frame++){background_frame(NULL);lv_refr_now();}
    LARGE_INTEGER frequency,start,end;QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&start);
    u32 max_pixels=0,max_regions=0;
    const int frames=120;
    unsigned long long total_pixels=0;
    monitor_pixels=refreshes=flushes=0;
    for(int frame=0;frame<frames;frame++) {
        u32 before=monitor_pixels;background_frame(NULL);
        u32 regions=lv_refr_get_buf_size();if(regions>max_regions)max_regions=regions;
        lv_refr_now();u32 count=monitor_pixels-before;if(count>max_pixels)max_pixels=count;total_pixels+=count;
#ifndef BENCH_LEGACY
        assert(count<=(effect==2?1600:LV_HOR_RES*FX_BAND_HEIGHT));
        assert(regions<=(effect==2?16:1));
#endif
    }
    QueryPerformanceCounter(&end);
    u32 measured_flushes=flushes,measured_refreshes=refreshes;
#ifndef BENCH_LEGACY
    u32 phase=background.phase;u32 before=monitor_pixels;
    for(int frame=0;frame<50;frame++) {
        nyx_background_interaction();background_frame(NULL);lv_refr_now();lv_tick_inc(20);
        assert(background.phase==phase&&monitor_pixels==before&&lv_refr_get_buf_size()==0);
    }
    lv_tick_inc(FX_IDLE_MS);background_frame(NULL);assert(background.phase!=phase);lv_refr_now();
#endif
    printf("{\"effect\":%u,\"frames\":%d,\"mean_refreshed_pixels\":%.2f,\"max_refreshed_pixels\":%u,\"max_pending_regions\":%u,\"flushes_per_frame\":%.2f,\"refreshes\":%u,\"host_mean_ms\":%.4f}\n",effect,frames,(double)total_pixels/frames,max_pixels,max_regions,(double)measured_flushes/frames,measured_refreshes,(double)(end.QuadPart-start.QuadPart)*1000.0/frequency.QuadPart/frames);
    lv_obj_del(background.holder);assert(background.holder==NULL&&background.task==NULL);
    return 0;
}
'''
cc=WORK/'toolchains/w64devkit/bin/gcc.exe'
flags=['-std=gnu11','-O2','-include','stdint.h','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(DIRECTORY),'-I'+str(PROJECT/'bdk')]
def compile_one(path):
    target=DIRECTORY/(path.stem+'.o')
    run=subprocess.run([str(cc),*flags,'-c',str(path),'-o',str(target)],capture_output=True,text=True)
    if run.returncode:raise RuntimeError(str(path)+'\n'+run.stderr)
    return target
with ThreadPoolExecutor(max_workers=6)as pool:objects=list(pool.map(compile_one,LVGL.rglob('*.c')))
results={}
for name,source in [('legacy',LEGACY),('optimized',current)]:
    stripped=re.sub(r'^#include[^\n]*\n','',source,flags=re.M)
    path=DIRECTORY/(name+'.c');path.write_text(('#define BENCH_LEGACY\n' if name=='legacy' else '')+common+stripped+harness,encoding='utf-8')
    obj=compile_one(path)
    exe=DIRECTORY/(name+'.exe');subprocess.run([str(cc),*[str(o)for o in objects],str(obj),'-o',str(exe)],check=True)
    results[name]=[]
    for effect in (1,2,3):
        run=subprocess.run([str(exe),str(effect),str(DIRECTORY/'res.pak')],capture_output=True,text=True,check=True,timeout=45)
        results[name].append(json.loads(run.stdout))
for old,new in zip(results['legacy'],results['optimized']):
    assert new['mean_refreshed_pixels']<old['mean_refreshed_pixels']/3
    new['pixel_reduction_percent']=round(100*(1-new['mean_refreshed_pixels']/old['mean_refreshed_pixels']),2)
results['method']='Actual LVGL 5 invalidation, merging, software drawing and flush in the same transparent-card scene. Host timing is not console FPS.'
(DIRECTORY/'results.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf-8')
print('PASS: real-LVGL background regression; all three effects cut refreshed pixels by >66%, <=16 regions, no input-time animation invalidation, cleanup releases tasks.')
for old,new in zip(results['legacy'],results['optimized']):
    print(f"Effect {new['effect']}: {old['mean_refreshed_pixels']:.0f} -> {new['mean_refreshed_pixels']:.0f} mean pixels/frame (-{new['pixel_reduction_percent']}%); host {old['host_mean_ms']:.3f} -> {new['host_mean_ms']:.3f} ms")
