

# File restincurl.h



[**FileList**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**restincurl.h**](restincurl_8h.md)

[Go to the source code of this file](restincurl_8h_source.md)



* `#include <algorithm>`
* `#include <atomic>`
* `#include <deque>`
* `#include <exception>`
* `#include <functional>`
* `#include <iostream>`
* `#include <iterator>`
* `#include <map>`
* `#include <memory>`
* `#include <mutex>`
* `#include <string>`
* `#include <thread>`
* `#include <vector>`
* `#include <assert.h>`
* `#include <curl/curl.h>`
* `#include <curl/easy.h>`
* `#include <fcntl.h>`
* `#include <string.h>`
* `#include <sys/select.h>`
* `#include <sys/time.h>`
* `#include <sys/types.h>`
* `#include <sys/stat.h>`
* `#include <unistd.h>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**restincurl**](namespacerestincurl.md) <br> |


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

















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**RESTINCURL\_ENABLE\_ASYNC**](restincurl_8h.md#define-restincurl_enable_async)  `1`<br>_Enables or disables asynchronous mode._  |
| define  | [**RESTINCURL\_ENABLE\_DEFAULT\_LOGGER**](restincurl_8h.md#define-restincurl_enable_default_logger)  `0`<br>_Enables a simple built-in logger._  |
| define  | [**RESTINCURL\_IDLE\_TIMEOUT\_SEC**](restincurl_8h.md#define-restincurl_idle_timeout_sec)  `60`<br>_How long to wait for the next request before the idle worker-thread is stopped._  |
| define  | [**RESTINCURL\_LOG**](restincurl_8h.md#define-restincurl_log) (msg) <br>_Macro to log debug messages._  |
| define  | [**RESTINCURL\_LOG\_TRACE**](restincurl_8h.md#define-restincurl_log_trace) (msg) <br>_Macro to log debug messages._  |
| define  | [**RESTINCURL\_LOG\_VERBOSE\_ENABLE**](restincurl_8h.md#define-restincurl_log_verbose_enable)  `0`<br>_Enable very verbose logging._  |
| define  | [**RESTINCURL\_MAX\_CONNECTIONS**](restincurl_8h.md#define-restincurl_max_connections)  `32L`<br>_Max concurrent connections._  |

## Macro Definition Documentation





### define RESTINCURL\_ENABLE\_ASYNC 

_Enables or disables asynchronous mode._ 
```C++
#define RESTINCURL_ENABLE_ASYNC `1`
```



In asynchronous mode, many requests can be served simultaneously using a worker-thread. In synchronous mode, you use the current thread to serve only one request at the time.


Default is 1 (asynchronous mode enabled). 


        

<hr>



### define RESTINCURL\_ENABLE\_DEFAULT\_LOGGER 

_Enables a simple built-in logger._ 
```C++
#define RESTINCURL_ENABLE_DEFAULT_LOGGER `0`
```



RESTinCurl has a very simple built in logger.


It writes to either std::clog, the Unix syslog or Androids log facility.



* Define RESTINCURL\_USE\_SYSLOG to use syslog
* Define RESTINCURL\_USE\_ANDROID\_NDK\_LOG to use the Android NDK logger.




By default, it will write to std::clog.


Default value is 0 (disabled) 


        

<hr>



### define RESTINCURL\_IDLE\_TIMEOUT\_SEC 

_How long to wait for the next request before the idle worker-thread is stopped._ 
```C++
#define RESTINCURL_IDLE_TIMEOUT_SEC `60`
```



This will delete curl's connection-cache and cause a new thread to be created and new connections to be made if there are new requests at at later time.


Note that this option is only relevant in asynchronous mode.


Default is 60 seconds. 


        

<hr>



### define RESTINCURL\_LOG 

_Macro to log debug messages._ 
```C++
#define RESTINCURL_LOG (
    msg
) 
```



If you want to log messages from the library to your own log facility, you may define this macro to do so.


Note that the logging statements in the library expect the log `msg` to be std::ostream compliant. The macro must be able to deal with statement such as:
* RESTINCURL\_LOG("test");
* RESTINCURL\_LOG("test " &lt;&lt; 1 &lt;&lt; " and " &lt;&lt; 2);




If you manually define this macro, you should not define `RESTINCURL_ENABLE_DEFAULT_LOGGER`. 


        

<hr>



### define RESTINCURL\_LOG\_TRACE 

_Macro to log debug messages._ 
```C++
#define RESTINCURL_LOG_TRACE (
    msg
) 
```



If you want to log trace-messages from the library to your own log facility, you may define this macro to do so.


Note that the logging statements in the library expect the log `msg` to be std::ostream compliant. The macro must be able to deal with statement such as:
* RESTINCURL\_LOG\_TRACE("test");
* RESTINCURL\_LOG\_TRACE("test " &lt;&lt; 1 &lt;&lt; " and " &lt;&lt; 2);




If you manually define this macro, you should not define `RESTINCURL_ENABLE_DEFAULT_LOGGER`.


This macro is only called if `RESTINCURL_LOG_VERBOSE_ENABLE` is defined and not 0. 


        

<hr>



### define RESTINCURL\_LOG\_VERBOSE\_ENABLE 

_Enable very verbose logging._ 
```C++
#define RESTINCURL_LOG_VERBOSE_ENABLE `0`
```



This option enabled very verbose logging, suitable to pinpoint problems during development / porting of the library itself.


Default is 0 (disabled). 


        

<hr>



### define RESTINCURL\_MAX\_CONNECTIONS 

_Max concurrent connections._ 
```C++
#define RESTINCURL_MAX_CONNECTIONS `32L`
```



This option restrains the maximum number of concurrent connections that will be used at any time. It sets libcurl's `CURLMOPT_MAXCONNECTS` option, and also puts any new requests in a waiting queue. As soon as some active request finish, the oldest waiting request will be served (FIFO queue).


The default value is 32 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

