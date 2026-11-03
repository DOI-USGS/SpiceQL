

# Struct SpiceQL::QueryOpts



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**QueryOpts**](structSpiceQL_1_1QueryOpts.md)



_Optional parameters shared across_ [_**SpiceQL**_](namespaceSpiceQL.md) _API calls._[More...](#detailed-description)

* `#include <api.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  bool | [**fullKernelPath**](#variable-fullkernelpath)   = `false`<br>_if true returns full kernel paths, default returns relative paths_  |
|  std::vector&lt; std::string &gt; | [**kernelList**](#variable-kernellist)   = `{}`<br>_additional kernels to load_  |
|  int | [**limitCk**](#variable-limitck)   = `-1`<br>_number of cks to limit to, default is -1 to retrieve all_  |
|  int | [**limitSpk**](#variable-limitspk)   = `1`<br>_number of spks to limit to, default is 1 to retrieve only one_  |
|  bool | [**searchKernels**](#variable-searchkernels)   = `true`<br>_whether to search the kernels for the user_  |
|  bool | [**useWeb**](#variable-useweb)   = `false`<br>_whether to use web_ [_**SpiceQL**_](namespaceSpiceQL.md) __ |












































## Detailed Description


Groups the options that most api functions accept so new options can be added in one place instead of to every function signature. Defaults match the defaults of the individual parameters they replace.


Converts to and from nlohmann::json (json j = options; j.get&lt;QueryOpts&gt;()). Keys missing from the json keep their defaults and unrecognized keys are ignored, so a new field only needs to be added here and to the NLOHMANN\_DEFINE\_TYPE call below. 


    
## Public Attributes Documentation




### variable fullKernelPath 

_if true returns full kernel paths, default returns relative paths_ 
```C++
bool SpiceQL::QueryOpts::fullKernelPath;
```




<hr>



### variable kernelList 

_additional kernels to load_ 
```C++
std::vector<std::string> SpiceQL::QueryOpts::kernelList;
```




<hr>



### variable limitCk 

_number of cks to limit to, default is -1 to retrieve all_ 
```C++
int SpiceQL::QueryOpts::limitCk;
```




<hr>



### variable limitSpk 

_number of spks to limit to, default is 1 to retrieve only one_ 
```C++
int SpiceQL::QueryOpts::limitSpk;
```




<hr>



### variable searchKernels 

_whether to search the kernels for the user_ 
```C++
bool SpiceQL::QueryOpts::searchKernels;
```




<hr>



### variable useWeb 

_whether to use web_ [_**SpiceQL**_](namespaceSpiceQL.md) __
```C++
bool SpiceQL::QueryOpts::useWeb;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/api.h`

