#pragma once

// SpiceQL logging shim.
//
// SpiceQL is embedded in ASP and USGSCSM. spdlog brings two problems there:
//   1. Its global logger registry is a singleton; when SpiceQL and another
//      library embed different spdlog builds, that singleton has an ABI
//      mismatch and crashes at load.
//   2. spdlog 1.16 headers do not compile against fmt 12 (the SPDLOG_* macro
//      and pattern_formatter paths fail fmt 12's tightened format_to checks).
//
// We do not need SpiceQL logging, so we drop the spdlog dependency entirely:
// the SPDLOG_* macros below expand to nothing and dump_backtrace() is a stub.

#define SPDLOG_TRACE(...)    ((void)0)
#define SPDLOG_DEBUG(...)    ((void)0)
#define SPDLOG_INFO(...)     ((void)0)
#define SPDLOG_WARN(...)     ((void)0)
#define SPDLOG_ERROR(...)    ((void)0)
#define SPDLOG_CRITICAL(...) ((void)0)

namespace spdlog {
  inline void dump_backtrace() {}
}
