/* Nova palette, static wallpapers and per-surface opacity.
 * SPDX-License-Identifier: GPL-2.0-or-later */
#include "gui.h"
#include "gui_nova.h"
#include "gui_choices.h"
#include "gui_options.h"
#include "gui_appearance.h"
#include "../config.h"
#include <libs/fatfs/ff.h>
#include <string.h>
#include <stdlib.h>

#define WALLPAPER_LIMIT 32
static lv_img_dsc_t *nova_wallpaper;
static char wallpaper_names[WALLPAPER_LIMIT][64];
static u32 wallpaper_count;
static void theme_to_front(lv_obj_t *object)
{
    lv_obj_t *parent=lv_obj_get_parent(object);if(!parent)return;
    lv_ll_move_before(&parent->child_ll,object,lv_ll_get_head(&parent->child_ll));lv_obj_invalidate(object);
}
static void theme_to_back(lv_obj_t *object)
{
    lv_obj_t *parent=lv_obj_get_parent(object);if(!parent)return;
    lv_ll_move_before(&parent->child_ll,object,NULL);lv_obj_invalidate(object);
}

static u32 wallpaper_u32(const u8 *bytes)
{ return (u32)bytes[0] | (u32)bytes[1]<<8 | (u32)bytes[2]<<16 | (u32)bytes[3]<<24; }
static void wallpaper_free(lv_img_dsc_t *image)
{ if(image){free((void *)image->data);free(image);} }

/* Decode a bounded uncompressed 24/32-bit BMP and crop/scale it ONCE. A single
 * source row and the final opaque image are allocated; no per-frame SD I/O.
 * The preview and the real background share this exact checked decoder. */
static lv_img_dsc_t *wallpaper_read(const char *file, u32 width, u32 height)
{
    if(!file||!*file||strlen(file)>=64||strchr(file,'/')||strchr(file,'\\')||strstr(file,".."))return NULL;
    char path[96];s_printf(path,"wallpapers/%s",file);
    FIL stream; if(f_open(&stream,path,FA_READ)!=FR_OK)return NULL;
    u8 header[54];UINT actual=0;lv_img_dsc_t *image=NULL;u8 *row=NULL;
    u32 bytes=f_size(&stream);
    if(bytes<54||bytes>16*1024*1024||f_read(&stream,header,54,&actual)!=FR_OK||actual!=54)goto done;
    if(header[0]!='B'||header[1]!='M'||wallpaper_u32(header+14)<40||header[26]!=1||header[27]||header[29]||wallpaper_u32(header+30))goto done;
    u32 offset=wallpaper_u32(header+10),declared=wallpaper_u32(header+2),source_width=wallpaper_u32(header+18);
    int signed_height=(int)wallpaper_u32(header+22);
    u32 source_height=signed_height<0?-(u32)signed_height:(u32)signed_height;
    u32 depth=header[28];
    if(!source_width||source_width>1920||!source_height||source_height>1080||(depth!=24&&depth!=32))goto done;
    u32 stride=(source_width*(depth/8)+3)&~3u;
    if(offset<54||offset>bytes||stride*source_height>bytes-offset||declared>bytes||declared<offset+stride*source_height)goto done;
    image=calloc(1,sizeof(*image));if(!image)goto done;
    image->data=malloc(width*height*4);row=malloc(stride);
    if(!image->data||!row){wallpaper_free(image);image=NULL;goto done;}
    image->header.cf=LV_IMG_CF_TRUE_COLOR;image->header.w=width;image->header.h=height;image->data_size=width*height*4;
    u32 crop_width=source_width,crop_height=source_width*height/width;
    if(crop_height>source_height){crop_height=source_height;crop_width=source_height*width/height;}
    if(!crop_width)crop_width=1;
    if(!crop_height)crop_height=1;
    u32 left=(source_width-crop_width)/2,top=(source_height-crop_height)/2,previous=0xFFFFFFFF;
    u32 *pixels=(u32 *)image->data;
    for(u32 y=0;y<height;++y){
        u32 source_y=top+y*crop_height/height;
        if(source_y!=previous){
            u32 physical=signed_height<0?source_y:source_height-1-source_y;
            if(f_lseek(&stream,offset+physical*stride)!=FR_OK||f_read(&stream,row,stride,&actual)!=FR_OK||actual!=stride){wallpaper_free(image);image=NULL;goto done;}
            previous=source_y;
        }
        for(u32 x=0;x<width;++x){const u8 *pixel=row+(left+x*crop_width/width)*(depth/8);pixels[y*width+x]=0xFF000000|((u32)pixel[2]<<16)|((u32)pixel[1]<<8)|pixel[0];}
    }
done:
    free(row);f_close(&stream);return image;
}

