/*
 * USB-PD driver for Nintendo Switch's TI BM92T36
 *
 * Copyright (c) 2020 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __BM92T36_H_
#define __BM92T36_H_

#include <utils/types.h>

#define BM92T36_I2C_ADDR 0x18

typedef struct _usb_pd_object_t
{
	u32 amperage;
	u32 voltage;
} usb_pd_object_t;

typedef struct _usb_pd_objects_t
{
	u32 pdo_no;
	usb_pd_object_t pdos[7];
	usb_pd_object_t selected_pdo;
} usb_pd_objects_t;

// Read-only BM92T36 register diagnostics. A read result of 0 means success.
typedef struct _bm92t36_diag_t
{
	u16 status1;
	u16 status2;
	u16 dp_status_first;
	u16 dp_status_second;
	u16 reg1c;
	u16 reg1d;
	int status1_result;
	int status2_result;
	int dp_status_first_result;
	int dp_status_second_result;
	int reg1c_result;
	int reg1d_result;
} bm92t36_diag_t;

int  bm92t36_get_version(u32 *value);
void bm92t36_get_source_info(bool *inserted, usb_pd_objects_t *usb_pd);
void bm92t36_get_diagnostics(bm92t36_diag_t *diag);

#endif
