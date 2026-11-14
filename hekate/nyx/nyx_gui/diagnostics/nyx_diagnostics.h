/* Native Nyx host for the supplied HWTest-RCM 0.15 diagnostics. */
#ifndef NYX_DIAGNOSTICS_H
#define NYX_DIAGNOSTICS_H
#include <bdk.h>

#define NYX_DIAG_G_IDENTITY (1u << 0)
#define NYX_DIAG_G_POWER    (1u << 1)
#define NYX_DIAG_G_MEMORY   (1u << 2)
#define NYX_DIAG_G_STORAGE  (1u << 3)
#define NYX_DIAG_G_WIRELESS (1u << 4)
#define NYX_DIAG_G_AUDIO    (1u << 5)
#define NYX_DIAG_G_SOUND    (1u << 6)
#define NYX_DIAG_G_DISPLAY  (1u << 7)
#define NYX_DIAG_G_INPUTS   (1u << 8)
#define NYX_DIAG_G_RAW      (1u << 9)
#define NYX_DIAG_G_ALL      ((1u << 10) - 1)
#define NYX_DIAG_PROBE_COUNT 42

typedef enum {
    NYX_DIAG_NOT_RUN, NYX_DIAG_OBSERVATION, NYX_DIAG_PASS,
    NYX_DIAG_WARN, NYX_DIAG_FAIL, NYX_DIAG_ERROR
} nyx_diag_state_t;
typedef struct {
    const char *name;
    const char *function;
    u32 group;
    nyx_diag_state_t state;
    u32 report_offset;
    u32 report_length;
} nyx_diag_probe_t;
typedef struct {
    const char *key;
    u32 severity; /* HWTest registry: 0 PASS, 1 WARN, 2 FAIL. */
    char detail[48];
} nyx_diag_finding_t;
typedef struct {
    /* Must suspend input polling and hardware maintenance before a probe.
     * Progress may call lv_refr_now(), never the normal LVGL task loop. */
    void (*suspend)(u32 groups, void *context);
    void (*progress)(u32 current, u32 total, const char *message, void *context);
    void (*resume)(u32 groups, void *context);
    void *context;
} nyx_diag_hooks_t;

const nyx_diag_probe_t *nyx_diag_probe(u32 index);
/* Return 0 for a completed execution, even if hardware findings FAIL.
 * 1 busy; 2 memory allocation; 3 invalid selection; 4 restoration issue. */
int nyx_diag_run(u32 groups, const nyx_diag_hooks_t *hooks);
int nyx_diag_run_probe(u32 index, const nyx_diag_hooks_t *hooks);
bool nyx_diag_running(void);
bool nyx_diag_truncated(void);
const char *nyx_diag_report(void);
u32 nyx_diag_finding_count(void);
bool nyx_diag_finding(u32 index, nyx_diag_finding_t *out);
bool nyx_diag_storage_ready(bool emmc);
/* Exports only the captured report, never reruns probes. Output path is
 * populated on success. Requires 128 bytes at path, may be NULL. */
int nyx_diag_export(char *path);
/* Normal runs call cleanup before resume. Available to the host as well. */
void nyx_diag_cleanup(void);
#endif
