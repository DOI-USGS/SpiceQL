

# Class SpiceQL::InventoryImpl



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**InventoryImpl**](classSpiceQL_1_1InventoryImpl.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; int &gt; | [**m\_frame\_codes**](#variable-m_frame_codes)  <br> |
|  std::vector&lt; std::string &gt; | [**m\_frame\_list**](#variable-m_frame_list)  <br> |
|  std::vector&lt; std::string &gt; | [**m\_frame\_names**](#variable-m_frame_names)  <br> |
|  nlohmann::json | [**m\_json\_inventory**](#variable-m_json_inventory)  <br> |
|  std::map&lt; std::string, std::vector&lt; std::string &gt; &gt; | [**m\_nontimedep\_kerns**](#variable-m_nontimedep_kerns)  <br> |
|  [**KernelSet**](classSpiceQL_1_1KernelSet.md) | [**m\_required\_kernels**](#variable-m_required_kernels)  <br> |
|  std::map&lt; std::string, [**TimeIndexedKernels**](classSpiceQL_1_1TimeIndexedKernels.md) \* &gt; | [**m\_timedep\_kerns**](#variable-m_timedep_kerns)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**InventoryImpl**](#function-inventoryimpl) (bool force\_regen=false, std::vector&lt; std::string &gt; mlist={}) <br> |
|  int | [**getFrameCode**](#function-getframecode) (std::string name) <br>_Resolve a frame/body name to its code using the cached map._  |
|  std::vector&lt; std::string &gt; | [**getFrameList**](#function-getframelist) () <br>_Returns the cached list of frame/config names._  |
|  std::string | [**getFrameName**](#function-getframename) (int code) <br>_Resolve a frame/body code to its name using the cached map._  |
|  T | [**getKey**](#function-getkey) (std::string key) <br> |
|  nlohmann::json | [**search\_for\_kernelset**](#function-search_for_kernelset) (std::string spiceql\_name, std::vector&lt; [**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type) &gt; types, double start\_time=-std::numeric\_limits&lt; double &gt;::max(), double stop\_time=std::numeric\_limits&lt; double &gt;::max(), std::vector&lt; [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) &gt; ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, std::vector&lt; [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) &gt; spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, bool full\_kernel\_path=false, int limit\_ck=-1, int limit\_spk=1) <br> |
|  nlohmann::json | [**search\_for\_kernelsets**](#function-search_for_kernelsets) (std::vector&lt; std::string &gt; spiceql\_names, std::vector&lt; [**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type) &gt; types, double start\_time=-std::numeric\_limits&lt; double &gt;::max(), double stop\_time=std::numeric\_limits&lt; double &gt;::max(), std::vector&lt; [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) &gt; ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, std::vector&lt; [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) &gt; spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED}, bool full\_kernel\_path=false, int limit\_ck=-1, int limit\_spk=1, bool overwrite=false) <br> |
|  void | [**write\_database**](#function-write_database) () <br> |




























## Public Attributes Documentation




### variable m\_frame\_codes 

```C++
std::vector<int> SpiceQL::InventoryImpl::m_frame_codes;
```




<hr>



### variable m\_frame\_list 

```C++
std::vector<std::string> SpiceQL::InventoryImpl::m_frame_list;
```




<hr>



### variable m\_frame\_names 

```C++
std::vector<std::string> SpiceQL::InventoryImpl::m_frame_names;
```




<hr>



### variable m\_json\_inventory 

```C++
nlohmann::json SpiceQL::InventoryImpl::m_json_inventory;
```




<hr>



### variable m\_nontimedep\_kerns 

```C++
std::map<std::string, std::vector<std::string> > SpiceQL::InventoryImpl::m_nontimedep_kerns;
```




<hr>



### variable m\_required\_kernels 

```C++
KernelSet SpiceQL::InventoryImpl::m_required_kernels;
```




<hr>



### variable m\_timedep\_kerns 

```C++
std::map<std::string, TimeIndexedKernels*> SpiceQL::InventoryImpl::m_timedep_kerns;
```




<hr>
## Public Functions Documentation




### function InventoryImpl 

```C++
SpiceQL::InventoryImpl::InventoryImpl (
    bool force_regen=false,
    std::vector< std::string > mlist={}
) 
```




<hr>



### function getFrameCode 

_Resolve a frame/body name to its code using the cached map._ 
```C++
int SpiceQL::InventoryImpl::getFrameCode (
    std::string name
) 
```





**Returns:**

the code, or 0 if the name is not in the cache. 





        

<hr>



### function getFrameList 

_Returns the cached list of frame/config names._ 
```C++
std::vector< std::string > SpiceQL::InventoryImpl::getFrameList () 
```



Reads the precomputed list from the DB; falls back to computing it from the live [**Config**](classSpiceQL_1_1Config.md) if the cache is unavailable. 


        

<hr>



### function getFrameName 

_Resolve a frame/body code to its name using the cached map._ 
```C++
std::string SpiceQL::InventoryImpl::getFrameName (
    int code
) 
```





**Returns:**

the name, or "" if the code is not in the cache. 





        

<hr>



### function getKey 

```C++
template<class T>
T SpiceQL::InventoryImpl::getKey (
    std::string key
) 
```




<hr>



### function search\_for\_kernelset 

```C++
nlohmann::json SpiceQL::InventoryImpl::search_for_kernelset (
    std::string spiceql_name,
    std::vector< Kernel::Type > types,
    double start_time=-std::numeric_limits< double >::max(),
    double stop_time=std::numeric_limits< double >::max(),
    std::vector< Kernel::Quality > ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
    std::vector< Kernel::Quality > spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
    bool full_kernel_path=false,
    int limit_ck=-1,
    int limit_spk=1
) 
```




<hr>



### function search\_for\_kernelsets 

```C++
nlohmann::json SpiceQL::InventoryImpl::search_for_kernelsets (
    std::vector< std::string > spiceql_names,
    std::vector< Kernel::Type > types,
    double start_time=-std::numeric_limits< double >::max(),
    double stop_time=std::numeric_limits< double >::max(),
    std::vector< Kernel::Quality > ckQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
    std::vector< Kernel::Quality > spkQualities={Kernel::Quality::SMITHED, Kernel::Quality::RECONSTRUCTED},
    bool full_kernel_path=false,
    int limit_ck=-1,
    int limit_spk=1,
    bool overwrite=false
) 
```




<hr>



### function write\_database 

```C++
void SpiceQL::InventoryImpl::write_database () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/inventoryimpl.h`

