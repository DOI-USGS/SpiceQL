

# Namespace SpiceQL::Memo



[**Namespace List**](namespaces.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Memo**](namespaceSpiceQL_1_1Memo.md)




















## Classes

| Type | Name |
| ---: | :--- |
| class | [**Memory**](classSpiceQL_1_1Memo_1_1Memory.md) <br> |
| struct | [**memoize**](structSpiceQL_1_1Memo_1_1memoize.md) &lt;typename Cache, typename Function&gt;<br> |






















## Public Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**\_hash\_combine**](#function-_hash_combine) (std::size\_t & seed, const T & v) <br> |
|  std::vector&lt; std::vector&lt; std::string &gt; &gt; | [**getPathsFromRegex**](#function-getpathsfromregex) (std::string root, std::vector&lt; std::string &gt; regexes) <br> |
|  std::vector&lt; std::pair&lt; double, double &gt; &gt; | [**getTimeIntervals**](#function-gettimeintervals) (std::string kpath) <br>_Get start and stop times a kernel._  |
|  std::string | [**globTimeIntervals**](#function-globtimeintervals) (std::string mission) <br>_Get start and stop times for all kernels._  |
|  size\_t | [**hash\_combine**](#function-hash_combine) (std::size\_t & seed) <br> |
|  size\_t | [**hash\_combine**](#function-hash_combine) (std::size\_t & seed, const T & t, const Params &... params) <br> |
|  std::vector&lt; std::string &gt; | [**ls**](#function-ls) (std::string const & root, bool recursive) <br>_ls, like in unix, kinda. Also it's a function. This is memoized so it'll load from cache if run multiple times with the same parameters._  |
|  [**memoize**](structSpiceQL_1_1Memo_1_1memoize.md)&lt; Cache, Function &gt; | [**make\_memoized**](#function-make_memoized) (Cache & fc, const std::string & id, Function f) <br> |




























## Public Functions Documentation




### function \_hash\_combine 

```C++
template<class T>
inline size_t SpiceQL::Memo::_hash_combine (
    std::size_t & seed,
    const T & v
) 
```




<hr>



### function getPathsFromRegex 

```C++
std::vector< std::vector< std::string > > SpiceQL::Memo::getPathsFromRegex (
    std::string root,
    std::vector< std::string > regexes
) 
```




<hr>



### function getTimeIntervals 

_Get start and stop times a kernel._ 
```C++
std::vector< std::pair< double, double > > SpiceQL::Memo::getTimeIntervals (
    std::string kpath
) 
```



For each segment in the kernel, get all start and stop times as a vector of double pairs. This gets all start and stop times regardless of the frame associated with it.


Input kernel is assumed to be a binary kernel with time dependant external orientation data.




**Parameters:**


* `kpath` Path to the kernel 



**Returns:**

std::vector of start and stop times 





        

<hr>



### function globTimeIntervals 

_Get start and stop times for all kernels._ 
```C++
std::string SpiceQL::Memo::globTimeIntervals (
    std::string mission
) 
```





**Returns:**

string json map of kernel names to list of time segments 





        

<hr>



### function hash\_combine 

```C++
inline size_t SpiceQL::Memo::hash_combine (
    std::size_t & seed
) 
```




<hr>



### function hash\_combine 

```C++
template<typename T, typename... Params>
inline size_t SpiceQL::Memo::hash_combine (
    std::size_t & seed,
    const T & t,
    const Params &... params
) 
```




<hr>



### function ls 

_ls, like in unix, kinda. Also it's a function. This is memoized so it'll load from cache if run multiple times with the same parameters._ 
```C++
std::vector< std::string > SpiceQL::Memo::ls (
    std::string const & root,
    bool recursive
) 
```



Iterates the input path and returning a list of files. Optionally, recursively.




**Parameters:**


* `root` The root directory to search 
* `recursive` recursively iterates through directories if true



**Returns:**

list of paths 





        

<hr>



### function make\_memoized 

```C++
template<typename Cache, typename Function>
memoize < Cache, Function > SpiceQL::Memo::make_memoized (
    Cache & fc,
    const std::string & id,
    Function f
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/memo.h`

