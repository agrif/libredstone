/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "region.h"

#include "error.h"
#include "memory.h"
#include "rsendian.h"
#include "list.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
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

/* for ordering chunk writes in in-place region writing */
struct OrderedChunkWrite
{
    struct ChunkWrite* write;
    uint32_t order;
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
        return NULL; /* TODO proper error handling */
    }
    
    if (fstat(fd, &stat_buf) < 0)
    {
        close(fd);
        return NULL;
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
    rs_return_if_fail(self);
    
    if (self->write && self->cached_writes)
        rs_region_flush(self);
    rs_assert(self->cached_writes == NULL);
    
    rs_free(self->path);
    if (self->map)
        munmap(self->map, self->fsize);
    close(self->fd);
    rs_free(self);
}

uint32_t rs_region_get_chunk_timestamp(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_return_val_if_fail(self, 0);
    rs_return_val_if_fail(x < 32 && z < 32, 0);
    if (self->timestamps == NULL)
        return 0;
    
    if (!rs_region_contains_chunk(self, x, z))
        return 0;
    
    return rs_endian_uint32(self->timestamps[x + 32*z]);
}

/* LOCAL helper function to return the start of chunk data, including
 * size, compression
 */
static void* _rs_region_get_data(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_return_val_if_fail(self, NULL);
    rs_return_val_if_fail(x < 32 && z < 32, NULL);
    if (self->locations == NULL)
        return NULL;
    
    return self->map + (rs_endian_uint24(self->locations[x + z*32].offset) * 4096);
}

uint32_t rs_region_get_chunk_length(RSRegion* self, uint8_t x, uint8_t z)
{
    if (!rs_region_contains_chunk(self, x, z))
        return 0;
    
    uint32_t* size_int = (uint32_t*)_rs_region_get_data(self, x, z);
    if (!size_int)
        return 0;
    
    /* size is big-endian, and 1 larger than it should be */
    return rs_endian_uint32(size_int[0]) - 1;
}

RSCompressionType rs_region_get_chunk_compression(RSRegion* self, uint8_t x, uint8_t z)
{
    if (!rs_region_contains_chunk(self, x, z))
        return RS_UNKNOWN_COMPRESSION;
    
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
    if (!rs_region_contains_chunk(self, x, z))
        return NULL;
    
    void* ret = _rs_region_get_data(self, x, z);
    if (!ret)
        return NULL;
    
    /* chunk data starts 5 bytes after */
    return ret + 5;
}

bool rs_region_contains_chunk(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_return_val_if_fail(self, false);
    rs_return_val_if_fail(x < 32 && z < 32, false);
    
    /* short-circuit if the file is empty */
    if (self->map == NULL)
        return false;
    
    uint16_t i = z * 32 + x;
    if (self->locations[i].offset == 0)
        return false;
    if (self->locations[i].sector_count == 0)
        return false;
    if (self->timestamps[i] == 0)
        return false;
    
    return true;
}

void rs_region_set_chunk_data(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc)
{
    uint32_t timestamp = time(NULL);
    rs_region_set_chunk_data_full(self, x, z, data, len, enc, timestamp);
}

void rs_region_set_chunk_data_full(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc, uint32_t timestamp)
{
    rs_return_if_fail(self);
    rs_return_if_fail(x < 32 && z < 32);
    if (!(self->write))
    {
        rs_critical("region is not opened in write mode.");
        return;
    }
    
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
        struct ChunkWrite* write = cell->data;
        rs_free(write->data);
        rs_free(write);
        self->cached_writes = rs_list_remove(self->cached_writes, cell);
    }
    
    if (len > 0 && data != NULL && enc == RS_AUTO_COMPRESSION)
        enc = rs_get_compression_type(data, len);
    
    /* copy the data */
    void* data_copy = NULL;
    if (len > 0 && data != NULL)
        data_copy = memcpy(rs_malloc(len), data, len);
    
    /* now, create a new write struct */
    struct ChunkWrite* job = rs_new0(struct ChunkWrite, 1);
    job->x = x;
    job->z = z;
    job->data = data_copy;
    job->length = len;
    job->encoding = enc;
    job->timestamp = timestamp;
    
    self->cached_writes = rs_list_push(self->cached_writes, job);
}

