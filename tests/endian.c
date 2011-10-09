/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "test-boilerplate.h"

#include <netinet/in.h>

#define TYPECAST(var, type) (*(type*)(&(var)))

RS_TEST(endian_uint16, "rs_endian_uint16")
{
    uint16_t x = 0x1234;
    rs_assert(rs_endian_uint16(rs_endian_uint16(x)) == x);
    rs_assert(rs_endian_uint16(x) == htons(x));
}

RS_TEST(endian_int16, "rs_endian_int16")
{
    int16_t x = -0x1234;
    rs_assert(rs_endian_int16(rs_endian_int16(x)) == x);
    uint16_t ox = htons(TYPECAST(x, uint16_t));
    rs_assert(rs_endian_int16(x) == TYPECAST(ox, int16_t));
}

RS_TEST(endian_uint32, "rs_endian_uint32")
{
    uint32_t x = 0x12345678;
    rs_assert(rs_endian_uint32(rs_endian_uint32(x)) == x);
    rs_assert(rs_endian_uint32(x) == htonl(x));
}

RS_TEST(endian_int32, "rs_endian_int32")
{
    int32_t x = -0x12345678;
    rs_assert(rs_endian_int32(rs_endian_int32(x)) == x);
    uint32_t ox = htonl(TYPECAST(x, uint32_t));
    rs_assert(rs_endian_int32(x) == TYPECAST(ox, int32_t));
}

static inline void check_uint64(uint64_t x, uint64_t ex)
{
    uint32_t* xcomp = (uint32_t*)&x;
    uint32_t* excomp = (uint32_t*)&ex;
    
    xcomp[0] = htonl(xcomp[0]);
    xcomp[1] = htonl(xcomp[1]);
    
    if (xcomp[0] == excomp[0])
    {
        rs_assert(xcomp[1] == excomp[1]);
    } else {
        rs_assert(xcomp[0] == excomp[1]);
        rs_assert(xcomp[1] == excomp[0]);
    }
}

RS_TEST(endian_uint64, "rs_endian_uint64")
{
    uint64_t x = 0x1234567812345678;
    rs_assert(rs_endian_uint64(rs_endian_uint64(x)) == x);
    check_uint64(x, rs_endian_uint64(x));
}

RS_TEST(endian_int64, "rs_endian_int64")
{
    int64_t x = -0x1234567812345678;
    rs_assert(rs_endian_int64(rs_endian_int64(x)) == x);
    int64_t ex = rs_endian_int64(x);
    check_uint64(TYPECAST(x, uint64_t), TYPECAST(ex, uint64_t));
}

RS_TEST(endian_float, "rs_endian_float")
{
    float x = 12.34;
    rs_assert(rs_endian_float(rs_endian_float(x)) == x);
    float ex = rs_endian_float(x);
    uint32_t ox = htonl(TYPECAST(x, uint32_t));
    rs_assert(TYPECAST(ex, uint32_t) == ox);
}

RS_TEST(endian_double, "rs_endian_double")
{
    double x = 12.34;
    rs_assert(rs_endian_double(rs_endian_double(x)) == x);
    double ex = rs_endian_double(x);
    check_uint64(TYPECAST(x, uint64_t), TYPECAST(ex, uint64_t));
}

RS_TEST_MAIN("endian",
             &endian_uint16,
             &endian_int16,
             &endian_uint32,
             &endian_int32,
             &endian_uint64,
             &endian_int64,
             &endian_float,
             &endian_double)
