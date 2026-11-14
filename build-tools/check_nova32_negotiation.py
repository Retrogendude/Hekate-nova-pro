"""Exercise HCI authentication + real L2CAP/HID negotiation over virtual H4.
No fabricated HID-open event: all signals go through BTstack packet handling.
The virtual peer intentionally ignores SDP, but implements standard HID channels.
"""
from pathlib import Path
import subprocess,json,hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v32/nyx/nyx_gui';d=w/'nova32-tests';d.mkdir(exist_ok=True)
c=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#undef assert
#define assert(x) do {if(!(x)){fprintf(stderr,"assertion %s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
#include "nova_bt.h"
#include "bluetooth.h"
static unsigned now,reads_n,reads_at,writes_n,auth_count,psm_count[32],mode_count,ssp_commands;
static uint8_t reads[16384],writes[16384];static nova_bt_peer pair[2];
extern void nova_test_import(nova_bt_peer out[2]);
static const uint8_t golden_keys[2][16]={{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},{47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32}};
static int wrong_key,wrong_host,ssp,incoming,encrypt_timeout,stall_first,incoming_collision;
static unsigned accepts,create_count,conn_ready[2],encrypt_due[2];
static int radio_health;
static unsigned periodic_at[2],streaming[8];
static int retry_wake;static unsigned page_slots,page_due[2],create_side[2];
static int cross_control,cross_interrupt,late_success,orphan_timeout,rom_profile;
static unsigned br_set[2],br_commands,qos_commands,default_policy,exit_sniff,link_policy,edr_dropped;

