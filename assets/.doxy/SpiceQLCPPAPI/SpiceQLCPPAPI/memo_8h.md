

# File memo.h



[**FileList**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**memo.h**](memo_8h.md)

[Go to the source code of this file](memo_8h_source.md)



* `#include <map>`
* `#include <fstream>`
* `#include <utility>`
* `#include <functional>`
* `#include <any>`
* `#include <string>`
* `#include <chrono>`
* `#include <iomanip>`
* `#include <ghc/fs_std.hpp>`
* `#include <SpiceQL/spiceql_logging.h>`
* `#include <cereal/archives/binary.hpp>`
* `#include <cereal/archives/portable_binary.hpp>`
* `#include <cereal/types/vector.hpp>`
* `#include <cereal/types/string.hpp>`
* `#include <cereal/types/utility.hpp>`
* `#include <cereal/archives/json.hpp>`
* `#include <SpiceQL/memoized_functions.h>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**SpiceQL**](namespaceSpiceQL.md) <br> |
| namespace | [**Memo**](namespaceSpiceQL_1_1Memo.md) <br> |
| namespace | [**std**](namespacestd.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**Cache**](classSpiceQL_1_1Memo_1_1Cache.md) <br> |
| class | [**Memory**](classSpiceQL_1_1Memo_1_1Memory.md) <br> |
| struct | [**memoize**](structSpiceQL_1_1Memo_1_1memoize.md) &lt;typename [**Cache**](classSpiceQL_1_1Memo_1_1Cache.md), typename Function&gt;<br> |
| struct | [**hash&lt; std::vector&lt; std::string &gt; &gt;**](structstd_1_1hash_3_01std_1_1vector_3_01std_1_1string_01_4_01_4.md) &lt;&gt;<br> |
| struct | [**hash&lt; std::vector&lt; std::string &gt;(const std::string &, bool)&gt;**](structstd_1_1hash_3_01std_1_1vector_3_01std_1_1string_01_4_07const_01std_1_1string_01_6_00_01bool_08_4.md) &lt;&gt;<br> |

















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**CACHED**](memo_8h.md#define-cached) (cache, func, ...) `cache(#func, func, \_\_VA\_ARGS\_\_)`<br> |

## Macro Definition Documentation





### define CACHED 

```C++
#define CACHED (
    cache,
    func,
    ...
) `cache(#func, func, __VA_ARGS__)`
```



Implements an almost transparent disk cache/memoization for C++ functions.


Originally adapted from: [https://github.com/temporaer/cached\_function](https://github.com/temporaer/cached_function) 


        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/memo.h`

