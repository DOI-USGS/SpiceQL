

# Class SpiceQL::Memo::Cache



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Memo**](namespaceSpiceQL_1_1Memo.md) **>** [**Cache**](classSpiceQL_1_1Memo_1_1Cache.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; std::string &gt; | [**m\_dependants**](#variable-m_dependants)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**Cache**](#function-cache) (std::vector&lt; std::string &gt; deps) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator) (const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator_1) (const std::string & descr, const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator_2) (const std::string & descr, std::size\_t seed, const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**use\_disk\_cache**](#function-use_disk_cache) (const std::string & descr, std::size\_t seed, const Func & f, Params &&... params) <br> |




























## Public Attributes Documentation




### variable m\_dependants 

```C++
std::vector<std::string> SpiceQL::Memo::Cache::m_dependants;
```




<hr>
## Public Functions Documentation




### function Cache 

```C++
inline SpiceQL::Memo::Cache::Cache (
    std::vector< std::string > deps
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Cache::operator() (
    const Func & f,
    Params &&... params
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Cache::operator() (
    const std::string & descr,
    const Func & f,
    Params &&... params
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Cache::operator() (
    const std::string & descr,
    std::size_t seed,
    const Func & f,
    Params &&... params
) 
```




<hr>



### function use\_disk\_cache 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Cache::use_disk_cache (
    const std::string & descr,
    std::size_t seed,
    const Func & f,
    Params &&... params
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/memo.h`

