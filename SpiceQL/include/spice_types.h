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
       * @brief Switch between Kernel quality string to enum
       *
       * @param qas Vector of Kernel::Quality strings
       * @return Vector of Kernel::Quality values
       **/
      static std::vector<Kernel::Quality> translateQualities(std::vector<std::string> qas);
      

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
}
