/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_MMAP_H_INCLUDED__
#define __RS_MMAP_H_INCLUDED__

#include <sys/types.h>

/* include this instead of mman.h, since it may not exist on some systems */

#define PROT_NONE       0
#define PROT_READ       1
#define PROT_WRITE      2

#define MAP_FILE        0
#define MAP_SHARED      1

#define MAP_FAILED      ((void *)-1)

/* Flags for msync. */
#define MS_SYNC         2

void*   mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int     munmap(void *addr, size_t len);
int     msync(void *addr, size_t len, int flags);

#endif /* __RS_MMAP_H_INCLUDED__ */
