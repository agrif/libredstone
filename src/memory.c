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
        rs_return_if_fail(funcs->malloc);
        rs_return_if_fail(funcs->free);
        rs_return_if_fail(funcs->realloc);
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
    
    if (!ret)
        rs_error("out of memory");
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
    
    if (ptr == NULL)
        return rs_malloc(size);
    
    if (memfuncs)
    {
        ret = memfuncs->realloc(memfuncs, ptr, size);
    } else {
        ret = realloc(ptr, size);
    }
    
    if (!ret)
        rs_error("out of memory");
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
    
    if (!ret)
        rs_error("out of memory");
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
