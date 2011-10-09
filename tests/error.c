/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "test-boilerplate.h"

RS_FAIL_TEST(critical_test, "rs_critical")
{
    rs_critical("critical error!!");
}

RS_TEST(critical_is_fatal_test, "rs_critical_is_fatal")
{
    rs_critical_is_fatal = false;
    rs_critical("critical (non-fatal) error!!");
}

RS_FAIL_TEST(error_test, "rs_error")
{
    rs_error("FATAL ERROR!!!!1!");
}

RS_FAIL_TEST(return_if_fail_test, "rs_return_if_fail")
{
    rs_return_if_fail(false);
}

RS_FAIL_TEST(return_if_reached_test, "rs_return_if_reached")
{
    rs_return_if_reached();
}

RS_TEST_MAIN("error",
             &critical_test,
             &critical_is_fatal_test,
             &error_test,
             &return_if_fail_test,
             &return_if_reached_test)
