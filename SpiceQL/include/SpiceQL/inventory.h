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

#include <SpiceQL/spice_types.h>

namespace SpiceQL {
    namespace Inventory { 
        nlohmann::json search_for_kernelset(std::string spiceql_name, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1);
        nlohmann::json search_for_kernelsets(std::vector<std::string> spiceql_names, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1,
                                      bool overwrite=false);    
        nlohmann::json search_for_kernelset_from_regex(std::vector<std::string> list, bool full_kernel_path=false);

        std::string getDbFilePath();
        void setDbFilePath(std::string db_file_path, bool override=false);

        void create_database(std::vector<std::string> mlist = {});

        /**
         * @brief Get the cached list of frame/config names from the database.
         *
         * @return std::vector<std::string> list of frame names
         */
        std::vector<std::string> getFrameList();

        /**
         * @brief Resolve a frame/body code to its name using the cached map.
         *
         * Uses the precomputed code<->name map built during create_database, so
         * no FKs need to be furnished at runtime.
         *
         * @param code NAIF frame/body code
         * @return the name, or "" if not in the cache
         */
        std::string getFrameNameFromCache(int code);

        /**
         * @brief Resolve a frame/body name to its code using the cached map.
         *
         * @param name frame/body name
         * @return the code, or 0 if not in the cache
         */
        int getFrameCodeFromCache(std::string name);
    }
}
