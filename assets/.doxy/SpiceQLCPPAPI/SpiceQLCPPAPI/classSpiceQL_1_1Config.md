

# Class SpiceQL::Config



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Config**](classSpiceQL_1_1Config.md)



_Object for interacting with_ [_**SpiceQL**_](namespaceSpiceQL.md) _target configs._[More...](#detailed-description)

* `#include <config.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Config**](#function-config-13) () <br>_Construct a new_ [_**Config**_](classSpiceQL_1_1Config.md) _object._ |
|   | [**Config**](#function-config-23) (std::string json) <br>_Construct a new_ [_**Config**_](classSpiceQL_1_1Config.md) _object from json file._ |
|  bool | [**contains**](#function-contains) (std::string key) <br>_Check if the given key is in the config._  |
|  std::vector&lt; std::string &gt; | [**findKey**](#function-findkey) (std::string key, bool recursive) <br>_Search for a key inside the config._  |
|  nlohmann::json | [**get**](#function-get-12) (std::string pointer="") <br>_get kernel lists from key_  |
|  nlohmann::json | [**get**](#function-get-22) (std::vector&lt; std::string &gt; pointers) <br>_get kernel lists from key vector_  |
|  nlohmann::json | [**getLatest**](#function-getlatest) (std::string pointer="") <br>_get kernel lists from key but only returns the latest version of each kernel._  |
|  nlohmann::json | [**getLatestRecursive**](#function-getlatestrecursive) (std::string pointer="") <br>_get kernel list from all instances of a key but only returns the latest version of each kernel._  |
|  std::string | [**getParentPointer**](#function-getparentpointer) (std::string searchPointer, int pointerPosition=0) <br>_Get the pointer at a position of a pointer._  |
|  nlohmann::json | [**getRecursive**](#function-getrecursive) (std::string pointer) <br>_get kernel list from all instances of a key_  |
|  nlohmann::json | [**globalConf**](#function-globalconf) () <br>_Get the Global Json object._  |
|  [**Config**](classSpiceQL_1_1Config.md#function-config-13) | [**operator[]**](#function-operator) (std::string pointer) <br>_get a value from the config object_  |
|  [**Config**](classSpiceQL_1_1Config.md#function-config-13) | [**operator[]**](#function-operator_1) (std::vector&lt; std::string &gt; pointers) <br> |
|  unsigned int | [**size**](#function-size) () <br> |




























## Detailed Description


The config class can wrap multiple config files and give an interface for interacting with the configs and obtaining kernel lists. 


    
## Public Functions Documentation




### function Config [1/3]

_Construct a new_ [_**Config**_](classSpiceQL_1_1Config.md) _object._
```C++
SpiceQL::Config::Config () 
```



Loads all config files into a config object. 


        

<hr>



### function Config [2/3]

_Construct a new_ [_**Config**_](classSpiceQL_1_1Config.md) _object from json file._
```C++
SpiceQL::Config::Config (
    std::string json
) 
```





**Parameters:**


* `json` path to json config file 




        

<hr>



### function contains 

_Check if the given key is in the config._ 
```C++
bool SpiceQL::Config::contains (
    std::string key
) 
```



Wrapper around nlohmann::json contains to see if the underlying config json contains the given key




**Parameters:**


* `key` a key or json style pointer 



**Returns:**

boolean true if key was found, otherwise false 





        

<hr>



### function findKey 

_Search for a key inside the config._ 
```C++
std::vector< std::string > SpiceQL::Config::findKey (
    std::string key,
    bool recursive
) 
```





**Parameters:**


* `key` a key or json style pointer 
* `recursive` if true, searches recursively for all matches 



**Returns:**

std::vector&lt;nlohmann::json::json\_pointer&gt; vector of pointers to where the key was found 





        

<hr>



### function get [1/2]

_get kernel lists from key_ 
```C++
nlohmann::json SpiceQL::Config::get (
    std::string pointer=""
) 
```



Returns a sub group of the JSON where regexes are expanded into file lists.




**Parameters:**


* `pointers` key or JSON pointer to subgroup on 



**Returns:**

nlohmann::json merged JSON 





        

<hr>



### function get [2/2]

_get kernel lists from key vector_ 
```C++
nlohmann::json SpiceQL::Config::get (
    std::vector< std::string > pointers
) 
```



Returns a sub group of the JSON where regexes are expanded into file lists. For each key in the vector, the resulting JSON is merged into a single json object.




**Parameters:**


* `pointers` list of keys or JSON pointer to subgroup on 



**Returns:**

nlohmann::json merged JSON 





        

<hr>



### function getLatest 

_get kernel lists from key but only returns the latest version of each kernel._ 
```C++
nlohmann::json SpiceQL::Config::getLatest (
    std::string pointer=""
) 
```



Returns a sub group of the JSON where regexes are expanded into file lists containing only the latest kernel versions. For example, insted of a list of all IK kernels you would get the IK with the latest v??? string or similar according to how a specific set of kernels are versioned.




**Parameters:**


* `pointers` key or JSON pointer to subgroup on 



**Returns:**

nlohmann::json new JSON subgroup with latest kernels 





        

<hr>



### function getLatestRecursive 

_get kernel list from all instances of a key but only returns the latest version of each kernel._ 
```C++
nlohmann::json SpiceQL::Config::getLatestRecursive (
    std::string pointer=""
) 
```



Searches recursively for all instances of the input key and returns a new merged JSON object with values of the JSON where regexes are expanded into file lists containing only the latest kernel versions. For example, insted of a list of all IK kernels you would get the IK with the latest v??? string or similar according to how a specific set of kernels are versioned.




**Parameters:**


* `pointer` key or JSON pointer to recursively subgroup on 



**Returns:**

nlohmann::json json subgroup with kernels 





        

<hr>



### function getParentPointer 

_Get the pointer at a position of a pointer._ 
```C++
std::string SpiceQL::Config::getParentPointer (
    std::string searchPointer,
    int pointerPosition=0
) 
```





**Parameters:**


* `searchPointer` Pointer/path to be searched 
* `pointerPosition` Position of the pointer to extract 



**Returns:**

std::string pointer at the position in the searchPointer 





        

<hr>



### function getRecursive 

_get kernel list from all instances of a key_ 
```C++
nlohmann::json SpiceQL::Config::getRecursive (
    std::string pointer
) 
```



Searches recursively for all instances of the input key and returns a new merged JSON object with values of the JSON where regexes are expanded into file lists.




**Parameters:**


* `pointer` key or JSON pointer to recursively subgroup on 



**Returns:**

nlohmann::json json subgroup with kernels 





        

<hr>



### function globalConf 

_Get the Global Json object._ 
```C++
nlohmann::json SpiceQL::Config::globalConf () 
```



The global JSON object represents the current [**SpiceQL**](namespaceSpiceQL.md) config for all frames. 





**Returns:**

nlohmann::json the config file 





        

<hr>



### function operator[] 

_get a value from the config object_ 
```C++
Config SpiceQL::Config::operator[] (
    std::string pointer
) 
```





**Parameters:**


* `pointer` json pointer to a key inside the config file 



**Returns:**

[**Config**](classSpiceQL_1_1Config.md) Returns a new config instance representing the sub object 





        

<hr>



### function operator[] 

```C++
Config SpiceQL::Config::operator[] (
    std::vector< std::string > pointers
) 
```




<hr>



### function size 

```C++
unsigned int SpiceQL::Config::size () 
```





**Returns:**

unsigned int 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/config.h`

