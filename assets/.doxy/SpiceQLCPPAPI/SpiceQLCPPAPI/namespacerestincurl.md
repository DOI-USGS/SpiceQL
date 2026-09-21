

# Namespace restincurl



[**Namespace List**](namespaces.md) **>** [**restincurl**](namespacerestincurl.md)




















## Classes

| Type | Name |
| ---: | :--- |
| class | [**Client**](classrestincurl_1_1Client.md) <br> |
| class | [**CurlException**](classrestincurl_1_1CurlException.md) <br> |
| struct | [**DataHandlerBase**](structrestincurl_1_1DataHandlerBase.md) <br> |
| class | [**EasyHandle**](classrestincurl_1_1EasyHandle.md) <br> |
| class | [**Exception**](classrestincurl_1_1Exception.md) <br> |
| struct | [**InDataHandler**](structrestincurl_1_1InDataHandler.md) &lt;typename T&gt;<br> |
| class | [**Options**](classrestincurl_1_1Options.md) <br> |
| struct | [**OutDataHandler**](structrestincurl_1_1OutDataHandler.md) &lt;typename T&gt;<br> |
| class | [**Request**](classrestincurl_1_1Request.md) <br> |
| class | [**RequestBuilder**](classrestincurl_1_1RequestBuilder.md) <br> |
| struct | [**Result**](structrestincurl_1_1Result.md) <br> |
| class | [**Signaler**](classrestincurl_1_1Signaler.md) <br> |
| class | [**SystemException**](classrestincurl_1_1SystemException.md) <br> |
| class | [**TlsLocker**](classrestincurl_1_1TlsLocker.md) <br> |
| class | [**Worker**](classrestincurl_1_1Worker.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**RequestType**](#enum-requesttype)  <br> |
| typedef std::function&lt; void(const [**Result**](structrestincurl_1_1Result.md) &result)&gt; | [**completion\_fn\_t**](#typedef-completion_fn_t)  <br> |
| typedef std::lock\_guard&lt; std::mutex &gt; | [**lock\_t**](#typedef-lock_t)  <br> |
















































## Public Types Documentation




### enum RequestType 

```C++
enum restincurl::RequestType {
    GET,
    PUT,
    POST,
    HEAD,
    DELETE,
    PATCH,
    OPTIONS,
    POST_MIME,
    INVALID
};
```




<hr>



### typedef completion\_fn\_t 

```C++
using restincurl::completion_fn_t = std::function<void (const Result& result)>;
```



Completion debug\_callback


This callback is called when a request completes, or fails.




**Parameters:**


* `result` The result of the request. 




        

<hr>



### typedef lock\_t 

```C++
using restincurl::lock_t = std::lock_guard<std::mutex>;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

