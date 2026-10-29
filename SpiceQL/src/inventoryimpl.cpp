#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <map>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

// we need to include this to overwrite and other std::fs imports
#include <ghc/fs_std.hpp>
#include <nlohmann/json.hpp>
// The BTree submodule's disk_fixed_alloc.h only defines the stdpmr namespace
// alias for clang and GCC. Provide it for MSVC so the BTree headers compile on
// Windows.
#if defined(_MSC_VER) && !defined(SPICEQL_STDPMR_DEFINED)
#define SPICEQL_STDPMR_DEFINED
#include <memory_resource>
namespace stdpmr = std::pmr;
#endif
#include <fc/btree.h>
#include <fc/disk_btree.h>
#include <SpiceQL/spiceql_logging.h>
#include <queue>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/utility.hpp>

#include <highfive/H5Easy.hpp>
#include <highfive/highfive.hpp>

#include <atomic>
#include <cstdlib>
#include <thread>

#include <SpiceUsr.h>

#include <SpiceQL/config.h>
#include <SpiceQL/daf_reader.h>
#include <SpiceQL/inventoryimpl.h>
#include <SpiceQL/utils.h>
#include <SpiceQL/query.h>
#include <SpiceQL/memo.h>
#include <SpiceQL/spiceql_version.h>

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
  string DB_INTERVAL_START_KEY = "intervals_start";
  string DB_INTERVAL_STOP_KEY = "intervals_stop";
  string DB_INTERVAL_START_INDICES_KEY = "intervals_start_kindex";
  string DB_INTERVAL_STOP_INDICES_KEY = "intervals_stop_kindex";
  string DB_INTERVAL_BODY_CODES_KEY = "interval_body_code";
  string DB_FRAME_CACHE_KEY = "spql_cache";
  string DB_FRAME_LIST_KEY = "spql_cache/frame_list";
  string DB_FRAME_CODES_KEY = "spql_cache/frame_codes";
  string DB_FRAME_NAMES_KEY = "spql_cache/frame_names";
  string CACHE_DIR_ENV_VAR = "SPICEQL_CACHE_DIR";
  static std::string  CACHE_DIRECTORY = "";


  void setCacheDir(string cache_dir, bool override) {
    const char* cache_dir_char = getenv(CACHE_DIR_ENV_VAR.c_str());

    // Priority order: override > env var > provided cache_dir > auto-generate
    if (override && cache_dir != "") {
      SPDLOG_DEBUG("Setting cache directory (override): {}", cache_dir);
      CACHE_DIRECTORY = cache_dir;
    }
    else if (cache_dir_char != NULL) {
      SPDLOG_DEBUG("Cache directory set in environment variable " + CACHE_DIR_ENV_VAR + ": " + cache_dir_char);
      CACHE_DIRECTORY = cache_dir_char;
    }
    else if (cache_dir != "") {
      SPDLOG_DEBUG("Setting cache directory to: {}", cache_dir);
      CACHE_DIRECTORY = cache_dir;
    }
    else {
      SPDLOG_DEBUG("Cache directory not set and not in environment variable " + CACHE_DIR_ENV_VAR + " and not overridden.");
      std::string tempname = "spiceql-cache-" + gen_random(10);
      CACHE_DIRECTORY = (fs::temp_directory_path() / tempname / "spiceql_cache").string();
    }

    if (!fs::is_directory(CACHE_DIRECTORY)) {
      SPDLOG_DEBUG("{} does not exist, attempting to create the directory", CACHE_DIRECTORY);
      fs::create_directories(CACHE_DIRECTORY);
    }

    SPDLOG_DEBUG("Setting cache directory to: {}", CACHE_DIRECTORY);
  }


  string getCacheDir() {
      if (CACHE_DIRECTORY == "") {
          // Auto-initialize cache directory using setCacheDir logic
          setCacheDir("");
      } 
      else { 
          SPDLOG_TRACE("Cache Directory Already Set: {}", CACHE_DIRECTORY);  
      }

      return CACHE_DIRECTORY;
  }


  string getHdfFile() { 
      static std::string db_path = (fs::path(getCacheDir()) / DB_HDF_FILE).string();
      return db_path;
  }
  

  // objs need to be passed in c-style because of a lack of copy contructor in BtreeMap
  namespace {
    using DbClock = std::chrono::steady_clock;

    inline double secondsSince(DbClock::time_point t0) {
      return std::chrono::duration<double>(DbClock::now() - t0).count();
    }

    const json &rawConfig() {
      static const json raw = [] {
        json out;
        try {
          for (const json &j : getAvailableConfigs()) {
            out.update(j);
          }
        } catch (std::exception &e) {
          // Reporting is best-effort, a bad config shouldn't fail the search
          SPDLOG_WARN("Could not read configs for quality reporting: {}", e.what());
        }
        return out;
      }();
      return raw;
    }


    vector<string> qualityOwners(const string &spiceql_name, const string &type, const string &quality) {
      const json &raw = rawConfig();
      vector<string> owners;
      unordered_set<string> seen;
      vector<string> pending = {spiceql_name};

      while (!pending.empty()) {
        string current = pending.back();
        pending.pop_back();

        // Guards against a cycle in deps as well as the diamond that shows up
        // when two entries inherit from the same parent.
        if (!seen.insert(current).second || !raw.contains(current)) continue;

        const json &entry = raw[current];
        if (!entry.is_object()) continue;

        if (entry.contains(type) && entry[type].is_object() && entry[type].contains(quality)) {
          owners.push_back(current);
        }

        if (entry.contains("deps")) {
          for (auto &dep : entry["deps"]) {
            if (!dep.is_string()) continue;
            string name = dep.get<string>();
            // Written as a json pointer ("/moc"). Every dep in the shipped
            // configs is a single top-level entry, so the leading slash is all
            // there is to strip.
            if (!name.empty() && name.front() == '/') name.erase(0, 1);
            if (!name.empty()) pending.push_back(name);
          }
        }
      }
      return owners;
    }
  }


  // Coverage of one kernel, or the reason it couldn't be read.
  struct CoverageResult {
    bool ok = false;
    string error;
    KernelCoverage coverage;
  };


  // Number of threads for reading coverage, default is 16. 
  // Can set with environment variable $SPICEQL_DB_WORKERS.
  static size_t coverageWorkerCount() {
    const char *env = getenv("SPICEQL_DB_WORKERS");
    if (env && *env) {
      long n = strtol(env, nullptr, 10);
      return n > 0 ? (size_t)n : 1;
    }
    return 16;
  }


  // Expected properties from reading a kernel using DAF reader.
  // ok/error: checks whether the read worked, error message if not
  // isCK: whether kernel is a CK or an SPK
  // bounds: A map of each ID to its earliest start and latest stop
  // intervals: A map of each ID to the merged coverage window
  struct DafRead {
    bool ok = false;
    string error;
    bool isCk = false;
    map<int, pair<double, double>> bounds;
    map<int, daf::Window> intervals;
  };


  /**
   * Thread safe: only daf_reader.h, no CSPICE calls.
   *
   * @param type The kernel's type in the config, "ck" or "spk"
   **/
  static DafRead readDafCoverage(const string &kernel, const string &type) {
    DafRead read;
    try {
      string resolved = kernel;
      if (!fs::exists(resolved)) {
        resolved = (getDataDirectory() / fs::path(kernel)).string();
      }

      daf::File f(resolved);
      // Files from before the "DAF/CK" and "DAF/SPK" ID words only say "NAIF/DAF".
      const string kind = f.kind().empty() ? toUpper(type) : f.kind();
      if (kind != "CK" && kind != "SPK") {
        throw invalid_argument("Kernel [" + kernel + "] is a " + kind + ", which has no coverage to collect");
      }
      if (f.nd() != 2 || f.ni() != 6) {
        throw invalid_argument("Kernel [" + kernel + "] does not have " + kind + " summaries (ND/NI " +
                               to_string(f.nd()) + "/" + to_string(f.ni()) + ")");
      }
      read.isCk = kind == "CK";

      // Maximum interval limit per kernel, in parity with getKernelCoverage
      const size_t maxIntervals = 100000;

      // Verify negative IDs for spacecraft and instruments 
      for (const daf::Summary &s : f.summaries()) {
        if (s.ic[0] >= 0) continue;
        const int id = s.ic[0];
        auto [bound, added] = read.bounds.try_emplace(id, s.dc[0], s.dc[1]);
        if (!added) {
          bound->second.first = min(bound->second.first, s.dc[0]);
          bound->second.second = max(bound->second.second, s.dc[1]);
        }
        daf::Window &w = read.intervals[id];
        if (read.isCk) {
          daf::Window seg = daf::ckSegmentCoverage(f, s);
          w.insert(w.end(), seg.begin(), seg.end());
        }
        else {
          w.emplace_back(s.dc[0], s.dc[1]);
        }
        // Merge list of time ranges if over max intervals limit
        if (w.size() > maxIntervals) {
          w = daf::merge(std::move(w));
          if (w.size() > maxIntervals) {
            throw runtime_error("Kernel [" + kernel + "] has more than " + to_string(maxIntervals) +
                                " intervals for ID " + to_string(id) + ", more than a coverage window holds");
          }
        }
      }
      for (auto &[id, w] : read.intervals) {
        w = daf::merge(std::move(w));
      }
      read.ok = true;
    }
    catch (exception &e) {
      read.error = e.what();
    }
    return read;
  }


  /**
   * Turns a DafRead into the same KernelCoverage getKernelCoverage gives,
   * converting CK ticks to ET. Uses CSPICE, so it runs in the calling thread
   * with the mission's SCLKs furnished.
   **/
  static CoverageResult convertDafCoverage(const DafRead &read) {
    CoverageResult result;
    if (!read.ok) {
      result.error = read.error;
      return result;
    }

    try {
      KernelCoverage &coverage = result.coverage;

      // Get ID code of spacecraft clock
      map<int, SpiceInt> clocks;
      auto clockFor = [&](int id) {
        auto it = clocks.find(id);
        if (it == clocks.end()) {
          SpiceInt sclk = 0;
          ckmeta_c(id, "SCLK", &sclk);
          checkNaifErrors();
          it = clocks.emplace(id, sclk).first;
        }
        return it->second;
      };
      auto toEt = [](SpiceInt sclk, double ticks) {
        double et;
        sct2e_c(sclk, ticks, &et);
        return et;
      };

      bool first = true;
      for (const auto &[id, ticks] : read.bounds) {
        double boundStart = ticks.first, boundStop = ticks.second;

        if (read.isCk) {
          // The bounds keep getKernelStartStopTimes' id / 1000 clock
          boundStart = toEt(id / 1000, ticks.first);
          boundStop = toEt(id / 1000, ticks.second);
          checkNaifErrors();
        }

        if (first) {
          coverage.start_time = boundStart;
          coverage.stop_time = boundStop;
          first = false;
        }
        else {
          coverage.start_time = min(coverage.start_time, boundStart);
          coverage.stop_time = max(coverage.stop_time, boundStop);
        }
      }

      // Convert only endpoint ticks to ET since we only care about
      // the range of the window coverage
      for (const auto &[id, ticks] : read.intervals) {
        daf::Window w = ticks;
        if (read.isCk) {
          SpiceInt sclk = clockFor(id);
          for (auto &iv : w) {
            iv.first = toEt(sclk, iv.first);
            iv.second = toEt(sclk, iv.second);
          }
          checkNaifErrors();
          w = daf::merge(std::move(w));
        }
        for (auto &[begin, end] : w) {
          coverage.interval_starts.push_back(begin);
          coverage.interval_stops.push_back(end);
          coverage.interval_body_codes.push_back(id);
        }
      }

      sortCoverageRows(coverage);
      result.ok = true;
    }
    catch (exception &e) {
      result.error = e.what();
    }
    return result;
  }


  // Raw DAF reads of kernels by kernel path
  using CoverageCache = unordered_map<string, DafRead>;


  /**
   * @brief Reads the kernels that aren't in the cache yet, on a pool of threads.
   *
   * Safe because daf_reader.h doesn't touch CSPICE. Missions that depend on
   * another one inherit its kernel lists, so each kernel is read once however 
   * many lists it's in.
   *
   * @param kernels Kernel paths, each with its type in the config ("ck" or "spk")
   **/
  static void readCoverage(const vector<pair<string, string>> &kernels, CoverageCache &cache) {
    vector<const pair<string, string> *> unread;
    unordered_set<string> queued;
    for (const auto &kernel : kernels) {
      if (!cache.count(kernel.first) && queued.insert(kernel.first).second) {
        unread.push_back(&kernel);
      }
    }
    if (unread.empty()) return;

    auto t_read = DbClock::now();
    vector<DafRead> reads(unread.size());
    size_t nthreads = max<size_t>(1, min(coverageWorkerCount(), unread.size()));
    atomic<size_t> next{0};
    auto work = [&] {
      for (size_t i = next++; i < unread.size(); i = next++) {
        reads[i] = readDafCoverage(unread[i]->first, unread[i]->second);
      }
    };
    vector<thread> threads;
    for (size_t t = 1; t < nthreads; t++) {
      threads.emplace_back(work);
    }
    work();
    for (thread &t : threads) {
      t.join();
    }

    for (size_t i = 0; i < unread.size(); i++) {
      cache.emplace(unread[i]->first, std::move(reads[i]));
    }
    SPDLOG_DEBUG("daf coverage: read {} of {} kernels in {:.3f}s on {} threads",
                 unread.size(), kernels.size(), secondsSince(t_read), nthreads);
  }


  // The kernels in one of a mission's CK or SPK lists
  static vector<string> timeKernels(const string &mission, const string &type, const string &quality) {
    Config conf;
    conf = conf[mission];

    json timeJson = conf.getRecursive(type);
    json ckKernelGrp = timeJson[type][quality]["kernels"];

    vector<string> kernels;
    for(auto &arr : ckKernelGrp) {
      for(auto &subArr : arr) {
        for (auto &kernel : subArr) {
          kernels.push_back(kernel);
        }
      }
    }
    return kernels;
  }


  void collectStartStopTimes(string mission, string type, string quality, TimeIndexedKernels *kernel_times,
                             CoverageCache &coverage_cache) {
    SPDLOG_TRACE("In globTimeIntervals.");
    vector<string> kernels = timeKernels(mission, type, quality);

    // Use threading to read all the kernels to get their coverage                          
    vector<pair<string, string>> typed;
    for (const string &kernel : kernels) {
      typed.emplace_back(kernel, type);
    }
    readCoverage(typed, coverage_cache);

    vector<CoverageResult> coverages;
    coverages.reserve(kernels.size());
    for (const string &kernel : kernels) {
      coverages.push_back(convertDafCoverage(coverage_cache.at(kernel)));
    }

    for (size_t k = 0; k < kernels.size(); k++) {
      const string &kernel = kernels[k];
      
      // Fall back to kernel start and stop times if interval data could not
      // be collected
      KernelCoverage &coverage = coverages[k].coverage;
      bool has_coverage = coverages[k].ok;
      pair<double, double> sstimes;
      if (has_coverage) {
        sstimes = {coverage.start_time, coverage.stop_time};
      }
      else {
        SPDLOG_WARN("Could not collect intervals for {}: {}", kernel, coverages[k].error);
        sstimes = getKernelStartStopTimes(kernel);
      }
      SPDLOG_TRACE("{} times: {}, {}", std::string(kernel), sstimes.first, sstimes.second); 
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

      if (has_coverage) {
        kernel_times->interval_starts.insert(kernel_times->interval_starts.end(),
            coverage.interval_starts.begin(), coverage.interval_starts.end());
        kernel_times->interval_stops.insert(kernel_times->interval_stops.end(),
            coverage.interval_stops.begin(), coverage.interval_stops.end());
        kernel_times->interval_body_codes.insert(kernel_times->interval_body_codes.end(),
            coverage.interval_body_codes.begin(), coverage.interval_body_codes.end());
        kernel_times->interval_kindex.insert(kernel_times->interval_kindex.end(),
            coverage.interval_starts.size(), index);
      }

      // get relative path to make db portable 
      fs::path relative_path_kernel = fs::relative(kernel, fs::absolute(getDataDirectory()));
      SPDLOG_TRACE("Relative Kernel: {}", relative_path_kernel.generic_string()); 
      kernel_times->file_paths.push_back(relative_path_kernel.string());
    }
  }


  static void insertFramePair(int code, const string &name,
                              vector<int> &codes, vector<string> &names,
                              unordered_set<int> &seen_codes) {
    if (name.empty()) return;
    if (seen_codes.count(code)) return;
    seen_codes.insert(code);
    codes.push_back(code);
    names.push_back(name);
  }


  void InventoryImpl::collectFrameInfo() {
    Config config;

    // Frame list = the top-level config keys (deps only merge into existing
    // keys, so this is the authoritative set).
    json globalConf = config.globalConf();
    for (auto &el : globalConf.items()) {
      m_frame_list.push_back(el.key());
    }
    sort(m_frame_list.begin(), m_frame_list.end());

    unordered_set<int> seen_codes;

    {
      SPICEINT_CELL(builtin, 10000);
      scard_c(0, &builtin);
      bltfrm_c(SPICE_FRMTYP_ALL, &builtin);
      checkNaifErrors();
      SpiceInt nbuiltin = card_c(&builtin);
      for (SpiceInt i = 0; i < nbuiltin; i++) {
        SpiceInt fcode = SPICE_CELL_ELEM_I(&builtin, i);
        SpiceChar fname[128];
        frmnam_c(fcode, 128, fname);
        if (strlen(fname) > 0) {
          insertFramePair((int)fcode, string(fname), m_frame_codes, m_frame_names, seen_codes);
        }
      }
      checkNaifErrors();
      SPDLOG_DEBUG("collectFrameInfo: {} built-in frames", nbuiltin);
    }

    for (auto &el : globalConf.items()) {
      string mission = el.key();
      json textKernels;
      try {
        textKernels = getLatestKernels(config[mission].getRecursive("fk"));
        json iks = getLatestKernels(config[mission].getRecursive("ik"));
        merge_json(textKernels, iks);
      }
      catch (exception &e) {
        SPDLOG_TRACE("collectFrameInfo: no fk/ik for {}: {}", mission, e.what());
      }

      if (textKernels.is_null() || textKernels.empty()) {
        continue;
      }

      KernelSet ks(textKernels);
      checkNaifErrors();

      const SpiceInt ROOM = 5000;
      const SpiceInt LENOUT = 128;
      {
        SpiceInt n = 0;
        SpiceBoolean found = SPICEFALSE;
        vector<SpiceInt> bodyCodes(ROOM);
        gipool_c("NAIF_BODY_CODE", 0, ROOM, &n, bodyCodes.data(), &found);
        if (found && n > 0) {
          SpiceInt nNames = 0;
          SpiceBoolean foundNames = SPICEFALSE;
          vector<SpiceChar> bodyNames(static_cast<size_t>(ROOM) * LENOUT);
          gcpool_c("NAIF_BODY_NAME", 0, ROOM, LENOUT, &nNames, (void *)bodyNames.data(), &foundNames);
          if (foundNames) {
            SpiceInt count = std::min(n, nNames);
            for (SpiceInt i = 0; i < count; i++) {
              string name(&bodyNames[static_cast<size_t>(i) * LENOUT]);
              insertFramePair((int)bodyCodes[i], name, m_frame_codes, m_frame_names, seen_codes);
            }
          }
        }
      }

      SPICEINT_CELL(idset, 10000);
      scard_c(0, &idset);
      kplfrm_c(SPICE_FRMTYP_ALL, &idset);
      checkNaifErrors();
      SpiceInt nframes = card_c(&idset);
      for (SpiceInt i = 0; i < nframes; i++) {
        SpiceInt fcode = SPICE_CELL_ELEM_I(&idset, i);
        SpiceChar fname[128];
        frmnam_c(fcode, 128, fname);
        if (strlen(fname) > 0) {
          insertFramePair((int)fcode, string(fname), m_frame_codes, m_frame_names, seen_codes);
        }
      }
      checkNaifErrors();
    }

    SPDLOG_DEBUG("collectFrameInfo: {} frames in list, {} code<->name pairs",
                 m_frame_list.size(), m_frame_codes.size());
  }


  InventoryImpl::InventoryImpl(bool force_regen, vector<string> mlist) : m_required_kernels() {
    fs::path db_root = getCacheDir();
    fs::path db_file = db_root / DB_HDF_FILE; 

    // create the database 
    if (!fs::exists(db_root) || force_regen) { 
      // check that a file can be created in db_root
      string msg = "";
      try {
        string temp_filename = "spiceql.tmp";
        SPDLOG_TRACE("Creating temporary file [{}] to check create/write permissions at DB path {}.", temp_filename, db_root.string());
        fs::path temp_path = db_root / temp_filename;
        std::ofstream temp_file(temp_path.string());
        if(!temp_file.is_open()) {
          msg = "Could not create file at DB path [" + db_root.string() + "].";
        }

        // write to file
        temp_file << "Writing to file." << std::endl;
        if (temp_file.fail()) {
          msg = "Could not write to file at DB path [" + db_root.string() + "].";
        }
        temp_file.close();
        fs::remove(temp_path);
      } catch (exception &e) {
        throw runtime_error("MESSAGE: " + msg + ", ERROR: " + string(e.what()) + ".");
      }

      auto t_total = DbClock::now();

      auto t_phase = DbClock::now();
      Config config; 
      SPDLOG_INFO("create_database: loading config took {:.2f}s", secondsSince(t_phase));

      t_phase = DbClock::now();
      json json_kernels = {};
      // Verify mlist has acceptable mission names
      vector<string> lowercase_mlist;
      if (mlist.size() > 0) {
        for (auto m : mlist) {
          m = toLower(m);
          if (!config.contains(m)) {
            SPDLOG_TRACE("Config does not contain mission: {}", m);
            throw runtime_error("Mission [" + m + "] is not an acceptable mission name.");
          }
          lowercase_mlist.push_back(m);
        }
        // Resolve only specified mission list
        json_kernels = getLatestKernels(config.get(lowercase_mlist));
      }
      else {
        // Resolve everything
        json_kernels = getLatestKernels(config.get()); 
      }
      
      SPDLOG_INFO("create_database: resolving kernel lists took {:.2f}s", secondsSince(t_phase));

      // load time kernels for creating the timed kernel DataBase 
      t_phase = DbClock::now();
      json lsk_json = getLatestKernels(config["base"].getRecursive("lsk")); 

      SPDLOG_TRACE("InventoryImpl LSKs: {}", lsk_json.dump(4));

      m_required_kernels.load(lsk_json); 
      SPDLOG_INFO("create_database: furnishing LSKs took {:.2f}s", secondsSince(t_phase));

      double sec_sclk = 0, sec_kernel_reads = 0, sec_text = 0;
      size_t n_kernels_read = 0, n_missions = 0;
      CoverageCache coverage_cache;

      // Read all the CKs and SPKs first then loop through each mission's SCLK 
      // files to convert CK times into ET
      t_phase = DbClock::now();
      {
        vector<pair<string, string>> time_kernels;
        for (auto &[mission, kernels] : json_kernels.items()) {
          for (const string type : {"ck", "spk"}) {
            if (!kernels.contains(type)) continue;
            for (auto &quality : KERNEL_QUALITIES) {
              if (!kernels[type].contains(quality)) continue;
              for (string &kernel : timeKernels(mission, type, quality)) {
                time_kernels.emplace_back(std::move(kernel), type);
              }
            }
          }
        }
        readCoverage(time_kernels, coverage_cache);
      }
      double sec_daf_reads = secondsSince(t_phase);
      SPDLOG_INFO("create_database: reading {} unique CKs and SPKs took {:.2f}s ({:.1f} ms each)",
                  coverage_cache.size(), sec_daf_reads,
                  coverage_cache.empty() ? 0.0 : sec_daf_reads * 1000.0 / coverage_cache.size());

      t_phase = DbClock::now();
      for (auto &[mission, kernels] : json_kernels.items()) {
        SPDLOG_TRACE("MISSION: {}", mission);
        n_missions++;

        auto t_step = DbClock::now();
        json sclk_json = getLatestKernels(config[mission].getRecursive("sclk")); 
        SPDLOG_TRACE("{} SCLKs: {}", mission, sclk_json.dump(4)); 
        KernelSet sclks_ks(sclk_json); 
        sec_sclk += secondsSince(t_step);

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
                auto t_read = DbClock::now();
                collectStartStopTimes(mission, kernel_type, quality, tkernels, coverage_cache); 
                sec_kernel_reads += secondsSince(t_read);
                // file_paths has one entry per kernel actually read
                n_kernels_read += tkernels->file_paths.size();
                m_timedep_kerns[map_key] = tkernels;
              }
            }
          } 
          else { // it's a txt kernel or some other non-time dependant kernel 
            auto t_txt = DbClock::now();
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
                  kernel_vec.push_back(relative_path_kernel.string());
                } 
              m_nontimedep_kerns[btree_key] = kernel_vec; 
            } 
            sec_text += secondsSince(t_txt);
          }
        } 
      }

      SPDLOG_INFO("create_database: {} missions took {:.2f}s "
                  "(sclk {:.2f}s, kernel time conversion {:.2f}s over {} kernels, text {:.2f}s)",
                  n_missions, secondsSince(t_phase), sec_sclk, sec_kernel_reads,
                  n_kernels_read, sec_text);

      // Precompute frame caches (frame list + bidirectional code<->name map)
      // so runtime resolution never needs to furnish slow FKs.
      t_phase = DbClock::now();
      collectFrameInfo();
      SPDLOG_INFO("create_database: collectFrameInfo took {:.2f}s", secondsSince(t_phase));

      // write everything out
      t_phase = DbClock::now();
      write_database();
      SPDLOG_INFO("create_database: write_database took {:.2f}s", secondsSince(t_phase));
      SPDLOG_INFO("create_database: TOTAL {:.2f}s", secondsSince(t_total));
    }
    else { // load the database
      // read_database();
    }
  }


  template<class T>
  T InventoryImpl::getKey(string key) { 
    string hdf_file = (fs::path(getCacheDir()) / DB_HDF_FILE).string();

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
                                  vector<Kernel::Quality> ckQualities, vector<Kernel::Quality> spkQualities, bool full_kernel_path, 
                                  int limit_ck, int limit_spk, bool overwrite, bool get_intervals) { 
      json kernels;
      // simply iterate over the names
      for(auto &name : spiceql_names) { 
        json subKernels = search_for_kernelset(name, types, start_time, stop_time,
                                  ckQualities, spkQualities, full_kernel_path, limit_ck, limit_spk, get_intervals); 
                                  
        SPDLOG_TRACE("subkernels for {}: {}", name, subKernels.dump(4));
        SPDLOG_TRACE("Overwrite? {}", overwrite);
        merge_json(kernels, subKernels, overwrite);
      }
      return kernels;
  }


  json InventoryImpl::search_for_kernelset(string spiceql_name, vector<Kernel::Type> types, double start_time, double stop_time,
                                  vector<Kernel::Quality> ckQualities, vector<Kernel::Quality> spkQualities, bool full_kernel_path,
                                  int limit_ck, int limit_spk, bool get_intervals) { 
    // get time dep kernels first 
    json kernels;
    json intervals = json::object();
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

        int limitQuality = limit_spk;
        vector<Kernel::Quality> qualities = spkQualities;
        string qkey = spiceql_name+"_spk_quality";
        if (type == Kernel::Type::CK) { 
          qualities = ckQualities;
          qkey = spiceql_name+"_ck_quality";
          limitQuality = limit_ck;
        } 

        std::sort(qualities.begin(), qualities.end());

        // The union across qualities, in load order, plus the quality that gets
        // reported for it. The union is a chain rather than a choice: a smithed
        // CK carries a tie-point correction but no spacecraft bus attitude, so
        // the lower tier is not redundant, it is load-bearing. Reporting the
        // best tier present would therefore overstate the result — a smithed
        // kernel overlapping one hour of a month-long window would label the
        // whole month smithed. The lowest tier that contributed is reported
        // instead, so the quality describes the weakest link the chain depends
        // on.
        vector<string> union_kernels;
        Kernel::Quality lowest_quality = Kernel::Quality::NOQUALITY;
        bool found_quality = false;

        // Each config entry that contributed a tier, mapped to the best tier it
        // contributed. Reported as <owner>_<type>_quality so a caller can see
        // what each part of the chain is worth, not just the floor. Ordered for
        // a stable result.
        std::map<string, Kernel::Quality> owner_qualities;

        // attempt all requested qualities  
        for(auto quality = qualities.begin(); quality != qualities.end(); ++quality) {
          string key = spiceql_name+"/"+Kernel::translateType(type)+"/"+Kernel::translateQuality(*quality)+"/";
          SPDLOG_DEBUG("Key: {}", key);

          if (m_timedep_kerns.contains(key)) { 
            SPDLOG_DEBUG("Key {} found", key); 
            time_indices = m_timedep_kerns[key]; 
          }
          else {
            // try to load the binary files 
            time_indices = new TimeIndexedKernels();
            
            try {
              SPDLOG_TRACE("Starting deserializing the DB");

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
              // load stop_times 
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
          vector<int> final_time_kernel_indices;

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
            final_time_kernel_indices.push_back(stop_lower_bound->second);
          }
          else { 
            for(auto it = stop_lower_bound;it != time_indices->stop_times.end(); it++) { 
              // if it's also in the start_time set, add it to the list
              iterations++;
              SPDLOG_TRACE("Is {} with stop time {} in the array? {}", time_indices->file_paths.at(it->second), it->first, start_time_kernels.contains(it->second)); 
              if (start_time_kernels.contains(it->second)) {
                final_time_kernel_indices.push_back(it->second);
              }
            }
          }
          
          // Sort the indices as the kernel dbs enforce load priority
          sort(final_time_kernel_indices.begin(), final_time_kernel_indices.end());
          for (auto index : final_time_kernel_indices) {
            final_time_kernels.push_back(time_indices->file_paths.at(index));
          }

          if (final_time_kernels.size()) {
            // Ascending, so the first tier to contribute is the lowest one.
            // NOQUALITY is itself a tier, hence the flag rather than a sentinel.
            if (!found_quality) {
              lowest_quality = *quality;
              found_quality = true;
            }

            // Ascending again, so plain assignment leaves each owner holding the
            // best tier it contributed.
            for (auto &owner : qualityOwners(spiceql_name, Kernel::translateType(type),
                                             Kernel::translateQuality(*quality))) {
              owner_qualities[owner] = *quality;
            }

            vector<string> tier;
            if (limitQuality > -1 && limitQuality < final_time_kernels.size()) { 
              int start_idx = final_time_kernels.size() - 1;
              int stop_idx = start_idx - limitQuality;
              for (auto i = start_idx; i > stop_idx; --i) {
                tier.push_back(final_time_kernels[i]);
              }
            }
            else { 
              tier = final_time_kernels;
            }

            if (get_intervals) {
              json group_intervals = get_group_intervals(key, tier);
              for (auto &[k, v] : group_intervals.items()) {
                intervals[k] = v;
              }
            }

            for (string &e : tier) { 
              union_kernels.push_back(full_kernel_path ? (data_dir / e).string() : e);
            }
          }
          SPDLOG_TRACE("NUMBER OF ITERATIONS: {}", iterations);
          SPDLOG_TRACE("NUMBER OF KERNELS FOUND: {}", final_time_kernels.size());  
        }

        if (union_kernels.size()) {
          kernels[Kernel::translateType(type)] = union_kernels;

          // Owners first, then the searched name. spiceql_name is usually an
          // owner of its own kernels too, and for that key the floor is what
          // should be reported — so the write order matters.
          for (auto &[owner, owner_quality] : owner_qualities) {
            kernels[owner+"_"+Kernel::translateType(type)+"_quality"] =
                Kernel::translateQuality(owner_quality);
          }
          kernels[qkey] = Kernel::translateQuality(lowest_quality);

          SPDLOG_DEBUG("Unioned {} {} kernels across qualities, reporting lowest quality {}",
                       union_kernels.size(), Kernel::translateType(type),
                       Kernel::translateQuality(lowest_quality));
        }
      }
      else { // text/non time based kernels
        SPDLOG_DEBUG("Trying to search time independent kernels");
        string key = spiceql_name+"/"+Kernel::translateType(type)+"/"; 
        SPDLOG_DEBUG("GETTING {} with key {}", Kernel::translateType(type), key);
        if (m_nontimedep_kerns.contains(key) && !m_nontimedep_kerns[key].empty()) {  
          vector<string> ks = m_nontimedep_kerns[key];
          if (full_kernel_path) {
            for(auto &e : ks) e = (data_dir / e).string(); // re-add the data dir
          }
          kernels[Kernel::translateType(type)] = ks;
        
        }
        else { 
          // load from DB 
          try { 
            vector<string> ks = getKey<vector<string>>(DB_SPICE_ROOT_KEY + "/"+key);
            if (full_kernel_path) {
              for(auto &e : ks) e = (data_dir / e).string(); // re-add the data dir
            }
            kernels[Kernel::translateType(type)] = ks;
          } catch (runtime_error &e) { 
            SPDLOG_TRACE("{}", e.what()); // usually a key not found error
            continue;
          }
        }
      }
    }

    if (get_intervals) {
      kernels["intervals"] = intervals;
    }
    return kernels;
  }


  // Checks if the kernel is stored in the group
  static bool kernelMatches(const string &kernel, const string &path, const string &key) {
    fs::path named(kernel);
    if (!named.has_parent_path()) {
      return true;
    }

    // A relative or full path to the kernel
    string named_str = named.generic_string();
    string stored_str = fs::path(path).generic_string();
    if (named_str == stored_str || named_str.ends_with("/" + stored_str) || stored_str.ends_with("/" + named_str)) {
      return true;
    }

    // A SpiceQL key, /<mission>/<type>/<quality>/<filename>
    string parent = named.parent_path().generic_string();
    if (parent.starts_with("/")) {
      parent.erase(0, 1);
    }
    return parent + "/" == key;
  }


  json InventoryImpl::get_group_intervals(string key, vector<string> kernels) {
    json intervals = json::object();
    if (kernels.empty()) {
      return intervals;
    }

    // Use the in-memory index while the DB is being built, otherwise load it
    TimeIndexedKernels loaded;
    TimeIndexedKernels *time_indices = &loaded;
    string group_key = DB_SPICE_ROOT_KEY + "/" + key;
    bool from_db = !m_timedep_kerns.contains(key);
    if (from_db) {
      try {
        loaded.file_paths = getKey<vector<string>>(group_key + DB_TIME_FILES_KEY);
      }
      catch (runtime_error &e) {
        SPDLOG_TRACE("Couldn't find {}. {}", group_key, e.what());
        return intervals;
      }
    }
    else {
      time_indices = m_timedep_kerns[key];
    }

    unordered_multimap<string, size_t> by_filename;
    for (size_t i = 0; i < time_indices->file_paths.size(); i++) {
      by_filename.emplace(fs::path(time_indices->file_paths[i]).filename().string(), i);
    }

    vector<size_t> matches;
    for (const string &kernel : kernels) {
      auto [first, last] = by_filename.equal_range(fs::path(kernel).filename().string());
      for (auto it = first; it != last; ++it) {
        // e.g. If a kernel filename without any path is passed in and exists in both
        // tgo/ck/predicted/ and cassis/ck/predicted, two interval entries will be recorded
        if (kernelMatches(kernel, time_indices->file_paths[it->second], key)) {
          matches.push_back(it->second);
        }
      }
    }
    if (matches.empty()) {
      return intervals;
    }

    // Only read the interval datasets once a kernel is known to be in the group
    if (from_db) {
      try {
        loaded.interval_starts = getKey<vector<double>>(group_key + DB_INTERVAL_START_KEY);
        loaded.interval_stops = getKey<vector<double>>(group_key + DB_INTERVAL_STOP_KEY);
        loaded.interval_kindex = getKey<vector<size_t>>(group_key + DB_INTERVAL_START_INDICES_KEY);
        loaded.interval_body_codes = getKey<vector<int>>(group_key + DB_INTERVAL_BODY_CODES_KEY);
      }
      catch (runtime_error &e) {
        SPDLOG_DEBUG("No intervals for {}. {}", group_key, e.what());
      }
    }

    size_t nrows = time_indices->interval_kindex.size();
    if (time_indices->interval_starts.size() != nrows || time_indices->interval_stops.size() != nrows ||
        time_indices->interval_body_codes.size() != nrows) {
      SPDLOG_WARN("Interval datasets for {} have mismatched sizes, ignoring them", group_key);
      nrows = 0;
    }

    // Rows are grouped by kernel in file_paths order
    auto kindex_begin = time_indices->interval_kindex.begin();
    auto kindex_end = kindex_begin + nrows;
    for (size_t index : matches) {
      json rows = json::array();
      auto [first, last] = equal_range(kindex_begin, kindex_end, index);
      for (auto it = first; it != last; ++it) {
        size_t row = it - kindex_begin;
        rows.push_back(json{{"start", time_indices->interval_starts[row]},
                            {"stop", time_indices->interval_stops[row]},
                            {"body_code", time_indices->interval_body_codes[row]}});
      }
      intervals["/" + key + fs::path(time_indices->file_paths[index]).filename().string()] = rows;
    }
    return intervals;
  }


  json InventoryImpl::get_kernel_intervals(vector<string> kernels) {
    json intervals = json::object();
    if (kernels.empty()) {
      return intervals;
    }

    string hdf_file = (fs::path(getCacheDir()) / DB_HDF_FILE).string();
    if (!fs::exists(hdf_file)) {
      throw runtime_error("DB for kernels (" + hdf_file + ") does not exist");
    }

    // Time dependent groups are <mission>/<ck|spk>/<quality>/
    vector<string> keys;
    {
      HighFive::File file(hdf_file, HighFive::File::ReadOnly);
      if (!file.exist(DB_SPICE_ROOT_KEY)) {
        return intervals;
      }
      HighFive::Group root = file.getGroup(DB_SPICE_ROOT_KEY);
      for (const string &mission : root.listObjectNames()) {
        if (root.getObjectType(mission) != HighFive::ObjectType::Group) {
          continue;
        }
        HighFive::Group mission_group = root.getGroup(mission);
        for (string type : {"ck", "spk"}) {
          if (!mission_group.exist(type) || mission_group.getObjectType(type) != HighFive::ObjectType::Group) {
            continue;
          }
          HighFive::Group type_group = mission_group.getGroup(type);
          for (const string &quality : type_group.listObjectNames()) {
            if (type_group.getObjectType(quality) == HighFive::ObjectType::Group) {
              keys.push_back(mission + "/" + type + "/" + quality + "/");
            }
          }
        }
      }
    }

    for (const string &key : keys) {
      json group_intervals = get_group_intervals(key, kernels);
      for (auto &[k, v] : group_intervals.items()) {
        intervals[k] = v;
      }
    }

    for (const string &kernel : kernels) {
      string suffix = "/" + fs::path(kernel).filename().string();
      bool found = false;
      for (auto &[k, v] : intervals.items()) {
        if (k.ends_with(suffix)) {
          found = true;
          break;
        }
      }
      if (!found) {
        SPDLOG_WARN("Could not find {} in any CK or SPK group", kernel);
      }
    }
    return intervals;
  }


  void InventoryImpl::write_database() { 
    fs::path db_root = getCacheDir(); 
    string hdf_file = (db_root / DB_HDF_FILE).string();
    
    // delete if exists
    fs::remove(hdf_file);
    H5Easy::File file(hdf_file, H5Easy::File::Create); 

    // Write version
    HighFive::Group group = file.getGroup("/");
    group.createAttribute<std::string>("SPICEQL_VERSION", SPICEQL_VERSION);

    // Write the precomputed frame caches: the frame list and the bidirectional
    // code<->name map (two aligned arrays, no redundant storage).
    if (!m_frame_list.empty()) {
      H5Easy::dump(file, "/" + DB_FRAME_LIST_KEY, m_frame_list, H5Easy::DumpMode::Overwrite);
    }
    if (!m_frame_codes.empty()) {
      H5Easy::dump(file, "/" + DB_FRAME_CODES_KEY, m_frame_codes, H5Easy::DumpMode::Overwrite);
      H5Easy::dump(file, "/" + DB_FRAME_NAMES_KEY, m_frame_names, H5Easy::DumpMode::Overwrite);
    }

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

        // Intervals are row-aligned rather than sorted per column like the
        // times above: row i of every interval dataset describes the same
        // interval, so a start keeps its stop and body code.
        // The start and stop kindex datasets therefore hold the same values.
        string group_key = DB_SPICE_ROOT_KEY + "/" + kernel_key + "/";
        if (kernels->interval_starts.size() > 0) {
          H5Easy::dump(file, group_key + DB_INTERVAL_START_KEY, kernels->interval_starts, H5Easy::DumpMode::Overwrite);
          H5Easy::dump(file, group_key + DB_INTERVAL_STOP_KEY, kernels->interval_stops, H5Easy::DumpMode::Overwrite);
          H5Easy::dump(file, group_key + DB_INTERVAL_START_INDICES_KEY, kernels->interval_kindex, H5Easy::DumpMode::Overwrite);
          H5Easy::dump(file, group_key + DB_INTERVAL_STOP_INDICES_KEY, kernels->interval_kindex, H5Easy::DumpMode::Overwrite);
          H5Easy::dump(file, group_key + DB_INTERVAL_BODY_CODES_KEY, kernels->interval_body_codes, H5Easy::DumpMode::Overwrite);
        }
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


  namespace {
    std::mutex g_frame_cache_mutex;
    std::string g_frame_cache_key;  // "<path>@<write-time>" of the loaded DB
    std::unordered_map<int, std::string> g_code_to_name;
    std::unordered_map<std::string, int> g_name_to_code;  // keyed on UPPER name

    void loadFrameCache(InventoryImpl *impl) {
      std::lock_guard<std::mutex> lock(g_frame_cache_mutex);

      string hdf_file = (fs::path(getCacheDir()) / DB_HDF_FILE).string();
      string key = hdf_file;
      try {
        if (fs::exists(hdf_file)) {
          key += "@" + std::to_string(
              static_cast<long long>(fs::last_write_time(hdf_file).time_since_epoch().count()));
        }
      } catch (...) { /* fall through with path-only key */ }

      if (key == g_frame_cache_key) {
        return;  // already loaded for this exact DB state
      }

      g_code_to_name.clear();
      g_name_to_code.clear();
      try {
        vector<int> codes = impl->getKey<vector<int>>(DB_FRAME_CODES_KEY);
        vector<string> names = impl->getKey<vector<string>>(DB_FRAME_NAMES_KEY);
        size_t n = std::min(codes.size(), names.size());
        g_code_to_name.reserve(n);
        g_name_to_code.reserve(n);
        for (size_t i = 0; i < n; i++) {
          g_code_to_name[codes[i]] = names[i];
          g_name_to_code[toUpper(names[i])] = codes[i];
        }
      }
      catch (exception &e) {
        SPDLOG_DEBUG("Frame code<->name cache unavailable: {}", e.what());
      }
      g_frame_cache_key = key;
    }
  }


  vector<string> InventoryImpl::getFrameList() {
    try {
      return getKey<vector<string>>(DB_FRAME_LIST_KEY);
    }
    catch (exception &e) {
      SPDLOG_DEBUG("Frame list cache unavailable ({}), computing from config", e.what());
      vector<string> frames;
      // Bind to a named json so .items() doesn't iterate a destroyed temporary.
      json globalConf = Config().globalConf();
      for (auto &el : globalConf.items()) {
        frames.push_back(el.key());
      }
      sort(frames.begin(), frames.end());
      return frames;
    }
  }


  string InventoryImpl::getFrameName(int code) {
    loadFrameCache(this);
    auto it = g_code_to_name.find(code);
    if (it != g_code_to_name.end()) return it->second;
    return "";
  }


  int InventoryImpl::getFrameCode(string name) {
    loadFrameCache(this);
    auto it = g_name_to_code.find(toUpper(name));
    if (it != g_name_to_code.end()) return it->second;
    return 0;
  }

};

