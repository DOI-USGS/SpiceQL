

# Class SpiceQL::CkSegment



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**CkSegment**](classSpiceQL_1_1CkSegment.md)



_C++ object repersenting NAIF spice CK Segment and it's metadata._ [More...](#detailed-description)

* `#include <io.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**CkSegment**](#function-cksegment) (std::vector&lt; std::vector&lt; double &gt; &gt; quats, std::vector&lt; double &gt; times, int bodyCode, std::string referenceFrame, std::string segmentId, std::vector&lt; std::vector&lt; double &gt; &gt; angularVelocities={}, std::string comment="") <br> |




























## Detailed Description


CK kernels consist of multiple CK segments. These specifically define a type 3 CK segment which consists of two parallel arrays of ephemeris times and orientations as SPICE quaternions.




**See also:** : [https://naif.jpl.nasa.gov/pub/naif/toolkit\_docs/C/cspice/q2m\_c.html](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/q2m_c.html) 



    
## Public Functions Documentation




### function CkSegment 

```C++
SpiceQL::CkSegment::CkSegment (
    std::vector< std::vector< double > > quats,
    std::vector< double > times,
    int bodyCode,
    std::string referenceFrame,
    std::string segmentId,
    std::vector< std::vector< double > > angularVelocities={},
    std::string comment=""
) 
```



Constructs a fully populated [**SpkSegment**](classSpiceQL_1_1SpkSegment.md) 

**Parameters:**


* `quats` Time ordered vector of orientations as quaternions 
* `times` times for the CK segment in ascending order 
* `bodyCode` Naif body code of an object whose state is described by the segments 
* `referenceFrame` Naif name of the reference system relative to which the state is 
* `segmentId` SPK segment identifier (max size 40) 
* `angularVelocities` Time ordered vector of state velocities dX, dY, dZ 
* `comment` The comment string for the new segment 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/io.h`

