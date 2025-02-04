/**
  * @file
  *
  *
  *
  *
 **/
#include <algorithm>
#include <chrono>
#include <fstream>

#include <SpiceUsr.h>

#include <ghc/fs_std.hpp>
#include <spdlog/spdlog.h>

#include <spdlog/spdlog.h>

#include "query.h"
#include "spice_types.h"
#include "utils.h"
#include "memoized_functions.h"
#include "config.h"

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

namespace SpiceQL {


 std::string getKernelStringValue(std::string key) {
   // check to make sure the key exists when calling findKeyWords(key)
   if (findKeywords(key).contains(key)){
      json results = findKeywords(key);
      std::string keyResult;
      if (results[key].is_string()) {
          keyResult = results[key];
      }
      else {
          keyResult = results[key].dump();
      }
      return keyResult;
    }
    // throw exception
    else{
      throw std::invalid_argument("key not in results");
    }
  }

  std::vector<string> getKernelVectorValue(std::string key) {

    // check to make sure the key exists when calling findKeyWords(key)
    if (findKeywords(key).contains(key)){

      // get json results of key
      json results = findKeywords(key);
      vector<string> kernelValues;

      // iterate over results @ key
      for(auto i : results[key]){
        // push values to vector
        kernelValues.push_back(to_string(i));
      }
      return kernelValues;
    }
    // throw exception
    else{
      throw std::invalid_argument("key not in results");
    }
  }

  // Comparator function for kernel paths
  bool fileNameComp(string a, string b) {
      string fna = static_cast<fs::path>(a).filename();
      string fnb = static_cast<fs::path>(b).filename();
      int comp = fna.compare(fnb);  
      SPDLOG_TRACE("Comparing {} and {}: {}", fna, fnb, comp);
      return comp < 0;
  }

  vector<string> getLatestKernel(vector<string> kernels) {
    if(kernels.empty()) {
      throw invalid_argument("Can't get latest kernel from empty vector");
    }
    vector<vector<string>> files = {};

    string extension = static_cast<fs::path>(kernels.at(0)).extension();
    transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    // ensure everything is different versions of the same file
    for(const fs::path &k : kernels) {
      string currentKernelExt = k.extension();
      transform(currentKernelExt.begin(), currentKernelExt.end(), currentKernelExt.begin(), ::tolower);
      if (currentKernelExt != extension) {
        throw invalid_argument("The input extensions (" + (string)k.filename() + ") are not different versions of the same file " + kernels.at(0));
      }
      bool foundList = false;
      for (int i = 0; i < files.size(); i++) {
        
        const fs::path &firstVecElem = files[i][0];
        string fileName = firstVecElem.filename();
        string kernelName = k.filename();
        SPDLOG_TRACE("filename: {}", fileName); 
        SPDLOG_TRACE("kernel name: {}", kernelName); 

        int findRes = fileName.find_first_of("0123456789");
        if (findRes != string::npos) {
          fileName = fileName.erase(findRes);
        }

        findRes = kernelName.find_first_of("0123456789");
        if (findRes != string::npos) {
          kernelName = kernelName.erase(findRes);
        }
        SPDLOG_TRACE("Truncated filename: {}", fileName); 
        SPDLOG_TRACE("Truncated kernel name: {}", kernelName); 

        if (fileName == kernelName) {
          files[i].push_back(k);
          foundList = true;
        }
      }
      if (!foundList) {
        files.push_back({k});
      }
      foundList = false;
    }

    vector<string> outKernels = {};
    for (auto kernelList : files) {
      outKernels.push_back(*(max_element(kernelList.begin(), kernelList.end(), fileNameComp)));
    }

    return outKernels;
  }


  json getLatestKernels(json kernels) {
    SPDLOG_TRACE("Looking for kernels to get Latest: {}", kernels.dump(2));
    vector<json::json_pointer> kptrs = findKeyInJson(kernels, "kernels", true);
    vector<vector<string>> lastest;

    for (json::json_pointer &ptr : kptrs) {
      SPDLOG_TRACE("Getting Latest Kernels from: {}", ptr.to_string());
      SPDLOG_TRACE("JSON: {}", kernels[ptr].dump());
      vector<vector<string>> kvect = json2DArrayTo2DVector(kernels[ptr]);
      vector<vector<string>> newLatest;
 
      for (auto &vec : kvect) {
        vector<string> latest = getLatestKernel(vec);
        SPDLOG_TRACE("Adding Kernels To Latest: {}", fmt::join(latest, ", "));
        newLatest.push_back(latest);
      }
      
      kernels[ptr] = newLatest;
    }

    return kernels;
  }


