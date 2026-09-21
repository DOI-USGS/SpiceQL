

# File utils.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**utils.h**](utils_8h.md)

[Go to the documentation of this file](utils_8h.md)


```C++

#pragma once

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <regex>
#include <optional>
#include <array>
#include <vector>

#include <nlohmann/json.hpp>

#include <SpiceQL/spice_types.h>

namespace SpiceQL {

  std::string getDefaultLsk();

  std::string gen_random(const int len);

  std::string toUpper(std::string s);


  std::string toLower(std::string s);


  std::string replaceAll(std::string str, const std::string &from, const std::string &to);


  std::vector<std::string> split(const std::string& s, char delim);


  std::vector<std::vector<std::string>> getPathsFromRegex (std::string root, std::vector<std::string> regexes);


  void mergeConfigs(nlohmann::json &baseConfig, const nlohmann::json &mergingConfig);


  std::vector<std::string> ls(std::string const & root, bool recursive);


  std::vector<std::string> glob(std::string const & root,
                             std::string const & reg = ".*",
                             bool recursive=false);


  std::vector<std::pair<double, double>> getTimeIntervals(std::string kpath);


  std::pair<double, double> getKernelStartStopTimes(std::string kpath);

  std::string globKernelStartStopTimes(std::string mission);

  std::string globTimeIntervals(std::string mission);


  std::vector<double> getTargetState(double et, std::string target, std::string observer, std::string frame="J2000", std::string abcorr="NONE"); // use j2000 for default reference frame

  std::vector<double> getTargetOrientation(double et, int toFrame, int refFrame=1); // use j2000 for default reference frame


  nlohmann::json findKeywords(std::string keytpl);


  std::vector<nlohmann::json::json_pointer> findKeyInJson(nlohmann::json in, std::string key, bool recursive=true);


   std::string getKernelType(std::string kernelPath);


  std::string getConfigDirectory();


  void setConfigDirectory(std::string configDir);


  std::vector<std::string> getAvailableConfigFiles();


  std::vector<nlohmann::json> getAvailableConfigs();


   std::string getMissionConfigFile(std::string mission);


   nlohmann::json getMissionConfig(std::string mission);


   std::string getMissionKeys(nlohmann::json config);


  std::string getRestUrl();


  void resolveConfigDependencies(nlohmann::json &config, const nlohmann::json &dependencies);


  size_t eraseAtPointer(nlohmann::json &j, nlohmann::json::json_pointer ptr);


  std::vector<std::string> jsonArrayToVector(nlohmann::json arr);


  std::vector<double> jsonDoubleArrayToVector(nlohmann::json arr);


  std::vector<int> jsonIntArrayToVector(nlohmann::json arr);


   std::vector<std::vector<std::string>> json2DArrayTo2DVector(nlohmann::json arr);


   std::vector<std::vector<double>> json2DFloatArrayTo2DVector(nlohmann::json arr);


  std::vector<std::vector<int>> json2DIntArrayTo2DVector(nlohmann::json arr, bool retainEmpty = false);


  std::vector<std::pair<double, double>> json2DArrayToDoublePair(nlohmann::json arr);


  std::string getDataDirectory();

 
  nlohmann::json merge_json(nlohmann::json &j1, nlohmann::json &j2, bool overwrite=false);


  std::string getRootDependency(nlohmann::json config, std::string pointer);


  bool checkNaifErrors(bool reset=true);


  nlohmann::json loadTranslationKernels(std::string mission, bool loadFk=true, bool loadIk=true, bool loadIak=true);

  nlohmann::json loadSelectKernels(std::string kernelType, std::string mission);

  std::string getAliasMapJsonFile();

  std::string inferMission(const std::vector<std::string>& nameCandidates,
                           const std::vector<int>& codeCandidates);

  nlohmann::json formatKernels(std::vector<std::string> kernelPaths);
}
```


