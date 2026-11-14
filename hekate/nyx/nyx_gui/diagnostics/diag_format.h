/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef NYX_DIAG_FORMAT_H
#define NYX_DIAG_FORMAT_H
#include <utils/types.h>
#include <stdarg.h>
void nyx_diag_vsnprintf(char *out, u32 capacity, const char *format, va_list arguments);
void nyx_diag_snprintf(char *out, u32 capacity, const char *format, ...);
#endif
