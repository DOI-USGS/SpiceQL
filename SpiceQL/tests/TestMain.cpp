#include <gtest/gtest.h>
#include "Fixtures.h"

#include <SpiceQL/spiceql_logging.h>

#include <cstdlib>  // setenv

int main(int argc, char **argv) {
   // Run the tests at trace level. The logger reads SPICEQL_LOG_LEVEL once on
   // first use, so set it before any test logs.
   setenv("SPICEQL_LOG_LEVEL", "trace", 1);

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
