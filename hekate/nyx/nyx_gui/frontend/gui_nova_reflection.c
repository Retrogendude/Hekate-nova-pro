/* Fast white diagonal sheen and a bounded blue/violet selection border.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_nova.h"
#include "../config.h"
#include <libs/lvgl/lv_draw/lv_draw_vbasic.h>
#include <string.h>
#define REFLECTION_TARGETS 32
#define REFLECTION_PERIOD 20
#define REFLECTION_SWEEP 12
#define REFLECTION_FIRST (REFLECTION_SWEEP + 150) /* 3 seconds after pass. */
#define REFLECTION_CYCLE (REFLECTION_FIRST + REFLECTION_SWEEP + 250) /* Then 5 seconds. */
#define REFLECTION_STRIPS 16
#define REFLECTION_HALF 64
/* Leave one entry for ordinary widgets in LVGL's 32-entry invalidation FIFO. */
#define REFLECTION_QUEUE_LIMIT 31
typedef struct {
    lv_obj_t *target,*overlay;
    lv_area_t dirty[REFLECTION_STRIPS];
    u32 phase,dirty_count;
    bool painted;
} reflection_target;
static reflection_target reflection_targets[REFLECTION_TARGETS];
static lv_task_t *reflection_task;
static lv_signal_func_t reflection_base_signal;
static u32 reflection_phase,reflection_pause_depth,reflection_interaction_tick,reflection_phase_tick,reflection_first_target;
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
    u32 phase=target->phase%REFLECTION_CYCLE;
    if(phase>=REFLECTION_FIRST)phase-=REFLECTION_FIRST;
    return -REFLECTION_HALF+(int)phase*(width+height/2+2*REFLECTION_HALF)/(REFLECTION_SWEEP-1);
}
static bool reflection_sweeping(u32 phase)
{
    phase%=REFLECTION_CYCLE;
    return phase<REFLECTION_SWEEP||(phase>=REFLECTION_FIRST&&phase<REFLECTION_FIRST+REFLECTION_SWEEP);
}
static void reflection_bounds(reflection_target *target)
{
    lv_area_t coords;lv_obj_get_coords(target->target,&coords);
    int height=lv_obj_get_height(target->target),center=reflection_center(target);
    target->dirty_count=0;
    /* Bounded strips avoid a full-tile rectangle. Transitions merge the
     * old/new spans before using LVGL's 32-entry invalidation queue. */
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
static u32 reflection_transition(const reflection_target *before,const reflection_target *after,const lv_area_t *visible,bool invalidate)
{
    u32 count=0;
    bool merged[REFLECTION_STRIPS]={false};
    for(u32 i=0;before->painted&&i<before->dirty_count;++i){
        lv_area_t area=before->dirty[i],clip;
        for(u32 j=0;after->painted&&j<after->dirty_count;++j){
            if(after->dirty[j].y1!=area.y1||after->dirty[j].y2!=area.y2)continue;
            if(after->dirty[j].x1<area.x1)area.x1=after->dirty[j].x1;
            if(after->dirty[j].x2>area.x2)area.x2=after->dirty[j].x2;
            merged[j]=true;break;
        }
        if(lv_area_intersect(&clip,&area,visible)){++count;if(invalidate)lv_inv_area(&clip);}
    }
    for(u32 j=0;after->painted&&j<after->dirty_count;++j)if(!merged[j]){
        lv_area_t clip;if(lv_area_intersect(&clip,&after->dirty[j],visible)){++count;if(invalidate)lv_inv_area(&clip);}
    }
    return count;
}
static u32 reflection_border_invalidate(const reflection_target *target,const lv_area_t *visible,bool invalidate)
{
    u32 count=0;
    lv_area_t c;lv_obj_get_coords(target->target,&c);
    const lv_area_t edges[]={
        {c.x1,c.y1,c.x2,c.y1+1},{c.x1,c.y2-1,c.x2,c.y2},
        {c.x1,c.y1+2,c.x1+7,c.y2-2},{c.x2-7,c.y1+2,c.x2,c.y2-2}
    };
    for(u32 i=0;i<4;++i){lv_area_t clip;if(lv_area_intersect(&clip,&edges[i],visible)){++count;if(invalidate)lv_inv_area(&clip);}}
    return count;
}

static void reflection_border_draw(const reflection_target *target,const lv_area_t *coords,const lv_area_t *clip)
{
    /* A 1-KiB palette and two-pixel row spans; no texture, heap allocation
     * or full-window invalidation. Fixed-point perimeter positions avoid
     * integer division for each pixel on Tegra's ARM control processor. */
    static lv_color_t palette[256];static bool ready;
    if(!ready){for(u32 i=0;i<256;++i){u32 t=i<128?i:255-i;palette[i]=LV_COLOR_MAKE(53+((103*t)>>7),144-((45*t)>>7),255);}ready=true;}
    int width=coords->x2-coords->x1+1,height=coords->y2-coords->y1+1;
    u32 step=(256u<<16)/(2u*(width+height)),phase=target->phase*4u;
    static const u8 rounded[]={7,5,4,3,2,1,1,0};
    for(int y=clip->y1;y<=clip->y2;++y){
        int row=y-coords->y1,edge=row<height-1-row?row:height-1-row;
        int inset=edge<8?rounded[edge]:0;
        if(edge<2){
            int left=coords->x1+inset,right=coords->x2-inset;
            if(left<clip->x1)left=clip->x1;if(right>clip->x2)right=clip->x2;
            for(int x=left;x<=right;x+=4){
                u32 pos=row<height/2?x-coords->x1:2*width+height-1-(x-coords->x1);
                lv_area_t span={x,y,x+3>right?right:x+3,y};
                lv_vfill(&span,clip,palette[(((pos*step)>>16)+phase)&255u],LV_OPA_COVER);
            }
        }else{
            lv_area_t left={coords->x1+inset,y,coords->x1+inset+1,y};
            lv_area_t right={coords->x2-inset-1,y,coords->x2-inset,y};
            u32 lp=2*width+2*height-1-row,rp=width+row;
            lv_vfill(&left,clip,palette[(((lp*step)>>16)+phase)&255u],LV_OPA_COVER);
            lv_vfill(&right,clip,palette[(((rp*step)>>16)+phase)&255u],LV_OPA_COVER);
        }
    }
}
static bool reflection_design(lv_obj_t *object,const lv_area_t *mask,lv_design_mode_t mode)
{
    if(mode==LV_DESIGN_COVER_CHK)return false;
    if(mode!=LV_DESIGN_DRAW_MAIN||!n_cfg.nova_halo||reflection_pause_depth)return true;
    reflection_target *target=lv_obj_get_free_ptr(object);
    if(!target)return true;
    lv_area_t coords,clip;lv_obj_get_coords(object,&coords);
    if(!lv_area_intersect(&clip,&coords,mask))return true;
    reflection_border_draw(target,&coords,&clip);
    if(!target->painted||lv_obj_get_width(object)<100||lv_obj_get_height(object)<64)return true;
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
    /* Advance by wall time even when a busy refresh cannot accept a phase.
     * Rendering may skip intermediate phases, never slow the actual sweep. */
    u32 steps=lv_tick_elaps(reflection_phase_tick)/REFLECTION_PERIOD;
    reflection_phase+=steps;reflection_phase_tick+=steps*REFLECTION_PERIOD;
    if(reflection_interacting&&lv_tick_elaps(reflection_interaction_tick)<160)return;
    reflection_interacting=false;
    u32 first=reflection_first_target;
    reflection_first_target=(reflection_first_target+1)%REFLECTION_TARGETS;
    for(u32 n=0;n<REFLECTION_TARGETS;++n){
        u32 i=(first+n)%REFLECTION_TARGETS;
        reflection_target *target=&reflection_targets[i];if(!target->target)continue;
        lv_area_t visible;
        if(!reflection_visible(target->target,&visible)){target->painted=false;continue;}
        reflection_target next=*target;
        next.phase=reflection_phase;next.painted=reflection_sweeping(reflection_phase);
        if(lv_obj_get_width(target->target)<100||lv_obj_get_height(target->target)<64)next.painted=false;
        if(next.painted)reflection_bounds(&next);
        /* Preflight the complete old/new transition against the GLOBAL queue,
         * including other widgets and earlier effects. LVGL only joins areas
         * after insertion, so a second 20ms phase before its 33ms refresh used
         * to overflow into a full-screen redraw. Never remove another widget's
         * invalidations. Retain the painted state until its entire next phase
         * fits; the next refresh then coalesces from that state to wall time. */
        u32 count=reflection_border_invalidate(target,&visible,false)+reflection_transition(target,&next,&visible,false);
        if(lv_refr_get_buf_size()+count>REFLECTION_QUEUE_LIMIT)continue;
        reflection_border_invalidate(target,&visible,true);
        reflection_transition(target,&next,&visible,true);
        *target=next;
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
    if(!selected||!vacant||lv_obj_get_width(object)<24||lv_obj_get_height(object)<24)return;
    lv_obj_t *overlay=lv_obj_create(object,NULL);if(!overlay)return;
    lv_obj_set_style(overlay,&lv_style_transp_tight);lv_obj_set_click(overlay,false);
    lv_obj_set_protect(overlay,LV_PROTECT_PARENT);
    lv_obj_set_size(overlay,lv_obj_get_width(object),lv_obj_get_height(object));lv_obj_set_pos(overlay,0,0);
    vacant->target=object;vacant->overlay=overlay;vacant->phase=reflection_phase;vacant->painted=reflection_sweeping(reflection_phase);
    reflection_bounds(vacant);lv_obj_set_free_ptr(overlay,vacant);
    if(!reflection_base_signal)reflection_base_signal=lv_obj_get_signal_func(overlay);
    lv_obj_set_signal_func(overlay,reflection_signal);lv_obj_set_design_func(overlay,reflection_design);
    if(!reflection_task){reflection_phase_tick=lv_tick_get();reflection_task=lv_task_create(reflection_frame,REFLECTION_PERIOD,LV_TASK_PRIO_LOWEST,NULL);}
}
void nyx_nova_reflection_reset(void)
{
    if(reflection_task)lv_task_del(reflection_task);
    reflection_task=NULL;
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].overlay)lv_obj_del(reflection_targets[i].overlay);
    memset(reflection_targets,0,sizeof(reflection_targets));reflection_phase=reflection_pause_depth=reflection_first_target=0;reflection_phase_tick=lv_tick_get();reflection_interacting=false;
}
void nyx_nova_reflection_pause(bool pause)
{
    if(pause)++reflection_pause_depth;else if(reflection_pause_depth)--reflection_pause_depth;
    lv_area_t screen={0,0,LV_HOR_RES-1,LV_VER_RES-1};
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].target){
        if(reflection_targets[i].painted)reflection_invalidate(&reflection_targets[i],&screen);
        reflection_border_invalidate(&reflection_targets[i],&screen,true);reflection_targets[i].painted=false;
    }
    if(reflection_task)lv_task_set_prio(reflection_task,reflection_pause_depth?LV_TASK_PRIO_OFF:LV_TASK_PRIO_LOWEST);
}
void nyx_nova_reflection_interaction(void)
{reflection_interaction_tick=lv_tick_get();reflection_interacting=true;}
