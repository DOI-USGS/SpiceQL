

# File daf\_reader.h



[**FileList**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**daf\_reader.h**](daf__reader_8h.md)

[Go to the source code of this file](daf__reader_8h_source.md)

_Reads coverage straight out of binary SPK and CK files._ [More...](#detailed-description)

* `#include <algorithm>`
* `#include <cmath>`
* `#include <cstdint>`
* `#include <cerrno>`
* `#include <cstring>`
* `#include <stdexcept>`
* `#include <string>`
* `#include <utility>`
* `#include <vector>`
* `#include <fcntl.h>`
* `#include <sys/stat.h>`
* `#include <unistd.h>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**SpiceQL**](namespaceSpiceQL.md) <br> |
| namespace | [**daf**](namespaceSpiceQL_1_1daf.md) <br> |
| namespace | [**detail**](namespaceSpiceQL_1_1daf_1_1detail.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**File**](classSpiceQL_1_1daf_1_1File.md) <br>_An open DAF, read with plain file I/O._  |
| struct | [**Summary**](structSpiceQL_1_1daf_1_1Summary.md) <br> |


















































## Detailed Description


A self-contained reader for NAIF's DAF format that gets the array summaries (which hold SPK and CK segment bounds) and the per segment CK coverage ckcov\_c (INTERVAL level) would, without going through CSPICE. Nothing here touches the kernel pool, the DAF handle table or CSPICE's error state, so it can be used from several threads at once and needs no kernels furnished.


The one thing it can't do alone is turn CK times into ET. CK times are encoded SCLK ticks, and converting them needs the spacecraft's SCLK kernel, so CK coverage comes back in ticks for the caller to convert.


Layouts follow NAIF's DAF and CK Required Reading, and the per-type coverage rules are ports of CSPICE's zzckcv01 through zzckcv06, so the windows match ckcov\_c's exactly.


DAF layout in short. A file is 1024 byte records. Record 1 is the file record: ID word, ND and NI (doubles and integers per summary), and the first summary record. Summary records form a doubly linked list; each starts with NEXT, PREV and NSUM, then NSUM summaries of ND doubles followed by NI 32 bit integers packed into the remaining double words. Array data is addressed in 1-based double words from the start of the file. 


    

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/daf_reader.h`

