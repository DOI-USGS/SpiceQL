// spdlog global initializer disabled - causes crashes due to ODR violations
// when multiple libraries (SpiceQL, USGSCSM) embed different spdlog versions.
// ASP does not use SpiceQL logging.

#include <SpiceQL/utils.h>
#include <SpiceQL/spiceql.h>
#include <SpiceQL/memoized_functions.h>
#include <SpiceQL/api.h>
