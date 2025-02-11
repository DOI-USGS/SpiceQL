#pragma once

/**
  *
  *
 **/

#include <iostream>
#include <unordered_map>

#include <nlohmann/json.hpp>

/**
 * @namespace SpiceQL types
 * 
 */
namespace SpiceQL {
  extern const std::vector<std::string> KERNEL_TYPES;
  extern const std::vector<std::string> KERNEL_QUALITIES;
  
  void load(std::string path, bool force_refurnsh=true); 
  void unload(std::string path);

  /**
   * @brief Base Kernel class
   *
   * This is mostly designed to enable the automatic unloading
   * of kernels. The kernel is furnsh-ed on instantiation and
   * unloaded in the destructor.
   *
   */
  class Kernel {
      public:

      /**
       * @brief Enumeration representing the different possible kernel types
       **/
      enum class Type { NA=0,
        CK, SPK, TSPK,
        LSK, MK, SCLK,
        IAK, IK, FK,
        DSK, PCK, EK
      };

      /**
       * @brief Enumeration representing the different possible kernel qualities
       **/
      enum class Quality  {
        NOQUALITY=0,       // Either Quaility doesn't apply (e.g. text kernels) -or-
                           // we dont care about quality (e.g. CK of any quality)
        NADIR = 1,         // Assumes Nadir pointing
        PREDICTED = 2,     // Based on predicted future location of the spacecraft/body
        RECONSTRUCTED = 3, // Supplemented by real spacecraft/body data
        SMITHED = 4,       // Controlled Kernels
      };

      /**
       * @brief Switch between Kernel type enum to string
       *
       * @param type Kernel::Type to translate to a string
       * @return String representation of the kernel type, eg. Kernel::Type::CK returns "ck"
       **/
      static std::string translateType(Type type);


      /**
       * @brief Switch between Kernel type string to enum
       *
       * @param type String to translate to a Kernel::Type, must be all lower case
       * @return Kernel::Type representation of the kernel type, eg. "ck" returns Kernel::Type::CK
       **/
      static Type translateType(std::string type);


     /**
       * @brief Switch between Quality enum to string
       *
       * @param qa Kernel::Quality to translate to a string
       * @return String representation of the kernel type, eg. Kernel::Quality::Reconstructed  returns "reconstructed"
       **/
      static std::string translateQuality(Quality qa);


      /**
       * @brief Switch between Kernel quality string to enum
       *
       * @param qa String to translate to a Kernel::Quality, must be all lower case
       * @return Kernel::Type representation of the kernel type, eg. "reconstructed" returns Kernel::Quality::Reconstructed
       **/
      static Quality translateQuality(std::string qa);
      

      /**
       * @brief Instantiate a kernel from path
       *
       * Load a kernel into memory by opening the kernel and furnishing.
       * This also increases the reference count of the kernel. If the kernel
       * has alrady been furnished, it is refurnshed.  
       *
       * @param path path to a kernel.
       *
      **/
      Kernel(std::string path);


      /**
       * @brief Construct a new Kernel object from another
       * 
       * Ensures the reference counter is incremented when a copy of
       * the kernel is created
       * 
       * @param other some other Kernel instance
       */
      // Kernel(Kernel &other);


      /**
        * @brief Delete the kernel object and decrease it's reference count
        *
        * Deletes the kernel object and decrements it's reference count. If the reference count hits 0,
        * the kernel is unloaded. 
        *
      **/
      ~Kernel();

      /*! path to the kernel */
      std::string path; 
      /*! type of kernel */
      Type type; 
      /*! quality of the kernel */
      Quality quality;
  };


  /**
   * @brief Class for furnishing kernels in bulk 
   * 
   * Given a json object, furnish every kernel under a 
   * "kernels" key. The kernels are unloaded as soon as the object 
   * goes out of scope. 
   *
   * Generally used on results from a kernel query. 
   */
  class KernelSet {
    public:

    /**
     * @brief Construct a new Kernel Set object
     * 
     * @param kernels 
     */
    KernelSet(nlohmann::json kernels);
    KernelSet() = default;
    ~KernelSet();

    void load(nlohmann::json kernels);

    //! map of path to kernel pointers
    std::vector<Kernel*> m_loadedKernels;
    
    //! json used to populate the loadedKernels
    nlohmann::json m_kernels; 
  };


  /**
   * @brief convert a UTC string to an ephemeris time
   *
   * Basically a wrapper around NAIF's cspice str2et function except it also temporarily loads the required kernels.
   * See Also: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/str2et_c.html
   *
   * @param et UTC string, e.g. "1988 June 13, 12:29:48 TDB"
   * @param searchKernels bool Whether to search the kernels for the user
   * @returns double precision ephemeris time
   **/
  double utcToEt(std::string utc, bool searchKernels = true, std::vector<std::string> kernel_list={});


