/**
 * @file
 *
 *
 **/
#pragma once

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <regex>
#include <optional>
#include <array>
#include <vector>

#include <nlohmann/json.hpp>

#include "spice_types.h"

/**
 * @namespace SpiceQL
 *
 */
namespace SpiceQL {


  /**
   * @brief generate a random string
   *
   * @param len length of the string
   * @return new random alphanumeric string
   */
  std::string gen_random(const int len);

  /**
   * @brief force a string to upper case
   *
   * @param s input string
   * @return copy of input string, in upper case
   */
  std::string toUpper(std::string s);


  /**
   * @brief force a string to lower case
   *
   * @param s input string
   * @return copy of input string, in lower case
   */
  std::string toLower(std::string s);


  /**
   * @brief find and replace one substring with another
   *
   * @param str input string to search
   * @param from substring to find
   * @param to substring to replace "from" instances to
   * @return std::string
   */
  std::string replaceAll(std::string str, const std::string &from, const std::string &to);


  /**
    * @brief glob, but with json
    *
    * Lambda for globbing files from a regular expression stored
    * in json. As they can be a single expression or a
    * list, we need to massage the json a little.
    *
    * @param root root path to search
    * @param r json list of regexes
    * @returns vector of paths
   **/
  std::vector<std::vector<std::string>> getPathsFromRegex (std::string root, std::vector<std::string> regexes);


  /**
   * @brief Merge two json configs
   *
   * When arrays are merged, the values from the base config will appear
   * first in the merged config.
   *
   * @param baseConfig The config to merge into
   * @param mergingConfig The config to merge into the base config
   */
  void mergeConfigs(nlohmann::json &baseConfig, const nlohmann::json &mergingConfig);


  /**
    * @brief ls, like in unix, kinda. Also it's a function.
    *
    * Iterates the input path and returning a list of files. Optionally, recursively.
    *
    * @param root The root directory to search
    * @param recursive recursively iterates through directories if true
    *
    * @returns list of paths
   **/
  std::vector<std::string> ls(std::string const & root, bool recursive);


  /**
    * @brief glob, like python's glob.glob, except C++
    *
    * Given a root and a regular expression, give all the files that match.
    *
    * @param root The root directory to search
    * @param reg std::regex object to pattern to search, defaults to ".*", or match averything.
    * @param recursive recursively iterates through directories if true
    *
    * @returns list of paths matching regex
   **/
  std::vector<std::string> glob(std::string const & root,
                             std::string const & reg = ".*",
                             bool recursive=false);


  /**
    * @brief Get start and stop times a kernel.
    *
    * For each segment in the kernel, get all start and stop times as a vector of double pairs.
    * This gets all start and stop times regardless of the frame associated with it.
    *
    * Input kernel is assumed to be a binary kernel with time dependant external orientation data.
    *
    * @param kpath Path to the kernel
    * @returns std::vector of start and stop times
   **/
  std::vector<std::pair<double, double>> getTimeIntervals(std::string kpath);


  std::pair<double, double> getKernelStartStopTimes(std::string kpath);

  std::string globKernelStartStopTimes(std::string mission);

  /**
   * @brief Get start and stop times for all kernels
   * 
   * @return string json map of kernel names to list of time segments
   */
  std::string globTimeIntervals(std::string mission);


  /**
   * @brief Gives the position and velocity for a given frame at some ephemeris time
   *
   * Mostly a C++ wrap for NAIF's spkezr_c
   *
   * @param et ephemeris time at which you want to optain the target state
   * @param target NAIF ID for the target frame
   * @param observer NAIF ID for the observing frame
   * @param frame The reference frame in which to get the positions in
   * @param abcorr aborration correction flag, default it NONE.
   *        This can set to:
   *           "NONE" - No correction
   *        For the "reception" case, i.e. photons from the target being recieved by the observer at the given time.
   *           "LT"   - One way light time correction
   *           "LT+S" - Correct for one-way light time and stellar aberration correction
   *           "CN"   - Converging Newtonian light time correction
   *           "CN+S" - Converged Newtonian light time correction and stellar aberration correction
   *        For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time
   *           "XLT"   - One-way light time correction using a newtonian formulation
   *           "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation
   *           "XCN"   - converged Newtonian light time correction
   *           "XCN+S" - converged Newtonian light time correction and stellar aberration correction.
   *  @return A vector of 7 elements with a 0 - 5 index state vector of position and velocity 
   *          in x,y,z,vx,vy,vz format followed by the light time adjustment at the 6th index.
  **/
  std::vector<double> getTargetState(double et, std::string target, std::string observer, std::string frame="J2000", std::string abcorr="NONE"); // use j2000 for default reference frame

