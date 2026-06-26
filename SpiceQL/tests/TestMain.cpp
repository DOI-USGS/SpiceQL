#include <gtest/gtest.h>
#include "Fixtures.h"

#include <SpiceQL/spiceql_logging.h>

#include <cstdlib>  // setenv

int main(int argc, char **argv) {
   // Run the tests at trace level. The logger reads SPICEQL_LOG_LEVEL once on
   // first use, so set it before any test logs.
   setenv("SPICEQL_LOG_LEVEL", "debug", 1);

   ::testing::InitGoogleTest(&argc, argv);

   // Builds the shared frame-cache database once (reused across test
   // processes); see FrameCacheEnvironment.
   ::testing::AddGlobalTestEnvironment(new FrameCacheEnvironment());

   return RUN_ALL_TESTS();
}
