/* Nyx adapter for probes from the supplied HWTest-RCM 0.15 archive. */
#ifndef DIAGNOSTIC_BRIDGE_H
#define DIAGNOSTIC_BRIDGE_H
#include <bdk.h>
#include <string.h>
#include <stdarg.h>
#define COL_DEFAULT 0
#define COL_OK 1
#define COL_WARN 2
#define COL_ERR 3
#define COL_INFO 4
typedef enum { DX_PASS, DX_WARN, DX_FAIL } dx_sev_t;
void diag_log(u32 color, const char *fmt, ...);
void dx_set(const char *key, dx_sev_t sev, const char *fmt, ...);
#define LOG(...) diag_log(COL_DEFAULT, __VA_ARGS__)
#define log_color(...) diag_log(__VA_ARGS__)
#define HEADER(s) diag_log(COL_INFO, "%s\n", s)
void probe_display(void);
void probe_backlight(void);
void probe_dram(void);
#endif
