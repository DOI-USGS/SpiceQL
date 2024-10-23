#include <iostream>
#include <regex>

#include <nlohmann/json.hpp>
#include <fc/btree.h>
#include <fc/disk_btree.h>
#include <spdlog/spdlog.h>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>

#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

#include "config.h"
#include "inventoryimpl.h"
#include "utils.h"
#include "query.h"
#include "memo.h"


using json = nlohmann::json;
using namespace std; 
namespace fc = frozenca;


// enable SIMD for btree operations
#ifdef FC_USE_SIMD 
#undef FC_USE_SIMD 
#define FC_USE_SIMD 1
#endif 


namespace SpiceQL { 

  string DB_SPICE_ROOT_KEY = "spice";
  string DB_HDF_FILE = "spiceqldb.hdf";
  string DB_START_TIME_KEY = "starttime"; 
  string DB_STOP_TIME_KEY = "stoptime"; 
  string DB_TIME_FILES_KEY = "path_index"; 
  string DB_SS_TIME_INDICES_KEY = "ss_index"; 

  string getCacheDir() { 
      static std::string  CACHE_DIRECTORY = "";

      if (CACHE_DIRECTORY == "") { 
          const char* cache_dir_char = getenv("SPICEQL_CACHE_DIR");
      
          std::string  cache_dir; 
      
          if (cache_dir_char == NULL) {
              std::string  tempname = "spiceql-cache-" + gen_random(10);
              cache_dir = fs::temp_directory_path() / tempname / "spiceql_cache"; 
          }
          else {
              cache_dir = cache_dir_char;
          }

          if (!fs::is_directory(cache_dir)) { 
              SPDLOG_DEBUG("{} does not exist, attempting to create the directory", cache_dir);
              fs::create_directories(cache_dir); 
          }
      
          CACHE_DIRECTORY = cache_dir;
          SPDLOG_DEBUG("Setting cache directory to: {}", CACHE_DIRECTORY);  
      } 
      else { 
          SPDLOG_TRACE("Cache Directory Already Set: {}", CACHE_DIRECTORY);  
      }

      return CACHE_DIRECTORY;
  }


  string getHdfFile() { 
      static std::string db_path = fs::path(getCacheDir()) / DB_HDF_FILE;
      return db_path;
  }
  

  // objs need to be passed in c-style because of a lack of copy contructor in BtreeMap
  void collectStartStopTimes(string mission, string type, string quality, TimeIndexedKernels *kernel_times) { 
    SPDLOG_TRACE("In globTimeIntervals.");
    Config conf;
    conf = conf[mission];
    
    json timeJson = conf.getRecursive(type);
    json ckKernelGrp = timeJson[type][quality]["kernels"];
    
    for(auto &arr : ckKernelGrp) {
      for(auto &subArr : arr) {
        for (auto &kernel : subArr) {
          pair<double, double> sstimes = getKernelStartStopTimes(kernel);
          SPDLOG_TRACE("{} times: {}, {}", kernel, sstimes.first, sstimes.second); 
          // use start_time as index to the majority of kernels, then use stop time in the value 
          // to get the final list
          size_t index = 0;
          
          index = kernel_times->file_paths.size(); 

          // cant contruct these in line for whatever reason 
          fc::BTreePair<double, size_t> p;
          p.first = sstimes.first; 
          p.second = index;
          if(kernel_times->start_times.contains(p.first)) { 
            p.first-=0.0000001; 
          } 
          kernel_times->start_times.insert(p);

          fc::BTreePair<double, size_t> p2;
          p2.first = sstimes.second; 
          p2.second = index; 

          if(kernel_times->stop_times.contains(p2.first)) { 
            p2.first+=0.0000001; 
          }  
          kernel_times->stop_times.insert(p2);

          // get relative path to make db portable 
          fs::path relative_path_kernel = fs::relative(kernel, getDataDirectory());
          SPDLOG_TRACE("Relative Kernel: {}", relative_path_kernel.generic_string()); 
          kernel_times->file_paths.push_back(relative_path_kernel);
        }
      }
    }
  }


