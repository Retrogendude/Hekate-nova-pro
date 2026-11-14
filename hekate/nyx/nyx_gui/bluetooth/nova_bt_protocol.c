#include "nova_bt.h"
#include <string.h>
static int nonzero(const uint8_t *p, unsigned n)
{ unsigned v=0; while(n--) v |= *p++; return v!=0; }
int nova_bt_validate_peers(const nova_bt_peer p[2])
{
    int count=0;
    for(unsigned i=0;i<2;i++) if(p[i].valid) {
        if(!nonzero(p[i].mac,6)||!nonzero(p[i].host,6)||!nonzero(p[i].key,16)) return 0;
        if((p[i].mac[0]&1)||(p[i].host[0]&1)) return 0;
        count++;
    }
    if(count==2 && (memcmp(p[0].host,p[1].host,6)||!memcmp(p[0].mac,p[1].mac,6))) return 0;
    return count;
}
int nova_bt_decode(nova_bt_pad *p, unsigned side, const uint8_t *r, unsigned n, uint32_t now)
{
    /* BTstack delivers HID DATA header 0xa1 followed by the report ID. */
    if(n && r[0]==0xa1){r++;n--;}
    if(side>1||n<13||(r[0]!=0x30&&r[0]!=0x21)) return 0;
    uint32_t buttons=r[3]|((uint32_t)r[4]<<8)|((uint32_t)r[5]<<16);
    p->buttons=buttons & (side?0x0016ffu:0xff2900u);
    unsigned at=side?9:6;
    p->x=r[at]|((r[at+1]&15)<<8);p->y=(r[at+1]>>4)|(r[at+2]<<4);
    p->battery=(r[2]>>4)&0xf;p->updated=now;p->connected=1;
    return 1;
}
