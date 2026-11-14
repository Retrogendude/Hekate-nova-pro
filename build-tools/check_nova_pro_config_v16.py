"""Exercise the actual RGB/opacity/wallpaper parser, defaults and writer."""
from pathlib import Path
import ast,re,subprocess
work=Path(__file__).resolve().parent;root=work/'hekate-6.5.4/nyx/nyx_gui'
source=(root/'config.c').read_text();header=(root/'config.h').read_text();locale=(root/'localization.h').read_text()
def function(name):
    at=re.search(r'^(?:static )?(?:void|bool|u32|int) '+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert at,name
    end=re.search(r'^\}',source[at.end():],re.M);return source[at.start():at.end()+end.end()]
tree=ast.parse((work/'check_nyx_config_writer.py').read_text())
preamble=next(ast.literal_eval(x.value)for x in tree.body if isinstance(x,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='preamble'for t in x.targets))
types=re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;',header,re.S).group()
constants='\n'.join(re.findall(r'^#define (?:HOME_LAYOUT|I18N)_\w+[^\n]*',header+'\n'+locale,re.M))
code='\n'.join(function(x)for x in ('set_nyx_default_configuration','valid_font_basename','valid_wallpaper_basename','nyx_sanitize_appearance','appearance_number','appearance_rgb','appearance_percent','nyx_appearance_key','create_nyx_config_entry'))
tests=r'''
static void reset(void){mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;mounts=opens=closes=writes=unmounts=0;used=0;written[0]=0;set_nyx_default_configuration();}
static void load_written(void){char copy[8192];strcpy(copy,written);for(char*line=strtok(copy,"\n");line;line=strtok(NULL,"\n")){char*value=strchr(line,'=');if(value){*value++=0;nyx_appearance_key(line,value);}}}
static u32 get_color(int i){return i==0?n_cfg.nova_text_color:i==1?n_cfg.nova_border_color:i==2?n_cfg.nova_reflection_color:i==3?n_cfg.nova_logo_color:i==4?n_cfg.nova_icon_color:n_cfg.nova_selection_color;}
int main(void){
 reset();require(n_cfg.nova_menu_opacity==25&&n_cfg.nova_icon_opacity==100&&!strcmp(n_cfg.wallpaper_file,"Nova-Carbon.bmp")&&n_cfg.theme_bg==0x10151D,"first-start Carbon and 75 percent transparency");++cases;
 const char*keys[]={"nova_text_color","nova_border_color","nova_reflection_color","nova_logo_color","nova_icon_color","nova_selection_color"};u32 defaults[]={0xF2F5FA,0x334050,0x55C9F1,0x55C9F1,0x55C9F1,0x1C2937};
 const char*valid[]={"000000","ffffff","ABCDEF","aAbBcC","000001","FF0000","00FF00","0000FF"};u32 values[]={0,0xFFFFFF,0xABCDEF,0xAABBCC,1,0xFF0000,0xFF00,0xFF};
 const char*bad[]={"","#55c9f1","0x55c9f1","55c9f","055c9f1","gggggg","55c9f1\n"," 55c9f1","-00001","１２３４５６","FFFFFF=0"};
 for(int field=0;field<6;++field){
  for(unsigned i=0;i<sizeof(valid)/sizeof(*valid);++i){reset();require(nyx_appearance_key(keys[field],valid[i]),"RGB key");for(int j=0;j<6;++j)require(get_color(j)==(j==field?values[i]:defaults[j]),"independent RGB value");require(n_cfg.theme_bg==0x10151D,"text/border/reflection do not alter background");++cases;}
  for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);++i){reset();nyx_appearance_key(keys[field],bad[i]);require(get_color(field)==defaults[field],"invalid RGB fallback");++cases;}
 }
 for(int field=0;field<2;++field)for(u32 value=0;value<=200;++value){char text[16];sprintf(text,"%u",value);reset();nyx_appearance_key(field?"nova_icon_opacity":"nova_menu_opacity",text);require((field?n_cfg.nova_icon_opacity:n_cfg.nova_menu_opacity)==(value<=100?value:100),"opacity range");require((field?n_cfg.nova_menu_opacity:n_cfg.nova_icon_opacity)==(field?25:100),"opacity independence");++cases;}
 const char*files[]={"","Nova-Carbon.bmp","A B.BMP","wallpaper_01.BmP"};
 for(unsigned i=0;i<sizeof(files)/sizeof(*files);++i){reset();nyx_appearance_key("wallpaper_file",files[i]);require(!strcmp(n_cfg.wallpaper_file,files[i]),"valid wallpaper basename");++cases;}
 const char*badfiles[]={"../evil.bmp","a/b.bmp","a\\b.bmp",".hidden.bmp","x.bmp\nfont_size=36","foo.png","foo.bmp.txt","é.bmp","..bmp","","x:\\a.bmp"};
 for(unsigned i=0;i<sizeof(badfiles)/sizeof(*badfiles);++i){reset();nyx_appearance_key("wallpaper_file",badfiles[i]);require(!n_cfg.wallpaper_file[0],"wallpaper traversal/injection/extension refused");++cases;}
 reset();memset(n_cfg.wallpaper_file,'a',64);n_cfg.nova_text_color=n_cfg.nova_border_color=n_cfg.nova_reflection_color=n_cfg.nova_menu_opacity=n_cfg.nova_icon_opacity=UINT32_MAX;nyx_sanitize_appearance();
 require(!n_cfg.wallpaper_file[0]&&n_cfg.nova_text_color==defaults[0]&&n_cfg.nova_border_color==defaults[1]&&n_cfg.nova_reflection_color==defaults[2]&&n_cfg.nova_menu_opacity==100&&n_cfg.nova_icon_opacity==100,"corrupt struct sanitized");++cases;
 for(int file=0;file<3;++file)for(u32 menu=0;menu<=100;menu+=25)for(u32 icons=0;icons<=100;icons+=25){
  reset();n_cfg.nova_logo_color=0xA020C0;n_cfg.nova_icon_color=0xE0C060;n_cfg.nova_selection_color=0x223344;n_cfg.nova_text_color=0xAABBCC;n_cfg.nova_border_color=0;n_cfg.nova_reflection_color=0xFFFFFF;n_cfg.nova_menu_opacity=menu;n_cfg.nova_icon_opacity=icons;strcpy(n_cfg.wallpaper_file,files[file]);
  require(!create_nyx_config_entry(false),"write extended settings");set_nyx_default_configuration();load_written();
  require(n_cfg.nova_logo_color==0xA020C0&&n_cfg.nova_icon_color==0xE0C060&&n_cfg.nova_selection_color==0x223344&&n_cfg.nova_text_color==0xAABBCC&&n_cfg.nova_border_color==0&&n_cfg.nova_reflection_color==0xFFFFFF&&n_cfg.nova_menu_opacity==menu&&n_cfg.nova_icon_opacity==icons&&!strcmp(n_cfg.wallpaper_file,files[file]),"all new fields roundtrip independently");++cases;
 }
 printf("PASS: %d Nova Pro configuration cases; independent RGB colors, all opacity values, black/white colors, malformed input, wallpaper names and 75 extended INI roundtrips.\n",cases);return 0;
}
'''
directory=work/'nova-pro-config-check-v16';directory.mkdir(exist_ok=True)
program=directory/'check.c';program.write_text(preamble+'\n'+constants+'\n'+types+'\nstatic nyx_config n_cfg;\n'+code+tests)
exe=directory/'check.exe';cc=work/'toolchains/w64devkit/bin/gcc.exe'
subprocess.run([str(cc),'-std=c11','-Wall','-Wextra','-Werror','-O2',str(program),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
