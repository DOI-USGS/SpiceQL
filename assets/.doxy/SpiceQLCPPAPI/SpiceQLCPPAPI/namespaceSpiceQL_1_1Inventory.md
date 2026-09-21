

# Namespace SpiceQL::Inventory



[**Namespace List**](namespaces.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Inventory**](namespaceSpiceQL_1_1Inventory.md)










































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**create\_database**](#function-create_database) (std::vector&lt; std::string &gt; mlist={}) <br> |
|  std::string | [**getDbFilePath**](#function-getdbfilepath) () <br> |
|  int | [**getFrameCodeFromCache**](#function-getframecodefromcache) (std::string name) <br>_Resolve a frame/body name to its code using the cached map._  |
|  std::vector&lt; std::string &gt; | [**getFrameList**](#function-getframelist) () <br>_Get the cached list of frame/config names from the database._  |
|  std::string | [**getFrameNameFromCache**](#function-getframenamefromcache) (int code) <br>_Resolve a frame/body code to its name using the cached map._  |
|  nlohmann::json | [**search\_for\_kernelset**](#function-search_for_kernelset) (std::string spiceql\_name, std::vector&lt; std::string &gt; types=KERNEL\_TYPES, double start\_time=-std::numeric\_limits&lt; double &gt;::max(), double stop\_time=std::numeric\_limits&lt; double &gt;::max(), std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool full\_kernel\_path=false, int limit\_ck=-1, int limit\_spk=1) <br> |
|  nlohmann::json | [**search\_for\_kernelset\_from\_regex**](#function-search_for_kernelset_from_regex) (std::vector&lt; std::string &gt; list, bool full\_kernel\_path=false) <br> |
|  nlohmann::json | [**search\_for\_kernelsets**](#function-search_for_kernelsets) (std::vector&lt; std::string &gt; spiceql\_names, std::vector&lt; std::string &gt; types=KERNEL\_TYPES, double start\_time=-std::numeric\_limits&lt; double &gt;::max(), double stop\_time=std::numeric\_limits&lt; double &gt;::max(), std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool full\_kernel\_path=false, int limit\_ck=-1, int limit\_spk=1, bool overwrite=false) <br> |
|  void | [**setDbFilePath**](#function-setdbfilepath) (std::string db\_file\_path, bool override=false) <br> |




























## Public Functions Documentation




### function create\_database 

```C++
void SpiceQL::Inventory::create_database (
    std::vector< std::string > mlist={}
) 
```




<hr>



### function getDbFilePath 

```C++
std::string SpiceQL::Inventory::getDbFilePath () 
```




<hr>



### function getFrameCodeFromCache 

_Resolve a frame/body name to its code using the cached map._ 
```C++
int SpiceQL::Inventory::getFrameCodeFromCache (
    std::string name
) 
```





**Parameters:**


* `name` frame/body name 



**Returns:**

the code, or 0 if not in the cache 





        

<hr>



### function getFrameList 

_Get the cached list of frame/config names from the database._ 
```C++
std::vector< std::string > SpiceQL::Inventory::getFrameList () 
```





**Returns:**

std::vector&lt;std::string&gt; list of frame names 





        

<hr>



### function getFrameNameFromCache 

_Resolve a frame/body code to its name using the cached map._ 
```C++
std::string SpiceQL::Inventory::getFrameNameFromCache (
    int code
) 
```



Uses the precomputed code&lt;-&gt;name map built during create\_database, so no FKs need to be furnished at runtime.




**Parameters:**


* `code` NAIF frame/body code 



**Returns:**

the name, or "" if not in the cache 





        

<hr>



### function search\_for\_kernelset 

```C++
nlohmann::json SpiceQL::Inventory::search_for_kernelset (
    std::string spiceql_name,
    std::vector< std::string > types=KERNEL_TYPES,
    double start_time=-std::numeric_limits< double >::max(),
    double stop_time=std::numeric_limits< double >::max(),
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool full_kernel_path=false,
    int limit_ck=-1,
    int limit_spk=1
) 
```




<hr>



### function search\_for\_kernelset\_from\_regex 

```C++
nlohmann::json SpiceQL::Inventory::search_for_kernelset_from_regex (
    std::vector< std::string > list,
    bool full_kernel_path=false
) 
```




<hr>



### function search\_for\_kernelsets 

```C++
nlohmann::json SpiceQL::Inventory::search_for_kernelsets (
    std::vector< std::string > spiceql_names,
    std::vector< std::string > types=KERNEL_TYPES,
    double start_time=-std::numeric_limits< double >::max(),
    double stop_time=std::numeric_limits< double >::max(),
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool full_kernel_path=false,
    int limit_ck=-1,
    int limit_spk=1,
    bool overwrite=false
) 
```




<hr>



### function setDbFilePath 

```C++
void SpiceQL::Inventory::setDbFilePath (
    std::string db_file_path,
    bool override=false
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/inventory.h`

