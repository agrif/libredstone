/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_TAG_H_INCLUDED__
#define __RS_TAG_H_INCLUDED__

#include <stdint.h>
#include <stdbool.h>

struct _RSTag;
typedef struct _RSTag RSTag;

/* used by iteration over contained tags, in lists/compounds */
typedef void* RSTagIterator;

typedef enum
{
    /* not exposed API-wise, but used internally */
    RS_TAG_END = 0,
    
    /* signded, big endian integers, of 8, 16, 32, 64 bits each */
    RS_TAG_BYTE = 1,
    RS_TAG_SHORT = 2,
    RS_TAG_INT = 3,
    RS_TAG_LONG = 4,
    
    /* big-endian, IEEE 754-2008 binary32/binary64 */
    RS_TAG_FLOAT = 5,
    RS_TAG_DOUBLE = 6,
    
    /* more complicated types */
    RS_TAG_BYTE_ARRAY = 7,
    RS_TAG_STRING = 8,
    RS_TAG_LIST = 9,
    RS_TAG_COMPOUND = 10,
} RSTagType;

RSTag* rs_tag_new0(RSTagType type);
RSTagType rs_tag_get_type(RSTag* self);

/* fancy-pants tag creator that can set values for you. The form these
 * calls must take is different depending on the type, so see below...
 *
 * integer types: rs_tag_new(RS_TAG_INT, 103301);
 * float types:   rs_tag_new(RS_TAG_DOUBLE, 1.337);
 * byte arrays:   rs_tag_new(RS_TAG_BYTE_ARRAY, length, data_pointer);
 * strings:       rs_tag_new(RS_TAG_STRING, "Hello, world!");
 * lists:         rs_tag_new(RS_TAG_LIST, tag1, tag2, tag3, NULL);
 * compounds:     rs_tag_new(RS_TAG_COMPOUND, "key1", tag1, "key2", tag2, NULL);
 */
RSTag* rs_tag_new(RSTagType type, ...);

/* recommended ref/unref memory management
 * Note: tags start out with a "floating reference", that is sunk when
 * it is first ref'd. So, if you create a tag and add it to something,
 * you don't have to unref it. This is *convenient*.
 */
void rs_tag_ref(RSTag* self);
void rs_tag_unref(RSTag* self);

/* for (all) integers -- conversion is automatic */
int64_t rs_tag_get_integer(RSTag* self);
void rs_tag_set_integer(RSTag* self, int64_t val);

/* for floats/doubles -- conversion is automatic */
double rs_tag_get_float(RSTag* self);
void rs_tag_set_float(RSTag* self, double val);

/* for byte arrays */
uint8_t* rs_tag_get_byte_array(RSTag* self);
uint32_t rs_tag_get_byte_array_length(RSTag* self);
void rs_tag_set_byte_array(RSTag* self, uint32_t len, uint8_t* data);

/* for strings */
const char* rs_tag_get_string(RSTag* self);
void rs_tag_set_string(RSTag* self, const char* str);

/* for lists */
void rs_tag_list_iterator_init(RSTag* self, RSTagIterator* it);
bool rs_tag_list_iterator_next(RSTagIterator* it, RSTag** tag);
RSTagType rs_tag_list_get_type(RSTag* self);
/* type-setting can only be done on EMPTY lists */
void rs_tag_list_set_type(RSTag* self, RSTagType type);
uint32_t rs_tag_list_get_length(RSTag* self);
RSTag* rs_tag_list_get(RSTag* self, uint32_t i);
void rs_tag_list_delete(RSTag* self, uint32_t i);
void rs_tag_list_insert(RSTag* self, uint32_t i, RSTag* tag);
void rs_tag_list_reverse(RSTag* self);

/* for compounds */
void rs_tag_compound_iterator_init(RSTag* self, RSTagIterator* it);
bool rs_tag_compound_iterator_next(RSTagIterator* it, const char** key, RSTag** value);
uint32_t rs_tag_compound_get_length(RSTag* self);
RSTag* rs_tag_compound_get(RSTag* self, const char* key);
void rs_tag_compound_set(RSTag* self, const char* key, RSTag* value);
void rs_tag_compound_delete(RSTag* self, const char* key);

#endif /* __RS_TAG_H_INCLUDED__ */
