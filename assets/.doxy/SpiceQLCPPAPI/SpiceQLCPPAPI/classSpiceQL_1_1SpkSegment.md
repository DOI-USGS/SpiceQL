

# Class SpiceQL::SpkSegment



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**SpkSegment**](classSpiceQL_1_1SpkSegment.md)



_C++ object repersenting NAIF spice SPK Segment and it's metadata._ [More...](#detailed-description)

* `#include <io.h>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**SpkSegment**](#function-spksegment) (std::vector&lt; std::vector&lt; double &gt; &gt; statePositions, std::vector&lt; double &gt; stateTimes, int bodyCode, int centerOfMotion, std::string referenceFrame, std::string segmentId, int degree, std::vector&lt; std::vector&lt; double &gt; &gt; stateVelocities={}, std::string segmentComment="") <br> |




























## Detailed Description


SPK kernels consist of multiple CK segments. These specifically define a type 13 SPK segment which consists of parallel arrary of ephemeris times in a 6 element state array's of x, y, z, vx, vy, vz 


    
## Public Functions Documentation




### function SpkSegment 

```C++
SpiceQL::SpkSegment::SpkSegment (
    std::vector< std::vector< double > > statePositions,
    std::vector< double > stateTimes,
    int bodyCode,
    int centerOfMotion,
    std::string referenceFrame,
    std::string segmentId,
    int degree,
    std::vector< std::vector< double > > stateVelocities={},
    std::string segmentComment=""
) 
```



Constructs a fully populated [**SpkSegment**](classSpiceQL_1_1SpkSegment.md)




**Parameters:**


* `statePositions` Time ordered vector of state positions X,Y,Z 
* `stateTimes` Time ordered vector of state ephemeris times (TDB) 
* `bodyCode` Naif body code of an object whose state is described by the segments 
* `centerOfMotion` Naif body code of an object which is the center of motion for bodyCode 
* `referenceFrame` Naif name of the reference system relative to which the state is 
* `segmentId` SPK segment identifier (max size 40) 
* `degree` Degree of the Hermite polynomials used to interpolate the states 
* `stateVelocities` Time ordered vector of state velocities dX, dY, dZ 
* `segmentComment` The comment string for the new segment 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/io.h`

