/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_NBT_H_INCLUDED__
#define __RS_NBT_H_INCLUDED__

#include "compression.h"
#include "region.h"

#include <stdint.h>
#include <stdbool.h>

struct _RSNBT;
typedef struct _RSNBT RSNBT;

/* reading */
RSNBT* rs_nbt_open(const char* path, RSCompressionType enc);
RSNBT* rs_nbt_parse(void* data, uint32_t len, RSCompressionType enc);
RSNBT* rs_nbt_parse_from_region(RSRegion* region, uint8_t x, uint8_t z);

/* freeing */
void rs_nbt_free(RSNBT* self);

#endif /* __RS_NBT_H_INCLUDED__ */
