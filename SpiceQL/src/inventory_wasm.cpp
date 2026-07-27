/**
 * @file
 *
 * WebAssembly (Emscripten) implementation of the Inventory:: kernel-database API.
 *
 * The native inventory (inventory.cpp / inventoryimpl.cpp) is backed by an HDF5
 * database (via HighFive) that indexes a large on-disk kernel tree by time. That
 * dependency is intentionally excluded from the WASM build (see the SPICEQL_WASM
 * branch in the top-level CMakeLists.txt), so there is no kernel-search index in
 * the browser.
 *
 * In the WASM build kernels come from exactly two places:
 *   1. the caller's explicit `kernelList` (resolved here), or
 *   2. kernels already furnished into CSPICE by the caller.
 *
 * Accordingly:
 *   - The time-indexed DB searches (search_for_kernelset / search_for_kernelsets)
 *     throw a clear error telling the caller to pass an explicit kernelList with
 *     searchKernels=false.
 *   - search_for_kernelset_from_regex, which api.cpp calls whenever a non-empty
 *     kernelList is supplied, is reimplemented to treat each list entry as a path
 *     in the Emscripten virtual filesystem and group them by kernel type using the
 *     shared formatKernels() helper (no HDF5, no globbing of a data tree).
 *   - Database-management and frame-cache helpers become no-ops / benign defaults
 *     so callers (config.cpp frameList(), utils.cpp codeToNameNoKernels()) degrade
 *     gracefully to NAIF/CSPICE lookups instead of failing.
 */

#include <nlohmann/json.hpp>

#include <SpiceQL/spiceql_logging.h>
#include <SpiceQL/inventory.h>
#include <SpiceQL/utils.h>

using json = nlohmann::json;
using namespace std;

namespace SpiceQL {
    namespace Inventory {

        static const char *kSearchUnavailableMsg =
            "Kernel search is unavailable in the WASM build (no HDF5 inventory). "
            "Furnish kernels directly, or pass an explicit kernelList with "
            "searchKernels=false.";

        json search_for_kernelset(string /*spiceql_name*/, vector<string> /*types*/,
                                   double /*start_time*/, double /*stop_time*/,
                                   vector<string> /*ckQualities*/, vector<string> /*spkQualities*/,
                                   bool /*full_kernel_path*/, int /*limit_ck*/, int /*limit_spk*/) {
            throw runtime_error(kSearchUnavailableMsg);
        }

        json search_for_kernelsets(vector<string> /*spiceql_names*/, vector<string> /*types*/,
                                    double /*start_time*/, double /*stop_time*/,
                                    vector<string> /*ckQualities*/, vector<string> /*spkQualities*/,
                                    bool /*full_kernel_path*/, int /*limit_ck*/, int /*limit_spk*/,
                                    bool /*overwrite*/) {
            throw runtime_error(kSearchUnavailableMsg);
        }

        json search_for_kernelset_from_regex(vector<string> list, bool /*full_kernel_path*/) {
            // In WASM, each list entry is an explicit path in the virtual FS.
            // Group them by kernel type into the JSON structure KernelSet expects.
            // formatKernels reads the type via CSPICE where possible and otherwise
            // falls back to the file extension.
            SPDLOG_TRACE("[WASM] search_for_kernelset_from_regex over {} explicit paths", list.size());
            return formatKernels(list);
        }

        string getDbFilePath() {
            // No HDF database in the WASM build.
            return "";
        }

        void setDbFilePath(string /*db_file_path*/, bool /*override*/) {
            // No-op: there is no database to point at.
        }

        void create_database(vector<string> /*mlist*/) {
            throw runtime_error(
                "create_database is unavailable in the WASM build (no HDF5 inventory).");
        }

        vector<string> getFrameList() {
            // No cached frame list; callers fall back to CSPICE lookups.
            return {};
        }

        string getFrameNameFromCache(int /*code*/) {
            // Empty => caller (codeToNameNoKernels) falls through to NAIF bodc2n_c.
            return "";
        }

        int getFrameCodeFromCache(string /*name*/) {
            // Zero => not found; caller falls through to NAIF lookups.
            return 0;
        }
    }
}
