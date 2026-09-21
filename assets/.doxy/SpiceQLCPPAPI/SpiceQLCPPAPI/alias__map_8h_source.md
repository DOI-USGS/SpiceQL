

# File alias\_map.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**alias\_map.h**](alias__map_8h.md)

[Go to the documentation of this file](alias__map_8h.md)


```C++
#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace SpiceQL {

  class AliasMap {
    public:
      static AliasMap& instance();

      void load(std::string path);

      std::string getSpiceqlName(const std::string& name);

      void addAliasKey(const std::string &alias, const std::string &spiceqlName);

      nlohmann::json getAliasMap();

      void setAliasMap(const nlohmann::json& newAliasMap);

    private:
      AliasMap() = default; // Prevents others from making new instances

      void ensure_init();

      void load_internal(std::string path);

      // These live inside the class so the Singleton "owns" them
      std::unordered_map<std::string, std::string> m_lookupTable;
      std::mutex m_mutex;
      bool m_initialized = false;
  };

  void load_aliases(std::string path = "");

}
```


