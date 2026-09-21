

# Struct restincurl::OutDataHandler

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**OutDataHandler**](structrestincurl_1_1OutDataHandler.md)



[More...](#detailed-description)

* `#include <restincurl.h>`



Inherits the following classes: [restincurl::DataHandlerBase](structrestincurl_1_1DataHandlerBase.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  T | [**data\_**](#variable-data_)  <br> |
|  size\_t | [**sendt\_bytes\_**](#variable-sendt_bytes_)   = `0`<br> |
































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**OutDataHandler**](#function-outdatahandler-13) () = default<br> |
|   | [**OutDataHandler**](#function-outdatahandler-23) (const T & v) <br> |
|   | [**OutDataHandler**](#function-outdatahandler-33) (T && v) <br> |


## Public Functions inherited from restincurl::DataHandlerBase

See [restincurl::DataHandlerBase](structrestincurl_1_1DataHandlerBase.md)

| Type | Name |
| ---: | :--- |
| virtual  | [**~DataHandlerBase**](structrestincurl_1_1DataHandlerBase.md#function-datahandlerbase) () = default<br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**read\_callback**](#function-read_callback) (char \* bufptr, size\_t size, size\_t nitems, void \* userdata) <br> |




















































## Detailed Description


Template implementation for output data to curl during a request.


This handler deals with the data sent to the HTTP server during a request (POST, PATCH etc). This implementation will typically use T=std::string and just store the data in a string. For json/XML payloads that's probably all you need. But if you send binary data, you may want to use a container like std::vector or std::deque in stead. 


    
## Public Attributes Documentation




### variable data\_ 

```C++
T restincurl::OutDataHandler< T >::data_;
```




<hr>



### variable sendt\_bytes\_ 

```C++
size_t restincurl::OutDataHandler< T >::sendt_bytes_;
```




<hr>
## Public Functions Documentation




### function OutDataHandler [1/3]

```C++
restincurl::OutDataHandler::OutDataHandler () = default
```




<hr>



### function OutDataHandler [2/3]

```C++
inline restincurl::OutDataHandler::OutDataHandler (
    const T & v
) 
```




<hr>



### function OutDataHandler [3/3]

```C++
inline restincurl::OutDataHandler::OutDataHandler (
    T && v
) 
```




<hr>
## Public Static Functions Documentation




### function read\_callback 

```C++
static inline size_t restincurl::OutDataHandler::read_callback (
    char * bufptr,
    size_t size,
    size_t nitems,
    void * userdata
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

