/* Hue X, saturated color -> white Y, independent value slider. No presets. */
#include "gui.h"
#include "gui_nova.h"
#include "gui_color_picker.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    lv_cont_ext_t container;
    lv_signal_func_t base_signal,plane_signal;
    lv_obj_t *plane,*hit,*cursor,*value,*swatch,*hex,*channels;
    lv_img_dsc_t image;
    lv_color_t *pixels;
    lv_style_t swatch_style,cursor_style;
    nyx_color_changed_t changed;
    u32 rgb,hue,saturation,brightness,cached_brightness;
    int width,height;
} color_picker_ext;

static void color_plane_fill(lv_obj_t *picker)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    if(!ext->pixels||ext->cached_brightness==ext->brightness)return;
    /* Cache the plane only when brightness changes. Moving the cursor or
     * switching targets at the same brightness never regenerates it. */
    for(int x=0;x<ext->width;++x){
        lv_color_t hue=lv_color_hsv_to_rgb(x*359u/(ext->width-1),100,100);
        for(int y=0;y<ext->height;++y){
            u32 s=(ext->height-1-y)*255u/(ext->height-1);
            u32 r=(255-((255-hue.red)*s+127)/255)*ext->brightness/100;
            u32 g=(255-((255-hue.green)*s+127)/255)*ext->brightness/100;
            u32 b=(255-((255-hue.blue)*s+127)/255)*ext->brightness/100;
            ext->pixels[y*ext->width+x]=LV_COLOR_MAKE(r,g,b);
        }
    }
    ext->cached_brightness=ext->brightness;lv_obj_invalidate(ext->plane);
}

static void color_picker_refresh(lv_obj_t *picker,bool notify)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    color_plane_fill(picker);
    lv_obj_set_pos(ext->cursor,ext->hue*(ext->width-1)/359-6,(100-ext->saturation)*(ext->height-1)/100-6);
    lv_bar_set_value(ext->value,ext->brightness);
    ext->swatch_style.body.main_color=ext->swatch_style.body.grad_color=LV_COLOR_HEX(ext->rgb);
    lv_cont_set_style(ext->swatch,&ext->swatch_style);
    char text[96];s_printf(text,"#%06X",ext->rgb);(lv_label_set_text)(ext->hex,text);
    s_printf(text,"R   %d\nG   %d\nB   %d",(ext->rgb>>16)&255,(ext->rgb>>8)&255,ext->rgb&255);
    (lv_label_set_text)(ext->channels,text);
    if(notify&&ext->changed)ext->changed(picker,ext->rgb);
}

static void color_picker_update(lv_obj_t *picker)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    lv_color_t c=lv_color_hsv_to_rgb(ext->hue,ext->saturation,ext->brightness);
    ext->rgb=((u32)c.red<<16)|((u32)c.green<<8)|c.blue;
    color_picker_refresh(picker,true);
}

static lv_res_t color_plane_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    lv_obj_t *picker=lv_obj_get_free_ptr(object);color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    lv_res_t result=ext->plane_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_PRESSED||signal==LV_SIGNAL_PRESSING){
        lv_point_t point;lv_indev_t *input=lv_indev_get_act();if(!input)return result;
        lv_indev_get_point(input,&point);lv_area_t area;lv_obj_get_coords(object,&area);
        int x=point.x-area.x1,y=point.y-area.y1;
        if(x<0)x=0;
        if(x>=ext->width)x=ext->width-1;
        if(y<0)y=0;
        if(y>=ext->height)y=ext->height-1;
        u32 hue=x*359u/(ext->width-1),saturation=100u-y*100u/(ext->height-1);
        if(hue!=ext->hue||saturation!=ext->saturation){ext->hue=hue;ext->saturation=saturation;color_picker_update(picker);}
    }
    return result;
}

static lv_res_t color_value_action(lv_obj_t *slider)
{
    lv_obj_t *picker=lv_obj_get_parent(slider);color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    u32 value=lv_slider_get_value(slider);
    if(value!=ext->brightness){ext->brightness=value;color_picker_update(picker);}
    return LV_RES_OK;
}

static lv_res_t color_picker_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    color_picker_ext *ext=lv_obj_get_ext_attr(object);
    lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP){free(ext->pixels);ext->pixels=NULL;}
    return result;
}

void nyx_color_picker_set(lv_obj_t *picker,u32 rgb)
{
    if(!picker)return;
    color_picker_ext *ext=lv_obj_get_ext_attr(picker);
    ext->rgb=rgb&0xFFFFFF;
    /* This LVGL version's rgb_to_hsv uses byte units whereas hsv_to_rgb
     * takes degrees/percent. Convert explicitly to avoid out-of-range
     * cursor positions and brightness for saturated or pale colors. */
    int r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;
    int maximum=r>g?r:g;if(b>maximum)maximum=b;
    int minimum=r<g?r:g;if(b<minimum)minimum=b;
    int delta=maximum-minimum,hue=0;
    if(delta){
        if(maximum==r)hue=60*(g-b)/delta;
        else if(maximum==g)hue=120+60*(b-r)/delta;
        else hue=240+60*(r-g)/delta;
        if(hue<0)hue+=360;
    }
    ext->hue=hue;ext->saturation=maximum?(delta*100+maximum/2)/maximum:0;
    ext->brightness=(maximum*100+127)/255;
    color_picker_refresh(picker,false); /* Selecting a target preserves exact RGB bytes. */
}
u32 nyx_color_picker_rgb(lv_obj_t *picker)
{return picker?((color_picker_ext *)lv_obj_get_ext_attr(picker))->rgb:0;}

