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


#include "config.h"
#include "database.h"
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
          
          index = kernel_times->index.size(); 

          // cant contruct these in line for whatever reason 
          fc::BTreePair<double, size_t> p;
          p.first = sstimes.second; 
          p.second = index; 
          kernel_times->start_times.insert(p);

          fc::BTreePair<double, size_t> p2;
          p2.first = sstimes.second; 
          p2.second = index;  
          kernel_times->stop_times.insert(p2);
          kernel_times->index.push_back(kernel);
        }
      }
    }
  }


  Database::Database(bool force_regen) : m_required_kernels() { 
    fs::path db_root = get_root_dir();

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

        if (mission == "Base") { 
            continue;
        }

        for(auto &[kernel_type, kernel_obj] : kernels.items()) { 
          if (kernel_type == "ck" || kernel_type == "spk") { 
            // we need to log the times
            for (auto &quality : Kernel::QUALITIES) {
              if (kernel_obj.contains(quality)) {

                // make the keys match Config's nested keys
                string map_key = mission + ":" + kernel_type +":"+quality+":kernels";
                
                // make sure no path symbols are in the key
                replaceAll(map_key, "/", ":");
                
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
              string btree_key = mission + ":" + kernel_type + ":kernels";
              replaceAll(btree_key, "/", ":"); 
              // preallocate a vector
              vector<string> kernel_vec;

              // Doing it bracketless, there are too many brackets
              for (auto &subarr: kernel_obj[ptr]) 
                for (auto &kernel : subarr) 
                  kernel_vec.push_back(kernel); 
              m_nontimedep_kerns[btree_key] = kernel_vec; 
            } 
          }
        } 
      }

      // write everything out
      write_database();
    }
    else { // load the database 
      read_database(); 
    }

  }

  string Database::get_root_dir() { 
    fs::path cache_dir = Memo::getCacheDir();
    return cache_dir / "dbindexes"; 
  }


  void Database::read_database() { 
    fs::path db_root = get_root_dir(); 

    // kernels are simple, store as a binary archive
    std::ifstream ofs_index(db_root/"non_time_dep.bin");
    cereal::BinaryInputArchive ia(ofs_index);
    ia >> m_nontimedep_kerns;

    // initialize empty time indexes by deleting existing keys
    for (auto it=m_timedep_kerns.begin(); it!=m_timedep_kerns.end(); ++it) { 
      // should naturally delete pointers
      m_timedep_kerns.erase(it); 
    } 
  }


  json Database::search_for_kernelset(string instrument, vector<Kernel::Type> types, double start_time, double stop_time,
                                  Kernel::Quality ckQuality, Kernel::Quality spkQuality, bool enforce_quality) { 
    // get time dep kernels first 
    json kernels;
    if (start_time > stop_time) { 
      throw range_error("start time cannot be greater than stop time.");
    }

    for (auto &type: types) { 
      // load time kernels
      if (type == Kernel::Type::CK || type == Kernel::Type::SPK) { 
        TimeIndexedKernels *time_indices = nullptr;
        bool found = false;        

        Kernel::Quality quality = spkQuality;
        string qkey = "spkQuality";
        if (type == Kernel::Type::CK) { 
          quality = ckQuality;
          qkey = "ckQuality";
        }

        // iterate down the qualities 
        for(int i = (int)quality; i > 0 && !found; i--) { 
          string key = instrument+":"+Kernel::translateType(type)+":"+Kernel::QUALITIES.at(i)+":"+"kernels";
          SPDLOG_DEBUG("Key: {}", key);

          if (m_timedep_kerns.contains(key)) { 
            SPDLOG_DEBUG("Key {} found", key); 
            
            // we can get the key 
            time_indices = m_timedep_kerns[key]; 
            found = true;
            quality = (Kernel::Quality)i; 
          }
          else { 
            // try to load the memory mapped files 
            fs::path cache = Memo::getCacheDir(); 
            cache /= "dbindexes" / (fs::path)key;
            SPDLOG_TRACE("Loading indices from directory: {}", (string)cache);
            
            if(!fs::exists(cache)) { 
              // skip 
              SPDLOG_TRACE("cache {} does not exist.", (string)cache);
              continue;
            }
            found = true;
            time_indices = new TimeIndexedKernels();
            quality = (Kernel::Quality)i; 

            ifstream start_times_ifs{cache/"start_time.bin", ios_base::in | ios_base::binary};
            start_times_ifs >> time_indices->start_times;
            ifstream stop_times_ifs{cache/"stop_time.bin", ios_base::in | ios_base::binary};
            stop_times_ifs >> time_indices->stop_times;        
            // indices are simple, store as a binary archive
            std::ifstream ifs_index(cache/"index.bin");
            cereal::BinaryInputArchive ia(ifs_index);
            ia >> time_indices->index; 
          }
          if (enforce_quality) break; // only interate once if quality is enforced 
        }

        if (time_indices) { 
          SPDLOG_TRACE("NUMBER OF KERNELS: {}", time_indices->index.size());
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
          start_time_kernels.insert(it->second);             
        }

        // Get everything stopping after the start_time; 
        auto stop_lower_bound = time_indices->stop_times.lower_bound(start_time);
        for(auto &it = stop_lower_bound;it != time_indices->stop_times.end(); it++) { 
          // if it's also in the start_time set, add it to the list
          iterations++;
          
          if (start_time_kernels.contains(it->second)) {
            final_time_kernels.push_back(time_indices->index.at(it->second));
          }
        } 
        if (final_time_kernels.size()) { 
          kernels[Kernel::translateType(type)] = final_time_kernels;
          kernels[qkey] = Kernel::translateQuality(quality);
        }
        SPDLOG_TRACE("NUMBER OF ITERATIONS: {}", iterations); 
      }
      else { // text/non time based kernels
        string key = instrument+":"+Kernel::translateType(type)+":"+"kernels"; 
        SPDLOG_DEBUG("GETTING {} with key {}", Kernel::translateType(type), key);
        if (m_nontimedep_kerns.contains(key) && !m_nontimedep_kerns[key].empty()) 
          kernels[Kernel::translateType(type)] = m_nontimedep_kerns[key];
      }
    }
    return kernels;
  }
  

  void Database::write_database() { 
    fs::path db_root = get_root_dir(); 

    for (auto it=m_timedep_kerns.begin(); it!=m_timedep_kerns.end(); ++it) {
      string kernel_key = it->first;         

      // make sure no path symbols are in the key
      fs::path db_subdir = db_root / kernel_key;
      
      fs::create_directories(db_subdir);
      
      TimeIndexedKernels *kernels = m_timedep_kerns[it->first];       
      
      // kernels are simple, store as a binary archive
      std::ofstream ofs_index(db_subdir/"index.bin");
      cereal::BinaryOutputArchive oa(ofs_index);
      oa << kernels->index;
      
      // the rest as memory mapped files 
      std::ofstream ofs_start_time{db_subdir/"start_time.bin", ios_base::out|ios_base::binary|ios_base::trunc};
      ofs_start_time << kernels->start_times;
      std::ofstream ofs_stop_time{db_subdir/"stop_time.bin", ios_base::out|ios_base::binary|ios_base::trunc};
      ofs_stop_time << kernels->stop_times;
    }

    db_root /= "non_time_dep.bin";
    std::ofstream ofs(db_root);
    cereal::BinaryOutputArchive oa(ofs);
    oa << m_nontimedep_kerns;
  }

};

