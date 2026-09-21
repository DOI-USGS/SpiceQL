

# Class SpiceQL::AliasMap



[**ClassList**](annotated.md) **>** [**SpiceQL**](namespaceSpiceQL.md) **>** [**AliasMap**](classSpiceQL_1_1AliasMap.md)










































## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**addAliasKey**](#function-addaliaskey) (const std::string & alias, const std::string & spiceqlName) <br>_Adds an alias to the lookup table for the given_ [_**SpiceQL**_](namespaceSpiceQL.md) _name._ |
|  nlohmann::json | [**getAliasMap**](#function-getaliasmap) () <br>_Retrieves the alias map lookup table as JSON object._  |
|  std::string | [**getSpiceqlName**](#function-getspiceqlname) (const std::string & name) <br>_Gets_ [_**SpiceQL**_](namespaceSpiceQL.md) _name given alias or frame name._ |
|  void | [**load**](#function-load) (std::string path) <br>_Loads an alias map from a JSON file path._  |
|  void | [**setAliasMap**](#function-setaliasmap) (const nlohmann::json & newAliasMap) <br>_Overwrites the current lookup table with a user-provided JSON object._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  AliasMap & | [**instance**](#function-instance) () <br>_Accessor for_ [_**AliasMap**_](classSpiceQL_1_1AliasMap.md) _instance._ |


























## Public Functions Documentation




### function addAliasKey 

_Adds an alias to the lookup table for the given_ [_**SpiceQL**_](namespaceSpiceQL.md) _name._
```C++
void SpiceQL::AliasMap::addAliasKey (
    const std::string & alias,
    const std::string & spiceqlName
) 
```





**Parameters:**


* `alias` Alias string for [**SpiceQL**](namespaceSpiceQL.md) name 
* `spiceqlName` The target [**SpiceQL**](namespaceSpiceQL.md) name 




        

<hr>



### function getAliasMap 

_Retrieves the alias map lookup table as JSON object._ 
```C++
nlohmann::json SpiceQL::AliasMap::getAliasMap () 
```





**Returns:**

A JSON object where keys are [**SpiceQL**](namespaceSpiceQL.md) names and the values are an array of aliases 





        

<hr>



### function getSpiceqlName 

_Gets_ [_**SpiceQL**_](namespaceSpiceQL.md) _name given alias or frame name._
```C++
std::string SpiceQL::AliasMap::getSpiceqlName (
    const std::string & name
) 
```





**Parameters:**


* `name` Given string name to look up 



**Returns:**

The [**SpiceQL**](namespaceSpiceQL.md) name or empty string if not found 





        

<hr>



### function load 

_Loads an alias map from a JSON file path._ 
```C++
void SpiceQL::AliasMap::load (
    std::string path
) 
```





**Parameters:**


* `path` Filesystem path to the JSON file 




        

<hr>



### function setAliasMap 

_Overwrites the current lookup table with a user-provided JSON object._ 
```C++
void SpiceQL::AliasMap::setAliasMap (
    const nlohmann::json & newAliasMap
) 
```





**Parameters:**


* `newAliasMap` The alias map as a JSON object 




        

<hr>
## Public Static Functions Documentation




### function instance 

_Accessor for_ [_**AliasMap**_](classSpiceQL_1_1AliasMap.md) _instance._
```C++
static AliasMap & SpiceQL::AliasMap::instance () 
```





**Returns:**

A reference to the global [**AliasMap**](classSpiceQL_1_1AliasMap.md) instance 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/alias_map.h`

