#pragma once 
/**
 * @file
 *
 * Public API for the kernel database
 *
 **/

#include <string>
#include <vector>
#include <tuple>
#include <limits>

#include <nlohmann/json.hpp>

#include "spice_types.h"

namespace SpiceQL {
    namespace Inventory { 
        nlohmann::json search_for_kernelset(std::string spiceql_name, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, bool enforce_quality=false);
        nlohmann::json search_for_kernelsets(std::vector<std::string> spiceql_names, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, bool enforce_quality=false, bool overwrite=false);    
        nlohmann::json search_for_kernelset_from_regex(std::vector<std::string> list);

        std::string getDbFilePath();

        void create_database(std::vector<std::string> mlist = {}); 
    }    
}