lv_obj_t *nyx_color_picker_create(lv_obj_t *parent,int x,int y,int width,int height,nyx_color_changed_t changed)
{
    lv_obj_t *picker=lv_cont_create(parent,NULL);lv_signal_func_t base=lv_obj_get_signal_func(picker);
    color_picker_ext *ext=lv_obj_allocate_ext_attr(picker,sizeof(*ext));if(!ext){lv_obj_del(picker);return NULL;}
    memset((u8 *)ext+sizeof(ext->container),0,sizeof(*ext)-sizeof(ext->container));
    ext->base_signal=base;ext->changed=changed;ext->width=width*3/5;ext->height=height-64;
    ext->cached_brightness=101;
    if(ext->width<2||ext->height<2||ext->width>640||ext->height>360){lv_obj_del(picker);return NULL;}
    ext->pixels=malloc(ext->width*ext->height*sizeof(lv_color_t));if(!ext->pixels){lv_obj_del(picker);return NULL;}
    lv_obj_set_signal_func(picker,color_picker_signal);
    lv_cont_set_style(picker,&lv_style_transp_tight);lv_cont_set_layout(picker,LV_LAYOUT_OFF);lv_cont_set_fit(picker,false,false);
    lv_obj_set_size(picker,width,height);lv_obj_set_pos(picker,x,y);lv_obj_set_click(picker,false);
    ext->image.header.cf=LV_IMG_CF_TRUE_COLOR;ext->image.header.w=ext->width;ext->image.header.h=ext->height;
    ext->image.data_size=ext->width*ext->height*sizeof(lv_color_t);ext->image.data=(const u8 *)ext->pixels;
    ext->plane=lv_img_create(picker,NULL);lv_img_set_src(ext->plane,&ext->image);lv_obj_set_pos(ext->plane,0,0);lv_obj_set_click(ext->plane,false);
    ext->hit=lv_obj_create(picker,NULL);lv_obj_set_style(ext->hit,&lv_style_transp_tight);lv_obj_set_size(ext->hit,ext->width,ext->height);
    lv_obj_set_pos(ext->hit,0,0);lv_obj_set_free_ptr(ext->hit,picker);ext->plane_signal=lv_obj_get_signal_func(ext->hit);lv_obj_set_signal_func(ext->hit,color_plane_signal);
    lv_obj_set_protect(ext->hit,LV_PROTECT_PRESS_LOST);
    ext->cursor=lv_cont_create(ext->hit,NULL);lv_cont_set_fit(ext->cursor,false,false);
    lv_style_copy(&ext->cursor_style,&lv_style_plain);ext->cursor_style.body.empty=1;ext->cursor_style.body.radius=LV_RADIUS_CIRCLE;
    ext->cursor_style.body.border.width=2;ext->cursor_style.body.border.color=LV_COLOR_WHITE;ext->cursor_style.body.border.opa=LV_OPA_COVER;
    lv_cont_set_style(ext->cursor,&ext->cursor_style);lv_obj_set_size(ext->cursor,13,13);lv_obj_set_click(ext->cursor,false);
    ext->value=lv_slider_create(picker,NULL);lv_slider_set_range(ext->value,0,100);lv_obj_set_size(ext->value,ext->width,20);
    lv_obj_set_pos(ext->value,0,height-22);lv_slider_set_action(ext->value,color_value_action);
    nyx_nova_text(picker,"Luminosité couleur",0,height-55,ext->width,NOVA_SMALL);
    int right=ext->width+24,right_width=width-right;
    ext->swatch=lv_cont_create(picker,NULL);lv_cont_set_fit(ext->swatch,false,false);lv_obj_set_click(ext->swatch,false);
    lv_style_copy(&ext->swatch_style,&lv_style_plain);ext->swatch_style.body.opa=LV_OPA_COVER;ext->swatch_style.body.radius=3;
    ext->swatch_style.body.border.width=1;ext->swatch_style.body.border.color=LV_COLOR_HEX(0x8B99A9);
    lv_obj_set_pos(ext->swatch,right,0);lv_obj_set_size(ext->swatch,right_width,48);
    ext->hex=nyx_nova_text(picker,"",right,68,right_width,NOVA_BODY);
    ext->channels=nyx_nova_text(picker,"",right,112,right_width,NOVA_SMALL);
    nyx_color_picker_set(picker,0x55C9F1);return picker;
}
