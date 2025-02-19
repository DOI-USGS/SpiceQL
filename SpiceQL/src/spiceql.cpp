#include <map>
#include <string>
#include <fmt/format.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <ghc/fs_std.hpp>
using namespace std;

#include "utils.h"
#include "spiceql.h"
#include "memoized_functions.h"
#include "api.h"

namespace SpiceQL { 
  // putting log init in a anonymous namespace 
  struct initializer {    

    ~initializer() {
    }

    initializer() {
      /** INIT LOG **/ {
        spdlog::enable_backtrace(32); // Store the latest 32 messages in a buffer. 

        static const char* DEFAULT_LOG_LEVEL = "warning";
        
        // init logging
        map<string, spdlog::level::level_enum> severityMap = {
          {"off", spdlog::level::off},
          {"critical", spdlog::level::critical},
          {"error", spdlog::level::err},
          {"warning", spdlog::level::warn}, 
          {"info", spdlog::level::info},
          {"debug", spdlog::level::debug},
          {"trace", spdlog::level::trace}
        };

        // init with log level
        const char* log_level_char = getenv("SPICEQL_LOG_LEVEL");
        string log_level_string; 
        spdlog::level::level_enum log_level;

        if (log_level_char != NULL) { 
          log_level_string = log_level_char; 
        }
        else {
          log_level_char = DEFAULT_LOG_LEVEL;
          log_level_string = log_level_char;
        }

        auto console = spdlog::stdout_logger_mt("console");
        spdlog::set_default_logger(console);
        
        spdlog::set_level(severityMap[SpiceQL::toLower(log_level_string)]);
        spdlog::set_pattern("SpiceQL [%H:%M:%S %z] [%l] [%s@%# %!] %v"); 
        
        // check for log file
        const char* log_dir = getenv("SPICEQL_LOG_DIR");
        fs::path log_file; 

        if (log_dir != NULL) { 
          log_file = (fs::absolute(log_dir) / "spiceql_logs.txt"); 
          
          // Create a file rotating logger with 10mb size max and 3 rotated files
          auto max_size = 1048576 * 10;
          auto max_files = 5;
          try {
              auto logger = spdlog::rotating_logger_mt("spiceql_file_log", log_file, max_size, max_files);
          }
          catch (const spdlog::spdlog_ex &ex) {
              SPDLOG_ERROR("file log init failed: {}", ex.what());
          }                
        } else { 
          log_dir = "NULL";
        }
        
        SPDLOG_DEBUG("Using log dir: {}", log_dir); 
        SPDLOG_DEBUG("Log level: {}", log_level_string); 
        SPDLOG_DEBUG("Log level enum: {}", log_level);
        SPDLOG_TRACE("Log dir: {}", log_dir);
      } 
    }
  };
  
  static initializer i;

}