static unsigned refused_cross,duplicates_refused,peak_channels;
static struct {unsigned side,psm,host,id,done;} abandoned[4];static unsigned na;
static unsigned peer_interrupt_requested[2];
extern unsigned nova_test_l2cap_count(void);
static int ignore_control,ignore_interrupt;static unsigned disconnect_requests;
static struct {unsigned host,peer,psm,side,configured;} channels[8];static unsigned nc;
uint32_t nova_bt_now(void){return now;}
uint32_t nova_bt_now_us(void){static uint32_t us;us+=100;return us;}
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
 if(rom_profile&&n+4>27&&!br_set[side]){edr_dropped++;return;}
 uint8_t b[1100]={2};assert(n+9<=sizeof b);put(b+1,0x2040+side);put(b+3,n+4);put(b+5,n);put(b+7,cid);memcpy(b+9,p,n);queue(b,n+9);
}
static void sig(unsigned side,unsigned code,unsigned id,const uint8_t*p,unsigned n){uint8_t b[128]={code,id};assert(n+4<=sizeof b);put(b+2,n);memcpy(b+4,p,n);acl(side,1,b,n+4);}
static void peer_request(unsigned side,unsigned psm){
 unsigned k=nc++;assert(k<8);channels[k].side=side;channels[k].psm=psm;channels[k].peer=0xa0+k;
 uint8_t req[4];put(req,psm);put(req+2,channels[k].peer);sig(side,2,0xc0+k,req,4);
}
static void maybe_interrupt(unsigned k){
 if(channels[k].psm==0x11&&channels[k].configured==3&&!peer_interrupt_requested[channels[k].side]){
  peer_interrupt_requested[channels[k].side]=1;peer_request(channels[k].side,0x13);
 }
}
static void cmd(const uint8_t*p,unsigned n){
 unsigned op=le(p);const uint8_t*a=p+3;uint8_t b[256]={0};unsigned len=0;(void)n;
 switch(op){
 case 0x0c56:ssp_commands++;assert(ssp&&a[0]==1);break;
 case 0x1001:len=8;b[0]=8;b[2]=0x20;b[3]=8;b[4]=15;b[6]=9;b[7]=0x24;break;
 case 0x1002:len=64;memset(b,255,len);break;
 case 0x1003:len=8;b[0]=b[1]=b[2]=255;b[3]=0xfe;if(ssp){b[6]=8;b[7]=0x80;}break;
 case 0x1004:len=10;b[0]=a[0];if(ssp){b[1]=2;if(a[0]==1)b[2]=9;if(a[0]==2)b[3]=1;}break;
 case 0x1005:len=7;b[1]=4;b[3]=8;break;
 case 0x1009:len=6;for(unsigned j=0;j<6;j++)b[j]=pair[0].host[5-j];if(wrong_host)b[0]^=1;break;
 case 0x1405:len=3;put(b,le(a));b[2]=le(a)==0x40?0:(uint8_t)-32;break;
 case 0x1403:len=3;put(b,le(a));b[2]=le(a)==0x40?250:120;break;
 case 0x0c2d:len=3;put(b,le(a));b[2]=a[2]?10:(uint8_t)-6;break;
 case 0x1401:len=4;put(b,le(a));put(b+2,le(a)==0x40?0:3);break;
 case 0x0c18:page_slots=le(a);break;
 case 0x0c14:len=248;break;
 case 0x080f:default_policy=le(a);break;
 case 0x080d:assert(le(a+2)==1);link_policy++;len=2;put(b,le(a));break;
 case 0x0804:exit_sniff++;cs(op);put(b+1,le(a));b[3]=0;ev(0x14,b,6);return;
 case 0x0807:qos_commands++;assert(a[3]==2&&a[4]==0x40&&a[5]==0x1f&&a[12]==0xc4&&a[13]==9);cs(op);memcpy(b+1,a,20);ev(0x0d,b,21);return;
 case 0x0405:{
   unsigned side=a[0]-1;assert(side<2);cs(op);create_count++;
   if(side==1&&!incoming)assert(!stall_first&&(nova_bt_get_status()->pad[0].hid_open||nova_bt_get_status()->pad[0].last_error));
   if(retry_wake&&++create_side[side]<=4){page_due[side]=now+(page_slots*5/8);return;}
   if(stall_first)return;
   if(incoming){memcpy(b,a,6);b[6]=8;b[7]=5;b[9]=1;ev(4,b,10);if(incoming_collision){memset(b,0,sizeof b);b[0]=0x0b;put(b+1,0x40+side);memcpy(b+3,a,6);b[9]=1;ev(3,b,11);}return;}
   conn_ready[side]=1;b[0]=0;put(b+1,0x40+side);memcpy(b+3,a,6);b[9]=1;b[10]=0;ev(3,b,11);return;}
 case 0x0409:{
   unsigned side=a[0]-1;assert(side<2&&incoming);assert(a[6]==0);accepts++;cs(op);
   conn_ready[side]=1;b[0]=0;put(b+1,0x40+side);memcpy(b+3,a,6);b[9]=1;ev(3,b,11);
   memset(b,0,sizeof b);memcpy(b+1,a,6);b[7]=a[6];ev(0x12,b,8);return;}

 case 0x041b:cs(op);put(b+1,le(a));b[3]=b[4]=b[5]=255;b[6]=0xfe;if(ssp){b[9]=8;b[10]=0x80;}ev(0x0b,b,11);return;
 case 0x041c:cs(op);put(b+1,le(a));b[3]=a[2];b[4]=2;if(ssp&&a[2]==1)b[5]=9;if(ssp&&a[2]==2)b[6]=1;ev(0x23,b,13);return;
 case 0x0411:{unsigned side=le(a)-0x40;assert(side<2);cs(op);for(unsigned j=0;j<6;j++)b[j]=pair[side].mac[5-j];ev(0x17,b,6);return;}
 case 0x040b:{
   unsigned side=a[0]-1;assert(side<2);int good=!memcmp(a+6,golden_keys[side],16);assert(good!=wrong_key);auth_count++;
   cc(op,a,6);b[0]=good?0:5;put(b+1,0x40+side);ev(6,b,3);return;}
 case 0x1408:len=3;put(b,le(a));b[2]=16;break;
 case 0x0413:{unsigned side=le(a)-0x40;assert(side<2&&conn_ready[side]&&a[2]==1);cs(op);
   if(encrypt_timeout){encrypt_due[side]=now+5000;return;}
   put(b+1,le(a));b[3]=a[2];ev(8,b,4);return;}
 case 0x040f:{unsigned side=le(a)-0x40;assert(side<2);br_set[side]=(le(a+2)&0x3306)==0x3306;br_commands++;
 cs(op);put(b+1,le(a));put(b+3,le(a+2));ev(0x1d,b,5);return;}
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
     assert(psm==0x11||psm==0x13);assert(nc<8);
     if((cross_control&&psm==0x11)||(cross_interrupt&&psm==0x13)){
      assert(na<4);abandoned[na].side=side;abandoned[na].psm=psm;abandoned[na].host=host;abandoned[na].id=id;na++;
      peer_request(side,psm);
      uint8_t pending[8]={0};put(pending+2,host);put(pending+4,1);sig(side,3,id,pending,8);return;
     }
     if(psm==0x13){int have=0;for(unsigned i=0;i<nc;i++)if(channels[i].side==side&&channels[i].psm==0x11&&channels[i].configured)have=1;assert(have);}
     unsigned k=nc++;channels[k].side=side;channels[k].psm=psm;channels[k].host=host;channels[k].peer=0x80+k;
     uint8_t rsp[8]={0};put(rsp,channels[k].peer);put(rsp+2,host);sig(side,3,id,rsp,8);
     uint8_t cfg[8]={0};put(cfg,host);cfg[4]=1;cfg[5]=2;put(cfg+6,672);sig(side,4,0x90+k,cfg,8);return;
   }
   if(code==3){
    unsigned peer=le(q+2),result=le(q+4);if(peer>=0xf0){assert(result==4);duplicates_refused++;return;}int k=by_peer(peer);assert(k>=0);
    if(result==1)return;
    if(result){assert(result==4);refused_cross++;return;}
    channels[k].host=le(q);assert(channels[k].host>=0x40);
    uint8_t cfg[8]={0};put(cfg,channels[k].host);cfg[4]=1;cfg[5]=2;put(cfg+6,672);sig(side,4,0x90+k,cfg,8);return;
   }
   if(code==4){int k=by_peer(le(q));assert(k>=0);uint8_t rsp[6]={0};put(rsp,channels[k].host);sig(side,5,id,rsp,6);channels[k].configured|=1;if(cross_control)maybe_interrupt(k);return;}
   if(code==5){int k=by_peer(le(q));assert(k>=0&&le(q+4)==0);channels[k].configured|=2;if(cross_control)maybe_interrupt(k);return;}
   if(code==10){uint8_t rsp[12]={0};put(rsp,le(q));unsigned l=le(q)==3?12:8;if(le(q)==3)rsp[4]=2;sig(side,11,id,rsp,l);return;}
   if(code==8){sig(side,9,id,q,len);return;}
   if(code==6){disconnect_requests++;sig(side,7,id,q,len);return;}
   fprintf(stderr,"unexpected L2CAP signal %u\n",code);assert(0);
 }else{
   int k=by_peer(cid);assert(k>=0&&channels[k].psm==0x13);
   assert(size==50&&q[0]==0xa2&&q[1]==1&&q[11]==3&&q[12]==0x30);
   for(unsigned j=13;j<size;j++)assert(!q[j]);
   mode_count++;streaming[k]=1;input(k);
 }
}
static void consume(void){
 if(retry_wake||radio_health)for(unsigned side=0;side<2;side++)if(now-periodic_at[side]>=40&&nova_bt_get_status()->pad[side].hid_open){
  for(unsigned k=nc;k>0;k--)if(channels[k-1].psm==0x13&&channels[k-1].side==side){if(streaming[k-1])input(k-1);break;}
  periodic_at[side]=now;
 }
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
 wrong_key=argc>1&&!strcmp(argv[1],"wrong-key");
 wrong_host=argc>1&&!strcmp(argv[1],"wrong-host");
 ssp=argc>1&&!strcmp(argv[1],"ssp-sc");
 incoming=argc>1&&(!strcmp(argv[1],"incoming")||!strcmp(argv[1],"incoming-collision")||!strcmp(argv[1],"encryption-timeout"));
 encrypt_timeout=argc>1&&!strcmp(argv[1],"encryption-timeout");
 radio_health=argc>1&&!strcmp(argv[1],"radio-health");
 retry_wake=argc>1&&!strcmp(argv[1],"retry-wake");
 stall_first=argc>1&&!strcmp(argv[1],"stalled-first");
 rom_profile=argc>1&&!strcmp(argv[1],"hwtest-rom");
 cross_control=argc>1&&(!strcmp(argv[1],"hid-cross-control")||!strcmp(argv[1],"hid-late-success")||!strcmp(argv[1],"hid-orphan-timeout"));
 cross_interrupt=argc>1&&!strcmp(argv[1],"hid-cross-interrupt");
 late_success=argc>1&&!strcmp(argv[1],"hid-late-success");
 orphan_timeout=argc>1&&!strcmp(argv[1],"hid-orphan-timeout");
 incoming_collision=argc>1&&!strcmp(argv[1],"incoming-collision");if(incoming)ssp=1;
 if(cross_control||cross_interrupt||rom_profile){incoming=1;ssp=1;}
 nova_test_import(pair);
 if(wrong_key)for(unsigned i=0;i<2;i++)for(unsigned j=0;j<8;j++){uint8_t t=pair[i].key[j];pair[i].key[j]=pair[i].key[15-j];pair[i].key[15-j]=t;}

 assert(nova_bt_start(pair));
 for(unsigned step=0;step<(retry_wake?85000u:35000u);step++){
   nova_bt_poll();consume();now++;
   if(stall_first&&now>=1000)break;
   for(unsigned i=0;i<2;i++)if(page_due[i]&&now>=page_due[i]){
    uint8_t failure[11]={4};put(failure+1,0x40+i);for(unsigned j=0;j<6;j++)failure[3+j]=pair[i].mac[5-j];failure[9]=1;ev(3,failure,11);page_due[i]=0;
   }
   for(unsigned i=0;i<2;i++)if(encrypt_due[i]&&now>=encrypt_due[i]){
    uint8_t enc[]={8,0x40+i,0,0};ev(8,enc,4);
    uint8_t dis[]={0,0x40+i,0,8};ev(5,dis,4);encrypt_due[i]=0;
   }
   unsigned count=nova_test_l2cap_count();if(count>peak_channels)peak_channels=count;assert(count<=8);
   const nova_bt_status*s=nova_bt_get_status();
   if(refused_cross)break;
   if(s->pad[0].report_id==0x30&&s->pad[1].report_id==0x30&&(!radio_health||(s->pad[0].rf.samples[4]&&s->pad[1].rf.samples[4])))break;
   if(s->pad[0].last_error==0x69 || s->pad[0].auth_error==5 || s->pad[0].encryption_error==8 || s->state==NOVA_BT_ERROR)break;
 }
 if(retry_wake||radio_health){for(unsigned j=0;j<nc;j++)if(channels[j].psm==0x13)input(j);for(unsigned j=0;j<30;j++){nova_bt_poll();consume();now++;}}
 const nova_bt_status*s=nova_bt_get_status();
#ifdef EXPECT_SDP_TIMEOUT
 assert(psm_count[1]&&!s->pad[0].reports&&s->pad[0].last_error==0x69);
 puts("PASS: descriptor-query path reproduces 0x69 when the SDP channel does not answer");
#else
 if(stall_first){assert(create_count==1&&!accepts&&!auth_count);puts("PASS: a pending first connection prevents a concurrent second HCI Create Connection");}
 else if(encrypt_timeout){
  for(unsigned j=0;j<12;j++){nova_bt_poll();consume();now++;}
  assert(s->pad[0].auth_seen&&s->pad[0].auth_error==0&&s->pad[0].encryption_seen&&s->pad[0].encryption_error==8);
  assert(!s->rx_acl&&!s->tx_acl&&!s->pad[0].connected&&!s->pad[0].encrypted);
  assert(s->pad[0].disconnect_reason==8&&s->pad[0].role==0);
  nova_bt_stop();nova_bt_stop();
  assert(s->state==NOVA_BT_OFF&&s->pad[0].encryption_error==8&&s->pad[0].encryption_seen&&s->pad[0].auth_seen&&s->pad[0].disconnect_reason==8);
  puts("PASS: incoming auth succeeds then encryption 08/disconnect 08; evidence survives repeated Stop; no inputs leak");
 }
 else if(wrong_host){assert(s->state==NOVA_BT_ERROR&&s->error==9&&s->host_identity==2&&!auth_count&&!s->tx_acl);puts("PASS: controller identity mismatch stops before authentication; no addresses logged");}
 else if(wrong_key){assert(s->pad[0].auth_error==5&&!s->pad[0].reports&&!s->rx_acl&&!s->tx_acl);puts("PASS: reversed key reproduces hardware authentication 05 and ACL 0/0");}
 else if(ignore_interrupt){
  assert(!psm_count[1]&&psm_count[19]&&s->pad[0].last_error==0x69&&s->pad[0].last_psm==19);
  for(unsigned j=0;j<12;j++){nova_bt_poll();consume();now++;}
  assert(disconnect_requests);puts("PASS: interrupt timeout closes the established control channel before retry");
 }
 else if(ignore_control){assert(!psm_count[1]&&psm_count[17]&&s->pad[0].last_error==0x69&&s->pad[0].last_psm==17&&!s->pad[0].reports);puts("PASS: HID control timeout is recorded as 0x69 with channel 0x0011");}
 else{
  if(retry_wake){
   assert(create_side[0]>=5&&create_side[1]>=5);
#ifndef EXPECT_OLD_PAGE_TIMEOUT
   assert(page_slots==0x2000);
#endif
  }
  if(radio_health){
   for(unsigned i=0;i<2;i++)for(unsigned k=0;k<5;k++)assert(s->pad[i].rf.samples[k]&&!s->pad[i].rf.error[k]);
   assert(s->pad[0].rf.value[0]==0&&s->pad[1].rf.value[0]==-32&&s->pad[1].rf.value[1]==120);
   assert(s->pad[0].rf.value[2]==-6&&s->pad[0].rf.value[3]==10&&s->pad[1].rf.value[4]==3);
   assert(s->rf_queries>=10&&!s->rf_bad_replies);puts("PASS: periodic RSSI, quality, current/max power and consecutive failed contacts through real HCI, with both HID inputs preserved");
  }
  assert(!refused_cross);
  if(cross_control||cross_interrupt){
   assert(na==2&&nc==4&&s->pad[0].reports&&s->pad[1].reports);
   assert(nova_test_l2cap_count()==6);
   /* Reject duplicate channels after establishment, preserving the active
    * connection and freeing temporary L2CAP allocations. */
   for(unsigned j=0;j<2;j++){uint8_t dup[4];put(dup,j?0x13:0x11);put(dup+2,0xf0+j);sig(0,2,0xf0+j,dup,4);}
   for(unsigned j=0;j<20;j++){nova_bt_poll();consume();now++;}
   assert(duplicates_refused==2&&nova_test_l2cap_count()==6);
   /* Respond to obsolete outgoing opens only AFTER both incoming HID links
    * carry inputs. Their old CID must never be mistaken for the active one. */
   for(unsigned i=0;i<na;i++){
    uint8_t rsp[8]={0};put(rsp+2,abandoned[i].host);
    if(late_success){
     unsigned k=nc++;channels[k].side=abandoned[i].side;channels[k].host=abandoned[i].host;channels[k].peer=0xe0+i;channels[k].psm=abandoned[i].psm;
     put(rsp,channels[k].peer);sig(abandoned[i].side,3,abandoned[i].id,rsp,8);
     uint8_t cfg[4]={0};put(cfg,channels[k].host);sig(abandoned[i].side,4,0xb0+i,cfg,4);
    }else if(!orphan_timeout){put(rsp+4,4);sig(abandoned[i].side,3,abandoned[i].id,rsp,8);}
   }
   if(orphan_timeout){now+=130000;for(unsigned k=0;k<nc;k++)if(channels[k].psm==0x13)input(k);}
   for(unsigned j=0;j<500;j++){nova_bt_poll();consume();now++;}
   assert(nova_test_l2cap_count()==4&&s->pad[0].hid_open&&s->pad[1].hid_open);
   assert(!s->pad[0].last_error&&!s->pad[1].last_error);
   for(unsigned i=0;i<4;i++)if(channels[i].psm==0x13)input(i);
   for(unsigned j=0;j<10;j++){nova_bt_poll();consume();now++;}
   assert(s->pad[0].connected&&s->pad[1].connected);
   printf("PASS: crossed HID %s accepted; late %s cannot close active sessions; pools return from six to four channels (peak %u)\n",cross_control?"control":"interrupt",late_success?"success":orphan_timeout?"timeout":"failure",peak_channels);
  }
  if(rom_profile){
   assert(s->pad[0].report_id==0x30&&s->pad[1].report_id==0x30&&br_commands==2&&qos_commands==2&&default_policy==5&&!edr_dropped);
   assert(s->pad[0].br_only&&s->pad[1].br_only);
   for(unsigned i=0;i<2;i++){uint8_t mode[]={0,0x40+i,0,2,24,0};ev(0x14,mode,6);}
   for(unsigned j=0;j<80;j++){nova_bt_poll();consume();now++;}
   assert(exit_sniff==2&&link_policy==2&&s->pad[0].mode_seen&&!s->pad[0].radio_mode&&!s->pad[1].radio_mode);
   puts("PASS: HWTest ROM profile forces BR on both links, enables sniff/role policy, sets QoS and returns sniff links to active; full reports survive the EDR-drop model");
  }
  if(!(s->pad[0].reports&&s->pad[1].reports))fprintf(stderr,"state%d err%d L:%u/%u psm%x stage%u R:%u/%u psm%x stage%u auth%u ctrl%u int%u mode%u now%u\n",s->state,s->error,s->pad[0].reports,s->pad[0].last_error,s->pad[0].last_psm,s->pad[0].stage,s->pad[1].reports,s->pad[1].last_error,s->pad[1].last_psm,s->pad[1].stage,auth_count,psm_count[17],psm_count[19],mode_count,now);
  assert(accepts==(incoming?2u:0));assert(s->host_identity==1);assert(ssp_commands==(unsigned)ssp);assert(!psm_count[1]&&psm_count[17]==2&&psm_count[19]==(cross_control?0u:2u)&&mode_count==2&&auth_count==2);
  assert(s->pad[0].reports&&s->pad[1].reports&&s->pad[0].hid_open&&s->pad[1].hid_open);
  assert(s->pad[0].buttons==(1u<<23)&&s->pad[1].buttons==8);
  assert(s->pad[0].x==2100&&s->pad[1].y==2001&&s->pad[0].mode_ack&&s->pad[1].mode_ack);
  for(unsigned side=0;side<2;side++)assert(s->pad[side].auth_seen&&s->pad[side].encryption_seen&&!s->pad[side].encryption_error&&s->pad[side].encrypted);
  if(incoming){assert(s->pad[0].role==0&&s->pad[1].role==0);puts("PASS: Joy-Con initiated HCI connections request central; Role Change updates both roles");}
  if(retry_wake){
   unsigned old_reports=s->pad[1].reports;uint8_t timeout[]={0,0x41,0,8};ev(5,timeout,4);
   for(unsigned j=0;j<15000;j++){nova_bt_poll();consume();now++;if(s->pad[1].reports>old_reports&&s->pad[1].hid_open)break;}
   assert(s->pad[1].reports>old_reports&&s->pad[1].hid_open&&s->pad[1].encrypted&&s->pad[1].disconnect_reason==8);
   puts("PASS: both peers wake after four failed pages each, beyond the old three-attempt limit; right input recovers after supervision timeout08");
  }
  uint8_t disconnect[]={0,0x40,0,0x13};ev(5,disconnect,4);for(unsigned j=0;j<4;j++){nova_bt_poll();consume();now++;}
  assert(!s->pad[0].connected&&!s->pad[0].buttons&&s->pad[1].connected);
  puts("PASS: two real HCI authentications, control/interrupt configuration, padded report command, ACK, buttons/sticks and disconnect through the full stack");
 }
#endif
 assert(nova_bt_trace_count()>0&&nova_bt_trace_count()<=128);nova_bt_stop();assert(!nova_test_l2cap_count());return 0;
}
'''
(d/'negotiation.c').write_text(c)
lib=p/'libs/btstack';cc=w/'toolchains/w64devkit/bin/gcc.exe'
(d/'l2cap-probe.c').write_text('#include "'+(lib/'l2cap.c').as_posix()+'"\nunsigned nova_test_l2cap_count(void){unsigned n=0;btstack_linked_list_iterator_t it;btstack_linked_list_iterator_init(&it,&l2cap_channels);while(btstack_linked_list_iterator_has_next(&it)){l2cap_channel_t *c=(l2cap_channel_t*)btstack_linked_list_iterator_next(&it);if(c->channel_type==L2CAP_CHANNEL_TYPE_CLASSIC)n++;}return n;}\n')
sources=[str(d/'pairing.o')]+[str(d/'l2cap-probe.c')if f.name=='l2cap.c'else str(f)for f in lib.rglob('*.c')]+[str(p/'bluetooth'/f)for f in ('nova_bt_stack.c','nova_bt_protocol.c','nova_bt_trace.c')]
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
    for mode in ([]if baseline else ['control-timeout','interrupt-timeout','wrong-key','wrong-host','ssp-sc','incoming','incoming-collision','encryption-timeout','stalled-first','hid-cross-control','hid-cross-interrupt','hid-late-success','hid-orphan-timeout','hwtest-rom','retry-wake','radio-health'])+['']:
        try:r=subprocess.run([str(exe),*([mode]if mode else [])],capture_output=True,text=True,timeout=10)
        except subprocess.TimeoutExpired as e:
            print((e.stderr or b'')[-6000:]);raise
        print(r.stdout,r.stderr);assert r.returncode==0,r.returncode;logs.append({'mode':('sdp-baseline'if baseline else mode or 'outgoing'),'output':r.stdout.strip()})
# Compile the actual previous policy against the same ABI and virtual peer.
# Regression success requires rejection at the policy assertion, not a crash.
old_source=(w/'hekate-nova-pro-v24/nyx/nyx_gui/bluetooth/nova_bt_stack.c').read_text()
(d/'old-stack.c').write_text(old_source)
old_sources=[str(d/'old-stack.c')if n==str(p/'bluetooth/nova_bt_stack.c')else n for n in sources]
old_exe=d/'negotiation-old.exe'
old_args=[str(cc),'-std=gnu11','-O1','-g','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
 '-I'+str(d/'known-config'),'-I'+str(p/'bluetooth'),'-I'+str(lib)]
r=subprocess.run([*old_args,str(d/'negotiation.c'),*old_sources,'-o',str(old_exe)],capture_output=True,text=True)
assert r.returncode==0,r.stderr
old_failures=[]
for mode,expected in [('incoming','a[6]==0'),('stalled-first','!stall_first')]:
 r=subprocess.run([str(old_exe),mode],capture_output=True,text=True,timeout=10)
 assert r.returncode==1 and expected in r.stderr,(mode,r.returncode,r.stderr)
 old_failures.append({'mode':mode,'expected_assertion':expected})
print('PASS: v24 policy fails both central-role and concurrent-paging regressions at their precise assertions')
# Reproduce the physical refusal with the actual v25 HID implementation.
(d/'v25-hid.c').write_text((w/'hekate-nova-pro-v25/nyx/nyx_gui/libs/btstack/classic/hid_host.c').read_text())
old_hid_sources=[str(d/'v25-hid.c')if n==str(lib/'classic/hid_host.c')else n for n in sources]
old_hid_exe=d/'negotiation-v25-hid.exe'
r=subprocess.run([*old_args,str(d/'negotiation.c'),*old_hid_sources,'-o',str(old_hid_exe)],capture_output=True,text=True)
assert r.returncode==0,r.stderr
r=subprocess.run([str(old_hid_exe),'hid-cross-control'],capture_output=True,text=True,timeout=10)
assert r.returncode==1 and '!refused_cross' in r.stderr,(r.returncode,r.stderr)
old_hid_failure={'source':'v25 hid_host.c','actual_l2cap_refusal':4,'assertion':'!refused_cross','physical_radio':False}
print('PASS: actual v25 HID code reproduces refusal0004 on a crossed control request; v26 accepts it')
# A controller model documented by HWTest: full EDR-sized inputs are lost
# unless BR packet types are selected. Run the actual v26 stack on that model.
(d/'v26-stack.c').write_text((w/'hekate-nova-pro-v26/nyx/nyx_gui/bluetooth/nova_bt_stack.c').read_text())
v26_sources=[str(d/'v26-stack.c')if n==str(p/'bluetooth/nova_bt_stack.c')else n for n in sources]
v26_exe=d/'negotiation-v26-rom.exe'
r=subprocess.run([*old_args,str(d/'negotiation.c'),*v26_sources,'-o',str(v26_exe)],capture_output=True,text=True)
assert r.returncode==0,r.stderr
r=subprocess.run([str(v26_exe),'hwtest-rom'],capture_output=True,text=True,timeout=10)
assert r.returncode==1 and 'report_id==0x30' in r.stderr,(r.returncode,r.stderr)
print('PASS: actual v26 stack fails to receive full inputs in the HWTest EDR-drop model; v27 does')
# Actual v28 stops at three outgoing attempts, even if the peers wake later.
(d/'v28-stack.c').write_text((w/'hekate-nova-pro-v28/nyx/nyx_gui/bluetooth/nova_bt_stack.c').read_text())
v28_sources=[str(d/'v28-stack.c')if n==str(p/'bluetooth/nova_bt_stack.c')else n for n in sources]
v28_exe=d/'negotiation-v28-retry.exe'
r=subprocess.run([*old_args,'-DEXPECT_OLD_PAGE_TIMEOUT',str(d/'negotiation.c'),*v28_sources,'-o',str(v28_exe)],capture_output=True,text=True)
assert r.returncode==0,r.stderr
r=subprocess.run([str(v28_exe),'retry-wake'],capture_output=True,text=True,timeout=10)
assert r.returncode==1 and 'create_side[0]>=5' in r.stderr,(r.returncode,r.stderr)
print('PASS: actual v28 stops before late wake-up; v32 continues and restores the right peer after timeout08')
(w/'nova32-negotiation-check.json').write_text(json.dumps({'passed':True,'logs':logs,'old_policy_failures':old_failures,'v25_hid_regression':old_hid_failure,'hwtest_rom_regression':True,'late_wake_v28_regression':True,'radio_health_through_hci':True,'physical_radio':False,
    'source_sha256':{f.relative_to(p).as_posix():hashlib.sha256(f.read_bytes()).hexdigest()for f in (p/'bluetooth').glob('*.[ch]')},
    'hid_host_sha256':hashlib.sha256((lib/'classic/hid_host.c').read_bytes()).hexdigest()},indent=2))
