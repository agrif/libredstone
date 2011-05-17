/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "error.h"

#include <stdarg.h>
#include <stdio.h>

void _rs_error_log(bool error, const char* filename, unsigned int line, const char* func, const char* str, ...)
{
    va_list ap;
    va_start(ap, str);
    
    if (error)
        printf("ERROR: ");
    else
        printf("CRITICAL: ");
    
    printf("%s:%i (%s) ", filename, line, func);
    vprintf(str, ap);
    
    va_end(ap);
}
