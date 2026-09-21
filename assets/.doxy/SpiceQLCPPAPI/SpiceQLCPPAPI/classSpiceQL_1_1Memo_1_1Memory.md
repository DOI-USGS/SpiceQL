

# Class SpiceQL::Memo::Memory



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Memo**](namespaceSpiceQL_1_1Memo.md) **>** [**Memory**](classSpiceQL_1_1Memo_1_1Memory.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::map&lt; std::size\_t, std::any &gt; | [**m\_data**](#variable-m_data)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  decltype(f(params...)) const | [**operator()**](#function-operator) (const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator_1) (std::string descr, const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator_2) (const std::string & descr, std::size\_t seed, const Func & f, Params &&... params) <br> |
|  decltype(f(params...)) const | [**operator()**](#function-operator_3) (std::size\_t seed, const Func & f, Params &&... params) <br> |




























## Public Attributes Documentation




### variable m\_data 

```C++
std::map<std::size_t, std::any> SpiceQL::Memo::Memory::m_data;
```




<hr>
## Public Functions Documentation




### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Memory::operator() (
    const Func & f,
    Params &&... params
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Memory::operator() (
    std::string descr,
    const Func & f,
    Params &&... params
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Memory::operator() (
    const std::string & descr,
    std::size_t seed,
    const Func & f,
    Params &&... params
) 
```




<hr>



### function operator() 

```C++
template<typename Func, typename... Params>
inline decltype(f(params...)) const SpiceQL::Memo::Memory::operator() (
    std::size_t seed,
    const Func & f,
    Params &&... params
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/memo.h`

