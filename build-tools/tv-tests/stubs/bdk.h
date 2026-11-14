
#pragma once
#include <utils/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define _GUI_H_
#define ALIGN(x,a) (((x)+(a)-1)&~((uintptr_t)(a)-1))
#define I2C_5 5
#define GPIO_PIN_1 2
#define GPIO_PORT_CC 28
#define FUSE_NX_HW_TYPE_HOAG 2
#define BPMP_MMU_MAINT_CLEAN_PHY 1
#define NYX_FB_SZ (1280*720*4)
#define NYX_FB2_ADDRESS ((uintptr_t)source_frame)
#define PINMUX_AUX(x) fake_mux
#define GPIO(x) fake_gpio[((x)-0x700)/16]
#define LV_HOR_RES 1280
#define LV_LAYOUT_OFF 0
#define LV_LABEL_LONG_BREAK 0
#define LV_TASK_PRIO_MID 2
#define LV_RES_OK 0
#define LV_SIGNAL_CLEANUP 1
static u32 fake_mux, fake_gpio[2], source_frame[1280*720];
typedef int lv_res_t; typedef int lv_signal_t; typedef int bpmp_freq_t;
#define BPMP_CLK_NORMAL 0
struct lv_obj; typedef struct lv_obj lv_obj_t;
typedef lv_res_t (*lv_signal_func_t)(lv_obj_t*,lv_signal_t,void*);
typedef lv_res_t (*lv_action_t)(lv_obj_t*);
struct lv_obj {lv_signal_func_t signal;};
typedef struct {int dummy;} lv_task_t;
static lv_obj_t objects[20]; static lv_task_t task_object;
static int object_n, task_fail, task_deleted, hardware, video_stops, dock_stops, frees, alloc_fail;
static int hwtype, id_ok, rail_fail, aux_fail, probe_fail, video_fail, hpd=1, mounts=1;
static int snapshot_calls, starts, aux_inits, video_starts, aux_ends, save_fail, saved, clock_sets;
static int gpu_dma_active, short_write, close_fail;
typedef struct {int dummy;} FIL;
#define FA_CREATE_ALWAYS 8
#define FA_WRITE 2
static u32 now;
static char shown[2048];
bpmp_freq_t bpmp_fid_current=3;
static void *test_malloc(size_t n) {return alloc_fail?NULL:malloc(n);}
static void test_free(void *p) {assert(!gpu_dma_active);frees++;free(p);}
#define malloc test_malloc
#define free test_free
#define s_printf sprintf
static lv_res_t base_signal(lv_obj_t *o,lv_signal_t s,void*p) {return 0;}
static lv_obj_t *nyx_create_standard_window(const char*t,lv_action_t a) {lv_obj_t *o=&objects[object_n++];o->signal=base_signal;return o;}
static lv_signal_func_t lv_obj_get_signal_func(lv_obj_t*o) {return o->signal;}
static void lv_obj_set_signal_func(lv_obj_t*o,lv_signal_func_t f) {o->signal=f;}
static void lv_win_add_btn(lv_obj_t*o,void*i,const char*t,lv_action_t a) {}
static lv_obj_t *lv_win_get_content(lv_obj_t*o) {return o;}
static lv_obj_t *lv_page_get_scrl(lv_obj_t*o) {return o;}
static void lv_page_set_scrl_layout(lv_obj_t*o,int x) {}
static lv_obj_t *lv_label_create(lv_obj_t*o,void*p) {return o;}
static void lv_label_set_long_mode(lv_obj_t*o,int x) {}
static void lv_obj_set_width(lv_obj_t*o,int x) {}
static void lv_obj_set_pos(lv_obj_t*o,int x,int y) {}
static void lv_label_set_text(lv_obj_t*o,const char*s) {snprintf(shown,sizeof(shown),"%s",s);}
static lv_task_t *lv_task_create(void(*f)(void*),u32 p,int pri,void*a) {return task_fail?NULL:&task_object;}
static void lv_task_del(lv_task_t*t) {task_deleted++;}
static u32 fuse_read_hw_type(void) {return hwtype;}
static u32 get_tmr_ms(void) {return now;}
static void bpmp_clk_rate_set(bpmp_freq_t c) {clock_sets++;hardware++;bpmp_fid_current=c;}
static void bpmp_mmu_maintenance(u32 op,bool force) {}
static void gpio_config(u32 port,u32 pin,int mode) {if(mode)fake_gpio[0]|=pin;else fake_gpio[0]&=~pin;}
static void gpio_output_enable(u32 port,u32 pin,int mode) {if(mode)fake_gpio[1]|=pin;else fake_gpio[1]&=~pin;}
static int i2c_recv_buf_small(void*b,u32 n,u32 bus,u32 addr,u32 reg) {*(u8*)b=0xC5;return rail_fail;}
static int i2c_send_byte(u32 bus,u32 addr,u32 reg,u8 val) {hardware++;return 0;}
static bool sd_mount(void) {return mounts;}
static void sd_unmount(void) {}
static int f_mkdir(const char*p) {return 0;}
static int sd_save_to_file(const void*b,u32 n,const char*p) {saved++;return save_fail;}

static int f_open(FIL*f,const char*p,int flags) {saved++;return save_fail;}
static int f_write(FIL*f,const void*p,u32 n,u32*written) {*written=short_write?n-1:n;return save_fail;}
static int f_close(FIL*f) {return close_fail;}

#define BTN_VOL_DOWN 2
static int volume_keys;
static u8 btn_read_vol(void) {return volume_keys;}
