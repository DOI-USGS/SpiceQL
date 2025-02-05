
#include <iostream>
#include <regex>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <ghc/fs_std.hpp>

#include "inventory.h"
#include "inventoryimpl.h"
#include "spice_types.h"

using json = nlohmann::json;
using namespace std; 

namespace SpiceQL { 
    namespace Inventory { 
        json search_for_kernelset(string instrument, vector<string> types, double start_time, double stop_time,  string ckQuality, string spkQuality, bool enforce_quality) { 
            InventoryImpl impl;
            vector<Kernel::Type> enum_types;
            Kernel::Quality enum_ck_quality = Kernel::translateQuality(ckQuality); 
            Kernel::Quality enum_spk_quality = Kernel::translateQuality(spkQuality);  

            for (auto &e:types) { 
                enum_types.push_back(Kernel::translateType(e));
            }

            return impl.search_for_kernelset(instrument, enum_types, start_time, stop_time, enum_ck_quality, enum_spk_quality, enforce_quality);
        }

        json search_for_kernelsets(vector<string> spiceql_names, vector<string> types, double start_time, double stop_time, 
                                      string ckQuality, string spkQuality, bool enforce_quality, bool overwrite) { 
            InventoryImpl impl;
            Kernel::Quality enum_ck_quality = Kernel::translateQuality(ckQuality); 
            Kernel::Quality enum_spk_quality = Kernel::translateQuality(spkQuality);   
            vector<Kernel::Type> enum_types;
            for (auto &e:types) { 
                enum_types.push_back(Kernel::translateType(e));
            } 

            json kernels = impl.search_for_kernelsets(spiceql_names, enum_types, start_time, stop_time, enum_ck_quality, enum_spk_quality, enforce_quality, overwrite);
            return kernels; 
        }

        string getDbFilePath() { 
            static string db_path = fs::path(getCacheDir()) / DB_HDF_FILE;
            return db_path;
        }
 
        void create_database(vector<string> mlist) { 
            // force generate the database
            InventoryImpl db(true, mlist);
        }
    }
}