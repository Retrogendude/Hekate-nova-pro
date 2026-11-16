from pathlib import Path
import ast,re,subprocess,json
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33.3/nyx/nyx_gui'
source=(P/'config.c').read_text();header=(P/'config.h').read_text();locale=(P/'localization.h').read_text()
tree=ast.parse((W/'check_nyx_config_writer.py').read_text())
preamble=next(ast.literal_eval(x.value)for x in tree.body if isinstance(x,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='preamble'for t in x.targets))
def function(name):
 start=re.search(r'^(?:static )?(?:void|bool|u32|int) '+name+r'\([^\n]*\)\s*\n\{',source,re.M);assert start,name
 end=re.search(r'^\}',source[start.end():],re.M);return source[start.start():start.end()+end.end()]
types=re.search(r'typedef struct _nyx_config\s*\{.*?\} nyx_config;',header,re.S)[0]
constants='\n'.join(re.findall(r'^#define (?:HOME_LAYOUT|I18N)_\w+[^\n]*',header+'\n'+locale,re.M))
code='\n'.join(function(n)for n in ['set_nyx_default_configuration','valid_font_basename','valid_wallpaper_basename','valid_launch_basename','nyx_sanitize_appearance','appearance_number','appearance_rgb','appearance_percent','nyx_appearance_key','create_nyx_config_entry'])
tests=r'''
static void reset(void){mounted=false;fail_mount=fail_open=fail_close=fail_write_at=0;mounts=opens=closes=writes=unmounts=0;used=0;written[0]=0;set_nyx_default_configuration();}
static void read_back(void){char copy[8192];strcpy(copy,written);for(char*l=strtok(copy,"\n");l;l=strtok(NULL,"\n")){char*v=strchr(l,'=');if(v){*v++=0;nyx_appearance_key(l,v);}}}
int main(void){
 reset();require(n_cfg.nova_halo_color1==0x3590FF&&n_cfg.nova_halo_color2==0x9C63FF,"legacy configuration keeps blue/violet defaults");++cases;
 const char*keys[]={"nova_halo_color1","nova_halo_color2"};const u32 defaults[]={0x3590FF,0x9C63FF};
 for(int stop=0;stop<2;++stop){
  const char*valid[]={"000000","FFFFFF","aAbBcC","000001","FF0000","00FF00"};const u32 values[]={0,0xFFFFFF,0xAABBCC,1,0xFF0000,0x00FF00};
  for(int i=0;i<6;++i){reset();require(nyx_appearance_key(keys[stop],valid[i]),"recognized key");require((stop?n_cfg.nova_halo_color2:n_cfg.nova_halo_color1)==values[i],"exact RGB");require((stop?n_cfg.nova_halo_color1:n_cfg.nova_halo_color2)==defaults[!stop],"other stop unchanged");require(n_cfg.theme_bg==0x10151D,"background unchanged");++cases;}
  const char*invalid[]={"","#FF0000","GG0000","1000000","-1","ABCDEF\nfont_size=36"};
  for(int i=0;i<6;++i){reset();nyx_appearance_key(keys[stop],invalid[i]);require((stop?n_cfg.nova_halo_color2:n_cfg.nova_halo_color1)==defaults[stop],"bad RGB fallback");++cases;}
 }
 reset();n_cfg.nova_halo_color1=0;n_cfg.nova_halo_color2=0xABCDEF;require(create_nyx_config_entry(true)==0,"writer succeeds");require(strstr(written,"\nnova_halo_color1=000000")&&strstr(written,"\nnova_halo_color2=ABCDEF"),"both keys serialized");int total_writes=writes;
 n_cfg.nova_halo_color1=n_cfg.nova_halo_color2=UINT32_MAX;read_back();require(n_cfg.nova_halo_color1==0&&n_cfg.nova_halo_color2==0xABCDEF,"round trip including black");++cases;
 for(int failure=1;failure<=total_writes;++failure){reset();fail_write_at=failure;require(create_nyx_config_entry(true)!=0,"write failure reported");require(!mounted,"mount ownership restored");++cases;}
 reset();n_cfg.nova_halo_color1=n_cfg.nova_halo_color2=UINT32_MAX;nyx_sanitize_appearance();require(n_cfg.nova_halo_color1==defaults[0]&&n_cfg.nova_halo_color2==defaults[1],"corrupt colors sanitized");++cases;
 printf("PASS: %d actual-C Halo config checks; independent RGB, backward defaults, writer/read round trip and all write failures\n",cases);return 0;
}
'''
D=W/'nova33_3-config-check';D.mkdir(exist_ok=True);(D/'test.c').write_text(preamble+'\n'+constants+'\n'+types+'\nstatic nyx_config n_cfg;\n'+code+tests)
subprocess.run([str(W/'toolchains/w64devkit/bin/gcc.exe'),'-std=c11','-Wall','-Wextra','-Werror','-O2',str(D/'test.c'),'-o',str(D/'test.exe')],check=True)
r=subprocess.run([str(D/'test.exe')],capture_output=True,text=True);print(r.stdout+r.stderr);assert not r.returncode
(W/'nova33_3-config-check.json').write_text(json.dumps({'passed':True,'actual_c':True,'result':r.stdout.strip()},indent=2))
