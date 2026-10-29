

# Namespace SpiceQL::daf



[**Namespace List**](namespaces.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**daf**](namespaceSpiceQL_1_1daf.md)


















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**detail**](namespaceSpiceQL_1_1daf_1_1detail.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**File**](classSpiceQL_1_1daf_1_1File.md) <br>_An open DAF, read with plain file I/O._  |
| struct | [**Summary**](structSpiceQL_1_1daf_1_1Summary.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::vector&lt; std::pair&lt; double, double &gt; &gt; | [**Window**](#typedef-window)  <br> |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**Window**](namespaceSpiceQL_1_1daf.md#typedef-window) | [**ckSegmentCoverage**](#function-cksegmentcoverage) (const [**File**](classSpiceQL_1_1daf_1_1File.md) & f, const [**Summary**](structSpiceQL_1_1daf_1_1Summary.md) & s) <br>_Coverage of one CK segment, in encoded SCLK ticks._  |
|  [**Window**](namespaceSpiceQL_1_1daf.md#typedef-window) | [**merge**](#function-merge) ([**Window**](namespaceSpiceQL_1_1daf.md#typedef-window) w) <br>_Sorts a window and merges intervals that overlap or touch._  |




























## Public Types Documentation




### typedef Window 

```C++
using SpiceQL::daf::Window = std::vector<std::pair<double, double>>;
```



A list of closed [start, stop] intervals. 


        

<hr>
## Public Functions Documentation




### function ckSegmentCoverage 

_Coverage of one CK segment, in encoded SCLK ticks._ 
```C++
inline Window SpiceQL::daf::ckSegmentCoverage (
    const File & f,
    const Summary & s
) 
```



The INTERVAL level coverage ckcov\_c would add for this segment, before merging: type 1 gives each epoch as a zero-length interval, type 2 each record's interval, types 3 and 5 each interpolation interval, type 4 each Chebyshev record's span and type 6 each mini-segment's interval. Types 5 and 6 are clipped to the segment's bounds, the others are not.




**Parameters:**


* `f` The open CK 
* `s` A summary from f, with ND = 2 and NI = 6 




        

<hr>



### function merge 

_Sorts a window and merges intervals that overlap or touch._ 
```C++
inline Window SpiceQL::daf::merge (
    Window w
) 
```



Gives the same window as inserting each interval with wninsd\_c, which is how spkcov\_c and ckcov\_c build theirs. 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/daf_reader.h`

