---

hide:
  - navigation
  - toc
  - title
---


<style>
  /* Material injects a level-1 heading from the site name here, because the
     page opens with the banner instead of one. Matched at the same specificity as
     the heading-block rule in extra.css, and later in the cascade, so the
     injected title stays hidden rather than picking up an accent block. */
  .md-content .md-typeset h1,
  .md-content__button {
    display: none;
  }
</style>

<!-- Add `sq-hero--full` to the class below for a full-screen hero: the banner
     fills the viewport under the header and the prose starts one scroll down.
     See the flag's block in assets/extra.css. -->
<div class="sq-hero sq-hero--full">
  <iframe src="assets/mars-banner.html"
          scrolling="no"
          loading="lazy"
          title="Mars Orbit Visualization — SpiceQL">
  </iframe>
</div>


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
    kernels = psql.searchForKernelsets(["odyssey", "mars"], ["sclk", "spk", "tspk", "ck"], 715662878.32324, 715663065.2303)
    print(kernels)


    # Make a query and it will return the kernels used 
    orientations, kernels = psql.getTargetOrientations([690201375.8323615], -74000, -74690, "ctx", searchKernels=True) 
    print(kernels)
    print(orientations)
    ```

=== "C++"

    ```C++ 
    #include <spiceql/spiceql.h>
    #include <nlohmann/json.hpp>
    
    // search for a kernel set
    nlohmann::json kernels1 = SpiceQL::searchForKernelsets({"odyssey", "mars"}, {"sclk", "spk", "tspk", "ck"}, 715662878.32324, 715663065.2303);
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
    
    `downloadIsisData.py` is a script that downloads kernels from NAIF and USGS sources. It includes a SpiceQL database.
    
    If you have ISIS, you may have already set up the data area.  If you don't have ISIS, you will need to download the `rclone` program, the `downloadIsisData` script, and ISIS's `rclone.conf`.  See [Downloading ISISDATA](https://astrogeology.usgs.gov/docs/how-to-guides/environment-setup-and-maintenance/isis-data-area/#downloading-isisdata) for more info on the download script, or install as below if you don't have ISIS:

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


