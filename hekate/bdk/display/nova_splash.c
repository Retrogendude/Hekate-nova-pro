/* Smooth Nova loading stages. SPDX-License-Identifier: GPL-2.0-or-later */
#include "nova_splash.h"
#include <soc/timer.h>

void nova_splash_animate(u32 *frame,u32 from,u32 to,u32 duration_ms)
{
    if(from>100)from=100;
    if(to>100)to=100;
    if(to<from)to=from;
    u32 steps=duration_ms/20;if(!steps)steps=1;
    for(u32 tick=0;tick<=steps;++tick){
        nova_splash_progress(frame,tick,from+(to-from)*tick/steps);
        if(tick<steps)usleep(duration_ms*1000/steps);
    }
}
