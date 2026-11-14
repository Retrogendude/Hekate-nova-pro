/* Smooth Nova loading stages. SPDX-License-Identifier: GPL-2.0-or-later */
#include "nova_splash.h"
#include <soc/timer.h>

u32 nova_splash_logo_color=0x55C9F1,nova_splash_bar_color=0x55C9F1;
static u32 *splash_frame;
static u32 splash_percent,splash_ceiling,splash_tick,splash_phase;
u32 nova_splash_tint(u32 color,u32 alpha)
{
    u32 result=0;
    if(alpha>255)alpha=255;
    for(u32 shift=0;shift<24;shift+=8){
        u32 bg=(0x10151D>>shift)&255,fg=(color>>shift)&255;
        result|=((fg*alpha+bg*(255-alpha)+127)/255)<<shift;
    }
    return result;
}
void nova_splash_set_colors(u32 logo,u32 bar)
{
    nova_splash_logo_color=logo<=0xFFFFFF?logo:0x55C9F1;
    nova_splash_bar_color=bar<=0xFFFFFF?bar:0x55C9F1;
}
static void splash_poll(u32 now)
{
    if((u32)(now-splash_tick)<20000)return;
    splash_tick=now;
    ++splash_phase;
    /* Advance below the current stage ceiling; only completed work permits
     * the next stage. Dots continue if a slow card reaches that ceiling. */
    if(splash_percent+1<splash_ceiling&&!(splash_phase%3))++splash_percent;
    nova_splash_progress(splash_frame,splash_phase,splash_percent);
}
void nova_splash_stage(u32 *frame,u32 from,u32 ceiling)
{
    timer_progress_hook=NULL;
    splash_frame=frame;splash_percent=from>100?100:from;
    splash_ceiling=ceiling>100?100:ceiling;
    splash_tick=get_tmr_us();splash_phase=0;
    nova_splash_progress(frame,0,splash_percent);
    timer_progress_hook=splash_poll;
}
void nova_splash_pause(void)
{ timer_progress_hook=NULL; }
void nova_splash_stop(void)
{ timer_progress_hook=NULL;splash_frame=NULL; }
void nova_splash_animate(u32 *frame,u32 from,u32 to,u32 duration_ms)
{
    if(splash_frame==frame&&splash_percent>from)from=splash_percent;
    nova_splash_stop();
    if(from>100)from=100;
    if(to>100)to=100;
    if(to<from)to=from;
    u32 steps=duration_ms/20;if(!steps)steps=1;
    for(u32 tick=0;tick<=steps;++tick){
        nova_splash_progress(frame,tick,from+(to-from)*tick/steps);
        if(tick<steps)usleep(duration_ms*1000/steps);
    }
}
