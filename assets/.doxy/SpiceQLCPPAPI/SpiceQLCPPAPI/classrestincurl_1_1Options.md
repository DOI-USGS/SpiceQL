

# Class restincurl::Options



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Options**](classrestincurl_1_1Options.md)



[More...](#detailed-description)

* `#include <restincurl.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Options**](#function-options) ([**EasyHandle**](classrestincurl_1_1EasyHandle.md) & eh) <br> |
|  Options & | [**Set**](#function-set-12) (const CURLoption & opt, const T & value) <br> |
|  Options & | [**Set**](#function-set-22) (const CURLoption & opt, const std::string & value) <br> |




























## Detailed Description


Curl option wrapper class


This is just a thin C++ wrapper over Curl's `curl_easy_setopt()` method. 


    
## Public Functions Documentation




### function Options 

```C++
inline restincurl::Options::Options (
    EasyHandle & eh
) 
```




<hr>



### function Set [1/2]

```C++
template<typename T>
inline Options & restincurl::Options::Set (
    const CURLoption & opt,
    const T & value
) 
```



Set an option




**Parameters:**


* `opt` CURLoption enum to change 
* `value` Value to set 




        

<hr>



### function Set [2/2]

```C++
inline Options & restincurl::Options::Set (
    const CURLoption & opt,
    const std::string & value
) 
```



Set an option




**Parameters:**


* `opt` CURLoption enum to change 
* `value` String value to set 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

