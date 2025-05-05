#include <iostream>
#include <regex>

// we need to include this to overwrite and other std::fs imports
#include <ghc/fs_std.hpp>
#include <nlohmann/json.hpp>
#include <fc/btree.h>
#include <fc/disk_btree.h>
#include <spdlog/spdlog.h>
#include <queue>

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
#include "version.h"


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
  string DB_START_TIME_INDICES_KEY = "start_kindex"; 
  string DB_STOP_TIME_INDICES_KEY = "stop_kindex"; 


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
          while(kernel_times->start_times.contains(p.first)) { 
            p.first-=0.001; 
          } 
          kernel_times->start_times.insert(p);

          fc::BTreePair<double, size_t> p2;
          p2.first = sstimes.second; 
          p2.second = index; 

          while(kernel_times->stop_times.contains(p2.first)) { 
            p2.first+=0.001; 
          }  
          kernel_times->stop_times.insert(p2);

          // get relative path to make db portable 
          fs::path relative_path_kernel = fs::relative(kernel, fs::absolute(getDataDirectory()));
          SPDLOG_TRACE("Relative Kernel: {}", relative_path_kernel.generic_string()); 
          kernel_times->file_paths.push_back(relative_path_kernel);
        }
      }
    }
  }


  InventoryImpl::InventoryImpl(bool force_regen, vector<string> mlist) : m_required_kernels() { 
    fs::path db_root = getCacheDir();
    fs::path db_file = db_root / DB_HDF_FILE; 

    // create the database 
    if (!fs::exists(db_root) || force_regen) { 
      // get everything
      Config config; 
      json json_kernels = getLatestKernels(config.get()); 

      // load time kernels for creating the timed kernel DataBase 
      json lsk_json = getLatestKernels(config["base"].getRecursive("lsk")); 

      SPDLOG_TRACE("InventoryImpl LSKs: {}", lsk_json.dump(4));

      m_required_kernels.load(lsk_json); 

      for (auto &[mission, kernels] : json_kernels.items()) {
        SPDLOG_TRACE("MISSION: {}", mission);
        // if the mission list is populated and the mission is NOT in the list, continue 
        if(mlist.size() > 0 && std::find(mlist.begin(), mlist.end(), mission) == mlist.end()) { 
          continue;
        }

        json sclk_json = getLatestKernels(config[mission].getRecursive("sclk")); 
        SPDLOG_TRACE("{} SCLKs: {}", mission, sclk_json.dump(4)); 
        KernelSet sclks_ks(sclk_json); 

        for(auto &[kernel_type, kernel_obj] : kernels.items()) { 
          if (kernel_type == "ck" || kernel_type == "spk") { 
            // we need to log the times
            for (auto &quality : KERNEL_QUALITIES) {
              if (kernel_obj.contains(quality)) {

                // make the keys match Config's nested keys
                string map_key = mission + "/" + kernel_type +"/"+quality;
                
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
              string btree_key = mission + "/" + kernel_type;
              vector<string> kernel_vec;

              // Doing it bracketless, there are too many brackets
              for (auto &subarr: kernel_obj[ptr]) 
                for (auto &kernel : subarr) { 
                  string k = kernel.get<string>();
                  fs::path relative_path_kernel = fs::relative(k, fs::absolute(getDataDirectory()));
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
      throw runtime_error("Failed to get key [" + key + "] from [" + hdf_file + "].");
    }
  }




  json InventoryImpl::search_for_kernelsets(vector<string> spiceql_names, vector<Kernel::Type> types, double start_time, double stop_time,
                                  vector<Kernel::Quality> ckQualities, vector<Kernel::Quality> spkQualities, bool enforce_quality, bool overwrite) { 
      json kernels; 
      // simply iterate over the names
      for(auto &name : spiceql_names) { 
        json subKernels = search_for_kernelset(name, types, start_time, stop_time,
                                  ckQualities, spkQualities, enforce_quality); 
        SPDLOG_TRACE("subkernels for {}: {}", name, subKernels.dump(4));
        SPDLOG_TRACE("Overwrite? {}", overwrite);
        merge_json(kernels, subKernels, overwrite);
      }
      return kernels;
  }



  json InventoryImpl::search_for_kernelset(string spiceql_name, vector<Kernel::Type> types, double start_time, double stop_time,
                                  vector<Kernel::Quality> ckQualities, vector<Kernel::Quality> spkQualities, bool enforce_quality) { 
    // get time dep kernels first 
    json kernels;
    spiceql_name = toLower(spiceql_name);

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

        vector<Kernel::Quality> qualities = spkQualities;
        string qkey = spiceql_name+"_spk_quality";
        if (type == Kernel::Type::CK) { 
          qualities = ckQualities;
          qkey = spiceql_name+"_ck_quality";
        } 

        // sort in descending order
        std::sort(qualities.begin(), qualities.end(), std::greater<>());

        // iterate down the qualities
        for(auto quality = qualities.begin(); quality != qualities.end() && !found; ++quality) {
          string key = spiceql_name+"/"+Kernel::translateType(type)+"/"+Kernel::translateQuality(*quality)+"/";
          SPDLOG_DEBUG("Key: {}", key);

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
              vector<size_t> start_file_index_v = getKey<vector<size_t>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_START_TIME_INDICES_KEY); 
              vector<size_t> stop_file_index_v = getKey<vector<size_t>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_STOP_TIME_INDICES_KEY); 
              vector<string> file_paths_v = getKey<vector<string>>(DB_SPICE_ROOT_KEY+"/"+key+"/"+DB_TIME_FILES_KEY); 

              time_indices->file_paths = file_paths_v;
              SPDLOG_TRACE("Index, start time, stop time sizes: {}, {}, {}", start_file_index_v.size(), start_times_v.size(), stop_times_v.size());
              // load start_times 
              for(size_t i = 0; i < start_times_v.size(); i++) {
                time_indices->start_times[start_times_v[i]] = start_file_index_v[i];
              }
              for(size_t i = 0; i < stop_times_v.size(); i++) {
                time_indices->stop_times[stop_times_v[i]] = stop_file_index_v[i];
              }
            }
            catch (runtime_error &e) { 
              // should probably replace with a more specific exception 
              SPDLOG_TRACE("Couldn't find "+DB_SPICE_ROOT_KEY+"/" + key+ ". " + e.what());
              continue;
            }
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
          if(start_upper_bound == time_indices->start_times.begin() && start_upper_bound->first <= start_time)  { 
             iterations++;
             start_time_kernels.insert(start_upper_bound->second);  
          }
          for(auto it = time_indices->start_times.begin() ;it != start_upper_bound; it++) {
            iterations++;
            SPDLOG_TRACE("Inserting {} with the start time {}", time_indices->file_paths.at(it->second), it->first);
            start_time_kernels.insert(it->second);             
          }

          SPDLOG_TRACE("NUMBER OF KERNELS MATCHING START TIME: {}", start_time_kernels.size()); 

          // Get everything stopping after the start_time; 
          auto stop_lower_bound = time_indices->stop_times.lower_bound(start_time);
          if(time_indices->stop_times.end() == stop_lower_bound && stop_lower_bound->first >= stop_time && start_time_kernels.contains(stop_lower_bound->second)) { 
            SPDLOG_TRACE("Is {} in the array? {}", stop_lower_bound->second, start_time_kernels.contains(stop_lower_bound->second)); 
            final_time_kernels.push_back(time_indices->file_paths.at(stop_lower_bound->second));
          }
          else { 
            for(auto it = stop_lower_bound;it != time_indices->stop_times.end(); it++) { 
              // if it's also in the start_time set, add it to the list
              iterations++;
              SPDLOG_TRACE("Is {} with stop time {} in the array? {}", time_indices->file_paths.at(it->second), it->first, start_time_kernels.contains(it->second)); 
              if (start_time_kernels.contains(it->second)) {
                final_time_kernels.push_back(time_indices->file_paths.at(it->second));
                if (type == Kernel::Type::SPK) break;
              }
            } 
          }

          if (final_time_kernels.size()) { 
            found = true;
            if (type == Kernel::Type::SPK) { 
              // the SPK in the last position is the higher priority one 
              kernels[Kernel::translateType(type)] = {data_dir / final_time_kernels.back()};
              kernels[qkey] = Kernel::translateQuality(*quality);   
            }
            else { 
              for(string &e : final_time_kernels) { 
                e = data_dir / e;
              }
              kernels[Kernel::translateType(type)] = final_time_kernels;
              kernels[qkey] = Kernel::translateQuality(*quality);
            }
          }
          SPDLOG_TRACE("NUMBER OF ITERATIONS: {}", iterations);
          SPDLOG_TRACE("NUMBER OF KERNELS FOUND: {}", final_time_kernels.size());  

          if (enforce_quality) break; // only interate once if quality is enforced 
        }
      }
      else { // text/non time based kernels
        SPDLOG_DEBUG("Trying to search time independant kernels");
        string key = spiceql_name+"/"+Kernel::translateType(type)+"/"; 
        SPDLOG_DEBUG("GETTING {} with key {}", Kernel::translateType(type), key);
        if (m_nontimedep_kerns.contains(key) && !m_nontimedep_kerns[key].empty()) {  
          vector<string> ks = m_nontimedep_kerns[key]; 
          for(auto &e : ks) e =  data_dir / e; // re-add the data dir 
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

    // Write version
    HighFive::Group group = file.getGroup("/");
    group.createAttribute<std::string>("SPICEQL_VERSION", SPICEQL_VERSION);

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
        vector<size_t> start_indices_v;
        start_indices_v.reserve(kernels->start_times.size());
        vector<size_t> stop_indices_v;
        stop_indices_v.reserve(kernels->stop_times.size());

        for (const auto &[k, v] : kernels->start_times) { 
          start_times_v.push_back(k);
          start_indices_v.push_back(v);
        }

        for (const auto &[k, v] : kernels->stop_times) { 
          stop_times_v.push_back(k);
          stop_indices_v.push_back(v);
        } 

        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_START_TIME_KEY, start_times_v, H5Easy::DumpMode::Overwrite);
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_STOP_TIME_KEY, stop_times_v, H5Easy::DumpMode::Overwrite);
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_START_TIME_INDICES_KEY, start_indices_v, H5Easy::DumpMode::Overwrite);
        H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+kernel_key+"/"+DB_STOP_TIME_INDICES_KEY, stop_indices_v, H5Easy::DumpMode::Overwrite);
      }
    }

    /* Save HDF file */
    {
      for(auto &e : m_nontimedep_kerns) { 
        if(e.second.size() > 0) {
          SPDLOG_DEBUG("Writing {} with {} kernels.", DB_SPICE_ROOT_KEY + "/"+e.first, e.second.size());
          H5Easy::dump(file, DB_SPICE_ROOT_KEY + "/"+e.first, e.second, H5Easy::DumpMode::Overwrite);
        }
      }
    }
    
  }

};

