/**
  * @file
  *
  *
 **/

#include <fmt/format.h>
#include <SpiceUsr.h>

#include <nlohmann/json.hpp>

#include <ghc/fs_std.hpp>
#include <fmt/ranges.h>

#include <spdlog/spdlog.h>

#include "inventory.h"
#include "spice_types.h"
#include "query.h"
#include "utils.h"
#include "config.h"

using namespace std;
using json = nlohmann::json;

namespace SpiceQL {

  /**
   * @brief Used here to do reverse lookups of enum stringss
   **/
  template < typename T> pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element) {
    pair<bool, int > result;
    auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end()) {
      result.second = distance(vecOfElements.begin(), it);
      result.first = true;
    }
    else {
      result.first = false;
      result.second = -1;
    }
    return result;
  }


  const std::vector<std::string> KERNEL_TYPES =  { "na", "ck", "spk", "tspk",
                                                    "lsk", "mk", "sclk",
                                                    "iak", "ik", "fk",
                                                    "dsk", "pck", "ek"};

  const std::vector<std::string> KERNEL_QUALITIES = { "noquality",
                                                       "predicted",
                                                       "nadir",
                                                       "reconstructed",
                                                       "smithed"};


  string Kernel::translateType(Kernel::Type type) {
    return KERNEL_TYPES[static_cast<int>(type)];
  }


  Kernel::Type Kernel::translateType(string type) {
    auto res = findInVector<string>(KERNEL_TYPES, type);
    if (res.first) {
      return static_cast<Kernel::Type>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", type));
  };


  string Kernel::translateQuality(Kernel::Quality qa) {
    return KERNEL_QUALITIES[static_cast<int>(qa)];
  }


  Kernel::Quality Kernel::translateQuality(string qa) {
    if (qa.empty()) {
      qa = "smithed";
    }
    auto res = findInVector<string>(KERNEL_QUALITIES, qa);
    
    if (res.first) {
      return static_cast<Kernel::Quality>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type, available types are: {}", qa, fmt::join(KERNEL_QUALITIES, ", ")));
  }

  vector<Kernel::Quality> Kernel::translateQualities(vector<string> qas) {
    vector<Kernel::Quality> qualities = {};
    for (string qa : qas) {
      qualities.push_back(Kernel::translateQuality(qa));
    }
    return qualities;
  } 


  Kernel::Kernel(string path) {
    this->path = path;
    if (fs::exists(this->path)) {
      SPDLOG_TRACE("path is valid");
    } else {
      SPDLOG_TRACE("appending path to data_dir");
      this->path = getDataDirectory() / fs::path(path);
    }

    load(this->path, true);
  }


  // Kernel::Kernel(Kernel &other) {
  //   load(other.path);
  //   this->path = other.path;
  // }


  Kernel::~Kernel() {
    unload(this->path);
  }


  void load(string path, bool force_refurnsh) {
    SPDLOG_DEBUG("Furnishing {}, force refurnish? {}.", path, force_refurnsh); 
    checkNaifErrors();
    furnsh_c(path.c_str());
    checkNaifErrors();
  }

  void unload(string path) {
    SPDLOG_TRACE("Unloading kernel {}", path);
    checkNaifErrors();
    unload_c(path.c_str());
    checkNaifErrors();
  }

  KernelSet::KernelSet(json kernels) {
    load(kernels);
  }

  void KernelSet::load(json kernels) { 
    SPDLOG_TRACE("Creating Kernelset: {}", kernels.dump());
    this->m_kernels.merge_patch(kernels);
    vector<string> iaks = {};

    // If kernels have "iak" key, pop it and load first
    if (kernels.contains("iak")) {
      auto iak_value = kernels["iak"];
      iaks = jsonArrayToVector(iak_value);
      // Remove the "iak" key from kernels
      kernels.erase("iak");
    }

    vector<string> kv = getKernelsAsVector(kernels);
    fs::path data_dir = getDataDirectory();

    for (auto &k : kv) {
      SPDLOG_TRACE("Initial kernel {}", k);
      
      try { 
        Kernel *kp = new Kernel(k);
        m_loadedKernels.emplace_back(kp);
      } catch (exception &e) { 
        throw runtime_error("something went wrong: " + string(e.what()));
      }
    }
    
    SPDLOG_TRACE("Loaded {} non IAK kernels", m_loadedKernels.size());
    SPDLOG_TRACE("Loading {} IAKs", iaks.size());

    // load iaks last
    for (auto &iak : iaks) {
      try { 
        Kernel *kp = new Kernel(iak);
        m_loadedKernels.emplace_back(kp);
      } catch (exception &e) { 
        throw runtime_error("something went wrong: " + string(e.what()));
      }
    }
  }


  void KernelSet::unload() {
    for(auto p : m_loadedKernels) {
      delete p;
    }
    m_loadedKernels.clear();
  }

  KernelSet::~KernelSet() { 
    this->unload();
  }

} 
 
