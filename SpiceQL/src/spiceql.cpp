// spdlog global initializer disabled - causes crashes due to ODR violations
// when multiple libraries (SpiceQL, USGSCSM) embed different spdlog versions.
// ASP does not use SpiceQL logging.

#include "utils.h"
#include "spiceql.h"
#include "memoized_functions.h"
#include "api.h"