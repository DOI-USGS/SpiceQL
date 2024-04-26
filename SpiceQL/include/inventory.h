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
        nlohmann::json search_kernels(std::string instrument, std::vector<std::string> types, std::string quality, 
                                                double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max());

    };
}