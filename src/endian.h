/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_ENDIAN_H_INCLUDED__
#define __RS_ENDIAN_H_INCLUDED__

/**
 * \defgroup endian Endianness Handling
 *
 * All of libredstone's endian functions take the form rs_endian_type,
 * where type is the type of variable that the function converts. All
 * of these functions convert from big-endian (the format that
 * Minecraft uses everywhere) to native-endian, whatever that may be.
 *
 * Calling these functions on a native-endian value has the effect of
 * switching back to big-endian.
 *
 * Functions are provided for signed and unsigned 16, 32, and 64 bit
 * integers, as well as unsigned 24-bit integers (stored in a 32-bit
 * int). There are also functions to convert floats (32-bit) and
 * doubles (64-bit).
 *
 * @{
 */

#include <stdint.h>

/* basic big endian converters */

uint16_t rs_endian_uint16(uint16_t in);
int16_t rs_endian_int16(int16_t in);

uint32_t rs_endian_uint24(uint32_t in);

uint32_t rs_endian_uint32(uint32_t in);
int32_t rs_endian_int32(int32_t in);

uint64_t rs_endian_uint64(uint64_t in);
int64_t rs_endian_int64(int64_t in);

float rs_endian_float(float in);
double rs_endian_double(double in);

/** @} */
#endif /* __RS_ENDIAN_H_INCLUDED__ */
