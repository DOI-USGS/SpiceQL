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


## INSTALL SPICEQL

```
mamba install -c conda-forge spiceql
```


This Library provides a C++ interface querying, reading and writing Naif SPICE kernels. Built on the [Naif Toolkit](https://naif.jpl.nasa.gov/naif/toolkit.html).


## C++ LIBRARY

The SpiceQL API is available via Python bindings in the module `pyspiceql`. The bindings are built using SWIG and are on by default. You can disable the bindings in your build by setting `SPICEQL_BUILD_BINDINGS` to `OFF` when configuring your build.


## PYTHON LIBRARY

The SpiceQL API is available via Python bindings in the module `pyspiceql`. The bindings are built using SWIG and are on by default. You can disable the bindings in your build by setting `SPICEQL_BUILD_BINDINGS` to `OFF` when configuring your build.
