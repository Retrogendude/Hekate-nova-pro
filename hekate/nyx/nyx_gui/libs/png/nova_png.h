/* Nyx decoder-only build configuration. GPL-2.0-or-later. */
#ifndef NOVA_PNG_H
#define NOVA_PNG_H
#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_ERROR_TEXT
#define LODEPNG_NO_COMPILE_ALLOCATORS
#define LODEPNG_MAX_ALLOC (20u*1024u*1024u)
#include "lodepng.h"
void *lodepng_malloc(size_t size);
void *lodepng_realloc(void *pointer,size_t size);
void lodepng_free(void *pointer);
#endif
