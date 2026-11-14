"""Compile and execute the actual SD font loader with bounded fake SD I/O."""
from pathlib import Path
import json
import re
import subprocess
import struct

WORK = Path(__file__).resolve().parent
PROJECT = WORK / "hekate-6.5.4"
source = (PROJECT / "nyx/nyx_gui/fonts/font_manager.c").read_text(encoding="utf-8")
source = re.sub(r"^#include[^\n]*\n", "", source, flags=re.M)
directory = WORK / "font-manager-check"
directory.mkdir(exist_ok=True)

PREAMBLE = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libs/lvgl/lv_misc/lv_font.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef unsigned int UINT;
bool nyx_font_available_at_size(const char *filename,u32 size);
typedef struct { u32 font_size; char font_file[64]; } config_t;
static config_t n_cfg;
static const char *nyx_tr(const char *text){return text;}
static void *allocations[4096]; static u32 allocation_count, allocation_calls, fail_allocation;
static void *checked_malloc(size_t size) {
    if (++allocation_calls == fail_allocation) return NULL;
    void *p = malloc(size);
    if (p) { assert(allocation_count < 4096); allocations[allocation_count++] = p; }
    return p;
}
static void checked_free(void *p) {
    if (!p) return;
    for (u32 i=0;i<allocation_count;i++) if (allocations[i]==p) {
        allocations[i]=allocations[--allocation_count]; free(p); return;
    }
    assert(!"Free of unowned pointer");
}
static void *checked_calloc(size_t count,size_t width) {
    void *p=checked_malloc(count*width); if(p)memset(p,0,count*width);return p;
}
static const u8 fake_bitmap[8*48] = {[0 ... 8*48-1] = 255};
lv_font_t interui_30={.h_px=30,.bpp=8}, interui_20={.h_px=20,.bpp=8},ubuntu_mono={.h_px=20,.bpp=8},hekate_symbol_30={.h_px=30,.bpp=8},hekate_symbol_20={.h_px=20,.bpp=8};
const u8 *lv_font_get_bitmap(const lv_font_t *font,u32 cp) {return font->get_bitmap?font->get_bitmap(font,cp):fake_bitmap;}
uint8_t lv_font_get_width(const lv_font_t *font,u32 cp) {return font->get_width?font->get_width(font,cp):8;}
uint8_t lv_font_get_bpp(const lv_font_t *font,u32 cp) {(void)cp;return font->bpp;}
static u8 *blob; static u32 blob_size; static u32 read_calls, fail_read, open_count; static bool fail_open,fail_close;
typedef struct {u32 offset;} FIL;
typedef struct {u32 unused;} DIR;
typedef struct {char fname[256];u8 fattrib;} FILINFO;
#define FR_OK 0
#define FA_READ 1
#define AM_DIR 0x10
#define AM_HID 0x02
#define f_size(file) blob_size
static int f_open(FIL *f,const char *name,int mode) {(void)mode;if(fail_open||strstr(name,"missing"))return 1;f->offset=0;open_count++;return 0;}
static int f_close(FIL *f) {(void)f;assert(open_count);open_count--;return fail_close;}
static int f_lseek(FIL *f,u32 offset) {if(offset>blob_size)return 1;f->offset=offset;return 0;}
static int f_read(FIL *f,void *out,u32 len,UINT *actual) {if(++read_calls==fail_read){*actual=0;return 1;}if(len>blob_size-f->offset)len=blob_size-f->offset;memcpy(out,blob+f->offset,len);f->offset+=len;*actual=len;return 0;}
static int f_opendir(DIR *d,const char*p){(void)d;(void)p;return 1;}
static int f_readdir(DIR*d,FILINFO*f){(void)d;f->fname[0]=0;return 0;}
static int f_closedir(DIR*d){(void)d;return 0;}
static bool mounted,fail_mount;
static bool sd_get_card_mounted(void){return mounted;}
static int sd_mount(void){if(fail_mount)return 1;mounted=true;return 0;}
static void sd_unmount(void){mounted=false;}
#define malloc checked_malloc
#define calloc checked_calloc
#define free checked_free
'''

HARNESS = r'''
#undef malloc
#undef calloc
#undef free
static unsigned checks;
#define CHECK(c) do { checks++; if(!(c)){fprintf(stderr,"Failed at %d: %s\n",__LINE__,#c);abort();}}while(0)
static void put32(u8 *p,u32 v){p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static u8 *pristine; static u32 pristine_size;
static void cleanup(void){release_font(&regular);release_font(&small);release_font(&mono);release_font(&regular_icons);release_font(&small_icons);CHECK(!allocation_count);CHECK(!open_count);}
static void reset(void) {
    cleanup();memcpy(blob,pristine,pristine_size);blob_size=pristine_size;
    read_calls=0;fail_read=0;fail_close=false;fail_open=false;fail_allocation=0;allocation_calls=0;mounted=false;fail_mount=false;
}
static void rehash_header(void){put32(blob+28,hash_bytes(blob,28));}
static void rehash_table(void){put32(blob+24,hash_bytes(blob+32,read32(blob+12)*32));rehash_header();}
static void rehash_section(u8 *entry){put32(entry+24,hash_bytes(blob+read32(entry+8),read32(entry+20)-read32(entry+8)));rehash_table();}
static u8 *entry_for(u32 height){for(u32 i=0;i<read32(blob+12);i++)if(read32(blob+32+i*32)==height)return blob+32+i*32;return NULL;}
int main(int argc,char**argv){
    CHECK(argc==2);FILE*f=fopen(argv[1],"rb");CHECK(f);fseek(f,0,SEEK_END);pristine_size=ftell(f);rewind(f);
    pristine=malloc(pristine_size);blob=malloc(pristine_size);CHECK(pristine&&blob);CHECK(fread(pristine,1,pristine_size,f)==pristine_size);fclose(f);memcpy(blob,pristine,pristine_size);blob_size=pristine_size;
    CHECK(nyx_font_filename_valid("Lecture.nyxf"));CHECK(nyx_font_filename_valid("My Font-2.nyxf"));CHECK(nyx_font_filename_valid("font.NYXF"));
    const char*bad[]={"",".hidden.nyxf","../evil.nyxf","font/evil.nyxf","font\\evil.nyxf","C:evil.nyxf","double..name.nyxf","bad?.nyxf","font.ttf","bad#.nyxf","Русский.nyxf"};
    for(u32 i=0;i<ARRAY_SIZE(bad);i++)CHECK(!nyx_font_filename_valid(bad[i]));
    CHECK(nyx_font_available(""));CHECK(nyx_font_available("Lecture.nyxf"));CHECK(!nyx_font_available("missing.nyxf"));
    CHECK(!mounted);mounted=true;CHECK(nyx_font_available("Lecture.nyxf"));CHECK(mounted);mounted=false;
    fail_mount=true;CHECK(!nyx_font_available("Lecture.nyxf"));fail_mount=false;
    CHECK(nyx_font_validate_header(blob,blob_size));CHECK(!nyx_font_validate_header(blob,blob_size-1));CHECK(!nyx_font_validate_header(blob,17*1024*1024));
    CHECK(load_selected("Lecture.nyxf",30,20));CHECK(regular.font.h_px==30&&small.font.h_px==20);
    CHECK(glyph_index(&regular.font,0x42F)>=0);CHECK(get_bitmap(&regular.font,0xE9));CHECK(get_width(&regular.font,0x110000)<0);cleanup();
    // Every supported size loads both its main and small section.
    for(u32 height=20;height<=36;height+=2){reset();u32 hint=((height*2/3)+1)&~1u;if(hint<14)hint=14;CHECK(load_selected("Lecture.nyxf",height,hint));CHECK(regular.font.h_px==height&&small.font.h_px==hint);cleanup();}
    // Header validation, including recomputed checksums for structural errors.
    for(u32 field=0;field<8;field++){reset();blob[field]^=0xFF;CHECK(!load_selected("Lecture.nyxf",30,20));cleanup();}
    reset();put32(blob+12,25);rehash_header();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();put32(blob+16,64);rehash_header();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();put32(blob+20,1);rehash_header();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();blob[24]^=1;rehash_header();CHECK(!load_selected("Lecture.nyxf",30,20));
    // Table entries cannot overlap or repeat heights, escape the file or omit a requested size.
    reset();put32(blob+32+32,14);rehash_table();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();put32(blob+32+8,0);rehash_table();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();put32(blob+32+20,blob_size+1);rehash_table();CHECK(!load_selected("Lecture.nyxf",30,20));
    reset();put32(entry_for(30),39);rehash_table();CHECK(!load_selected("Lecture.nyxf",30,20));
    // Selected section: hash, count, offsets, width, sorted Unicode and bitmap bounds.
    for(u32 mode=0;mode<12;mode++){
        reset();u8*entry=entry_for(30);u32 glyph_start=read32(entry+8);u8*g=blob+glyph_start;
        switch(mode){
        case 0:blob[read32(entry+12)]^=1;break;
        case 1:put32(entry+4,1025);break;
        case 2:put32(entry+12,glyph_start+1);break;
        case 3:put32(entry+16,2*1024*1024+1);break;
        case 4:put32(g+4,0);break;
        case 5:put32(g+4,129);break;
        case 6:put32(g+8,1);break;
        case 7:put32(g+12,read32(g));break;
        case 8:put32(g,0xD800);break;
        case 9:put32(g,0x110000);break;
        case 10:put32(g,31);break;
        case 11:put32(entry+28,1);break;
        }
        if(mode>=4&&mode<=10)rehash_section(entry);else rehash_table();
        CHECK(!load_selected("Lecture.nyxf",30,20));cleanup();
    }
    // Short/failed reads and close errors clean up every opened handle and allocation.
    reset();CHECK(load_selected("Lecture.nyxf",30,20));u32 reads=read_calls;cleanup();
    for(u32 pos=1;pos<=reads;pos++){reset();fail_read=pos;CHECK(!load_selected("Lecture.nyxf",30,20));cleanup();}
    reset();fail_close=true;CHECK(!load_selected("Lecture.nyxf",30,20));cleanup();
    reset();CHECK(load_selected("Lecture.nyxf",30,20));u32 allocations_needed=allocation_calls;cleanup();
    for(u32 pos=1;pos<=allocations_needed;pos++){reset();fail_allocation=pos;CHECK(!load_selected("Lecture.nyxf",30,20));cleanup();}
    // Bitmap scaling accepts 8bpp built-ins and creates visible 4bpp pixels.
    reset();CHECK(scale_font(&interui_30,36,&regular,false,false));CHECK(regular.font.h_px==36);CHECK(regular.bitmap[0]==255);cleanup();
    reset();CHECK(scale_font(&hekate_symbol_30,24,&regular_icons,true,false));CHECK(regular_icons.bitmap[0]==255);cleanup();
    reset();n_cfg.font_size=36;strcpy(n_cfg.font_file,"Lecture.nyxf");nyx_font_manager_init();CHECK(nyx_font_size()==36);CHECK(nyx_font_regular()->h_px==36&&nyx_font_small()->h_px==24);CHECK(!strcmp(nyx_font_status(),"SD font loaded"));cleanup();
    reset();n_cfg.font_size=35;strcpy(n_cfg.font_file,"missing.nyxf");nyx_font_manager_init();CHECK(nyx_font_size()==30);CHECK(nyx_font_regular()==&interui_30);CHECK(strstr(nyx_font_status(),"Default"));cleanup();
    reset();n_cfg.font_size=24;n_cfg.font_file[0]=0;nyx_font_manager_init();CHECK(nyx_font_regular()->h_px==24);CHECK(nyx_font_small()->h_px==16);CHECK(nyx_font_mono()->h_px==16);cleanup();
    reset();n_cfg.font_size=36;strcpy(n_cfg.font_file,"Lecture.nyxf");nyx_font_manager_init();u32 init_cost=allocation_calls;cleanup();
    for(u32 failure=1;failure<=init_cost;failure++) {
        reset();n_cfg.font_size=36;strcpy(n_cfg.font_file,"Lecture.nyxf");fail_allocation=failure;nyx_font_manager_init();
        const lv_font_t *faces[]={nyx_font_regular(),nyx_font_small(),nyx_font_mono()};
        for(unsigned face=0;face<3;face++) { const lv_font_t *p=faces[face];u32 links=0;
            for(;p;p=p->next_page) { CHECK(++links<5);CHECK(p->h_px==faces[face]->h_px); }
        }
        cleanup();
    }
    // Save-time validation checks the pending size without replacing active fonts.
    reset();CHECK(load_selected("Lecture.nyxf",30,20));const u8 *active=regular.bitmap;
    CHECK(nyx_font_available_at_size("Lecture.nyxf",36));CHECK(regular.bitmap==active&&regular.font.h_px==30);CHECK(!mounted);cleanup();
    reset();CHECK(!nyx_font_available_at_size("Lecture.nyxf",35));CHECK(!nyx_font_available_at_size("Lecture.nyxf",38));
    reset();u8 *bad_entry=entry_for(36);blob[read32(bad_entry+12)]^=1;CHECK(!nyx_font_available_at_size("Lecture.nyxf",36));cleanup();
    reset();bad_entry=entry_for(36);put32(bad_entry,39);rehash_table();CHECK(!nyx_font_available_at_size("Lecture.nyxf",36));cleanup();
    free(pristine);free(blob);printf("PASS: %u actual-C font loader assertions; all sizes, corrupt/truncated files, I/O and allocation failures, 8bpp scaling and fallback\n",checks);return 0;
}
'''

test = directory / "test.c"
test.write_text(PREAMBLE + "\n" + source + "\n" + HARNESS, encoding="utf-8")
executable = directory / "test.exe"
compiler = WORK / "toolchains/w64devkit/bin/gcc.exe"
subprocess.run([str(compiler), "-std=gnu11", "-Wall", "-Wextra", "-I" + str(PROJECT / "bdk"),
                str(test), "-o", str(executable)], check=True)
font = PROJECT / "nyx/nyx_gui/fonts/sd/Lecture.nyxf"
subprocess.run([str(executable), str(font)], check=True)

# The converter's output is byte-defined and can be independently checked.
for path in sorted(font.parent.glob("*.nyxf")):
    data = path.read_bytes()
    assert len(data) == struct.unpack_from("<I", data, 8)[0]
    assert data[:4] == b"NYXF"
    assert struct.unpack_from("<I", data, 12)[0] == 12
    print(path.name, len(data), "bytes; 12 sizes")
