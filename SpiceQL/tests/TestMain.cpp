#include <gtest/gtest.h>
#include "Fixtures.h"

#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
   spdlog::set_level(spdlog::level::trace);
   spdlog::set_pattern("SpiceQL-TESTS [%H:%M:%S %z] [%l] [%s@%# %!] %v"); 
   
   testing::Environment* const spiceql_env = testing::AddGlobalTestEnvironment(new TempTestingFiles);

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}