  InventoryImpl::InventoryImpl(bool force_regen) : m_required_kernels() { 
    fs::path db_root = getCacheDir();
    fs::path db_file = db_root / DB_HDF_FILE; 

    // create the database 
    if (!fs::exists(db_root) || force_regen) { 
      // get everything
      Config config; 
      json json_kernels = getLatestKernels(config.get()); 

      // load time kernels for creating the timed kernel DataBase 
      json sclk_json = getLatestKernels(config.getRecursive("sclk")); 
      json lsk_json = getLatestKernels(config.getRecursive("lsk")); 
      
      m_required_kernels.load(sclk_json); 
      m_required_kernels.load(lsk_json); 

      for (auto &[mission, kernels] : json_kernels.items()) {
        fmt::print("mission: {}\n", mission);    

        for(auto &[kernel_type, kernel_obj] : kernels.items()) { 
          if (kernel_type == "ck" || kernel_type == "spk") { 
            // we need to log the times
            for (auto &quality : Kernel::QUALITIES) {
              if (kernel_obj.contains(quality)) {

                // make the keys match Config's nested keys
                string map_key = mission + "/" + kernel_type +"/"+quality+"/kernels";
                
                // make sure no path symbols are in the key
                // replaceAll(map_key, "/", ":");
                
                TimeIndexedKernels *tkernels = new TimeIndexedKernels();
                // btrees cannot be copied, so use pointers
                collectStartStopTimes(mission, kernel_type, quality, tkernels); 
                m_timedep_kerns[map_key] = tkernels;
              }
            }
          } 
          else { // it's a txt kernel or some other non-time dependant kernel 
            vector<json::json_pointer> ptrs = findKeyInJson(kernel_obj, "kernels", true); 
            
            for (auto &ptr : ptrs) { 
              string btree_key = mission + "/" + kernel_type + "/kernels";
              vector<string> kernel_vec;

              // Doing it bracketless, there are too many brackets
              for (auto &subarr: kernel_obj[ptr]) 
                for (auto &kernel : subarr) { 
                  string k = kernel.get<string>();
                  fs::path relative_path_kernel = fs::relative(k, getDataDirectory());
                  SPDLOG_TRACE("Relative Kernel: {}", relative_path_kernel.generic_string()); 
                  kernel_vec.push_back(relative_path_kernel); 
                } 
              m_nontimedep_kerns[btree_key] = kernel_vec; 
            } 
          }
        } 
      }

      // write everything out
      write_database();
    }
    else { // load the database 
      // read_database(); 
    }
  }


  template<class T>
  T InventoryImpl::getKey(string key) { 
    string hdf_file = fs::path(getCacheDir()) / DB_HDF_FILE;

    if (!fs::exists(hdf_file)) { 
      throw runtime_error("DB for kernels (" + hdf_file + ") does not exist");
    }
    HighFive::File file(hdf_file, HighFive::File::ReadOnly);    

    try { 
      if (!file.exist(key)) 
        throw runtime_error("Key ["+key+"] does not exist");
      // get key
      auto dataset = file.getDataSet(key);
      // allocate data 
      T data = dataset.read<T>();    
      // load data into variable
      dataset.read(data);
      return data; 
    } catch (exception &e) { 
      throw runtime_error("Failed to key [" + key + "] from [" + hdf_file + "].");
    }
  }


