#include "unity.h"
#include "unity_fixture.h"
#include "viflashdrv.h"

TEST_GROUP(TST_VIFLASHDRV);

TEST_GROUP_RUNNER(TST_VIFLASHDRV)
{
  RUN_TEST_CASE(TST_VIFLASHDRV, VIFLASH_SomeFunc);
}

TEST_SETUP(TST_VIFLASHDRV)
{

}

TEST_TEAR_DOWN(TST_VIFLASHDRV)
{
}

// ===================================================================================
// Test VIFLASH_SomeFunc =============================================================
TEST(TST_VIFLASHDRV, VIFLASH_SomeFunc)
{

}