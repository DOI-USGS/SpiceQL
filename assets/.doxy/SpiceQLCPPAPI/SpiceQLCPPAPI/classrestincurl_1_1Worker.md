

# Class restincurl::Worker



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Worker**](classrestincurl_1_1Worker.md)










































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**Close**](#function-close) () <br> |
|  void | [**CloseWhenFinished**](#function-closewhenfinished) () <br> |
|  void | [**Enqueue**](#function-enqueue) (Request::ptr\_t req) <br> |
|  size\_t | [**GetNumActiveRequests**](#function-getnumactiverequests) () const<br> |
|  bool | [**HaveThread**](#function-havethread) () noexcept const<br> |
|  bool | [**IsDone**](#function-isdone) () const<br> |
|  void | [**Join**](#function-join) () const<br> |
|  void | [**PrepareThread**](#function-preparethread) () <br> |
|   | [**Worker**](#function-worker) () = default<br> |
|   | [**~Worker**](#function-worker) () <br> |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  std::unique\_ptr&lt; Worker &gt; | [**Create**](#function-create) () <br> |


























## Public Functions Documentation




### function Close 

```C++
inline void restincurl::Worker::Close () 
```




<hr>



### function CloseWhenFinished 

```C++
inline void restincurl::Worker::CloseWhenFinished () 
```




<hr>



### function Enqueue 

```C++
inline void restincurl::Worker::Enqueue (
    Request::ptr_t req
) 
```




<hr>



### function GetNumActiveRequests 

```C++
inline size_t restincurl::Worker::GetNumActiveRequests () const
```




<hr>



### function HaveThread 

```C++
inline bool restincurl::Worker::HaveThread () noexcept const
```




<hr>



### function IsDone 

```C++
inline bool restincurl::Worker::IsDone () const
```




<hr>



### function Join 

```C++
inline void restincurl::Worker::Join () const
```




<hr>



### function PrepareThread 

```C++
inline void restincurl::Worker::PrepareThread () 
```




<hr>



### function Worker 

```C++
restincurl::Worker::Worker () = default
```




<hr>



### function ~Worker 

```C++
inline restincurl::Worker::~Worker () 
```




<hr>
## Public Static Functions Documentation




### function Create 

```C++
static inline std::unique_ptr< Worker > restincurl::Worker::Create () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

