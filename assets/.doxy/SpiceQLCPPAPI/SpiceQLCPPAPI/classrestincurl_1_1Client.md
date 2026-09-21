

# Class restincurl::Client



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Client**](classrestincurl_1_1Client.md)



[More...](#detailed-description)

* `#include <restincurl.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|  std::unique\_ptr&lt; [**RequestBuilder**](classrestincurl_1_1RequestBuilder.md) &gt; | [**Build**](#function-build) () <br> |
|   | [**Client**](#function-client) (const bool init=true) <br> |
|  void | [**Close**](#function-close) () <br> |
|  void | [**CloseWhenFinished**](#function-closewhenfinished) () <br> |
|  size\_t | [**GetNumActiveRequests**](#function-getnumactiverequests) () <br> |
|  bool | [**HaveWorker**](#function-haveworker) () const<br> |
|  void | [**WaitForFinish**](#function-waitforfinish) () <br> |
| virtual  | [**~Client**](#function-client) () <br> |




























## Detailed Description


The high level abstraction of the Curl library.


An instance of a [**Client**](classrestincurl_1_1Client.md) will, if asynchronous mode is enabled, create a worker-thread when the first request is made. This single worker-thread is then shared between all requests made for this client. When no requests are active, the thread will wait for a while (see RESTINCURL\_IDLE\_TIMEOUT\_SEC), keeping libcurl's connection-cache warm, to serve new requests as efficiently as possible. When the idle time period expires, the thread will terminate and close the connection-cache associated with the client. If a new request is made later on, a new worker-thread will be created. 


    
## Public Functions Documentation




### function Build 

```C++
inline std::unique_ptr< RequestBuilder > restincurl::Client::Build () 
```



Build a request


Requests are "built" using a series of statements to fully express what you want to do and how you want RESTinCurl to do it.


Example 
```C++
restincurl::Client client;
client.Build()->Get("https://api.example.com")
    .AcceptJson()
    .Header("X-Client", "restincurl")
    .WithCompletion([&](const Result& result) {
        // Do something
    })
    .Execute();
```
 


        

<hr>



### function Client 

```C++
inline restincurl::Client::Client (
    const bool init=true
) 
```



Constructor




**Parameters:**


* `init` Set to true if you need to initialize libcurl.

Libcurl require us to call an initialization method once, before using the library. If you use libcurl exclusively from RESTinCurl, `init` needs to be `true` (default). If you use RESTinCurl in a project that already initialize libcurl, you should pass `false` to the constructor.


RESTinCurl will only call libcurl's initialization once, no matter how many times you call the constructor. It's therefore safe to always use the default value when you want RESTinCurl to deal with libcurl's initialization. 


        

<hr>



### function Close 

```C++
inline void restincurl::Client::Close () 
```



Close the client.


This method aborts any and all requests.


The worked-thread will exit shortly after this method is called, if it was running.


This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function CloseWhenFinished 

```C++
inline void restincurl::Client::CloseWhenFinished () 
```



Shut down the event-loop and clean up internal resources when all active and queued requests are done.


This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function GetNumActiveRequests 

```C++
inline size_t restincurl::Client::GetNumActiveRequests () 
```



Get the number of active / ongoing requests.


This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function HaveWorker 

```C++
inline bool restincurl::Client::HaveWorker () const
```



Check if the client instance has a worker-thread.


This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function WaitForFinish 

```C++
inline void restincurl::Client::WaitForFinish () 
```



Wait for the worker-thread to finish.


You should call [**Close()**](classrestincurl_1_1Client.md#function-close) first.


This method is only available when `RESTINCURL_ENABLE_ASYNC` is nonzero. 


        

<hr>



### function ~Client 

```C++
inline virtual restincurl::Client::~Client () 
```



Destructor


The destructor will try to clean up resources (when in asynchronous mode) ant may wait for a little while for IO operations to stop and the worker-thread to finish.


This is to prevent your application for crashing if you exit the main thread while the worker-thread is still working and accessing memory own by the [**Client**](classrestincurl_1_1Client.md) instance. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