void rs_region_clear_chunk(RSRegion* self, uint8_t x, uint8_t z)
{
    rs_region_set_chunk_data_full(self, x, z, NULL, 0, RS_UNKNOWN_COMPRESSION, 0);
}

/* helper to convert to/from file representation of encodings */
inline uint8_t _rs_region_get_encoding(RSCompressionType enc)
{
    switch (enc)
    {
    case RS_GZIP:
        return 1;
    case RS_ZLIB:
        return 2;
    default:
        rs_return_val_if_reached(0); /* unhandled compression type */
    };
    return 0;
}

/* writes are cached until this is called */
void rs_region_flush(RSRegion* self)
{
    rs_return_if_fail(self);
    
    RSList* cell;
    
    if (self->write && self->cached_writes)
    {
        /* check to see if this is a brand-new file */
        if (self->map == NULL)
        {
            /* it is! so we have to write everything */
            
            /* first, figure out how big the file needs to be */
            uint32_t final_size = 0;
            cell = self->cached_writes;
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
            
            /* make sure final size is on sector boundary */
            rs_assert(final_size % 4096 == 0);
            
            /* resize the file, and remap */
            if (ftruncate(self->fd, final_size) < 0)
            {
                rs_error("file resize failed"); /* FIXME */
            }
            self->fsize = final_size;
            self->map = mmap(NULL, final_size, PROT_READ | PROT_WRITE, MAP_SHARED, self->fd, 0);
            if (self->map == MAP_FAILED)
            {
                rs_error("remap failed"); /* FIXME */
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
                uint8_t enc = _rs_region_get_encoding(write->encoding);
                
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
            
            RSList* cell;
            uint32_t sectors_added = 0;
            
            void* read_head;
            uint32_t read_head_sector;
            void* write_head;
            uint32_t write_head_sector;
            
            /* first, seperate cached_writes into shrinking chunks and
             * growing chunks, sorted by offset, in order to
             * efficiently rewrite the region in only two passes
             */
            
            RSList* shrinks = NULL;
            RSList* grows = NULL;
            
            for (cell = self->cached_writes; cell != NULL; cell = cell->next)
            {
                struct ChunkWrite* write = cell->data;
                RSList** dest = NULL;
                
                bool exists = rs_region_contains_chunk(self, write->x, write->z);
                if (write->data == NULL && !exists)
                {
                    /* clearing a non-existant chunk is a no-op */
                    continue;
                }
                
                if (write->data == NULL || (exists && write->length <= rs_region_get_chunk_length(self, write->x, write->z)))
                {
                    /* this write will shrink the file */
                    dest = &shrinks;
                } else {
                    /* this write will grow the file */
                    dest = &grows;
                    sectors_added += (write->length + 4 + 1) / 4096;
                    if ((write->length + 4 + 1) % 4096 > 0)
                        sectors_added++;
                    if (exists)
                    {
                        sectors_added -= self->locations[write->z * 32 + write->x].sector_count;
                    }
                }
                
                /* make sure we have a list to insert into */
                rs_assert(dest != NULL);
                
                struct OrderedChunkWrite* ordered_write = rs_new(struct OrderedChunkWrite, 1);
                ordered_write->write = write;
                ordered_write->order = rs_endian_uint24(self->locations[write->z * 32 + write->x].offset);
                if (!exists)
                {
                    /* so this chunk is being *added* not modified. We
                     * want to add chunks at the very beginning of the
                     * grow process (taking place at the *end* of the
                     * file), so we need these to show up at the very
                     * end of the list.
                     */
                    ordered_write->order = UINT32_MAX;
                }
                
                while (true)
                {
                    if (*dest == NULL)
                    {
                        /* end of the line -- must insert it here */
                        *dest = rs_list_push(*dest, ordered_write);
                        break;
                    } else {
                        /* check to see if we should insert here */
                        struct OrderedChunkWrite* other = (*dest)->data;
                        if (ordered_write->order <= other->order)
                        {
                            /* make sure we don't have any duplicate writes */
                            rs_assert(write->x != other->write->x || write->z != other->write->z);
                            
                            /* insert before this element! */
                            *dest = rs_list_push(*dest, ordered_write);
                            break;
                        }
                        
                        dest = &((*dest)->next);
                    }
                }   
            }
            
            /* save the old file size, write to new one */
            uint32_t new_fsize = self->fsize;
            
            /* shrink the file */
            if (shrinks)
            {
                /* skip ahead to the first interesting part */
                struct OrderedChunkWrite* first = shrinks->data;
                read_head = _rs_region_get_data(self, first->write->x, first->write->z);
                read_head_sector = rs_endian_uint24(self->locations[first->write->z * 32 + first->write->x].offset);
                write_head = read_head;
                write_head_sector = read_head_sector;
            }
            for (cell = shrinks; cell != NULL; cell = cell->next)
            {
                struct OrderedChunkWrite* ordered_write = cell->data;
                struct ChunkWrite* write = ordered_write->write;
                
                /* move the read head up to the start of this data */
                void* data_start = _rs_region_get_data(self, write->x, write->z);
                rs_assert(data_start >= read_head);
                rs_assert((size_t)(data_start - read_head) % 4096 == 0);
                rs_assert(read_head_sector >= write_head_sector);
                uint32_t sectors_to_write = (data_start - read_head) / 4096;
                
                for (uint8_t z = 0; z < 32; z++)
                {
                    for (uint8_t x = 0; x < 32; x++)
                    {
                        uint32_t sector = rs_endian_uint24(self->locations[z * 32 + x].offset);
                        if (sector >= read_head_sector && sector < read_head_sector + sectors_to_write)
                        {
                            /* we need to shift this location */
                            uint32_t new_sector = sector - (read_head_sector - write_head_sector);
                            rs_assert(new_sector <= sector);
                            self->locations[z * 32 + x].offset = rs_endian_uint24(new_sector);
                        }
                    }
                }
                
                memmove(write_head, read_head, data_start - read_head);
                write_head += data_start - read_head;
                write_head_sector += sectors_to_write;
                read_head += data_start - read_head;
                read_head_sector += sectors_to_write;
                rs_assert(read_head == data_start);
                
                uint16_t i = write->z * 32 + write->x;
                uint8_t old_size = self->locations[i].sector_count;
                
                if (write->data == NULL)
                {
                    /* deleting this chunk */
                    self->locations[i].offset = 0;
                    self->locations[i].sector_count = 0;
                    self->timestamps[i] = 0;
                } else {
                    /* shrinking this chunk */
                    
                    /* calculate total size in sectors */
                    uint8_t new_size = (write->length + 4 + 1) / 4096;
                    if ((write->length + 4 + 1) % 4096 > 0)
                        new_size++;
                    
                    self->locations[i].offset = rs_endian_uint24(write_head_sector);
                    self->locations[i].sector_count = new_size;
                    self->timestamps[i] = rs_endian_uint32(write->timestamp);
                    
                    /* convert compression types */
                    uint8_t enc = _rs_region_get_encoding(write->encoding);
                    
                    /* write data carefully */
                    ((uint32_t*)write_head)[0] = rs_endian_uint32(write->length + 1);
                    ((uint8_t*)write_head)[4] = enc;
                    memcpy(write_head + 4 + 1, write->data, write->length);
                    
                    write_head += new_size * 4096;
                    new_fsize += new_size * 4096;
                    write_head_sector += new_size;
                }
                
                /* move the read head forward */
                read_head += old_size * 4096;
                new_fsize -= old_size * 4096;
                read_head_sector += old_size;
            }
            rs_list_foreach(shrinks, rs_free);
            rs_list_free(shrinks);
            
            /* resize the file to account for both shrinking and growing */
            new_fsize += sectors_added * 4096;
            rs_assert(new_fsize % 4096 == 0);
            if (msync(self->map, self->fsize, MS_SYNC) < 0)
            {
                rs_error("sync failed"); /* FIXME */
            }
            munmap(self->map, self->fsize);
            if (ftruncate(self->fd, new_fsize) < 0)
            {
                rs_error("file resize failed"); /* FIXME */
            }
            self->map = mmap(NULL, new_fsize, PROT_READ | PROT_WRITE, MAP_SHARED, self->fd, 0);
            if (self->map == MAP_FAILED)
            {
                rs_error("remap failed"); /* FIXME */
            }
            self->fsize = new_fsize;
            self->locations = (struct ChunkLocation*)(self->map);
            self->timestamps = (uint32_t*)(self->map + 4096);
            
            /* grow the file (in reverse, so copying doesn't overwrite
             * information we still need)
             */
            write_head = self->map + self->fsize;
            write_head_sector = self->fsize / 4096;
            read_head = write_head - (sectors_added * 4096);
            read_head_sector = write_head_sector - sectors_added;
            grows = rs_list_reverse(grows);
            for (cell = grows; cell != NULL; cell = cell->next)
            {
                struct OrderedChunkWrite* ordered_write = cell->data;
                struct ChunkWrite* write = ordered_write->write;
                uint16_t i = write->z * 32 + write->x;
                if (rs_region_contains_chunk(self, write->x, write->z))
                {
                    /* enlarging chunk */
                    
                    /* move read head to just past this chunk */
                    void* data_end = _rs_region_get_data(self, write->x, write->z);
                    data_end += self->locations[i].sector_count * 4096;
                    
                    rs_assert(data_end <= read_head);
                    rs_assert((size_t)(read_head - data_end) % 4096 == 0);
                    rs_assert(read_head_sector <= write_head_sector);
                    uint32_t sectors_to_write = (read_head - data_end) / 4096;
                    
                    for (uint8_t z = 0; z < 32; z++)
                    {
                        for (uint8_t x = 0; x < 32; x++)
                        {
                            uint32_t sector = rs_endian_uint24(self->locations[z * 32 + x].offset);
                            if (sector >= read_head_sector - sectors_to_write && sector < read_head_sector)
                            {
                                /* we need to shift this location */
                                uint32_t new_sector = sector + (write_head_sector - read_head_sector);
                                rs_assert(new_sector >= sector);
                                self->locations[z * 32 + x].offset = rs_endian_uint24(new_sector);
                            }
                        }
                    }
                    
                    write_head -= read_head - data_end;
                    write_head_sector -= sectors_to_write;
                    memmove(write_head, data_end, read_head - data_end);
                    read_head -= read_head - data_end;
                    read_head_sector -= sectors_to_write;
                    rs_assert(read_head == data_end);
                    
                    /* write out the new, enlarged chunk */
                    
                    uint8_t old_sectors = self->locations[i].sector_count;
                    uint8_t sectors = (write->length + 4 + 1) / 4096;
                    if ((write->length + 4 + 1) % 4096 > 0)
                        sectors++;
                    
                    write_head -= sectors * 4096;
                    write_head_sector -= sectors;
                    
                    self->locations[i].offset = rs_endian_uint24(write_head_sector);
                    self->locations[i].sector_count = sectors;
                    self->timestamps[i] = rs_endian_uint32(write->timestamp);
                    
                    /* convert compression type */
                    uint8_t enc = _rs_region_get_encoding(write->encoding);
                    
                    /* write data carefully */
                    ((uint32_t*)write_head)[0] = rs_endian_uint32(write->length + 1);
                    ((uint8_t*)write_head)[4] = enc;
                    memcpy(write_head + 4 + 1, write->data, write->length);
                    
                    /* move read head past old chunk */
                    read_head -= old_sectors * 4096;
                    read_head_sector -= old_sectors;
                } else {
                    /* adding chunk */
                    uint8_t sectors = (write->length + 4 + 1) / 4096;
                    if ((write->length + 4 + 1) % 4096 > 0)
                        sectors++;
                    
                    write_head -= sectors * 4096;
                    write_head_sector -= sectors;
                    
                    self->locations[i].offset = rs_endian_uint24(write_head_sector);
                    self->locations[i].sector_count = sectors;
                    self->timestamps[i] = rs_endian_uint32(write->timestamp);
                    
                    /* convert compression type */
                    uint8_t enc = _rs_region_get_encoding(write->encoding);
                    
                    /* write data carefully */
                    ((uint32_t*)write_head)[0] = rs_endian_uint32(write->length + 1);
                    ((uint8_t*)write_head)[4] = enc;
                    memcpy(write_head + 4 + 1, write->data, write->length);
                }
            }
            rs_list_foreach(grows, rs_free);
            rs_list_free(grows);
            
            /* all done (*phew*) */
        }
    }
    
    /* clear the cached writes */
    cell = self->cached_writes;
    for (; cell != NULL; cell = cell->next)
    {
        struct ChunkWrite* write = cell->data;
        rs_free(write->data);
        rs_free(write);
    }
    rs_list_free(self->cached_writes);
    self->cached_writes = NULL;
    
    /* sync the memory */
    if (self->map && msync(self->map, self->fsize, MS_SYNC) < 0)
    {
        rs_error("sync failed"); /* FIXME */
    }
}