  json InventoryImpl::search_for_kernelset(string instrument, vector<Kernel::Type> types, double start_time, double stop_time,
                                  Kernel::Quality ckQuality, Kernel::Quality spkQuality, bool enforce_quality) { 
    // get time dep kernels first 
    json kernels;
    instrument = toLower(instrument);

    fs::path data_dir = getDataDirectory();

    if (start_time > stop_time) { 
      throw range_error("start time cannot be greater than stop time.");
    }

    for (auto &type: types) { 
      // load time kernel
      if (type == Kernel::Type::CK || type == Kernel::Type::SPK) { 
        SPDLOG_DEBUG("Trying to search time dependent kernels");
        TimeIndexedKernels *time_indices = nullptr;
        bool found = false;        

        Kernel::Quality quality = spkQuality;
        string qkey = "spkQuality";
        if (type == Kernel::Type::CK) { 
          quality = ckQuality;
          qkey = "ckQuality";
        }

        // iterate down the qualities 
        for(int i = (int)quality; i >= 0 && !found; i--) { 
          string key = instrument+"/"+Kernel::translateType(type)+"/"+Kernel::QUALITIES.at(i)+"/"+"kernels";
          SPDLOG_DEBUG("Key: {}", key);
          quality = (Kernel::Quality)i; 

          if (m_timedep_kerns.contains(key)) { 
            SPDLOG_DEBUG("Key {} found", key); 
            
            // we can get the key 
            time_indices = m_timedep_kerns[key]; 
            found = true;
          }
          else {
            // try to load the binary files 
            time_indices = new TimeIndexedKernels();
            
            try {
              SPDLOG_TRACE("Starting desrializing the DB");

              vector<double> start_times_v = getKey<vector<double>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_START_TIME_KEY); 
              vector<double> stop_times_v = getKey<vector<double>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_STOP_TIME_KEY);
              vector<size_t> file_index_v = getKey<vector<size_t>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_SS_TIME_INDICES_KEY); 
              vector<string> file_paths_v = getKey<vector<string>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_TIME_FILES_KEY); 

              time_indices->file_paths = file_paths_v;
              
              // load start_times 
              for(size_t i = 0; i < start_times_v.size(); i++) {
                time_indices->start_times[start_times_v[i]] = file_index_v[i];
                time_indices->stop_times[stop_times_v[i]] = file_index_v[i];
              }

              found = true;
            }
            catch (runtime_error &e) { 
              // should probably replace with a more specific exception 
              SPDLOG_TRACE("Couldn't find "+DB_SPICE_ROOT_KEY+"/" + key+ ". " + e.what());
              continue;
            }
          }
          if (enforce_quality) break; // only interate once if quality is enforced 
        }

        if (time_indices) { 
          SPDLOG_TRACE("NUMBER OF KERNELS: {}", time_indices->file_paths.size());
          SPDLOG_TRACE("NUMBER OF START TIMES: {}", time_indices->start_times.size());
          SPDLOG_TRACE("NUMBER OF STOP TIMES: {}", time_indices->stop_times.size()); 
        } else { 
          // no kernels found 
          continue;
        }
        size_t iterations = 0; 
      
        // init containers
        unordered_set<size_t> start_time_kernels; 
        vector<string> final_time_kernels;

        // Get everything starting before the stop_time; 
        auto start_upper_bound = time_indices->start_times.upper_bound(stop_time);
        for(auto it = time_indices->start_times.begin() ;it != start_upper_bound; it++) {
          iterations++;
          start_time_kernels.insert(it->second);             
        }

        SPDLOG_TRACE("NUMBER OF KERNELS MATCHING START TIME: {}", start_time_kernels.size()); 

        // Get everything stopping after the start_time; 
        auto stop_lower_bound = time_indices->stop_times.lower_bound(start_time);
        if(time_indices->stop_times.end() == stop_lower_bound && start_time_kernels.contains(stop_lower_bound->second)) { 
          final_time_kernels.push_back(time_indices->file_paths.at(stop_lower_bound->second));
        }
        else { 
          for(auto &it = stop_lower_bound;it != time_indices->stop_times.end(); it++) { 
            // if it's also in the start_time set, add it to the list
            iterations++;
            
            if (start_time_kernels.contains(it->second)) {
              final_time_kernels.push_back(data_dir / time_indices->file_paths.at(it->second));
            }
          } 
        }
        if (final_time_kernels.size()) { 
          kernels[Kernel::translateType(type)] = final_time_kernels;
          kernels[qkey] = Kernel::translateQuality(quality);
        }
        SPDLOG_TRACE("NUMBER OF ITERATIONS: {}", iterations);
        SPDLOG_TRACE("NUMBER OF KERNELS FOUND: {}", final_time_kernels.size());  
      }
      else { // text/non time based kernels
        SPDLOG_DEBUG("Trying to search time independant kernels");
        string key = instrument+"/"+Kernel::translateType(type)+"/"+"kernels"; 
        SPDLOG_DEBUG("GETTING {} with key {}", Kernel::translateType(type), key);
        if (m_nontimedep_kerns.contains(key) && !m_nontimedep_kerns[key].empty()) {  
          vector<string> ks = m_nontimedep_kerns[key]; 
          for(auto &e : ks) e = data_dir / e; // re-add the data dir 
          kernels[Kernel::translateType(type)] = ks;
        
        }
        else { 
          // load from DB 
          try { 
            vector<string> ks = getKey<vector<string>>(DB_SPICE_ROOT_KEY + "/"+key);
            for(auto &e : ks) e = data_dir / e; // re-add the data dir
            kernels[Kernel::translateType(type)] = ks;
          } catch (runtime_error &e) { 
            SPDLOG_TRACE("{}", e.what()); // usually a key not found error
            continue;
          }
        }
      }
    }
    return kernels;
  }
  

  void InventoryImpl::write_database() { 
    fs::path db_root = getCacheDir(); 
    string hdf_file = db_root / DB_HDF_FILE;
    
    // delete if exists
    fs::remove(hdf_file);
    H5Easy::File file(hdf_file, H5Easy::File::Create); 

    for (auto it=m_timedep_kerns.begin(); it!=m_timedep_kerns.end(); ++it) {
      string kernel_key = it->first; 
      TimeIndexedKernels *kernels = m_timedep_kerns[kernel_key];       

      /* Save HDF files */
      if (kernels->file_paths.size() > 0) {
        // save index
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_TIME_FILES_KEY, kernels->file_paths, H5Easy::DumpMode::Overwrite);

        // preallocate everything 
        vector<double> start_times_v;
        start_times_v.reserve(kernels->start_times.size());
        vector<double> stop_times_v;
        stop_times_v.reserve(kernels->stop_times.size());
        vector<size_t> indices_v;
        indices_v.reserve(kernels->start_times.size());

        for (const auto &[k, v] : kernels->start_times) { 
          start_times_v.push_back(k);
          indices_v.push_back(v);
        }

        for (const auto &[k, v] : kernels->stop_times) { 
          stop_times_v.push_back(k);
        } 

        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_START_TIME_KEY, start_times_v, H5Easy::DumpMode::Overwrite);
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_STOP_TIME_KEY, stop_times_v, H5Easy::DumpMode::Overwrite);
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_SS_TIME_INDICES_KEY, indices_v, H5Easy::DumpMode::Overwrite);
      }
    }

    /* Save HDF file */
    {
      for(auto &e : m_nontimedep_kerns) { 
        if(e.second.size() > 0) {
          SPDLOG_DEBUG("Writing {} of with {} kernels.", e.first, e.second.size());
          H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+e.first, e.second, H5Easy::DumpMode::Overwrite);
        }
      }
    }

  }

};

