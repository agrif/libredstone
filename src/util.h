/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_UTIL_H_INCLUDED__
#define __RS_UTIL_H_INCLUDED__

#include <assert.h>

#define rs_assert(val) assert(val)

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#endif /* __RS_UTIL_H_INCLUDED__ */
