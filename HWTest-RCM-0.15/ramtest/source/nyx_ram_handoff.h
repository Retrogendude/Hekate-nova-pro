/* Nyx <-> isolated RAM-service contract. The entire first 4 MiB is excluded
 * from the destructive test. This header and return payload live in its tail. */
#ifndef NYX_RAM_HANDOFF_H
#define NYX_RAM_HANDOFF_H
#include <utils/types.h>
#define NYX_RAM_HANDOFF_ADDR 0x803DF000u
#define NYX_RAM_RETURN_ADDR  0x803E0000u
#define NYX_RAM_RETURN_MAX   0x00020000u
#define NYX_RAM_HANDOFF_MAGIC 0x524D594Eu /* NYMR */
#define NYX_RAM_RESULT_MAGIC  0x5253594Eu /* NYSR */
#define NYX_RAM_VERSION 1u
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
    u32 result_hash;
} nyx_ram_handoff_t;
static inline u32 nyx_ram_hash(const void *data, u32 size)
{
    const u8 *p = (const u8 *)data;
    u32 hash = 2166136261u;
    while (size--) hash = (hash ^ *p++) * 16777619u;
    return hash;
}
#endif
