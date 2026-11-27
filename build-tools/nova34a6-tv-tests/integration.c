
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef unsigned u32;typedef unsigned char u8;typedef int lv_task_prio_t;
typedef struct{u32 last_run,period;void(*task)(void*);void*param;}lv_task_t;
typedef struct{lv_task_t*tasks[2];}system_maintenance_tasks_t;
static system_maintenance_tasks_t system_tasks;static bool diagnostics_suspended,display_frame_dirty,display_frame_ready,disp_init_done;
static int tv,bt,refresh,compose,init,order;static u32 now;
static u32 lv_tick_get(void){return now;}static u32 lv_tick_elaps(u32 t){return now-t;}
static void gui_bluetooth_poll_fast(void){bt++;}static void gui_bluetooth_draw_poll(void){}
static void lv_refr_now(void){refresh++;order=1;}static void gui_tv_service(void){tv++;assert(order==0||order==1);}
static void gui_tv_present(void){tv++;assert(order==2);}static void vic_compose_poll(void(*f)(void)){compose++;order=2;}
static void _nyx_disp_init(void){init++;order=2;}
void manual_system_maintenance(bool refresh)
{
	if (diagnostics_suspended)
	{
		if (refresh)
			lv_refr_now();
		return;
	}
	// Long storage/USB operations also service wireless input cooperatively.
	gui_bluetooth_poll_fast();
	for (u32 task_idx = 0; task_idx < (sizeof(system_maintenance_tasks_t) / sizeof(lv_task_t *)); task_idx++)
	{
		lv_task_t *task = system_tasks.tasks[task_idx];
		if (task && (lv_tick_elaps(task->last_run) >= task->period))
		{
			task->last_run = lv_tick_get();
			task->task(task->param);
		}
	}
	if (refresh)
		lv_refr_now();
	gui_tv_service();
}
static void _disp_fb_present(uint32_t elapsed, uint32_t pixels)
{
	(void)elapsed;
	(void)pixels;
	if (!display_frame_dirty)
		return;

	/* LVGL calls this once after all dirty rectangles have been flushed.
	 * Rotating the full frame per rectangle stalls input and scrolling. */
	if (disp_init_done)
		vic_compose_poll(gui_bluetooth_draw_poll);
	else if (display_frame_ready)
	{
		disp_init_done = true;
		_nyx_disp_init();
	}
	display_frame_dirty = false;
	display_frame_ready = false;
	gui_tv_present();
}
static int waits,reads;static u8 buttons[8];
#define BTN_POWER 1
#define BTN_SINGLE 128
static void btn_wait_service(void){waits++;}
static u8 btn_read(void){return buttons[reads++%8];}
static u32 get_tmr_ms(void){return now++;}
u8 btn_wait()
{
	u8 res = 0, btn = btn_read();
	bool pwr = false;

	//Power button down, raise a filter.
	if (btn & BTN_POWER)
	{
		pwr = true;
		btn &= ~BTN_POWER;
	}

	do
	{
		btn_wait_service();
		res = btn_read();
		//Power button up, remove filter.
		if (!(res & BTN_POWER) && pwr)
			pwr = false;
		else if (pwr) //Power button still down.
			res &= ~BTN_POWER;
	} while (btn == res);

	return res;
}
u8 btn_wait_timeout(u32 time_ms, u8 mask)
{
	u32 timeout = get_tmr_ms() + time_ms;
	u8 res = btn_read() & mask;

	while (get_tmr_ms() < timeout)
	{
		btn_wait_service();
		if (res == mask)
			break;
		else
			res = btn_read() & mask;
	};

	return res;
}
u8 btn_wait_timeout_single(u32 time_ms, u8 mask)
{
	u8 single_button = mask & BTN_SINGLE;
	mask &= ~BTN_SINGLE;

	u32 timeout = get_tmr_ms() + time_ms;
	u8 res = btn_read();

	while (get_tmr_ms() < timeout)
	{
		btn_wait_service();
		if ((res & mask) == mask)
		{
			if (single_button && (res & ~mask)) // Undesired button detected.
				res = btn_read();
			else
				return (res & mask);
		}
		else
			res = btn_read();
	};

	// Timed out.
	if (!single_button || !time_ms)
		return (res & mask);
	else
		return 0; // Return no button press if single button requested.
}
int main(void){
 manual_system_maintenance(false);assert(tv==1&&bt==1&&!refresh);
 manual_system_maintenance(true);assert(tv==2&&bt==2&&refresh==1);diagnostics_suspended=true;
 manual_system_maintenance(true);assert(tv==2&&bt==2&&refresh==2);diagnostics_suspended=false;
 display_frame_dirty=true;disp_init_done=true;_disp_fb_present(0,0);assert(tv==3&&compose==1&&!display_frame_dirty);
 display_frame_dirty=true;display_frame_ready=true;disp_init_done=false;_disp_fb_present(0,0);assert(tv==4&&init==1&&disp_init_done);
 order=0;buttons[0]=buttons[1]=0;buttons[2]=1;assert(btn_wait()==1&&waits==2);
 reads=waits=0;now=0;buttons[2]=2;assert(btn_wait_timeout(100,2)==2&&waits>0);
 reads=waits=0;now=0;assert(btn_wait_timeout_single(100,2)==2&&waits>0);
 puts("PASS: actual maintenance/frame-present/button-wait functions pump TV; diagnostics guard preserved; no LVGL scheduler reentry.");
}
