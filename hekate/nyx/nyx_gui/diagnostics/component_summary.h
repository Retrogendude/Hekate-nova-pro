/* Scoped diagnostic evidence, never a whole-chip health certificate. GPL-2.0-or-later. */
#ifndef NYX_COMPONENT_SUMMARY_H
#define NYX_COMPONENT_SUMMARY_H
#include "nyx_diagnostics.h"
enum {NYX_COMPONENT_NOT_TESTED,NYX_COMPONENT_PARTIAL,NYX_COMPONENT_PASS,NYX_COMPONENT_WARN,NYX_COMPONENT_ANOMALY,NYX_COMPONENT_TEST_ERROR};
typedef struct {
    const char *name,*scope,*hint;
    const char *keys[8];
    u8 probes[8];
    u8 key_count,required_count,probe_count;
    u8 storage; /* 0 none, 1 eMMC, 2 microSD. */
} nyx_component_definition;
typedef struct {
    const nyx_component_definition *definition;
    u32 state,found,required_found,executed;
    bool partial;
    char measurement[160];
} nyx_component_result;
enum {NYX_MANUAL_COMMANDS,NYX_MANUAL_TOUCH,NYX_MANUAL_DISPLAY,NYX_MANUAL_RAM,NYX_MANUAL_COUNT};
/* Last captured measurements in this Nyx session, separate from auto probes. */
void nyx_component_observation(u32 kind,const char *text,bool warning);
u32 nyx_component_count(void);
bool nyx_component_get(u32 index,nyx_component_result *out);
const char *nyx_component_state(u32 state);
char *nyx_component_summary_text(void); /* Owned allocation, includes justified leads. */
char *nyx_component_detail_text(u32 index); /* Owned allocation, evidence and scope. */
#endif
