/* Soft white diagonal sheen over the complete selected tile.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_nova.h"
#include "../config.h"
#include <libs/lvgl/lv_draw/lv_draw_vbasic.h>
#include <string.h>
#define REFLECTION_TARGETS 32
#define REFLECTION_PERIOD 40
#define REFLECTION_SWEEP 12
#define REFLECTION_CYCLE 72
#define REFLECTION_STRIPS 10
#define REFLECTION_HALF 64
typedef struct {
    lv_obj_t *target,*overlay;
    lv_area_t dirty[REFLECTION_STRIPS];
    u32 phase,dirty_count;
    bool painted;
} reflection_target;
static reflection_target reflection_targets[REFLECTION_TARGETS];
static lv_task_t *reflection_task;
static lv_signal_func_t reflection_base_signal;
static u32 reflection_phase,reflection_pause_depth,reflection_interaction_tick;
static bool reflection_interacting;
static bool reflection_visible(lv_obj_t *target, lv_area_t *visible)
{
    *visible = (lv_area_t){0, 0, LV_HOR_RES - 1, LV_VER_RES - 1};
    for (lv_obj_t *object = target; object; object = lv_obj_get_parent(object)) {
        if (lv_obj_get_hidden(object)) return false;
        lv_area_t coords, overlap;
        lv_obj_get_coords(object, &coords);
        if (!lv_area_intersect(&overlap, &coords, visible)) return false;
        *visible = overlap;
    }
    /* Screen-level windows/modal covers obscure the content below them.
     * The top navigation remains visible above a Nova window. */
    lv_obj_t *root = target;
    while (lv_obj_get_parent(root) && lv_obj_get_parent(root) != lv_scr_act()) root = lv_obj_get_parent(root);
    for (lv_obj_t *child = lv_obj_get_child(lv_scr_act(), NULL); child && child != root; child = lv_obj_get_child(lv_scr_act(), child)) {
        if (lv_obj_get_hidden(child)) continue;
        lv_area_t coords, overlap;
        lv_obj_get_coords(child, &coords);
        if (lv_area_intersect(&overlap, visible, &coords) &&
            coords.x1 <= visible->x1 && coords.x2 >= visible->x2 &&
            coords.y1 <= visible->y1 && coords.y2 >= visible->y2) {
            lv_obj_type_t type; lv_obj_get_type(child, &type);
            if ((type.type[0] && !strcmp(type.type[0], "lv_win")) ||
                (lv_obj_get_click(child) && lv_obj_get_width(child) >= LV_HOR_RES - 2 && lv_obj_get_height(child) >= NYX_NOVA_BOTTOM)) return false;
        }
    }
    return true;
}




/* One reusable 129-pixel row. Raised-cosine opacity reaches zero at both
 * edges. Only this 516-byte constant is sampled; no tile-sized image. */
static const u32 reflection_gradient_row[]={
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,
    0x02FFFFFF,0x03FFFFFF,0x03FFFFFF,0x04FFFFFF,0x04FFFFFF,0x05FFFFFF,0x05FFFFFF,0x06FFFFFF,
    0x06FFFFFF,0x07FFFFFF,0x08FFFFFF,0x08FFFFFF,0x09FFFFFF,0x0AFFFFFF,0x0BFFFFFF,0x0BFFFFFF,
    0x0CFFFFFF,0x0DFFFFFF,0x0EFFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x10FFFFFF,0x11FFFFFF,0x12FFFFFF,
    0x13FFFFFF,0x14FFFFFF,0x15FFFFFF,0x16FFFFFF,0x17FFFFFF,0x17FFFFFF,0x18FFFFFF,0x19FFFFFF,
    0x1AFFFFFF,0x1BFFFFFF,0x1BFFFFFF,0x1CFFFFFF,0x1DFFFFFF,0x1EFFFFFF,0x1EFFFFFF,0x1FFFFFFF,
    0x20FFFFFF,0x20FFFFFF,0x21FFFFFF,0x21FFFFFF,0x22FFFFFF,0x22FFFFFF,0x23FFFFFF,0x23FFFFFF,
    0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,
    0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x25FFFFFF,0x24FFFFFF,0x24FFFFFF,0x24FFFFFF,
    0x24FFFFFF,0x23FFFFFF,0x23FFFFFF,0x22FFFFFF,0x22FFFFFF,0x21FFFFFF,0x21FFFFFF,0x20FFFFFF,
    0x20FFFFFF,0x1FFFFFFF,0x1EFFFFFF,0x1EFFFFFF,0x1DFFFFFF,0x1CFFFFFF,0x1BFFFFFF,0x1BFFFFFF,
    0x1AFFFFFF,0x19FFFFFF,0x18FFFFFF,0x17FFFFFF,0x17FFFFFF,0x16FFFFFF,0x15FFFFFF,0x14FFFFFF,
    0x13FFFFFF,0x12FFFFFF,0x11FFFFFF,0x10FFFFFF,0x0FFFFFFF,0x0FFFFFFF,0x0EFFFFFF,0x0DFFFFFF,
    0x0CFFFFFF,0x0BFFFFFF,0x0BFFFFFF,0x0AFFFFFF,0x09FFFFFF,0x08FFFFFF,0x08FFFFFF,0x07FFFFFF,
    0x06FFFFFF,0x06FFFFFF,0x05FFFFFF,0x05FFFFFF,0x04FFFFFF,0x04FFFFFF,0x03FFFFFF,0x03FFFFFF,
    0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,0x02FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,
};

