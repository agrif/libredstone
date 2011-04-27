/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "nbt.h"

#include "util.h"
#include "memory.h"
#include "endian.h"
#include "list.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

/* This implemention of Minecraft's NBT format is based on info from
 * <http://www.minecraft.net/docs/NBT.txt>.
 */

struct _RSNBT
{
    char* root_name;
    RSTag* root;
};

RSNBT* rs_nbt_new(void)
{
    RSNBT* self = rs_new0(RSNBT, 1);
    rs_nbt_set_name(self, "");
    return self;
}

RSNBT* rs_nbt_parse_from_file(const char* path)
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
    RSTag* ret = rs_tag_new0(type);
    
    /* temporary vars used in the switch */
    RSTagType subtype;
    char* string;
    
    int8_t int_byte;
    int16_t int_short;
    int32_t int_int;
    int64_t int_long;

    float float_float;
    double float_double;
    
    switch (type)
    {
    case RS_TAG_BYTE:
        if (*lenp < 1)
            break;
        int_byte = ((int8_t*)(*datap))[0];
        rs_tag_set_integer(ret, int_byte);
        *datap += 1;
        *lenp -= 1;
        return ret;
    case RS_TAG_SHORT:
        if (*lenp < 2)
            break;
        int_short = rs_endian_int16(((int16_t*)(*datap))[0]);
        rs_tag_set_integer(ret, int_short);
        *datap += 2;
        *lenp -= 2;
        return ret;
    case RS_TAG_INT:
        if (*lenp < 4)
            break;
        int_int = rs_endian_int32(((int32_t*)(*datap))[0]);
        rs_tag_set_integer(ret, int_int);
        *datap += 4;
        *lenp -= 4;
        return ret;
    case RS_TAG_LONG:
        if (*lenp < 8)
            break;
        int_long = rs_endian_int64(((int64_t*)(*datap))[0]);
        rs_tag_set_integer(ret, int_long);
        *datap += 8;
        *lenp -= 8;
        return ret;

    case RS_TAG_FLOAT:
        if (*lenp < 4)
            break;
        float_float = rs_endian_float(((float*)(*datap))[0]);
        rs_tag_set_float(ret, float_float);
        *datap += 4;
        *lenp -= 4;
        return ret;
    case RS_TAG_DOUBLE:
        if (*lenp < 8)
            break;
        float_double = rs_endian_double(((double*)(*datap))[0]);
        rs_tag_set_float(ret, float_double);
        *datap += 8;
        *lenp -= 8;
        return ret;
    
    case RS_TAG_BYTE_ARRAY:
        if (*lenp < 4)
            break;
        int_int = rs_endian_int32(((int32_t*)(*datap))[0]);
        *datap += 4;
        *lenp -= 4;
        
        if (*lenp < int_int)
            break;
        rs_tag_set_byte_array(ret, int_int, *datap);
        *datap += int_int;
        *lenp -= int_int;
        return ret;
    case RS_TAG_STRING:
        string = _rs_nbt_parse_string(datap, lenp);
        rs_tag_set_string(ret, string);
        rs_free(string);
        return ret;
    case RS_TAG_LIST:
        if (*lenp < 5)
            break;
        subtype = ((uint8_t*)(*datap))[0];
        *datap += 1;
        *lenp -= 1;
        int_int = rs_endian_int32(((int32_t*)(*datap))[0]);
        *datap += 4;
        *lenp -= 4;
        
        rs_tag_list_set_type(ret, subtype);
        if (int_int <= 0)
            return ret;
        
        while (*lenp > 0)
        {
            RSTag* tmptag = _rs_nbt_parse_tag(subtype, datap, lenp);
            if (!tmptag)
                break;
            rs_tag_list_insert(ret, 0, tmptag);
            int_int--;
            if (int_int == 0)
            {
                rs_tag_list_reverse(ret);
                return ret;
            }
        }
        
        /* if we make it here, it's an error */
        break;
    case RS_TAG_COMPOUND:
        while (*lenp > 0)
        {
            subtype = ((uint8_t*)(*datap))[0];
            *datap += 1;
            *lenp -= 1;
            
            if (subtype == RS_TAG_END)
                return ret;
            
            string = _rs_nbt_parse_string(datap, lenp);
            if (!string)
                break;
            
            RSTag* tmptag = _rs_nbt_parse_tag(subtype, datap, lenp);
            if (!tmptag)
            {
                rs_free(string);
                break;
            }
            
            rs_tag_compound_set(ret, string, tmptag);
            rs_free(string);
        }
        
        /* if we make it here, it's a failure (too little info,
         * couldn't parse a tag, ...
         */
        break;
    };
    
    /* if we get here, it's a failure */
    rs_tag_unref(ret);
    rs_assert(false);
    return NULL;
}

