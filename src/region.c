/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "region.h"

#include "util.h"
#include "memory.h"
#include "endian.h"
#include "list.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

/* This implemention of Minecraft's region format is based on info from
 * <http://www.minecraftwiki.net/wiki/Beta_Level_Format>.
 */

/* for chunk location table */
#pragma pack(1)
struct ChunkLocation
{
    uint32_t offset:24;
    uint8_t sector_count;
};
#pragma pack()

/* for cached chunk writes */
struct ChunkWrite
{
    uint8_t x, z;
    void* data;
    uint32_t length;
    RSCompressionType encoding;
    uint32_t timestamp;
};

/* overall region info */
struct _RSRegion
{
    char* path;
    bool write;
    int fd;
    off_t fsize;
    void* map;
    
    struct ChunkLocation* locations;
    uint32_t* timestamps;
    
    /* list of ChunkWrite structs */
    RSList* cached_writes;
};

RSRegion* rs_region_open(const char* path, bool write)
{
    RSRegion* self;
    struct stat stat_buf;
    void* map = NULL;
    int fd = open(path, write ? (O_RDWR | O_CREAT) : O_RDONLY, 0666);
    if (fd < 0)
    {
        return NULL;
    }
    
    if (fstat(fd, &stat_buf) < 0)
    {
        rs_assert(false); /* stat failed */
    }
    
    /* zero size is valid, but anything between 0 and 8192 isn't */
    if (stat_buf.st_size > 0 && stat_buf.st_size < 8192)
    {
        close(fd);
        return NULL;
    }
    
    if (stat_buf.st_size > 0)
    {
        map = mmap(NULL, stat_buf.st_size, write ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED)
        {
            close(fd);
            return NULL;
        }
    }
    
    self = rs_new0(RSRegion, 1);
    self->path = rs_strdup(path);
    self->write = write;
    self->fd = fd;    
    self->fsize = stat_buf.st_size;
    self->map = map;
    self->cached_writes = NULL;
    
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
    
    if (self->write && self->cached_writes)
        rs_region_flush(self);
    
    rs_list_free(self->cached_writes);
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

uint32_t rs_region_get_chunk_length(RSRegion* self, uint8_t x, uint8_t z)
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

/* valid until region is closed/flushed */
void* rs_region_get_chunk_data(RSRegion* self, uint8_t x, uint8_t z)
{
    void* ret = _rs_region_get_data(self, x, z);
    if (!ret)
        return NULL;
    
    /* chunk data starts 5 bytes after */
    return ret + 5;
}

void rs_region_set_chunk_data(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc)
{
    uint32_t timestamp = time(NULL);
    rs_region_set_chunk_data_full(self, x, z, data, len, enc, timestamp);
}

void rs_region_set_chunk_data_full(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc, uint32_t timestamp)
{
    rs_assert(self);
    
    if (self->write == false || x >= 32 || z >= 32)
        return;
    
    /* first, check if there's a cached write already, and clear it if
     * needed
     */
    RSList* cell = self->cached_writes;
    for (; cell != NULL; cell = cell->next)
    {
        struct ChunkWrite* write = cell->data;
        rs_assert(write);
        
        if (write->x == x && write->z == z)
            break;
    }
    
    if (cell)
    {
        rs_free(cell->data);
        self->cached_writes = rs_list_remove(self->cached_writes, cell);
    }
    
    /* now, create a new write struct */
    struct ChunkWrite* job = rs_new0(struct ChunkWrite, 1);
    job->x = x;
    job->z = z;
    job->data = data;
    job->length = len;
    job->encoding = enc;
    job->timestamp = timestamp;
    
    self->cached_writes = rs_list_push(self->cached_writes, job);
}

void rs_region_clear_chunk(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_region_set_chunk_data_full(self, x, z, NULL, 0, RS_UNKNOWN_COMPRESSION, 0);
}

/* writes are cached until this is called */
void rs_region_flush(RSRegion* self)
{
    rs_assert(self);
    
    if (self->write && self->cached_writes)
    {
        /* check to see if this is a brand-new file */
        if (self->map == NULL)
        {
            /* it is! so we have to write everything */
            
            /* first, figure out how big the file needs to be */
            uint32_t final_size = 0;
            RSList* cell = self->cached_writes;
            for (; cell != NULL; cell = cell->next)
            {
                struct ChunkWrite* write = cell->data;
                rs_assert(write);
                
                /* be sure to account for extra size/compression info */
                final_size += write->length + 4 + 1;
                
                /* force size to be on sector boundaries */
                if (final_size % 4096 > 0)
                    final_size += (4096 - final_size % 4096);
            }
            
            /* add on 4096 * 2 to account for location/timestamp headers */
            final_size += 4096 * 2;
            
            rs_assert(final_size % 4096 == 0);
            
            /* resize the file, and remap */
            if (ftruncate(self->fd, final_size) < 0)
            {
                rs_assert(false); /* file resize failed */
            }
            self->fsize = final_size;
            self->map = mmap(NULL, final_size, PROT_READ | PROT_WRITE, MAP_SHARED, self->fd, 0);
            if (self->map == MAP_FAILED)
            {
                rs_assert(false); /* remap failed */
            }
            
            self->locations = (struct ChunkLocation*)(self->map);
            self->timestamps = (uint32_t*)(self->map + 4096);
            
            /* zero out the headers */
            memset(self->map, 0, 4096 * 2);
            
            /* now, we can iterate through the writes and copy them in */
            uint32_t cur_sector = 2;
            for (cell = self->cached_writes; cell != NULL; cell = cell->next)
            {
                struct ChunkWrite* write = cell->data;
                rs_assert(write);
                
                unsigned int i = write->x + write->z*32;
                
                /* handle chunk clears */
                if (write->length == 0)
                {
                    self->locations[i].offset = 0;
                    self->locations[i].sector_count = 0;
                    self->timestamps[i] = 0;
                    continue;
                }
                
                uint8_t sector_count = (write->length + 4 + 1) / 4096;
                if ((write->length + 4 + 1) % 4096 > 0)
                    sector_count++;
                
                self->locations[i].offset = rs_endian_uint24(cur_sector);
                self->locations[i].sector_count = sector_count;
                self->timestamps[i] = rs_endian_uint32(write->timestamp);
                
                /* convert compression types */
                uint8_t enc = 0;
                switch (write->encoding)
                {
                case RS_GZIP:
                    enc = 1; break;
                case RS_ZLIB:
                    enc = 2; break;
                default:
                    rs_assert(false); /* unhandled compression type */
                };
                
                /* write the pre-data header (carefully) */
                void* dest = _rs_region_get_data(self, write->x, write->z);
                ((uint32_t*)dest)[0] = rs_endian_uint32(write->length + 1);
                ((uint8_t*)dest)[4] = enc;
                
                /* write out the data */
                rs_assert(write->data);
                memcpy(dest + 4 + 1, write->data, write->length);
                
                /* move along */
                cur_sector += sector_count;
            }
        } else {
            /* there's already header info in place, we just need to
             * shuffle things around and update it
             */
            
            /* for now, this is unsupported */
            rs_assert(false);
        }
    }
    
    rs_list_free(self->cached_writes);
    self->cached_writes = NULL;
    
    /* sync the memory */
    if (self->map && msync(self->map, self->fsize, MS_SYNC) < 0)
    {
        rs_assert(false); /* sync failed */
    }
}