static int reflection_center(const reflection_target *target)
{
    int width=lv_obj_get_width(target->target),height=lv_obj_get_height(target->target);
    return -REFLECTION_HALF+(int)(target->phase%REFLECTION_CYCLE)*(width+height/2+2*REFLECTION_HALF)/(REFLECTION_SWEEP-1);
}
static void reflection_bounds(reflection_target *target)
{
    lv_area_t coords;lv_obj_get_coords(target->target,&coords);
    int height=lv_obj_get_height(target->target),center=reflection_center(target);
    target->dirty_count=0;
    /* Ten bounded strips avoid a full-tile rectangle or overflowing LVGL's
     * 32-entry invalidation queue. Only the old/new diagonal bands redraw. */
    for(u32 i=0;i<REFLECTION_STRIPS;++i){
        int top=i*height/REFLECTION_STRIPS,bottom=(i+1)*height/REFLECTION_STRIPS-1;
        lv_area_t band={coords.x1+center-bottom/2-REFLECTION_HALF-2,coords.y1+top,
            coords.x1+center-top/2+REFLECTION_HALF+2,coords.y1+bottom};
        lv_area_t clipped;
        if(lv_area_intersect(&clipped,&coords,&band))target->dirty[target->dirty_count++]=clipped;
    }
}
static void reflection_invalidate(const reflection_target *target,const lv_area_t *visible)
{
    for(u32 i=0;i<target->dirty_count;++i){lv_area_t clipped;
        if(lv_area_intersect(&clipped,&target->dirty[i],visible))lv_inv_area(&clipped);
    }
}
static bool reflection_design(lv_obj_t *object,const lv_area_t *mask,lv_design_mode_t mode)
{
    if(mode==LV_DESIGN_COVER_CHK)return false;
    if(mode!=LV_DESIGN_DRAW_MAIN||!n_cfg.nova_halo||reflection_pause_depth)return true;
    reflection_target *target=lv_obj_get_free_ptr(object);
    if(!target||!target->painted)return true;
    lv_area_t coords,clip;lv_obj_get_coords(object,&coords);
    if(!lv_area_intersect(&clip,&coords,mask))return true;
    int center=reflection_center(target),height=lv_obj_get_height(object);
    static const u8 corner_inset[]={7,5,4,3,2,2,2,2};
    /* Direct row spans: no large alpha image, no raster buffer, no per-frame
     * allocation. The border and the rounded corners remain untouched. */
    for(int y=clip.y1;y<=clip.y2;++y){
        int row=y-coords.y1,edge=row<height-1-row?row:height-1-row;
        if(edge<1)continue;
        int inset=edge<8?corner_inset[edge]:2;
        lv_area_t row_clip={coords.x1+inset,y,coords.x2-inset,y},effective;
        if(!lv_area_intersect(&effective,&row_clip,&clip))continue;
        int left=coords.x1+center-row/2-REFLECTION_HALF;
        lv_area_t span={left,y,left+2*REFLECTION_HALF,y};
        if(span.x2<effective.x1||span.x1>effective.x2)continue;
        /* Per-pixel alpha retains the transparent fade below 16/255. */
        lv_vmap(&span,&effective,(const u8 *)reflection_gradient_row,
            LV_OPA_COVER,false,true,LV_COLOR_WHITE,LV_OPA_TRANSP);
    }
    return true;
}
static lv_res_t reflection_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    reflection_target *target=lv_obj_get_free_ptr(object);
    lv_res_t result=reflection_base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP&&target)memset(target,0,sizeof(*target));
    return result;
}
static void reflection_frame(void *unused)
{
    (void)unused;if(reflection_pause_depth||!n_cfg.nova_halo)return;
    if(reflection_interacting&&lv_tick_elaps(reflection_interaction_tick)<160)return;
    reflection_interacting=false;++reflection_phase;
    for(u32 i=0;i<REFLECTION_TARGETS;++i){
        reflection_target *target=&reflection_targets[i];if(!target->target)continue;
        lv_area_t visible;
        if(!reflection_visible(target->target,&visible)){target->painted=false;continue;}
        if(target->painted)reflection_invalidate(target,&visible);
        target->phase=reflection_phase;target->painted=reflection_phase%REFLECTION_CYCLE<REFLECTION_SWEEP;
        if(!target->painted)continue;
        reflection_bounds(target);reflection_invalidate(target,&visible);
    }
}
void nyx_nova_reflect(lv_obj_t *object,bool selected)
{
    if(!object)return;
    selected=selected&&n_cfg.nova_halo;
    reflection_target *vacant=NULL;
    for(u32 i=0;i<REFLECTION_TARGETS;++i){
        reflection_target *target=&reflection_targets[i];
        if(target->target==object){if(!selected)lv_obj_del(target->overlay);return;}
        if(!target->target&&!vacant)vacant=target;
    }
    if(!selected||!vacant||lv_obj_get_width(object)<100||lv_obj_get_height(object)<50)return;
    lv_obj_t *overlay=lv_obj_create(object,NULL);if(!overlay)return;
    lv_obj_set_style(overlay,&lv_style_transp_tight);lv_obj_set_click(overlay,false);
    lv_obj_set_protect(overlay,LV_PROTECT_PARENT);
    lv_obj_set_size(overlay,lv_obj_get_width(object),lv_obj_get_height(object));lv_obj_set_pos(overlay,0,0);
    vacant->target=object;vacant->overlay=overlay;vacant->phase=reflection_phase;vacant->painted=reflection_phase%REFLECTION_CYCLE<REFLECTION_SWEEP;
    reflection_bounds(vacant);lv_obj_set_free_ptr(overlay,vacant);
    if(!reflection_base_signal)reflection_base_signal=lv_obj_get_signal_func(overlay);
    lv_obj_set_signal_func(overlay,reflection_signal);lv_obj_set_design_func(overlay,reflection_design);
    if(!reflection_task)reflection_task=lv_task_create(reflection_frame,REFLECTION_PERIOD,LV_TASK_PRIO_LOWEST,NULL);
}
void nyx_nova_reflection_reset(void)
{
    if(reflection_task)lv_task_del(reflection_task);
    reflection_task=NULL;
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].overlay)lv_obj_del(reflection_targets[i].overlay);
    memset(reflection_targets,0,sizeof(reflection_targets));reflection_phase=reflection_pause_depth=0;reflection_interacting=false;
}
void nyx_nova_reflection_pause(bool pause)
{
    if(pause)++reflection_pause_depth;else if(reflection_pause_depth)--reflection_pause_depth;
    lv_area_t screen={0,0,LV_HOR_RES-1,LV_VER_RES-1};
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].painted){reflection_invalidate(&reflection_targets[i],&screen);reflection_targets[i].painted=false;}
    if(reflection_task)lv_task_set_prio(reflection_task,reflection_pause_depth?LV_TASK_PRIO_OFF:LV_TASK_PRIO_LOWEST);
}
void nyx_nova_reflection_interaction(void)
{reflection_interaction_tick=lv_tick_get();reflection_interacting=true;}
