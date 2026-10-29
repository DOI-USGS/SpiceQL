

# Namespace SpiceQL::daf::detail



[**Namespace List**](namespaces.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**daf**](namespaceSpiceQL_1_1daf.md) **>** [**detail**](namespaceSpiceQL_1_1daf_1_1detail.md)










































## Public Functions

| Type | Name |
| ---: | :--- |
|  std::runtime\_error | [**badSegment**](#function-badsegment) (const [**File**](classSpiceQL_1_1daf_1_1File.md) & f, int type, const std::string & why) <br> |
|  int | [**nint**](#function-nint) (double d) <br> |
|  void | [**walkInterpolationIntervals**](#function-walkinterpolationintervals) (Tick && tick, size\_t ntick, const std::vector&lt; double &gt; & starts, Add add) <br> |




























## Public Functions Documentation




### function badSegment 

```C++
inline std::runtime_error SpiceQL::daf::detail::badSegment (
    const File & f,
    int type,
    const std::string & why
) 
```




<hr>



### function nint 

```C++
inline int SpiceQL::daf::detail::nint (
    double d
) 
```




<hr>



### function walkInterpolationIntervals 

```C++
template<class Tick, class Add>
void SpiceQL::daf::detail::walkInterpolationIntervals (
    Tick && tick,
    size_t ntick,
    const std::vector< double > & starts,
    Add add
) 
```



Shared walk of types 3 and 5: each interpolation interval runs from its start to the last record epoch before the next interval starts, and the last one runs to the last epoch. Gives the same intervals as the loop in zzckcv03 / zzckcv05. Calls add(begin, finish) per interval; finish is NAN when CSPICE would have used a value it never set.


zzckcv03 steps through the epochs one at a time. Epochs in a CK are required to increase, so this searches them instead, reading each from the file as it's needed. A segment with one interval, the common case, costs two reads however many records it has. The search gallops forward from the last interval's epochs before it bisects, so a segment with many intervals reads near where it already is rather than all over the epoch list.




**Parameters:**


* `tick` tick(i) is epoch i, for i in [0, ntick) 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/daf_reader.h`

