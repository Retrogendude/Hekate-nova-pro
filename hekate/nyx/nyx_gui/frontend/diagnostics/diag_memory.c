/*
 * hwtest - one-shot Switch hardware probe / repair report.
 * Split from main.c: memclk probes
 */
#include "diagnostic_bridge.h"


/* ------------------------------------------------------------------------ */
/* DRAM identity (LPDDR4 / LPDDR4X via EMC mode-register reads)             */
/*                                                                          */
/* The two LPDDR chips on the SoM identify themselves via JEDEC mode        */
/* registers. Hekate's gui_info reads:                                      */
/*   MR5 : manufacturer ID (1 = Samsung, 6 = Hynix, 255 = Micron, ...)      */
/*   MR6 : revision ID 1                                                    */
/*   MR7 : revision ID 2                                                    */
/*   MR8 : density encoding (bits 5:2)                                      */
/* sdram_read_mrx() drives the EMC bus to issue a mode-register read on     */
/* both chips and both ranks; we surface chip0/chip1 separately so a board  */
/* with one bad LPDDR shows a clear asymmetry. Erista is LPDDR4, Mariko is  */
/* LPDDR4X - the only practical difference here is naming.                  */
static const char *_dram_vendor_name(u8 v)
{
    switch (v) {
    case 1:   return "Samsung";
    case 5:   return "Nanya";
    case 6:   return "Hynix";
    case 8:   return "Winbond";
    case 19:  return "CXMT";
    case 255: return "Micron";
    default:  return NULL;  /* caller prints numeric */
    }
}

static const char *_dram_density_str(u8 mr8)
{
    switch ((mr8 & 0x3C) >> 2) {
    case 2: return "512MB";
    case 3: return "768MB";
    case 4: return "1GB";
    case 5: return "1.5GB";
    case 6: return "2GB";
    default: return NULL;
    }
}


void probe_dram(void)
{
    HEADER("[DRAM identity]");

    /* HIDREV major: 1 = Erista (LPDDR4), 2 = Mariko (LPDDR4X). The shift
     * returns the major nibble, not a bool, so we have to compare. */
    bool mariko = ((APB_MISC(APB_MISC_GP_HIDREV) >> 4) & 0xF) >= 2;
    LOG("  Type         : %s\n", mariko ? "LPDDR4X (Mariko)" : "LPDDR4 (Erista)");

    u8 dram_id = fuse_read_dramid(true);
    LOG("  Fuse DRAM ID : %d\n", dram_id);

    emc_mr_data_t v   = sdram_read_mrx(MR5_MAN_ID);
    emc_mr_data_t r0  = sdram_read_mrx(MR6_REV_ID1);
    emc_mr_data_t r1  = sdram_read_mrx(MR7_REV_ID2);
    emc_mr_data_t den = sdram_read_mrx(MR8_DENSITY);

    /* Per-chip readout. chip0 = first LPDDR, chip1 = second LPDDR. */
    for (int c = 0; c < 2; c++) {
        u8 vid = c ? v.chip1.rank0_ch0   : v.chip0.rank0_ch0;
        u8 rev0 = c ? r0.chip1.rank0_ch0 : r0.chip0.rank0_ch0;
        u8 rev1 = c ? r1.chip1.rank0_ch0 : r1.chip0.rank0_ch0;
        u8 d8  = c ? den.chip1.rank0_ch0 : den.chip0.rank0_ch0;

        const char *vname = _dram_vendor_name(vid);
        const char *dname = _dram_density_str(d8);
        u32 col = (vname && dname) ? COL_OK : COL_WARN;

        if (vname)
            log_color(col, "  Chip %d vend  : %s\n", c, vname);
        else
            log_color(col, "  Chip %d vend  : Unknown (id %d)\n", c, vid);
        log_color(col, "  Chip %d rev   : %X.%02X\n", c, rev0, rev1);
        if (dname)
            log_color(col, "  Chip %d dens  : %s\n", c, dname);
        else
            log_color(col, "  Chip %d dens  : Unknown (MR8 0x%02X)\n", c, d8);
    }

    /* Cross-rank symmetry check. If a stacked chip has both ranks, MR5 of
     * rank 0 and rank 1 should match; otherwise we're seeing only one
     * working die. Channels also should match - mismatch = bad LPDDR. */
    bool sym0 = (v.chip0.rank0_ch0 == v.chip0.rank0_ch1) &&
                (r0.chip0.rank0_ch0 == r0.chip0.rank0_ch1);
    bool sym1 = (v.chip1.rank0_ch0 == v.chip1.rank0_ch1) &&
                (r0.chip1.rank0_ch0 == r0.chip1.rank0_ch1);
    log_color((sym0 && sym1) ? COL_OK : COL_WARN,
        "  Channel sym  : chip0 %s, chip1 %s\n",
        sym0 ? "OK" : "MISMATCH",
        sym1 ? "OK" : "MISMATCH");
    dx_set("dram_sym", (sym0 && sym1) ? DX_PASS : DX_FAIL,
        (sym0 && sym1) ? "" : "channel asymmetry");

    /* MR4 (per-die refresh-rate / thermal class) is deliberately not read
     * here: in the RCM/BPMP context the LPDDR MR4 mode-register read does
     * not return a trustworthy live value, so a threshold test on it only
     * yields false signals. Catching bad DRAM (marginal cells that stall
     * HOS boot) requires a trained controller plus an actual R/W stress
     * march (a real memtester), which is out of scope for this passive
     * single-pass probe. */
}


