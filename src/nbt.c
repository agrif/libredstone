/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "nbt.h"

#include "util.h"
#include "memory.h"
#include "endian.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

/* This implemention of Minecraft's NBT format is based on info from
 * <http://www.minecraft.net/docs/NBT.txt>.
 */

struct _RSNBT
{
    int dummy;
};

RSNBT* rs_nbt_open(const char* path)
{
    RSNBT* self;
    struct stat stat_buf;
    void* map = NULL;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        return NULL;
    }
    
    if (fstat(fd, &stat_buf) < 0)
    {
        rs_assert(false); /* stat failed */
    }
    
    if (stat_buf.st_size <= 0)
    {
        close(fd);
        return NULL;
    }
    
    map = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        return NULL;
    }
    
    RSCompressionType enc = rs_get_compression_type(map, stat_buf.st_size);
    self = rs_nbt_parse(map, stat_buf.st_size, enc);
    
    munmap(map, stat_buf.st_size);
    return self;
}

RSNBT* rs_nbt_parse_from_region(RSRegion* region, uint8_t x, uint8_t z)
{
    rs_assert(region);
    void* data = rs_region_get_chunk_data(region, x, z);
    uint32_t  len = rs_region_get_chunk_length(region, x, z);
    RSCompressionType enc = rs_region_get_chunk_compression(region, x, z);
    
    if (!data || len == 0)
        return NULL;
    
    return rs_nbt_parse(data, len, enc);
}

RSNBT* rs_nbt_parse(void* data, uint32_t len, RSCompressionType enc)
{
    RSNBT* self = rs_new(RSNBT, 1);
    return self;
}

void rs_nbt_free(RSNBT* self)
{
    rs_free(self);
}
