#pragma once
/**
 * @file
 *
 * internal database for the kernel inventory 
 *
 **/

#include <string>
#include <vector>
#include <tuple>
#include <limits>

#include <fc/btree.h>
#include <nlohmann/json.hpp>

#include "spice_types.h"

namespace SpiceQL {

  class TimeIndexedKernels { 
    public: 
    frozenca::BTreeMultiMap<double, size_t> start_times; 
    frozenca::BTreeMultiMap<double, size_t> stop_times; 
    std::vector<std::string> index; 

  };


  class Database {
    public:
    Database(bool force_regen=false); 
    std::string get_root_dir();
    void read_database();
    void write_database();
    nlohmann::json search_kernels(std::string instrument, std::vector<Kernel::Type> types, Kernel::Quality quality, 
                                            double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max());

    nlohmann::json m_json_inventory; 

    std::map<std::string, std::vector<std::string>> m_nontimedep_kerns;  
    std::map<std::string, TimeIndexedKernels*> m_timedep_kerns;      

    // Kernels that always need to be furnished
    KernelSet m_required_kernels; 
  };
}