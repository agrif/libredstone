/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "test-boilerplate.h"

RS_FAIL_TEST(critical_test, "rs_critical")
{
    rs_critical("critical error!!");
}

RS_TEST_MAIN("error",
             &critical_test)
