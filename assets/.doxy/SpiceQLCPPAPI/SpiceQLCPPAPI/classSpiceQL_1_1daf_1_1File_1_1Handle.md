

# Class SpiceQL::daf::File::Handle



[**ClassList**](annotated.md) **>** [**Handle**](classSpiceQL_1_1daf_1_1File_1_1Handle.md)



[More...](#detailed-description)






































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Handle**](#function-handle-12) () = default<br> |
|   | [**Handle**](#function-handle-22) (const Handle &) = delete<br> |
|  uint64\_t | [**open**](#function-open) (const std::string & path) <br> |
|  Handle & | [**operator=**](#function-operator) (const Handle &) = delete<br> |
|  void | [**read**](#function-read) (uint64\_t offset, char \* dst, size\_t n) const<br> |
|   | [**~Handle**](#function-handle) () <br> |




























## Detailed Description


The open file. On POSIX each read is one pread, which keeps no file position, so a cache miss costs one system call instead of the seek and read an ifstream makes. With many threads reading at once those extra seeks added up to most of the run time. Closed by its destructor, so a [**File**](classSpiceQL_1_1daf_1_1File.md) that fails to construct doesn't leak it. 


    
## Public Functions Documentation




### function Handle [1/2]

```C++
Handle::Handle () = default
```




<hr>



### function Handle [2/2]

```C++
Handle::Handle (
    const Handle &
) = delete
```




<hr>



### function open 

```C++
inline uint64_t Handle::open (
    const std::string & path
) 
```




<hr>



### function operator= 

```C++
Handle & Handle::operator= (
    const Handle &
) = delete
```




<hr>



### function read 

```C++
inline void Handle::read (
    uint64_t offset,
    char * dst,
    size_t n
) const
```




<hr>



### function ~Handle 

```C++
inline Handle::~Handle () 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/daf_reader.h`

