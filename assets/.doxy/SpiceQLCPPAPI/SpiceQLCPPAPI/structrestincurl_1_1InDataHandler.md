

# Struct restincurl::InDataHandler

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**InDataHandler**](structrestincurl_1_1InDataHandler.md)



[More...](#detailed-description)

* `#include <restincurl.h>`



Inherits the following classes: [restincurl::DataHandlerBase](structrestincurl_1_1DataHandlerBase.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  T & | [**data\_**](#variable-data_)  <br> |
































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**InDataHandler**](#function-indatahandler) (T & data) <br> |


## Public Functions inherited from restincurl::DataHandlerBase

See [restincurl::DataHandlerBase](structrestincurl_1_1DataHandlerBase.md)

| Type | Name |
| ---: | :--- |
| virtual  | [**~DataHandlerBase**](structrestincurl_1_1DataHandlerBase.md#function-datahandlerbase) () = default<br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**write\_callback**](#function-write_callback) (char \* ptr, size\_t size, size\_t nitems, void \* userdata) <br> |




















































## Detailed Description


Template implementation for input data to curl during a request.


This handler deals with the data received from the HTTP server during a request. This implementation will typically use T=std::string and just store the received data in a string. For json/XML payloads that's probably all you need. But if you receive binary data, you may want to use a container like std::vector or std::deque in stead. 


    
## Public Attributes Documentation




### variable data\_ 

```C++
T& restincurl::InDataHandler< T >::data_;
```




<hr>
## Public Functions Documentation




### function InDataHandler 

```C++
inline restincurl::InDataHandler::InDataHandler (
    T & data
) 
```




<hr>
## Public Static Functions Documentation




### function write\_callback 

```C++
static inline size_t restincurl::InDataHandler::write_callback (
    char * ptr,
    size_t size,
    size_t nitems,
    void * userdata
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

