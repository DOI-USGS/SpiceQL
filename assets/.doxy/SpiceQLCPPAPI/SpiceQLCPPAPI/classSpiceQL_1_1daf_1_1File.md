

# Class SpiceQL::daf::File



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**daf**](namespaceSpiceQL_1_1daf.md) **>** [**File**](classSpiceQL_1_1daf_1_1File.md)



_An open DAF, read with plain file I/O._ [More...](#detailed-description)

* `#include <daf_reader.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**File**](#function-file) (const std::string & path) <br> |
|  std::string | [**kind**](#function-kind) () const<br> |
|  int | [**nd**](#function-nd) () const<br> |
|  int | [**ni**](#function-ni) () const<br> |
|  const std::string & | [**path**](#function-path) () const<br> |
|  std::vector&lt; double &gt; | [**read**](#function-read-12) (int64\_t begin, int64\_t end) const<br>_Reads double words [begin, end] (1-based, inclusive), like dafgda\_c._  |
|  double | [**read**](#function-read-22) (int64\_t addr) const<br> |
|  const std::vector&lt; [**Summary**](structSpiceQL_1_1daf_1_1Summary.md) &gt; & | [**summaries**](#function-summaries) () const<br> |




























## Detailed Description


Opening reads the file record and every summary. Array data is read on request, in bulk. 


    
## Public Functions Documentation




### function File 

```C++
inline explicit SpiceQL::daf::File::File (
    const std::string & path
) 
```




<hr>



### function kind 

```C++
inline std::string SpiceQL::daf::File::kind () const
```



The kernel type from the ID word ("CK", "SPK", "PCK", ...), or "" for old "NAIF/DAF" files. 


        

<hr>



### function nd 

```C++
inline int SpiceQL::daf::File::nd () const
```




<hr>



### function ni 

```C++
inline int SpiceQL::daf::File::ni () const
```




<hr>



### function path 

```C++
inline const std::string & SpiceQL::daf::File::path () const
```




<hr>



### function read [1/2]

_Reads double words [begin, end] (1-based, inclusive), like dafgda\_c._ 
```C++
inline std::vector< double > SpiceQL::daf::File::read (
    int64_t begin,
    int64_t end
) const
```




<hr>



### function read [2/2]

```C++
inline double SpiceQL::daf::File::read (
    int64_t addr
) const
```



Reads one double word. 


        

<hr>



### function summaries 

```C++
inline const std::vector< Summary > & SpiceQL::daf::File::summaries () const
```



Every array summary in the file, in file order. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/daf_reader.h`

