

# Struct SpiceQL::KernelCoverage



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**KernelCoverage**](structSpiceQL_1_1KernelCoverage.md)



_Coverage of one CK or SPK, as interval rows._ [More...](#detailed-description)

* `#include <utils.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; int &gt; | [**interval\_body\_codes**](#variable-interval_body_codes)  <br> |
|  std::vector&lt; double &gt; | [**interval\_starts**](#variable-interval_starts)  <br> |
|  std::vector&lt; double &gt; | [**interval\_stops**](#variable-interval_stops)  <br> |
|  double | [**start\_time**](#variable-start_time)   = `0`<br> |
|  double | [**stop\_time**](#variable-stop_time)   = `0`<br> |












































## Detailed Description


The vectors are row-aligned: row i of interval\_starts, interval\_stops and interval\_body\_codes describes one interval. Rows are sorted by start time. start\_time and stop\_time are the kernel's bounds, the same values getKernelStartStopTimes returns. 


    
## Public Attributes Documentation




### variable interval\_body\_codes 

```C++
std::vector<int> SpiceQL::KernelCoverage::interval_body_codes;
```




<hr>



### variable interval\_starts 

```C++
std::vector<double> SpiceQL::KernelCoverage::interval_starts;
```




<hr>



### variable interval\_stops 

```C++
std::vector<double> SpiceQL::KernelCoverage::interval_stops;
```




<hr>



### variable start\_time 

```C++
double SpiceQL::KernelCoverage::start_time;
```




<hr>



### variable stop\_time 

```C++
double SpiceQL::KernelCoverage::stop_time;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/utils.h`

