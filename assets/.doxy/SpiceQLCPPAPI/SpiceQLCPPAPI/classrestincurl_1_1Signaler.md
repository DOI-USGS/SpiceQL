

# Class restincurl::Signaler



[**ClassList**](annotated.md) **>** [**restincurl**](namespacerestincurl.md) **>** [**Signaler**](classrestincurl_1_1Signaler.md)






















## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::array&lt; int, 2 &gt; | [**pipefd\_t**](#typedef-pipefd_t)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  int | [**GetReadFd**](#function-getreadfd) () <br> |
|  void | [**Signal**](#function-signal) () <br> |
|   | [**Signaler**](#function-signaler) () <br> |
|  bool | [**WasSignalled**](#function-wassignalled) () <br> |
|   | [**~Signaler**](#function-signaler) () <br> |




























## Public Types Documentation




### typedef pipefd\_t 

```C++
using restincurl::Signaler::pipefd_t = std::array<int, 2>;
```




<hr>
## Public Functions Documentation




### function GetReadFd 

```C++
inline int restincurl::Signaler::GetReadFd () 
```




<hr>



### function Signal 

```C++
inline void restincurl::Signaler::Signal () 
```




<hr>



### function Signaler 

```C++
inline restincurl::Signaler::Signaler () 
```




<hr>



### function WasSignalled 

```C++
inline bool restincurl::Signaler::WasSignalled () 
```




<hr>



### function ~Signaler 

```C++
inline restincurl::Signaler::~Signaler () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/restincurl.h`

