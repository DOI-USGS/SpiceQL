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


  int translateNameToCode(string frame, string mission, bool searchKernels) {    
    SpiceInt code;
    SpiceBoolean found;
    json kernelsToLoad = {};

    if (mission != "" && searchKernels) {
      kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk"});
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
      kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk"});
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
      kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk"});
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
    load(path, true);
  }


  // Kernel::Kernel(Kernel &other) {
  //   load(other.path);
  //   this->path = other.path;
  // }


  Kernel::~Kernel() {
    unload(this->path);
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
       lsks = Inventory::search_for_kernelset("base", {"lsk"});
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
      SPDLOG_TRACE("calling strSclkToEt({}, {}, {}, {})", frameCode, sclk, mission, searchKernels);
      json sclks;
      json lsks;
      if (searchKernels) {
        lsks = Inventory::search_for_kernelset("base", {"lsk"}); 
        sclks = Inventory::search_for_kernelset(mission, {"fk", "sclk"});
      }

      KernelSet sclkSet(sclks);
      KernelSet lskSet(lsks);
      
      SpiceDouble et;
      checkNaifErrors();
      try {
        scs2e_c(frameCode, sclk.c_str(), &et);
        checkNaifErrors();
        SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);
      }
      catch(exception &e) { 
        // we want the platforms code, if they passs in an instrument code (e.g. -85600), truncate it to (-85)
        frameCode = (abs(frameCode / 1000) > 0) ? frameCode/1000 : frameCode;
        scs2e_c(frameCode, sclk.c_str(), &et);
        checkNaifErrors();
        SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et); 
      }
      return et;
  }

  double doubleSclkToEt(int frameCode, double sclk, string mission, bool searchKernels) {
      Config missionConf;
      json sclks;

      if (searchKernels) {
        // sclks = loadSelectKernels("sclk", mission);
        sclks = Inventory::search_for_kernelset(mission, {"lsk", "fk", "sclk"});
      }

      KernelSet sclkSet(sclks);
      
      // we want the platforms code, if they passs in an instrument code (e.g. -85600), truncate it to (-85)
      frameCode = (abs(frameCode / 1000) > 0) ? frameCode/1000 : frameCode; 

      SpiceDouble et;
      checkNaifErrors();
      sct2e_c(frameCode, sclk, &et);
      checkNaifErrors();
      SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);

      return et;
  }


  string doubleEtToSclk(int frameCode, double et, string mission, bool searchKernels) {
      Config missionConf;
      json sclks;

      if (searchKernels) {
        sclks = Inventory::search_for_kernelset(mission, {"lsk", "fk", "sclk"});
      }

      KernelSet sclkSet(sclks);

      SpiceChar sclk[100];
      checkNaifErrors();
      sce2s_c(frameCode, et, 100, sclk);
      checkNaifErrors();
      SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);

      return string(sclk);
  }

  json findMissionKeywords(string key, string mission, bool searchKernels) {
    json translationKernels = {};

    if (mission != "" && searchKernels) {
      translationKernels = Inventory::search_for_kernelset(mission, {"iak", "fk", "ik"});
    }

    KernelSet kset(translationKernels);

    return findKeywords(key);
  }


  json findTargetKeywords(string key, string mission, bool searchKernels) {
    json baseKernels = {};
    json missionKernels = {};

    if (mission != "" && searchKernels) {
      baseKernels = Inventory::search_for_kernelset("base", {"pck"});
      missionKernels = Inventory::search_for_kernelset(mission, {"pck"});
    }
    KernelSet baseKset(baseKernels);
    KernelSet missionKset(missionKernels);
    return findKeywords(key);
  }


  json getTargetFrameInfo(int targetId, string mission, bool searchKernels) {
    SpiceInt frameCode;
    SpiceChar frameName[128];
    SpiceBoolean found;

    json frameInfo;
    json baseKernels = {};
    json missionKernels = {};

    if (mission != "" && searchKernels) {
      baseKernels = Inventory::search_for_kernelset("base", {"fk"});
      missionKernels = Inventory::search_for_kernelset(mission, {"fk"});
    }
    
    KernelSet baseKset(baseKernels);
    KernelSet missionKset(missionKernels);

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

  void load(string path, bool force_refurnsh) {
    SPDLOG_DEBUG("Furnishing {}, force refurnish? {}.", path, force_refurnsh); 
    checkNaifErrors();
    furnsh_c(path.c_str());
    checkNaifErrors();
  }

  void unload(string path) {
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
    
    vector<string> kv = getKernelsAsVector(kernels);

    for (auto &k : kv) {
      SPDLOG_TRACE("Creating shared kernel {}", k);
      if (!fs::exists(k)) { 
        throw runtime_error("Kernel " + k + " does not exist");
      }
      
      try { 
        Kernel *kp = new Kernel(k);
        m_loadedKernels.emplace_back(kp);
      } catch (exception &e) { 
        throw runtime_error("something went wrong: " + string(e.what()));
      }
    }
  }

  KernelSet::~KernelSet() { 
    for(auto p : m_loadedKernels) { 
      delete p;
    }
    m_loadedKernels.clear();
  }

} 
 
