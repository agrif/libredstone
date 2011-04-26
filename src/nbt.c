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
    RSTagType root_type;
    char* root_name;
    RSTag* root;
};

struct _RSTag
{
    uint32_t refcount;
    RSTagType type;
    
    union
    {
        char* string;
    };
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

/* internal helper to parse string tags */
static inline char* _rs_nbt_parse_string(void** datap, uint32_t* lenp)
{
    if (*lenp < 2)
        return NULL;
    
    uint16_t* data = *datap;
    uint16_t strlen = rs_endian_uint16(data[0]);
    if (*lenp < 2 + strlen)
        return NULL;
    
    char* ret = memcpy(rs_new(char, strlen + 1), *datap + 2, strlen);
    ret[strlen] = 0;
    *datap += 2 + strlen;
    *lenp -= 2 + strlen;
    
    return ret;
}

/* internal helper to parse nbt tags recursively */
static RSTag* _rs_nbt_parse_tag(RSTagType type, void** datap, uint32_t* lenp)
{
    RSTag* ret = rs_tag_new(type);
    void* data = *datap;
    uint32_t len = *lenp;
    
    /* temporary vars used in the switch */
    char* string;
    
    switch (type)
    {
    case RS_TAG_STRING:
        string = _rs_nbt_parse_string(datap, lenp);
        rs_tag_set_string(ret, string);
        rs_free(string);
        return ret;
    };
    
    /* if we get here, it's a failure */
    rs_tag_unref(ret);
    printf("unhandled tag type: %i\n", type);
    return NULL;
}

RSNBT* rs_nbt_parse(void* data, uint32_t len, RSCompressionType enc)
{
    uint8_t* expanded = NULL;
    size_t expanded_size = 0;
    
    rs_decompress(enc, data, len, &expanded, &expanded_size);
    if (!expanded)
        return NULL;
    
    /* make sure there's actually *some* data to work with */
    if (expanded_size < 4)
    {
        rs_free(expanded);
        return NULL;
    }
    
    RSNBT* self = rs_new0(RSNBT, 1);
    void* read_head = expanded;
    uint32_t left = expanded_size;
    
    /* first, figure out what the root type is */
    self->root_type = expanded[0];
    read_head++;
    left--;
    
    /* now, read in the root name */
    self->root_name = _rs_nbt_parse_string(&read_head, &left);
    if (self->root_name == NULL)
    {
        rs_free(expanded);
        rs_nbt_free(self);
        return NULL;
    }
    
    self->root = _rs_nbt_parse_tag(self->root_type, &read_head, &left);
    if (self->root == NULL || left != 0)
    {
        rs_free(expanded);
        rs_nbt_free(self);
        return NULL;
    }
    
    rs_free(expanded);
    return self;
}

void rs_nbt_free(RSNBT* self)
{
    rs_assert(self);
    
    if (self->root_name)
        rs_free(self->root_name);
    if (self->root)
        rs_tag_unref(self->root);
    
    rs_free(self);
}

const char* rs_nbt_get_name(RSNBT* self)
{
    rs_assert(self);
    return self->root_name;
}

void rs_nbt_set_name(RSNBT* self, const char* name)
{
    rs_assert(self);
    if (self->root_name)
        rs_free(self->root_name);
    self->root_name = rs_strdup(name);
}

RSTag* rs_nbt_get_root(RSNBT* self)
{
    rs_assert(self);
    return self->root;
}

void rs_nbt_set_root(RSNBT* self, RSTag* root)
{
    rs_assert(self);
    if (root)
        rs_tag_ref(root);
    if (self->root)
        rs_tag_unref(self->root);
    self->root = root;
}

/* tag stuff */

RSTag* rs_tag_new(RSTagType type)
{
    RSTag* self = rs_new0(RSTag, 1);
    self->refcount = 1;
    self->type = type;
    return self;
}

RSTagType rs_tag_get_type(RSTag* self)
{
    rs_assert(self);
    return self->type;
}

/* internal free, used by unref */
static void _rs_tag_free(RSTag* self)
{
    rs_assert(self);
    rs_assert(self->refcount == 0);
    
    switch (self->type)
    {
    case RS_TAG_STRING:
        rs_free(self->string);
        break;
    };
    
    rs_free(self);
}

void rs_tag_ref(RSTag* self)
{
    rs_assert(self);
    self->refcount++;
}

void rs_tag_unref(RSTag* self)
{
    rs_assert(self);
    rs_assert(self->refcount > 0);
    
    self->refcount--;
    if (self->refcount == 0)
        _rs_tag_free(self);
}

const char* rs_tag_get_string(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_STRING);
    return self->string;
}

void rs_tag_set_string(RSTag* self, const char* str)
{
    rs_assert(self && self->type == RS_TAG_STRING);
    if (self->string)
        rs_free(self->string);
    self->string = rs_strdup(str);
}
