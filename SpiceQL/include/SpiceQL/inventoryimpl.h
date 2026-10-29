#pragma once
/**
 * @file
 *
 * internal database for the kernel inventory 
 *
 **/

#include <string>
#include <vector>
#include <tuple>
#include <limits>

// The BTree submodule's disk_fixed_alloc.h only defines the stdpmr namespace
// alias for clang and GCC. Provide it for MSVC so the BTree headers compile on
// Windows.
#if defined(_MSC_VER) && !defined(SPICEQL_STDPMR_DEFINED)
#define SPICEQL_STDPMR_DEFINED
#include <memory_resource>
namespace stdpmr = std::pmr;
#endif

#include <fc/btree.h>
#include <nlohmann/json.hpp>

#include <SpiceQL/spice_types.h>

namespace SpiceQL {

  extern std::string DB_HDF_FILE;
  extern std::string DB_START_TIME_KEY;
  extern std::string DB_STOP_TIME_KEY;
  extern std::string DB_TIME_FILES_KEY;
  extern std::string DB_SS_TIME_INDICES_KEY;
  extern std::string DB_SPICE_ROOT_KEY;
  // Per-kernel coverage rows, stored alongside the time index.
  extern std::string DB_INTERVAL_START_KEY;
  extern std::string DB_INTERVAL_STOP_KEY;
  extern std::string DB_INTERVAL_START_INDICES_KEY;
  extern std::string DB_INTERVAL_STOP_INDICES_KEY;
  extern std::string DB_INTERVAL_BODY_CODES_KEY;
  // Precomputed frame caches (built during create_database) so runtime
  // resolution never needs to furnish slow FKs. Stored under one group.
  extern std::string DB_FRAME_CACHE_KEY;
  extern std::string DB_FRAME_LIST_KEY;
  extern std::string DB_FRAME_CODES_KEY;
  extern std::string DB_FRAME_NAMES_KEY;

  std::string getCacheDir();
  void setCacheDir(std::string cache_dir, bool override=false);
  std::string getHdfFile();
  
  class TimeIndexedKernels { 
    public: 
    frozenca::BTreeMap<double, size_t> start_times; 
    frozenca::BTreeMap<double, size_t> stop_times; 
    std::vector<std::string> file_paths; 

    // Coverage rows, grouped by kernel in file_paths order and sorted by start
    // within a kernel. Row i of the interval_* vectors is one interval of
    // kernel interval_kindex[i].
    std::vector<double> interval_starts;
    std::vector<double> interval_stops;
    std::vector<size_t> interval_kindex;
    std::vector<int> interval_body_codes;
  };


  class InventoryImpl {
    public:
    InventoryImpl(bool force_regen=false, std::vector<std::string> mlist = {});
    template<class T> T getKey(std::string key);
    void write_database();

    /**
     * @brief Returns the cached list of frame/config names.
     *
     * Reads the precomputed list from the DB; falls back to computing it from
     * the live Config if the cache is unavailable.
     */
    std::vector<std::string> getFrameList();

    /**
     * @brief Resolve a frame/body code to its name using the cached map.
     * @return the name, or "" if the code is not in the cache.
     */
    std::string getFrameName(int code);

    /**
     * @brief Resolve a frame/body name to its code using the cached map.
     * @return the code, or 0 if the name is not in the cache.
     */
    int getFrameCode(std::string name);
    nlohmann::json search_for_kernelset(std::string spiceql_name, std::vector<Kernel::Type> types, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(),
                                            std::vector<Kernel::Quality> ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, std::vector<Kernel::Quality> spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
                                            bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1, bool get_intervals=false);
    nlohmann::json search_for_kernelsets(std::vector<std::string> spiceql_names, std::vector<Kernel::Type> types, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(),
                                            std::vector<Kernel::Quality> ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, std::vector<Kernel::Quality> spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
                                            bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1, bool overwrite=false, bool get_intervals=false);

    /**
     * @brief Get the coverage intervals of kernels in one CK or SPK group.
     *
     * Used when the group is already known, e.g. by search_for_kernelset. To
     * search every group, use get_kernel_intervals.
     *
     * A kernel can be named by its bare filename, its path relative to the data
     * dir (or a full path ending in it), or its SpiceQL key
     * (/<mission>/<type>/<quality>/<filename>). Kernels not in the group are
     * skipped.
     *
     * @param key the group, as <mission>/<type>/<quality>/
     * @param kernels the kernels to get intervals for
     * @returns json object of SpiceQL key to an array of
     *          {"start", "stop", "body_code"} intervals
     */
    nlohmann::json get_group_intervals(std::string key, std::vector<std::string> kernels);

    /**
     * @brief Get the coverage intervals of kernels from any CK or SPK group.
     *
     * Calls get_group_intervals on every time dependent group in the DB, 
     * so a kernel in more than one group gets an entry for each.
     *
     * @param kernels the kernels to get intervals for
     * @returns json object of SpiceQL key to an array of intervals
     */
    nlohmann::json get_kernel_intervals(std::vector<std::string> kernels);
    nlohmann::json m_json_inventory;

    std::map<std::string, std::vector<std::string>> m_nontimedep_kerns;
    std::map<std::string, TimeIndexedKernels*> m_timedep_kerns;

    // Sorted, de-duplicated frame/config names.
    std::vector<std::string> m_frame_list;
    // Bidirectional frame/body code<->name map stored as two aligned arrays
    // (index i pairs m_frame_codes[i] with m_frame_names[i]) to avoid storing
    // the data twice.
    std::vector<int> m_frame_codes;
    std::vector<std::string> m_frame_names;

    // Kernels that always need to be furnished
    KernelSet m_required_kernels;

    private:
    /**
     * @brief Enumerate frame/body code<->name pairs and the frame list into the
     * member caches. Furnishes each mission's text kernels, reads the
     * NAIF_BODY_CODE/NAIF_BODY_NAME pools, and records the config frame list.
     */
    void collectFrameInfo();
  };
}
