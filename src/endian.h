/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_ENDIAN_H_INCLUDED__
#define __RS_ENDIAN_H_INCLUDED__

#include <stdint.h>

/* basic big endian converters */

uint16_t rs_endian_uint16(uint16_t in);
uint32_t rs_endian_uint24(uint32_t in);
uint32_t rs_endian_uint32(uint32_t in);
uint64_t rs_endian_uint64(uint64_t in);

#endif /* __RS_ENDIAN_H_INCLUDED__ */
