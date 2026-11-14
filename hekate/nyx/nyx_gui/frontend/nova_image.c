#include "nova_image.h"
#include "../libs/png/nova_png.h"
#include <stdlib.h>

bool nova_image_png(const u8 *data,u32 bytes,u32 width,u32 height,bool cover,u32 **pixels)
{
    if(!pixels)return false;
    *pixels=NULL;
    if(!data||bytes<33||bytes>8u*1024u*1024u||!width||!height||width>1280||height>720)return false;
    LodePNGState state;lodepng_state_init(&state);
    unsigned sw=0,sh=0;u8 *rgba=NULL;u32 *out=NULL;
    unsigned error=lodepng_inspect(&sw,&sh,&state,data,bytes);
    if(error||!sw||!sh||sw>1920||sh>1080)goto done;
    state.decoder.zlibsettings.max_output_size=20u*1024u*1024u;
    error=lodepng_decode(&rgba,&sw,&sh,&state,data,bytes);
    if(error||!rgba)goto done;
    out=calloc(width*height,sizeof(u32));if(!out)goto done;
    u32 crop_w=sw,crop_h=sh,dw=width,dh=height;
    if(cover){
        crop_h=sw*height/width;
        if(crop_h>sh){crop_h=sh;crop_w=sh*width/height;}
    }else{
        dh=width*sh/sw;
        if(dh>height){dh=height;dw=height*sw/sh;}
    }
    if(!crop_w)crop_w=1;
    if(!crop_h)crop_h=1;
    if(!dw)dw=1;
    if(!dh)dh=1;
    u32 sx=(sw-crop_w)/2,sy=(sh-crop_h)/2,dx=(width-dw)/2,dy=(height-dh)/2;
    for(u32 y=0;y<dh;++y)for(u32 x=0;x<dw;++x){
        const u8 *p=rgba+((sy+y*crop_h/dh)*sw+sx+x*crop_w/dw)*4;
        out[(dy+y)*width+dx+x]=((u32)p[3]<<24)|((u32)p[0]<<16)|((u32)p[1]<<8)|p[2];
    }
    *pixels=out;
done:
    lodepng_free(rgba);lodepng_state_cleanup(&state);return *pixels!=NULL;
}