  /**
   * @brief Gives quaternion and angular velocity for a given frame at a given ephemeris time
   *
   * Gets an orientation for an input frame in some reference frame.
   * The orientations returned from this function can be used to transform a position
   * in the source frame to the ref frame.
   *
   * @param et ephemeris time at which you want to optain the target pointing
   * @param toframe the source frame's NAIF code.
   * @param refframe the reference frame's NAIF code, orientations are relative to this reference frame
   * @returns SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with)
  **/
  std::vector<double> getTargetOrientation(double et, int toFrame, int refFrame=1); // use j2000 for default reference frame


  /**
    * @brief finds key:values in kernel pool
    *
    * Given a key template, returns matching key:values from the kernel pool
    *   by using gnpool, gcpool, gdpool, and gipool
    *
    * @param keytpl input key template to search for
    *
    * @returns json list of found key:values
   **/
  nlohmann::json findKeywords(std::string keytpl);


  /**
    * @brief recursively search keys in json.
    *
    * Given a root and a regular expression, give all the files that match.
    *
    * @param in input json to search
    * @param key key to search for
    * @param recursive recursively iterates through objects if true
    *
    * @returns vector of refernces to matching json objects
   **/
  std::vector<nlohmann::json::json_pointer> findKeyInJson(nlohmann::json in, std::string key, bool recursive=true);


  /**
    * @brief get the Kernel type (CK, SPK, etc.)
    *
    *
    * @param kernelPath path to kernel
    * @returns Kernel type as a string
   **/
   std::string getKernelType(std::string kernelPath);


  /**
   * @brief Get the directory pointing to the db files
   * 
   * Default behavior returns the installed DB files in $CONDA_PREFIX/etc/SpiceQL/db.
   *
   * If the env var $SPICEQL_DEV_DB is set, this returns the local source path of 
   * _SOURCE_PREFIX/SpiceQL/db/ 
   * 
   * @return std::string directory containing db files
   */
   std::string getConfigDirectory();
  

  /**
   * @brief Returns a vector of all the available configs
   * 
   * Returns the db files in either the installed or debug directory depending 
   * on whether or not SPICEQL_DEV_DB is set. 
   *
   * @see getConfigDirectory
   *
   * @return std::vector<std::string> 
   */
  std::vector<std::string> getAvailableConfigFiles();


  /**
   * @brief Get names of available config files as a json vector
   *
   * This iterates through all the configs in the db folder either installed 
   * or in the debug directory depending on whether or not SPICEQL_DEV_DB is set. Loads them 
   * as vector of json obects and returns the vector. 
   *
   * @return std::vector<nlohmann::json> 
   */
  std::vector<nlohmann::json> getAvailableConfigs();


  /**
    * @brief Returns the path to the Mission specific Spice config file.
    *
    * Given a mission, search a prioritized list of directories for
    * the json config file. This function checks in the order:
    *
    *   1. The local build dir, i.e. $CMAKE_SOURCE_DIR
    *   2. The install dir, i.e. $CMAKE_PREFIX
    *
    * @param mission mission name of the config file
    *
    * @returns path object of the condig file
   **/
   std::string getMissionConfigFile(std::string mission);


  /**
    * @brief Returns the path to the Mission specific Spice config file.
    *
    * Given a mission, search a prioritized list of directories for
    * the json config file. This function checks in the order:
    *
    *   1. The local build dir, i.e. $CMAKE_SOURCE_DIR
    *   2. The install dir, i.e. $CMAKE_PREFIX
    *
    * @param mission mission name of the config file
    *
    * @returns path object of the config file
   **/
   nlohmann::json getMissionConfig(std::string mission);


