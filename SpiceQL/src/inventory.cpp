
#include <iostream>
#include <regex>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "inventory.h"
#include "database.h"
#include "spice_types.h"

using json = nlohmann::json;
using namespace std; 

namespace SpiceQL { 
    Inventory::Inventory(bool force_regen) { 
        m_impl = new Database(force_regen);
    }
    
    Inventory::~Inventory() { 
        delete m_impl;
    }

    nlohmann::json Inventory::search_kernels(std::string instrument, std::vector<string> types, string quality, double start_time, double stop_time) { 
        vector<Kernel::Type> enum_types;
        Kernel::Quality enum_quality = Kernel::translateQuality(quality); 

        for (auto &e:types) { 
            enum_types.push_back(Kernel::translateType(e));
        }

        return m_impl->search_kernels(instrument, enum_types, enum_quality, start_time, stop_time);
    }
}