---

hide:
  - navigation
  - toc
  - title
---

<style>
  .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

![banner](assets/banner3.png)


This Library provides a C++ interface querying, reading and writing Naif SPICE kernels. Built on the [Naif Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html).


## Getting Started

We reccomend installing using [miniforge](https://github.com/conda-forge/miniforge) 

```
mamba install -c conda-forge spiceql
```

To create the inital database, set your required environment variables. 

```bash 
# Path to some directory with kernels. SpiceQL searches for them recusively
export SPICEROOT="path/to/spice/kernels/"
# anywhere you have write permissions to
export SPICEQL_CACHE_DIR="path/to/cache/"
```

Run `create_database()`, this is more easily done through python. 

!!! warning 
    This might take serveral hours depending on the number of kernels in your folder

```bash 
SPICEQL_LOG_LEVEL=INFO python -c "import pyspiceql; pyspiceql.create_database()"
```

### Basic usage

=== "Python"

    ```python 
    import pyspiceql as psql 

    # search for a kernel set
    kernels = psql.search_for_kernelsets(["odyssey", "mars"], ["sclk", "spk", "tspk", "ck"], 715662878.32324, 715663065.2303)
    print(kernels)


    # Make a query and it will return the kernels used 
    orientations, kernels = spql.getTargetOrientations([690201375.8323615], -74000, -74690, "ctx", searchKernels=True) 
    print(kernels)
    print(orientations)
    ```

=== "C++"

    ```C++ 
    #include <spiceql/spiceql.h>
    #include <spiceql/inventory.h>
    #include <nlohmann/json.hpp>
    
    // search for a kernel set
    nlohmann::json kernels1 = SpiceQL::Inventory::search_for_kernelsets({"odyssey", "mars"}, {"sclk", "spk", "tspk", "ck"}, 715662878.32324, 715663065.2303);
    std::cout << kernels1.dump(2) << std::endl;
    
    // Make a query and it will return the kernels used 
    auto [orientations, kernels2] = SpiceQL::getTargetOrientations({690201375.8323615}, -74000, -74690, "ctx", {"smithed", "reconstructed"}, false, true);
    std::cout << kernels2.dump(2) << std::endl;
    // nx4 aray of quaternions 
    std::cout << orientations.size() << std::endl;
    ```

### Online Interface 

Some functions allow for running over the web, these contain the optional parameter `useWeb`. See the [function list](SpiceQLCPPAPI/namespace_spice_q_l.md) for a list of functions with this parameter. 

=== "Python"

    ```python 
    import pyspiceql as psql 

    # Make a query and it will return the kernels used 
    orientations, kernels = spql.getTargetOrientations([690201375.8323615], -74000, -74690, "ctx", useWeb=True) 
    print(kernels)
    print(orientations)
    ```

=== "C++"

    ```C++ 
    #include <spiceql/spiceql.h>
    #include <nlohmann/json.hpp>
    
    // Make a query and it will return the kernels used 
    auto [orientations, kernels] = SpiceQL::getTargetOrientations({690201375.8323615}, -74000, -74690, "ctx", {"smithed", "reconstructed"}, true);
    std::cout << kernels.dump(2) << std::endl;
    // nx4 aray of quaternions 
    std::cout << orientations.size() << std::endl;
    ```

## Where to get NAIF kernels

=== "downloadIsisData.py"
    
    `downloadIsisData.py` is a script that downloads from NAIF and USGS sources in parallel. It includes a SpiceQL database. 

    ```bash
    # Install rclone 
    mamba install rclone
    
    # Download the script and rclone config file
    curl -LJO https://github.com/USGS-Astrogeology/ISIS3/raw/dev/isis/scripts/downloadIsisData
    curl -LJO https://github.com/USGS-Astrogeology/ISIS3/raw/dev/isis/config/rclone.conf
    
    # Use python 3 when you run the script,
    # and use --config to point to where you downloaded the config file 
    python3 downloadIsisData --config rclone.conf <mission> $SPICEROOT
    # set your cache dir to the same as SPICEROOT
    export SPICEQL_CACHE_DIR=$SPICEROOT
    ```

=== "Direct from NAIF" 

    Download data from [NAIF](https://naif.jpl.nasa.gov/naif/data_archived.html) 

    You can download using `wget` or similar application. 


