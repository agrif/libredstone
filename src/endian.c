/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "endian.h"

#define UNKNOWN_ENDIAN 0
#define BIG_ENDIAN 1
#define LITTLE_ENDIAN 2

static int endianness = UNKNOWN_ENDIAN;

static inline void rs_endian_init(void)
{
    if (endianness == UNKNOWN_ENDIAN)
    {
        /* figure out what our endianness is! */
        short word = 0x0001;
        char* byte = (char*)(&word);
        endianness = byte[0] ? LITTLE_ENDIAN : BIG_ENDIAN;
    }
}

uint16_t rs_endian_uint16(uint16_t in)
{
    rs_endian_init();
    return (endianness == LITTLE_ENDIAN) ? ((in >> 8) | (in << 8)) : in;
}

int16_t rs_endian_int16(int16_t in)
{
    uint16_t tmp = rs_endian_uint16(((uint16_t*)(&in))[0]);
    return ((int16_t*)(&tmp))[0];
}

uint32_t rs_endian_uint24(uint32_t in)
{
    rs_endian_init();
    return (endianness == LITTLE_ENDIAN) ? (rs_endian_uint32(in) >> 8) : in;
}

uint32_t rs_endian_uint32(uint32_t in)
{
    rs_endian_init();
    return (endianness == LITTLE_ENDIAN) ? (((in & 0x000000FF) << 24) + ((in & 0x0000FF00) << 8) + ((in & 0x00FF0000) >> 8) + ((in & 0xFF000000) >> 24)) : in;
}

int32_t rs_endian_int32(int32_t in)
{
    uint32_t tmp = rs_endian_uint32(((uint32_t*)(&in))[0]);
    return ((int32_t*)(&tmp))[0];
}

uint64_t rs_endian_uint64(uint64_t in)
{
    rs_endian_init();
    
    if (endianness == LITTLE_ENDIAN)
    {
        uint64_t ret = 0;
        ret += (in & 0x00000000000000FF) << 56;
        ret += (in & 0x000000000000FF00) << 40;
        ret += (in & 0x0000000000FF0000) << 24;
        ret += (in & 0x00000000FF000000) << 8;
        ret += (in & 0x000000FF00000000) >> 8;
        ret += (in & 0x0000FF0000000000) >> 24;
        ret += (in & 0x00FF000000000000) >> 40;
        ret += (in & 0xFF00000000000000) >> 56;
        return ret;
    }
    
    return in;
}

int64_t rs_endian_int64(int64_t in)
{
    uint64_t tmp = rs_endian_uint64(((uint64_t*)(&in))[0]);
    return ((int64_t*)(&tmp))[0];
}

float rs_endian_float(float in)
{
    uint32_t tmp = rs_endian_uint32(((uint32_t*)(&in))[0]);
    return ((float*)(&tmp))[0];
}

double rs_endian_double(double in)
{
    uint64_t tmp = rs_endian_uint64(((uint64_t*)(&in))[0]);
    return ((double*)(&tmp))[0];
}
