/* A bounded moving border highlight. SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui_nova.h"
#include "../config.h"
#include <string.h>

/* Only the previous/new short highlight rectangles are invalidated. Never
 * invalidate the complete panel, move a child each frame or allocate a buffer
 * in the timer. There are no blurred shadows or whole-screen animation layers. */
#define REFLECTION_TARGETS 32
#define REFLECTION_PERIOD 80
typedef struct {
    lv_obj_t *target, *overlay;
    lv_area_t dirty;
    u32 phase;
    bool painted;
} reflection_target;
static reflection_target reflection_targets[REFLECTION_TARGETS];
static lv_task_t *reflection_task;
static lv_signal_func_t reflection_base_signal;
static u32 reflection_phase, reflection_pause_depth, reflection_interaction_tick;
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

/* Eight straight sides form an inset rounded/chamfered 8px perimeter. The
 * short diagonal segments lie inside the existing antialiased card border. */
static u32 reflection_perimeter(lv_obj_t *object)
{
    int w = lv_obj_get_width(object) - 3, h = lv_obj_get_height(object) - 3;
    return w > 18 && h > 18 ? 2 * (w + h) - 26 : 0;
}
static lv_point_t reflection_point(lv_obj_t *object, u32 distance)
{
    int w = lv_obj_get_width(object) - 3, h = lv_obj_get_height(object) - 3;
    const lv_point_t vertices[] = {{7,1},{w-6,1},{w+1,8},{w+1,h-6},{w-6,h+1},{7,h+1},{1,h-6},{1,8},{7,1}};
    const u32 lengths[] = {w-13,7,h-14,7,w-13,7,h-14,7};
    u32 total = 0; for (u32 i=0;i<8;++i) total+=lengths[i];
    if (!total) return vertices[0];
    distance %= total;
    for (u32 i = 0; i < 8; ++i) {
        if (distance <= lengths[i]) return (lv_point_t){
            vertices[i].x + (vertices[i+1].x-vertices[i].x)*(int)distance/(int)lengths[i],
            vertices[i].y + (vertices[i+1].y-vertices[i].y)*(int)distance/(int)lengths[i]};
        distance -= lengths[i];
    }
    return vertices[0];
}
static u32 reflection_length(lv_obj_t *object)
{ return lv_obj_get_width(object) < 100 ? 24 : lv_obj_get_width(object) < 300 ? 40 : 72; }

static lv_area_t reflection_bounds(reflection_target *target)
{
    lv_area_t coords, bounds = {32767,32767,-32767,-32767};
    lv_obj_get_coords(target->overlay, &coords);
    u32 perimeter = reflection_perimeter(target->overlay), length = reflection_length(target->overlay);
    u32 start = perimeter ? (target->phase % 72) * perimeter / 72 : 0;
    for (u32 at = 0; at <= length; ++at) {
        lv_point_t point = reflection_point(target->overlay, start + at);
        int x=coords.x1+point.x,y=coords.y1+point.y;
        if(x<bounds.x1)bounds.x1=x;
        if(x>bounds.x2)bounds.x2=x;
        if(y<bounds.y1)bounds.y1=y;
        if(y>bounds.y2)bounds.y2=y;
    }
    bounds.x1-=2;bounds.y1-=2;bounds.x2+=2;bounds.y2+=2;
    return bounds;
}

static bool reflection_design(lv_obj_t *object, const lv_area_t *mask, lv_design_mode_t mode)
{
    if (mode == LV_DESIGN_COVER_CHK) return false;
    if (mode != LV_DESIGN_DRAW_MAIN || !n_cfg.nova_halo || reflection_pause_depth) return true;
    reflection_target *target = lv_obj_get_free_ptr(object);
    if (!target || !target->painted) return true;
    lv_area_t coords; lv_obj_get_coords(object, &coords);
    u32 perimeter=reflection_perimeter(object),length=reflection_length(object);
    if (!perimeter) return true;
    u32 start=(target->phase%72)*perimeter/72;
    lv_style_t line; lv_style_copy(&line,&lv_style_plain);
    line.line.width=2;line.line.rounded=0;
    line.line.color=lv_color_mix(LV_COLOR_WHITE,nyx_nova_color(NOVA_COLOR_REFLECTION),60);
    for (u32 i=0;i<length;++i) {
        lv_point_t a=reflection_point(object,start+i),b=reflection_point(object,start+i+1);
        a.x+=coords.x1;a.y+=coords.y1;b.x+=coords.x1;b.y+=coords.y1;
        line.line.opa=30+225*i/length;
        lv_draw_line(&a,&b,mask,&line,LV_OPA_COVER);
    }
    return true;
}

static lv_res_t reflection_signal(lv_obj_t *object, lv_signal_t signal, void *parameter)
{
    reflection_target *target=lv_obj_get_free_ptr(object);
    lv_res_t result=reflection_base_signal(object,signal,parameter);
    if (signal==LV_SIGNAL_CLEANUP && target) memset(target,0,sizeof(*target));
    return result;
}
static void reflection_frame(void *unused)
{
    (void)unused;
    if (reflection_pause_depth || !n_cfg.nova_halo) return;
    if (reflection_interacting && lv_tick_elaps(reflection_interaction_tick)<160) return;
    reflection_interacting=false;
    ++reflection_phase;
    for (u32 i=0;i<REFLECTION_TARGETS;++i) {
        reflection_target *target=&reflection_targets[i];
        if(!target->target)continue;
        lv_area_t visible,dirty;
        if(!reflection_visible(target->target,&visible)) {target->painted=false;continue;}
        if(target->painted && lv_area_intersect(&dirty,&target->dirty,&visible))lv_inv_area(&dirty);
        target->phase=reflection_phase;target->painted=true;
        target->dirty=reflection_bounds(target);
        if(lv_area_intersect(&dirty,&target->dirty,&visible))lv_inv_area(&dirty);
    }
}

void nyx_nova_reflect(lv_obj_t *object, bool selected)
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
    /* Protect before reparenting: lv_win immediately forwards unprotected
     * new children into its scroll page during their creation signal. */
    lv_obj_t *overlay=lv_obj_create(lv_scr_act(),NULL);
    lv_obj_set_protect(overlay,LV_PROTECT_PARENT);
    lv_obj_set_style(overlay,&lv_style_transp_tight);
    lv_obj_set_parent(overlay,object);
    lv_obj_set_size(overlay,lv_obj_get_width(object),lv_obj_get_height(object));lv_obj_set_pos(overlay,0,0);lv_obj_set_click(overlay,false);
    vacant->target=object;vacant->overlay=overlay;vacant->phase=reflection_phase;vacant->painted=true;
    vacant->dirty=reflection_bounds(vacant);
    lv_obj_set_free_ptr(overlay,vacant);
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
    /* Clear a previous glint once, never touch a complete panel. */
    for(u32 i=0;i<REFLECTION_TARGETS;++i)if(reflection_targets[i].painted){lv_inv_area(&reflection_targets[i].dirty);reflection_targets[i].painted=false;}
    if(reflection_task)lv_task_set_prio(reflection_task,reflection_pause_depth?LV_TASK_PRIO_OFF:LV_TASK_PRIO_LOWEST);
}
void nyx_nova_reflection_interaction(void)
{ reflection_interaction_tick=lv_tick_get();reflection_interacting=true; }
