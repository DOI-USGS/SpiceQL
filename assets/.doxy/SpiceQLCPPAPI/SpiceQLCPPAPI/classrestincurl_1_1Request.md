

# Class restincurl::Request



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Request**](classrestincurl_1_1Request.md)






















## Public Types

| Type | Name |
| ---: | :--- |
| typedef curl\_slist \* | [**headers\_t**](#typedef-headers_t)  <br> |
| typedef std::unique\_ptr&lt; Request &gt; | [**ptr\_t**](#typedef-ptr_t)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**AddFileAsMimeData**](#function-addfileasmimedata) (const std::string & path, const std::string & name, const std::string & remoteName, const std::string & mimeType) <br> |
|  void | [**Complete**](#function-complete) (CURLcode cc, const CURLMSG &) <br> |
|  void | [**Execute**](#function-execute) () <br> |
|  [**EasyHandle**](classrestincurl_1_1EasyHandle.md) & | [**GetEasyHandle**](#function-geteasyhandle) () noexcept<br> |
|  headers\_t & | [**GetHeaders**](#function-getheaders) () <br> |
|  RequestType | [**GetRequestType**](#function-getrequesttype) () noexcept<br> |
|  FILE \* | [**GetSourceFp**](#function-getsourcefp) () <br> |
|  void | [**InitMime**](#function-initmime) () <br> |
|  void | [**OpenSourceFile**](#function-opensourcefile) (const std::string & path) <br> |
|  void | [**Prepare**](#function-prepare) (const RequestType rq, [**completion\_fn\_t**](namespacerestincurl.md#typedef-completion_fn_t) completion) <br> |
|   | [**Request**](#function-request-12) () <br> |
|   | [**Request**](#function-request-22) (EasyHandle::ptr\_t && eh) <br> |
|  void | [**SetDefaultInHandler**](#function-setdefaultinhandler) (std::unique\_ptr&lt; [**DataHandlerBase**](structrestincurl_1_1DataHandlerBase.md) &gt; ptr) <br> |
|  void | [**SetDefaultOutHandler**](#function-setdefaultouthandler) (std::unique\_ptr&lt; [**DataHandlerBase**](structrestincurl_1_1DataHandlerBase.md) &gt; ptr) <br> |
|  std::string & | [**getDefaultInBuffer**](#function-getdefaultinbuffer) () <br> |
|   | [**~Request**](#function-request) () <br> |




























## Public Types Documentation




### typedef headers\_t 

```C++
using restincurl::Request::headers_t = curl_slist *;
```




<hr>



### typedef ptr\_t 

```C++
using restincurl::Request::ptr_t = std::unique_ptr<Request>;
```




<hr>
## Public Functions Documentation




### function AddFileAsMimeData 

```C++
inline void restincurl::Request::AddFileAsMimeData (
    const std::string & path,
    const std::string & name,
    const std::string & remoteName,
    const std::string & mimeType
) 
```




<hr>



### function Complete 

```C++
inline void restincurl::Request::Complete (
    CURLcode cc,
    const CURLMSG &
) 
```




<hr>



### function Execute 

```C++
inline void restincurl::Request::Execute () 
```




<hr>



### function GetEasyHandle 

```C++
inline EasyHandle & restincurl::Request::GetEasyHandle () noexcept
```




<hr>



### function GetHeaders 

```C++
inline headers_t & restincurl::Request::GetHeaders () 
```




<hr>



### function GetRequestType 

```C++
inline RequestType restincurl::Request::GetRequestType () noexcept
```




<hr>



### function GetSourceFp 

```C++
inline FILE * restincurl::Request::GetSourceFp () 
```




<hr>



### function InitMime 

```C++
inline void restincurl::Request::InitMime () 
```




<hr>



### function OpenSourceFile 

```C++
inline void restincurl::Request::OpenSourceFile (
    const std::string & path
) 
```




<hr>



### function Prepare 

```C++
inline void restincurl::Request::Prepare (
    const RequestType rq,
    completion_fn_t completion
) 
```




<hr>



### function Request [1/2]

```C++
inline restincurl::Request::Request () 
```




<hr>



### function Request [2/2]

```C++
inline restincurl::Request::Request (
    EasyHandle::ptr_t && eh
) 
```




<hr>



### function SetDefaultInHandler 

```C++
inline void restincurl::Request::SetDefaultInHandler (
    std::unique_ptr< DataHandlerBase > ptr
) 
```




<hr>



### function SetDefaultOutHandler 

```C++
inline void restincurl::Request::SetDefaultOutHandler (
    std::unique_ptr< DataHandlerBase > ptr
) 
```




<hr>



### function getDefaultInBuffer 

```C++
inline std::string & restincurl::Request::getDefaultInBuffer () 
```




<hr>



### function ~Request 

```C++
inline restincurl::Request::~Request () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

