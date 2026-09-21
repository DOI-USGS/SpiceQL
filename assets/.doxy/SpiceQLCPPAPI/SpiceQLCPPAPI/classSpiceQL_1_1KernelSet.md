

# Class SpiceQL::KernelSet



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**KernelSet**](classSpiceQL_1_1KernelSet.md)



_Class for furnishing kernels in bulk._ [More...](#detailed-description)

* `#include <spice_types.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  nlohmann::json | [**m\_kernels**](#variable-m_kernels)  <br>_json used to populate the loadedKernels_  |
|  std::vector&lt; [**Kernel**](classSpiceQL_1_1Kernel.md) \* &gt; | [**m\_loadedKernels**](#variable-m_loadedkernels)  <br>_map of path to kernel pointers_  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**KernelSet**](#function-kernelset-12) (nlohmann::json kernels) <br>_Construct a new_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _Set object._ |
|   | [**KernelSet**](#function-kernelset-22) () = default<br> |
|  void | [**load**](#function-load) (nlohmann::json kernels) <br> |
|  void | [**unload**](#function-unload) () <br> |
|   | [**~KernelSet**](#function-kernelset) () <br> |




























## Detailed Description


Given a json object, furnish every kernel under a "kernels" key. The kernels are unloaded as soon as the object goes out of scope.


Generally used on results from a kernel query. 


    
## Public Attributes Documentation




### variable m\_kernels 

_json used to populate the loadedKernels_ 
```C++
nlohmann::json SpiceQL::KernelSet::m_kernels;
```




<hr>



### variable m\_loadedKernels 

_map of path to kernel pointers_ 
```C++
std::vector<Kernel*> SpiceQL::KernelSet::m_loadedKernels;
```




<hr>
## Public Functions Documentation




### function KernelSet [1/2]

_Construct a new_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _Set object._
```C++
SpiceQL::KernelSet::KernelSet (
    nlohmann::json kernels
) 
```





**Parameters:**


* `kernels` 




        

<hr>



### function KernelSet [2/2]

```C++
SpiceQL::KernelSet::KernelSet () = default
```




<hr>



### function load 

```C++
void SpiceQL::KernelSet::load (
    nlohmann::json kernels
) 
```




<hr>



### function unload 

```C++
void SpiceQL::KernelSet::unload () 
```




<hr>



### function ~KernelSet 

```C++
SpiceQL::KernelSet::~KernelSet () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/spice_types.h`

