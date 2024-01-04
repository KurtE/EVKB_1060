// External  SDRAM memory allocation functions.  Attempt to use external memory,
// but automatically fall back to internal RAM if external RAM can't be used.

#include <stdlib.h>
#include "smalloc.h"
#include "wiring.h"

#if defined(ARDUINO_TEENSY_MICROMOD)
// Teensy Micromod external RAM address range is 0x90000000 to 0x9FFFFFFF
#define HAS_SDRAM
#define IS_SDRAM(addr) (((uint32_t)addr >> 28) == 9)
#endif


void *sdram_malloc(size_t size)
{
#ifdef HAS_SDRAM
    void *ptr = sm_malloc_pool(&extsdram_smalloc_pool, size);
    if (ptr) return ptr;
#endif
    return malloc(size);
}

void sdram_free(void *ptr)
{
#ifdef HAS_SDRAM
    if (IS_SDRAM(ptr)) {
        sm_free_pool(&extsdram_smalloc_pool, ptr);
        return;
    }
#endif
    free(ptr);
}

void *sdram_calloc(size_t nmemb, size_t size)
{
#ifdef HAS_SDRAM
    // Note: It is assumed that the pool was created with do_zero set to true
    void *ptr = sm_malloc_pool(&extsdram_smalloc_pool, nmemb*size);
    if (ptr) return ptr;
#endif
    return calloc(nmemb, size);
}

void *sdram_realloc(void *ptr, size_t size)
{
#ifdef HAS_SDRAM
    if (IS_SDRAM(ptr)) {
        return sm_realloc_pool(&extsdram_smalloc_pool, ptr, size);
    }
#endif
    return realloc(ptr, size);
}
