/* Decoder-only adapter; upstream lodepng.c/h are unmodified, version 20261102.
 * Original: https://github.com/lvandeve/lodepng (Lode Vandevenne). */
#include "nova_png.h"
#ifdef NYX_LOAD_ADDR
#include <mem/heap.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
/* BDK has no realloc. Retain an allocation's length in an aligned prefix,
 * copy on growth, and leave the old allocation intact on failure. */
typedef struct {size_t size;} png_allocation;
void *lodepng_malloc(size_t size)
{
    if(!size||size>LODEPNG_MAX_ALLOC)return NULL;
    png_allocation *allocation=malloc(sizeof(*allocation)+size);
    if(!allocation)return NULL;
    allocation->size=size;return allocation+1;
}
void lodepng_free(void *pointer)
{if(pointer)free((png_allocation *)pointer-1);}
void *lodepng_realloc(void *pointer,size_t size)
{
    if(!pointer)return lodepng_malloc(size);
    if(!size){lodepng_free(pointer);return NULL;}
    png_allocation *allocation=(png_allocation *)pointer-1;
    if(size<=allocation->size)return pointer;
    void *replacement=lodepng_malloc(size);if(!replacement)return NULL;
    memcpy(replacement,pointer,allocation->size);lodepng_free(pointer);return replacement;
}
#include "lodepng.c"
