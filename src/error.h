/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_ERROR_H_INCLUDED__
#define __RS_ERROR_H_INCLUDED__

#include "util.h"

#include <stdbool.h>
#include <assert.h>

/**
 * Assert that something is true.
 *
 * Use this for internal-consistancy checks ONLY, and do NOT use it to
 * check stuff that comes from the outside world. These can be
 * compiled out, so don't use it for checks you always want!
 *
 * For something equivalent to use on user-provided data, use
 * rs_critical(), rs_error(), and the rs_return_* family of functions.
 *
 * \param val the truth value to check
 * \sa rs_critical, rs_error
 * \sa rs_return_if_fail, rs_return_val_if_fail
 * \sa rs_return_if_reached, rs_return_val_if_reached
 */
#define rs_assert(val) assert(val)

/** Internal log function, don't use! */
void _rs_error_log(bool error, const char* filename, unsigned int line, const char* func, const char* str, ...);

/**
 * printf-like error logger, non-fatal
 *
 * Use this when the program is not currently crashing, but will
 * likely fail in the future because of some very unexpected error. An
 * example would be getting a NULL pointer as the 'self'
 * parameter. All the rs_return_* variants use this; you should use
 * those if the default error message is sufficient.
 *
 * For a fatal version, see rs_error().
 *
 * \sa rs_error
 * \sa rs_return_if_fail, rs_return_val_if_fail
 * \sa rs_return_if_reached, rs_return_val_if_reached
 */
#define rs_critical(...) _rs_error_log(false, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * printf-like error logger, fatal
 *
 * Use this when a program has hit a dead end, and nothing can be done
 * to salvage it. These should be rare because they cause an immediate
 * exit.
 *
 * For a non-fatal version, see rs_critical().
 *
 * \sa rs_critical
 */
#define rs_error(...) _rs_error_log(true, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * A weaker rs_assert() for external bugs.
 *
 * Use this instead of rs_assert() for programming errors originating
 * outside of libredstone. It will return immediately from the calling
 * function if the test fails, and print out an informative message
 * via rs_critical. This will not exit the program; however, after a
 * call to this function, the program may be in a dangerous state.
 *
 * If you need to return a value, use rs_return_val_if_fail().
 *
 * \param test the truth value to check
 * \sa rs_critical
 * \sa rs_return_val_if_fail
 * \sa rs_return_if_reached, rs_return_val_if_reached
 */
#define rs_return_if_fail(test) RS_STMT_START {     \
        if (!(test))                                \
        {                                           \
            rs_critical("\"" #test "\" failed.");   \
            return;                                 \
        }                                           \
    } RS_STMT_END

/**
 * A weaker rs_assert() for external bugs (with value).
 *
 * Use this instead of rs_assert() for programming errors originating
 * outside of libredstone. It will return immediately from the calling
 * function if the test fails, and print out an informative message
 * via rs_critical. This will not exit the program; however, after a
 * call to this function, the program may be in a dangerous state.
 *
 * If you don't need to return a value, use rs_return_if_fail().
 *
 * \param test the truth value to check
 * \param val the value to return
 * \sa rs_critical
 * \sa rs_return_if_fail
 * \sa rs_return_if_reached, rs_return_val_if_reached
 */
#define rs_return_val_if_fail(test, val) RS_STMT_START { \
        if (!(test))                                     \
        {                                                \
            rs_critical("\"" #test "\" failed.");        \
            return val;                                  \
        }                                                \
    } RS_STMT_END

/**
 * A weaker rs_assert(false) for external bugs.
 *
 * Use this instead of rs_assert() for programming errors originating
 * outside of libredstone. It will return immediately from the calling
 * function, and print out an informative message via
 * rs_critical. This will not exit the program; however, after a call
 * to this function, the program may be in a dangerous state.
 *
 * If you need to return a value, use rs_return_val_if_reached().
 *
 * \sa rs_critical
 * \sa rs_return_if_fail, rs_return_val_if_fail
 * \sa rs_return_val_if_reached
 */
#define rs_return_if_reached() RS_STMT_START {      \
        rs_critical("Unreachable code reached.");   \
        return;                                     \
    } RS_STMT_END

/**
 * A weaker rs_assert(false) for external bugs (with value).
 *
 * Use this instead of rs_assert() for programming errors originating
 * outside of libredstone. It will return immediately from the calling
 * function, and print out an informative message via
 * rs_critical. This will not exit the program; however, after a call
 * to this function, the program may be in a dangerous state.
 *
 * If you don't need to return a value, use rs_return_if_reached().
 *
 * \param val the value to return
 * \sa rs_critical
 * \sa rs_return_if_fail, rs_return_val_if_fail
 * \sa rs_return_if_reached
 */
#define rs_return_val_if_reached(val) RS_STMT_START { \
        rs_critical("Unreachable code reached.");     \
        return val;                                   \
    } RS_STMT_END

#endif /* __RS_ERROR_H_INCLUDED__ */
