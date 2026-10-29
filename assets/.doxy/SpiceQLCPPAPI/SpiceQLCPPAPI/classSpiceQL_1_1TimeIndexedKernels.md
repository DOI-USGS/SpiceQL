

# Class SpiceQL::TimeIndexedKernels



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**TimeIndexedKernels**](classSpiceQL_1_1TimeIndexedKernels.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; std::string &gt; | [**file\_paths**](#variable-file_paths)  <br> |
|  std::vector&lt; int &gt; | [**interval\_body\_codes**](#variable-interval_body_codes)  <br> |
|  std::vector&lt; size\_t &gt; | [**interval\_kindex**](#variable-interval_kindex)  <br> |
|  std::vector&lt; double &gt; | [**interval\_starts**](#variable-interval_starts)  <br> |
|  std::vector&lt; double &gt; | [**interval\_stops**](#variable-interval_stops)  <br> |
|  frozenca::BTreeMap&lt; double, size\_t &gt; | [**start\_times**](#variable-start_times)  <br> |
|  frozenca::BTreeMap&lt; double, size\_t &gt; | [**stop\_times**](#variable-stop_times)  <br> |












































## Public Attributes Documentation




### variable file\_paths 

```C++
std::vector<std::string> SpiceQL::TimeIndexedKernels::file_paths;
```




<hr>



### variable interval\_body\_codes 

```C++
std::vector<int> SpiceQL::TimeIndexedKernels::interval_body_codes;
```




<hr>



### variable interval\_kindex 

```C++
std::vector<size_t> SpiceQL::TimeIndexedKernels::interval_kindex;
```




<hr>



### variable interval\_starts 

```C++
std::vector<double> SpiceQL::TimeIndexedKernels::interval_starts;
```




<hr>



### variable interval\_stops 

```C++
std::vector<double> SpiceQL::TimeIndexedKernels::interval_stops;
```




<hr>



### variable start\_times 

```C++
frozenca::BTreeMap<double, size_t> SpiceQL::TimeIndexedKernels::start_times;
```




<hr>



### variable stop\_times 

```C++
frozenca::BTreeMap<double, size_t> SpiceQL::TimeIndexedKernels::stop_times;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/inventoryimpl.h`

