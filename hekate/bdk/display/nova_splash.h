/* Original Nova Pro splash. SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NOVA_SPLASH_H
#define NOVA_SPLASH_H
#include <utils/types.h>
#include "nova_boot_art.inc"

/* The early display uses portrait pitch. Bootlogo instructions require a
 * 90-degree counterclockwise rotation of the landscape artwork. */
static inline void nova_splash_pixel(u32 *frame,u32 x,u32 y,u32 color)
{ frame[(1279-x)*720+y]=color; }
static inline void nova_splash_render(u32 *frame)
{
    for(u32 i=0;i<720*1280;++i)frame[i]=0x10151D;
    u32 position=0;
    for(u32 run=0;run<sizeof(nova_boot_rle);run+=2){
        u32 count=nova_boot_rle[run],color=nova_boot_palette[nova_boot_rle[run+1]];
        for(u32 i=0;i<count;++i,++position)nova_splash_pixel(frame,440+position%400,220+position/400,color);
    }
}
static inline void nova_splash_progress(u32 *frame,u32 phase,u32 percent)
{
    if(percent>100)percent=100;
    u32 filled=percent*240/100;
    for(u32 x=0;x<240;++x){
        u32 color=x<percent*240/100?0x55C9F1:0x273241;
        if(percent<100&&filled>x&&filled-x<8)color=0x88D9F6;
        for(u32 y=0;y<3;++y)nova_splash_pixel(frame,520+x,446+y,color);
    }
    for(u32 dot=0;dot<3;++dot)for(int y=-2;y<=2;++y)for(int x=-2;x<=2;++x)
        if(x*x+y*y<=5)nova_splash_pixel(frame,630+dot*10+x,480+y,dot==(phase/8)%3?0x55C9F1:0x334050);
}
void nova_splash_animate(u32 *frame,u32 from,u32 to,u32 duration_ms);
#endif
