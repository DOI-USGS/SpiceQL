#pragma once
/**
  * @file
  *
  * Functions that interigate the user's kernel installation to return kernels lists
  * or SPICE information are located here
  *
 **/

#include <vector>
#include <set>
#include <iostream>
#include <nlohmann/json.hpp>

#include "spice_types.h"


namespace SpiceQL {
  /**
    * @brief get the latest kernel in a list
    *
    * Returns the kernels with the latest version string (e.g. the highest v??? or similar
    * sub-string in a kernel path name) in the input list and returns it as a path object.
    * Given multiple different kernels, like de### and mar###, each will be evaluted on there
    * own to return the highest version of each.
    *
    * @param kernels vector of strings, should be a list of kernel paths.
    * @returns std::vector<std::string> vector paths to latest Kernels
   **/
  std::vector<std::string> getLatestKernel(std::vector<std::string> kernels);


   /**
    * @brief returns a JSON object of only the latest version of each kernel type
    *
    * Recursively iterates Kernel groups in the input JSON and gets the kernels
    * with the latest version string (e.g. the highest v??? sub-string in a kernel path name).
    *
    * New JSON is returned.
    *
    * @param kernels A Kernel JSON object
    * @returns A new Kernel JSON object with reduced kernel sets
   **/
  nlohmann::json getLatestKernels(nlohmann::json kernels);


  /**
    * @brief return's kernel values in the form of a vector 
    *
    *  Takes in a kernel key and returns the value associated with that kernel as a vector of string
    *  Note: This function is for when the kernel has more than 1 value associated with it, ie: INS-236800_FOV_REF_VECTOR  = ( 1.0, 0.0, 0.0 )
    * 
    * @param key key - Kernel to get values from 
    * @returns vector of values in the form of a string 
   **/
  std::vector<std::string> getKernelVectorValue(std::string key);


  /**
    * @brief return's kernel value from key
    *
    *  Takes in a kernel key and returns the value associated with that kernel as a string 
    *  Note: this function is for when the kernal has a single value associated with it, ie:   INS-236800_FOV_REF_ANGLE   = ( 5.27 )
    * 
    * @param key key - Kernel to get values from 
    * @returns string of value associated with key
   **/
  std::string getKernelStringValue(std::string key);


  /**
   * @brief Returns all kernels available for a mission
   *
   * Returns a structured json object containing all available kernels for a specified mission
   * along with their dependencies.
   *
   * TODO: Add a "See Also" on json format after the format matures a bit more.
   *
   * @param root root path to search
   * @param conf json conf file
   * @returns list of paths matching ext
  **/
  nlohmann::json listMissionKernels(std::string root,  nlohmann::json conf);


  /**
    * @brief acquire all kernels of a type according to a configuration JSON object
    *
    * Given the root directotry with kernels, a JSON configuration object and a kernel type string (e.g. ck, fk, spk),
    * return a JSON object containing kernels. The kernel config's regular expressions
    * are replaced by a concrete kernel list located in the passed in root.
    *
    * @param root Directory with kernels somewhere in the directory or its subdirectories
    * @param conf JSON config file, usually this is a JSON object read from one of the db files that shipped with the library
    * @param kernelType Some CK kernel type, see KERNEL_TYPES
   **/
  nlohmann::json globKernels(std::string root, nlohmann::json conf, std::string kernelType);

  
  /**
   * @brief Get all the kernels in the json as a vector
   * 
   * Recusively iterates all the kernel keys and flattens them in a vector.
   *
   *
   * @param kernels json object with kernel query results
   * @return vector<string> list of kernels
   */
  std::vector<std::string> getKernelsAsVector(nlohmann::json kernels);


 /**
   * @brief Get all the kernels in the json as a set
   * 
   * Recusively iterates all the kernel keys and flattens them in a vector.
   *
   *
   * @param kernels json object with kernel query results
   * @return set<string> set of kernels
   */
  std::set<std::string> getKernelsAsSet(nlohmann::json kernels);

  }