void nyx_nova_wallpaper_load(void)
{
    wallpaper_free(nova_wallpaper);nova_wallpaper=NULL;
    if(n_cfg.wallpaper_file[0])nova_wallpaper=wallpaper_read(n_cfg.wallpaper_file,LV_HOR_RES,LV_VER_RES);
}
void nyx_nova_backdrop(lv_obj_t *parent, int y)
{
    if(!nova_wallpaper)return;
    lv_obj_t *image=lv_img_create(lv_scr_act(),NULL);
    lv_obj_set_protect(image,LV_PROTECT_PARENT);
    if(lv_obj_get_parent(image)!=parent)lv_obj_set_parent(image,parent);
    lv_img_set_src(image,nova_wallpaper);
    lv_obj_set_pos(image,0,y);lv_obj_set_click(image,false);lv_obj_set_top(image,false);
    theme_to_back(image);
}

void nyx_nova_screen(lv_obj_t *screen)
{
    /* One opaque root, with the wallpaper explicitly at the back. A separate
     * opaque base child can mask a wallpaper when siblings are reordered. */
    static lv_style_t root_style;
    lv_style_copy(&root_style,&lv_style_plain);
    root_style.body.main_color=root_style.body.grad_color=nyx_nova_color(NOVA_COLOR_BACKGROUND);
    root_style.body.empty=0;root_style.body.opa=LV_OPA_COVER;
    root_style.body.border.width=root_style.body.shadow.width=root_style.body.radius=0;
    root_style.body.padding.hor=root_style.body.padding.ver=root_style.body.padding.inner=0;
    lv_cont_set_style(screen,&root_style);lv_cont_set_layout(screen,LV_LAYOUT_OFF);lv_cont_set_fit(screen,false,false);
    lv_obj_set_size(screen,LV_HOR_RES,LV_VER_RES);nyx_nova_backdrop(screen,0);
}

typedef struct {lv_win_ext_t window;lv_signal_func_t base_signal;} nova_window_ext;
static lv_res_t nova_window_signal(lv_obj_t *object,lv_signal_t signal,void *parameter)
{
    nova_window_ext *ext=lv_obj_get_ext_attr(object);
    lv_res_t result=ext->base_signal(object,signal,parameter);
    if(signal==LV_SIGNAL_CLEANUP)nyx_nova_navigation_lock(false);
    return result;
}
void nyx_nova_window(lv_obj_t *win)
{
    if(lv_obj_get_signal_func(win)!=nova_window_signal){
        lv_signal_func_t base=lv_obj_get_signal_func(win);
        nova_window_ext *ext=lv_obj_allocate_ext_attr(win,sizeof(*ext));
        if(ext){ext->base_signal=base;lv_obj_set_signal_func(win,nova_window_signal);nyx_nova_navigation_lock(true);}
    }
    static lv_style_t background;
    lv_style_copy(&background,lv_theme_get_current()->win.bg);
    background.body.empty=0;background.body.opa=LV_OPA_COVER;
    background.body.radius=8;background.body.border.width=1;
    background.body.main_color=background.body.grad_color=nyx_nova_color(NOVA_COLOR_BACKGROUND);
    background.body.border.color=nyx_nova_color(NOVA_COLOR_BORDER);
    lv_win_set_style(win,LV_WIN_STYLE_BG,&background);
    lv_obj_set_pos(win,0,NYX_NOVA_HEADER);lv_obj_set_size(win,LV_HOR_RES,NYX_NOVA_BOTTOM-NYX_NOVA_HEADER);
    lv_win_set_btn_size(win,52);
    lv_page_set_scrl_layout(((lv_win_ext_t *)lv_obj_get_ext_attr(win))->page,LV_LAYOUT_OFF);
    if(nova_wallpaper){
        nyx_nova_backdrop(win,-NYX_NOVA_HEADER);
        /* The shared background image hides underlying menu cards. */
        lv_win_ext_t *ext=lv_obj_get_ext_attr(win);
        theme_to_front(ext->page);theme_to_front(ext->header);
    }
    /* Window borders are static. */
}

