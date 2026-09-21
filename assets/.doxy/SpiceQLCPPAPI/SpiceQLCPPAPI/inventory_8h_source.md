

# File inventory.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**inventory.h**](inventory_8h.md)

[Go to the documentation of this file](inventory_8h.md)


```C++
#pragma once 

#include <string>
#include <vector>
#include <tuple>
#include <limits>

#include <nlohmann/json.hpp>

#include <SpiceQL/spice_types.h>

namespace SpiceQL {
    namespace Inventory { 
        nlohmann::json search_for_kernelset(std::string spiceql_name, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1);
        nlohmann::json search_for_kernelsets(std::vector<std::string> spiceql_names, std::vector<std::string> types=KERNEL_TYPES, double start_time=-std::numeric_limits<double>::max(), double stop_time=std::numeric_limits<double>::max(), 
                                      std::vector<std::string> ckQualities={"smithed", "reconstructed"}, std::vector<std::string> spkQualities={"smithed", "reconstructed"}, bool full_kernel_path=false, int limit_ck=-1, int limit_spk=1,
                                      bool overwrite=false);    
        nlohmann::json search_for_kernelset_from_regex(std::vector<std::string> list, bool full_kernel_path=false);

        std::string getDbFilePath();
        void setDbFilePath(std::string db_file_path, bool override=false);

        void create_database(std::vector<std::string> mlist = {});

        std::vector<std::string> getFrameList();

        std::string getFrameNameFromCache(int code);

        int getFrameCodeFromCache(std::string name);
    }
}
```


