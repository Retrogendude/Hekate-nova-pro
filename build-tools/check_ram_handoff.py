"""Host-C tests of the actual isolated RAM service's handoff guards/results."""
from pathlib import Path
import re
import subprocess

work = Path(__file__).resolve().parent
source = work / 'HWTest-RCM-0.15/ramtest/source'
text = (source / 'ramtest_main.c').read_text(encoding='utf-8')
contract = (source / 'nyx_ram_handoff.h').read_text(encoding='utf-8')
contract = contract.replace('#include <utils/types.h>', '')

def function(name):
    start = re.search(r'^static (?:bool|void) ' + name + r'\(', text, re.M).start()
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

header_address = int(re.search(r'NYX_RAM_HANDOFF_ADDR (0x\w+)u', contract).group(1), 16)
payload_address = int(re.search(r'NYX_RAM_RETURN_ADDR\s+(0x\w+)u', contract).group(1), 16)
payload_max = int(re.search(r'NYX_RAM_RETURN_MAX\s+(0x\w+)u', contract).group(1), 16)
assert 0x80000000 <= header_address < payload_address
assert payload_address + payload_max <= 0x80400000
assert header_address + 60 <= payload_address
assert 'overlaps(phys, RAM_WINDOW_BYTES, DRAM_START, SZ_4M)' in text

harness = r'''
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef uint8_t u8;
'''+contract+r'''
static nyx_ram_handoff_t record;
static u8 image[NYX_RAM_RETURN_MAX];
#undef NYX_RAM_HANDOFF_ADDR
#define NYX_RAM_HANDOFF_ADDR ((uintptr_t)&record)
#undef NYX_RAM_RETURN_ADDR
#define NYX_RAM_RETURN_ADDR ((uintptr_t)image)
static bool g_abort_user, g_run_failed;
static u32 g_errors, g_passes_completed, g_pass_total, g_regions_testable, g_ram_mb;
static u32 mtc_error;
static u32 ram_mtc_errors(void) { return mtc_error; }
#define BPMP_MMU_MAINT_CLEAN_WAY 1
static void bpmp_mmu_maintenance(u32 unused, bool unused2) { (void)unused; (void)unused2; }
'''+function('nyx_return_valid')+'\n'+function('nyx_record_result')+r'''
static int checks;
#define CHECK(v) do { ++checks; if (!(v)) { fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#v);return 1;} } while(0)
static void valid_image(void) {
 memset(&record,0,sizeof record);
 for (u32 i=0;i<sizeof image;i++) image[i]=(u8)(i*31u+17u);
 record.magic=NYX_RAM_HANDOFF_MAGIC;record.version=NYX_RAM_VERSION;
 record.auto_run=1;record.payload_size=4096;record.payload_hash=nyx_ram_hash(image,record.payload_size);
}
static void clean_stats(void) {
 g_abort_user=g_run_failed=false;g_errors=0;mtc_error=0;
 g_passes_completed=g_pass_total=7;g_regions_testable=1018;g_ram_mb=4096;
}
int main(void) {
 valid_image();CHECK(nyx_return_valid());
 record.magic^=1;CHECK(!nyx_return_valid());valid_image();
 record.version=0;CHECK(!nyx_return_valid());valid_image();
 record.auto_run=0;CHECK(!nyx_return_valid());valid_image();
 record.payload_size=0;CHECK(!nyx_return_valid());valid_image();
 record.payload_size=255;CHECK(!nyx_return_valid());valid_image();
 record.payload_size=256;record.payload_hash=nyx_ram_hash(image,256);CHECK(nyx_return_valid());valid_image();
 record.payload_size=NYX_RAM_RETURN_MAX;record.payload_hash=nyx_ram_hash(image,sizeof image);CHECK(nyx_return_valid());valid_image();
 record.payload_size=NYX_RAM_RETURN_MAX+1;CHECK(!nyx_return_valid());valid_image();
 record.payload_size=UINT32_MAX;CHECK(!nyx_return_valid());valid_image();
 image[13]^=0x40;CHECK(!nyx_return_valid());valid_image();
 record.payload_hash^=1;CHECK(!nyx_return_valid());valid_image();
 clean_stats();nyx_record_result(123456,false);
 CHECK(record.result_magic==NYX_RAM_RESULT_MAGIC);CHECK(record.result==NYX_RAM_RESULT_PASS);
 CHECK(record.elapsed_ms==123456);CHECK(record.tested_mib==4072);CHECK(record.dram_mib==4096);
 CHECK(record.patterns_completed==7&&record.patterns_total==7);
 CHECK(record.result_hash==nyx_ram_hash(&record.result,8u*sizeof(u32)));
 CHECK(nyx_return_valid());
 record.errors++;CHECK(record.result_hash!=nyx_ram_hash(&record.result,8u*sizeof(u32)));
 clean_stats();g_errors=1;nyx_record_result(4,false);CHECK(record.result==NYX_RAM_RESULT_FAIL);
 clean_stats();g_run_failed=true;nyx_record_result(4,false);CHECK(record.result==NYX_RAM_RESULT_FAIL);
 clean_stats();g_passes_completed=6;nyx_record_result(4,false);CHECK(record.result==NYX_RAM_RESULT_FAIL);
 clean_stats();g_abort_user=true;nyx_record_result(4,false);CHECK(record.result==NYX_RAM_RESULT_ABORT);
 clean_stats();nyx_record_result(4,true);CHECK(record.result==NYX_RAM_RESULT_ERROR);
 clean_stats();g_pass_total=g_passes_completed=0;nyx_record_result(4,false);CHECK(record.result==NYX_RAM_RESULT_ERROR);
 clean_stats();mtc_error=0x123;nyx_record_result(4,true);CHECK(record.mtc_errors==0x123);
 printf("PASS: %d actual-C handoff/result checks; protected address bounds checked.\n",checks);
 return 0;
}
'''
c = work / 'check_ram_handoff_generated.c'
exe = work / 'check_ram_handoff_generated.exe'
c.write_text(harness, encoding='utf-8')
subprocess.run([str(work/'toolchains/w64devkit/bin/gcc.exe'), '-std=c11', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(exe)], check=True)
subprocess.run([str(exe)], check=True)