  /**
   * @brief convert et string to a UTC string
   *
   * Basically a wrapper around NAIF's cspice et2utc_c function except it also temporarily loads the required kernels.
   * See Also: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/et2utc_c.html
   *
   * @param et ephemeris time
   * @param precision number of decimal 
   * @param searchKernels bool Whether to search the kernels for the user
   * @returns double precision ephemeris time
   **/
  std::string etToUtc(double et, std::string format = "C", double precision = 8, bool searchKernels=true, std::vector<std::string> kernel_list={});


  /**
   * @brief Converts a given string spacecraft clock time to an ephemeris time
   *
   * Given a known frame code strSclkToEt converts a given spacecraft clock time as a string
   * to an ephemeris time. Call this function if your clock time looks something like:
   * 1/281199081:48971
   *
   * @param frameCode int Frame id to use
   * @param sclk string Spacecraft Clock formatted as a string
   * @param mission string Mission name as it relates to the config files
   * @param searchKernels bool Whether to search the kernels for the user
   * @return double
   */
  double strSclkToEt(int frameCode, std::string sclk, std::string mission, bool searchKernels=true, std::vector<std::string> kernel_list={});

  /**
   * @brief Converts a given double spacecraft clock time to an ephemeris time
   *
   * Given a known frame code doubleSclkToEt converts a given spacecraft clock time as a double
   * to an ephemeris time. Call this function if your clock time looks something like:
   * 922997380.174174
   *
   * @param frameCode int Frame id to use
   * @param sclk int Spacecraft Clock formatted as an int
   * @param mission string Mission name as it relates to the config files
   * @param searchKernels bool Whether to search the kernels for the user
   * @return double
   */
  double doubleSclkToEt(int frameCode, double sclk, std::string mission, bool searchKernels=true, std::vector<std::string> kernel_list={});


  /**
   * @brief Converts a given double ephemeris time to an sclk string
   *
   *
   * @param frameCode int Frame id to use
   * @param et ephemeris time
   * @param mission string Mission name as it relates to the config files
   * @param searchKernels bool Whether to search the kernels for the user
   * @return string
   */
  std::string doubleEtToSclk(int frameCode, double et, std::string mission, bool searchKernels, std::vector<std::string> kernel_list={});


  /**
   * @brief Get the center, class id, and class of a given frame
   *
   * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
   *
   * @param frame String frame name to translate to a NAIF code
   * @param mission Mission name as it relates to the config files
   * @param searchKernels bool Whether to search the kernels for the user
   * @return 3 element vector of the given frames center, class id, and class
  **/
  std::vector<int> getFrameInfo(int frame, std::string mission, bool searchKernels=true, std::vector<std::string> kernel_list={});

  /**
   * @brief Switch between NAIF frame string name to integer frame code
   *
   * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
   *
   * @param frame String frame name to translate to a NAIF code
   * @param mission Mission name as it relates to the config files
   * @param searchKernels bool Whether to search the kernels for the user
   * @return integer Naif frame code
  **/
  int translateNameToCode(std::string frame, std::string mission="", bool searchKernels=true, std::vector<std::string> kernel_list={});

  /**
   * @brief Switch between NAIF frame integer code to string frame name
   *
   * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
   *
   * @param frame int NAIF frame code to translate
   * @param searchKernels bool Whether to search the kernels for the user
   * @param mission Mission name as it relates to the config files
   * @return string Naif frame name
  **/
  std::string translateCodeToName(int frame, std::string mission="", bool searchKernels=true, std::vector<std::string> kernel_list={});

  /**
    * @brief returns kernel values for a specific mission in the form of a json
    *
    *  Takes in a kernel key and returns the value associated with the inputted mission as a json
    * 
    * @param key key - Kernel to get values from 
    * @param mission mission name
    * @param searchKernels bool Whether to search the kernels for the user
    * @returns json of values
  **/
  nlohmann::json findMissionKeywords(std::string key, std::string mission, bool searchKernels=true, std::vector<std::string> kernel_list={});

  /**
    * @brief returns Target values in the form of a vector
    *
    *  Takes in a target and key and returns the value associated in the form of vector.
    *  Note: This function is mainly for obtaining target keywords. For obtaining other values, use findMissionKeywords.
    * 
    * @param key keyword for desired values
    * @param mission mission name as it relates to the config files
    * @param searchKernels bool Whether to search the kernels for the user
    * @returns vector of values
  **/
  nlohmann::json findTargetKeywords(std::string key, std::string mission, bool searchKernels = true, std::vector<std::string> kernel_list={});

  /**
    * @brief returns frame name and frame code associated to the target ID.
    *
    *  Takes in a target id and returns the frame name and frame code in json format
    * 
    * @param targetId target ID
    * @param mission mission name as it relates to the config files
    * @param searchKernels bool Whether to search the kernels for the user
    * @returns json of frame name and frame code
  **/
  nlohmann::json getTargetFrameInfo(int targetId, std::string mission, bool searchKernels=true, std::vector<std::string> kernel_list={});
}
