/* Validated partition plans. SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef GUI_PARTITION_PLAN_H
#define GUI_PARTITION_PLAN_H
#include <utils/types.h>

/* These are the existing Hekate FULL allocations, including boot/alignment
 * reserves. 29/58 GiB are rounded descriptions, not truncated allocations. */
#define NOVA_EMU_32_MIB 29856u
#define NOVA_EMU_64_MIB 59680u
#define NOVA_EMU_RESERVE_MIB 32u
#define NOVA_ANDROID_SYSTEM_MIB 6144u
#define NOVA_LINUX_MIN_MIB 8192u
#define NOVA_ANDROID_MIN_MIB 10240u
#define NOVA_DISK_ALIGN_SECTORS 0x8000u

typedef enum {
    NOVA_PLAN_OK, NOVA_PLAN_CAPACITY, NOVA_PLAN_MODEL,
    NOVA_PLAN_EMMC_UNAVAILABLE, NOVA_PLAN_EMMC_UPGRADED,
    NOVA_PLAN_LINUX_SIZE, NOVA_PLAN_ANDROID_SIZE,
    NOVA_PLAN_EMUMMC_SIZE, NOVA_PLAN_INVALID
} nova_plan_result;
typedef struct {
    u32 total_sectors;
    u32 fat_min_mib;
    u32 emu_mib,linux_mib,android_mib;
    u32 fat_mib,alignment_sectors;
    bool emu_double;
} nova_partition_plan;

nova_plan_result nova_partition_validate(nova_partition_plan *plan);
nova_plan_result nova_partition_simplified(nova_partition_plan *plan,
    u32 actual_model,u32 chosen_model,u32 emmc_data_mib,u32 linux_choice,u32 android_choice);
void nova_partition_widths(const nova_partition_plan *plan,u32 width,u32 out[4]);
/* Installation assistant: model family 0=V1/V2, 2=Lite, 3=OLED.
 * Flags: bit0=emuMMC, bit1=Linux, bit2=Android. No storage I/O. */
nova_plan_result nova_partition_installation(nova_partition_plan *plan,u32 actual_model,
    u32 chosen_model,u32 emmc_data_mib,u32 systems);
bool nova_install_target_valid(const nova_partition_plan *plan,u32 sectors,u32 gpp_sectors,
    u32 index,u32 type,u32 start,u32 size);
#endif
