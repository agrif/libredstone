#include "region.h"

#include "util.h"
#include "memory.h"
#include "endian.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>

/* This implemention of Minecraft's region format is based on info from
 * <http://www.minecraftwiki.net/wiki/Beta_Level_Format>.
 */

#pragma pack(1)
struct ChunkLocation
{
    uint32_t offset:24;
    uint8_t sector_count;
};
#pragma pack()

/* overall region info */
struct _RSRegion
{
    char* path;
    int fd;
    off_t fsize;
    void* map;
    
    struct ChunkLocation* locations;
    uint32_t* timestamps;
};

RSRegion* rs_region_open(const char* path)
{
    RSRegion* self;
    struct stat stat_buf;
    void* map = NULL;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return NULL;
    
    assert(fstat(fd, &stat_buf) >= 0);
    
    /* zero size is valid, but anything between 0 and 8192 isn't */
    if (stat_buf.st_size > 0 && stat_buf.st_size < 8192)
    {
        close(fd);
        return NULL;
    }
    
    if (stat_buf.st_size > 0)
    {
        map = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED)
        {
            close(fd);
            return NULL;
        }
    }
    
    self = rs_new0(RSRegion, 1);
    self->path = rs_strdup(path);
    self->fd = fd;    
    self->fsize = stat_buf.st_size;
    self->map = map;
    
    self->locations = NULL;
    self->timestamps = NULL;
    if (self->map)
    {
        self->locations = (struct ChunkLocation*)(self->map);
        self->timestamps = (uint32_t*)(self->map + 4096);
    }
    
    return self;
}

void rs_region_close(RSRegion* self)
{
    rs_assert(self);
    
    rs_free(self->path);
    if (self->map)
        munmap(self->map, self->fsize);
    close(self->fd);
    rs_free(self);
}

uint32_t rs_region_get_chunk_timestamp(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_assert(self);
    if (self->timestamps == NULL || x >= 32 || z >= 32)
        return 0;
    
    return rs_endian_uint32(self->timestamps[x + 32*z]);
}

/* LOCAL helper function to return the start of chunk data, including
 * size, compression
 */
static void* _rs_region_get_data(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_assert(self);
    if (self->locations == NULL || x >= 32 || z >= 32)
        return NULL;
    
    return self->map + (rs_endian_uint24(self->locations[x + z*32].offset) * 4096);
}

uint32_t rs_region_get_chunk_size(RSRegion* self, uint8_t x, uint8_t z)
{
    uint32_t* size_int = (uint32_t*)_rs_region_get_data(self, x, z);
    if (!size_int)
        return 0;
    
    /* size is big-endian, and 1 larger than it should be */
    return rs_endian_uint32(size_int[0]) - 1;
}

RSCompressionType rs_region_get_chunk_compression(RSRegion* self, uint8_t x, uint8_t z)
{
    uint8_t* compression_byte = (uint8_t*)_rs_region_get_data(self, x, z);
    if (!compression_byte)
        return RS_UNKNOWN_COMPRESSION;
    
    /* compression byte is the fifth byte */
    switch (compression_byte[4])
    {
    case 1:
        return RS_GZIP;
    case 2:
        return RS_ZLIB;
    default:
        break;
    };
    
    return RS_UNKNOWN_COMPRESSION;
}

/* valid until region is closed */
void* rs_region_get_chunk_data(RSRegion* self, uint8_t x, uint8_t z)
{
    void* ret = _rs_region_get_data(self, x, z);
    if (!ret)
        return NULL;
    
    /* chunk data starts 5 bytes after */
    return ret + 5;
}
