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
    RSTagType root_type;
    char* root_name;
    RSTag* root;
};

/* used in the compound tag RSList */
typedef struct
{
    char* key;
    RSTag* value;
} RSTagCompoundNode;

struct _RSTag
{
    uint32_t refcount;
    RSTagType type;
    
    union
    {
        int8_t int_byte;
        int16_t int_short;
        int32_t int_int;
        int64_t int_long;
        
        float float_float;
        double float_double;
        
        struct
        {
            uint32_t size;
            uint8_t* data;
        } byte_array;
        char* string;
        struct
        {
            RSTagType type;
            RSList* items;
        } list;
        RSList* compound;
    };
};

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
    RSTag* ret = rs_tag_new(type);
    
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
        rs_tag_set_byte_array(ret, *datap, int_int);
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
            rs_tag_unref(tmptag);
        }
        
        /* if we make it here, it's a failure (too little info,
         * couldn't parse a tag, ...
         */
        break;
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
    
    RSList* cell;
    
    switch (self->type)
    {
    case RS_TAG_BYTE_ARRAY:
        rs_free(self->byte_array.data);
        break;
    case RS_TAG_STRING:
        rs_free(self->string);
        break;
    case RS_TAG_LIST:
        rs_list_foreach(self->list.items, (RSListFunction)rs_tag_unref);
        rs_list_free(self->list.items);
        break;
    case RS_TAG_COMPOUND:
        cell = self->compound;
        for (; cell != NULL; cell = cell->next)
        {
            RSTagCompoundNode* node = (RSTagCompoundNode*)(cell->data);
            rs_assert(node);
            rs_assert(node->key);
            rs_assert(node->value);
            
            rs_free(node->key);
            rs_tag_unref(node->value);
            rs_free(node);
        }
        
        rs_list_free(self->compound);
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

/* for integers */
int64_t rs_tag_get_integer(RSTag* self)
{
    rs_assert(self);
    switch (self->type)
    {
    case RS_TAG_BYTE:
        return self->int_byte;
    case RS_TAG_SHORT:
        return self->int_short;
    case RS_TAG_INT:
        return self->int_int;
    case RS_TAG_LONG:
        return self->int_long;
    };
    
    rs_assert(false); /* not an integer! */
    return 0;
}

void rs_tag_set_integer(RSTag* self, int64_t val)
{
    rs_assert(self);
    switch (self->type)
    {
    case RS_TAG_BYTE:
        self->int_byte = val;
        return;
    case RS_TAG_SHORT:
        self->int_short = val;
        return;
    case RS_TAG_INT:
        self->int_int = val;
        return;
    case RS_TAG_LONG:
        self->int_long = val;
        return;
    };
    
    rs_assert(false); /* not an integer */
}

/* for floats/doubles -- conversion is automatic */
double rs_tag_get_float(RSTag* self)
{
    rs_assert(self);
    rs_assert(self->type == RS_TAG_FLOAT || self->type == RS_TAG_DOUBLE);
    
    if (self->type == RS_TAG_FLOAT)
        return self->float_float;
    return self->float_double;
}

void rs_tag_set_float(RSTag* self, double val)
{
    rs_assert(self);
    rs_assert(self->type == RS_TAG_FLOAT || self->type == RS_TAG_DOUBLE);
    
    if (self->type == RS_TAG_FLOAT)
    {
        self->float_float = val;
    } else {
        self->float_double = val;
    }
}

/* for byte arrays */
uint8_t* rs_tag_get_byte_array(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_BYTE_ARRAY);
    return self->byte_array.data;
}

uint32_t rs_tag_get_byte_array_length(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_BYTE_ARRAY);
    return self->byte_array.size;
}

void rs_tag_set_byte_array(RSTag* self, uint8_t* data, uint32_t len)
{
    rs_assert(self && self->type == RS_TAG_BYTE_ARRAY);
    uint8_t* olddata = self->byte_array.data;
    self->byte_array.data = rs_memdup(data, len);
    self->byte_array.size = len;
    if (olddata)
        rs_free(olddata);
}

/* for strings */
const char* rs_tag_get_string(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_STRING);
    return self->string;
}

void rs_tag_set_string(RSTag* self, const char* str)
{
    rs_assert(self && self->type == RS_TAG_STRING);
    char* oldstring = self->string;
    self->string = rs_strdup(str);
    if (oldstring)
        rs_free(oldstring);
}

void rs_tag_list_iterator_init(RSTag* self, RSTagIterator* it)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    rs_assert(it);
    
    *it = self->list.items;
}

