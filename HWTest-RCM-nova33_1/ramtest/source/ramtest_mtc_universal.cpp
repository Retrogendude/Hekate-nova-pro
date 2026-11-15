/*
 * hwtest dedicated RAM build - universal public Atmosphere MTC bridge.
 *
 * Both Erista and Mariko MTC engines are linked into one payload.  The caller
 * selects the SoC immediately on entry, before hardware initialization; every
 * operation below dispatches exclusively to that selected implementation.
 *
 * DRAM-ID to timing-table selection follows Atmosphere-NX fusee_mtc.cpp.
 */
#include <exosphere.hpp>
#include "ramtest_mtc.h"

extern "C" u32 fuse_read_dramid(int raw_id);

namespace ams::nxboot {
    void DoMemoryTrainingMariko(bool *out_did_training, int index, void *mtc_tables_buffer);
    void DoPeriodicTrainingMariko(void *mtc_tables_buffer);
    void RestoreMemoryClockRateMariko(void *mtc_tables_buffer);
    u32 GetMemoryTrainingErrorsMariko();
    u32 GetMemoryTrainingFailedMaskMariko();

    void DoMemoryTrainingErista(int index, void *mtc_tables_buffer);
    void DoPeriodicTrainingErista(void *mtc_tables_buffer);
    void RestoreMemoryClockRateErista(void *mtc_tables_buffer);
    u32 GetMemoryTrainingErrorsErista();
    u32 GetMemoryTrainingFailedMaskErista();
    bool DidMemoryTrainingErista();
}

namespace {

alignas(8) u8 g_mtc_tables_buffer[0x26C0];
u32 g_platform = RAM_MTC_PLATFORM_UNKNOWN;
u32 g_dram_id = 0xFFFFFFFFu;
u32 g_bridge_errors = 0;
u32 g_physical_mib = 0;
bool g_trained = false;

/* Public Atmosphere Mariko mapping. Values are table indices. */
constexpr u8 kMarikoTrainingTableIndices[] = {
    0x00, /* Erista Icosa Samsung 4 GB (never selected on Mariko) */
    0x02, /* Erista Icosa Hynix 4 GB   (never selected on Mariko) */
    0x03, /* Erista Icosa Micron 4 GB  (never selected on Mariko) */
    0x10, /* Mariko Iowa Hynix 1y 4 GB */
    0x01, /* Erista Icosa Samsung 6 GB (never selected on Mariko) */
    0x10, /* Mariko Hoag Hynix 1y 4 GB */
    0x10, /* Mariko Aula Hynix 1y 4 GB */
    0x00, /* Mariko Iowax1x2 Samsung 4 GB */
    0x05, /* Mariko Iowa Samsung 4 GB */
    0x06, /* Mariko Iowa Samsung 8 GB */
    0x07, /* Mariko Iowa Hynix 4 GB */
    0x08, /* Mariko Iowa Micron 4 GB */
    0x05, /* Mariko Hoag Samsung 4 GB */
    0x06, /* Mariko Hoag Samsung 8 GB */
    0x07, /* Mariko Hoag Hynix 4 GB */
    0x08, /* Mariko Hoag Micron 4 GB */
    0x09, /* Mariko Iowa Samsung 4 GB Y */
    0x0C, /* Mariko Iowa Samsung 1y 4 GB X */
    0x0D, /* Mariko Iowa Samsung 1y 8 GB X */
    0x0C, /* Mariko Hoag Samsung 1y 4 GB X */
    0x12, /* Mariko Iowa Samsung 1z 4 GB */
    0x12, /* Mariko Hoag Samsung 1z 4 GB */
    0x12, /* Mariko Aula Samsung 1z 4 GB */
    0x0D, /* Mariko Hoag Samsung 1y 8 GB X */
    0x0C, /* Mariko Aula Samsung 1y 4 GB X */
    0x0F, /* Mariko Iowa Micron 1y 4 GB */
    0x0F, /* Mariko Hoag Micron 1y 4 GB */
    0x0F, /* Mariko Aula Micron 1y 4 GB */
    0x0D, /* Mariko Aula Samsung 1y 8 GB X */
    0x13, /* Mariko Iowa Hynix 1a 4 GB */
    0x13, /* Mariko Hoag Hynix 1a 4 GB */
    0x13, /* Mariko Aula Hynix 1a 4 GB */
    0x14, /* Mariko Iowa Micron 1a 4 GB */
    0x14, /* Mariko Hoag Micron 1a 4 GB */
    0x14, /* Mariko Aula Micron 1a 4 GB */
};

int table_index_for_erista(u32 id)
{
    /* Atmosphere secmon_smc_info.cpp:GetDramIdAdjusted selects the Samsung
     * 6GB-compatible density/rank profile for expanded Erista RAM. The Hekate
     * 8GB ID (7) has no distinct Atmosphere table: retain its 8GB geometry and
     * use that upstream timing profile, never a 4GB profile. */
    if(g_physical_mib==8192)return id==7||id==4?0x01:-1;
    if(g_physical_mib==6144)return id==4?0x01:-1;
    if(g_physical_mib!=4096)return -1;
    switch (id) {
        case 0: return 0x00; /* Samsung 4 GB */
        case 1: return 0x02; /* Hynix 4 GB */
        case 2: return 0x03; /* Micron 4 GB */
        default: return -1;
    }
}

int table_index_for_mariko(u32 id)
{
    if (id >= sizeof(kMarikoTrainingTableIndices))
        return -1;

    /* IDs 0,1,2,4 are Erista-only in Atmosphere's global mapping. */
    if (id == 0 || id == 1 || id == 2 || id == 4)
        return -1;

    const bool eight_gb=id==9||id==13||id==18||id==23||id==28;
    if(g_physical_mib!=(eight_gb?8192u:4096u))return -1;

    return kMarikoTrainingTableIndices[id];
}

} // anonymous namespace

