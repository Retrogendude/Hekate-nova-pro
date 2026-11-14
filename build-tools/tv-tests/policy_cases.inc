int main(void) {
 unsigned tests=0;
 const dpout_mode_t mode={74250,1280,720,110,40,220,5,5,20,1,1};
 const u8 rates[]={6,10,20};
 for(unsigned lanes=1;lanes<=4;lanes*=2) for(unsigned ri=0;ri<3;ri++) {
   dpout_link_t l={0};l.lanes=lanes;l.bw=rates[ri];
   bool enough=(uint64_t)74250000*24<(uint64_t)27*l.bw*1000000*8*lanes;
   assert((_link_calc(&l,&mode)==0)==enough);
   if(enough) {assert(l.tu>=32&&l.tu<=64);assert(l.active_frac>=1&&l.active_frac<=15);assert(l.active_count<l.tu);assert(l.watermark<=30);}
   tests++;
 }
 unsigned volts[]={5000,9000,12000,15000};
 for(unsigned v=0;v<4;v++) for(unsigned ma=0;ma<=3000;ma++) {
   u32 ask=_op_ma(volts[v],ma);assert(ask<=ma);
   assert(ask<=(volts[v]==15000?1200:volts[v]==12000?1500:2000));tests++;
 }
 u32 mv,ma;_pdo_decode(((15000/50)<<10)|(3000/10),&mv,&ma);assert(mv==15000&&ma==3000);tests++;
 for(unsigned t=1;t<=3;t++){_pdo_decode((t<<30)|0x3fffffff,&mv,&ma);assert(!mv&&!ma);tests++;}
 dock_t d={0};d.pdo_n=2;d.pdo_mv[0]=5000;d.pdo_ma[0]=500;d.pdo_mv[1]=15000;d.pdo_ma[1]=3000;
 assert(_choose(&d)&&d.pick==2&&d.dock_power);tests++;
 d.pdo_mv[1]=20000;assert(!_choose(&d));tests++;
 memset(&d,0,sizeof(d));d.pdo_n=3;d.pdo_mv[0]=5000;d.pdo_ma[0]=3000;d.pdo_mv[1]=20000;d.pdo_ma[1]=3000;d.pdo_mv[2]=9000;d.pdo_ma[2]=2000;
 assert(_choose(&d)&&d.pick==3);tests++;
 memset(&d,0,sizeof(d));d.pdo_n=1;d.pdo_mv[0]=5000;assert(!_choose(&d));tests++;
 const u8 raw[]={0,31,32,65,126,127,255};char text[9];memset(text,'x',sizeof(text));_text(raw,text,7);
 assert(!strcmp(text,".. A~..")&&text[8]=='x');tests++;
 printf("PASS: %u power-profile / link-calculation / text cases (no hardware).\n",tests);
}