RSNBT* rs_nbt_parse(void* data, size_t len, RSCompressionType enc)
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
    RSTagType root_type = expanded[0];
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
    
    self->root = _rs_nbt_parse_tag(root_type, &read_head, &left);
    if (self->root == NULL || left != 0)
    {
        rs_free(expanded);
        rs_nbt_free(self);
        return NULL;
    }
    
    /* now we must sink the floating reference */
    rs_tag_ref(self->root);
    
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

/* writing */

/* two helpers -- size calculators and writers */

static uint32_t _rs_nbt_tag_length(RSTag* tag)
{
    uint32_t tmp = 0;
    RSTagIterator it;
    const char* subname;
    RSTag* subtag;
    switch (rs_tag_get_type(tag))
    {
    case RS_TAG_BYTE:
        return 1;
    case RS_TAG_SHORT:
        return 2;
    case RS_TAG_INT:
        return 4;
    case RS_TAG_LONG:
        return 8;
    
    case RS_TAG_FLOAT:
        return 4;
    case RS_TAG_DOUBLE:
        return 8;
        
    case RS_TAG_BYTE_ARRAY:
        return 4 + rs_tag_get_byte_array_length(tag);
    case RS_TAG_STRING:
        return 2 + strlen(rs_tag_get_string(tag));
    case RS_TAG_LIST:
        tmp = 1 + 4;
        rs_tag_list_iterator_init(tag, &it);
        while (rs_tag_list_iterator_next(&it, &subtag))
            tmp += _rs_nbt_tag_length(subtag);
        return tmp;
    case RS_TAG_COMPOUND:
        tmp = 1; /* one extra for the TAG_End */
        rs_tag_compound_iterator_init(tag, &it);
        while (rs_tag_compound_iterator_next(&it, &subname, &subtag))
        {
            tmp += 1; /* type */
            tmp += 2 + strlen(subname); /* name */
            tmp += _rs_nbt_tag_length(subtag);
        }
        return tmp;
    };
    
    return 0;
}

