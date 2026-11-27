#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define s_printf sprintf
#include <display/tv/dpout.h>
#include <display/tv/usbpd.h>
static unsigned checks;
#define C(x) do {checks++;assert(x);}while(0)
typedef struct {
    u32 pclk_khz, xres, yres;
    u32 hfp, hsw, hbp, vfp, vsw, vbp;
    u32 href, vref;
} dpout_mode_t;

static const dpout_mode_t s_mode = { 74250, 1280, 720, 110, 40, 220, 5, 5, 20, 1, 1 };

/* The link, as computed and as trained. */
typedef struct {
    u8  lanes, bw;            /* bw: 6 or 10, the DPCD code               */
    bool enhanced, tps3;
    u8  aux_rd_interval;      /* DPCD 00E, units of 4 ms                  */
    u32 tu, active_count, active_frac, watermark;
    bool active_polarity;
    u32 hblank_sym, vblank_sym;
    u8  vs[4], pe[4], pc[4];
    u8  vs_floor;             /* the swing training starts from (margin retries) */
} dpout_link_t;

static int _link_calc(dpout_link_t *l, const dpout_mode_t *m)
{
    const u64 f = 100000;
    const u32 link_rate = 27 * l->bw * 1000 * 1000;
    const u32 bpp = 24;
    u64 ratio_f, lowest_err = 64 * f, watermark_f;
    u32 num_linkclk_line, best_tu = 64, best_count = 0, best_frac = 0, best_pol = 0;

    if ((u64)m->pclk_khz * 1000 * bpp >= (u64)link_rate * 8 * l->lanes)
        return 1;
    num_linkclk_line = (u32)(((u64)link_rate * m->xres) / (m->pclk_khz * 1000));
    ratio_f = (u64)m->pclk_khz * 1000 * bpp * f / 8;
    ratio_f = ratio_f / (link_rate * l->lanes);

    for (int i = 64; i >= 32; i--) {
        u64 activesym_f = ratio_f * i;
        u64 activecount_f = (activesym_f / f) * f;
        u64 frac_f = activesym_f - activecount_f;
        u32 activecount = (u32)(activecount_f / f);
        u32 polarity, activefrac = 0;
        u64 approx_f, err_f;
        bool neg;

        if (frac_f < f / 2) {
            polarity = 0;
        } else {
            polarity = 1;
            frac_f = f - frac_f;
        }
        if (frac_f != 0) {
            frac_f = (f * f) / frac_f;
            if (frac_f > 15 * f)
                activefrac = polarity ? 1 : 15;
            else
                activefrac = polarity ? (u32)(frac_f / f) + 1 : (u32)(frac_f / f);
        }
        if (activefrac == 1)
            polarity = 0;
        if (polarity == 1)
            approx_f = activefrac ? (activecount_f + (activefrac * f - f) * f) / (activefrac * f)
                                  : activecount_f + f;
        else
            approx_f = activefrac ? activecount_f + f / activefrac : activecount_f;
        if (activesym_f < approx_f) {
            err_f = num_linkclk_line * ((approx_f - activesym_f) / i);
            neg = true;
        } else {
            err_f = num_linkclk_line * ((activesym_f - approx_f) / i);
            neg = false;
        }
        if ((neg && lowest_err > err_f) || err_f == 0) {
            lowest_err = err_f;
            best_tu = i;
            best_count = activecount;
            best_pol = polarity;
            best_frac = activefrac;
            if (err_f == 0)
                break;
        }
    }
    if (best_frac == 0) {
        l->active_polarity = false;
        l->active_count = best_pol ? best_count : best_count - 1;
        l->active_frac = 1;
    } else {
        l->active_polarity = best_pol != 0;
        l->active_count = best_count;
        l->active_frac = best_frac;
    }
    l->tu = best_tu;
    watermark_f = (ratio_f * l->tu * (f - ratio_f)) / f;
    l->watermark = (u32)((watermark_f + lowest_err) / f) + bpp / 4 - 1;
    if (l->watermark > 30)
        l->watermark = 30;
    {
        u32 per_line = (m->xres * bpp) / (8 * l->lanes);
        if (l->watermark > per_line)
            l->watermark = per_line;
    }
    {
        int h = (int)(((u64)(m->hbp + m->hfp + m->hsw - 7) * link_rate) / (m->pclk_khz * 1000))
                - 3 * (l->enhanced ? 1 : 0) - (12 / l->lanes);
        int vb = (int)(((u64)(m->xres - 25) * link_rate) / (m->pclk_khz * 1000)) - (36 / l->lanes) - 4;
        l->hblank_sym = h < 0 ? 0 : (u32)h;
        l->vblank_sym = vb < 0 ? 0 : (u32)vb;
    }
    return 0;
}
int main(void){dpout_link_t l={0};l.lanes=2;l.enhanced=true;
 u8 rates[]={0x14,0x0A,6};for(u32 i=0;i<3;i++){l.bw=rates[i];C(!_link_calc(&l,&s_mode));C(l.tu>=32&&l.tu<=64&&l.active_count<l.tu&&l.watermark<=30);}
 l.lanes=1;l.bw=6;C(_link_calc(&l,&s_mode)!=0);
 printf("PASS: %u real 720p60 link-budget/TU assertions at all fallback rates.\n",checks);
}
