/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "redstone.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef void (*RSTestFunc)();

typedef struct
{
    const char* name;
    RSTestFunc func;
    enum
    {
        NORMALLY_PASSES,
        NORMALLY_FAILS,
    } type;
} RSTest;

#define RS_TEST(sym, name)                                      \
    static void sym##_func();                                   \
    static RSTest sym = {name, sym##_func, NORMALLY_PASSES};    \
    static void sym##_func()

#define RS_FAIL_TEST(sym, name)                             \
    static void sym##_func();                               \
    static RSTest sym = {name, sym##_func, NORMALLY_FAILS}; \
    static void sym##_func()

#define RS_TEST_MAIN(name, ...)                                 \
    static RSTest* rs_tests[] = { __VA_ARGS__, NULL};           \
    int main(int argc, char** argv)                             \
    {                                                           \
        return rs_test_main(name, rs_tests, argc, argv);        \
    }

static int rs_test_main(const char* name, RSTest** tests, int argc, char** argv)
{
    if (argc == 2)
    {
        if (!strcmp(argv[1], "name"))
        {
            printf("%s\n", name);
            return 0;
        } else if (!strcmp(argv[1], "count")) {
            unsigned int count = 0;
            while (*tests)
            {
                count++;
                tests++;
            }
            
            printf("%u\n", count);
            return 0;
        }
    } else if (argc == 3) {
        char* endptr = NULL;
        long testnum = strtol(argv[2], &endptr, 10);
        
        if (*endptr == 0)
        {
            RSTest* test = tests[testnum];
            
            if (!strcmp(argv[1], "name"))
            {
                printf("%s\n", test->name);
                return 0;
            } else if (!strcmp(argv[1], "type")) {
                switch (test->type)
                {
                case NORMALLY_PASSES:
                    printf("NORMALLY_PASSES\n");
                    return 0;
                case NORMALLY_FAILS:
                    printf("NORMALLY_FAILS\n");
                    return 0;
                };
            } else if (!strcmp(argv[1], "run")) {
                test->func();
                return 0;
            }
        }
    }
    
    /* if we're still here, something screwed up */
    fprintf(stderr, "Usage: %s (name | count | name # | type # | run #)\n", argv[0]);
    return 1;
}
