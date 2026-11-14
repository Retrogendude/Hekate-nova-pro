"""Decode both compressed halves FROM the final injector payload, with actual LZ C."""
from pathlib import Path
import re,subprocess,json,hashlib
work=Path(__file__).resolve().parent;project=work/'hekate-6.5.4';directory=work/'packed-hekate-check';directory.mkdir(exist_ok=True)
arm=Path('C:/devkitPro/devkitARM/bin');symbols=subprocess.check_output([str(arm/'arm-none-eabi-nm.exe'),'-S',str(project/'build/loader/loader.elf')],text=True)
payload=(project/'output/hekate.bin').read_bytes()
for n in (0,1):
    match=re.search(r'^(\w+) (\w+) [rRdD] payload_0'+str(n)+r'$',symbols,re.M);assert match
    offset=int(match[1],16)-0x40007000;size=int(match[2],16);assert 0<=offset<offset+size<=len(payload)-6
    (directory/f'part{n}.bin').write_bytes(payload[offset:offset+size])
code=r'''
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lz.h"
int main(void){unsigned char*out=malloc(140288);unsigned int used=0;for(int n=0;n<2;n++){char path[32];sprintf(path,"part%d.bin",n);FILE*f=fopen(path,"rb");assert(f);fseek(f,0,SEEK_END);unsigned int size=ftell(f);rewind(f);unsigned char*data=malloc(size);assert(data&&fread(data,1,size,f)==size);fclose(f);used+=LZ_Uncompress(data,out+used,size);assert(used<=140288);free(data);}FILE*f=fopen("unpacked.bin","wb");assert(f&&fwrite(out,1,used,f)==used);fclose(f);free(out);return 0;}
'''
program=directory/'check.c';program.write_text(code);cc=work/'toolchains/w64devkit/bin/gcc.exe';exe=directory/'check.exe';lz=project/'bdk/libs/compr'
subprocess.run([str(cc),'-std=c11','-O2','-I'+str(lz),str(program),str(lz/'lz.c'),'-o',str(exe)],check=True)
subprocess.run([str(exe)],cwd=directory,check=True)
unpacked=(directory/'unpacked.bin').read_bytes();raw=(project/'output/hekate_unc.bin').read_bytes();assert unpacked==raw,'The final loader contains a stale Hekate image.'
result={'final_payload_bytes':len(payload),'decoded_bytes':len(raw),'decoded_sha256':hashlib.sha256(raw).hexdigest(),'matches_current_uncompressed_binary':True}
(work/'packed-hekate-check.json').write_text(json.dumps(result,indent=2))
print(f'PASS: actual-C decompression of the final injector payload matches all {len(raw)} bytes of the newly compiled Hekate, including the Nova Pro boot artwork.')
