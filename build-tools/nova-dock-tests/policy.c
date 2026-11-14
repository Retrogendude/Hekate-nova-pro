#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <display/tv/usbpd.h>
typedef uint64_t u64;
typedef struct {
    u32 pclk_khz, xres, yres;
    u32 hfp, hsw, hbp, vfp, vsw, vbp;
    u32 href, vref;
} dpout_mode_t;
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
static void _text(const u8 *in, char *out, u32 n)
{
    for (u32 i = 0; i < n; i++)
        out[i] = (in[i] >= 0x20 && in[i] < 0x7F) ? (char)in[i] : '.';
    out[n] = 0;
}
#define DOCK_ID_MV          5000
#define DOCK_ID_MA          500
#define DOCK_MV             15000
#define DOCK_MA_MIN         2600
#define DOCK_MA_MAX         3000
static void _pdo_decode(u32 pdo, u32 *mv, u32 *ma)
{
    if (pdo >> 30) { // This handshake supports fixed PDOs only.
        *mv = *ma = 0;
        return;
    }
    *ma = (pdo & 0x3FF) * 10;
    *mv = ((pdo >> 10) & 0x3FF) * 50;
}
static bool _choose(dock_t *d)
{
    d->pick = 0;
    if (d->pdo_n == 2 && d->pdo_mv[0] == DOCK_ID_MV && d->pdo_ma[0] == DOCK_ID_MA) {
        if (d->pdo_mv[1] == DOCK_MV && d->pdo_ma[1] >= DOCK_MA_MIN && d->pdo_ma[1] <= DOCK_MA_MAX) {
            d->dock_power = true;
            d->pick = 2;
            return true;
        }
        return false;
    }
    u32 best = 0;
    for (u32 i = 0; i < d->pdo_n; i++) {
        u32 mv = d->pdo_mv[i], ma = d->pdo_ma[i];
        if (mv != 5000 && mv != 9000 && mv != 12000 && mv != 15000)
            continue;
        if (ma < 100 || ma > 3000)
            continue;
        if (mv * ma > best || (!d->pick && mv == 5000)) {
            best = mv * ma;
            d->pick = i + 1;
        }
    }
    return d->pick != 0;
}
static u32 _op_ma(u32 mv, u32 ma)
{
    static const u32 steps[] = { 100, 150, 500, 900, 1200, 1500, 2000, 3000 };
    if (!mv || !ma)
        return 0;
    u32 reserve = (mv > 5000 ? 4500000 : 2500000) / mv;
    u32 limit = ma > reserve ? ma - reserve : 0;
    u32 cap = mv >= 15000 ? 1200 : mv == 12000 ? 1500 : 2000;
    if (limit > cap)
        limit = cap;
    for (int i = 7; i >= 0; i--)
        if (limit >= steps[i])
            return steps[i];
    return ma < 100 ? ma : 100;
}
int main(void) {
 unsigned tests=0;
 const dpout_mode_t mode={74250,1280,720,110,40,220,5,5,20,1,1};
 const u8 rates[]={6,10,20};
 for(unsigned lanes=1;lanes<=4;lanes*=2) for(unsigned ri=0;ri<3;ri++) {
   dpout_link_t l={0};l.lanes=lanes;l.bw=rates[ri];
   bool enough=(uint64_t)74250000*24<(uint64_t)27*l.bw*1000000*8*lanes;
   assert((_link_calc(&l,&mode)==0)==enough);
   if(enough) {assert(l.tu>=32&&l.tu<=64);assert(l.active_frac>=1&&l.active_frac<=15);assert(l.active_count<l.tu);assert(l.watermark<=30);}
   tests++;
 }
 unsigned volts[]={5000,9000,12000,15000};
 for(unsigned v=0;v<4;v++) for(unsigned ma=0;ma<=3000;ma++) {
   u32 ask=_op_ma(volts[v],ma);assert(ask<=ma);
   assert(ask<=(volts[v]==15000?1200:volts[v]==12000?1500:2000));tests++;
 }
 u32 mv,ma;_pdo_decode(((15000/50)<<10)|(3000/10),&mv,&ma);assert(mv==15000&&ma==3000);tests++;
 for(unsigned t=1;t<=3;t++){_pdo_decode((t<<30)|0x3fffffff,&mv,&ma);assert(!mv&&!ma);tests++;}
 dock_t d={0};d.pdo_n=2;d.pdo_mv[0]=5000;d.pdo_ma[0]=500;d.pdo_mv[1]=15000;d.pdo_ma[1]=3000;
 assert(_choose(&d)&&d.pick==2&&d.dock_power);tests++;
 d.pdo_mv[1]=20000;assert(!_choose(&d));tests++;
 memset(&d,0,sizeof(d));d.pdo_n=3;d.pdo_mv[0]=5000;d.pdo_ma[0]=3000;d.pdo_mv[1]=20000;d.pdo_ma[1]=3000;d.pdo_mv[2]=9000;d.pdo_ma[2]=2000;
 assert(_choose(&d)&&d.pick==3);tests++;
 memset(&d,0,sizeof(d));d.pdo_n=1;d.pdo_mv[0]=5000;assert(!_choose(&d));tests++;
 const u8 raw[]={0,31,32,65,126,127,255};char text[9];memset(text,'x',sizeof(text));_text(raw,text,7);
 assert(!strcmp(text,".. A~..")&&text[8]=='x');tests++;
 printf("PASS: %u power-profile / link-calculation / text cases (no hardware).\n",tests);
}
