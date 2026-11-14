"""Run the actual language defaults, strict loader expression and writer as C."""
from pathlib import Path
import ast
import re
import subprocess

work = Path(__file__).resolve().parent
project = work/'hekate-nova-pro-v20-dock'
config = (project/'nyx/nyx_gui/config.c').read_text(encoding='utf-8')
types = (project/'nyx/nyx_gui/config.h').read_text(encoding='utf-8')
loader = (project/'nyx/nyx_gui/nyx.c').read_text(encoding='utf-8')
locale = (project/'nyx/nyx_gui/localization.h').read_text(encoding='utf-8')
enum = re.search(r'(?:typedef\s+)?enum[^{}]*\{[^}]*\bI18N_EN\b[^}]*\}[^;]*;', locale, re.S)
locale_constants = enum.group() if enum else '\n'.join(re.findall(r'^#define I18N_\w+[^\n]*', locale, re.M))
assert 'I18N_LANGUAGE_COUNT' in locale_constants
base_tree = ast.parse((work/'check_nyx_config_writer.py').read_text(encoding='utf-8'))
preamble = next(ast.literal_eval(node.value) for node in base_tree.body
                if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'preamble' for t in node.targets))

def function(name):
    start = re.search(r'^(?:static )?(?:void|int|bool|u32) '+name+r'\([^\n]*\)\s*\n\{', config, re.M).start()
    opening = config.index('{', start)
    end = config.index('\n}', opening) + 2
    return config[start:end]

assignment = re.search(r'n_cfg\.tv_auto = [^;]+;', loader).group()
screen_assignment = re.search(r'n_cfg\.tv_keep_screen = [^;]+;', loader).group()
config_type = re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;', types, re.S).group()
layout_constants = '\n'.join(re.findall(r'^#define HOME_LAYOUT_\w+[^\n]*', types, re.M))
harness = (preamble+'\n'+locale_constants+'\n'+layout_constants+'\n'+config_type+
           '\nstatic nyx_config n_cfg;\n'+function('set_nyx_default_configuration')+
           '\n'+function('valid_font_basename')+'\n'+function('valid_wallpaper_basename')+'\n'+function('valid_launch_basename')+'\n'+function('nyx_sanitize_appearance')+
           '\n'+function('create_nyx_config_entry')+r'''
struct entry { const char *val; };
static u32 parse_auto(const char *value) {
 struct entry item={value}; struct entry *kv=&item;
''' + assignment + r'''
 return n_cfg.tv_auto;
}
static u32 parse_screen(const char *value) {
 struct entry item={value}; struct entry *kv=&item;
''' + screen_assignment + r'''
 return n_cfg.tv_keep_screen;
}
static void reset(void) {
 mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;
 mounts=opens=closes=writes=unmounts=0;used=0;written[0]=0;
 set_nyx_default_configuration();
}
int main(void) {
 reset();require(!n_cfg.tv_auto&&n_cfg.tv_keep_screen,"defaults preserve handheld behavior");cases++;
 for(unsigned automatic=0;automatic<2;automatic++)for(unsigned screen=0;screen<2;screen++) {
  reset();n_cfg.tv_auto=automatic;n_cfg.tv_keep_screen=screen;n_cfg.home_layout=HOME_LAYOUT_CAROUSEL;
  require(create_nyx_config_entry(false)==0,"persist dock choices");
  char key[32];sprintf(key,"\ntv_auto=%u\n",automatic);require(strstr(written,key)!=NULL,"saved automatic field");
  sprintf(key,"\ntv_keep_screen=%u\n",screen);require(strstr(written,key)!=NULL,"saved screen field");
  require(strstr(written,"\nhome_layout=1\n")!=NULL,"unrelated setting preserved");
  set_nyx_default_configuration();char tmp[8192];strcpy(tmp,written);
  for(char*line=strtok(tmp,"\n");line;line=strtok(NULL,"\n")) {
   char *v=strchr(line,'=');if(!v)continue;*v++=0;
   if(!strcmp(line,"tv_auto"))parse_auto(v);
   if(!strcmp(line,"tv_keep_screen"))parse_screen(v);
  }
  require(n_cfg.tv_auto==automatic&&n_cfg.tv_keep_screen==screen,"simulated reboot roundtrip");cases++;
 }
 const char*bad[]={"","-1","2","99","01","+1","1x"," 1","1 ","ON"};
 for(unsigned i=0;i<sizeof bad/sizeof *bad;i++) {
  require(!parse_auto(bad[i]),"invalid auto value defaults OFF");
  require(parse_screen(bad[i]),"invalid screen value keeps LCD visible");cases++;
 }
 require(parse_auto("1")&&!parse_auto("0"),"valid auto loader values");
 require(parse_screen("1")&&!parse_screen("0"),"valid LCD loader values");cases++;
 reset();n_cfg.tv_auto=99;n_cfg.tv_keep_screen=99;
 require(create_nyx_config_entry(false)==0&&!n_cfg.tv_auto&&n_cfg.tv_keep_screen==1,"normalize corrupted config");cases++;
 int total=writes;
 for(int i=1;i<=total;i++){reset();fail_write_at=i;require(create_nyx_config_entry(false)!=0,"detect truncated config write");cases++;}
 printf("PASS: %d dock configuration cases: defaults, restart roundtrips, strict loader and write failures.\n",cases);
 return 0;
}
''')
generated=work/'dock_config_generated_v20.c'
binary=work/'dock_config_generated_v20.exe'
generated.write_text(harness,encoding='utf-8')
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(generated),'-o',str(binary)],check=True)
subprocess.run([str(binary)],check=True)
