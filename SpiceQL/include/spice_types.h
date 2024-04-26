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
        NADIR = 1,         // Assumes Nadir pointing
        PREDICTED = 2,     // Based on predicted future location of the spacecraft/body
        RECONSTRUCTED = 3, // Supplemented by real spacecraft/body data
        SMITHED = 4,       // Controlled Kernels
        NA = SMITHED       // Either Quaility doesn't apply (e.g. text kernels) -or-
                           // we dont care about quality (e.g. CK of any quality)
      };

      /**
       * used for converting string and int kernal quality
       */
      static const std::vector<std::string> QUALITIES;
      
      /**
       * used for converting between string and int kernal types
       */
      static const std::vector<std::string> TYPES;


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
      Kernel(Kernel &other);


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
   * @brief typedef of std::shared_ptr<Kernel>
   *
   * This basically allows the Kernel to exist as a reference counted
   * variable. Once all references to the Kernel cease to exist, the kernel
   * is unloaded.
   */
  typedef std::shared_ptr<Kernel> SharedKernel;


  /**
   * @brief typedef of std::unique_ptr<Kernel>
   *
   * This basically allows the Kernel to exist only within the
   * call stack it is used in.
   *
   */
  typedef std::unique_ptr<Kernel> StackKernel;


  /**
   * @brief Singleton class for interacting with the cspice kernel pool 
   * 
   * Contains functions required to load and unload kernels and 
   * keep track of furnished kernels. 
   */
  class KernelPool {
    public:
    
    /**
     * Delete constructors and such as this is a singleton
     */
    KernelPool(KernelPool const &other) = delete;
    void operator=(KernelPool const &other) = delete;

    /**
     * @brief Get the Ref Map object
     * 
     * @return KernelRefMap& 
     */
    static KernelPool &getInstance();
    
    
    /**
     * @brief get a kernel's reference count
     * 
     * Everytime KernelPool::load is called, the reference count is increased by one. 
     * This returns the number of Kernel objects currently referencing the 
     * input kernel.  
     *
     * @param key key for the kernel to get the ref count for, usually the complete file path
     * @return unsigned int The number of references to the input kernel. If key doesn't exist, this is 0. 
     */
    unsigned int getRefCount(std::string key);


    /**
     * @brief get reference counts for all kernels in the pool 
     * 
     * Everytime KernelPool::load is called, the reference count is increased by one. 
     * This returns the number of Kernel objects referencing every Kernel in the pool.
     *
     * @return std::map<std::string, int> Map of kernel path to reference count.
     */
    std::unordered_map<std::string, int> getRefCounts();


    /**
     * @brief Get the list of Loaded Kernels. 
     * 
     * @return std::vector<std::string> list of loaded kernels.
     */
    std::vector<std::string> getLoadedKernels(); 


    /**
     * @brief load kernel into the kernel pool 
     * 
     * This should be called for furnshing kernel instead of furnsh_c directly 
     * so that they are tracked throughout the process. 
     *
     * @param kernelPath Path to the kernel to load 
     * @param force_refurnsh If true, call furnsh on the kernel even if the kernel is already in the pool. Default is True. 
     */
    int load(std::string kernelPath, bool force_refurnsh=true);


    /**
     * @brief reduce the reference count for a kernel 
     * 
     * This reduces the ref count by one, and if the ref count hits 0, 
     * the kernel is unfurnished. Use this instead of calling unload_c 
     * directly as you cause errors from desyncs. 
     * 
     * @param kernelPath path to the kernel
     */
    int unload(std::string kernelPath);    


    /**
     * @brief load SCLKs 
     * 
     * Any SCLKs in the data area are furnished. 
     */
    void loadClockKernels();

    private: 

    /**
     * @brief load leapsecond kernels
     * 
     * Load the LSK distributed with SpiceQL
     *
     */
    void loadLeapSecondKernel();


    //! Default constructor, default implentation. Singletons shouldn't be constructed from anywhere
    //! other than the getInstance() function.
    KernelPool();
    ~KernelPool() = default;
    
    //! map for tracking what kernels have been furnished and how often. 
    std::unordered_map<std::string, int> refCounts;

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
    ~KernelSet() = default;

    void load(nlohmann::json kernels);

    //! map of path to kernel pointers
    std::vector<SharedKernel> loadedKernels;
    
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
  double utcToEt(std::string utc, bool searchKernels = true);


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
  std::string etToUtc(double et, std::string format = "C", double precision = 8, bool searchKernels=true);


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
  double strSclkToEt(int frameCode, std::string sclk, std::string mission, bool searchKernels=true);

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
  double doubleSclkToEt(int frameCode, double sclk, std::string mission, bool searchKernels=true);

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
  std::vector<int> getFrameInfo(int frame, std::string mission, bool searchKernels=true);

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
  int translateNameToCode(std::string frame, std::string mission="", bool searchKernels=true);

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
  std::string translateCodeToName(int frame, std::string mission="", bool searchKernels=true);

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
  nlohmann::json findMissionKeywords(std::string key, std::string mission, bool searchKernels=true);

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
  nlohmann::json findTargetKeywords(std::string key, std::string mission, bool searchKernels = true);

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
  nlohmann::json getTargetFrameInfo(int targetId, std::string mission, bool searchKernels=true);
}
