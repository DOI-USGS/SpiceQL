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


namespace SpiceQL { 

    class Database;
    
    class Inventory { 
        Database *m_impl;

        public: 
        Inventory(bool force_regen=false);
        ~Inventory();
        nlohmann::json search_for_kernelset(std::string instrument, std::vector<std::string> types, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::string ckQuality="smithed", std::string spkQuality="smithed", bool enforce_quality=false);

    };
}