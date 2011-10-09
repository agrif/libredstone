/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "test-boilerplate.h"

RS_TEST(malloc_test, "rs_malloc")
{
    uint8_t* data = (uint8_t*)rs_malloc(200);
    rs_assert(data);
    data[0] = 0x12;
    data[199] = 0x34;
    
    rs_free(data);
}

RS_TEST(malloc0_test, "rs_malloc0")
{
    uint8_t* data = (uint8_t*)rs_malloc0(200);
    rs_assert(data);
    for (int i = 0; i < 200; i++)
    {
        rs_assert(data[i] == 0);
    }
    
    rs_free(data);
}

RS_TEST(free_null_test, "rs_free(NULL)")
{
    rs_free(NULL);
}

RS_TEST(realloc_test, "rs_realloc")
{
    uint8_t* data = (uint8_t*)rs_malloc(200);
    rs_assert(data);
    data[0] = 0x12;
    data[199] = 0x34;
    
    data = (uint8_t*)rs_realloc(data, 400);
    rs_assert(data);
    data[0] = 0x56;
    data[399] = 0x78;
    
    rs_free(data);
}

RS_TEST(realloc_null_test, "rs_realloc(NULL, ...)")
{
    uint8_t* data = (uint8_t*)rs_realloc(NULL, 200);
    rs_assert(data);
    data[0] = 0x12;
    data[199] = 0x34;
    
    rs_free(data);
}

RS_TEST(memdup_test, "rs_memdup")
{
    uint8_t* data = (uint8_t*)rs_malloc(200);
    rs_assert(data);
    data[0] = 0x12;
    data[199] = 0x34;
    
    uint8_t* otherdata = (uint8_t*)rs_memdup(data, 200);
    rs_assert(otherdata);
    rs_assert(data[0] == otherdata[0]);
    rs_assert(data[199] == otherdata[199]);
    
    rs_free(data);
    rs_free(otherdata);
}

RS_TEST(memdup_null_test, "rs_memdup(NULL, ...)")
{
    rs_assert(rs_memdup(NULL, 5) == NULL);
}

RS_TEST(strdup_test, "rs_strdup")
{
    const char* str = "libredstone";
    char* otherstr = rs_strdup(str);
    
    rs_assert(otherstr);
    rs_assert(strcmp(str, otherstr) == 0);
    
    rs_free(otherstr);
}

RS_TEST(strdup_null_test, "rs_strdup(NULL)")
{
    rs_assert(rs_strdup(NULL));
}

static RSMemoryFunctions* active_funcs = NULL;

static void* test_malloc(void* funcs, size_t size)
{
    rs_assert(funcs == active_funcs);
    rs_assert(size > 0);
    return malloc(size);
}

static void* test_malloc0(void* funcs, size_t size)
{
    rs_assert(funcs == active_funcs);
    return calloc(1, size);
}

static void test_free(void* funcs, void* ptr)
{
    rs_assert(funcs == active_funcs);
    rs_assert(ptr);
    free(ptr);
}

static void* test_realloc(void* funcs, void* ptr, size_t newsize)
{
    rs_assert(funcs == active_funcs);
    rs_assert(ptr);
    rs_assert(newsize > 0);
    return realloc(ptr, newsize);
}

RS_TEST(memory_func_test, "RSMemoryFunctions")
{
    RSMemoryFunctions funcs = {test_malloc, test_free, test_realloc, test_malloc0};
    active_funcs = &funcs;
    rs_set_memory_functions(active_funcs);
    
    malloc_test_func();
    malloc0_test_func();
    free_null_test_func();
    realloc_test_func();
    realloc_null_test_func();
}

RS_TEST(memory_func_no0_test, "RSMemoryFunctions (no malloc0)")
{
    RSMemoryFunctions funcs = {test_malloc, test_free, test_realloc, NULL};
    active_funcs = &funcs;
    rs_set_memory_functions(active_funcs);
    
    malloc_test_func();
    malloc0_test_func();
    free_null_test_func();
    realloc_test_func();
    realloc_null_test_func();
}

RS_FAIL_TEST(memory_func_null_test, "RSMemoryFunctions (all null)")
{
    RSMemoryFunctions funcs = {NULL, NULL, NULL, NULL};
    rs_set_memory_functions(&funcs);
}

RS_TEST(memory_func_unset_test, "RSMemoryFunctions (unset)")
{
    RSMemoryFunctions funcs = {test_malloc, test_free, test_realloc, test_malloc0};
    rs_set_memory_functions(&funcs);
    rs_set_memory_functions(NULL);
    
    malloc_test_func();
    malloc0_test_func();
    free_null_test_func();
    realloc_test_func();
    realloc_null_test_func();
}

RS_TEST_MAIN("memory",
             &malloc_test,
             &malloc0_test,
             &free_null_test,
             &realloc_test,
             &realloc_null_test,
             &memdup_test,
             &memdup_null_test,
             &strdup_test,
             &strdup_null_test,
             &memory_func_test,
             &memory_func_no0_test,
             &memory_func_null_test,
             &memory_func_unset_test)
