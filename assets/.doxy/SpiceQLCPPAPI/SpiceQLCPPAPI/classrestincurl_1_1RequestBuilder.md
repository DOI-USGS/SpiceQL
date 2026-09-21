

# Class restincurl::RequestBuilder



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**RequestBuilder**](classrestincurl_1_1RequestBuilder.md)



[More...](#detailed-description)

* `#include <restincurl.h>`

















## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::unique\_ptr&lt; RequestBuilder &gt; | [**ptr\_t**](#typedef-ptr_t)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  RequestBuilder & | [**AcceptJson**](#function-acceptjson) () <br> |
|  RequestBuilder & | [**BasicAuthentication**](#function-basicauthentication) (const std::string & name, const std::string & passwd) <br> |
|  void | [**Build**](#function-build) () <br> |
|  bool | [**CanSendFile**](#function-cansendfile) () noexcept const<br> |
|  RequestBuilder & | [**ConnectTimeout**](#function-connecttimeout) (const long timeout) <br> |
|  RequestBuilder & | [**Delete**](#function-delete) (const std::string & url) <br> |
|  void | [**Execute**](#function-execute) () <br> |
|  void | [**ExecuteSynchronous**](#function-executesynchronous) () <br> |
|  RequestBuilder & | [**Get**](#function-get) (const std::string & url) <br> |
|  RequestBuilder & | [**Head**](#function-head) (const std::string & url) <br> |
|  RequestBuilder & | [**Header**](#function-header-12) (const char \* value) <br> |
|  RequestBuilder & | [**Header**](#function-header-22) (const std::string & name, const std::string & value) <br> |
|  RequestBuilder & | [**IgnoreIncomingData**](#function-ignoreincomingdata) () <br> |
|  RequestBuilder & | [**Option**](#function-option) (const CURLoption & opt, const T & value) <br> |
|  RequestBuilder & | [**Options**](#function-options) (const std::string & url) <br> |
|  RequestBuilder & | [**Patch**](#function-patch) (const std::string & url) <br> |
|  RequestBuilder & | [**Post**](#function-post) (const std::string & url) <br> |
|  RequestBuilder & | [**PostMime**](#function-postmime) (const std::string & url) <br> |
|  RequestBuilder & | [**Put**](#function-put) (const std::string & url) <br> |
|   | [**RequestBuilder**](#function-requestbuilder) ([**Worker**](classrestincurl_1_1Worker.md) & worker) <br> |
|  RequestBuilder & | [**RequestTimeout**](#function-requesttimeout) (const long timeout) <br> |
|  RequestBuilder & | [**SendData**](#function-senddata-12) ([**OutDataHandler**](structrestincurl_1_1OutDataHandler.md)&lt; T &gt; & dh) <br> |
|  RequestBuilder & | [**SendData**](#function-senddata-22) (T data) <br> |
|  RequestBuilder & | [**SendFile**](#function-sendfile) (const std::string & path) <br> |
|  RequestBuilder & | [**SendFileAsForm**](#function-sendfileasform) (const std::string & path) <br> |
|  RequestBuilder & | [**SendFileAsMimeData**](#function-sendfileasmimedata) (const std::string & path, const std::string & name={}, const std::string & remoteName={}, const std::string & mimeType={}) <br> |
|  RequestBuilder & | [**SetReadHandler**](#function-setreadhandler) (size\_t(\*)(char \*, size\_t, size\_t, void \*) handler, void \* userdata) <br> |
|  RequestBuilder & | [**SetWriteHandler**](#function-setwritehandler) (size\_t(\*)(char \*, size\_t, size\_t, void \*) handler, void \* userdata) <br> |
|  RequestBuilder & | [**StoreData**](#function-storedata-12) ([**InDataHandler**](structrestincurl_1_1InDataHandler.md)&lt; T &gt; & dh) <br> |
|  RequestBuilder & | [**StoreData**](#function-storedata-22) (T & data) <br> |
|  RequestBuilder & | [**Trace**](#function-trace) (bool enable=true) <br> |
|  RequestBuilder & | [**WithCompletion**](#function-withcompletion) ([**completion\_fn\_t**](namespacerestincurl.md#typedef-completion_fn_t) fn) <br> |
|  RequestBuilder & | [**WithJson**](#function-withjson-12) () <br> |
|  RequestBuilder & | [**WithJson**](#function-withjson-22) (std::string body) <br> |
|   | [**~RequestBuilder**](#function-requestbuilder) () <br> |
























## Protected Functions

| Type | Name |
| ---: | :--- |
|  RequestBuilder & | [**Prepare**](#function-prepare) (RequestType rt, const std::string & url) <br> |




## Detailed Description


Convenient interface to build requests.


Even if this is a light-weight wrapper around libcurl, we have a simple and modern way to define our requests that contains convenience-methods for the most common use-cases. 


    
## Public Types Documentation




### typedef ptr\_t 

```C++
using restincurl::RequestBuilder::ptr_t = std::unique_ptr<RequestBuilder>;
```




<hr>
## Public Functions Documentation




### function AcceptJson 

```C++
inline RequestBuilder & restincurl::RequestBuilder::AcceptJson () 
```



Sets the accept header to "Application/json" 


        

<hr>



### function BasicAuthentication 

```C++
inline RequestBuilder & restincurl::RequestBuilder::BasicAuthentication (
    const std::string & name,
    const std::string & passwd
) 
```



HTTP Basic Authentication


Authenticate the request with HTTP Basic Authentication.




**Parameters:**


* `name` Name to authenticate with 
* `passwd` Password to authenticate with

Note that if name or password is empty, authentication is ignored. This makes it simple to add optional authentication to your project, by simply assigning values to the strings you pass here, or not. 


        

<hr>



### function Build 

```C++
inline void restincurl::RequestBuilder::Build () 
```




<hr>



### function CanSendFile 

```C++
inline bool restincurl::RequestBuilder::CanSendFile () noexcept const
```




<hr>



### function ConnectTimeout 

```C++
inline RequestBuilder & restincurl::RequestBuilder::ConnectTimeout (
    const long timeout
) 
```



Set the connect timeout for a request




**Parameters:**


* `timeout` Timeout in milliseconds. Set to -1 to use the default. 




        

<hr>



### function Delete 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Delete (
    const std::string & url
) 
```



Use a HTTP DELETE request 


        

<hr>



### function Execute 

```C++
inline void restincurl::RequestBuilder::Execute () 
```



Execute the request asynchronously


This will queue the request for processing. If the number of active requests are less than `RESTINCURL_MAX_CONNECTIONS`, the request will start executing almost immediately.


The method returns immediately.




**Exception:**


* [**restincurl::Exception**](classrestincurl_1_1Exception.md) derived exceptions on error

This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function ExecuteSynchronous 

```C++
inline void restincurl::RequestBuilder::ExecuteSynchronous () 
```



Execute the request synchronously


This will execute the request and call the callback (if you declared one) in the current thread before the method returns.




**Exception:**


* [**restincurl::Exception**](classrestincurl_1_1Exception.md) derived exceptions on error

This method is available even when `RESTINCURL_ENABLE_ASYNC` is enabled ( != 0). 


        

<hr>



### function Get 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Get (
    const std::string & url
) 
```



Use a HTTP GET request 


        

<hr>



### function Head 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Head (
    const std::string & url
) 
```



Use a HTTP HEAD request 


        

<hr>



### function Header [1/2]

```C++
inline RequestBuilder & restincurl::RequestBuilder::Header (
    const char * value
) 
```



Specify a HTTP header for the request.




**Parameters:**


* `value` The value of the header-line, properly formatted according to the relevant HTTP specifications. 




        

<hr>



### function Header [2/2]

```C++
inline RequestBuilder & restincurl::RequestBuilder::Header (
    const std::string & name,
    const std::string & value
) 
```



Specify a HTTP header for the request.




**Parameters:**


* `name` Name of the header 
* `value` The value of the header

This is a convenience method that will build the appropriate header for you. 


        

<hr>



### function IgnoreIncomingData 

```C++
inline RequestBuilder & restincurl::RequestBuilder::IgnoreIncomingData () 
```



Do not process incoming data


The response body will be read from the network, but not buffered and not available for later inspection. 


        

<hr>



### function Option 

```C++
template<typename T>
inline RequestBuilder & restincurl::RequestBuilder::Option (
    const CURLoption & opt,
    const T & value
) 
```



Sets a Curl options.




**Parameters:**


* `opt` CURLoption enum specifying the option 
* `value` Value to set.

It is critical that the type of the value is of the same type that libcurl is expecting for the option. RESTinCurl makes no attempt to validate or cast the values.


Please refer to the libcurl documentation for curl\_easy\_setopt() 


        

<hr>



### function Options 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Options (
    const std::string & url
) 
```



Use a HTTP OPTIONS request 


        

<hr>



### function Patch 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Patch (
    const std::string & url
) 
```



Use a HTTP PATCH request 


        

<hr>



### function Post 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Post (
    const std::string & url
) 
```



Use a HTTP POST request 


        

<hr>



### function PostMime 

```C++
inline RequestBuilder & restincurl::RequestBuilder::PostMime (
    const std::string & url
) 
```



Use a HTTP POST request 


        

<hr>



### function Put 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Put (
    const std::string & url
) 
```



Use a HTTP PUT request 


        

<hr>



### function RequestBuilder 

```C++
inline restincurl::RequestBuilder::RequestBuilder (
    Worker & worker
) 
```




<hr>



### function RequestTimeout 

```C++
inline RequestBuilder & restincurl::RequestBuilder::RequestTimeout (
    const long timeout
) 
```



Set request timeout




**Parameters:**


* `timeout` Timeout in milliseconds. Set to -1 to use the default. 




        

<hr>



### function SendData [1/2]

```C++
template<typename T>
inline RequestBuilder & restincurl::RequestBuilder::SendData (
    OutDataHandler < T > & dh
) 
```



Specify Data Handler for outbound data


You can use this method when you need to use a Data Handler, rather than a simple string, to provide the data for a POST, PUT etc. request.




**Parameters:**


* `dh` Data Handler instance.

Note that the Data Handler is passed by reference. It is your responsibility that the instance is present at least until the request has finished (your code owns the Data Handler instance). 


        

<hr>



### function SendData [2/2]

```C++
template<typename T>
inline RequestBuilder & restincurl::RequestBuilder::SendData (
    T data
) 
```



Convenience method to specify a object that contains the data to send during a request.




**Parameters:**


* `data` Data to send. Typically this will be a std::string, std::vector&lt;char&gt; or a similar object.

RESTinCurl takes ownership of this data (by moving it). 


        

<hr>



### function SendFile 

```C++
inline RequestBuilder & restincurl::RequestBuilder::SendFile (
    const std::string & path
) 
```



Send a file




**Parameters:**


* `path` Full path to the file to send.



**Exception:**


* [**SystemException**](classrestincurl_1_1SystemException.md) if the file cannot be opened. 
* [**Exception**](classrestincurl_1_1Exception.md) if the method is called for a non-send operation 




        

<hr>



### function SendFileAsForm 

```C++
inline RequestBuilder & restincurl::RequestBuilder::SendFileAsForm (
    const std::string & path
) 
```



Send a file




**Parameters:**


* `path` Full path to the file to send.



**Exception:**


* [**SystemException**](classrestincurl_1_1SystemException.md) if the file cannot be opened. 
* [**Exception**](classrestincurl_1_1Exception.md) if the method is called for a non-send operation 




        

<hr>



### function SendFileAsMimeData 

```C++
inline RequestBuilder & restincurl::RequestBuilder::SendFileAsMimeData (
    const std::string & path,
    const std::string & name={},
    const std::string & remoteName={},
    const std::string & mimeType={}
) 
```



Send a file as a multipart/form mime segment




**Parameters:**


* `path` Full path to the file tro send 
* `name` Otional name to use for the file in the mime segment 
* `remoteName` Optional name to label the file as for the remote end 
* `mimeType` Optional mime-type for the file



**Exception:**


* [**Exception**](classrestincurl_1_1Exception.md) if the method is called for a non-mime-post operation 




        

<hr>



### function SetReadHandler 

```C++
inline RequestBuilder & restincurl::RequestBuilder::SetReadHandler (
    size_t(*)(char *, size_t, size_t, void *) handler,
    void * userdata
) 
```



Set a Curl compatible read handler.




**Parameters:**


* `handler` Curl C API read handler

You probably don't need to call this directly. 


        

<hr>



### function SetWriteHandler 

```C++
inline RequestBuilder & restincurl::RequestBuilder::SetWriteHandler (
    size_t(*)(char *, size_t, size_t, void *) handler,
    void * userdata
) 
```



Set a Curl compatible write handler.




**Parameters:**


* `handler` Curl C API write handler

You probably don't need to call this directly. 


        

<hr>



### function StoreData [1/2]

```C++
template<typename T>
inline RequestBuilder & restincurl::RequestBuilder::StoreData (
    InDataHandler < T > & dh
) 
```



Specify Data Handler for inbound data


You can use this method when you need to use a Data Handler, rather than a simple string, to receive data during the request.




**Parameters:**


* `dh` Data Handler instance.

Note that the Data Handler is passed by reference. It is your responsibility that the instance is present at least until the request has finished (your code owns the Data Handler instance). 


        

<hr>



### function StoreData [2/2]

```C++
template<typename T>
inline RequestBuilder & restincurl::RequestBuilder::StoreData (
    T & data
) 
```



Convenience method to specify a object that receives incoming data during a request.




**Parameters:**


* `data` Buffer to hold incoming data. Typically this will be a std::string, std::vector&lt;char&gt; or a similar object.

Note that data is passed by reference. It is your responsibility that the instance is present at least until the request has finished (your code owns the object). 


        

<hr>



### function Trace 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Trace (
    bool enable=true
) 
```



Enables or disables trace logging for requests.


The trace logging will show detailed information about what libcurl does and data sent and received during a request.


Basically it sets `CURLOPT_DEBUGFUNCTION` and `CURLOPT_VERBOSE`. 


        

<hr>



### function WithCompletion 

```C++
inline RequestBuilder & restincurl::RequestBuilder::WithCompletion (
    completion_fn_t fn
) 
```



Specify a callback that will be called when the request is complete (or failed).




**Parameters:**


* `fn` Callback to be called

For asynchronous requests, the callback will be called from the worker-thread shared by all requests and timers for the client instance. It is imperative that you return immediately, and don't keep the thread busy more than strictly required. If you need do do some computing or IO in response to the information you receive, you should do that in another thread. 


        

<hr>



### function WithJson [1/2]

```C++
inline RequestBuilder & restincurl::RequestBuilder::WithJson () 
```



Sets the content-type to "Application/json; charset=utf-8" 


        

<hr>



### function WithJson [2/2]

```C++
inline RequestBuilder & restincurl::RequestBuilder::WithJson (
    std::string body
) 
```



Sets the content-type to "Application/json; charset=utf-8"




**Parameters:**


* `body` Json payload to send with the request. 




        

<hr>



### function ~RequestBuilder 

```C++
inline restincurl::RequestBuilder::~RequestBuilder () 
```




<hr>
## Protected Functions Documentation




### function Prepare 

```C++
inline RequestBuilder & restincurl::RequestBuilder::Prepare (
    RequestType rt,
    const std::string & url
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

