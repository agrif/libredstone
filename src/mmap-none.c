/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "config.h"
#ifdef MMAP_NONE
 
#include "error.h"
#include "memory.h"
#include "mmap.h"

#include <stdio.h>
#include <unistd.h>

void* mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	if (prot != PROT_READ)
		return MAP_FAILED;
	
	void* data = rs_malloc(len);
	if (!data)
		return MAP_FAILED;
	
	lseek(fildes, off, SEEK_SET);
	ssize_t amount_read = 0;
	while (amount_read < len)
	{
		ssize_t res = read(fildes, data + amount_read, len - amount_read);
		if (res <= 0)
		{
			rs_free(data);
			return MAP_FAILED;
		}
		
		amount_read += res;
		printf("read %i vs %i\n", amount_read, len);
	}
	
	return data;
}

int munmap(void *addr, size_t len)
{
	rs_free(addr);
	return 0;
}

int msync(void *addr, size_t len, int flags)
{
	return 0;
}

#endif /* MMAP_NONE */