  /**
    * @brief Returns a string of mission keys from the JSON config file.
    *
    * @param config JSON config file
    *
    * @returns string of mission keys
   **/
   std::string getMissionKeys(nlohmann::json config);


   /**
    * @brief Returns the REST URL
    *
    * @returns SpiceQL's REST URL as string
   **/
   std::string getRestUrl();


   /**
    * @brief resolve the dependencies in a config in place
    *
    * Given a config with "deps" keys in it and a second config to extract
    * dependencies from, recursively resolve all of the deps into their actual
    * values. Only allows up to 10 recurssions.
    *
    * @param config The config to populate
    * @param dependencies The config to pull dependencies from
    *
    * @returns The full instrument config
    */
   void resolveConfigDependencies(nlohmann::json &config, const nlohmann::json &dependencies);


   /**
    * @brief erase a part of a json object based on a json pointer
    *
    * @param j The json object ot erase part of. Modified in place
    * @param ptr The object to erase
    *
    * @returns The number of objects removed
    */
   size_t eraseAtPointer(nlohmann::json &j, nlohmann::json::json_pointer ptr);


  /**
    * @brief Returns std::vector<string> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns string vector containing arr data
   **/
   std::vector<std::string> jsonArrayToVector(nlohmann::json arr);


  /**
    * @brief Returns std::vector<double> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns double containing arr data
   **/
   std::vector<double> jsonDoubleArrayToVector(nlohmann::json arr);


  /**
    * @brief Returns std::vector<int> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns int containing arr data
   **/
   std::vector<int> jsonIntArrayToVector(nlohmann::json arr);


  /**
    * @brief Returns std::vector<std::vector<string>> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns string vector containing arr data
   **/
   std::vector<std::vector<std::string>> json2DArrayTo2DVector(nlohmann::json arr);


   /**
    * @brief Returns std::vector<std::vector<float>> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns double vector containing arr data
   **/
   std::vector<std::vector<double>> json2DFloatArrayTo2DVector(nlohmann::json arr);


   /**
    * @brief Returns std::vector<std::vector<int>> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns int vector containing arr data
   **/
   std::vector<std::vector<int>> json2DIntArrayTo2DVector(nlohmann::json arr);


  /**
    * @brief Returns std::vector<std::vector<string>> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array.
    * Throws exception if the json obj is not an Nx2 array of doubles.
    *
    * @param arr input json arr
    *
    * @returns pair vector containing arr data
   **/
   std::vector<std::pair<double, double>> json2DArrayToDoublePair(nlohmann::json arr);


  /**
    * @brief Returns std::vector<string> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns string vector containing arr data
   **/
   std::string getDataDirectory();


   /**
    * @brief Merges the right json to the left json 
    *
    * @param j1 input json obj, this json is having it's values edited 
    * @param j2 input json obj to be merged into j1
    * @param overwrite if true, overwrites the keys in j1 in favors of those in j2. If overwrite=true, this is the same as nlohmann::json::merge_patch  
    * 
    * @returns j1 with j2 keys + values
   **/ 
   nlohmann::json merge_json(nlohmann::json &j1, nlohmann::json &j2, bool overwrite=false);


  /**
    * @brief Returns the root most dependency for a json pointer
    *
    * Given a config json, recursively find the root pointer for the
    * given json pointer
    *
    * @param config unevaluated config json
    * @param pointer json pointer to get the root dependency for
    * 
    *
    * @returns string vector containing arr data
   **/
   std::string getRootDependency(nlohmann::json config, std::string pointer);


  /**
   * @brief raises a C++ exception if NAIF has an error buffered. 
   * 
   * @param reset true if NAIF status errors should be reset 
   */
  bool checkNaifErrors(bool reset=true);


   /**
   * @brief Loads translation kernels (fk, ik, and iaks) associated to mission name.
   * 
   * @param mission mission name of the config file
   */
  nlohmann::json loadTranslationKernels(std::string mission, bool loadFk=true, bool loadIk=true, bool loadIak=true);

  /**
   * @brief Loads PCK kernels associated to mission name.
   *
   * @param kernelType kernelType to search for and load
   * @param mission mission name of the config file
   */
  nlohmann::json loadSelectKernels(std::string kernelType, std::string mission);
}
