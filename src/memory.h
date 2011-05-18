/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_MEMORY_H_INCLUDED__
#define __RS_MEMORY_H_INCLUDED__

/**
 * \defgroup memory Memory Management
 *
 * Functions for allocating, reallocating, and freeing memory.
 *
 * @{
 */

#include <stdlib.h>
#include <string.h>

/* basic memory management hooks / functions */

/** malloc function type */
typedef void* (*RSMalloc)(void*, size_t);
/** realloc function type */
typedef void* (*RSRealloc)(void*, void*, size_t);
/** free function type */
typedef void (*RSFree)(void*, void*);

/** Memory management vtable.
 *
 * You can tell libredstone what memory management functions to use by
 * populating a struct like this. All of the functions provided must
 * accept a pointer to the struct they are listed in as the first
 * argument; through this, you can pass in whatever data you
 * want. Apart from this extra first argument, these functions should
 * conform to the usual specs.
 *
 * \sa rs_set_memory_functions
 */
typedef struct
{
    /** required -- malloc replacement */
    RSMalloc malloc;
    /** required -- free replacement */
    RSFree free;
    /** required -- realloc replacement */
    RSRealloc realloc;

    /** optional -- a malloc that returns a zero-filled buffer */
    RSMalloc malloc0;
} RSMemoryFunctions;

/** Set the memory management vtable.
 *
 * Use this to tell libredstone what memory functions to use. See
 * RSMemoryFunctions for more information.
 *
 * \param funcs the vtable to use.
 * \sa RSMemoryFunctions
 */
void rs_set_memory_functions(RSMemoryFunctions* funcs);

/** A safer malloc.
 *
 * This function uses the user-provided memory functions, and will
 * automatically check the return value for you. It will never return
 * NULL.
 *
 * You should generally use rs_new instead.
 *
 * \param size the amount of memory to allocate.
 * \return the allocated memory.
 * \sa rs_malloc0, rs_free, rs_new
 */
void* rs_malloc(size_t size);

/** A malloc that returns zero-filled buffers.
 *
 * This function is exactly lie rs_malloc, but the buffer it returns
 * is already filled with zeros.
 *
 * You should generally use rs_new0 instead.
 *
 * \param size the amount of memory to allocate.
 * \return the allocated memory, filled with zeros.
 * \sa rs_malloc, rs_free, rs_new0
 */
void* rs_malloc0(size_t size);

/** A safer realloc.
 *
 * This function uses the user-provided memory functions, and will
 * automatically check the return value for you. It will never return
 * NULL.
 *
 * If you pass a NULL pointer as the original memory, this will just
 * act like a call to rs_malloc.
 *
 * You should generally use rs_renew instead.
 *
 * \param ptr the memory to reallocate.
 * \param size the amount of memory to allocate.
 * \sa rs_malloc, rs_free, rs_renew
 */
void* rs_realloc(void* ptr, size_t size);

/** A safer free.
 *
 * This function uses the user-provided memory functions, and will do nothing if the pointer provided is NULL.
 *
 * \param ptr the memory to free.
 * \sa rs_malloc
 */
void rs_free(void* ptr);

/** Dynamic array allocator.
 *
 * This function uses rs_malloc to create a dynamic array with the
 * given type and length, and automatically casts it to the correct
 * pointer type. You should use this instead of rs_malloc, most of the
 * time.
 *
 * \param type the type to allocate for.
 * \param n the number of elements to allocate.
 * \return the allocated memory.
 * \sa rs_free, rs_new0
 */
#define rs_new(type, n) ((type*)rs_malloc(sizeof(type) * (n)))

/** Dynamic array allocator (zero-initialized).
 *
 * This function uses rs_malloc0 to create a dynamic array with the
 * given type and length, and automatically casts it to the correct
 * pointer type. You should use this instead of rs_malloc0, most of the
 * time.
 *
 * \param type the type to allocate for.
 * \param n the number of elements to allocate.
 * \return the allocated memory (filled with zeros).
 * \sa rs_free, rs_new
 */
#define rs_new0(type, n) ((type*)rs_malloc0(sizeof(type) * (n)))

/** Dynamic array reallocator.
 *
 * This function uses rs_realloc to resize a dynamic array returned by
 * rs_new. You should generally use this instead of rs_realloc.
 *
 * \param type the type to allocate for.
 * \param mem the old memory to reallocate.
 * \param n the number of elements to allocate.
 * \return the allocated memory.
 * \sa rs_free, rs_new
 */
#define rs_renew(type, mem, n) ((type*)rs_realloc((mem), sizeof(type) * (n)))

/** Convenience memory duplicator.
 *
 * This function will duplicate the memory given, and return the
 * duplicate as a freshly-allocated array that must be freed with
 * rs_free.
 *
 * \param ptr the memory to duplicate.
 * \param size the number of bytes to duplicate.
 * \return the duplicated memory.
 * \sa rs_free, rs_strdup
 */
void* rs_memdup(const void* ptr, size_t size);

/** Convenience string duplicator.
 *
 * This function duplicates the given string (using rs_memdup) and
 * returns the duplicate as a freshly-allocated chunk of memory that
 * must be freed with rs_free.
 *
 * \param str the string to duplicate.
 * \return the duplicated string.
 * \sa rs_free, rs_memdup
 */
#define rs_strdup(str) ((char*)rs_memdup((str), strlen(str) + 1))

/** @} */
#endif /* __RS_MEMORY_H_INCLUDED__ */
