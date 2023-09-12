#ifndef TestUtilities_h
#define TestUtilities_h

#include "gmock/gmock.h"

#include <string>
#include <vector>

#include <nlohmann/json.hpp>


namespace spiceql {

  ::testing::AssertionResult AssertExceptionMessage(
          const char* e_expr,
          const char* contents_expr,
          std::exception &e,
          std::string contents);
}

#endif