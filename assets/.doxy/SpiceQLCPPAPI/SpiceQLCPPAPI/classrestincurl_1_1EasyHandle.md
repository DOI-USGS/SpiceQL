

# Class restincurl::EasyHandle



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**EasyHandle**](classrestincurl_1_1EasyHandle.md)






















## Public Types

| Type | Name |
| ---: | :--- |
| typedef decltype(curl\_easy\_init()) | [**handle\_t**](#typedef-handle_t)  <br> |
| typedef std::unique\_ptr&lt; EasyHandle &gt; | [**ptr\_t**](#typedef-ptr_t)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**Close**](#function-close) () <br> |
|   | [**EasyHandle**](#function-easyhandle) () <br> |
|   | [**operator handle\_t**](#function-operator-handle_t) () noexcept const<br> |
|   | [**~EasyHandle**](#function-easyhandle) () <br> |




























## Public Types Documentation




### typedef handle\_t 

```C++
using restincurl::EasyHandle::handle_t = decltype(curl_easy_init());
```




<hr>



### typedef ptr\_t 

```C++
using restincurl::EasyHandle::ptr_t = std::unique_ptr<EasyHandle>;
```




<hr>
## Public Functions Documentation




### function Close 

```C++
inline void restincurl::EasyHandle::Close () 
```




<hr>



### function EasyHandle 

```C++
inline restincurl::EasyHandle::EasyHandle () 
```




<hr>



### function operator handle\_t 

```C++
inline restincurl::EasyHandle::operator handle_t () noexcept const
```




<hr>



### function ~EasyHandle 

```C++
inline restincurl::EasyHandle::~EasyHandle () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

