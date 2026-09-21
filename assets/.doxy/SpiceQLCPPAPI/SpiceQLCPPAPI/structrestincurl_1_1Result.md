

# Struct restincurl::Result



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Result**](structrestincurl_1_1Result.md)



[More...](#detailed-description)

* `#include <restincurl.h>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::string | [**body**](#variable-body)  <br> |
|  CURLcode | [**curl\_code**](#variable-curl_code)   = `{}`<br> |
|  long | [**http\_response\_code**](#variable-http_response_code)   = `{}`<br> |
|  std::string | [**msg**](#variable-msg)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Result**](#function-result-12) () = default<br> |
|   | [**Result**](#function-result-22) (const CURLcode & code) <br> |
|  bool | [**isOk**](#function-isok) () noexcept const<br> |




























## Detailed Description


The [**Result**](structrestincurl_1_1Result.md) from a request\ 


    
## Public Attributes Documentation




### variable body 

```C++
std::string restincurl::Result::body;
```



The body of the request returned by the server.


Note that if you specified your own body handler or body variable, for the request, `body` will be empty. 


        

<hr>



### variable curl\_code 

```C++
CURLcode restincurl::Result::curl_code;
```



The CURLcode returned by libcurl for this request.


CURLE\_OK (or 0) indicates success. 


        

<hr>



### variable http\_response\_code 

```C++
long restincurl::Result::http_response_code;
```



The HTTP result code for the request 


        

<hr>



### variable msg 

```C++
std::string restincurl::Result::msg;
```



If the request was unsuccessful (curl\_code != 0), the error string reported by libcurl. 


        

<hr>
## Public Functions Documentation




### function Result [1/2]

```C++
restincurl::Result::Result () = default
```




<hr>



### function Result [2/2]

```C++
inline restincurl::Result::Result (
    const CURLcode & code
) 
```




<hr>



### function isOk 

```C++
inline bool restincurl::Result::isOk () noexcept const
```



Check if the reqtest appears to be successful 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

