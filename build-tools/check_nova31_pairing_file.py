"""Public binary format against an independent Python/zlib fixture."""
from pathlib import Path
import subprocess,json,hashlib,zlib,struct
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v31/nyx/nyx_gui';d=w/'nova31-tests';d.mkdir(exist_ok=True)
raw=bytearray(b'NJOYPAIR'+bytes([1,3,0,0]))
for i in range(2):raw+=bytes([0x40,0,0,0,0,i+1,0x60,0,0,0,0,3])+bytes(range(16+i*32,0+i*32,-1))
assert len(raw)==68
def seal(b):return bytes(b)+struct.pack('<I',zlib.crc32(b))
gold=seal(raw);bad=[]
for at,val in [(8,2),(9,0),(9,7),(10,1),(11,1),(9,1),(12,0x41)]:
 b=raw.copy();b[at]=val;bad.append(seal(b))
for at,n in [(24,16),(18,6),(12,6)]:
 b=raw.copy();b[at:at+n]=bytes(n);bad.append(seal(b))
b=raw.copy();b[46]^=1;bad.append(seal(b)) # mismatched hosts
b=raw.copy();b[40:46]=b[12:18];bad.append(seal(b)) # duplicate peer address
arr=lambda b:'{'+','.join(map(str,b))+'}'
c=r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nova_bt.h"
#define assert(x) do{if(!(x)){fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);exit(1);}}while(0)
'''+f'static const uint8_t golden[72]={arr(gold)};\nstatic const uint8_t bad[][72]={{'+','.join(arr(b)for b in bad)+r'''};
int main(void){
 nova_bt_peer p[2],q[2];uint8_t out[72],damaged[72];assert(nova_bt_pairing_decode(p,golden,72)==2);
 assert(p[0].key[0]==16&&p[0].key[15]==1&&p[1].key[0]==48&&p[1].key[15]==33);
 assert(nova_bt_pairing_encode(out,p)==2&&!memcmp(out,golden,72));
 for(unsigned n=0;n<72;n++)assert(!nova_bt_pairing_decode(q,golden,n));
 assert(!nova_bt_pairing_decode(q,golden,73));
 for(unsigned n=0;n<72;n++)for(unsigned bit=0;bit<8;bit++){
  memcpy(damaged,golden,72);damaged[n]^=1u<<bit;memset(q,0xa5,sizeof q);assert(!nova_bt_pairing_decode(q,damaged,72));
  nova_bt_peer zero[2]={0};assert(!memcmp(q,zero,sizeof q));
 }
 for(unsigned n=0;n<sizeof bad/sizeof bad[0];n++)assert(!nova_bt_pairing_decode(q,bad[n],72));
 memset(&p[1],0,sizeof p[1]);assert(nova_bt_pairing_encode(out,p)==1&&nova_bt_pairing_decode(q,out,72)==1&&!q[1].valid);
 memset(p,0,sizeof p);memset(out,0xa5,72);assert(!nova_bt_pairing_encode(out,p));for(unsigned n=0;n<72;n++)assert(!out[n]);
 puts("PASS: 72-byte private pairing format, independent zlib CRC fixture, exact key order, 576 bit corruptions, short/long files, bad formats/keys/hosts and one-controller records");
}
'''
f=d/'pairing-file.c';f.write_text(c);cc=w/'toolchains/w64devkit/bin/gcc.exe'
r=subprocess.run([str(cc),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-I'+str(p/'bluetooth'),str(f),str(p/'bluetooth/nova_bt_protocol.c'),'-o',str(f.with_suffix('.exe'))],capture_output=True,text=True);assert not r.returncode,r.stderr
r=subprocess.run([str(f.with_suffix('.exe'))],capture_output=True,text=True,timeout=10);assert not r.returncode,r.stderr;print(r.stdout)
(w/'nova31-pairing-file-check.json').write_text(json.dumps({'passed':True,'corruption_cases':576,'independent_zlib_crc':True,'synthetic_keys_only':True,'source_sha256':{n:hashlib.sha256((p/'bluetooth'/n).read_bytes()).hexdigest()for n in ['nova_bt_protocol.c','nova_bt.h']},'logs':r.stdout},indent=2))
