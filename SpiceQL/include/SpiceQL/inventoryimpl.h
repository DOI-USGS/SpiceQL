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
                                            bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1);
    nlohmann::json search_for_kernelsets(std::vector<std::string> spiceql_names, std::vector<Kernel::Type> types, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(),
                                            std::vector<Kernel::Quality> ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, std::vector<Kernel::Quality> spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
                                            bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1, bool overwrite=false);
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
