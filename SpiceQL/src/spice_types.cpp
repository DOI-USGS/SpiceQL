/**
  * @file
  *
  *
 **/

#include <fmt/format.h>
#include <SpiceUsr.h>

#include <nlohmann/json.hpp>

#include <ghc/fs_std.hpp>

#include <spdlog/spdlog.h>

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


  const std::vector<std::string> Kernel::TYPES =  { "na", "ck", "spk", "tspk",
                                                    "lsk", "mk", "sclk",
                                                    "iak", "ik", "fk",
                                                    "dsk", "pck", "ek"};

  const std::vector<std::string> Kernel::QUALITIES = { "na",
                                                       "predicted",
                                                       "nadir",
                                                       "reconstructed",
                                                       "smithed"};


  string Kernel::translateType(Kernel::Type type) {
    return Kernel::TYPES[static_cast<int>(type)];
  }


  Kernel::Type Kernel::translateType(string type) {
    auto res = findInVector<string>(Kernel::TYPES, type);
    if (res.first) {
      return static_cast<Kernel::Type>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", type));
  };


  string Kernel::translateQuality(Kernel::Quality qa) {
    return Kernel::QUALITIES[static_cast<int>(qa)];
  }


  Kernel::Quality Kernel::translateQuality(string qa) {
    auto res = findInVector<string>(Kernel::QUALITIES, qa);
    if (res.first) {
      return static_cast<Kernel::Quality>(res.second);
    }

    std::string qualities = std::accumulate(Kernel::QUALITIES.begin(), Kernel::QUALITIES.end(), std::string(", "));
    throw invalid_argument(fmt::format("{} is not a valid kernel type, available types are: {}", qa, qualities));
  }


  int translateNameToCode(string frame, string mission, bool searchKernels) {    
    SpiceInt code;
    SpiceBoolean found;
    json kernelsToLoad = {};

    if (mission != "" && searchKernels) {
      kernelsToLoad = loadTranslationKernels(mission);
    } 
    KernelSet kset(kernelsToLoad);

    checkNaifErrors();
    bodn2c_c(frame.c_str(), &code, &found);
    checkNaifErrors();

    if (!found) {
      namfrm_c(frame.c_str(), &code);
      checkNaifErrors();
    }

    if (code == 0) {
      throw invalid_argument(fmt::format("Frame code for frame name [{}] not found.", frame));
    }

    return code;
  }


  string translateCodeToName(int frame, string mission, bool searchKernels) {
    SpiceChar name[128];
    SpiceBoolean found;
    json kernelsToLoad = {};

    if (mission != "" && searchKernels){
      kernelsToLoad = loadTranslationKernels(mission);
    }
    KernelSet kset(kernelsToLoad);

    checkNaifErrors();
    bodc2n_c(frame, 128, name, &found);
    checkNaifErrors();

    if(!found) {  
      frmnam_c(frame, 128, name);
      checkNaifErrors();
    }

    if(strlen(name) == 0) {
       throw invalid_argument(fmt::format("Frame name for code {} not found.", frame));
    }
    
    return string(name);
  }

  vector<int> getFrameInfo(int frame, string mission, bool searchKernels) {
    SpiceInt cent;
    SpiceInt frclss;
    SpiceInt clssid;
    SpiceBoolean found;

    json kernelsToLoad = {};

    if (mission != "" && searchKernels) {
      // Load only the FKs
      kernelsToLoad = loadTranslationKernels(mission, true, false, false);
    }
    KernelSet kset(kernelsToLoad);

    checkNaifErrors();
    frinfo_c(frame, &cent, &frclss, &clssid, &found);
    checkNaifErrors();
    SPDLOG_TRACE("RETURN FROM FRINFO: {}, {}, {}, {}", cent, frclss, clssid, found);

    if (!found) {
       throw invalid_argument(fmt::format("Frame info for code {} not found.", frame));
    }

    return {cent, frclss, clssid};
  }

  Kernel::Kernel(string path) {
    this->path = path;
    KernelPool::getInstance().load(path, true);
  }


  Kernel::Kernel(Kernel &other) {
    KernelPool::getInstance().load(other.path);
    this->path = other.path;
  }


  Kernel::~Kernel() {
    KernelPool::getInstance().unload(this->path);
  }


  double utcToEt(string utc, bool searchKernels) {
      Config conf;
      conf = conf["base"];
      json lsks = {};

      // get lsk kernel
      if (searchKernels) {
       lsks = conf.getLatest("lsk");
      }

      KernelSet lsk(lsks);

      SpiceDouble et;
      checkNaifErrors();
      str2et_c(utc.c_str(), &et);
      checkNaifErrors();

      return et;
  }

  string etToUtc(double et, string format, double precision, bool searchKernels) {
      Config conf;
      conf = conf["base"];
      json lsks = {};

      // get lsk kernel
      if (searchKernels) {
       lsks = conf.getLatest("lsk");
      }

      KernelSet lsk(lsks);

      SpiceChar utc_spice[100]; 
      checkNaifErrors();
      et2utc_c(et, format.c_str(), precision, 100, utc_spice);
      checkNaifErrors();
      string utc_string(utc_spice);
      return utc_string;
  }

  double strSclkToEt(int frameCode, string sclk, string mission, bool searchKernels) {
      Config missionConf;
      json sclks;

      if (searchKernels) {
        sclks = loadSelectKernels("sclk", mission);
      }

      KernelSet sclkSet(sclks);

      SpiceDouble et;
      checkNaifErrors();
      scs2e_c(frameCode, sclk.c_str(), &et);
      checkNaifErrors();
      SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);
      
      return et;
  }

  double doubleSclkToEt(int frameCode, double sclk, string mission, bool searchKernels) {
      Config missionConf;
      json sclks;

      if (searchKernels) {
        sclks = loadSelectKernels("sclk", mission);
      }

      KernelSet sclkSet(sclks);

      SpiceDouble et;
      checkNaifErrors();
      sct2e_c(frameCode, sclk, &et);
      checkNaifErrors();
      SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);

      return et;
  }

  json findMissionKeywords(string key, string mission, bool searchKernels) {
    json translationKernels = {};

    if (mission != "" && searchKernels) {
      translationKernels = loadTranslationKernels(mission);
    }

    KernelSet kset(translationKernels);

    return findKeywords(key);
  }


  json findTargetKeywords(string key, string mission, bool searchKernels) {
    json kernelsToLoad = {};

    if (mission != "" && searchKernels) {
      kernelsToLoad["base"] = loadSelectKernels("pck", "base");
      kernelsToLoad[mission] = loadSelectKernels("pck", mission);
    }
    KernelSet kset(kernelsToLoad);
    return findKeywords(key);
  }


  json getTargetFrameInfo(int targetId, string mission, bool searchKernels) {
    SpiceInt frameCode;
    SpiceChar frameName[128];
    SpiceBoolean found;

    json frameInfo;
    json kernelsToLoad = {};

    if (mission != "" && searchKernels) {
      kernelsToLoad["base"] = loadSelectKernels("pck", "base");
      kernelsToLoad[mission] = loadSelectKernels("pck", mission);
    }
    KernelSet kset(kernelsToLoad);

    checkNaifErrors();
    cidfrm_c(targetId, 128, &frameCode, frameName, &found);
    checkNaifErrors();

    if(!found) {  
      throw invalid_argument(fmt::format("Frame info for target id {} not found.", targetId));
    }

    frameInfo["frameCode"] = frameCode;
    frameInfo["frameName"] = frameName;

    return frameInfo;
  }


  int KernelPool::load(string path, bool force_refurnsh) {
    SPDLOG_DEBUG("Furnishing {}, force refurnish? {}.", path, force_refurnsh);

    int refCount; 

    auto it = refCounts.find(path);

    if (it != refCounts.end()) {
      SPDLOG_TRACE("{} already furnished.", path);

      // it's been furnished before, increment ref count
      it->second += 1;
      refCount = it->second; 
 
      if (force_refurnsh) {
        checkNaifErrors();
        furnsh_c(path.c_str());
        checkNaifErrors();
      }
    }
    else { 
      refCount = 1;  
      // load the kernel and register in onto the kernel map 
      checkNaifErrors();
      furnsh_c(path.c_str());
      checkNaifErrors();
      refCounts.emplace(path, 1);
    }


    SPDLOG_TRACE("refcout of {}: {}", path, refCount);
    return refCount;
  }


  int KernelPool::unload(string path) {
    try { 
      int &refcount = refCounts.at(path);
      
      // if the map contains the last copy of the kernel, delete it
      if (refcount == 1) {
        // unfurnsh the kernel
        checkNaifErrors();
        unload_c(path.c_str());
        checkNaifErrors();
        
        refCounts.erase(path);
        return 0;
      }
      else {
        checkNaifErrors();
        unload_c(path.c_str());
        checkNaifErrors();
        
        refcount--;
        
        return refcount;
      }
    }
    catch(out_of_range &e) {
      throw out_of_range(path + " is not a kernel that has been loaded."); 
    }
  }


  unsigned int KernelPool::getRefCount(std::string key) {
    try {
      return refCounts.at(key);
    } catch(out_of_range &e) {
      return 0;
    }
  }


  unordered_map<string, int> KernelPool::getRefCounts() {
    return refCounts;
  }


  KernelPool &KernelPool::getInstance() {
    static KernelPool pool;
    return pool;
  }


  KernelPool::KernelPool() : refCounts() { 
    loadLeapSecondKernel();

    checkNaifErrors();
    // create aliases for spacecrafts 
    boddef_c("mess", -236); // NAIF uses MESSENGER, we use mess for short
    checkNaifErrors();
  
  }


  vector<string> KernelPool::getLoadedKernels() {
    vector<string> res;

    for( const auto& [key, value] : refCounts ) {
      res.emplace_back(key);
    }
    return res;
  }

  void KernelPool::loadClockKernels() { 
    json clocks;

    // if data dir not set, should raise an exception 
    fs::path dataDir = getDataDirectory();

    vector<json> confs = getAvailableConfigs();
    // get SCLKs
    for(auto &j : confs) {
      vector<json::json_pointer> p = findKeyInJson(j, "sclk", true);
      
      if (!p.empty()) {
        json sclks = j[p.at(0)];
        clocks[p.at(0)] = sclks;
      }
    }
  
    clocks = listMissionKernels(dataDir, clocks);
    clocks = getLatestKernels(clocks);

    vector<json::json_pointer> kpointers = findKeyInJson(clocks, "kernels", true);
    for (auto &p : kpointers) {
        json sclks = clocks[p];
        
        for (auto &e : sclks) { 
          load(e.get<string>());
        }
    }
  }


  void KernelPool::loadLeapSecondKernel() {
    // get the distribution's LSK
    fs::path dbPath = getConfigDirectory();
    string lskPath = dbPath / "kernels" / "naif0011.tls";
    load(lskPath);
  }


  KernelSet::KernelSet(json kernels) {
    load(kernels);
  }

  void KernelSet::load(json kernels) { 
    SPDLOG_TRACE("Creating Kernelset: {}", kernels.dump());
    this->m_kernels.merge_patch(kernels);

    vector<string> kv = getKernelsAsVector(kernels);
  
    vector<SharedKernel> res;
    for (auto &k : kv) {
      SPDLOG_TRACE("Creating shared kernel {}", k);
      SharedKernel sk(new Kernel(k));
      res.emplace_back(sk);
    }
    loadedKernels.insert(loadedKernels.end(), res.begin(), res.end());
  }

} 
 