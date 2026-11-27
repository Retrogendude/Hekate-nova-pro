#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)

#define DPCD_TEST_CRC 0x240
#define DPCD_TEST_SINK_MISC 0x246
#define TEST_CRC_SUPPORTED BIT(5)
#define TEST_CRC_COUNT_MASK 0x0F
#define DPCD_TEST_SINK 0x270
#define TEST_SINK_START BIT(0)
static u8 control[3],pixels[6];
static int op,fail_at,misc_reads,starts,stops,slept;
void msleep(u32 ms){C(ms==100);slept++;}
int dpout_dpcd_read(u32 addr,u8*b,u32 n){
 op++;if(op==fail_at)return 1;
 if(addr==0x246){C(n==1&&misc_reads<3);*b=control[misc_reads++];}
 else{C(addr==0x240&&n==6);memcpy(b,pixels,6);}return 0;
}
int dpout_dpcd_write(u32 addr,const u8*b,u32 n){
 C(addr==0x270&&n==1&&*b<=1);if(*b)starts++;else stops++;
 op++;return op==fail_at;
}
static void _read_sink_crc(dpout_video_t *v)
{
    u8 b = 0, c0, d[6] = { 0 };
    const u8 allowed = TEST_CRC_SUPPORTED | TEST_CRC_COUNT_MASK;
    v->sink_crc_test = v->sink_crc_valid = false;
    v->sink_crc_frames = 0;
    memset(v->sink_crc, 0, sizeof(v->sink_crc));
    if (dpout_dpcd_read(DPCD_TEST_SINK_MISC, &b, 1) ||
        !(b & TEST_CRC_SUPPORTED) || (b & ~allowed))
        return;
    v->sink_crc_test = true;
    b = TEST_SINK_START;
    if (dpout_dpcd_write(DPCD_TEST_SINK, &b, 1))
        goto stop;
    if (dpout_dpcd_read(DPCD_TEST_SINK_MISC, &b, 1) ||
        !(b & TEST_CRC_SUPPORTED) || (b & ~allowed))
        goto stop;
    c0 = b & TEST_CRC_COUNT_MASK;
    msleep(100);
    if (dpout_dpcd_read(DPCD_TEST_SINK_MISC, &b, 1) ||
        !(b & TEST_CRC_SUPPORTED) || (b & ~allowed) ||
        dpout_dpcd_read(DPCD_TEST_CRC, d, sizeof(d)))
        goto stop;
    v->sink_crc_frames = ((b & TEST_CRC_COUNT_MASK) - c0) & TEST_CRC_COUNT_MASK;
    v->sink_crc[0] = d[0] | (d[1] << 8);
    v->sink_crc[1] = d[2] | (d[3] << 8);
    v->sink_crc[2] = d[4] | (d[5] << 8);
    v->sink_crc_valid = v->sink_crc_frames != 0;
stop:
    b = 0;
    if (dpout_dpcd_write(DPCD_TEST_SINK, &b, 1))
        v->sink_crc_valid = false;
}
static void reset(dpout_video_t*v){memset(v,0,sizeof(*v));v->sink_crc_valid=true;v->sink_crc_frames=7;op=fail_at=misc_reads=starts=stops=slept=0;control[0]=control[1]=0x20;control[2]=0x23;memset(pixels,0,sizeof pixels);}
int main(void){dpout_video_t v;
 reset(&v);_read_sink_crc(&v);C(v.sink_crc_valid&&v.sink_crc_test&&v.sink_crc_frames==3&&starts==1&&stops==1&&op==6);C(!v.sink_crc[0]); // valid black CRC
 reset(&v);control[0]=0x2B;control[1]=0x20;control[2]=0x20;memset(pixels,0x34,6);_read_sink_crc(&v);C(!v.sink_crc_valid&&!v.sink_crc_frames&&stops==1); // enabling clears old count: no false delta from an earlier run
 reset(&v);control[1]=0x2E;control[2]=0x21;_read_sink_crc(&v);C(v.sink_crc_valid&&v.sink_crc_frames==3); // counter wrap
 for(int i=1;i<=6;i++){reset(&v);fail_at=i;_read_sink_crc(&v);C(!v.sink_crc_valid);C(stops==(i==1?0:1));} // each AUX read/write failure
 reset(&v);control[0]=0;_read_sink_crc(&v);C(!v.sink_crc_test&&!v.sink_crc_valid&&!starts&&!stops);
 for(int i=0;i<3;i++){reset(&v);control[i]=0xFF;_read_sink_crc(&v);C(!v.sink_crc_valid);C(stops==(i?1:0));}
 reset(&v);control[2]=0x20;_read_sink_crc(&v);C(!v.sink_crc_valid&&!v.sink_crc_frames);
 printf("PASS: %u fresh-CRC capture assertions; reset counter, wrap, zero CRC, stale data and every AUX failure.\n",checks);
}