static void _rs_nbt_write_tag(RSTag* tag, void** destp)
{
    void* dest = *destp;
    RSTagIterator it;
    const char* subname;
    RSTag* subtag;
    
    switch (rs_tag_get_type(tag))
    {
    case RS_TAG_BYTE:
        ((int8_t*)dest)[0] = rs_tag_get_integer(tag);
        *destp += 1;
        break;
    case RS_TAG_SHORT:
        ((int16_t*)dest)[0] = rs_endian_int16(rs_tag_get_integer(tag));
        *destp += 2;
        break;
    case RS_TAG_INT:
        ((int32_t*)dest)[0] = rs_endian_int32(rs_tag_get_integer(tag));
        *destp += 4;
        break;
    case RS_TAG_LONG:
        ((int64_t*)dest)[0] = rs_endian_int64(rs_tag_get_integer(tag));
        *destp += 8;
        break;

    case RS_TAG_FLOAT:
        ((float*)dest)[0] = rs_endian_float(rs_tag_get_float(tag));
        *destp += 4;
        break;
    case RS_TAG_DOUBLE:
        ((double*)dest)[0] = rs_endian_double(rs_tag_get_float(tag));
        *destp += 8;
        break;
    
    case RS_TAG_BYTE_ARRAY:
        ((int32_t*)dest)[0] = rs_endian_int32(rs_tag_get_byte_array_length(tag));
        dest += 4;
        memcpy(dest, rs_tag_get_byte_array(tag), rs_tag_get_byte_array_length(tag));
        dest += rs_tag_get_byte_array_length(tag);
        *destp = dest;
        break;
    case RS_TAG_STRING:
        subname = rs_tag_get_string(tag);
        ((int16_t*)(dest))[0] = rs_endian_int16(strlen(subname));
        dest += 2;
        memcpy(dest, subname, strlen(subname));
        dest += strlen(subname);
        *destp = dest;
        break;
    case RS_TAG_LIST:
        ((uint8_t*)dest)[0] = rs_tag_list_get_type(tag);
        dest++;
        ((int32_t*)dest)[0] = rs_endian_int32(rs_tag_list_get_length(tag));
        dest += 4;
        *destp = dest;
        
        rs_tag_list_iterator_init(tag, &it);
        while (rs_tag_list_iterator_next(&it, &subtag))
        {
            _rs_nbt_write_tag(subtag, destp);
        }
        break;
    case RS_TAG_COMPOUND:
        rs_tag_compound_iterator_init(tag, &it);
        while (rs_tag_compound_iterator_next(&it, &subname, &subtag))
        {
            ((uint8_t*)dest)[0] = rs_tag_get_type(subtag);
            dest++;
            ((int16_t*)(dest))[0] = rs_endian_int16(strlen(subname));
            dest += 2;
            memcpy(dest, subname, strlen(subname));
            dest += strlen(subname);
            
            *destp = dest;
            _rs_nbt_write_tag(subtag, destp);
            dest = *destp;
        }
        
        ((uint8_t*)dest)[0] = 0;
        *destp += 1;
        break;
    default:
        rs_assert(false); /* unhandled tag type */
    };
}

bool rs_nbt_write(RSNBT* self, void** datap, size_t* lenp, RSCompressionType enc)
{
    rs_assert(self);
    rs_assert(datap && lenp);
    
    if (self->root == NULL)
        return false;
    if (self->root_name == NULL)
        return false;
    
    /* first, get how big the uncompressed version will be */
    uint32_t rawlen = _rs_nbt_tag_length(self->root);
    rawlen += 2 + strlen(self->root_name); /* for name */
    rawlen += 1; /* for type byte */
    
    void* rawbuf = rs_malloc(rawlen);
    void* rawhead = rawbuf;
    
    ((uint8_t*)rawhead)[0] = rs_tag_get_type(self->root);
    rawhead++;
    ((int16_t*)(rawhead))[0] = rs_endian_int16(strlen(self->root_name));
    rawhead += 2;
    memcpy(rawhead, self->root_name, strlen(self->root_name));
    rawhead += strlen(self->root_name);
    
    _rs_nbt_write_tag(self->root, &rawhead);
    
    rs_compress(enc, rawbuf, rawlen, (uint8_t**)datap, lenp);
    rs_free(rawbuf);
    if (*datap == NULL)
        return false;
    return true;
}

bool rs_nbt_write_to_region(RSNBT* self, RSRegion* region, uint8_t x, uint8_t z)
{
    rs_assert(region);
    
    void* outdata;
    size_t outlen;
    if (!rs_nbt_write(self, &outdata, &outlen, RS_ZLIB))
        return false;
    
    rs_region_set_chunk_data(region, x, z, outdata, outlen, RS_ZLIB);
    
    rs_free(outdata);
    return true;
}

bool rs_nbt_write_to_file(RSNBT* self, const char* path)
{
    void* outdata;
    size_t outlen;
    if (!rs_nbt_write(self, &outdata, &outlen, RS_GZIP))
        return false;
    
    FILE* f = fopen(path, "wb");
    if (!f)
    {
        rs_free(outdata);
        return false;
    }
    
    if (fwrite(outdata, 1, outlen, f) != outlen)
    {
        rs_free(outdata);
        fclose(f);
        return false;
    }
    
    rs_free(outdata);
    fclose(f);
    return true;
}

/* info getting/setting */

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
