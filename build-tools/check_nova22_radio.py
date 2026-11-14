"""Exercise actual UART/GPIO lifecycle with registers substituted in RAM."""
from pathlib import Path
import re, subprocess, json, hashlib
w=Path(__file__).resolve().parent;p=w/'hekate-nova-pro-v22'
source=p/'nyx/nyx_gui/bluetooth/nova_bt_radio.c';d=w/'nova22-tests'
s=source.read_text();s=re.sub(r'^#include[^\n]*\n','',s,flags=re.M)
s=re.sub(r'^#define U .*\n','',s,flags=re.M)
s=s.replace('GPIO(alias)=(mask<<8)|(value&mask);','GPIO(alias)=(mask<<8)|(value&mask);test_alias(alias,mask,value);')
before=r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <soc/clock.h>
#include <soc/gpio.h>
#include <soc/pinmux.h>
#include <soc/uart.h>
#include <soc/t210.h>
#include <soc/timer.h>
#undef GPIO
#undef CLOCK
#undef PINMUX_AUX
static u32 gp[1024],ck[1024],pm[1024],now,writes,freeze;
static uart_t fake_uart;
#define GPIO(off) gp[(off)/4]
#define CLOCK(off) ck[(off)/4]
#define PINMUX_AUX(off) pm[(off)/4]
#define U (&fake_uart)
static void test_alias(u32 alias,u32 mask,u32 value){
 if(alias>=0x18c&&alias<=0x1ac){assert(!(mask&1));assert(!freeze||now>=220);}
 if(alias==0x1ac&&(mask&BIT(4))&&(value&BIT(4)))freeze=1;
 u32 *reg=&GPIO(alias-0x80);*reg=(*reg&~mask)|(value&mask);writes++;
}
u32 get_tmr_ms(void){return now;}
void msleep(u32 ms){now+=ms;}
void clock_enable_uart(u32 id){assert(id==UART_D);CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_U)|=BIT(CLK_U_UARTD);}
void clock_disable_uart(u32 id){assert(id==UART_D);CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_U)&=~BIT(CLK_U_UARTD);}
int clock_uart_use_src_div(u32 id,u32 rate){assert(id==UART_D&&rate==115200);CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UARTD)=0;return 1;}
'''
after=r'''
int main(void){
 for(unsigned i=0;i<1024;i++){pm[i]=0xa500+i;gp[i]=0x80+i;}
 u32 original_pm[1024];memcpy(original_pm,pm,sizeof pm);
 const u32 regs[]={0x10c,0x11c,0x12c,0x208,0x218,0x200};u32 original_gp[6];
 for(unsigned i=0;i<6;i++)original_gp[i]=GPIO(regs[i]);
 CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UARTD)=0x1234;
 assert(nova_bt_radio_open());assert(now==220);assert(saved.active);
 for(unsigned i=0;i<3;i++)assert((GPIO(regs[i])&1)==(original_gp[i]&1));
 assert(U->UART_MCR==(UART_MCR_RTS_EN|UART_MCR_CTS_EN));
 assert((GPIO(0x12c)&BIT(4))&&!(GPIO(0x12c)&BIT(1)));
 assert(!nova_bt_radio_write(12));U->UART_MSR=BIT(4);U->UART_LSR=UART_LSR_THRE;
 assert(nova_bt_radio_write(0));U->UART_LSR=0;
 for(unsigned i=1;i<16;i++)assert(nova_bt_radio_write(i));
 assert(!nova_bt_radio_write(16));U->UART_LSR=UART_LSR_THRE;assert(nova_bt_radio_write(16));
 U->UART_LSR=0;assert(nova_bt_radio_read()==-1);
 U->UART_LSR=UART_LSR_RDR;U->UART_THR_DLAB=0x41;assert(nova_bt_radio_read()==0x41);
 for(unsigned i=1;i<=4;i++){U->UART_LSR=BIT(i);assert(nova_bt_radio_read()==-2);}
 nova_bt_radio_close();assert(!saved.active);assert(!nova_bt_radio_write(5));assert(nova_bt_radio_read()==-1);
 for(unsigned i=0;i<6;i++)assert(GPIO(regs[i])==original_gp[i]);
 assert(!memcmp(pm,original_pm,sizeof pm));assert(CLOCK(CLK_RST_CONTROLLER_CLK_SOURCE_UARTD)==0x1234);
 assert(!(CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_U)&BIT(CLK_U_UARTD)));
 u32 old_writes=writes;nova_bt_radio_close();assert(writes==old_writes);
 CLOCK(CLK_RST_CONTROLLER_CLK_OUT_ENB_U)|=BIT(CLK_U_UARTD);
 assert(!nova_bt_radio_open()&&writes==old_writes); /* refuse an occupied UART */
 puts("PASS real radio lifecycle: exclusive ownership, masked GPIO restoration, FIFO bursts, CTS backpressure and UART faults");
}
'''
(d/'radio.c').write_text(before+s+after)
cmd=[str(w/'toolchains/w64devkit/bin/gcc.exe'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-Wno-pointer-to-int-cast','-I'+str(p/'bdk'),str(d/'radio.c'),'-o',str(d/'radio.exe')]
r=subprocess.run(cmd,capture_output=True,text=True);assert r.returncode==0,r.stderr
r=subprocess.run([str(d/'radio.exe')],capture_output=True,text=True,timeout=10);print(r.stdout,r.stderr);assert r.returncode==0
(w/'nova22-radio-check.json').write_text(json.dumps({'passed':True,'output':r.stdout,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'hardware_io':'registers substituted'},indent=2))
