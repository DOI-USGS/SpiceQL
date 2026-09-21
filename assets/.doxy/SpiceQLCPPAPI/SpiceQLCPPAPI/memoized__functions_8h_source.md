

# File memoized\_functions.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**memoized\_functions.h**](memoized__functions_8h.md)

[Go to the documentation of this file](memoized__functions_8h.md)


```C++

#pragma once

#include <vector>
#include <string>

#include <nlohmann/json.hpp>

#include <SpiceQL/utils.h>

namespace SpiceQL {
  namespace  Memo {

  std::vector<std::string> ls(std::string const & root, bool recursive);

  
  std::vector<std::vector<std::string>> getPathsFromRegex (std::string root, std::vector<std::string> regexes);

  std::vector<std::pair<double, double>> getTimeIntervals(std::string kpath);


  std::string globTimeIntervals(std::string mission);
  
  }
}
```


