/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "tag.h"

#include "util.h"
#include "memory.h"
#include "list.h"

#include <stdarg.h>

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

RSTag* rs_tag_new0(RSTagType type)
{
    rs_assert(type != RS_TAG_END);
    
    RSTag* self = rs_new0(RSTag, 1);
    self->refcount = 0; /* floating reference */
    self->type = type;
    return self;
}

RSTagType rs_tag_get_type(RSTag* self)
{
    rs_assert(self);
    return self->type;
}

RSTag* rs_tag_new(RSTagType type, ...)
{
    va_list ap;
    RSTag* self = rs_tag_new0(type);
    
    char* key;
    RSTag* tag;
    
    va_start(ap, type);
    switch (type)
    {
    case RS_TAG_BYTE:
    case RS_TAG_SHORT:
    case RS_TAG_INT:
    case RS_TAG_LONG:
        rs_tag_set_integer(self, va_arg(ap, int));
        break;
    case RS_TAG_FLOAT:
    case RS_TAG_DOUBLE:
        rs_tag_set_float(self, va_arg(ap, double));
        break;
    case RS_TAG_BYTE_ARRAY:
        rs_tag_set_byte_array(self, va_arg(ap, int), va_arg(ap, void*));
        break;
    case RS_TAG_STRING:
        rs_tag_set_string(self, va_arg(ap, char*));
        break;
    case RS_TAG_LIST:
        while (tag = va_arg(ap, RSTag*))
        {
            rs_tag_list_insert(self, 0, tag);
        }
        rs_tag_list_reverse(self);
        break;
    case RS_TAG_COMPOUND:
        while (key = va_arg(ap, char*))
        {
            tag = va_arg(ap, RSTag*);
            rs_assert(tag);
            
            rs_tag_compound_set(self, key, tag);
        }
        break;
    };
    va_end(ap);
    
    return self;
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

    if (self->refcount > 0)
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

void rs_tag_set_byte_array(RSTag* self, uint32_t len, uint8_t* data)
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
