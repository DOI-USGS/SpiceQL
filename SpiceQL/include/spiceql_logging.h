#pragma once

// SpiceQL logging.
//
// SpiceQL does not use spdlog. spdlog keeps its loggers in a global registry
// reached through a function-local-static singleton. When SpiceQL and another
// library (for example USGSCSM) are loaded into one process and were each built
// against a different spdlog, that singleton has an ABI mismatch and the
// process crashes at load. spdlog 1.16 headers also do not compile against
// fmt 12.
//
// This is a small, self-contained logger that lives entirely in namespace
// SpiceQL, so it exports no symbols that can collide with another library's
// spdlog. Messages are formatted with fmt (already a SpiceQL dependency) and
// written to stderr. The level is read once from the SPICEQL_LOG_LEVEL
// environment variable (off, critical, error, warning, info, debug, trace);
// the default is warning, matching the previous spdlog behavior. The SPDLOG_*
// macro names are kept so existing call sites do not change.

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace SpiceQL {

  enum class LogLevel {
    Trace = 0, Debug, Info, Warn, Error, Critical, Off
  };

  // Map a SPICEQL_LOG_LEVEL string to a level. Unknown values fall back to the
  // default (warning).
  inline LogLevel parseLogLevel(const std::string &s) {
    if (s == "trace")                  return LogLevel::Trace;
    if (s == "debug")                  return LogLevel::Debug;
    if (s == "info")                   return LogLevel::Info;
    if (s == "warn" || s == "warning") return LogLevel::Warn;
    if (s == "error" || s == "err")    return LogLevel::Error;
    if (s == "critical")               return LogLevel::Critical;
    if (s == "off")                    return LogLevel::Off;
    return LogLevel::Warn;
  }

  // Current level, read from the environment once on first use. A function-local
  // static is private to SpiceQL, so there is no cross-library singleton hazard.
  inline LogLevel currentLogLevel() {
    static const LogLevel level = [] {
      const char *env = std::getenv("SPICEQL_LOG_LEVEL");
      return env ? parseLogLevel(env) : LogLevel::Warn;
    }();
    return level;
  }

  inline const char *logLevelName(LogLevel lvl) {
    switch (lvl) {
      case LogLevel::Trace:    return "trace";
      case LogLevel::Debug:    return "debug";
      case LogLevel::Info:     return "info";
      case LogLevel::Warn:     return "warning";
      case LogLevel::Error:    return "error";
      case LogLevel::Critical: return "critical";
      default:                 return "off";
    }
  }

  // The format string is taken at runtime (fmt::string_view, not the
  // compile-time fmt::format_string), because a few call sites build the
  // message by string concatenation rather than a literal. This matches the
  // permissive behavior of the spdlog macros this replaces.
  template<typename... Args>
  inline void logMessage(LogLevel lvl, fmt::string_view f, Args &&... args) {
    if (lvl < currentLogLevel())
      return;
    std::cerr << "SpiceQL [" << logLevelName(lvl) << "] "
              << fmt::vformat(f, fmt::make_format_args(args...)) << "\n";
  }

} // namespace SpiceQL

// Keep the SPDLOG_* spelling so existing call sites are untouched.
#define SPDLOG_TRACE(...)    ::SpiceQL::logMessage(::SpiceQL::LogLevel::Trace,    __VA_ARGS__)
#define SPDLOG_DEBUG(...)    ::SpiceQL::logMessage(::SpiceQL::LogLevel::Debug,    __VA_ARGS__)
#define SPDLOG_INFO(...)     ::SpiceQL::logMessage(::SpiceQL::LogLevel::Info,     __VA_ARGS__)
#define SPDLOG_WARN(...)     ::SpiceQL::logMessage(::SpiceQL::LogLevel::Warn,     __VA_ARGS__)
#define SPDLOG_ERROR(...)    ::SpiceQL::logMessage(::SpiceQL::LogLevel::Error,    __VA_ARGS__)
#define SPDLOG_CRITICAL(...) ::SpiceQL::logMessage(::SpiceQL::LogLevel::Critical, __VA_ARGS__)

// dump_backtrace() was a spdlog feature (replay the last N messages on error).
// It is kept as a no-op stub so the few call sites in utils.cpp still compile.
namespace spdlog {
  inline void dump_backtrace() {}
}
