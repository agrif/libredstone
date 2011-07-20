/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_NBT_H_INCLUDED__
#define __RS_NBT_H_INCLUDED__

#include "compression.h"
#include "region.h"
#include "tag.h"

#include <stdint.h>
#include <stdbool.h>

struct _RSNBT;
typedef struct _RSNBT RSNBT;

/* creating / reading / freeing */
RSNBT* rs_nbt_new(void);
RSNBT* rs_nbt_parse(void* data, size_t len, RSCompressionType enc);
RSNBT* rs_nbt_parse_from_region(RSRegion* region, uint8_t x, uint8_t z);
RSNBT* rs_nbt_parse_from_file(const char* path);
void rs_nbt_free(RSNBT* self);

/* writing (returns true on success) */
bool rs_nbt_write(RSNBT* self, void** datap, size_t* lenp, RSCompressionType enc);
/* must flush region after writes */
bool rs_nbt_write_to_region(RSNBT* self, RSRegion* region, uint8_t x, uint8_t z);
bool rs_nbt_write_to_file(RSNBT* self, const char* path);

/* getting info */
const char* rs_nbt_get_name(RSNBT* self);
void rs_nbt_set_name(RSNBT* self, const char* name);
RSTag* rs_nbt_get_root(RSNBT* self);
void rs_nbt_set_root(RSNBT* self, RSTag* root);

/* conveniences that call equivalent functions on root tag */
#define rs_nbt_find(self, name) (rs_tag_find(rs_nbt_get_root(self), (name)))
#define rs_nbt_print(self, dest) (rs_tag_print(rs_nbt_get_root(self), (dest)))
#define rs_nbt_pretty_print(self, dest) (rs_tag_pretty_print(rs_nbt_get_root(self), (dest)))
#define rs_nbt_get_chain(self, ...) (rs_tag_compound_get_chain(rs_nbt_get_root(self), __VA_ARGS__))

#endif /* __RS_NBT_H_INCLUDED__ */
