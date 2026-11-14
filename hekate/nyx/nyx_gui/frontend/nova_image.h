/* Bounded PNG decoding and alpha-preserving scaling. GPL-2.0-or-later. */
#ifndef NOVA_IMAGE_H
#define NOVA_IMAGE_H
#include <utils/types.h>
/* Returns an owned BGRA32 buffer, suitable for LV_IMG_CF_TRUE_COLOR_ALPHA.
 * cover crops to fill; otherwise letterbox to fit. No hardware or SD I/O. */
bool nova_image_png(const u8 *data,u32 bytes,u32 width,u32 height,bool cover,u32 **pixels);
#endif
