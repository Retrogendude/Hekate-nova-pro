
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#define BIT(n) (1u<<(n))
#include <display/tv/usbpd.h>
static int status_result,pdo_result,reads;
static u16 status;
static u16 device_id=0x03B0;
static u8 offers[29];
static int _rd(u8 reg,void*buf,u32 n) {
 ++reads;
 assert(reg==0x03||reg==0x08||reg==0x4E); // no ALERT_STATUS consumption or control write
 memset(buf,0,n);
 if(reg==0x4E) {assert(n==2);((u8*)buf)[0]=device_id;((u8*)buf)[1]=device_id>>8;return 0;}
 if(reg==0x03) {assert(n==2);((u8*)buf)[0]=status;((u8*)buf)[1]=status>>8;return status_result;}
 assert(n==29);memcpy(buf,offers,n);return pdo_result;
}
#define STATUS1_REG         0x03
#define READ_PDOS_SRC_REG   0x08
#define DEV_ID_REG          0x4E
#define DOCK_ID_MV          5000
#define DOCK_ID_MA          500
#define DOCK_MV             15000
#define DOCK_MA_MIN         2600
#define DOCK_MA_MAX         3000
static void _pdo_decode(u32 pdo, u32 *mv, u32 *ma)
{
    if (pdo >> 30) { // This handshake supports fixed PDOs only.
        *mv = *ma = 0;
        return;
    }
    *ma = (pdo & 0x3FF) * 10;
    *mv = ((pdo >> 10) & 0x3FF) * 50;
}
int pd_dock_attached(bool *inserted, bool *official_dock)
{
    u8 status[2], pdos[29];
    *inserted = false;
    *official_dock = false;
    int r = _rd(STATUS1_REG, status, sizeof(status));
    if (r)
        return r;
    *inserted = ((status[0] | status[1] << 8) & PD_S1_INSERT) != 0;
    if (!*inserted) {
        u8 id[2];
        r = _rd(DEV_ID_REG, id, sizeof(id));
        if (r)
            return r;
        // All-zero replies from an unresponsive controller are not an unplug.
        if ((id[0] | (u16)id[1] << 8) != 0x03B0)
            return 1;
        return 0;
    }
    r = _rd(READ_PDOS_SRC_REG, pdos, sizeof(pdos));
    if (r)
        return r;
    if (pdos[0] != 8)
        return 0;
    u32 mv[2], ma[2];
    for (u32 i = 0; i < 2; i++) {
        const u8 *b = pdos + 1 + i * 4;
        u32 pdo = (u32)b[0] | (u32)b[1] << 8 | (u32)b[2] << 16 | (u32)b[3] << 24;
        _pdo_decode(pdo, &mv[i], &ma[i]);
    }
    *official_dock = mv[0] == DOCK_ID_MV && ma[0] == DOCK_ID_MA &&
                     mv[1] == DOCK_MV && ma[1] >= DOCK_MA_MIN && ma[1] <= DOCK_MA_MAX;
    return 0;
}

static void pdo(unsigned index,unsigned mv,unsigned ma,unsigned type) {
 u32 v=(mv/50<<10)|(ma/10)|(type<<30);memcpy(offers+1+index*4,&v,4);
}
int main(void) {
 unsigned cases=0;bool inserted,dock;
 assert(!pd_dock_attached(&inserted,&dock)&&!inserted&&!dock&&reads==2);cases++;
 device_id=0;assert(pd_dock_attached(&inserted,&dock)!=0&&!dock);cases++;device_id=0x03B0;
 status=PD_S1_INSERT;offers[0]=8;pdo(0,5000,500,0);
 for(unsigned ma=2600;ma<=3000;ma+=100){pdo(1,15000,ma,0);assert(!pd_dock_attached(&inserted,&dock)&&inserted&&dock);cases++;}
 pdo(1,15000,2000,0);assert(!pd_dock_attached(&inserted,&dock)&&inserted&&!dock);cases++;
 pdo(1,20000,3000,0);assert(!pd_dock_attached(&inserted,&dock)&&!dock);cases++;
 pdo(1,15000,3000,0);pdo(0,5000,3000,0);assert(!pd_dock_attached(&inserted,&dock)&&!dock);cases++;
 pdo(0,5000,500,0);pdo(1,15000,3000,3);assert(!pd_dock_attached(&inserted,&dock)&&!dock);cases++;
 offers[0]=4;assert(!pd_dock_attached(&inserted,&dock)&&inserted&&!dock);cases++;
 offers[0]=255;assert(!pd_dock_attached(&inserted,&dock)&&!dock);cases++;
 offers[0]=8;status_result=1;assert(pd_dock_attached(&inserted,&dock)&&!inserted&&!dock);cases++;
 status_result=0;pdo_result=1;assert(pd_dock_attached(&inserted,&dock)&&inserted&&!dock);cases++;
 printf("PASS: %u passive dock reader cases; status/PDO/device ID reads only, no commands or alerts.\n",cases);
}
