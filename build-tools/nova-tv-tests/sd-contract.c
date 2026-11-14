#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
static bool sd_init_done, sd_mounted;
static int init_result, mount_result, initializations, mounts, sd_fs;
static struct { bool mute; } gfx_con;
#define FR_OK 0
#define EPRINTF(...) ((void)0)
#define EPRINTFARGS(...) ((void)0)
static bool sdmmc_get_sd_inserted(void) {return true;}
static int sd_initialize(bool b) {initializations++;sd_init_done=!init_result;return init_result;}
static int f_mount(void*p,const char*s,int v) {mounts++;return mount_result;}
int real_sd_mount()
{
	if (sd_init_done && sd_mounted)
		return 0;

	int res = 0;

	if (!sd_init_done)
		res = sd_initialize(false);

	if (res)
	{
		gfx_con.mute = false;
		EPRINTF("Failed to init SD card.");
		if (!sdmmc_get_sd_inserted())
			EPRINTF("Make sure that it is inserted.");
		else
			EPRINTF("SD Card Reader is not properly seated!");
	}
	else
	{
		if (!sd_mounted)
			res = f_mount(&sd_fs, "0:", 1); // Volume 0 is SD.
		if (res == FR_OK)
		{
			sd_mounted = true;
			return 0;
		}
		else
		{
			gfx_con.mute = false;
			EPRINTFARGS("Failed to mount SD card (FatFS Error %d).\nMake sure that a FAT partition exists..", res);
		}
	}

	return 1;
}
int main(void) {
 sd_init_done=sd_mounted=true;assert(real_sd_mount()==0&&!initializations&&!mounts);
 sd_init_done=sd_mounted=false;assert(real_sd_mount()==0&&sd_init_done&&sd_mounted&&initializations==1&&mounts==1);
 sd_init_done=sd_mounted=false;init_result=1;assert(real_sd_mount()!=0&&!sd_mounted);
 sd_init_done=sd_mounted=false;init_result=0;mount_result=13;assert(real_sd_mount()!=0&&!sd_mounted);
 sd_init_done=true;mount_result=0;assert(real_sd_mount()==0&&sd_mounted);
 puts("PASS: real BDK sd_mount returns zero on success; five paths checked.");
}