extern "C" void ram_mtc_select_platform(u32 platform)
{
    g_platform = platform;
    g_trained = false;
    g_bridge_errors = 0;
    g_dram_id = 0xFFFFFFFFu;
    g_physical_mib = 0;
}
extern "C" int ram_mtc_set_capacity(u32 physical_mib)
{
    g_physical_mib=physical_mib;
    if(physical_mib==4096||physical_mib==8192||
        (physical_mib==6144&&g_platform==RAM_MTC_PLATFORM_ERISTA))return 0;
    g_bridge_errors|=RAM_MTC_ERR_CAPACITY;return -1;
}
extern "C" int ram_mtc_check_capacity(u32 physical_mib)
{
    if(g_physical_mib&&physical_mib==g_physical_mib)return 0;
    g_bridge_errors|=RAM_MTC_ERR_CAPACITY;g_trained=false;return -1;
}

extern "C" u32 ram_mtc_platform(void)
{
    return g_platform;
}

extern "C" int ram_mtc_is_trained(void)
{
    return g_trained ? 1 : 0;
}

extern "C" int ram_mtc_train(void)
{
    g_trained = false;
    g_bridge_errors = 0;
    /* Honor the same effective 8GB ID that initialized the memory controller. */
    g_dram_id = fuse_read_dramid(0);

    if (g_platform == RAM_MTC_PLATFORM_MARIKO) {
        const int index = table_index_for_mariko(g_dram_id);
        if (index < 0) {
            g_bridge_errors = RAM_MTC_ERR_UNSUPPORTED;
            return -1;
        }

        ams::nxboot::DoMemoryTrainingMariko(&g_trained, index, g_mtc_tables_buffer);
        g_bridge_errors |= ams::nxboot::GetMemoryTrainingErrorsMariko();
        return (g_trained && g_bridge_errors == 0) ? 0 : -1;
    }

    if (g_platform == RAM_MTC_PLATFORM_ERISTA) {
        const int index = table_index_for_erista(g_dram_id);
        if (index < 0) {
            g_bridge_errors = RAM_MTC_ERR_UNSUPPORTED;
            return -1;
        }

        ams::nxboot::DoMemoryTrainingErista(index, g_mtc_tables_buffer);
        g_bridge_errors |= ams::nxboot::GetMemoryTrainingErrorsErista();
        g_trained = ams::nxboot::DidMemoryTrainingErista();
        return (g_trained && g_bridge_errors == 0) ? 0 : -1;
    }

    g_bridge_errors = RAM_MTC_ERR_UNSUPPORTED;
    return -1;
}

extern "C" void ram_mtc_periodic(void)
{
    if (!g_trained)
        return;

    if (g_platform == RAM_MTC_PLATFORM_MARIKO)
        ams::nxboot::DoPeriodicTrainingMariko(g_mtc_tables_buffer);
    else if (g_platform == RAM_MTC_PLATFORM_ERISTA)
        ams::nxboot::DoPeriodicTrainingErista(g_mtc_tables_buffer);
}

extern "C" void ram_mtc_restore(void)
{
    if (!g_trained)
        return;

    if (g_platform == RAM_MTC_PLATFORM_MARIKO) {
        ams::nxboot::RestoreMemoryClockRateMariko(g_mtc_tables_buffer);
        g_bridge_errors |= ams::nxboot::GetMemoryTrainingErrorsMariko();
    } else if (g_platform == RAM_MTC_PLATFORM_ERISTA) {
        ams::nxboot::RestoreMemoryClockRateErista(g_mtc_tables_buffer);
        g_bridge_errors |= ams::nxboot::GetMemoryTrainingErrorsErista();
    }
    g_trained = false;
}

extern "C" u32 ram_mtc_errors(void)
{
    if (g_platform == RAM_MTC_PLATFORM_MARIKO)
        return g_bridge_errors | ams::nxboot::GetMemoryTrainingErrorsMariko();
    if (g_platform == RAM_MTC_PLATFORM_ERISTA)
        return g_bridge_errors | ams::nxboot::GetMemoryTrainingErrorsErista();
    return g_bridge_errors | RAM_MTC_ERR_UNSUPPORTED;
}

extern "C" u32 ram_mtc_failed_training_mask(void)
{
    if (g_platform == RAM_MTC_PLATFORM_MARIKO)
        return ams::nxboot::GetMemoryTrainingFailedMaskMariko();
    if (g_platform == RAM_MTC_PLATFORM_ERISTA)
        return ams::nxboot::GetMemoryTrainingFailedMaskErista();
    return 0;
}

extern "C" u32 ram_mtc_dram_id(void)
{
    return g_dram_id;
}