  json globKernels(string root, json conf, string kernelType) {
    SPDLOG_TRACE("globKernels({}, {}, {})", root, conf.dump(), kernelType);
    vector<json::json_pointer> pointers = findKeyInJson(conf, kernelType, true);

    json ret;

    // iterate pointers
    for(auto pointer : pointers) {
      json category = conf[pointer];

      if (category.contains("kernels")) {
        ret[pointer]["kernels"] = getPathsFromRegex(root, jsonArrayToVector(category.at("kernels")));
      }

      if (category.contains("deps")) {
        if (category.at("deps").contains("sclk")) {
          ret[pointer]["deps"]["sclk"] = getPathsFromRegex(root, category.at("deps").at("sclk"));
        }
        if (category.at("deps").contains("pck")) {
          ret[pointer]["deps"]["pck"] = getPathsFromRegex(root, category.at("deps").at("pck"));
        }
        if (category.at("deps").contains("objs")) {
          ret[pointer]["deps"]["objs"] = category.at("deps").at("objs");
        }
      }

      // iterate over potential qualities
      for(auto qual: KERNEL_QUALITIES) {
        if(!category.contains(qual)) {
          continue;
        }

        vector<vector<string>> binKernels = getPathsFromRegex(root, jsonArrayToVector(category[qual].at("kernels")));
        if (!binKernels.empty()) {
          ret[pointer][qual]["kernels"] = binKernels;
        } 

        if (category[qual].contains("deps")) {
          if (category[qual].at("deps").contains("sclk")) {
            ret[pointer][qual]["deps"]["sclk"] = getPathsFromRegex(root, jsonArrayToVector(category[qual].at("deps").at("sclk")));
          }
          if (category[qual].at("deps").contains("pck")) {
            ret[pointer][qual]["deps"]["pck"] = getPathsFromRegex(root, jsonArrayToVector(category[qual].at("deps").at("pck")));
          }
          if (category[qual].at("deps").contains("objs")) {
            ret[pointer][qual]["deps"]["objs"] = category[qual].at("deps").at("objs");
          }
        }
      }
    }

    SPDLOG_DEBUG("Kernels To Search: {}", conf.dump());
    SPDLOG_DEBUG("Kernels: {}", ret.dump());
    return  ret.empty() ? "{}"_json : ret;
  }


  json listMissionKernels(string root, json conf) {
    json kernels;

    // the kernels group is now the conf with
    for(auto &kernelType: {"ck", "spk", "tspk", "fk", "ik", "iak", "pck", "lsk", "sclk"}) {
        kernels.merge_patch(globKernels(root, conf, kernelType));
    }
    return kernels;
  }

  vector<string> getKernelsAsVector(json kernels) {
    SPDLOG_TRACE("geKernelsAsVector json: {}", kernels.dump());

    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels");
    vector<string> kernelVect;

    if (pointers.empty() && kernels.is_array()) {
      vector<vector<string>> ks = json2DArrayTo2DVector(kernels);
      for (auto &subarr : ks) { 
        kernelVect.insert(kernelVect.end(), subarr.begin(), subarr.end());
      } 
    }
    if (pointers.empty() && kernels.is_object()) { 
      // Assume it's in the format {"sclk" : ["path1", "path2"], "ck" : ["path1"], ...}  
      
      json iaks = {};
      // we want to furnish them last
      if(kernels.contains("iak")) { 
        iaks = kernels["iak"]; 
        kernels.erase("iak");
      }
      for (auto& [key, val] : kernels.items()) { 
        SPDLOG_TRACE("Getting Kernels of Type: {}", key);
        if(!val.empty() && val.is_array()) { 
           vector<string> ks = jsonArrayToVector(val);
          for (auto &kernel : ks) { 
            SPDLOG_TRACE("Adding: {}", kernel);
            kernelVect.push_back(kernel);
          } 
        }
      }

      // add iaks at the end
      if(!iaks.empty()) { 
        vector<string> ks = jsonArrayToVector(iaks);
        for (auto &kernel : ks) { 
            SPDLOG_TRACE("Adding: {}", kernel);
            kernelVect.push_back(kernel);
        }  
      }

    }
    else {
      for (auto & p : pointers) {
        if (!kernels[p].empty()) {
          vector<vector<string>> ks = json2DArrayTo2DVector(kernels[p]);
          for (auto &subarr : ks) { 
            kernelVect.insert(kernelVect.end(), subarr.begin(), subarr.end());
          }
        }
        else {
          SPDLOG_WARN("Unable to get {}, with kernels {}", p.to_string(), kernels[p].dump());
        }
      }    
    }
    return kernelVect;
  }


  set<string> getKernelsAsSet(json kernels) {
    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels");

    set<string> kset;
    
    if (pointers.empty() && kernels.is_array()) {
      vector<vector<string>> ks = json2DArrayTo2DVector(kernels);
      for (auto &subarr : ks) {
        for (auto &k : subarr) {
          kset.emplace(k);
        }
      }
    }
    else {
      for (auto & p : pointers) {
        vector<vector<string>> ks = json2DArrayTo2DVector(kernels[p]);
        for (auto &subarr : ks) {
          for (auto &k : subarr) {
            kset.emplace(k);
          }
        }
      }
    }    

    return kset;
  }
}
