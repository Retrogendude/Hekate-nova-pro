"""Run the actual language defaults, strict loader expression and writer as C."""
from pathlib import Path
import ast
import re
import subprocess

work = Path(__file__).resolve().parent
project = work/'hekate-6.5.4'
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

assignment = re.search(r'n_cfg\.language = kv->val\[0\].*?;', loader, re.S).group()
config_type = re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;', types, re.S).group()
layout_constants = '\n'.join(re.findall(r'^#define HOME_LAYOUT_\w+[^\n]*', types, re.M))
harness = (preamble+'\n'+locale_constants+'\n'+layout_constants+'\n'+config_type+
           '\nstatic nyx_config n_cfg;\n'+function('set_nyx_default_configuration')+
           '\n'+function('valid_font_basename')+'\n'+function('valid_wallpaper_basename')+'\n'+function('valid_launch_basename')+'\n'+function('nyx_sanitize_appearance')+
           '\n'+function('create_nyx_config_entry')+r'''
struct entry { const char *val; };
static u32 parse_language(const char *value) {
 struct entry item={value}; struct entry *kv=&item;
''' + assignment + r'''
 return n_cfg.language;
}
static void reset(void) {
 mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;
 mounts=opens=closes=writes=unmounts=0;used=0;written[0]=0;
 set_nyx_default_configuration();
}
int main(void) {
 reset();require(n_cfg.language==I18N_EN,"default English");cases++;
 require(I18N_LANGUAGE_COUNT==7,"stable seven-language protocol");
 for(u32 language=0;language<I18N_LANGUAGE_COUNT;language++) {
  reset();n_cfg.language=language;n_cfg.home_layout=HOME_LAYOUT_CAROUSEL;
  require(create_nyx_config_entry(false)==0,"save locale");
  char field[32];sprintf(field,"\nlanguage=%u\n",language);
  require(strstr(written,field)!=NULL,"serialized locale field");
  require(strstr(written,"\nhome_layout=1\n")!=NULL,"independent layout preserved");
  char value[16];sprintf(value,"%u",language);
  require(parse_language(value)==language,"strict locale roundtrip");cases++;
 }
 const char *invalid[]={"","-1","7","99","01","+1","1x"," 1","1 ","Russian"};
 for(unsigned i=0;i<sizeof invalid/sizeof *invalid;i++) {
  n_cfg.language=I18N_LANGUAGE_COUNT-1;
  require(parse_language(invalid[i])==I18N_EN,"malformed locale falls back");cases++;
 }
 u32 invalid_values[]={I18N_LANGUAGE_COUNT,99,UINT32_MAX};
 for(unsigned i=0;i<sizeof invalid_values/sizeof *invalid_values;i++) {
  reset();n_cfg.language=invalid_values[i];
  require(create_nyx_config_entry(false)==0,"invalid locale saves fallback");
  require(n_cfg.language==I18N_EN&&strstr(written,"\nlanguage=0\n"),"normalized English");cases++;
 }
 printf("PASS: %d actual-C language cases: defaults, seven roundtrips, malformed loader values and writer fallback.\n",cases);
 return 0;
}
''')
generated=work/'language_config_generated_v18.c'
binary=work/'language_config_generated_v18.exe'
generated.write_text(harness,encoding='utf-8')
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror',str(generated),'-o',str(binary)],check=True)
subprocess.run([str(binary)],check=True)
