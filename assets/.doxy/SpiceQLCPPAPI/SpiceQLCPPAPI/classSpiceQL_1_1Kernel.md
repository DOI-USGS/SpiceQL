

# Class SpiceQL::Kernel



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Kernel**](classSpiceQL_1_1Kernel.md)



_Base_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _class._[More...](#detailed-description)

* `#include <spice_types.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**Quality**](#enum-quality)  <br>_Enumeration representing the different possible kernel qualities._  |
| enum  | [**Type**](#enum-type)  <br>_Enumeration representing the different possible kernel types._  |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::string | [**path**](#variable-path)  <br> |
|  [**Quality**](classSpiceQL_1_1Kernel.md#enum-quality) | [**quality**](#variable-quality)  <br> |
|  [**Type**](classSpiceQL_1_1Kernel.md#enum-type) | [**type**](#variable-type)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Kernel**](#function-kernel) (std::string path) <br>_Instantiate a kernel from path._  |
|   | [**~Kernel**](#function-kernel) () <br>_Construct a new_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _object from another._ |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  std::string | [**getExt**](#function-getext) (std::string type) <br>_Get_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _extension._ |
|  bool | [**isBinary**](#function-isbinary) (std::string type) <br>_Check if kernel type is binary._  |
|  bool | [**isCk**](#function-isck) (std::string type) <br>_Check if kernel type is CK._  |
|  bool | [**isSpk**](#function-isspk) (std::string type) <br>_Check if kernel type is SPK._  |
|  bool | [**isText**](#function-istext) (std::string type) <br>_Check if kernel type is text-based._  |
|  std::vector&lt; [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) &gt; | [**translateQualities**](#function-translatequalities) (std::vector&lt; std::string &gt; qas) <br>_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _quality string to enum._ |
|  std::string | [**translateQuality**](#function-translatequality-12) ([**Quality**](classSpiceQL_1_1Kernel.md#enum-quality) qa) <br>_Switch between_ [_**Quality**_](classSpiceQL_1_1Kernel.md#enum-quality) _enum to string._ |
|  [**Quality**](classSpiceQL_1_1Kernel.md#enum-quality) | [**translateQuality**](#function-translatequality-22) (std::string qa) <br>_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _quality string to enum._ |
|  std::string | [**translateType**](#function-translatetype-12) ([**Type**](classSpiceQL_1_1Kernel.md#enum-type) type) <br>_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type enum to string._ |
|  [**Type**](classSpiceQL_1_1Kernel.md#enum-type) | [**translateType**](#function-translatetype-22) (std::string type) <br>_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type string to enum._ |


























## Detailed Description


This is mostly designed to enable the automatic unloading of kernels. The kernel is furnsh-ed on instantiation and unloaded in the destructor. 


    
## Public Types Documentation




### enum Quality 

_Enumeration representing the different possible kernel qualities._ 
```C++
enum SpiceQL::Kernel::Quality {
    NOQUALITY =0,
    NADIR = 1,
    PREDICTED = 2,
    RECONSTRUCTED = 3,
    SMITHED = 4
};
```




<hr>



### enum Type 

_Enumeration representing the different possible kernel types._ 
```C++
enum SpiceQL::Kernel::Type {
    NA =0,
    CK,
    SPK,
    TSPK,
    LSK,
    MK,
    SCLK,
    IAK,
    IK,
    FK,
    DSK,
    PCK,
    EK
};
```




<hr>
## Public Attributes Documentation




### variable path 

```C++
std::string SpiceQL::Kernel::path;
```



path to the kernel 


        

<hr>



### variable quality 

```C++
Quality SpiceQL::Kernel::quality;
```



quality of the kernel 


        

<hr>



### variable type 

```C++
Type SpiceQL::Kernel::type;
```



type of kernel 


        

<hr>
## Public Functions Documentation




### function Kernel 

_Instantiate a kernel from path._ 
```C++
SpiceQL::Kernel::Kernel (
    std::string path
) 
```



Load a kernel into memory by opening the kernel and furnishing. This also increases the reference count of the kernel. If the kernel has alrady been furnished, it is refurnshed. 





**Parameters:**


* `path` path to a kernel. 




        

<hr>



### function ~Kernel 

_Construct a new_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _object from another._
```C++
SpiceQL::Kernel::~Kernel () 
```



Ensures the reference counter is incremented when a copy of the kernel is created




**Parameters:**


* `other` some other [**Kernel**](classSpiceQL_1_1Kernel.md) instance

Delete the kernel object and decrease it's reference count


Deletes the kernel object and decrements it's reference count. If the reference count hits 0, the kernel is unloaded. 


        

<hr>
## Public Static Functions Documentation




### function getExt 

_Get_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _extension._
```C++
static std::string SpiceQL::Kernel::getExt (
    std::string type
) 
```





**Parameters:**


* `type` [**Kernel**](classSpiceQL_1_1Kernel.md) type string 



**Returns:**

[**Kernel**](classSpiceQL_1_1Kernel.md) extension as string 





        

<hr>



### function isBinary 

_Check if kernel type is binary._ 
```C++
static bool SpiceQL::Kernel::isBinary (
    std::string type
) 
```





**Parameters:**


* `type` [**Kernel**](classSpiceQL_1_1Kernel.md) type string 



**Returns:**

Whether kernel type is binary 





        

<hr>



### function isCk 

_Check if kernel type is CK._ 
```C++
static bool SpiceQL::Kernel::isCk (
    std::string type
) 
```





**Parameters:**


* `type` [**Kernel**](classSpiceQL_1_1Kernel.md) type string 



**Returns:**

Whether kernel type is CK 





        

<hr>



### function isSpk 

_Check if kernel type is SPK._ 
```C++
static bool SpiceQL::Kernel::isSpk (
    std::string type
) 
```





**Parameters:**


* `type` [**Kernel**](classSpiceQL_1_1Kernel.md) type string 



**Returns:**

Whether kernel type is SPK 





        

<hr>



### function isText 

_Check if kernel type is text-based._ 
```C++
static bool SpiceQL::Kernel::isText (
    std::string type
) 
```





**Parameters:**


* `type` [**Kernel**](classSpiceQL_1_1Kernel.md) type string 



**Returns:**

Whether kernel type is text-based 





        

<hr>



### function translateQualities 

_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _quality string to enum._
```C++
static std::vector< Kernel::Quality > SpiceQL::Kernel::translateQualities (
    std::vector< std::string > qas
) 
```





**Parameters:**


* `qas` Vector of [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) strings 



**Returns:**

Vector of [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) values 





        

<hr>



### function translateQuality [1/2]

_Switch between_ [_**Quality**_](classSpiceQL_1_1Kernel.md#enum-quality) _enum to string._
```C++
static std::string SpiceQL::Kernel::translateQuality (
    Quality qa
) 
```





**Parameters:**


* `qa` [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality) to translate to a string 



**Returns:**

String representation of the kernel type, eg. Kernel::Quality::Reconstructed returns "reconstructed" 





        

<hr>



### function translateQuality [2/2]

_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _quality string to enum._
```C++
static Quality SpiceQL::Kernel::translateQuality (
    std::string qa
) 
```





**Parameters:**


* `qa` String to translate to a [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality), must be all lower case 



**Returns:**

[**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type) representation of the kernel type, eg. "reconstructed" returns Kernel::Quality::Reconstructed 





        

<hr>



### function translateType [1/2]

_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type enum to string._
```C++
static std::string SpiceQL::Kernel::translateType (
    Type type
) 
```





**Parameters:**


* `type` [**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type) to translate to a string 



**Returns:**

String representation of the kernel type, eg. Kernel::Type::CK returns "ck" 





        

<hr>



### function translateType [2/2]

_Switch between_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type string to enum._
```C++
static Type SpiceQL::Kernel::translateType (
    std::string type
) 
```





**Parameters:**


* `type` String to translate to a [**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type), must be all lower case 



**Returns:**

[**Kernel::Type**](classSpiceQL_1_1Kernel.md#enum-type) representation of the kernel type, eg. "ck" returns Kernel::Type::CK 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/spice_types.h`

