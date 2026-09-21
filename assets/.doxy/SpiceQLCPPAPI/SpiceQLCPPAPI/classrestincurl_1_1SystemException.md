

# Class restincurl::SystemException



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**SystemException**](classrestincurl_1_1SystemException.md)



[More...](#detailed-description)

* `#include <restincurl.h>`



Inherits the following classes: [restincurl::Exception](classrestincurl_1_1Exception.md)






















































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**SystemException**](#function-systemexception) (const std::string & msg, const int e) <br> |
|  int | [**getErrorCode**](#function-geterrorcode) () noexcept const<br> |


## Public Functions inherited from restincurl::Exception

See [restincurl::Exception](classrestincurl_1_1Exception.md)

| Type | Name |
| ---: | :--- |
|   | [**Exception**](classrestincurl_1_1Exception.md#function-exception) (const std::string & msg) <br> |






















































## Detailed Description


[**Exception**](classrestincurl_1_1Exception.md) thrown when some system function, like `pipe()` failed. 


    
## Public Functions Documentation




### function SystemException 

```C++
inline restincurl::SystemException::SystemException (
    const std::string & msg,
    const int e
) 
```




<hr>



### function getErrorCode 

```C++
inline int restincurl::SystemException::getErrorCode () noexcept const
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

