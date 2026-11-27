"""Exercise the actual C loader with Chinese data, cached roles and failures."""
from pathlib import Path
import ast,re,subprocess,json
W=Path(__file__).resolve().parent;P=W/'hekate-nova-pro-v33'
old=ast.parse((W/'check_font_manager.py').read_text(encoding='utf-8'))
preamble=next(ast.literal_eval(n.value)for n in old.body if isinstance(n,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='PREAMBLE'for t in n.targets))
preamble=preamble.replace('u32 font_size; char font_file[64];','u32 font_size,language; char font_file[64];')
header=(P/'nyx/nyx_gui/fonts/font_manager.h').read_text(encoding='utf-8');header=re.sub(r'^#include[^\n]*\n','',header,flags=re.M)
preamble+='\n#define I18N_EN 0\n#define I18N_RU 4\n#define I18N_ZH_CN 7\n'+header
preamble=preamble.replace('static int f_open(', 'static char last_open_path[120];\nstatic int f_open(').replace('(void)mode;if(fail_open','(void)mode;strcpy(last_open_path,name);if(fail_open')
source=re.sub(r'^#include[^\n]*\n','',(P/'nyx/nyx_gui/fonts/font_manager.c').read_text(encoding='utf-8'),flags=re.M)
harness=r'''
#undef malloc
#undef calloc
#undef free
static unsigned checks;
#define CHECK(c) do{++checks;if(!(c)){fprintf(stderr,"Font assertion %d: %s\n",__LINE__,#c);abort();}}while(0)
static void clean(void){for(u32 i=0;i<ARRAY_SIZE(ui_faces);++i)release_font(&ui_faces[i]);release_font(&regular);release_font(&small);release_font(&mono);release_font(&regular_icons);release_font(&small_icons);CHECK(!allocation_count);CHECK(!open_count);}
static void read_file(const char *path){FILE*f=fopen(path,"rb");CHECK(f);fseek(f,0,SEEK_END);blob_size=ftell(f);rewind(f);blob=malloc(blob_size);CHECK(blob);CHECK(fread(blob,1,blob_size,f)==blob_size);fclose(f);}
int main(int argc,char **argv){
 CHECK(argc==2);read_file(argv[1]);
 for(u32 lang=0;lang<8;++lang){CHECK(nyx_font_language_locked(lang)==(lang==7));CHECK(!strcmp(nyx_font_effective_filename(lang,""),lang==7?NYX_FONT_CHINESE:lang==4?NYX_FONT_ORIGINAL:"Lecture.nyxf"));CHECK(!strcmp(nyx_font_effective_filename(lang,"Custom.nyxf"),lang==7?NYX_FONT_CHINESE:"Custom.nyxf"));}
 CHECK(nyx_font_available_at_size(NYX_FONT_ORIGINAL,30));CHECK(nyx_font_validate_header(blob,blob_size));CHECK(!nyx_font_validate_header(blob,17*1024*1024));
 for(u32 size=20;size<=36;size+=2){
  n_cfg.language=7;n_cfg.font_size=size;strcpy(n_cfg.font_file,"Lecture-fixe.nyxf");
  nyx_font_manager_init();CHECK(n_cfg.language==7);CHECK(!strcmp(n_cfg.font_file,"Lecture-fixe.nyxf"));CHECK(!mounted);CHECK(strstr(last_open_path,"bootloader/sys/")==last_open_path);
  CHECK(glyph_index(nyx_font_regular(),0x4E2D)>=0);CHECK(glyph_index(nyx_font_small(),0x4E2D)>=0);CHECK(glyph_index(nyx_font_mono(),0x4E2D)>=0);
  for(u32 px=14;px<=36;px+=2){const lv_font_t*f=nyx_font_ui(px);CHECK(get_width(f,0x4E2D)>0);CHECK(get_bitmap(f,0x4E2D));u32 reads=read_calls,allocs=allocation_calls;CHECK(nyx_font_ui(px)==f);CHECK(read_calls==reads);CHECK(allocation_calls==allocs);}
  clean();
 }
 n_cfg.language=7;n_cfg.font_size=30;strcpy(n_cfg.font_file,"Custom.nyxf");fail_open=true;nyx_font_manager_init();CHECK(n_cfg.language==0);CHECK(!strcmp(n_cfg.font_file,"Custom.nyxf"));CHECK(strstr(nyx_font_status(),"English"));clean();fail_open=false;
 blob[28]^=1;CHECK(!nyx_font_available_at_size(NYX_FONT_CHINESE,30));CHECK(!mounted);clean();blob[28]^=1;
 n_cfg.language=1;strcpy(n_cfg.font_file,NYX_FONT_ORIGINAL);nyx_font_manager_init();CHECK(nyx_font_regular()==&interui_30);CHECK(!strcmp(n_cfg.font_file,NYX_FONT_ORIGINAL));clean();
 for(u32 failure=1;failure<=16;++failure){n_cfg.language=7;n_cfg.font_size=30;strcpy(n_cfg.font_file,"Personal.nyxf");allocation_calls=0;fail_allocation=failure;nyx_font_manager_init();CHECK(!strcmp(n_cfg.font_file,"Personal.nyxf"));CHECK(nyx_font_regular());if(n_cfg.language==7)CHECK(get_width(nyx_font_regular(),0x4E2D)>0);clean();fail_allocation=0;}
 free(blob);printf("PASS: %u actual-C Chinese font, locale policy, cached role, fallback, allocation and preservation assertions\n",checks);return 0;
}
'''
out=W/'nova33-font-tests';out.mkdir(exist_ok=True);(out/'test.c').write_text(preamble+'\n'+source+'\n'+harness,encoding='utf-8')
cc=W/'toolchains/w64devkit/bin/gcc.exe';exe=out/'test.exe'
subprocess.run([str(cc),'-std=gnu11','-O2','-I'+str(P/'bdk'),str(out/'test.c'),'-o',str(exe)],check=True)
r=subprocess.run([str(exe),str(P/'nyx/nyx_gui/fonts/sd/Nova-Chinese-Simplified.nyxf')],capture_output=True,text=True)
print(r.stdout+r.stderr);assert r.returncode==0
(W/'nova33-font-check.json').write_text(json.dumps({'passed':True,'result':r.stdout.strip(),'physical_console_test':False},indent=2))
