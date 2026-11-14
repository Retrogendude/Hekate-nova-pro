"""Actual-C appearance parser/sanitizer/writer, with malformed INI values."""
from pathlib import Path
import ast
import re
import subprocess

WORK=Path(__file__).resolve().parent
PROJECT=WORK/'hekate-6.5.4'
SOURCE=(PROJECT/'nyx/nyx_gui/config.c').read_text(encoding='utf-8')
HEADER=(PROJECT/'nyx/nyx_gui/config.h').read_text(encoding='utf-8')
LOCALE=(PROJECT/'nyx/nyx_gui/localization.h').read_text(encoding='utf-8')
FONTS=(PROJECT/'nyx/nyx_gui/fonts/font_manager.c').read_text(encoding='utf-8')

def function(source,name):
    signature=re.search(r'^(?:static )?(?:void|bool|u32|int) '+name+r'\([^\n]*\)\s*\n\{',source,re.M)
    assert signature,name
    end=re.search(r'^\}',source[signature.end():],re.M)
    return source[signature.start():signature.end()+end.end()]

tree=ast.parse((WORK/'check_nyx_config_writer.py').read_text(encoding='utf-8'))
preamble=next(ast.literal_eval(n.value) for n in tree.body if isinstance(n,ast.Assign)
    and any(isinstance(t,ast.Name) and t.id=='preamble' for t in n.targets))
config_type=re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;',HEADER,re.S).group()
constants='\n'.join(re.findall(r'^#define HOME_LAYOUT_\w+[^\n]*',HEADER,re.M)+
    re.findall(r'^#define I18N_\w+[^\n]*',LOCALE,re.M))
body='\n'.join(function(SOURCE,name) for name in ('set_nyx_default_configuration',
    'valid_font_basename','valid_wallpaper_basename','valid_launch_basename','nyx_sanitize_appearance','appearance_number','appearance_rgb','appearance_percent','nyx_appearance_key','create_nyx_config_entry'))
