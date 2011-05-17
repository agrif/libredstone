/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "memory.h"
#include "error.h"

#include <string.h>

static RSMemoryFunctions* memfuncs = NULL;

void rs_set_memory_functions(RSMemoryFunctions* funcs)
{
    if (funcs)
    {
        rs_assert(funcs->malloc);
        rs_assert(funcs->free);
        rs_assert(funcs->realloc);
    }
    
    memfuncs = funcs;
}

void* rs_malloc(size_t size)
{
    void* ret = NULL;

    if (memfuncs)
    {
        ret = memfuncs->malloc(memfuncs, size);
    } else {
        ret = malloc(size);
    }
    
    rs_assert(ret);
    return ret;
}

void rs_free(void* ptr)
{
    if (!ptr)
        return;
    
    if (memfuncs)
    {
        memfuncs->free(memfuncs, ptr);
    } else {
        free(ptr);
    }
}

void* rs_realloc(void* ptr, size_t size)
{
    void* ret = NULL;
    
    if (memfuncs)
    {
        ret = memfuncs->realloc(memfuncs, ptr, size);
    } else {
        ret = realloc(ptr, size);
    }
    
    rs_assert(ret);
    return ret;
}

void* rs_malloc0(size_t size)
{
    void* ret = NULL;
    
    if (memfuncs)
    {
        if (memfuncs->malloc0)
        {
            ret = memfuncs->malloc0(memfuncs, size);
        } else {
            ret = memset(rs_malloc(size), 0, size);
        }
    } else {
        ret = calloc(size, 1);
    }
    
    rs_assert(ret);
    return ret;
}           

void* rs_memdup(const void* ptr, size_t size)
{
    void* ret;
    if (!ptr)
        return NULL;
    
    ret = rs_malloc(size);
    return memcpy(ret, ptr, size);
}
