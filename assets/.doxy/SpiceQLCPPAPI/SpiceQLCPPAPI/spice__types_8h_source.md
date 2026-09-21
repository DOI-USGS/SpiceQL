

# File spice\_types.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**spice\_types.h**](spice__types_8h.md)

[Go to the documentation of this file](spice__types_8h.md)


```C++
#pragma once


#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace SpiceQL {
  extern const std::vector<std::string> KERNEL_TYPES;
  extern const std::vector<std::string> KERNEL_QUALITIES;
  
  void load(std::string path, bool force_refurnsh=true);
  void unload(std::string path);

  std::vector<std::string> getLoadedKernels();

  bool isLskLoaded();

  class Kernel {
      public:

      enum class Type { NA=0,
        CK, SPK, TSPK,
        LSK, MK, SCLK,
        IAK, IK, FK,
        DSK, PCK, EK
      };

      enum class Quality  {
        NOQUALITY=0,       // Either Quaility doesn't apply (e.g. text kernels) -or-
                           // we dont care about quality (e.g. CK of any quality)
        NADIR = 1,         // Assumes Nadir pointing
        PREDICTED = 2,     // Based on predicted future location of the spacecraft/body
        RECONSTRUCTED = 3, // Supplemented by real spacecraft/body data
        SMITHED = 4,       // Controlled Kernels
      };

      static std::string translateType(Type type);


      static Type translateType(std::string type);

      static std::string getExt(std::string type);

      static bool isBinary(std::string type);

      static bool isText(std::string type);

      static bool isCk(std::string type);

      static bool isSpk(std::string type);

      static std::string translateQuality(Quality qa);


      static Quality translateQuality(std::string qa);

      static std::vector<Kernel::Quality> translateQualities(std::vector<std::string> qas);
      

      Kernel(std::string path);


      // Kernel(Kernel &other);


      ~Kernel();

      std::string path; 
      Type type; 
      Quality quality;
  };


  class KernelSet {
    public:

    KernelSet(nlohmann::json kernels);
    KernelSet() = default;
    ~KernelSet();

    void load(nlohmann::json kernels);
    void unload();
    
    std::vector<Kernel*> m_loadedKernels;
    
    nlohmann::json m_kernels; 
  };
}
```