static struct {
    bool open;
    nyx_config pending;
    lv_obj_t *window,*parent_close,*tabs,*target_choices,*menu_choices,*icon_choices,*sliders[3],*values[3],*hex,*file,*counter;
    lv_obj_t *preview,*sample,*sample_text,*sample_icon,*sample_glyph,*sample_option_glyph,*preview_image,*preview_caption;
    lv_style_t preview_style,sample_style,sample_text_style,sample_icon_style,reflection_style,glyph_style,option_style;
    lv_img_dsc_t *thumbnail;
    u32 target,wallpaper;
} nova_theme;
enum { THEME_TARGET=40,THEME_MENU_OPACITY,THEME_ICON_OPACITY };

static void theme_notice(const char *message)
{ nyx_show_help(message); }
static u32 *theme_target_color(void)
{
    return nova_theme.target==NOVA_COLOR_TEXT?&nova_theme.pending.nova_text_color:
        nova_theme.target==NOVA_COLOR_BORDER?&nova_theme.pending.nova_border_color:
        nova_theme.target==NOVA_COLOR_LOGO?&nova_theme.pending.nova_logo_color:
        nova_theme.target==NOVA_COLOR_ICONS?&nova_theme.pending.nova_icon_color:
        nova_theme.target==NOVA_COLOR_SELECTION?&nova_theme.pending.nova_selection_color:&nova_theme.pending.theme_bg;
}
static void theme_preview_refresh(void)
{
    u32 bg=nova_theme.pending.theme_bg==0x141A19?0x10151D:nova_theme.pending.theme_bg;
    lv_style_copy(&nova_theme.preview_style,lv_theme_get_current()->panel);
    nova_theme.preview_style.body.empty=0;nova_theme.preview_style.body.opa=LV_OPA_COVER;
    nova_theme.preview_style.body.main_color=nova_theme.preview_style.body.grad_color=LV_COLOR_HEX(bg);
    nova_theme.preview_style.body.border.color=LV_COLOR_HEX(nova_theme.pending.nova_border_color);
    lv_cont_set_style(nova_theme.preview,&nova_theme.preview_style);
    lv_style_copy(&nova_theme.sample_style,&nova_theme.preview_style);
    nova_theme.sample_style.body.main_color=nova_theme.sample_style.body.grad_color=LV_COLOR_HEX(nova_theme.pending.nova_selection_color);
    nova_theme.sample_style.body.opa=nova_theme.pending.nova_menu_opacity*255/100;
    lv_cont_set_style(nova_theme.sample,&nova_theme.sample_style);
    lv_style_copy(&nova_theme.sample_icon_style,&nova_theme.sample_style);
    nova_theme.sample_icon_style.body.opa=nova_theme.pending.nova_icon_opacity*255/100;
    lv_cont_set_style(nova_theme.sample_icon,&nova_theme.sample_icon_style);
    lv_style_copy(&nova_theme.glyph_style,&lv_style_plain);
    nova_theme.glyph_style.image.color=LV_COLOR_HEX(nova_theme.pending.nova_logo_color);
    nova_theme.glyph_style.image.opa=LV_OPA_COVER;
    lv_img_set_style(nova_theme.sample_glyph,&nova_theme.glyph_style);
    lv_style_copy(&nova_theme.option_style,&nova_theme.glyph_style);
    nova_theme.option_style.image.color=LV_COLOR_HEX(nova_theme.pending.nova_icon_color);
    lv_img_set_style(nova_theme.sample_option_glyph,&nova_theme.option_style);
    lv_style_copy(&nova_theme.sample_text_style,nyx_nova_text_style(NOVA_TITLE));
    nova_theme.sample_text_style.text.color=LV_COLOR_HEX(nova_theme.pending.nova_text_color);
    lv_label_set_style(nova_theme.sample_text,&nova_theme.sample_text_style);
    lv_style_copy(&nova_theme.reflection_style,nyx_nova_text_style(NOVA_SMALL));
    nova_theme.reflection_style.text.color=LV_COLOR_WHITE;
    lv_label_set_style(nova_theme.preview_caption,&nova_theme.reflection_style);
}
static void theme_controls_refresh(void)
{
    u32 value=*theme_target_color();if(nova_theme.target==NOVA_COLOR_BACKGROUND&&value==0x141A19)value=0x10151D;
    char text[24];s_printf(text,"#%06X",value);(lv_label_set_text)(nova_theme.hex,text);
    for(u32 i=0;i<3;++i){u32 channel=(value>>(16-i*8))&255;lv_bar_set_value(nova_theme.sliders[i],channel);s_printf(text,"%d",channel);(lv_label_set_text)(nova_theme.values[i],text);}
    theme_preview_refresh();
}
static lv_res_t theme_select(lv_obj_t *button)
{
    u32 group=lv_obj_get_free_num(lv_obj_get_parent(button)),value=lv_obj_get_free_num(button);
    if(group==THEME_TARGET){static const u32 roles[]={NOVA_COLOR_BACKGROUND,NOVA_COLOR_TEXT,NOVA_COLOR_BORDER,NOVA_COLOR_LOGO,NOVA_COLOR_ICONS,NOVA_COLOR_SELECTION};nova_theme.target=value<6?roles[value]:0;theme_controls_refresh();}
    else if(group==THEME_MENU_OPACITY){nova_theme.pending.nova_menu_opacity=value<=4?value*25:100;theme_preview_refresh();}
    else if(group==THEME_ICON_OPACITY){nova_theme.pending.nova_icon_opacity=value<=4?value*25:100;theme_preview_refresh();}
    return LV_RES_OK;
}
static lv_res_t theme_slider(lv_obj_t *slider)
{
    u32 channel=lv_obj_get_free_num(slider);if(channel>2)return LV_RES_OK;
    u32 *color=theme_target_color(),shift=16-channel*8;
    if(nova_theme.target==NOVA_COLOR_BACKGROUND&&*color==0x141A19)*color=0x10151D;
    *color=(*color&~(255u<<shift))|((u32)lv_slider_get_value(slider)<<shift);theme_controls_refresh();
    return LV_RES_OK;
}
static lv_res_t theme_swatch(lv_obj_t *button)
{ *theme_target_color()=lv_obj_get_free_num(button);theme_controls_refresh();return LV_RES_OK; }

