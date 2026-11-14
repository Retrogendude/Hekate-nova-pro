/* Nyx <-> isolated RAM-service contract. The entire first 4 MiB is excluded
 * from the destructive test. This header and return payload live in its tail. */
#ifndef NYX_RAM_HANDOFF_H
#define NYX_RAM_HANDOFF_H
#include <utils/types.h>
#define NYX_RAM_UI_ADDR      0x80000000u
#define NYX_RAM_UI_MAX       0x00300000u
#define NYX_RAM_UI_MAGIC     0x4955524Eu
#define NYX_RAM_HANDOFF_ADDR 0x803DF000u
#define NYX_RAM_RETURN_ADDR  0x803E0000u
#define NYX_RAM_RETURN_MAX   0x00020000u
#define NYX_RAM_HANDOFF_MAGIC 0x524D594Eu /* NYMR */
#define NYX_RAM_RESULT_MAGIC  0x5253594Eu /* NYSR */
#define NYX_RAM_VERSION 2u
#define NYX_RAM_MODE_QUICK 0u
#define NYX_RAM_MODE_ADVANCED 1u
#define NYX_RAM_MODE_CUSTOM 2u
#define NYX_RAM_PATTERN_ALL 0x7Fu
#define NYX_RAM_RESULT_WORDS 12u
#define NYX_RAM_RESULT_PASS 1u
#define NYX_RAM_RESULT_FAIL 2u
#define NYX_RAM_RESULT_ABORT 3u
#define NYX_RAM_RESULT_ERROR 4u
typedef struct {
    u32 magic;
    u32 version;
    u32 payload_size;
    u32 payload_hash;
    u32 auto_run;
    u32 result_magic;
    u32 result;
    u32 errors;
    u32 patterns_completed;
    u32 patterns_total;
    u32 tested_mib;
    u32 dram_mib;
    u32 elapsed_ms;
    u32 mtc_errors;
    u32 mode;
    u32 selected_mask;
    u32 completed_mask;
    u32 verified_mib; /* Read back with at least one selected pattern. */
    u32 result_hash;
    u32 request_hash;
} nyx_ram_handoff_t;
static inline u32 nyx_ram_hash(const void *data, u32 size)
{
    const u8 *p = (const u8 *)data;
    u32 hash = 2166136261u;
    while (size--) hash = (hash ^ *p++) * 16777619u;
    return hash;
}
static inline u32 nyx_ram_count(u32 mask)
{u32 count=0;while(mask){count+=mask&1u;mask>>=1;}return count;}
static inline bool nyx_ram_selection_valid(u32 mode,u32 mask)
{
    if(mode>NYX_RAM_MODE_CUSTOM||!mask||(mask&~NYX_RAM_PATTERN_ALL))return false;
    return mode==NYX_RAM_MODE_QUICK?mask==1u:mode==NYX_RAM_MODE_ADVANCED?mask==NYX_RAM_PATTERN_ALL:true;
}
static inline u32 nyx_ram_request_hash(const nyx_ram_handoff_t *h)
{
    const u32 words[]={h->version,h->payload_size,h->payload_hash,h->auto_run,h->mode,h->selected_mask};
    return nyx_ram_hash(words,sizeof(words));
}
static inline bool nyx_ram_request_valid(const nyx_ram_handoff_t *h)
{
    return h->magic==NYX_RAM_HANDOFF_MAGIC&&h->version==NYX_RAM_VERSION&&h->auto_run==1u&&
        h->payload_size>=0x100u&&h->payload_size<=NYX_RAM_RETURN_MAX&&
        nyx_ram_selection_valid(h->mode,h->selected_mask)&&h->request_hash==nyx_ram_request_hash(h);
}
static inline bool nyx_ram_result_valid(const nyx_ram_handoff_t *h)
{
    if(h->version!=NYX_RAM_VERSION||h->result_magic!=NYX_RAM_RESULT_MAGIC||
        h->result<NYX_RAM_RESULT_PASS||h->result>NYX_RAM_RESULT_ERROR||
        !nyx_ram_selection_valid(h->mode,h->selected_mask)||
        (h->completed_mask&~h->selected_mask)||h->patterns_total!=nyx_ram_count(h->selected_mask)||
        h->patterns_completed!=nyx_ram_count(h->completed_mask)||h->tested_mib>h->dram_mib||h->verified_mib>h->tested_mib||
        h->result_hash!=nyx_ram_hash(&h->result,NYX_RAM_RESULT_WORDS*sizeof(u32)))return false;
    return h->result!=NYX_RAM_RESULT_PASS||(!h->errors&&!h->mtc_errors&&h->tested_mib&&h->verified_mib==h->tested_mib&&h->completed_mask==h->selected_mask);
}
#endif