bool rs_tag_list_iterator_next(RSTagIterator* it, RSTag** tag)
{
    rs_assert(it);
    rs_assert(tag);
    
    RSList* cell = (RSList*)(*it);
    if (!cell)
        return false;
    
    *tag = (RSTag*)(cell->data);
    *it = cell->next;
    
    return true;
}

RSTagType rs_tag_list_get_type(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    return self->list.type;
}

void rs_tag_list_set_type(RSTag* self, RSTagType type)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    rs_assert(self->list.items == NULL);
    self->list.type = type;
}

uint32_t rs_tag_list_get_length(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    return rs_list_size(self->list.items);
}

RSTag* rs_tag_list_get(RSTag* self, uint32_t i)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    return (RSTag*)rs_list_nth(self->list.items, i);
}

void rs_tag_list_delete(RSTag* self, uint32_t i)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    
    RSList* cell = rs_list_nth_cell(self->list.items, i);
    if (cell)
    {
        rs_tag_unref((RSTag*)(cell->data));
        self->list.items = rs_list_remove(self->list.items, cell);
    }
}

void rs_tag_list_insert(RSTag* self, uint32_t i, RSTag* tag)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    rs_assert(tag);
    rs_assert(tag->type == self->list.type);
    
    RSList* cell = rs_list_cell_new();
    
    rs_tag_ref(tag);
    cell->data = tag;
    cell->next = NULL;
    
    if (i == 0)
    {
        cell->next = self->list.items;
        self->list.items = cell;
        return;
    }
    
    uint32_t len = rs_tag_list_get_length(self);
    if (i >= len)
    {
        RSList* last = rs_list_nth_cell(self->list.items, len - 1);
        last->next = cell;
    } else {
        RSList* prev = rs_list_nth_cell(self->list.items, i - 1);
        RSList* next = prev->next;
        
        prev->next = cell;
        cell->next = next;
    }
}

void rs_tag_list_reverse(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_LIST);
    self->list.items = rs_list_reverse(self->list.items);
}       

/* for compounds */
void rs_tag_compound_iterator_init(RSTag* self, RSTagIterator* it)
{
    rs_assert(self && self->type == RS_TAG_COMPOUND);
    rs_assert(it);
    
    *it = self->compound;
}

bool rs_tag_compound_iterator_next(RSTagIterator* it, const char** key, RSTag** value)
{
    rs_assert(it);
    
    RSList* cell = (RSList*)(*it);
    if (!cell)
        return false;
    
    RSTagCompoundNode* node = (RSTagCompoundNode*)(cell->data);
    
    if (key)
        *key = node->key;
    if (value)
        *value = node->value;
    
    *it = cell->next;
    
    return true;
}

uint32_t rs_tag_compound_get_length(RSTag* self)
{
    rs_assert(self && self->type == RS_TAG_COMPOUND);
    return rs_list_size(self->compound);
}

RSTag* rs_tag_compound_get(RSTag* self, const char* key)
{
    rs_assert(self && self->type == RS_TAG_COMPOUND);
    rs_assert(key);
    
    RSList* cell = self->compound;
    for (; cell != NULL; cell = cell->next)
    {
        RSTagCompoundNode* node = (RSTagCompoundNode*)(cell->data);
        rs_assert(node && node->key);
        
        if (strcmp(node->key, key) == 0)
            return node->value;
    }
    
    return NULL;
}

void rs_tag_compound_set(RSTag* self, const char* key, RSTag* value)
{
    rs_assert(self && self->type == RS_TAG_COMPOUND);
    rs_assert(key && value);
    
    rs_tag_compound_delete(self, key);
    
    RSTagCompoundNode* node = rs_new0(RSTagCompoundNode, 1);
    
    node->key = rs_strdup(key);
    rs_tag_ref(value);
    node->value = value;
    
    self->compound = rs_list_push(self->compound, node);
}

void rs_tag_compound_delete(RSTag* self, const char* key)
{
    rs_assert(self && self->type == RS_TAG_COMPOUND);
    rs_assert(key);
    
    RSList* cell = self->compound;
    for (; cell != NULL; cell = cell->next)
    {
        RSTagCompoundNode* node = (RSTagCompoundNode*)(cell->data);
        rs_assert(node && node->key);
        
        if (strcmp(node->key, key) == 0)
            break;
    }
    
    if (cell)
        self->compound = rs_list_remove(self->compound, cell);
}