static void wallpaper_scan(void)
{
    wallpaper_count=0;DIR directory;FILINFO entry;
    if(f_opendir(&directory,"wallpapers")!=FR_OK)return;
    while(wallpaper_count<WALLPAPER_LIMIT&&f_readdir(&directory,&entry)==FR_OK&&entry.fname[0]){
        if(entry.fattrib&(AM_DIR|AM_HID))continue;
        u32 bytes=strlen(entry.fname);
        if(bytes<5||bytes>=64||entry.fname[0]=='.'||strstr(entry.fname,".."))continue;
        if(entry.fname[bytes-4]!='.'||(entry.fname[bytes-3]|32)!='b'||(entry.fname[bytes-2]|32)!='m'||(entry.fname[bytes-1]|32)!='p')continue;
        bool valid=true;for(u32 i=0;i<bytes;++i){char c=entry.fname[i];if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'||c==' ')){valid=false;break;}}
        if(valid)strcpy(wallpaper_names[wallpaper_count++],entry.fname);
    }
    f_closedir(&directory);
    for(u32 i=1;i<wallpaper_count;++i)for(u32 j=i;j>0&&strcmp(wallpaper_names[j-1],wallpaper_names[j])>0;--j){char temp[64];strcpy(temp,wallpaper_names[j]);strcpy(wallpaper_names[j],wallpaper_names[j-1]);strcpy(wallpaper_names[j-1],temp);}
}
static void wallpaper_preview_refresh(void)
{
    if(nova_theme.preview_image){lv_obj_del(nova_theme.preview_image);nova_theme.preview_image=NULL;}
    wallpaper_free(nova_theme.thumbnail);nova_theme.thumbnail=NULL;
    const char *file=nova_theme.pending.wallpaper_file;
    (lv_label_set_text)(nova_theme.file,*file?file:nyx_tr("Aucun fond d’écran"));
    char position[24];s_printf(position,"%d / %d",nova_theme.wallpaper+1,wallpaper_count+1);(lv_label_set_text)(nova_theme.counter,position);
    bool mounted=sd_get_card_mounted();
    if(*file&&!sd_mount())nova_theme.thumbnail=wallpaper_read(file,516,405);
    if(!mounted)sd_unmount();
    if(nova_theme.thumbnail){nova_theme.preview_image=lv_img_create(nova_theme.preview,NULL);lv_img_set_src(nova_theme.preview_image,nova_theme.thumbnail);lv_obj_set_pos(nova_theme.preview_image,0,0);lv_obj_set_click(nova_theme.preview_image,false);theme_to_back(nova_theme.preview_image);}
    else if(*file)theme_notice("Image introuvable ou invalide. Choisissez une image BMP non compressée.");
}
static lv_res_t wallpaper_step(lv_obj_t *button)
{
    nova_theme.wallpaper=(nova_theme.wallpaper+(lv_obj_get_free_num(button)?1:wallpaper_count))%(wallpaper_count+1);
    nova_theme.pending.wallpaper_file[0]=0;
    if(nova_theme.wallpaper)strcpy(nova_theme.pending.wallpaper_file,wallpaper_names[nova_theme.wallpaper-1]);
    wallpaper_preview_refresh();return LV_RES_OK;
}
static void theme_apply(const nyx_config *source)
{
    n_cfg.theme_bg=source->theme_bg;n_cfg.nova_text_color=source->nova_text_color;
    n_cfg.nova_logo_color=source->nova_logo_color;n_cfg.nova_icon_color=source->nova_icon_color;n_cfg.nova_selection_color=source->nova_selection_color;
    n_cfg.nova_border_color=source->nova_border_color;n_cfg.nova_reflection_color=source->nova_reflection_color;
    n_cfg.nova_menu_opacity=source->nova_menu_opacity;n_cfg.nova_icon_opacity=source->nova_icon_opacity;
    memcpy(n_cfg.wallpaper_file,source->wallpaper_file,sizeof(n_cfg.wallpaper_file));
}
static lv_res_t theme_save(lv_obj_t *button)
{
    (void)button;bool mounted=sd_get_card_mounted();
    if(sd_mount()){theme_notice("SD card unavailable. Nothing was saved.");return LV_RES_OK;}
    if(f_stat("bootloader/sys/nyx.bin",NULL)!=FR_OK){if(!mounted)sd_unmount();theme_notice("Nyx is missing. Nothing was saved.");return LV_RES_OK;}
    if(nova_theme.pending.wallpaper_file[0]){
        lv_img_dsc_t *check=wallpaper_read(nova_theme.pending.wallpaper_file,32,18);
        if(!check){if(!mounted)sd_unmount();theme_notice("Image introuvable ou invalide. Choisissez une image BMP non compressée.");return LV_RES_OK;}
        wallpaper_free(check);
    }
    if(!nyx_options_commit_boot()){if(!mounted)sd_unmount();theme_notice("Could not save settings. Check the SD card and retry.");return LV_RES_OK;}
    nyx_config previous=n_cfg;theme_apply(&nova_theme.pending);
    if(create_nyx_config_entry(true)){theme_apply(&previous);theme_notice("Could not save appearance settings. Check the SD card and try again.");return LV_RES_OK;}
    nyx_options_appearance_active(true,true);reload_nyx(NULL,false);
    theme_notice("Settings saved. Reload Nyx to apply the changes.");return LV_RES_OK;
}
static lv_res_t theme_close(lv_obj_t *button)
{
    lv_res_t result=nyx_win_close_action(button);
    wallpaper_free(nova_theme.thumbnail);nova_theme.thumbnail=NULL;
    nova_theme.open=false;close_btn=nova_theme.parent_close;nova_theme.parent_close=NULL;
    nyx_options_appearance_active(false,false);return result;
}
static lv_res_t theme_reset(lv_obj_t *button)
{
    (void)button;nova_theme.pending.theme_bg=0x141A19;nova_theme.pending.nova_text_color=0xF2F5FA;
    nova_theme.pending.nova_border_color=0x334050;nova_theme.pending.nova_reflection_color=0x55C9F1;
    nova_theme.pending.nova_logo_color=nova_theme.pending.nova_icon_color=0x55C9F1;nova_theme.pending.nova_selection_color=0x1C2937;
    nova_theme.pending.nova_menu_opacity=nova_theme.pending.nova_icon_opacity=100;nova_theme.pending.wallpaper_file[0]=0;nova_theme.wallpaper=0;
    nyx_choices_set_selected(nova_theme.menu_choices,4);nyx_choices_set_selected(nova_theme.icon_choices,4);
    theme_controls_refresh();wallpaper_preview_refresh();return LV_RES_OK;
}
static void theme_page(lv_obj_t *page)
{
    lv_page_set_style(page,LV_PAGE_STYLE_BG,&lv_style_transp_tight);lv_page_set_style(page,LV_PAGE_STYLE_SCRL,&lv_style_transp_tight);
    lv_page_set_scrl_fit(page,false,false);lv_page_set_scrl_layout(page,LV_LAYOUT_OFF);lv_page_set_scrl_width(page,LV_HOR_RES);lv_page_set_scrl_height(page,lv_obj_get_height(page));
    lv_obj_set_pos(lv_page_get_scrl(page),0,0);lv_page_set_sb_mode(page,LV_SB_MODE_OFF);lv_obj_set_drag(lv_page_get_scrl(page),false);
}
static lv_obj_t *theme_button(lv_obj_t *parent,const char *text,lv_action_t action,int x,int y,int width,u32 id)
{
    lv_obj_t *button=lv_btn_create(parent,NULL);lv_btn_set_fit(button,false,false);lv_btn_set_layout(button,LV_LAYOUT_OFF);
    lv_obj_set_size(button,width,48);lv_obj_set_pos(button,x,y);lv_obj_set_free_num(button,id);lv_btn_set_action(button,LV_BTN_ACTION_CLICK,action);
    lv_obj_t *label=nyx_nova_text(button,text,12,13,width-24,NOVA_BODY);lv_label_set_align(label,LV_LABEL_ALIGN_CENTER);nyx_nova_fit_text(label,26);return button;
}

lv_res_t nyx_nova_theme_window(lv_obj_t *button)
{
    (void)button;if(nova_theme.open)return LV_RES_OK;
    memset(&nova_theme,0,sizeof(nova_theme));nova_theme.open=true;nova_theme.pending=n_cfg;nova_theme.parent_close=close_btn;
    wallpaper_count=0;
    bool mounted=sd_get_card_mounted();if(!sd_mount())wallpaper_scan();if(!mounted)sd_unmount();
    for(u32 i=0;i<wallpaper_count;++i)if(!strcmp(wallpaper_names[i],n_cfg.wallpaper_file))nova_theme.wallpaper=i+1;
    lv_obj_t *win=nyx_create_standard_window("Thèmes et couleurs",theme_close);nova_theme.window=win;
    lv_win_add_btn(win,NULL,nyx_tr("Réinitialiser"),theme_reset);lv_win_add_btn(win,NULL,nyx_tr("Save & reload"),theme_save);
    lv_win_ext_t *ext=lv_obj_get_ext_attr(win);theme_page(ext->page);
    lv_obj_t *tabs=lv_tabview_create(ext->page,NULL);nova_theme.tabs=tabs;
    lv_obj_set_size(tabs,LV_HOR_RES,lv_obj_get_height(ext->page));lv_obj_set_pos(tabs,0,0);lv_tabview_set_sliding(tabs,false);
    lv_obj_t *colors=lv_tabview_add_tab(tabs,"Couleurs");theme_page(colors);
    lv_obj_t *wallpaper=lv_tabview_add_tab(tabs,"Fond et transparence");theme_page(wallpaper);
    int panel_height=lv_obj_get_height(colors)-28;if(panel_height>405)panel_height=405;
    lv_obj_t *palette=nyx_nova_panel(colors,34,14,672,panel_height);
    nova_theme.preview=nyx_nova_panel(colors,730,14,516,panel_height);
    nova_theme.sample=nyx_nova_panel(nova_theme.preview,28,94,460,218);
    nova_theme.sample_icon=nyx_nova_icon_box(nova_theme.sample,"brand-mark",24,24,56,false);
    nova_theme.sample_glyph=lv_obj_get_child(nova_theme.sample_icon,NULL);
    nova_theme.sample_option_glyph=nyx_nova_glyph(nova_theme.sample,"settings-2",386,40,24);
    nova_theme.sample_text=nyx_nova_text(nova_theme.sample,"Hekate Nova Pro",24,106,410,NOVA_TITLE);nyx_nova_fit_text(nova_theme.sample_text,52);
    nova_theme.preview_caption=nyx_nova_text(nova_theme.sample,"Reflet blanc discret",24,174,410,NOVA_SMALL);
    nyx_nova_text(nova_theme.preview,"Aperçu",28,24,400,NOVA_TITLE);
    nova_theme.target_choices=nyx_choices_create(palette,"Fond\nTextes\nContours\nLogo\nIcônes\nSélection",0,3,624,42,theme_select,THEME_TARGET,false);lv_obj_set_pos(nova_theme.target_choices,24,20);
    nova_theme.hex=nyx_nova_text(palette,"",24,122,590,NOVA_TITLE);
    const char *channels[]={"R","G","B"};
    for(u32 i=0;i<3;++i){int y=180+i*46;nyx_nova_text(palette,channels[i],24,y-4,32,NOVA_BODY);lv_obj_t *slider=lv_slider_create(palette,NULL);nova_theme.sliders[i]=slider;lv_obj_set_size(slider,482,24);lv_obj_set_pos(slider,68,y);lv_bar_set_range(slider,0,255);lv_obj_set_free_num(slider,i);lv_slider_set_action(slider,theme_slider);nova_theme.values[i]=nyx_nova_text(palette,"",568,y-4,70,NOVA_BODY);}
    static const u32 presets[]={0x10151D,0x19212D,0x334050,0xF2F5FA,0x55C9F1,0xA78BFA,0x8AE6B6,0xF4B860};
    static lv_style_t swatches[8];
    for(u32 i=0;i<8;++i){lv_style_copy(&swatches[i],lv_theme_get_current()->btn.rel);swatches[i].body.empty=0;swatches[i].body.opa=LV_OPA_COVER;swatches[i].body.main_color=swatches[i].body.grad_color=LV_COLOR_HEX(presets[i]);lv_obj_t *swatch=theme_button(palette,"",theme_swatch,24+i*78,332,64,presets[i]);lv_btn_set_style(swatch,LV_BTN_STYLE_REL,&swatches[i]);lv_btn_set_style(swatch,LV_BTN_STYLE_PR,&swatches[i]);}
    lv_obj_t *left=nyx_nova_panel(wallpaper,34,14,672,panel_height);
    nyx_nova_text(left,"Fond d’écran fixe",24,22,620,NOVA_TITLE);
    nova_theme.file=nyx_nova_text(left,"",24,78,620,NOVA_BODY);nyx_nova_fit_text(nova_theme.file,52);
    theme_button(left,"Précédent",wallpaper_step,24,148,270,0);theme_button(left,"Suivant",wallpaper_step,318,148,270,1);
    nova_theme.counter=nyx_nova_text(left,"",24,218,620,NOVA_SMALL);
    lv_obj_t *help=nyx_nova_text(left,"Copiez vos images BMP dans le dossier wallpapers à la racine de la carte SD.",24,268,620,NOVA_SMALL);nyx_nova_fit_text(help,76);
    help=nyx_nova_text(left,"BMP 24/32 bits · 1920 × 1080 maximum · recadrage automatique",24,354,620,NOVA_KICKER);nyx_nova_fit_text(help,38);
    lv_obj_t *right=nyx_nova_panel(wallpaper,730,14,516,panel_height);
    nyx_nova_text(right,"Transparence",24,22,460,NOVA_TITLE);
    nyx_nova_text(right,"Fond des menus",24,88,460,NOVA_BODY);
    lv_obj_t *choices=nyx_choices_create(right,"0%\n25%\n50%\n75%\n100%",n_cfg.nova_menu_opacity/25,5,468,52,theme_select,THEME_MENU_OPACITY,true);lv_obj_set_pos(choices,24,130);
    nova_theme.menu_choices=choices;
    nyx_nova_text(right,"Fond des icônes",24,220,460,NOVA_BODY);
    choices=nyx_choices_create(right,"0%\n25%\n50%\n75%\n100%",n_cfg.nova_icon_opacity/25,5,468,52,theme_select,THEME_ICON_OPACITY,true);lv_obj_set_pos(choices,24,262);
    nova_theme.icon_choices=choices;
    help=nyx_nova_text(right,"0% : transparent · 100% : opaque. Les textes et symboles restent lisibles.",24,338,468,NOVA_SMALL);nyx_nova_fit_text(help,60);
    theme_controls_refresh();wallpaper_preview_refresh();nyx_options_appearance_active(true,false);return LV_RES_OK;
}
