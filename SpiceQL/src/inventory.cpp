
#include <iostream>
#include <regex>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <ghc/fs_std.hpp>
#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

#include "inventory.h"
#include "inventoryimpl.h"
#include "spice_types.h"
#include "utils.h"

using json = nlohmann::json;
using namespace std; 

namespace SpiceQL { 
    namespace Inventory { 
        json search_for_kernelset(string instrument, vector<string> types, double start_time, double stop_time,  
                                  vector<string> ckQualities, vector<string> spkQualities, bool full_kernel_path, 
                                  int limit_ck, int limit_spk) { 
            InventoryImpl impl;
            
            vector<Kernel::Quality> enum_ck_qualities = Kernel::translateQualities(ckQualities);
            vector<Kernel::Quality> enum_spk_qualities = Kernel::translateQualities(spkQualities);
            
            vector<Kernel::Type> enum_types;
            for (auto &e:types) { 
                enum_types.push_back(Kernel::translateType(e));
            }

            return impl.search_for_kernelset(instrument, enum_types, start_time, stop_time, enum_ck_qualities, enum_spk_qualities, full_kernel_path, limit_ck, limit_spk);
        }

        json search_for_kernelsets(vector<string> spiceql_names, vector<string> types, double start_time, double stop_time, 
                                   vector<string> ckQualities, vector<string> spkQualities, bool full_kernel_path, 
                                   int limit_ck, int limit_spk, bool overwrite) { 
            InventoryImpl impl;
              
            vector<Kernel::Quality> enum_ck_qualities = Kernel::translateQualities(ckQualities);
            vector<Kernel::Quality> enum_spk_qualities = Kernel::translateQualities(spkQualities);

            vector<Kernel::Type> enum_types;
            for (auto &e:types) { 
                enum_types.push_back(Kernel::translateType(e));
            } 

            json kernels = impl.search_for_kernelsets(spiceql_names, enum_types, start_time, stop_time, enum_ck_qualities, enum_spk_qualities, full_kernel_path, limit_ck, limit_spk, overwrite);
            return kernels; 
        }



        json search_for_kernelset_from_regex(vector<string> list, bool full_kernel_path) { 
            // strings should be formatted similar to the hdf keys e.g. 
            // mro/sclk/name 
            // mro/ck/reconstructed/name 
            // <mission>/<quality>/<filename regex> or <mission>/<filename regex>
            json kernels = {} ;

            string hdf_file = getDbFilePath();
            fs::path data_dir = getDataDirectory();

            if (!fs::exists(hdf_file)) { 
            throw runtime_error("DB for kernels (" + hdf_file + ") does not exist");
            }
            
            HighFive::File file(hdf_file, HighFive::File::ReadOnly);    
            
            for(auto &e : list) { 
                string temp;
                if (e[0] != '/') { 
                    e = "/" + e;
                }

                SPDLOG_TRACE("Searching for {}", e);
                fs::path p = fs::path(e); 
                string key = p.parent_path();
                string quality = "NA"; 
                string kernel_type; 

                std::vector<std::string> components;
                int i = 0;
                for (const auto& part : p.parent_path()) {
                    i++; 
                    if (i == 3)
                        kernel_type = part; 
                    else if (i == 4) 
                        quality = part; 
                }

                string regex = p.filename();

                string hdfkey = DB_SPICE_ROOT_KEY + key;
                if (file.exist(hdfkey + "/" + DB_TIME_FILES_KEY)) {
                    SPDLOG_TRACE("Is time Kernel"); 
                    hdfkey += "/" + DB_TIME_FILES_KEY; 
                }
                
                vector<string> file_names;

                if (!file.exist(hdfkey)) 
                    throw runtime_error("Key ["+hdfkey+"] does not exist");
                try { 
                    SPDLOG_TRACE("Loading {}", hdfkey);

                    // get all the files under the key
                    auto dataset = file.getDataSet(hdfkey);
                    // allocate data 
                    file_names = dataset.read<vector<string>>();    
                    // load data into variable
                    dataset.read(file_names);
                } catch (exception &e) {  
                    // if anything goes wrong, skip 
                    SPDLOG_ERROR("Exception while reading {}: {}", hdfkey, e.what());
                    continue;
                }

                // iterate through files and filter 
                for(auto &f : file_names) { 
                    temp = fs::path(f).filename();
                    SPDLOG_TRACE("Checking using regex \"{}\": {}", regex, temp);
                    if (regex_search(temp.c_str(), basic_regex(regex, regex_constants::optimize|regex_constants::ECMAScript)) && temp.at(0) != '.' ) {
                        SPDLOG_TRACE("{} matches; adding {} at {}", temp, f, key); 
                        fs::path f_path = fs::path(f);
                        if (full_kernel_path) {
                            f_path = data_dir / f_path;
                        }
                        if (kernels[kernel_type].is_null())
                            kernels[kernel_type] = {f_path};
                        else 
                            kernels[kernel_type].push_back(f_path);

                        if (kernel_type == "spk")
                            kernels["spk_quality"] = quality; 
                        else if (kernel_type == "ck") 
                            kernels["ck_quality"] = quality;
                    }
                }
            }
            
            return kernels; 
        }

        
        /**
         * @brief Get the file path to the database file
         * 
         * @return string 
         */
        string getDbFilePath() { 
            static string db_path = fs::path(getCacheDir()) / DB_HDF_FILE;
            SPDLOG_TRACE("db_path: {}", db_path);
            return db_path;
        }
        

        /**
         * @brief Set the file path to the database file
         * 
         * @param db_file_path The path to the database file
         * @param override If true, the database file path will be set to the provided path regardless of the environment variable
         */
        void setDbFilePath(string db_file_path, bool override) {
            setCacheDir(db_file_path, override);
        }

        void create_database(vector<string> mlist) { 
            // force generate the database
            InventoryImpl db(true, mlist);
        }
    }
}