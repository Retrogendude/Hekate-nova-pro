"""Test the actual passive dock reader with controlled I2C replies."""
from pathlib import Path
import re, subprocess
w = Path(__file__).resolve().parent
p = w/'hekate-nova-pro-v20-dock'
s = (p/'bdk/display/tv/usbpd.c').read_text()
def function(name):
    m = re.search(r'^(?:static )?(?:void|int) '+name+r'\([^;]*?\)\n\{',s,re.M)
    assert m,name
    end = re.search(r'^\}',s[m.end():],re.M)
    return s[m.start():m.end()+end.end()]+'\n'
code = r'''
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
'''
code += '\n'.join(re.findall(r'^#define (?:DOCK_\w+|STATUS1_REG|READ_PDOS_SRC_REG|DEV_ID_REG)[^\n]*',s,re.M))+'\n'
code += function('_pdo_decode')+function('pd_dock_attached')
code += r'''
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
'''
d = w/'nova-dock-tests'
(d/'passive-probe.c').write_text(code)
subprocess.run([str(w/'toolchains/w64devkit/bin/gcc.exe'),str(d/'passive-probe.c'),
                '-I'+(d/'stubs').as_posix(),'-I'+(p/'bdk').as_posix(),
                '-o',str(d/'passive-probe.exe'),'-Wall','-Wextra','-Werror'],check=True)
subprocess.run([str(d/'passive-probe.exe')],check=True)
