

# File config.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**config.h**](config_8h.md)

[Go to the documentation of this file](config_8h.md)


```C++
#pragma once

#include <iostream>
#include <regex>

#include <nlohmann/json.hpp>

#include <SpiceQL/utils.h>

namespace SpiceQL {

  std::vector<std::string> frameList();

  class Config {
    public:

      Config();


      Config(std::string json);


      Config operator[](std::string pointer);

      Config operator[](std::vector<std::string> pointers);

      unsigned int size();


      nlohmann::json get(std::string pointer = "");


      nlohmann::json get(std::vector<std::string> pointers);


      nlohmann::json getRecursive(std::string pointer);


      nlohmann::json getLatestRecursive(std::string pointer="");


      nlohmann::json getLatest(std::string pointer = "");


      nlohmann::json globalConf();


      std::vector<std::string> findKey(std::string key, bool recursive);


      bool contains(std::string key);

      std::string getParentPointer(std::string searchPointer, int pointerPosition = 0);

    private:
      Config(nlohmann::json json, std::string pointer);


      nlohmann::json evaluateConfig(std::string pointerToEval = "");

      nlohmann::json config;

      std::string confPointer;
  };

}
```


