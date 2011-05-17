/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_ERROR_H_INCLUDED__
#define __RS_ERROR_H_INCLUDED__

#include "util.h"

#include <stdbool.h>
#include <assert.h>

/* internal-consistancy checks ONLY, do not use this to check stuff
 * that comes from the user (of the library, or the program using
 * it). These *can* be compiled out, so don't use it for checks you
 * always want.
 */
#define rs_assert(val) assert(val)

/* internally used */
void _rs_error_log(bool error, const char* filename, unsigned int line, const char* func, const char* str, ...);

/* printf-like. Use this when the program is not *currently* crashing,
 * but likely will in the future because of some very unexpected
 * error. An example would be getting a NULL pointer as a self
 * object. All the rs_return_if_fail(...) variants use this.
 */
#define rs_critical(...) _rs_error_log(false, __FILE__, __LINE__, __func__, __VA_ARGS__)

/* printf-like. Use this when the program has hit a dead end, and
 * nothing can be done to salvage it. These should be *rare*, because
 * they cause an immediate exit
 */
#define rs_error(...) _rs_error_log(true, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define rs_return_if_fail(test) RS_STMT_START { \
        if (!(test))                            \
        {                                       \
            rs_critical(#test " failed.");      \
            return;                             \
        }                                       \
    } RS_STMT_END
#define rs_return_val_if_fail(test, val) RS_STMT_START { \
        if (!(test))                                     \
        {                                                \
            rs_critical(#test " failed.");               \
            return val;                                  \
        }                                                \
    } RS_STMT_END
    
#define rs_return_if_reached() RS_STMT_START {      \
        rs_critical("Unreachable code reached.");   \
        return;                                     \
    } RS_STMT_END
#define rs_return_val_if_reached(val) RS_STMT_START { \
        rs_critical("Unreachable code reached.");     \
        return val;                                   \
    } RS_STMT_END

#endif /* __RS_ERROR_H_INCLUDED__ */
