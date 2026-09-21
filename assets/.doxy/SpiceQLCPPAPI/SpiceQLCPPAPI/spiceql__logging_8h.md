

# File spiceql\_logging.h



[**FileList**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**spiceql\_logging.h**](spiceql__logging_8h.md)

[Go to the source code of this file](spiceql__logging_8h_source.md)



* `#include <fmt/format.h>`
* `#include <fmt/ranges.h>`
* `#include <cstdlib>`
* `#include <iostream>`
* `#include <string>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**SpiceQL**](namespaceSpiceQL.md) <br> |
| namespace | [**spdlog**](namespacespdlog.md) <br> |



















































## Macros

| Type | Name |
| ---: | :--- |
| define  | [**SPDLOG\_CRITICAL**](spiceql__logging_8h.md#define-spdlog_critical) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Critical, \_\_VA\_ARGS\_\_)`<br> |
| define  | [**SPDLOG\_DEBUG**](spiceql__logging_8h.md#define-spdlog_debug) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Debug,    \_\_VA\_ARGS\_\_)`<br> |
| define  | [**SPDLOG\_ERROR**](spiceql__logging_8h.md#define-spdlog_error) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Error,    \_\_VA\_ARGS\_\_)`<br> |
| define  | [**SPDLOG\_INFO**](spiceql__logging_8h.md#define-spdlog_info) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Info,     \_\_VA\_ARGS\_\_)`<br> |
| define  | [**SPDLOG\_TRACE**](spiceql__logging_8h.md#define-spdlog_trace) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Trace,    \_\_VA\_ARGS\_\_)`<br> |
| define  | [**SPDLOG\_WARN**](spiceql__logging_8h.md#define-spdlog_warn) (...) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Warn,     \_\_VA\_ARGS\_\_)`<br> |

## Macro Definition Documentation





### define SPDLOG\_CRITICAL 

```C++
#define SPDLOG_CRITICAL (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Critical, __VA_ARGS__)`
```




<hr>



### define SPDLOG\_DEBUG 

```C++
#define SPDLOG_DEBUG (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Debug,    __VA_ARGS__)`
```




<hr>



### define SPDLOG\_ERROR 

```C++
#define SPDLOG_ERROR (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Error,    __VA_ARGS__)`
```




<hr>



### define SPDLOG\_INFO 

```C++
#define SPDLOG_INFO (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Info,     __VA_ARGS__)`
```




<hr>



### define SPDLOG\_TRACE 

```C++
#define SPDLOG_TRACE (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Trace,    __VA_ARGS__)`
```




<hr>



### define SPDLOG\_WARN 

```C++
#define SPDLOG_WARN (
    ...
) `::SpiceQL::logMessage(::SpiceQL::LogLevel::Warn,     __VA_ARGS__)`
```




<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/spiceql_logging.h`