body+='\n'+function(FONTS,'nyx_font_filename_valid')
tests=r'''
static void reset(void) {
 mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;
 mounts=opens=closes=writes=unmounts=0;used=0;written[0]=0;
 set_nyx_default_configuration();
}
static void expect_field(const char *key,const char *value,u32 expected) {
 reset();require(nyx_appearance_key(key,value),"recognized appearance key");
 u32 actual=0;
 if(!strcmp(key,"font_size"))actual=n_cfg.font_size;
 else if(!strcmp(key,"background_effect"))actual=n_cfg.background_effect;
 else if(!strcmp(key,"background_speed"))actual=n_cfg.background_speed;
 else if(!strcmp(key,"background_opacity"))actual=n_cfg.background_opacity;
 require(actual==expected,"numeric parser/sanitizer result");++cases;
}
static void load_written(void) {
 char copy[8192];strcpy(copy,written);
 for(char *line=strtok(copy,"\n");line;line=strtok(NULL,"\n")) {
  char *value=strchr(line,'=');if(!value)continue;*value++=0;
  nyx_appearance_key(line,value);
 }
}
int main(void) {
 reset();require(n_cfg.font_size==30&&!n_cfg.font_file[0]&&n_cfg.background_effect==0&&
  n_cfg.background_speed==2&&n_cfg.background_opacity==20,"appearance defaults");++cases;
 const char *keys[]={"font_size","background_effect","background_speed","background_opacity"};
 u32 fallback[]={30,0,2,20};
 for(int key=0;key<4;key++)for(u32 value=0;value<100;value++) {
  char number[16];sprintf(number,"%u",value);
  bool valid=key==0?(value>=20&&value<=36&&!(value&1)):
   key==1?value<=3:key==2?(value>=1&&value<=3):(value>=10&&value<=50);
  expect_field(keys[key],number,valid?value:fallback[key]);
 }
 const char *bad_numbers[]={"","-1","+1"," 20","20 ","20\n","1x","100","4294967295","1.0","0x20","２０"};
 for(int key=0;key<4;key++)for(unsigned i=0;i<sizeof bad_numbers/sizeof *bad_numbers;i++)
  expect_field(keys[key],bad_numbers[i],fallback[key]);
 expect_field("font_size","020",30);expect_field("background_effect","03",3);
 expect_field("background_speed","02",2);expect_field("background_opacity","09",20);
 for(u32 value=0;value<100;value++) {
  char number[16];sprintf(number,"%u",value);reset();
  require(n_cfg.nova_halo==1,"Nova halo defaults on");
  require(nyx_appearance_key("nova_halo",number),"Nova halo key recognized");
  require(n_cfg.nova_halo==(value<=1?value:1),"Nova halo range");++cases;
 }
 for(unsigned i=0;i<sizeof bad_numbers/sizeof *bad_numbers;i++) {
  reset();require(nyx_appearance_key("nova_halo",bad_numbers[i])&&n_cfg.nova_halo==1,"malformed Nova halo value");++cases;
 }
 for(u32 value=0;value<100;value++) {
  reset();n_cfg.font_size=n_cfg.background_effect=n_cfg.background_speed=n_cfg.background_opacity=value;
  nyx_sanitize_appearance();
  require(n_cfg.font_size==((value>=20&&value<=36&&!(value&1))?value:30),"font size bounds");
  require(n_cfg.background_effect==(value<=3?value:0),"effect bounds");
  require(n_cfg.background_speed==((value>=1&&value<=3)?value:2),"speed bounds");
  require(n_cfg.background_opacity==((value>=10&&value<=50)?value:20),"opacity bounds");++cases;
 }
 reset();n_cfg.font_size=n_cfg.background_effect=n_cfg.background_speed=n_cfg.background_opacity=UINT32_MAX;
 memset(n_cfg.font_file,'X',sizeof n_cfg.font_file);nyx_sanitize_appearance();
 require(!n_cfg.font_file[0]&&n_cfg.font_size==30&&!n_cfg.background_effect&&n_cfg.background_speed==2&&n_cfg.background_opacity==20,"corrupt u32 and unterminated path");++cases;
 const char *good_names[]={"","NotoSans.nyxf","Readable 2.nyxf","Readable_Bold-2.NyXf","A.NYXF"};
 for(unsigned i=0;i<sizeof good_names/sizeof *good_names;i++) {
  reset();require(nyx_appearance_key("font_file",good_names[i]),"font key recognized");
  require(!strcmp(n_cfg.font_file,good_names[i]),"valid basename preserved");
  if(good_names[i][0])require(nyx_font_filename_valid(good_names[i]),"loader/writer accept same filename");
  ++cases;
 }
 const char *bad_names[]={".hidden.nyxf","..","../a.nyxf","a/../b.nyxf","a\\b.nyxf","C:a.nyxf",
  "a..b.nyxf","a.nyxf\nlanguage=4","a.nyxf\r","a?.nyxf","a*.nyxf","a|b.nyxf",
  "a<b.nyxf","a>b.nyxf","a\"b.nyxf","a:b.nyxf","é.nyxf","a.ttf",".nyxf","font.nyxf ","font.nyx"};
 for(unsigned i=0;i<sizeof bad_names/sizeof *bad_names;i++) {
  reset();require(nyx_appearance_key("font_file",bad_names[i]),"bad font key still recognized");
  require(!n_cfg.font_file[0]&&!nyx_font_filename_valid(bad_names[i]),"invalid basename refused consistently");++cases;
 }
 char max_name[66];memset(max_name,'a',58);strcpy(max_name+58,".nyxf");
 reset();require(nyx_appearance_key("font_file",max_name)&&strlen(n_cfg.font_file)==63,"63-byte basename accepted");++cases;
 memset(max_name,'a',59);strcpy(max_name+59,".nyxf");
 require(nyx_appearance_key("font_file",max_name)&&!n_cfg.font_file[0]&&!nyx_font_filename_valid(max_name),"64-byte basename refused");++cases;
 /* Agreement on every ASCII character prevents a font being offered by the
  * picker, then silently reset by the configuration writer. */
 for(unsigned c=1;c<128;c++) {
  char name[]="A B.nyxf";name[1]=(char)c;
  require(valid_font_basename(name)==nyx_font_filename_valid(name),"font loader/config filename contract");++cases;
 }
 reset();nyx_config unchanged=n_cfg;
 require(!nyx_appearance_key("unknown","36")&&!memcmp(&unchanged,&n_cfg,sizeof n_cfg),"unknown keys leave settings untouched");++cases;
 reset();nyx_appearance_key("font_file","evil.nyxf\nlanguage=4");
 require(create_nyx_config_entry(false)==0&&!strstr(written,"\nlanguage=4\n"),"font INI injection refused");++cases;
 const char *roundtrip_names[]={"","NotoSans.nyxf","Readable Bold.NYXF"};
 for(unsigned font=0;font<3;font++)for(u32 size=20;size<=36;size+=2)for(u32 effect=0;effect<4;effect++)
  for(u32 speed=1;speed<=3;speed++)for(u32 opacity=10;opacity<=50;opacity+=10) {
   reset();strcpy(n_cfg.font_file,roundtrip_names[font]);n_cfg.font_size=size;
   n_cfg.background_effect=effect;n_cfg.background_speed=speed;n_cfg.background_opacity=opacity;
   u32 halo=(size/2+effect)&1;n_cfg.nova_halo=halo;
   n_cfg.language=I18N_RU;n_cfg.home_layout=HOME_LAYOUT_CAROUSEL;
   require(create_nyx_config_entry(false)==0&&writes==63,"all appearance combinations write all fields");
   set_nyx_default_configuration();load_written();
   require(!strcmp(n_cfg.font_file,roundtrip_names[font])&&n_cfg.font_size==size&&
    n_cfg.background_effect==effect&&n_cfg.background_speed==speed&&n_cfg.background_opacity==opacity&&n_cfg.nova_halo==halo,"actual INI appearance roundtrip");++cases;
  }
 printf("PASS: %d actual-C appearance configuration cases; strict numeric ranges, filename parser agreement, traversal/injection limits, corrupted values and 1620 full INI roundtrips.\n",cases);
 return 0;
}
'''
directory=WORK/'appearance-config-check-v18'
directory.mkdir(exist_ok=True)
harness=directory/'check.c';executable=directory/'check.exe'
harness.write_text(preamble+'\ntypedef uint8_t u8;\n'+constants+'\n'+config_type+
    '\nstatic nyx_config n_cfg;\n'+body+'\n'+tests,encoding='utf-8')
subprocess.run([str(WORK/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror','-O2',
    str(harness),'-o',str(executable)],check=True)
subprocess.run([str(executable)],check=True)
