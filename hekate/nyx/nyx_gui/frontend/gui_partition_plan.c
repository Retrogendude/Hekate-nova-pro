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
    /* Keep the proposed free capacity visible even when a different model
     * is previewed. Validation for destructive actions remains separate. */
    nova_plan_result capacity=nova_partition_validate(plan);
    bool same_family=actual_model==chosen_model||(actual_model<2u&&chosen_model<2u);
    if(!same_family)return NOVA_PLAN_MODEL;
    if(!emmc_data_mib)return NOVA_PLAN_EMMC_UNAVAILABLE;
    if((u64)emmc_data_mib+NOVA_EMU_RESERVE_MIB>plan->emu_mib)return NOVA_PLAN_EMMC_UPGRADED;
    return capacity;
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

nova_plan_result nova_partition_installation(nova_partition_plan *plan,u32 actual_model,
    u32 chosen_model,u32 emmc_data_mib,u32 systems)
{
    if(!plan)return NOVA_PLAN_INVALID;
    plan->fat_mib=plan->emu_mib=plan->linux_mib=plan->android_mib=0;plan->emu_double=false;
    if(!systems||(systems&~7u)||actual_model>3u||
        (chosen_model!=0&&chosen_model!=2&&chosen_model!=3))return NOVA_PLAN_INVALID;
    if(actual_model!=chosen_model&&!(actual_model<2&&chosen_model==0))return NOVA_PLAN_MODEL;
    if(systems&1u){
        if(!emmc_data_mib)return NOVA_PLAN_EMMC_UNAVAILABLE;
        plan->emu_mib=chosen_model==3?NOVA_EMU_64_MIB:NOVA_EMU_32_MIB;
        if((u64)emmc_data_mib+NOVA_EMU_RESERVE_MIB>plan->emu_mib)return NOVA_PLAN_EMMC_UPGRADED;
    }
    plan->linux_mib=(systems&2u)?32768u:0;
    plan->android_mib=(systems&4u)?32768u:0;
    nova_plan_result result=nova_partition_validate(plan);
    /* Prefer 32GiB per OS; fall back together to the existing 16GiB presets.
     * Never reduce a full emuMMC or the FAT32 minimum to force a fit. */
    if(result==NOVA_PLAN_CAPACITY){
        if(systems&2u)plan->linux_mib=16384u;
        if(systems&4u)plan->android_mib=16384u;
        result=nova_partition_validate(plan);
    }
    return result;
}

bool nova_install_target_valid(const nova_partition_plan *p,u32 sectors,u32 gpp,
    u32 index,u32 type,u32 start,u32 size)
{
    if(!p||!p->emu_mib||p->emu_double||!gpp||type!=0xE0)return false;
    u64 expected=0x8000ULL+((u64)p->fat_mib+p->linux_mib+p->android_mib)*2048;
    u64 expected_size=(u64)p->emu_mib*2048-0x800;
    u32 expected_index=p->linux_mib&&!p->android_mib?2:1;
    return index==expected_index&&start==expected&&size==expected_size&&
        expected+expected_size<=sectors&&0xC000ULL+gpp<=expected_size&&
        (sectors-sectors%NOVA_DISK_ALIGN_SECTORS)==p->total_sectors;
}

/* Explicit assistant choices never fall back to a smaller OS partition. */
nova_plan_result nova_partition_installation_sizes(nova_partition_plan *plan,u32 actual_model,
    u32 chosen_model,u32 emmc_data_mib,u32 systems,u32 linux_mib,u32 android_mib)
{
    nova_plan_result result=nova_partition_installation(plan,actual_model,chosen_model,emmc_data_mib,systems);
    if(result!=NOVA_PLAN_OK&&result!=NOVA_PLAN_CAPACITY)return result;
    if(((systems&2u)&&linux_mib!=16384u&&linux_mib!=32768u)||
        ((systems&4u)&&android_mib!=16384u&&android_mib!=32768u))return NOVA_PLAN_INVALID;
    plan->linux_mib=(systems&2u)?linux_mib:0;
    plan->android_mib=(systems&4u)?android_mib:0;
    return nova_partition_validate(plan);
}
