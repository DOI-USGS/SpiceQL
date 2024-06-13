
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

    nlohmann::json Inventory::search_for_kernelset(std::string instrument, std::vector<string> types, double start_time, double stop_time,  string ckQuality, string spkQuality, bool enforce_quality) { 
        vector<Kernel::Type> enum_types;
        Kernel::Quality enum_ck_quality = Kernel::translateQuality(ckQuality); 
        Kernel::Quality enum_spk_quality = Kernel::translateQuality(spkQuality);  

        for (auto &e:types) { 
            enum_types.push_back(Kernel::translateType(e));
        }

        return m_impl->search_for_kernelset(instrument, enum_types, start_time, stop_time, enum_ck_quality, enum_spk_quality, enforce_quality);
    }



}