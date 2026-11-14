/* Pure capacity arithmetic; never performs storage I/O. GPL-2.0-or-later. */
#include "gui_partition_plan.h"

nova_plan_result nova_partition_validate(nova_partition_plan *plan)
{
    if(!plan)return NOVA_PLAN_INVALID;
    plan->fat_mib=0;
    plan->alignment_sectors=plan->total_sectors%NOVA_DISK_ALIGN_SECTORS;
    u64 total=(plan->total_sectors-plan->alignment_sectors)>>11;
    if(!plan->fat_min_mib||total<=16u)return NOVA_PLAN_CAPACITY;
    if(plan->linux_mib&&plan->linux_mib<NOVA_LINUX_MIN_MIB)return NOVA_PLAN_LINUX_SIZE;
    if(plan->android_mib&&plan->android_mib<NOVA_ANDROID_MIN_MIB)return NOVA_PLAN_ANDROID_SIZE;
    /* Expert sliders can clamp to the remaining aligned capacity. Preserve
     * those valid 16-MiB endpoints as well as whole-GiB presets. */
    if((plan->linux_mib|plan->android_mib)&15u)return NOVA_PLAN_INVALID;
    if(plan->emu_mib){
        u32 copies=plan->emu_double?2u:1u;
        if(plan->emu_mib%copies)return NOVA_PLAN_EMUMMC_SIZE;
        u32 size=plan->emu_mib/copies;
        if(size!=NOVA_EMU_32_MIB&&size!=NOVA_EMU_64_MIB&&
            (size<4096u+NOVA_EMU_RESERVE_MIB||((size-NOVA_EMU_RESERVE_MIB)&1023u)))
            return NOVA_PLAN_EMUMMC_SIZE;
    }else if(plan->emu_double)return NOVA_PLAN_EMUMMC_SIZE;
    u64 used=(u64)plan->emu_mib+plan->linux_mib+plan->android_mib+16u;
    if(used>total||total-used<plan->fat_min_mib)return NOVA_PLAN_CAPACITY;
    plan->fat_mib=(u32)(total-used);
    return NOVA_PLAN_OK;
}

nova_plan_result nova_partition_simplified(nova_partition_plan *plan,
    u32 actual_model,u32 chosen_model,u32 emmc_data_mib,u32 linux_choice,u32 android_choice)
{
    if(!plan)return NOVA_PLAN_INVALID;
    plan->fat_mib=0;
    if(actual_model>3u||chosen_model>3u||linux_choice>2u||android_choice>2u)return NOVA_PLAN_INVALID;
    plan->emu_double=false;
    plan->emu_mib=chosen_model==3u?NOVA_EMU_64_MIB:NOVA_EMU_32_MIB;
    plan->linux_mib=linux_choice?(linux_choice==1u?16384u:32768u):0u;
    plan->android_mib=android_choice?(android_choice==1u?16384u:32768u):0u;
    if(actual_model!=chosen_model)return NOVA_PLAN_MODEL;
    if(!emmc_data_mib)return NOVA_PLAN_EMMC_UNAVAILABLE;
    if((u64)emmc_data_mib+NOVA_EMU_RESERVE_MIB>plan->emu_mib)return NOVA_PLAN_EMMC_UPGRADED;
    return nova_partition_validate(plan);
}

void nova_partition_widths(const nova_partition_plan *plan,u32 width,u32 out[4])
{
    const u32 sizes[]={plan->fat_mib,plan->emu_mib,plan->linux_mib,plan->android_mib};
    u64 sum=(u64)sizes[0]+sizes[1]+sizes[2]+sizes[3],prefix=0;
    u32 previous=0;
    for(u32 i=0;i<4;++i){
        prefix+=sizes[i];u32 end=sum?(u32)(prefix*width/sum):0;
        out[i]=end-previous;previous=end;
    }
}
