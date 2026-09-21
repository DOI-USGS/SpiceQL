

# Class restincurl::TlsLocker



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**TlsLocker**](classrestincurl_1_1TlsLocker.md)



[More...](#detailed-description)

* `#include <restincurl.h>`







































## Public Static Functions

| Type | Name |
| ---: | :--- |
|  void | [**tlsLOckInit**](#function-tlslockinit) () <br> |
|  void | [**tlsLockKill**](#function-tlslockkill) () <br> |


























## Detailed Description


Thread support for the TLS layer used by libcurl.


Some TLS libraries require that you supply callback functions to deal with thread synchronization.


See [https://curl.haxx.se/libcurl/c/threadsafe.html](https://curl.haxx.se/libcurl/c/threadsafe.html)


You can deal with this yourself, or in the case that RESTinCurl support your TLS library, you can use this class.


Currently supported libraries:



* Opeenssl (only required for OpenSSL &lt;= 1.0.2) define `RESTINCURL_WITH_OPENSSL_THREADS`




The class is written so that you can use it in your code, and it will only do something when the appropriate define is set.


This class is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


    
## Public Static Functions Documentation




### function tlsLOckInit 

```C++
static inline void restincurl::TlsLocker::tlsLOckInit () 
```



Enable the built-in support. 


        

<hr>



### function tlsLockKill 

```C++
static inline void restincurl::TlsLocker::tlsLockKill () 
```



Free up resources used when finished using TLS. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

