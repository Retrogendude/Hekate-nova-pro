"""Exercise HCI authentication + real L2CAP/HID negotiation over virtual H4.
No fabricated HID-open event: all signals go through BTstack packet handling.
The virtual peer intentionally ignores SDP, but implements standard HID channels.
"""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v23/nyx/nyx_gui';d=w/'nova23-tests';d.mkdir(exist_ok=True)
c=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nova_bt.h"
#include "bluetooth.h"
static unsigned now,reads_n,reads_at,writes_n,auth_count,psm_count[32],mode_count;
static uint8_t reads[16384],writes[16384];static nova_bt_peer pair[2];
static int ignore_control,ignore_interrupt;static unsigned disconnect_requests;
static struct {unsigned host,peer,psm,side,configured;} channels[4];static unsigned nc;
uint32_t nova_bt_now(void){return now;}
int nova_bt_radio_open(void){return 1;}void nova_bt_radio_close(void){}
int nova_bt_radio_read(void){return reads_at<reads_n?reads[reads_at++]:-1;}
int nova_bt_radio_write(uint8_t v){assert(writes_n<sizeof writes);writes[writes_n++]=v;return 1;}
static unsigned le(const uint8_t*p){return p[0]|(p[1]<<8);}
static void put(uint8_t*p,unsigned v){p[0]=v;p[1]=v>>8;}
static void queue(const uint8_t*p,unsigned n){
 if(reads_at==reads_n)reads_at=reads_n=0;
 if(reads_n+n>sizeof reads){memmove(reads,reads+reads_at,reads_n-reads_at);reads_n-=reads_at;reads_at=0;}
 assert(reads_n+n<=sizeof reads);memcpy(reads+reads_n,p,n);reads_n+=n;
}
static void ev(unsigned type,const uint8_t*p,unsigned n){uint8_t b[260]={4,type,n};assert(n<=255);memcpy(b+3,p,n);queue(b,n+3);}
static void cc(unsigned op,const uint8_t*p,unsigned n){uint8_t b[252]={1,op,op>>8,0};assert(n<=248);memcpy(b+4,p,n);ev(0x0e,b,n+4);}
static void cs(unsigned op){uint8_t b[]={0,1,op,op>>8};ev(0x0f,b,4);}
static void acl(unsigned side,unsigned cid,const uint8_t*p,unsigned n){
 uint8_t b[1100]={2};assert(n+9<=sizeof b);put(b+1,0x2040+side);put(b+3,n+4);put(b+5,n);put(b+7,cid);memcpy(b+9,p,n);queue(b,n+9);
}
static void sig(unsigned side,unsigned code,unsigned id,const uint8_t*p,unsigned n){uint8_t b[128]={code,id};assert(n+4<=sizeof b);put(b+2,n);memcpy(b+4,p,n);acl(side,1,b,n+4);}
static void cmd(const uint8_t*p,unsigned n){
 unsigned op=le(p);const uint8_t*a=p+3;uint8_t b[256]={0};unsigned len=0;(void)n;
 switch(op){
 case 0x1001:len=8;b[0]=6;b[3]=6;b[4]=15;break;
 case 0x1002:len=64;memset(b,255,len);break;
 case 0x1003:len=8;b[0]=b[1]=b[2]=255;b[3]=0xfe;break;
 case 0x1004:len=10;b[0]=a[0];break;
 case 0x1005:len=7;b[1]=4;b[3]=8;break;
 case 0x1009:len=6;for(unsigned j=0;j<6;j++)b[j]=pair[0].host[5-j];break;
 case 0x0c14:len=248;break;
 case 0x0405:{
   unsigned side=a[0]-1;assert(side<2);cs(op);b[0]=0;put(b+1,0x40+side);memcpy(b+3,a,6);b[9]=1;b[10]=0;ev(3,b,11);return;}
 case 0x041b:cs(op);put(b+1,le(a));b[3]=b[4]=b[5]=255;b[6]=0xfe;ev(0x0b,b,11);return;
 case 0x041c:cs(op);put(b+1,le(a));b[3]=a[2];b[4]=0;ev(0x23,b,13);return;
 case 0x0411:{unsigned side=le(a)-0x40;assert(side<2);cs(op);for(unsigned j=0;j<6;j++)b[j]=pair[side].mac[5-j];ev(0x17,b,6);return;}
 case 0x040b:{
   unsigned side=a[0]-1;assert(side<2);assert(!memcmp(a+6,pair[side].key,16));auth_count++;
   cc(op,a,6);b[0]=0;put(b+1,0x40+side);ev(6,b,3);return;}
 case 0x1408:len=3;put(b,le(a));b[2]=16;break;
 case 0x0413:cs(op);put(b+1,le(a));b[3]=a[2];ev(8,b,4);return;
 case 0x040f:cs(op);put(b+1,le(a));put(b+3,le(a+2));ev(0x1d,b,5);return;
 case 0x041d:cs(op);put(b+1,le(a));b[3]=6;b[4]=15;ev(0x0c,b,8);return;
 case 0x080b:cs(op);memcpy(b+1,a,6);b[7]=0;ev(0x12,b,8);return;
 case 0x0419:cs(op);memcpy(b+1,a,6);memcpy(b+7,"Joy-Con",8);ev(7,b,255);return;
 case 0x0406:cs(op);put(b+1,le(a));b[3]=a[2];ev(5,b,4);return;
 default:break;
 }
 cc(op,b,len);
}
static int by_peer(unsigned cid){for(unsigned i=0;i<nc;i++)if(channels[i].peer==cid)return i;return -1;}
static void input(unsigned index){
 unsigned side=channels[index].side;uint8_t b[50]={0xa1,0x21,0,0x80};
 b[14]=0x80;b[15]=3;b[16]=0x30;acl(side,channels[index].host,b,17);
 memset(b,0,sizeof b);b[0]=0xa1;b[1]=0x30;b[3]=0x80;
 unsigned x=side?2000:2100,y=side?2001:1900,at=side?10:7;
 b[at]=x;b[at+1]=((x>>8)&15)|((y&15)<<4);b[at+2]=y>>4;
 if(side)b[4]=8;else b[6]=0x80;
 acl(side,channels[index].host,b,50);
}
static void data(const uint8_t*p,unsigned n){
 unsigned handle=le(p)&0xfff,side=handle-0x40,cid=le(p+6);assert(side<2&&n>=8);
 uint8_t completed[]={1,handle,handle>>8,1,0};ev(0x13,completed,5);
 const uint8_t*q=p+8;unsigned size=n-8;
 if(cid==1){
   assert(size>=4);unsigned code=q[0],id=q[1],len=le(q+2);assert(len+4<=size);q+=4;
   if(code==2){
     unsigned psm=le(q),host=le(q+2);assert(psm<32);psm_count[psm]++;
     if(psm==1||(ignore_control&&psm==0x11)||(ignore_interrupt&&psm==0x13))return;
     assert(psm==0x11||psm==0x13);assert(nc<4);
     if(psm==0x13){int have=0;for(unsigned i=0;i<nc;i++)if(channels[i].side==side&&channels[i].psm==0x11&&channels[i].configured)have=1;assert(have);}
     unsigned k=nc++;channels[k].side=side;channels[k].psm=psm;channels[k].host=host;channels[k].peer=0x80+k;
     uint8_t rsp[8]={0};put(rsp,channels[k].peer);put(rsp+2,host);sig(side,3,id,rsp,8);
     uint8_t cfg[8]={0};put(cfg,host);cfg[4]=1;cfg[5]=2;put(cfg+6,672);sig(side,4,0x90+k,cfg,8);return;
   }
   if(code==4){int k=by_peer(le(q));assert(k>=0);uint8_t rsp[6]={0};put(rsp,channels[k].host);sig(side,5,id,rsp,6);channels[k].configured|=1;return;}
   if(code==5){int k=by_peer(le(q));assert(k>=0&&le(q+4)==0);channels[k].configured|=2;return;}
   if(code==10){uint8_t rsp[12]={0};put(rsp,le(q));unsigned l=le(q)==3?12:8;if(le(q)==3)rsp[4]=2;sig(side,11,id,rsp,l);return;}
   if(code==8){sig(side,9,id,q,len);return;}
   if(code==6){disconnect_requests++;sig(side,7,id,q,len);return;}
   fprintf(stderr,"unexpected L2CAP signal %u\n",code);assert(0);
 }else{
   int k=by_peer(cid);assert(k>=0&&channels[k].psm==0x13);
   assert(size==50&&q[0]==0xa2&&q[1]==1&&q[11]==3&&q[12]==0x30);
   for(unsigned j=13;j<size;j++)assert(!q[j]);
   mode_count++;input(k);
 }
}
static void consume(void){
 while(writes_n){
   unsigned type=writes[0],n;if(type==1){if(writes_n<4)return;n=4+writes[3];}
   else{assert(type==2);if(writes_n<5)return;n=5+le(writes+3);}
   if(writes_n<n)return;
   if(type==1)cmd(writes+1,n-1);else data(writes+1,n-1);
   memmove(writes,writes+n,writes_n-n);writes_n-=n;
 }
}
int main(int argc,char**argv){
 ignore_control=argc>1&&!strcmp(argv[1],"control-timeout");
 ignore_interrupt=argc>1&&!strcmp(argv[1],"interrupt-timeout");
 for(unsigned i=0;i<2;i++){pair[i].valid=1;pair[i].mac[0]=0x40;pair[i].mac[5]=i+1;pair[i].host[0]=0x60;pair[i].host[5]=3;for(unsigned j=0;j<16;j++)pair[i].key[j]=0x10+j+i*16;}
 assert(nova_bt_start(pair));
 for(unsigned step=0;step<35000;step++){
   nova_bt_poll();consume();now++;
   const nova_bt_status*s=nova_bt_get_status();
   if(s->pad[0].reports&&s->pad[1].reports)break;
   if(s->pad[0].last_error==0x69)break;
 }
 const nova_bt_status*s=nova_bt_get_status();
#ifdef EXPECT_SDP_TIMEOUT
 assert(psm_count[1]&&!s->pad[0].reports&&s->pad[0].last_error==0x69);
 puts("PASS: descriptor-query path reproduces 0x69 when the SDP channel does not answer");
#else
 if(ignore_interrupt){
  assert(!psm_count[1]&&psm_count[19]&&s->pad[0].last_error==0x69&&s->pad[0].last_psm==19);
  for(unsigned j=0;j<12;j++){nova_bt_poll();consume();now++;}
  assert(disconnect_requests);puts("PASS: interrupt timeout closes the established control channel before retry");
 }
 else if(ignore_control){assert(!psm_count[1]&&psm_count[17]&&s->pad[0].last_error==0x69&&s->pad[0].last_psm==17&&!s->pad[0].reports);puts("PASS: HID control timeout is recorded as 0x69 with channel 0x0011");}
 else{
  if(!(s->pad[0].reports&&s->pad[1].reports))fprintf(stderr,"state%d err%d L:%u/%u psm%x stage%u R:%u/%u psm%x stage%u auth%u ctrl%u int%u mode%u now%u\n",s->state,s->error,s->pad[0].reports,s->pad[0].last_error,s->pad[0].last_psm,s->pad[0].stage,s->pad[1].reports,s->pad[1].last_error,s->pad[1].last_psm,s->pad[1].stage,auth_count,psm_count[17],psm_count[19],mode_count,now);
  assert(!psm_count[1]&&psm_count[17]==2&&psm_count[19]==2&&mode_count==2&&auth_count==2);
  assert(s->pad[0].reports&&s->pad[1].reports&&s->pad[0].hid_open&&s->pad[1].hid_open);
  assert(s->pad[0].buttons==(1u<<23)&&s->pad[1].buttons==8);
  assert(s->pad[0].x==2100&&s->pad[1].y==2001&&s->pad[0].mode_ack&&s->pad[1].mode_ack);
  uint8_t disconnect[]={0,0x40,0,0x13};ev(5,disconnect,4);for(unsigned j=0;j<4;j++){nova_bt_poll();consume();now++;}
  assert(!s->pad[0].connected&&!s->pad[0].buttons&&s->pad[1].connected);
  puts("PASS: two real HCI authentications, control/interrupt configuration, padded report command, ACK, buttons/sticks and disconnect through the full stack");
 }
#endif
 assert(nova_bt_trace_count()>0&&nova_bt_trace_count()<=128);nova_bt_stop();return 0;
}
'''
(d/'negotiation.c').write_text(c)
lib=p/'libs/btstack';cc=w/'toolchains/w64devkit/bin/gcc.exe'
sources=[str(f)for f in lib.rglob('*.c')]+[str(p/'bluetooth'/f)for f in ('nova_bt_stack.c','nova_bt_protocol.c','nova_bt_trace.c')]
logs=[]
for baseline in (False,True):
    cfg=d/('sdp-config'if baseline else 'known-config');cfg.mkdir(exist_ok=True)
    config=(p/'bluetooth/btstack_config.h').read_text()
    if baseline:config=config.replace('#define NOVA_HID_KNOWN_PSMS','/* Generic upstream descriptor-query behavior. */')
    (cfg/'btstack_config.h').write_text(config)
    exe=d/('negotiation-sdp.exe'if baseline else 'negotiation.exe')
    args=[str(cc),'-std=gnu11','-O1','-g','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
        '-I'+str(cfg),'-I'+str(p/'bluetooth'),'-I'+str(lib)]
    if baseline:args+=['-DEXPECT_SDP_TIMEOUT']
    r=subprocess.run([*args,str(d/'negotiation.c'),*sources,'-o',str(exe)],capture_output=True,text=True)
    assert r.returncode==0,r.stderr
    for mode in ([]if baseline else ['control-timeout','interrupt-timeout'])+['']:
        try:r=subprocess.run([str(exe),*([mode]if mode else [])],capture_output=True,text=True,timeout=10)
        except subprocess.TimeoutExpired as e:
            print((e.stderr or b'')[-6000:]);raise
        print(r.stdout,r.stderr);assert r.returncode==0,r.returncode;logs.append(r.stdout.strip())
(w/'nova23-negotiation-check.json').write_text(json.dumps({'passed':True,'logs':logs,'physical_radio':False,
    'source_sha256':{f.relative_to(p).as_posix():hashlib.sha256(f.read_bytes()).hexdigest()for f in (p/'bluetooth').glob('*.[ch]')},
    'hid_host_sha256':hashlib.sha256((lib/'classic/hid_host.c').read_bytes()).hexdigest()},indent=2))
