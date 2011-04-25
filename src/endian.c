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
