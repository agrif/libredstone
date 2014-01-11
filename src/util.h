/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_UTIL_H_INCLUDED__
#define __RS_UTIL_H_INCLUDED__

/**
 * Begins multi-line preprocessor macros.
 *
 * Use this and RS_STMT_END to write multi-line preprocessor macros
 * that work in places where the compiler only expects one statement.
 *
 * \sa RS_STMT_END
 */
#define RS_STMT_START do

/**
 * Ends multi-line preprocessor macros.
 *
 * Use this and RS_STMT_BEGIN to write multi-line preprocessor macros
 * that work in places where the compiler only expects one statement.
 *
 * \sa RS_STMT_BEGIN
 */
#define RS_STMT_END while(0)

/** Minimum macro. */
#define MIN(a, b) ((a) > (b) ? (b) : (a))
/** Maximum macro. */
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#endif /* __RS_UTIL_H_INCLUDED__ */
