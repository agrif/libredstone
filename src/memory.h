#ifndef __RS_MEMORY_H_INCLUDED__
#define __RS_MEMORY_H_INCLUDED__

#include <stdlib.h>

/* basic memory management hooks / functions */

/* some typedefs, always take passed struct as first */
typedef void* (*RSMalloc)(void*, size_t);
typedef void* (*RSRealloc)(void*, void*, size_t);
typedef void (*RSFree)(void*, void*);

/* memory management vtable */
typedef struct
{
    /* these two are required */
    RSMalloc malloc;
    RSFree free;
    RSRealloc realloc;

    /* may be null -- malloc and memset will be used instead */
    RSMalloc malloc0;
} RSMemoryFunctions;

void rs_set_memory_functions(RSMemoryFunctions* funcs);

/* core functions */
void* rs_malloc(size_t size);
void* rs_malloc0(size_t size);
void* rs_realloc(void* ptr, size_t size);
void rs_free(void* ptr);

/* convenience functions */

#define rs_new(type, n) ((type*)rs_malloc(sizeof(type) * (n)))
#define rs_new0(type, n) ((type*)rs_malloc0(sizeof(type) * (n)))
#define rs_renew(type, mem, n) ((type*)rs_realloc((mem), sizeof(type) * (n)))

void* rs_memdup(const void* ptr, size_t size);

#endif /* __RS_MEMORY_H_INCLUDED__ */
