

# Struct SpiceQL::Memo::memoize

**template &lt;typename [**Cache**](classSpiceQL_1_1Memo_1_1Cache.md), typename Function&gt;**



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**Memo**](namespaceSpiceQL_1_1Memo.md) **>** [**memoize**](structSpiceQL_1_1Memo_1_1memoize.md)


























## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Cache**](classSpiceQL_1_1Memo_1_1Cache.md) | [**m\_fc**](#variable-m_fc)  <br> |
|  const Function | [**m\_func**](#variable-m_func)  <br> |
|  std::string | [**m\_id**](#variable-m_id)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**memoize**](#function-memoize) ([**Cache**](classSpiceQL_1_1Memo_1_1Cache.md) & fc, std::string id, const Function & f) <br> |
|  decltype(std::bind(m\_func, args...)()) | [**operator()**](#function-operator) (Params &&... args) <br> |




























## Public Attributes Documentation




### variable m\_fc 

```C++
Cache SpiceQL::Memo::memoize< Cache, Function >::m_fc;
```




<hr>



### variable m\_func 

```C++
const Function SpiceQL::Memo::memoize< Cache, Function >::m_func;
```




<hr>



### variable m\_id 

```C++
std::string SpiceQL::Memo::memoize< Cache, Function >::m_id;
```




<hr>
## Public Functions Documentation




### function memoize 

```C++
inline SpiceQL::Memo::memoize::memoize (
    Cache & fc,
    std::string id,
    const Function & f
) 
```




<hr>



### function operator() 

```C++
template<typename... Params>
inline decltype(std::bind(m_func, args...)()) SpiceQL::Memo::memoize::operator() (
    Params &&... args
) 
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/memo.h`

