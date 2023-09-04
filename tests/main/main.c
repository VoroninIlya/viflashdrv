#include "unity_fixture.h"
#include "viflashdrv.h"

static void runAllTests(void)
{
  RUN_TEST_GROUP(TST_VIFLASHDRV);
}

int main(int argc, const char* argv[])
{
  return UnityMain(argc, argv, runAllTests);
}