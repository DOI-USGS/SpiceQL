

# File query.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**query.h**](query_8h.md)

[Go to the documentation of this file](query_8h.md)


```C++
#pragma once

#include <vector>
#include <set>
#include <iostream>
#include <nlohmann/json.hpp>

#include <SpiceQL/spice_types.h>


namespace SpiceQL {
  std::vector<std::string> getLatestKernel(std::vector<std::string> kernels);


  nlohmann::json getLatestKernels(nlohmann::json kernels);


  std::vector<std::string> getKernelVectorValue(std::string key);


  std::string getKernelStringValue(std::string key);


  nlohmann::json listMissionKernels(std::string root,  nlohmann::json conf);


  nlohmann::json globKernels(std::string root, nlohmann::json conf, std::string kernelType);

  
  std::vector<std::string> getKernelsAsVector(nlohmann::json kernels);


  std::set<std::string> getKernelsAsSet(nlohmann::json kernels);

  }
```


