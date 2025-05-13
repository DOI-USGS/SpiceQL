/**
 * 
 * 
 * 
 **/

#include <exception>
#include <fstream>
#include <regex>
#include <chrono>
#include <float.h>

#include <SpiceUsr.h>
#include <SpiceZfc.h>
#include <SpiceZmc.h>

#include <ghc/fs_std.hpp>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/compile.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.h"
#include "memo.h"
#include "memoized_functions.h"
#include "query.h"
#include "spice_types.h"
#include "utils.h"
#include "inventory.h"

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;
 
string calForm = "YYYY MON DD HR:MN:SC.###### TDB ::TDB";

// FMT formatter for fs::path, this enables passing path objects to FMT calls.
template <> struct fmt::formatter<fs::path> {
  char presentation = 'f';

  constexpr auto parse(format_parse_context& ctx) {
    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();
    if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  template <typename FormatContext>
  auto format(const fs::path& p, FormatContext& ctx) {
  // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
    // ctx.out() is an output iterator to write to.
    return format_to(
        ctx.out(),
        "{}",
        p.c_str());
  }
};


namespace SpiceQL {

  string gen_random(const int len) {
      size_t seed = 0;
      seed = Memo::hash_combine(seed, clock(), time(NULL), getpid());
      srand(seed);

      static const char alphanum[] =
          "0123456789"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz";
      string tmp_s;
      tmp_s.reserve(len);

      for (int i = 0; i < len; ++i) {
          tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      return tmp_s;
  }

  
  string toUpper(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
    return s;
  }


  string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return tolower(c); });
    return s;
  }
  

  string replaceAll(string str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
  }


  vector<vector<string>> getPathsFromRegex(string root, vector<string> regexes) {
    vector<string> files_to_search = Memo::ls(root, true);
      
    vector<vector<string>> kernels; 
    string temp;
    vector<string> paths;
    paths.reserve(files_to_search.size());
    
    for (auto &regex : regexes) { 
      paths.clear();
      SPDLOG_INFO("Searching for kernels matching {} in {} files", regex, files_to_search.size());
      
      for (auto &f : files_to_search) {
        temp = fs::path(f).filename();
        if (regex_search(temp.c_str(), basic_regex(regex, regex_constants::optimize|regex_constants::ECMAScript)) && temp.at(0) != '.' ) {
          paths.push_back(f);
        }
      }

      SPDLOG_DEBUG("found: {}", fmt::join(paths, ", "));
      if (!paths.empty()) { 
        kernels.push_back(paths);
      }
    }

    return kernels;
  }

  void mergeConfigs(json &baseConfig, const json &mergingConfig) {
    for (json::const_iterator it = mergingConfig.begin(); it != mergingConfig.end(); ++it) {
      if (baseConfig.contains(it.key())) {
        if (baseConfig[it.key()].is_object()) {
          if (it.value().is_object()) {
            mergeConfigs(baseConfig[it.key()], it.value());
          }
          else {
            throw invalid_argument("Invalid merge. Cannot merge an object with a non-object.");
          }
        }
        else {
          if (it.value().is_object()) {
            throw invalid_argument("Invalid merge. Cannot merge an object with a non-object.");
          }

          // Ensure that we are going to append to an array
          if (!baseConfig[it.key()].is_array()) {
            baseConfig[it.key()] = json::array({baseConfig[it.key()]});
          }

          if (it.value().is_array()) {
            baseConfig[it.key()].insert(baseConfig[it.key()].end(), it.value().begin(), it.value().end());
          }
          else {
            baseConfig[it.key()] += it.value();
          }
        }
      }

      else {
        baseConfig[it.key()] = it.value();
      }
    }
  }


  vector<double> getTargetState(double et, string target, string observer, string frame, string abcorr) {
    SPDLOG_TRACE("getTargetState(et={}, target={}, observer={}, frame={}, abcorr={})", et, target, observer, frame, abcorr);

    // convert params to spice types
    ConstSpiceChar *target_spice = target.c_str();  // better way to do this?
    ConstSpiceChar *observer_spice = observer.c_str();
    ConstSpiceChar *frame_spice = frame.c_str();
    ConstSpiceChar *abcorr_spice = abcorr.c_str();

    // define outputs
    SpiceDouble lt;
    SpiceDouble starg_spice[6];
    checkNaifErrors();
    spkezr_c( target_spice, et, frame_spice, abcorr_spice, observer_spice, starg_spice, &lt );
    checkNaifErrors();

    // convert to std::array for output
    vector<double> lt_starg = {0, 0, 0, 0, 0, 0, lt};
    for(int i = 0; i < 6; i++) {
      lt_starg[i] = starg_spice[i];
    }

    return lt_starg;
  }

  json merge_json(json &j1, json &j2, bool overwrite) { 
    if(overwrite) { 
      SPDLOG_TRACE("Overwriting Kernels");
      j1.merge_patch(j2);
    }
    else { 
      if(j1.is_null()) { 
        j1 = j2;
        return j1; 
      }

      SPDLOG_TRACE("Merging Kernels");
      for(auto &[k, v] : j2.items()) { 
        if(j1.contains(k) && j1[k].is_array()) { 
          // create a set for quick lookups for duplicates
          std::unordered_set<string> k_set(j1[k].begin(), j1[k].end());
          SPDLOG_DEBUG("Merging {} and {}", j1[k].dump(), v.dump());
          for(auto & e: v) { 
            if(!k_set.contains(e)) j1[k].push_back(e);
          }
        }
        else { 
          j1[k] = v;
        }
      }
    }
    return j1;
  }

  vector<double> getTargetOrientation(double et, int toFrame, int refFrame) {
    // Much of this function is from ISIS SpiceRotation.cpp
    SpiceDouble stateCJ[6][6];
    SpiceDouble CJ_spice[3][3];
    SpiceDouble av_spice[3];
    SpiceDouble quat_spice[4];

    vector<double> orientation = {0, 0, 0, 0};

    bool has_av = true;
    
    // First try getting the entire state matrix (6x6), which includes CJ and the angular velocity
    checkNaifErrors();
    frmchg_((int *) &refFrame, (int *) &toFrame, &et, (doublereal *) stateCJ);
    SpiceBoolean ckfailure = failed_c();
    reset_c();                   // Reset Naif error system to allow caller to recover

    if (!ckfailure) {
      // Transpose and isolate CJ and av
      xpose6_c(stateCJ, stateCJ);
      xf2rav_c(stateCJ, CJ_spice, av_spice);

      // Convert to std::array for output
      for(int i = 0; i < 3; i++) {
        orientation.push_back(av_spice[i]);
      }
    }
    else {  // TODO This case is untested
      // Recompute CJ_spice ignoring av
      reset_c(); // reset frmchg_ failure

      refchg_((int *) &refFrame, (int *) &toFrame, &et, (doublereal *) CJ_spice);
      xpose_c(CJ_spice, CJ_spice);

      has_av = false;
    }

    checkNaifErrors();
    // Translate matrix to std:array quaternion
    m2q_c(CJ_spice, quat_spice);

    for(int i = 0; i < 4; i++) {
      orientation[i] = quat_spice[i];
    }

    return orientation;
  }


  // Given a string keyname template, search the kernel pool for matching keywords and their values
  // returns json with up to ROOM=50 matching keynames:values
  // if no keys are found, returns null
  json findKeywords(string keytpl) {
    // Define gnpool i/o
    const SpiceInt START = 0;
    const SpiceInt ROOM = 200;
    const SpiceInt LENOUT = 200;
    ConstSpiceChar *cstr = keytpl.c_str();
    SpiceInt nkeys;
    SpiceChar kvals [ROOM][LENOUT];
    SpiceBoolean gnfound;

    // Call gnpool to search for input key template
    checkNaifErrors();
    gnpool_c(cstr, START, ROOM, LENOUT, &nkeys, kvals, &gnfound);
    checkNaifErrors();

    if(!gnfound) {
      return nullptr;
    }

    // Call gXpool for each key found in gnpool
    // accumulate results to json allResults

    // Define gXpool params
    ConstSpiceChar *fkey;
    SpiceInt nvals;
    SpiceChar cvals [ROOM][LENOUT];
    SpiceDouble dvals[ROOM];
    SpiceInt ivals[ROOM];
    SpiceBoolean gcfound = false, gdfound = false, gifound = false;

    json allResults;
    
    // iterate over kvals;
    for(int i = 0; i < nkeys; i++) {
      json jresultVal;

      fkey = &kvals[i][0];

      checkNaifErrors();
      gdpool_c(fkey, START, ROOM, &nvals, dvals, &gdfound);
      checkNaifErrors();

      if (gdfound) {
        // format output
        if (nvals == 1) {
          jresultVal = dvals[0];
        }
        else {
          for(int j=0; j<nvals; j++) {
            jresultVal.push_back(dvals[j]);
          }
        }
      }

      if (!gdfound) {
        gipool_c(fkey, START, ROOM, &nvals, ivals, &gifound);
        checkNaifErrors();
      }

      if (gifound) {
        // format output
        if (nvals == 1) {
          jresultVal = ivals[0];
        }
        else {
          for(int j=0; j<nvals; j++) {
            jresultVal.push_back(ivals[j]);
          }
        }
      }

      if (!gifound || !gdfound) {
        gcpool_c(fkey, START, ROOM, LENOUT, &nvals, cvals, &gcfound);
        checkNaifErrors();
      }

      if (gcfound) {
        // format gcpool output
        string str_cval;
        if (nvals == 1) {
          str_cval.assign(&cvals[0][0]);
          string lower = toLower(str_cval);

          // if null or boolean, do a conversion
          if (lower == "true") {
            jresultVal = true;
          }
          else if (lower == "false") {
            jresultVal = false;
          }
          else if (lower == "null") {
            jresultVal = nullptr;
          }
          else {
            jresultVal = str_cval;
          }
        }
        else {
          for(int j=0; j<nvals; j++) {
            str_cval.assign(&cvals[j][0]);
            string lower = toLower(str_cval);

            // if null or boolean, do a conversion
            if (lower == "true") {
              jresultVal.push_back(true);
            }
            else if (lower == "false") {
              jresultVal.push_back(false);
            }
            else if (lower == "null") {
              jresultVal.push_back(nullptr);
            }
            else {
              jresultVal.push_back(str_cval);
            }
          }
        }
      }

      // append to allResults:
      //     key:list-of-values
      string resultKey(fkey);
      allResults[resultKey] = jresultVal;
    }

    return allResults;
  }

  vector<json::json_pointer> findKeyInJson(json in, string key, bool recursive) {
    function<vector<json::json_pointer>(json::json_pointer, string, vector<json::json_pointer>, bool)> recur = [&recur, &in](json::json_pointer elem, string key, vector<json::json_pointer> vec, bool recursive) -> vector<json::json_pointer> {
      json e = in[elem];
      for (auto &it : e.items()) {
        json::json_pointer pointer = elem/it.key();
        if (recursive && it.value().is_structured()) {
          vec = recur(pointer, key, vec, recursive);
        }
        if(it.key() == key) {
          vec.push_back(pointer);
        }
      }
      return vec;
    };

    vector<json::json_pointer> res;
    json::json_pointer p = ""_json_pointer;
    res = recur(p, key, res, recursive);
    return res;
  }


  vector<vector<string>> json2DArrayTo2DVector(json arr) {
    vector<vector<string>> res;

    if (arr.is_array()) {
      for(auto &subarr : arr) {
        if (subarr.empty() || subarr.is_null()) {
          continue; 
        }
        
        vector<string> subres; 

        if (!subarr.is_array()) { 
          throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
        }
        for(auto &k : subarr) {
          // should be a single element
          if (k.empty()) { 
            continue;
          }
          if (k.is_array()) { // needs to be scalar
            throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
          }
          subres.emplace_back(k);
        }
        res.push_back(subres);
      }
    }
    else if (arr.is_string()) {
      vector<string> subres; 
      subres.emplace_back(arr);
      res.emplace_back(subres);
    }
    else {
      throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
    }

    return res;
  }

  vector<vector<double>> json2DFloatArrayTo2DVector(json arr) {
    vector<vector<double>> res;

    if (arr.is_array()) {
      for(auto &subarr : arr) {
        if (subarr.empty() || subarr.is_null()) {
          continue; 
        }
        
        vector<double> subres; 

        if (!subarr.is_array()) { 
          throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
        }
        for(auto &k : subarr) {
          // should be a single element
          if (k.empty()) { 
            continue;
          }
          if (k.is_array()) { // needs to be scalar
            throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
          }
          subres.emplace_back(k);
        }
        res.push_back(subres);
      }
    }
    else if (arr.is_number_float()) {
      vector<double> subres; 
      subres.emplace_back(arr);
      res.emplace_back(subres);
    }
    else {
      throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
    }

    return res;
  }

  vector<vector<int>> json2DIntArrayTo2DVector(json arr) {
    vector<vector<int>> res;

    if (arr.is_array()) {
      for(auto &subarr : arr) {
        if (subarr.empty() || subarr.is_null()) {
          continue; 
        }
        
        vector<int> subres; 

        if (!subarr.is_array()) { 
          throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
        }
        for(auto &k : subarr) {
          // should be a single element
          if (k.empty()) { 
            continue;
          }
          if (k.is_array()) { // needs to be scalar
            throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
          }
          subres.emplace_back(k);
        }
        res.push_back(subres);
      }
    }
    else if (arr.is_number_integer()) {
      vector<int> subres; 
      subres.emplace_back(arr);
      res.emplace_back(subres);
    }
    else {
      throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
    }

    return res;
  }

  vector<pair<double, double>> json2DArrayToDoublePair(json arr) {
    vector<pair<double, double>> res;

    if (arr.is_array()) {
      for(auto &subarr : arr) {
        if (subarr.is_null() || subarr.empty()) {
          continue; 
        }
        
        pair<double, double> subres; 

        if (!subarr.is_array()) {
          throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
        }
        if (subarr.size() != 2) {
          throw invalid_argument("Input json is not a valid Nx2 Json array: " + arr.dump());
        }
        if (!(subarr[0].is_number() && subarr[0].is_number())) { 
          throw invalid_argument("Input json is not a valid Nx2 Json array of doubles: " + arr.dump());
        }

        subres.first = subarr[0].get<double>();
        subres.second = subarr[1].get<double>();
        res.push_back(subres);
      }
    }
    else if (arr.is_null()) { 
      vector<pair<double, double>> empty; 
      return empty;   
    }
    else {
      throw invalid_argument("Input json is not a valid 2D Json array: " + arr.dump());
    }

    return res;
  }


  vector<string> jsonArrayToVector(json arr) {
    vector<string> res;

    if (arr.is_array()) {
      for(auto it : arr) {
        res.emplace_back(it);
      }
    }
    else if (arr.is_string()) {
      res.emplace_back(arr);
    }
    else {
      spdlog::dump_backtrace();
      throw invalid_argument("Input json is not a valid Json array: " + arr.dump());
    }

    return res;
  }

  vector<double> jsonDoubleArrayToVector(json arr) {
    vector<double> res;

    if (arr.is_array()) {
      for(auto it : arr) {
        res.emplace_back(it);
      }
    }
    else if (arr.is_number_float()) {
      res.emplace_back(arr);
    }
    else {
      spdlog::dump_backtrace();
      throw invalid_argument("Input json is not a valid Json array: " + arr.dump());
    }

    return res;
  }


  vector<int> jsonIntArrayToVector(json arr) {
    vector<int> res;

    if (arr.is_array()) {
      for(auto it : arr) {
        res.emplace_back(it);
      }
    }
    else if (arr.is_number_integer()) {
      res.emplace_back(arr);
    }
    else {
      spdlog::dump_backtrace();
      throw invalid_argument("Input json is not a valid Json array: " + arr.dump());
    }

    return res;
  }


  vector<string> ls(string const & root, bool recursive) {
    vector<string> paths;
    
    SPDLOG_TRACE("ls({}, {})", root, recursive);

    if (fs::exists(root) && fs::is_directory(root)) {
      for (auto i = fs::recursive_directory_iterator(root); i != fs::recursive_directory_iterator(); ++i ) {
        if (fs::exists(*i)) {
          paths.emplace_back(i->path());
        }

        if(!recursive) {
          // simply disable recursion if recurse flag is off
          i.disable_recursion_pending();
        }
      }
    }

    return paths;
  }


  vector<string> glob(string const & root, string const & reg, bool recursive) {
    vector<string> paths;
    vector<string> files_to_search = Memo::ls(root, recursive);
    for (auto &f : files_to_search) {
      if (regex_search(f.c_str(), basic_regex(reg, regex_constants::optimize|regex_constants::ECMAScript)) && string(fs::path(f).filename()).at(0) != '.') {
        paths.emplace_back(f);
      }
    }

    return paths;
  }


  vector<pair<double, double>> getTimeIntervals(string kpath) {
    auto formatIntervals = [&](SpiceCell &coverage) -> vector<pair<double, double>> {
      //Get the number of intervals in the object.
      checkNaifErrors();

      int niv = card_c(&coverage) / 2;
      //Convert the coverage interval start and stop times to TDB
      SpiceDouble begin, end;

      vector<pair<double, double>> res;

      for(int j = 0;  j < niv;  j++) {
        //Get the endpoints of the jth interval.
        wnfetd_c(&coverage, j, &begin, &end);
        checkNaifErrors();
        pair<double, double> p = {begin, end};
        res.emplace_back(p);
      }

      return res;
    };


    SpiceChar fileType[32], source[2048];
    SpiceInt handle;
    SpiceBoolean found;

    Kernel k(kpath);

    checkNaifErrors();
    kinfo_c(kpath.c_str(), 32, 2048, fileType, source, &handle, &found);
    checkNaifErrors();

    string currFile = fileType;

    //create a spice cell capable of containing all the objects in the kernel.
    SPICEINT_CELL(currCell, 200000);

    //this resizing is done because otherwise a spice cell will append new data
    //to the last "currCell"
    ssize_c(0, &currCell);
    ssize_c(200000, &currCell);

    SPICEDOUBLE_CELL(cover, 200000);

    if (currFile == "SPK") {
      spkobj_c(kpath.c_str(), &currCell);
    }
    else if (currFile == "CK") {
      ckobj_c(kpath.c_str(), &currCell);
    }
    else if (currFile == "TEXT") {
      throw invalid_argument("Input Kernel is a text kernel which has no intervals");
    }
    checkNaifErrors();

    vector<pair<double, double>> result;

    for(int bodyCount = 0 ; bodyCount < card_c(&currCell) ; bodyCount++) {
      //get the NAIF body code
      int body = SPICE_CELL_ELEM_I(&currCell, bodyCount);

      //only provide coverage for negative NAIF codes
      //(Positive codes indicate planetary bodies, negatives indicate
      // spacecraft and instruments)
      checkNaifErrors();
      if (body < 0) {
        vector<pair<double, double>> times;
        //find the correct coverage window
        if(currFile == "SPK") {
          SPICEDOUBLE_CELL(cover, 200000);
          ssize_c(0, &cover);
          ssize_c(200000, &cover);
          spkcov_c(kpath.c_str(), body, &cover);
          times = formatIntervals(cover);
        }
        else if(currFile == "CK") {
          //  200,000 is the max coverage window size for a CK kernel
          SPICEDOUBLE_CELL(cover, 200000);
          ssize_c(0, &cover);
          ssize_c(200000, &cover);

          // A SPICE SEGMENT is composed of SPICE INTERVALS
          ckcov_c(kpath.c_str(), body, SPICEFALSE, "SEGMENT", 0.0, "TDB", &cover);

          times = formatIntervals(cover);
        }
        checkNaifErrors();

        result.reserve(result.size() + distance(times.begin(), times.end()));
        result.insert(result.end(), times.begin(), times.end());

      }
    }
    return result;
  }


  pair<double, double> getKernelStartStopTimes(string kpath) {
    SPDLOG_TRACE("getKernelStartStopTimes({})", kpath);

    double start_time = 0;
    double stop_time = 0;
    
    auto getStartStopFromInterval = [&](SpiceCell &coverage) {

      //Get the number of intervals in the object.
      checkNaifErrors();
      int niv = card_c(&coverage) / 2;
      //Convert the coverage interval start and stop times to TDB
      SpiceDouble begin, end;

      for(int j = 0;  j < niv;  j++) {
        //Get the endpoints of the jth interval.
        SPDLOG_TRACE("Collecting start stop times");
        wnfetd_c(&coverage, j, &begin, &end);
        checkNaifErrors();

        if (start_time == 0 && stop_time == 0) { 
          start_time = begin; 
          stop_time = end;
        }        

        start_time = min(start_time, begin);
        stop_time = max(stop_time, end);
      }
      checkNaifErrors();
    };

    SpiceChar fileType[32], source[2048];
    SpiceInt handle;
    SpiceBoolean found;

    Kernel k(kpath);

    checkNaifErrors();
    kinfo_c(kpath.c_str(), 32, 2048, fileType, source, &handle, &found);
    checkNaifErrors();

    string currFile = fileType;

    //create a spice cell capable of containing all the objects in the kernel.
    SPICEINT_CELL(currCell, 300000);

    //this resizing is done because otherwise a spice cell will append new data
    //to the last "currCell"
    ssize_c(0, &currCell);
    ssize_c(300000, &currCell);

    SPICEDOUBLE_CELL(cover, 300000);

    if (currFile == "SPK") {
      spkobj_c(kpath.c_str(), &currCell);
    }
    else if (currFile == "CK") {
      ckobj_c(kpath.c_str(), &currCell);
    }
    else if (currFile == "TEXT") {
      throw invalid_argument("Input Kernel is a text kernel which has no intervals");
    }
    checkNaifErrors();

    vector<pair<double, double>> result;

    for(int bodyCount = 0 ; bodyCount < card_c(&currCell) ; bodyCount++) {
      //get the NAIF body code
      int body = SPICE_CELL_ELEM_I(&currCell, bodyCount);

      //only provide coverage for negative NAIF codes
      //(Positive codes indicate planetary bodies, negatives indicate
      // spacecraft and instruments)
      checkNaifErrors();
      if (body < 0) {
        //find the correct coverage window
        if(currFile == "SPK") {
          SPICEDOUBLE_CELL(cover, 300000);
          ssize_c(0, &cover);
          ssize_c(300000, &cover);
          SPDLOG_TRACE("Getting spk coverage");
          spkcov_c(kpath.c_str(), body, &cover);
          getStartStopFromInterval(cover);
        }
        else if(currFile == "CK") {
          //  200,000 is the max coverage window size for a CK kernel
          SPICEDOUBLE_CELL(cover, 300000);
          ssize_c(0, &cover);
          ssize_c(300000, &cover);
          SPDLOG_TRACE("Getting ck coverage");

          // A SPICE SEGMENT is composed of SPICE INTERVALS
          ckcov_c(kpath.c_str(), body, SPICEFALSE, "SEGMENT", 0.0, "TDB", &cover);
          SPDLOG_TRACE("collecting start stop times");

          getStartStopFromInterval(cover);
        }
        checkNaifErrors();
      }
    }
    return pair<double, double>(start_time, stop_time);
  }


  string globTimeIntervals(string mission) { 
    SPDLOG_TRACE("In globTimeIntervals.");
    Config conf;
    conf = conf[mission];
    json new_json = {};
    json sclk_json = getLatestKernels(conf.get("sclk"));
    KernelSet sclks(sclk_json);

    // Get CK Times
    json ckJson = conf.getRecursive("ck");

    vector<json::json_pointer> ckKernelGrps = findKeyInJson(ckJson, "kernels");
    for(auto &ckKernelGrp : ckKernelGrps) { 
      vector<vector<string>> kernelList = json2DArrayTo2DVector(ckJson[ckKernelGrp]);
      for(auto &subList : kernelList) { 
        for (auto & kernel : subList) {
          vector<pair<double, double>> timeIntervals = getTimeIntervals(kernel);
          new_json[kernel] = timeIntervals;
        }
      }
    }
    
    // get SPK times
    json spkJson = conf.getRecursive("spk");
    vector<json::json_pointer> spkKernelGrps = findKeyInJson(spkJson, "kernels");
    for(auto &spkKernelGrp : spkKernelGrps) { 
      vector<vector<string>> kernelList = json2DArrayTo2DVector(spkJson[spkKernelGrp]);
      for(auto &subList : kernelList) { 
        for (auto & kernel : subList) {
          vector<pair<double, double>> timeIntervals = getTimeIntervals(kernel);
          new_json[kernel] = timeIntervals;
        }
      }
    }
    return new_json.dump();
  }


  string globKernelStartStopTimes(string mission) { 
    SPDLOG_TRACE("In globTimeIntervals.");
    Config conf;
    conf = conf[mission];
    json new_json = {};
    json sclk_json = getLatestKernels(conf.get("sclk"));
    KernelSet sclks(sclk_json);

    // Get CK Times
    json ckJson = conf.getRecursive("ck");

    vector<json::json_pointer> ckKernelGrps = findKeyInJson(ckJson, "kernels");
    for(auto &ckKernelGrp : ckKernelGrps) { 
      vector<vector<string>> kernelList = json2DArrayTo2DVector(ckJson[ckKernelGrp]);
      for(auto &subList : kernelList) { 
        for (auto & kernel : subList) {
          pair<double, double> sstimes = getKernelStartStopTimes(kernel);
          new_json[kernel] = sstimes;
        }
      }
    }
    
    // get SPK times
    json spkJson = conf.getRecursive("spk");
    vector<json::json_pointer> spkKernelGrps = findKeyInJson(spkJson, "kernels");
    for(auto &spkKernelGrp : spkKernelGrps) { 
      vector<vector<string>> kernelList = json2DArrayTo2DVector(spkJson[spkKernelGrp]);
      for(auto &subList : kernelList) { 
        for (auto & kernel : subList) {
          pair<double, double> sstimes = getKernelStartStopTimes(kernel);
          new_json[kernel] = sstimes;
        }
      }
    }
    return new_json.dump();
  }


  string getDataDirectory() {
      char* isisdata_ptr = getenv("ISISDATA");
      fs::path isisDataDir = isisdata_ptr == NULL ? "" : isisdata_ptr;

      char* alespice_ptr = getenv("ALESPICEROOT");
      fs::path aleDataDir = alespice_ptr == NULL ? "" : alespice_ptr;
      
      char* spiceroot_ptr = getenv("SPICEROOT");
      fs::path spiceDataDir = spiceroot_ptr == NULL ? "" : spiceroot_ptr;
 
      if (fs::is_directory(spiceDataDir)) {
         return spiceDataDir;
      }

      if (fs::is_directory(aleDataDir)) {
        return aleDataDir;
      }

      if (fs::is_directory(isisDataDir)) {
        return isisDataDir;
      }
      throw runtime_error(fmt::format("Please set env var SPICEROOT, ISISDATA or ALESPICEROOT in order to proceed."));
  }


  string getConfigDirectory() {
    // If running tests or debugging locally
    char* condaPrefix = std::getenv("CONDA_PREFIX");
    fs::path installDbPath = "";

    if(condaPrefix) { 
      SPDLOG_TRACE("CONDA_PREFIX: {}", string(condaPrefix));
      installDbPath = fs::absolute(condaPrefix) / "etc" / "SpiceQL" / "db";
    } else { 
      SPDLOG_TRACE("CONDA_PREFIX not set");
    }

    fs::path debugDbPath = fs::absolute(_SOURCE_PREFIX) / "SpiceQL" / "db";

    // Use installDbPath unless $SPICEQL_DEV_DB is set
    fs::path dbPath;
    if (std::getenv("SPICEQL_DEV_DB") && toLower(string(std::getenv("SPICEQL_DEV_DB"))) == "true") {
      dbPath = debugDbPath; 
    } else if (installDbPath.empty()) {
      throw runtime_error("Config Directory Not Found.");
    } else {
      dbPath = installDbPath;
    }
    SPDLOG_TRACE("SpiceQL DB Path: {}", dbPath.string()); 

    if (!fs::is_directory(dbPath)) {
      throw runtime_error("Config Directory Not Found.");
    }

    return dbPath; 
  }  


  vector<string> getAvailableConfigFiles() {
    vector<string> confs; 
    fs::path dbDir = getConfigDirectory();
    return glob(dbDir, ".json", false);
  }

  vector<json> getAvailableConfigs() {
    vector<string> confPaths = getAvailableConfigFiles();
    vector<json> confs;

    for(auto & c: confPaths) {
      ifstream ifs(c);
      json jf = json::parse(ifs);
      confs.emplace_back(jf);
    }
    return confs; 
  }

  string getMissionConfigFile(string mission) {
  
    vector<string> paths = getAvailableConfigFiles();

    for(const fs::path &p : paths) {
      if (p.filename() == fmt::format("{}.json", mission)) {
        return p;
      }
    }

    throw invalid_argument(fmt::format("Config file for \"{}\" not found", mission));
  }


  json getMissionConfig(string mission) {
    fs::path dbPath = getMissionConfigFile(mission);

    ifstream i(dbPath);
    json conf = json::parse(i);
    return conf;
  }


  string getMissionKeys(json config) {
    string missionKeys = "";
    int i = 0;
    int configNumKeys = config.size();
    for (auto& [key, val] : config.items()) {
      missionKeys += key;
      if (i != configNumKeys - 1) {
        missionKeys += ", ";
      }
      i++;
    }
    return missionKeys;
  }


  string getRestUrl() {
    char* spiceqlRestUrlEnvVar = std::getenv("SPICEQL_REST_URL");
    string spiceqlRestUrl;
    if (spiceqlRestUrlEnvVar == NULL) {
      spiceqlRestUrl = "https://astrogeology.usgs.gov/apis/spiceq/latest/";
    } else {
      spiceqlRestUrl = string(spiceqlRestUrlEnvVar);
    }
    SPDLOG_TRACE("SpiceQL REST URL: {}", spiceqlRestUrl); 
    return spiceqlRestUrl;
  }  


  void resolveConfigDependencies(json &config, const json &dependencies) {
    SPDLOG_TRACE("IN resolveConfigDependencies");
    vector<json::json_pointer> depLists = findKeyInJson(config, "deps");
    
    // 10 seems like a reasonable number of recursive dependencies to allow
    int maxRecurssion = 10;
    int numRecurssions = 0;
    while (!depLists.empty()) {
      for (auto & depList: depLists) {
        vector<string> depsToMerge = jsonArrayToVector(config[depList]);
        eraseAtPointer(config, depList);
        json::json_pointer mergeInto = depList.parent_pointer();
        for(auto & depString: depsToMerge) {
          json::json_pointer mergeFrom(depString);
          mergeConfigs(config[mergeInto], dependencies[mergeFrom]);
        }
      }
      depLists = findKeyInJson(config, "deps");
      if (++numRecurssions > maxRecurssion) {
        throw invalid_argument(fmt::format("Could not resolve config dependencies, "
                                           "max recursion depth of {} reached", maxRecurssion));
      }
    }
  }


  size_t eraseAtPointer(json &j, json::json_pointer ptr) {
    vector<string> path;
    while(!ptr.empty()) {
      path.insert(path.begin(), ptr.back());
      ptr.pop_back();
    }
    json::json_pointer parentObj;
    for (size_t i = 0; i < path.size() - 1; i++) {
      parentObj.push_back(path[i]);
    }
    if (j.contains(parentObj)) {
      return j[parentObj].erase(path.back());
    }
    else {
      return 0;
    }
  }


  string getKernelType(string kernelPath) {
    SpiceChar type[6];
    SpiceChar source[6];
    SpiceInt handle;
    SpiceBoolean found;

    Kernel k(kernelPath);
    checkNaifErrors();
    kinfo_c(kernelPath.c_str(), 6, 6, type, source, &handle, &found);
    checkNaifErrors();

    if (!found) {
      throw domain_error("Kernel Type not found");
    }

    return string(type);
  }


  string getRootDependency(json config, string pointer) {
    json::json_pointer depPointer(pointer);
    depPointer /= "deps";
    json deps = config[depPointer];
    
    for (auto path: deps) {
      fs::path fsDataPath(getDataDirectory() + (string)path);
      if (fs::exists(fsDataPath)) {
        return path;
      }
      else {
        string recursePath = getRootDependency(config, (string)path);
        if (recursePath != "") {
          return recursePath;
        }
      }
    }
    return "";
  }


  bool checkNaifErrors(bool reset) {
    static bool initialized = false; 

    if(!initialized) {
      SpiceChar returnAct[32] = "RETURN";
      SpiceChar printAct[32] = "NONE";
      erract_c("SET", sizeof(returnAct), returnAct);   // Reset action to return
      errprt_c("SET", sizeof(printAct), printAct);     // ... and print nothing
      initialized = true;
    }
    
    if(!failed_c()) return true;

    // This method has been documented with the information provided
    //   from the NAIF documentation at:
    //    naif/cspice61/packages/cspice/doc/html/req/error.html

    // This message is a character string containing a very terse, usually
    // abbreviated, description of the problem. The message is a character
    // string of length not more than 25 characters. It always has the form:
    // SPICE(...)
    // Short error messages used in CSPICE are CONSTANT, since they are
    // intended to be used in code. That is, they don't contain any data which
    // varies with the specific instance of the error they indicate.
    // Because of the brief format of the short error messages, it is practical
    // to use them in a test to determine which type of error has occurred.
    const int SHORT_DESC_LEN = 26;
    char naifShort[SHORT_DESC_LEN];
    getmsg_c("SHORT", SHORT_DESC_LEN, naifShort);

    // This message may be up to 1840 characters long. The CSPICE error handling
    // mechanism makes no use of its contents. Its purpose is to provide human-readable
    // information about errors. Long error messages generated by CSPICE routines often
    // contain data relevant to the specific error they describe.
    const int LONG_DESC_LEN = 1841;
    char naifLong[LONG_DESC_LEN];
    getmsg_c("LONG", LONG_DESC_LEN, naifLong);

    // Search for known naif errors...
    string errMsg = ""; 

    // Now process the error
    if(reset) {
      reset_c();
    }
    
    errMsg += "Error Occured:" + string(naifShort) + " " + string(naifLong);
    throw runtime_error(errMsg);
  }

  json loadTranslationKernels(string mission, bool loadFk, bool loadIk, bool loadIak) {
    Config c;
    json j;
    vector<string> kernelsToGet = {};

    if (!(loadFk || loadIk || loadIak)) {
      throw invalid_argument("Not loading any kernels. Please select a set of translation kernels to load.");
    }
    else {
      if (loadFk)
        kernelsToGet.push_back("fk");
      if (loadIk)
        kernelsToGet.push_back("ik");
      if (loadIak)
        kernelsToGet.push_back("iak");
    }
    
    if (c.contains(mission)) {
      j = c[mission].get(kernelsToGet);
      json missionKernels = {};
      if (loadFk)
        missionKernels["fk"] = j["fk"];
      if (loadIk)
        missionKernels["ik"] = j["ik"];
      if (loadIak)
        missionKernels["iak"] = j["iak"];
      j = getLatestKernels(missionKernels);
    }
    else {
      string missionKeys = getMissionKeys(c.globalConf());
      SPDLOG_WARN("Could not find mission: \"{}\" in config. \n Double-check mission variable, manually furnish kernels, or try including frame and mission name. List of available missions: [{}].", mission, missionKeys);
    }
    return j;
  }

  json loadSelectKernels(string kernelType, string mission) {
    Config missionConf;
    json kernels;
    
    // Check the kernel type
    // This will throw an invalid_argument error
    Kernel::translateType(kernelType);

    if (missionConf.contains(mission)) {
      SPDLOG_TRACE("Found {} in config, getting only {} {}.", mission, mission, kernelType);
      missionConf = missionConf[mission];
      kernels = missionConf.getLatest(kernelType);
    }
    else {
      SPDLOG_TRACE("Coudn't find {} in config explicitly, loading all {} kernels", mission, kernelType);
      kernels = missionConf.getLatestRecursive(kernelType);
    }

    return kernels;
  }
}
