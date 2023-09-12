#include "TestUtilities.h"

#include <cmath>
#include <string>


/**
 * Custom IException assertion that checks that the exception message contains
 * a substring.
 */
::testing::AssertionResult spiceql::AssertExceptionMessage(
        const char* e_expr,
        const char* contents_expr,
        std::exception &e,
        std::string contents) {
if (std::string(e.what()).find(contents) != std::string::npos ) return ::testing::AssertionSuccess();

return ::testing::AssertionFailure() << "Exception "<< e_expr << "\'s error message (\""
    << e.what() << "\") does not contain " << contents_expr << " (\""
    << contents << "\").";
}