/* White diagonal sheen over the selected glyph only.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_nova.h"
#include "../config.h"
#include <string.h>
#include <stdlib.h>
#define REFLECTION_TARGETS 32
#define REFLECTION_PERIOD 50
typedef struct {
    lv_obj_t *target,*overlay;
    lv_area_t dirty;
    u32 phase;
    bool painted;
    const lv_img_dsc_t *source;
    lv_img_dsc_t mask;
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


static void reflection_mask(reflection_target *target)
{
    u32 w=target->source->header.w,h=target->source->header.h;
    u8 *alpha=(u8 *)target->mask.data;
    /* A soft, diagonal white band traverses the icon in 1.2 s, then rests
     * for 2.4 s. The original alpha mask keeps all empty pixels transparent. */
    int center=-((int)h)-16+(int)(target->phase%72)*(int)(2*w+h+32)/23;
    for(u32 y=0;y<h;++y)for(u32 x=0;x<w;++x){
        int distance=(int)(2*x)-(int)y-center;if(distance<0)distance=-distance;
        u32 strength=distance<14?(14-distance)*176/14:0;
        alpha[y*w+x]=target->source->data[y*w+x]*strength/255;
    }
}
static bool reflection_design(lv_obj_t *object,const lv_area_t *clip,lv_design_mode_t mode)
{
    if(mode==LV_DESIGN_COVER_CHK)return false;
    if(mode!=LV_DESIGN_DRAW_MAIN||!n_cfg.nova_halo||reflection_pause_depth)return true;
    reflection_target *target=lv_obj_get_free_ptr(object);
    if(!target||!target->painted)return true;
    lv_style_t white;lv_style_copy(&white,&lv_style_plain);
    white.image.color=LV_COLOR_WHITE;white.image.opa=LV_OPA_COVER;
    lv_area_t coords;lv_obj_get_coords(object,&coords);
    lv_draw_img(&coords,clip,&target->mask,&white,LV_OPA_COVER);
    return true;
}
static lv_res_t reflection_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    reflection_target *target=lv_obj_get_free_ptr(object);
    lv_res_t result=reflection_base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP&&target){free((void *)target->mask.data);memset(target,0,sizeof(*target));}
    return result;
}
static void reflection_frame(void *unused)
{
    (void)unused;if(reflection_pause_depth||!n_cfg.nova_halo)return;
    if(reflection_interacting&&lv_tick_elaps(reflection_interaction_tick)<160)return;
    reflection_interacting=false;++reflection_phase;
    for(u32 i=0;i<REFLECTION_TARGETS;++i){
        reflection_target *target=&reflection_targets[i];if(!target->target)continue;
        lv_area_t visible,dirty;
        if(!reflection_visible(target->target,&visible)){target->painted=false;continue;}
        if(target->painted&&lv_area_intersect(&dirty,&target->dirty,&visible))lv_inv_area(&dirty);
        target->phase=reflection_phase;target->painted=reflection_phase%72<24;
        if(!target->painted)continue;
        reflection_mask(target);lv_obj_get_coords(target->overlay,&target->dirty);
        if(lv_area_intersect(&dirty,&target->dirty,&visible))lv_inv_area(&dirty);
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
    if(!selected||!vacant)return;
    lv_obj_type_t type;lv_obj_get_type(object,&type);
    if(!type.type[0]||strcmp(type.type[0],"lv_img"))return;
    const lv_img_dsc_t *source=lv_img_get_src(object);
    if(!source||source->header.cf!=LV_IMG_CF_ALPHA_8BIT||source->header.w>40||source->header.h>40)return;
    u8 *alpha=calloc(source->header.w*source->header.h,1);if(!alpha)return;
    lv_obj_t *overlay=lv_obj_create(object,NULL);if(!overlay){free(alpha);return;}
    lv_obj_set_style(overlay,&lv_style_transp_tight);lv_obj_set_click(overlay,false);
    lv_obj_set_size(overlay,source->header.w,source->header.h);lv_obj_set_pos(overlay,0,0);
    vacant->target=object;vacant->overlay=overlay;vacant->source=source;
    vacant->mask.header.cf=LV_IMG_CF_ALPHA_8BIT;vacant->mask.header.w=source->header.w;vacant->mask.header.h=source->header.h;
    vacant->mask.data=alpha;vacant->mask.data_size=source->header.w*source->header.h;
    vacant->phase=reflection_phase;vacant->painted=reflection_phase%72<24;
    reflection_mask(vacant);lv_obj_get_coords(overlay,&vacant->dirty);lv_obj_set_free_ptr(overlay,vacant);
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
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].painted){lv_inv_area(&reflection_targets[i].dirty);reflection_targets[i].painted=false;}
    if(reflection_task)lv_task_set_prio(reflection_task,reflection_pause_depth?LV_TASK_PRIO_OFF:LV_TASK_PRIO_LOWEST);
}
void nyx_nova_reflection_interaction(void)
{reflection_interaction_tick=lv_tick_get();reflection_interacting=true;}
