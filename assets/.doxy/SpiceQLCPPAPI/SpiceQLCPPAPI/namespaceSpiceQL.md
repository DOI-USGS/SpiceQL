

# Namespace SpiceQL



[**Namespace List**](namespaces.md) **>** [**SpiceQL**](namespaceSpiceQL.md)



[More...](#detailed-description)














## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**Inventory**](namespaceSpiceQL_1_1Inventory.md) <br> |
| namespace | [**Memo**](namespaceSpiceQL_1_1Memo.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**AliasMap**](classSpiceQL_1_1AliasMap.md) <br> |
| class | [**CkSegment**](classSpiceQL_1_1CkSegment.md) <br>_C++ object repersenting NAIF spice CK Segment and it's metadata._  |
| class | [**Config**](classSpiceQL_1_1Config.md) <br>_Object for interacting with_ [_**SpiceQL**_](namespaceSpiceQL.md) _target configs._ |
| class | [**InventoryImpl**](classSpiceQL_1_1InventoryImpl.md) <br> |
| class | [**Kernel**](classSpiceQL_1_1Kernel.md) <br>_Base_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _class._ |
| class | [**KernelSet**](classSpiceQL_1_1KernelSet.md) <br>_Class for furnishing kernels in bulk._  |
| class | [**SpkSegment**](classSpiceQL_1_1SpkSegment.md) <br>_C++ object repersenting NAIF spice SPK Segment and it's metadata._  |
| class | [**TimeIndexedKernels**](classSpiceQL_1_1TimeIndexedKernels.md) <br> |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**LogLevel**](#enum-loglevel)  <br> |




## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::string | [**DB\_FRAME\_CACHE\_KEY**](#variable-db_frame_cache_key)  <br> |
|  std::string | [**DB\_FRAME\_CODES\_KEY**](#variable-db_frame_codes_key)  <br> |
|  std::string | [**DB\_FRAME\_LIST\_KEY**](#variable-db_frame_list_key)  <br> |
|  std::string | [**DB\_FRAME\_NAMES\_KEY**](#variable-db_frame_names_key)  <br> |
|  std::string | [**DB\_HDF\_FILE**](#variable-db_hdf_file)  <br> |
|  std::string | [**DB\_SPICE\_ROOT\_KEY**](#variable-db_spice_root_key)  <br> |
|  std::string | [**DB\_SS\_TIME\_INDICES\_KEY**](#variable-db_ss_time_indices_key)  <br> |
|  std::string | [**DB\_START\_TIME\_KEY**](#variable-db_start_time_key)  <br> |
|  std::string | [**DB\_STOP\_TIME\_KEY**](#variable-db_stop_time_key)  <br> |
|  std::string | [**DB\_TIME\_FILES\_KEY**](#variable-db_time_files_key)  <br> |
|  const std::vector&lt; std::string &gt; | [**KERNEL\_QUALITIES**](#variable-kernel_qualities)  <br> |
|  const std::vector&lt; std::string &gt; | [**KERNEL\_TYPES**](#variable-kernel_types)  <br> |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**addAliasKey**](#function-addaliaskey) (const std::string & key, const std::string & value) <br>_Adds or updates a key-value pair in the aliasMap._  |
|  bool | [**checkNaifErrors**](#function-checknaiferrors) (bool reset=true) <br>_raises a C++ exception if NAIF has an error buffered._  |
|  std::vector&lt; std::vector&lt; double &gt; &gt; | [**concatStates**](#function-concatstates) (std::vector&lt; std::vector&lt; double &gt; &gt; statePositions, std::vector&lt; std::vector&lt; double &gt; &gt; stateVelocities) <br> |
|  LogLevel | [**currentLogLevel**](#function-currentloglevel) () <br> |
|  std::pair&lt; std::string, nlohmann::json &gt; | [**doubleEtToSclk**](#function-doubleettosclk) (int frameCode, double et, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Converts a given double spacecraft clock time to an ephemeris time._  |
|  std::pair&lt; std::vector&lt; double &gt;, nlohmann::json &gt; | [**doubleEtsToSclkTicks**](#function-doubleetstosclkticks) (int frameCode, std::vector&lt; double &gt; ets, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Converts ephemeris times to encoded SCLK "ticks" (doubles)._  |
|  std::pair&lt; double, nlohmann::json &gt; | [**doubleSclkToEt**](#function-doublesclktoet) (int frameCode, double sclk, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Converts a given double spacecraft clock time to an ephemeris time._  |
|  size\_t | [**eraseAtPointer**](#function-eraseatpointer) (nlohmann::json & j, nlohmann::json::json\_pointer ptr) <br>_erase a part of a json object based on a json pointer_  |
|  std::pair&lt; std::string, nlohmann::json &gt; | [**etToUtc**](#function-ettoutc) (double et, std::string format="", double precision=0, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_convert et string to a UTC string_  |
|  std::pair&lt; std::vector&lt; double &gt;, nlohmann::json &gt; | [**extractExactCkTimes**](#function-extractexactcktimes) (double observStart, double observEnd, int targetFrame, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Extracts all segment times between observStart and observeEnd._  |
|  std::vector&lt; nlohmann::json::json\_pointer &gt; | [**findKeyInJson**](#function-findkeyinjson) (nlohmann::json in, std::string key, bool recursive=true) <br>_recursively search keys in json._  |
|  nlohmann::json | [**findKeywords**](#function-findkeywords) (std::string keytpl) <br>_finds key:values in kernel pool_  |
|  std::pair&lt; nlohmann::json, nlohmann::json &gt; | [**findMissionKeywords**](#function-findmissionkeywords) (std::string key, std::string mission, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_returns kernel text keyword values for a specific mission as json_  |
|  std::pair&lt; nlohmann::json, nlohmann::json &gt; | [**findTargetKeywords**](#function-findtargetkeywords) (std::string key, std::string mission, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_returns Target values in the form of a vector_  |
|  nlohmann::json | [**formatKernels**](#function-formatkernels) (std::vector&lt; std::string &gt; kernelPaths) <br>_Format a list of kernel paths into a kernels JSON object organized by type._  |
|  std::vector&lt; std::string &gt; | [**frameList**](#function-framelist) () <br>_Get a list of all frames in the supported by the library._  |
|  std::pair&lt; std::vector&lt; std::vector&lt; int &gt; &gt;, nlohmann::json &gt; | [**frameTrace**](#function-frametrace) (double et, int initialFrame, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Given an ephemeris time and a starting frame, find the path from that starting frame to J2000 (1)._  |
|  std::string | [**gen\_random**](#function-gen_random) (const int len) <br>_generate a random string_  |
|  nlohmann::json | [**getAliasMap**](#function-getaliasmap) () <br>_Accessor for the aliasMap._  |
|  std::string | [**getAliasMapJsonFile**](#function-getaliasmapjsonfile) () <br>_Gets default path for JSON file of aliases._  |
|  std::vector&lt; std::string &gt; | [**getAvailableConfigFiles**](#function-getavailableconfigfiles) () <br>_Returns a vector of all the available configs._  |
|  std::vector&lt; nlohmann::json &gt; | [**getAvailableConfigs**](#function-getavailableconfigs) () <br>_Get names of available config files as a json vector._  |
|  std::string | [**getCacheDir**](#function-getcachedir) () <br> |
|  std::string | [**getConfigDirectory**](#function-getconfigdirectory) () <br>_Get the directory pointing to the db files._  |
|  std::string | [**getDataDirectory**](#function-getdatadirectory) () <br>_Returns std::vector&lt;string&gt; interpretation of a json array._  |
|  std::string | [**getDefaultLsk**](#function-getdefaultlsk) () <br>_Load the local LSK file that comes installed with_ [_**SpiceQL**_](namespaceSpiceQL.md) _._ |
|  std::pair&lt; std::vector&lt; std::vector&lt; double &gt; &gt;, nlohmann::json &gt; | [**getExactTargetOrientations**](#function-getexacttargetorientations) (double startEt, double stopEt, int toFrame, int refFrame, int exactCkFrame, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Returns exact target orientations for given time intervals and parameters._  |
|  std::pair&lt; std::vector&lt; int &gt;, nlohmann::json &gt; | [**getFrameInfo**](#function-getframeinfo) (int frame, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Get the center, class id, and class of a given frame._  |
|  std::string | [**getHdfFile**](#function-gethdffile) () <br> |
|  std::pair&lt; double, double &gt; | [**getKernelStartStopTimes**](#function-getkernelstartstoptimes) (std::string kpath) <br> |
|  std::string | [**getKernelStringValue**](#function-getkernelstringvalue) (std::string key) <br>_return's kernel value from key_  |
|  std::string | [**getKernelType**](#function-getkerneltype) (std::string kernelPath) <br>_get the_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type (CK, SPK, etc.)_ |
|  std::vector&lt; std::string &gt; | [**getKernelVectorValue**](#function-getkernelvectorvalue) (std::string key) <br>_return's kernel values in the form of a vector_  |
|  std::set&lt; std::string &gt; | [**getKernelsAsSet**](#function-getkernelsasset) (nlohmann::json kernels) <br>_Get all the kernels in the json as a set._  |
|  std::vector&lt; std::string &gt; | [**getKernelsAsVector**](#function-getkernelsasvector) (nlohmann::json kernels) <br>_Get all the kernels in the json as a vector._  |
|  std::vector&lt; std::string &gt; | [**getLatestKernel**](#function-getlatestkernel) (std::vector&lt; std::string &gt; kernels) <br>_get the latest kernel in a list_  |
|  nlohmann::json | [**getLatestKernels**](#function-getlatestkernels) (nlohmann::json kernels) <br>_returns a JSON object of only the latest version of each kernel type_  |
|  std::vector&lt; std::string &gt; | [**getLoadedKernels**](#function-getloadedkernels) () <br>_Returns the file paths of all kernels currently loaded in the CSPICE pool._  |
|  nlohmann::json | [**getMissionConfig**](#function-getmissionconfig) (std::string mission) <br>_Returns the path to the Mission specific Spice config file._  |
|  std::string | [**getMissionConfigFile**](#function-getmissionconfigfile) (std::string mission) <br>_Returns the path to the Mission specific Spice config file._  |
|  std::string | [**getMissionKeys**](#function-getmissionkeys) (nlohmann::json config) <br>_Returns a string of mission keys from the JSON config file._  |
|  std::vector&lt; std::vector&lt; std::string &gt; &gt; | [**getPathsFromRegex**](#function-getpathsfromregex) (std::string root, std::vector&lt; std::string &gt; regexes) <br>_glob, but with json_  |
|  std::string | [**getRestUrl**](#function-getresturl) () <br>_Returns the REST URL._  |
|  std::string | [**getRootDependency**](#function-getrootdependency) (nlohmann::json config, std::string pointer) <br>_Returns the root most dependency for a json pointer._  |
|  std::string | [**getSpiceqlName**](#function-getspiceqlname) (const std::string & name) <br>_Translates a given name using the aliasMap and checks if the name is in the frameList._  |
|  std::pair&lt; nlohmann::json, nlohmann::json &gt; | [**getTargetFrameInfo**](#function-gettargetframeinfo) (int targetId, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_returns frame name and frame code associated to the target ID._  |
|  std::vector&lt; double &gt; | [**getTargetOrientation**](#function-gettargetorientation) (double et, int toFrame, int refFrame=1) <br>_Gives quaternion and angular velocity for a given frame at a given ephemeris time._  |
|  std::pair&lt; std::vector&lt; std::vector&lt; double &gt; &gt;, nlohmann::json &gt; | [**getTargetOrientations**](#function-gettargetorientations) (std::vector&lt; double &gt; ets, int toFrame, int refFrame, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Gives quaternion and angular velocity for a given frame at a set of ephemeris times._  |
|  std::pair&lt; std::vector&lt; std::vector&lt; double &gt; &gt;, nlohmann::json &gt; | [**getTargetOrientationsRanged**](#function-gettargetorientationsranged) (double startEt, double stopEt, int numRecords, int toFrame, int refFrame, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Gives quaternion and angular velocity for a given frame at a set of ephemeris times._  |
|  std::vector&lt; double &gt; | [**getTargetState**](#function-gettargetstate) (double et, std::string target, std::string observer, std::string frame="J2000", std::string abcorr="NONE") <br>_Gives the position and velocity for a given frame at some ephemeris time._  |
|  std::pair&lt; std::vector&lt; std::vector&lt; double &gt; &gt;, nlohmann::json &gt; | [**getTargetStates**](#function-gettargetstates) (std::vector&lt; double &gt; ets, std::string target, std::string observer, std::string frame, std::string abcorr, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Gives the positions and velocities for a given frame given a set of ephemeris times._  |
|  std::pair&lt; std::vector&lt; std::vector&lt; double &gt; &gt;, nlohmann::json &gt; | [**getTargetStatesRanged**](#function-gettargetstatesranged) (double startEt, double stopEt, int numRecords, std::string target, std::string observer, std::string frame, std::string abcorr, std::string mission="", std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Gives the positions and velocities for a given start and stop ephemeris times and number of records._  |
|  std::vector&lt; std::pair&lt; double, double &gt; &gt; | [**getTimeIntervals**](#function-gettimeintervals) (std::string kpath) <br>_Get start and stop times a kernel._  |
|  std::vector&lt; std::string &gt; | [**glob**](#function-glob) (std::string const & root, std::string const & reg=".\*", bool recursive=false) <br>_glob, like python's glob.glob, except C++_  |
|  std::string | [**globKernelStartStopTimes**](#function-globkernelstartstoptimes) (std::string mission) <br> |
|  nlohmann::json | [**globKernels**](#function-globkernels) (std::string root, nlohmann::json conf, std::string kernelType) <br>_acquire all kernels of a type according to a configuration JSON object_  |
|  std::string | [**globTimeIntervals**](#function-globtimeintervals) (std::string mission) <br>_Get start and stop times for all kernels._  |
|  std::string | [**inferMission**](#function-infermission) (const std::vector&lt; std::string &gt; & nameCandidates, const std::vector&lt; int &gt; & codeCandidates) <br>_Infer the_ [_**SpiceQL**_](namespaceSpiceQL.md) _config (mission) name from identifying inputs._ |
|  bool | [**isLskLoaded**](#function-islskloaded) () <br>_Returns true if a leapseconds kernel (LSK) is loaded in the CSPICE pool._  |
|  std::vector&lt; std::vector&lt; std::string &gt; &gt; | [**json2DArrayTo2DVector**](#function-json2darrayto2dvector) (nlohmann::json arr) <br>_Returns std::vector&lt;std::vector&lt;string&gt;&gt; interpretation of a json array._  |
|  std::vector&lt; std::pair&lt; double, double &gt; &gt; | [**json2DArrayToDoublePair**](#function-json2darraytodoublepair) (nlohmann::json arr) <br>_Returns std::vector&lt;std::vector&lt;string&gt;&gt; interpretation of a json array._  |
|  std::vector&lt; std::vector&lt; double &gt; &gt; | [**json2DFloatArrayTo2DVector**](#function-json2dfloatarrayto2dvector) (nlohmann::json arr) <br>_Returns std::vector&lt;std::vector&lt;float&gt;&gt; interpretation of a json array._  |
|  std::vector&lt; std::vector&lt; int &gt; &gt; | [**json2DIntArrayTo2DVector**](#function-json2dintarrayto2dvector) (nlohmann::json arr, bool retainEmpty=false) <br>_Returns std::vector&lt;std::vector&lt;int&gt;&gt; interpretation of a json array._  |
|  std::vector&lt; std::string &gt; | [**jsonArrayToVector**](#function-jsonarraytovector) (nlohmann::json arr) <br>_Returns std::vector&lt;string&gt; interpretation of a json array._  |
|  std::vector&lt; double &gt; | [**jsonDoubleArrayToVector**](#function-jsondoublearraytovector) (nlohmann::json arr) <br>_Returns std::vector&lt;double&gt; interpretation of a json array._  |
|  std::vector&lt; int &gt; | [**jsonIntArrayToVector**](#function-jsonintarraytovector) (nlohmann::json arr) <br>_Returns std::vector&lt;int&gt; interpretation of a json array._  |
|  nlohmann::json | [**listMissionKernels**](#function-listmissionkernels) (std::string root, nlohmann::json conf) <br>_Returns all kernels available for a mission._  |
|  void | [**load**](#function-load) (std::string path, bool force\_refurnsh=true) <br> |
|  nlohmann::json | [**loadSelectKernels**](#function-loadselectkernels) (std::string kernelType, std::string mission) <br>_Loads PCK kernels associated to mission name._  |
|  nlohmann::json | [**loadTranslationKernels**](#function-loadtranslationkernels) (std::string mission, bool loadFk=true, bool loadIk=true, bool loadIak=true) <br>_Loads translation kernels (fk, ik, and iaks) associated to mission name._  |
|  void | [**load\_aliases**](#function-load_aliases) (std::string path="") <br>_Free function to trigger loading aliases._  |
|  const char \* | [**logLevelName**](#function-loglevelname) (LogLevel lvl) <br> |
|  void | [**logMessage**](#function-logmessage) (LogLevel lvl, fmt::string\_view f, Args &&... args) <br> |
|  std::vector&lt; std::string &gt; | [**ls**](#function-ls) (std::string const & root, bool recursive) <br>_ls, like in unix, kinda. Also it's a function._  |
|  void | [**mergeConfigs**](#function-mergeconfigs) (nlohmann::json & baseConfig, const nlohmann::json & mergingConfig) <br>_Merge two json configs._  |
|  nlohmann::json | [**merge\_json**](#function-merge_json) (nlohmann::json & j1, nlohmann::json & j2, bool overwrite=false) <br>_Merges the right json to the left json._  |
|  LogLevel | [**parseLogLevel**](#function-parseloglevel) (const std::string & s) <br> |
|  std::string | [**replaceAll**](#function-replaceall) (std::string str, const std::string & from, const std::string & to) <br>_find and replace one substring with another_  |
|  void | [**resolveConfigDependencies**](#function-resolveconfigdependencies) (nlohmann::json & config, const nlohmann::json & dependencies) <br>_resolve the dependencies in a config in place_  |
|  std::pair&lt; std::string, nlohmann::json &gt; | [**searchForKernelsets**](#function-searchforkernelsets) (std::vector&lt; std::string &gt; spiceqlNames, std::vector&lt; std::string &gt; types={"ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"}, double startTime=-std::numeric\_limits&lt; double &gt;::max(), double stopTime=std::numeric\_limits&lt; double &gt;::max(), std::vector&lt; std::string &gt; ckQualities={"smithed", "reconstructed"}, std::vector&lt; std::string &gt; spkQualities={"smithed", "reconstructed"}, bool useWeb=false, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, bool overwrite=false) <br>_Searches for kernels given mission(s) and parameters._  |
|  void | [**setAliasMap**](#function-setaliasmap) (const nlohmann::json & newAliasMap) <br>_Setter for the aliasMap._  |
|  void | [**setCacheDir**](#function-setcachedir) (std::string cache\_dir, bool override=false) <br> |
|  void | [**setConfigDirectory**](#function-setconfigdirectory) (std::string configDir) <br>_Override the directory pointing to the db files._  |
|  nlohmann::json | [**spiceAPIQuery**](#function-spiceapiquery) (std::string functionName, nlohmann::json args, std::string method="GET") <br>_Query implementation for_ [_**SpiceQL**_](namespaceSpiceQL.md) _'s REST API._ |
|  std::vector&lt; std::string &gt; | [**split**](#function-split) (const std::string & s, char delim) <br>_turn a string into a vector with a deliminator_  |
|  std::pair&lt; double, nlohmann::json &gt; | [**strSclkToEt**](#function-strsclktoet) (int frameCode, std::string sclk, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Converts a given string spacecraft clock time to an ephemeris time._  |
|  std::string | [**toLower**](#function-tolower) (std::string s) <br>_force a string to lower case_  |
|  std::string | [**toUpper**](#function-toupper) (std::string s) <br>_force a string to upper case_  |
|  std::pair&lt; std::string, nlohmann::json &gt; | [**translateCodeToName**](#function-translatecodetoname) (int frame, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Switch between NAIF frame integer code to string frame name._  |
|  std::pair&lt; int, nlohmann::json &gt; | [**translateNameToCode**](#function-translatenametocode) (std::string frame, std::string mission="", bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_Switch between NAIF frame string name to integer frame code._  |
|  void | [**unload**](#function-unload) (std::string path) <br> |
|  std::string | [**url\_encode**](#function-url_encode) (const std::string & value) <br>_URL encodes a given string._  |
|  std::pair&lt; double, nlohmann::json &gt; | [**utcToEt**](#function-utctoet) (std::string utc, bool useWeb=false, bool searchKernels=true, bool fullKernelPath=false, int limitCk=-1, int limitSpk=1, std::vector&lt; std::string &gt; kernelList={}) <br>_convert a UTC string to an ephemeris time_  |
|  void | [**writeCk**](#function-writeck) (std::string fileName, std::vector&lt; std::vector&lt; double &gt; &gt; quats, std::vector&lt; double &gt; times, int bodyCode, std::string referenceFrame, std::string segmentId, std::vector&lt; std::vector&lt; double &gt; &gt; angularVelocity={}, std::string comment="") <br>_Write CK segments to a file._  |
|  void | [**writeCk**](#function-writeck) (std::string fileName, std::vector&lt; [**CkSegment**](classSpiceQL_1_1CkSegment.md) &gt; segments) <br>_Write CK segments to a file._  |
|  void | [**writeComment**](#function-writecomment) (std::string fileName, std::string comment) <br> |
|  void | [**writeSpk**](#function-writespk) (std::string fileName, std::vector&lt; [**SpkSegment**](classSpiceQL_1_1SpkSegment.md) &gt; segments) <br>_Write SPK segments to a file._  |
|  void | [**writeSpk**](#function-writespk) (std::string fileName, std::vector&lt; std::vector&lt; double &gt; &gt; statePositions, std::vector&lt; double &gt; stateTimes, int bodyCode, int centerOfMotion, std::string referenceFrame, std::string segmentId, int polyDegree, std::vector&lt; std::vector&lt; double &gt; &gt; stateVelocities={}, std::string segmentComment="") <br>_Write SPK to path._  |
|  void | [**writeTextKernel**](#function-writetextkernel) (std::string fileName, std::string type, nlohmann::json & keywords, std::string comment="") <br>_Write json key value pairs into a NAIF text kernel._  |




























## Detailed Description


types 


    
## Public Types Documentation




### enum LogLevel 

```C++
enum SpiceQL::LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off
};
```




<hr>
## Public Attributes Documentation




### variable DB\_FRAME\_CACHE\_KEY 

```C++
std::string SpiceQL::DB_FRAME_CACHE_KEY;
```




<hr>



### variable DB\_FRAME\_CODES\_KEY 

```C++
std::string SpiceQL::DB_FRAME_CODES_KEY;
```




<hr>



### variable DB\_FRAME\_LIST\_KEY 

```C++
std::string SpiceQL::DB_FRAME_LIST_KEY;
```




<hr>



### variable DB\_FRAME\_NAMES\_KEY 

```C++
std::string SpiceQL::DB_FRAME_NAMES_KEY;
```




<hr>



### variable DB\_HDF\_FILE 

```C++
std::string SpiceQL::DB_HDF_FILE;
```




<hr>



### variable DB\_SPICE\_ROOT\_KEY 

```C++
std::string SpiceQL::DB_SPICE_ROOT_KEY;
```




<hr>



### variable DB\_SS\_TIME\_INDICES\_KEY 

```C++
std::string SpiceQL::DB_SS_TIME_INDICES_KEY;
```




<hr>



### variable DB\_START\_TIME\_KEY 

```C++
std::string SpiceQL::DB_START_TIME_KEY;
```




<hr>



### variable DB\_STOP\_TIME\_KEY 

```C++
std::string SpiceQL::DB_STOP_TIME_KEY;
```




<hr>



### variable DB\_TIME\_FILES\_KEY 

```C++
std::string SpiceQL::DB_TIME_FILES_KEY;
```




<hr>



### variable KERNEL\_QUALITIES 

```C++
const std::vector<std::string> SpiceQL::KERNEL_QUALITIES;
```




<hr>



### variable KERNEL\_TYPES 

```C++
const std::vector<std::string> SpiceQL::KERNEL_TYPES;
```




<hr>
## Public Functions Documentation




### function addAliasKey 

_Adds or updates a key-value pair in the aliasMap._ 
```C++
void SpiceQL::addAliasKey (
    const std::string & key,
    const std::string & value
) 
```





**Parameters:**


* `key` The key to add or update. 
* `value` The value to associate with the key. 




        

<hr>



### function checkNaifErrors 

_raises a C++ exception if NAIF has an error buffered._ 
```C++
bool SpiceQL::checkNaifErrors (
    bool reset=true
) 
```





**Parameters:**


* `reset` true if NAIF status errors should be reset 




        

<hr>



### function concatStates 

```C++
std::vector< std::vector< double > > SpiceQL::concatStates (
    std::vector< std::vector< double > > statePositions,
    std::vector< std::vector< double > > stateVelocities
) 
```



Combine the state positions and velocities into a single vector




**Returns:**

Single vector with {X1, Y1, Z1, dX1, dY1, dZ1, X2, Y2, Z2, dX2, dY2, dZ2, ...} 





        

<hr>



### function currentLogLevel 

```C++
inline LogLevel SpiceQL::currentLogLevel () 
```




<hr>



### function doubleEtToSclk 

_Converts a given double spacecraft clock time to an ephemeris time._ 
```C++
std::pair< std::string, nlohmann::json > SpiceQL::doubleEtToSclk (
    int frameCode,
    double et,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given a known frame code doubleSclkToEt converts a given spacecraft clock time as a double to an ephemeris time. Call this function if your clock time looks something like: 922997380.174174




**Parameters:**


* `frameCode` int Frame id to use 
* `et` double Spacecraft ephemeris time to convert to an SCLK 
* `mission` string Mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load 



**Returns:**

double 





        

<hr>



### function doubleEtsToSclkTicks 

_Converts ephemeris times to encoded SCLK "ticks" (doubles)._ 
```C++
std::pair< std::vector< double >, nlohmann::json > SpiceQL::doubleEtsToSclkTicks (
    int frameCode,
    std::vector< double > ets,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given a spacecraft clock id, converts a vector of ephemeris times to the continuous encoded spacecraft clock values (ticks) produced by NAIF's sce2c\_c. These are the values CK writers (e.g. ckw03\_c) expect. Unlike doubleEtToSclk, which returns the human-readable clock string via sce2s\_c, this returns the encoded double directly so callers can write CK segments without furnishing the SCLK/LSK themselves.




**Parameters:**


* `frameCode` int Spacecraft clock id (e.g. -74 for MRO) 
* `ets` vector&lt;double&gt; ephemeris times to convert 
* `mission` string Mission name as it relates to the config files 
* `useWeb` bool Whether to run the conversion via the remote web service 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load 



**Returns:**

vector&lt;double&gt; of encoded SCLK ticks matching the input ets 





        

<hr>



### function doubleSclkToEt 

_Converts a given double spacecraft clock time to an ephemeris time._ 
```C++
std::pair< double, nlohmann::json > SpiceQL::doubleSclkToEt (
    int frameCode,
    double sclk,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given a known frame code doubleSclkToEt converts a given spacecraft clock time as a double to an ephemeris time. Call this function if your clock time looks something like: 922997380.174174




**Parameters:**


* `frameCode` int Frame id to use 
* `sclk` int Spacecraft Clock formatted as an int 
* `mission` string Mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load 



**Returns:**

double 





        

<hr>



### function eraseAtPointer 

_erase a part of a json object based on a json pointer_ 
```C++
size_t SpiceQL::eraseAtPointer (
    nlohmann::json & j,
    nlohmann::json::json_pointer ptr
) 
```





**Parameters:**


* `j` The json object ot erase part of. Modified in place 
* `ptr` The object to erase



**Returns:**

The number of objects removed 





        

<hr>



### function etToUtc 

_convert et string to a UTC string_ 
```C++
std::pair< std::string, nlohmann::json > SpiceQL::etToUtc (
    double et,
    std::string format="",
    double precision=0,
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Basically a wrapper around NAIF's cspice et2utc\_c function except it also temporarily loads the required kernels. See Also: [https://naif.jpl.nasa.gov/pub/naif/toolkit\_docs/C/cspice/et2utc\_c.html](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/et2utc_c.html)




**Parameters:**


* `et` ephemeris time 
* `precision` number of decimal 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

double precision ephemeris time 





        

<hr>



### function extractExactCkTimes 

_Extracts all segment times between observStart and observeEnd._ 
```C++
std::pair< std::vector< double >, nlohmann::json > SpiceQL::extractExactCkTimes (
    double observStart,
    double observEnd,
    int targetFrame,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given an observation start and observation end, extract all times assocaited with segments in a CK file. The times returned are all times assocaited with concrete CK segment times with no interpolation. This function is limited to loading one CK file at a time, if a time window covers multiple CK files, the function will throw an error. For this reason, limitCK is set to 1 by default.




**Parameters:**


* `observStart` Ephemeris time to start searching at 
* `observEnd` Ephemeris time to stop searching at 
* `targetFrame` Target reference frame to get ephemeris data in 
* `ckQualities` vector of string describing the quality of cks to try and obtain 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

A list of times 





        

<hr>



### function findKeyInJson 

_recursively search keys in json._ 
```C++
std::vector< nlohmann::json::json_pointer > SpiceQL::findKeyInJson (
    nlohmann::json in,
    std::string key,
    bool recursive=true
) 
```



Given a root and a regular expression, give all the files that match.




**Parameters:**


* `in` input json to search 
* `key` key to search for 
* `recursive` recursively iterates through objects if true



**Returns:**

vector of refernces to matching json objects 





        

<hr>



### function findKeywords 

_finds key:values in kernel pool_ 
```C++
nlohmann::json SpiceQL::findKeywords (
    std::string keytpl
) 
```



Given a key template, returns matching key:values from the kernel pool by using gnpool, gcpool, gdpool, and gipool




**Parameters:**


* `keytpl` input key template to search for



**Returns:**

json list of found key:values 





        

<hr>



### function findMissionKeywords 

_returns kernel text keyword values for a specific mission as json_ 
```C++
std::pair< nlohmann::json, nlohmann::json > SpiceQL::findMissionKeywords (
    std::string key,
    std::string mission,
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Takes in a kernel key from iaks, iks, and fks and returns the value associated with the input mission (e.g. LRO, MRO, sun etc) as json. findMissionKeywords is a aggregation of cspice's gnpool\_c, gcpool\_c, gdpool\_c, and gipool\_c. Input key supports wildcards, e.g. "LRO\_\*", "\*\_BORESIGHT\_SAMPLE", or "\*-8600\*".




**Parameters:**


* `key` kernel text keyword to look for 
* `mission` spiceql name to search for (e.g. LRO, MRO, sun etc) 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `searchKernels` bool Whether to search the kernels for the user 
* `kernelList` vector of additional kernels to load



**Returns:**

json object of key value pairs 





        

<hr>



### function findTargetKeywords 

_returns Target values in the form of a vector_ 
```C++
std::pair< nlohmann::json, nlohmann::json > SpiceQL::findTargetKeywords (
    std::string key,
    std::string mission,
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Takes in a target and key and returns the value associated in the form of vector from PCKs. findTargetKeywords is a aggregation of cspice's gnpool\_c, gcpool\_c, gdpool\_c, and gipool\_c. Input key supports wildcards, e.g. "\*\_RADII" or "\*-8600\*". Note: This function is mainly for obtaining target keywords. For obtaining other values, use findMissionKeywords.




**Parameters:**


* `key` keyword for desired values 
* `mission` mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

json object of key value pairs 





        

<hr>



### function formatKernels 

_Format a list of kernel paths into a kernels JSON object organized by type._ 
```C++
nlohmann::json SpiceQL::formatKernels (
    std::vector< std::string > kernelPaths
) 
```





**Parameters:**


* `kernelPaths` vector of kernel file paths 



**Returns:**

nlohmann::json object with kernels organized by type 





        

<hr>



### function frameList 

_Get a list of all frames in the supported by the library._ 
```C++
std::vector< std::string > SpiceQL::frameList () 
```





**Returns:**

std::vector&lt;std::string&gt; list of frames 





        

<hr>



### function frameTrace 

_Given an ephemeris time and a starting frame, find the path from that starting frame to J2000 (1)._ 
```C++
std::pair< std::vector< std::vector< int > >, nlohmann::json > SpiceQL::frameTrace (
    double et,
    int initialFrame,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



This function uses NAIF routines and builds a path from the initalframe to J2000 making note of all the in between frames




**Parameters:**


* `et` ephemeris times at which you want to optain the frame trace 
* `initialFrame` the initial frame's NAIF code. 
* `mission` [**Config**](classSpiceQL_1_1Config.md) subset as it relates to the mission 
* `ckQualities` vector of strings describing the quality of cks to try and obtain 
* `spkQualities` vector of strings describing the quality of spks to try and obtain 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

A two element vector of vectors ints, where the first element is the sequence of time dependent frames and the second is the sequence of constant frames 





        

<hr>



### function gen\_random 

_generate a random string_ 
```C++
std::string SpiceQL::gen_random (
    const int len
) 
```





**Parameters:**


* `len` length of the string 



**Returns:**

new random alphanumeric string 





        

<hr>



### function getAliasMap 

_Accessor for the aliasMap._ 
```C++
nlohmann::json SpiceQL::getAliasMap () 
```





**Returns:**

const reference to the aliasMap JSON object. 





        

<hr>



### function getAliasMapJsonFile 

_Gets default path for JSON file of aliases._ 
```C++
std::string SpiceQL::getAliasMapJsonFile () 
```





**Returns:**

string file path 





        

<hr>



### function getAvailableConfigFiles 

_Returns a vector of all the available configs._ 
```C++
std::vector< std::string > SpiceQL::getAvailableConfigFiles () 
```



Returns the db files in either the installed or debug directory depending on whether or not SPICEQL\_DEV\_DB is set.




**See also:** [**getConfigDirectory**](namespaceSpiceQL.md#function-getconfigdirectory)


**Returns:**

std::vector&lt;std::string&gt; 





        

<hr>



### function getAvailableConfigs 

_Get names of available config files as a json vector._ 
```C++
std::vector< nlohmann::json > SpiceQL::getAvailableConfigs () 
```



This iterates through all the configs in the db folder either installed or in the debug directory depending on whether or not SPICEQL\_DEV\_DB is set. Loads them as vector of json obects and returns the vector.




**Returns:**

std::vector&lt;nlohmann::json&gt; 





        

<hr>



### function getCacheDir 

```C++
std::string SpiceQL::getCacheDir () 
```




<hr>



### function getConfigDirectory 

_Get the directory pointing to the db files._ 
```C++
std::string SpiceQL::getConfigDirectory () 
```



The directory is resolved in the following order of precedence:
* A directory previously set via [**setConfigDirectory()**](namespaceSpiceQL.md#function-setconfigdirectory).
* The env var $SPICEQL\_CONFIG\_DIR, if set and non-empty.
* The local source path \_SOURCE\_PREFIX/SpiceQL/db/, if $SPICEQL\_DEV\_DB is set.
* The installed DB files in $CONDA\_PREFIX/etc/SpiceQL/db.






**Returns:**

std::string directory containing db files 





        

<hr>



### function getDataDirectory 

_Returns std::vector&lt;string&gt; interpretation of a json array._ 
```C++
std::string SpiceQL::getDataDirectory () 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

string vector containing arr data 





        

<hr>



### function getDefaultLsk 

_Load the local LSK file that comes installed with_ [_**SpiceQL**_](namespaceSpiceQL.md) _._
```C++
std::string SpiceQL::getDefaultLsk () 
```




<hr>



### function getExactTargetOrientations 

_Returns exact target orientations for given time intervals and parameters._ 
```C++
std::pair< std::vector< std::vector< double > >, nlohmann::json > SpiceQL::getExactTargetOrientations (
    double startEt,
    double stopEt,
    int toFrame,
    int refFrame,
    int exactCkFrame,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given a start and stop ephemeris time, extract all times assocaited with segments in a CK file. The times returned are all times assocaited with concrete CK segment times with no interpolation. This function is limited to loading one CK file at a time, if a time window covers multiple CK files, the function will throw an error.




**Parameters:**


* `startEts` vector of start ephemeris times 
* `stopEts` vector of stop ephemeris times 
* `exposureDuration` vector of exposure durations 
* `toFrame` target frame 
* `refFrame` reference frame 
* `mission` mission name as it relates to the config files 
* `ckQualities` vector of string describing the quality of cks to try and obtain 
* `useWeb` bool whether to use web [**SpiceQL**](namespaceSpiceQL.md) 
* `searchKernels` bool whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitQuality` bool only returns higher priority quality kernel if true 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

Vector of SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with) 





        

<hr>



### function getFrameInfo 

_Get the center, class id, and class of a given frame._ 
```C++
std::pair< std::vector< int >, nlohmann::json > SpiceQL::getFrameInfo (
    int frame,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



See [NAIF's Docs on frame codes](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html) for more information




**Parameters:**


* `frame` String frame name to translate to a NAIF code 
* `mission` Mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

3 element vector of the given frames center, class id, and class 





        

<hr>



### function getHdfFile 

```C++
std::string SpiceQL::getHdfFile () 
```




<hr>



### function getKernelStartStopTimes 

```C++
std::pair< double, double > SpiceQL::getKernelStartStopTimes (
    std::string kpath
) 
```




<hr>



### function getKernelStringValue 

_return's kernel value from key_ 
```C++
std::string SpiceQL::getKernelStringValue (
    std::string key
) 
```



Takes in a kernel key and returns the value associated with that kernel as a string Note: this function is for when the kernal has a single value associated with it, ie: INS-236800\_FOV\_REF\_ANGLE = ( 5.27 )




**Parameters:**


* `key` key - [**Kernel**](classSpiceQL_1_1Kernel.md) to get values from 



**Returns:**

string of value associated with key 





        

<hr>



### function getKernelType 

_get the_ [_**Kernel**_](classSpiceQL_1_1Kernel.md) _type (CK, SPK, etc.)_
```C++
std::string SpiceQL::getKernelType (
    std::string kernelPath
) 
```





**Parameters:**


* `kernelPath` path to kernel 



**Returns:**

[**Kernel**](classSpiceQL_1_1Kernel.md) type as a string 





        

<hr>



### function getKernelVectorValue 

_return's kernel values in the form of a vector_ 
```C++
std::vector< std::string > SpiceQL::getKernelVectorValue (
    std::string key
) 
```



Takes in a kernel key and returns the value associated with that kernel as a vector of string Note: This function is for when the kernel has more than 1 value associated with it, ie: INS-236800\_FOV\_REF\_VECTOR = ( 1.0, 0.0, 0.0 )




**Parameters:**


* `key` key - [**Kernel**](classSpiceQL_1_1Kernel.md) to get values from 



**Returns:**

vector of values in the form of a string 





        

<hr>



### function getKernelsAsSet 

_Get all the kernels in the json as a set._ 
```C++
std::set< std::string > SpiceQL::getKernelsAsSet (
    nlohmann::json kernels
) 
```



Recusively iterates all the kernel keys and flattens them in a vector.




**Parameters:**


* `kernels` json object with kernel query results 



**Returns:**

set&lt;string&gt; set of kernels 





        

<hr>



### function getKernelsAsVector 

_Get all the kernels in the json as a vector._ 
```C++
std::vector< std::string > SpiceQL::getKernelsAsVector (
    nlohmann::json kernels
) 
```



Recusively iterates all the kernel keys and flattens them in a vector.




**Parameters:**


* `kernels` json object with kernel query results 



**Returns:**

vector&lt;string&gt; list of kernels 





        

<hr>



### function getLatestKernel 

_get the latest kernel in a list_ 
```C++
std::vector< std::string > SpiceQL::getLatestKernel (
    std::vector< std::string > kernels
) 
```



Returns the kernels with the latest version string (e.g. the highest v??? or similar sub-string in a kernel path name) in the input list and returns it as a path object. Given multiple different kernels, like de### and mar###, each will be evaluted on there own to return the highest version of each.




**Parameters:**


* `kernels` vector of strings, should be a list of kernel paths. 



**Returns:**

std::vector&lt;std::string&gt; vector paths to latest Kernels 





        

<hr>



### function getLatestKernels 

_returns a JSON object of only the latest version of each kernel type_ 
```C++
nlohmann::json SpiceQL::getLatestKernels (
    nlohmann::json kernels
) 
```



Recursively iterates [**Kernel**](classSpiceQL_1_1Kernel.md) groups in the input JSON and gets the kernels with the latest version string (e.g. the highest v??? sub-string in a kernel path name).


New JSON is returned.




**Parameters:**


* `kernels` A [**Kernel**](classSpiceQL_1_1Kernel.md) JSON object 



**Returns:**

A new [**Kernel**](classSpiceQL_1_1Kernel.md) JSON object with reduced kernel sets 





        

<hr>



### function getLoadedKernels 

_Returns the file paths of all kernels currently loaded in the CSPICE pool._ 
```C++
std::vector< std::string > SpiceQL::getLoadedKernels () 
```



Reflects every kernel in the pool regardless of type or how it was loaded, including kernels furnished outside of [**SpiceQL**](namespaceSpiceQL.md) (e.g. by a host application such as ISIS). 


        

<hr>



### function getMissionConfig 

_Returns the path to the Mission specific Spice config file._ 
```C++
nlohmann::json SpiceQL::getMissionConfig (
    std::string mission
) 
```



Given a mission, search a prioritized list of directories for the json config file. This function checks in the order:



* The local build dir, i.e. $CMAKE\_SOURCE\_DIR
* The install dir, i.e. $CMAKE\_PREFIX






**Parameters:**


* `mission` mission name of the config file



**Returns:**

path object of the config file 





        

<hr>



### function getMissionConfigFile 

_Returns the path to the Mission specific Spice config file._ 
```C++
std::string SpiceQL::getMissionConfigFile (
    std::string mission
) 
```



Given a mission, search a prioritized list of directories for the json config file. This function checks in the order:



* The local build dir, i.e. $CMAKE\_SOURCE\_DIR
* The install dir, i.e. $CMAKE\_PREFIX






**Parameters:**


* `mission` mission name of the config file



**Returns:**

path object of the condig file 





        

<hr>



### function getMissionKeys 

_Returns a string of mission keys from the JSON config file._ 
```C++
std::string SpiceQL::getMissionKeys (
    nlohmann::json config
) 
```





**Parameters:**


* `config` JSON config file



**Returns:**

string of mission keys 





        

<hr>



### function getPathsFromRegex 

_glob, but with json_ 
```C++
std::vector< std::vector< std::string > > SpiceQL::getPathsFromRegex (
    std::string root,
    std::vector< std::string > regexes
) 
```



Lambda for globbing files from a regular expression stored in json. As they can be a single expression or a list, we need to massage the json a little.




**Parameters:**


* `root` root path to search 
* `r` json list of regexes 



**Returns:**

vector of paths 





        

<hr>



### function getRestUrl 

_Returns the REST URL._ 
```C++
std::string SpiceQL::getRestUrl () 
```





**Returns:**

[**SpiceQL**](namespaceSpiceQL.md)'s REST URL as string 





        

<hr>



### function getRootDependency 

_Returns the root most dependency for a json pointer._ 
```C++
std::string SpiceQL::getRootDependency (
    nlohmann::json config,
    std::string pointer
) 
```



Given a config json, recursively find the root pointer for the given json pointer




**Parameters:**


* `config` unevaluated config json 
* `pointer` json pointer to get the root dependency for



**Returns:**

string vector containing arr data 





        

<hr>



### function getSpiceqlName 

_Translates a given name using the aliasMap and checks if the name is in the frameList._ 
```C++
std::string SpiceQL::getSpiceqlName (
    const std::string & name
) 
```



If the name exists as a key in aliasMap, returns the mapped value. If the name exists in frameList, returns the name itself. Otherwise, returns an empty string.




**Parameters:**


* `name` The name to translate. 
* `frameList` The list of valid frame names. 



**Returns:**

The translated name or an empty string if not found. 





        

<hr>



### function getTargetFrameInfo 

_returns frame name and frame code associated to the target ID._ 
```C++
std::pair< nlohmann::json, nlohmann::json > SpiceQL::getTargetFrameInfo (
    int targetId,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Takes in a target id and returns the frame name and frame code in json format




**Parameters:**


* `targetId` target ID 
* `mission` mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

json of frame name and frame code 





        

<hr>



### function getTargetOrientation 

_Gives quaternion and angular velocity for a given frame at a given ephemeris time._ 
```C++
std::vector< double > SpiceQL::getTargetOrientation (
    double et,
    int toFrame,
    int refFrame=1
) 
```



Gets an orientation for an input frame in some reference frame. The orientations returned from this function can be used to transform a position in the source frame to the ref frame.




**Parameters:**


* `et` ephemeris time at which you want to optain the target pointing 
* `toframe` the source frame's NAIF code. 
* `refframe` the reference frame's NAIF code, orientations are relative to this reference frame 



**Returns:**

SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with) 





        

<hr>



### function getTargetOrientations 

_Gives quaternion and angular velocity for a given frame at a set of ephemeris times._ 
```C++
std::pair< std::vector< std::vector< double > >, nlohmann::json > SpiceQL::getTargetOrientations (
    std::vector< double > ets,
    int toFrame,
    int refFrame,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Gets orientations for an input frame in some reference frame. The orientations returned from this function can be used to transform a position in the source frame to the ref frame.




**Parameters:**


* `ets` ephemeris times at which you want to optain the target pointing 
* `toframe` the source frame's NAIF code. 
* `refframe` the reference frame's NAIF code, orientations are relative to this reference frame 
* `mission` [**Config**](classSpiceQL_1_1Config.md) subset as it relates to the mission 
* `ckQualities` vector of string describing the quality of cks to try and obtain 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**See also:** [**SpiceQL::getTargetOrientation**](namespaceSpiceQL.md#function-gettargetorientation)


**Returns:**

Vector of SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with) 





        

<hr>



### function getTargetOrientationsRanged 

_Gives quaternion and angular velocity for a given frame at a set of ephemeris times._ 
```C++
std::pair< std::vector< std::vector< double > >, nlohmann::json > SpiceQL::getTargetOrientationsRanged (
    double startEt,
    double stopEt,
    int numRecords,
    int toFrame,
    int refFrame,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Gets orientations for an input frame in some reference frame. The orientations returned from this function can be used to transform a position in the source frame to the ref frame.




**Parameters:**


* `startEt` ephemeris times at which you want to obtain the target orientation 
* `stopEt` ephemeris times at which you want to obtain the target orientation 
* `numRecords` the number of orientations to obtain between startEt and stopEt 
* `toframe` the source frame's NAIF code. 
* `refframe` the reference frame's NAIF code, orientations are relative to this reference frame 
* `mission` [**Config**](classSpiceQL_1_1Config.md) subset as it relates to the mission 
* `ckQualities` vector of string describing the quality of cks to try and obtain 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**See also:** [**SpiceQL::getTargetOrientation**](namespaceSpiceQL.md#function-gettargetorientation)


**Returns:**

Vector of SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with) 





        

<hr>



### function getTargetState 

_Gives the position and velocity for a given frame at some ephemeris time._ 
```C++
std::vector< double > SpiceQL::getTargetState (
    double et,
    std::string target,
    std::string observer,
    std::string frame="J2000",
    std::string abcorr="NONE"
) 
```



Mostly a C++ wrap for NAIF's spkezr\_c




**Parameters:**


* `et` ephemeris time at which you want to optain the target state 
* `target` NAIF ID for the target frame 
* `observer` NAIF ID for the observing frame 
* `frame` The reference frame in which to get the positions in 
* `abcorr` aborration correction flag, default it NONE. This can set to: "NONE" - No correction For the "reception" case, i.e. photons from the target being recieved by the observer at the given time. "LT" - One way light time correction "LT+S" - Correct for one-way light time and stellar aberration correction "CN" - Converging Newtonian light time correction "CN+S" - Converged Newtonian light time correction and stellar aberration correction For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time "XLT" - One-way light time correction using a newtonian formulation "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation "XCN" - converged Newtonian light time correction "XCN+S" - converged Newtonian light time correction and stellar aberration correction. 



**Returns:**

A vector of 7 elements with a 0 - 5 index state vector of position and velocity in x,y,z,vx,vy,vz format followed by the light time adjustment at the 6th index. 





        

<hr>



### function getTargetStates 

_Gives the positions and velocities for a given frame given a set of ephemeris times._ 
```C++
std::pair< std::vector< std::vector< double > >, nlohmann::json > SpiceQL::getTargetStates (
    std::vector< double > ets,
    std::string target,
    std::string observer,
    std::string frame,
    std::string abcorr,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Mostly a C++ wrap for NAIF's spkezr\_c




**Parameters:**


* `ets` ephemeris times at which you want to obtain the target state 
* `target` NAIF ID for the target frame 
* `observer` NAIF ID for the observing frame 
* `frame` The reference frame in which to get the positions in 
* `abcorr` aborration correction flag, default it NONE. This can set to: "NONE" - No correction For the "reception" case, i.e. photons from the target being recieved by the observer at the given time. "LT" - One way light time correction "LT+S" - Correct for one-way light time and stellar aberration correction "CN" - Converging Newtonian light time correction "CN+S" - Converged Newtonian light time correction and stellar aberration correction For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time "XLT" - One-way light time correction using a newtonian formulation "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation "XCN" - converged Newtonian light time correction "XCN+S" - converged Newtonian light time correction and stellar aberration correction. 
* `mission` [**Config**](classSpiceQL_1_1Config.md) subset as it relates to the mission 
* `ckQualities` vector of strings describing the quality of cks to try and obtain 
* `spkQualities` string of strings describing the quality of spks to try and obtain 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**See also:** [**SpiceQL::getTargetState**](namespaceSpiceQL.md#function-gettargetstate) 


**See also:** [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality)


**Returns:**

A vector of vectors with a Nx7 state vector of positions and velocities in x,y,z,vx,vy,vz format followed by the light time adjustment. 





        

<hr>



### function getTargetStatesRanged 

_Gives the positions and velocities for a given start and stop ephemeris times and number of records._ 
```C++
std::pair< std::vector< std::vector< double > >, nlohmann::json > SpiceQL::getTargetStatesRanged (
    double startEt,
    double stopEt,
    int numRecords,
    std::string target,
    std::string observer,
    std::string frame,
    std::string abcorr,
    std::string mission="",
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Mostly a C++ wrap for NAIF's spkezr\_c




**Parameters:**


* `startEt` ephemeris times at which you want to obtain the target state 
* `stopEt` ephemeris times at which you want to obtain the target state 
* `numRecords` the number of states to obtain between startEt and stopEt 
* `target` NAIF ID for the target frame 
* `observer` NAIF ID for the observing frame 
* `frame` The reference frame in which to get the positions in 
* `abcorr` aborration correction flag, default it NONE. This can set to: "NONE" - No correction For the "reception" case, i.e. photons from the target being recieved by the observer at the given time. "LT" - One way light time correction "LT+S" - Correct for one-way light time and stellar aberration correction "CN" - Converging Newtonian light time correction "CN+S" - Converged Newtonian light time correction and stellar aberration correction For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time "XLT" - One-way light time correction using a newtonian formulation "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation "XCN" - converged Newtonian light time correction "XCN+S" - converged Newtonian light time correction and stellar aberration correction. 
* `mission` [**Config**](classSpiceQL_1_1Config.md) subset as it relates to the mission 
* `ckQualities` vector of strings describing the quality of cks to try and obtain 
* `spkQualities` string of strings describing the quality of spks to try and obtain 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**See also:** [**SpiceQL::getTargetState**](namespaceSpiceQL.md#function-gettargetstate) 


**See also:** [**Kernel::Quality**](classSpiceQL_1_1Kernel.md#enum-quality)


**Returns:**

A vector of vectors with a Nx7 state vector of positions and velocities in x,y,z,vx,vy,vz format followed by the light time adjustment. 





        

<hr>



### function getTimeIntervals 

_Get start and stop times a kernel._ 
```C++
std::vector< std::pair< double, double > > SpiceQL::getTimeIntervals (
    std::string kpath
) 
```



For each segment in the kernel, get all start and stop times as a vector of double pairs. This gets all start and stop times regardless of the frame associated with it.


Input kernel is assumed to be a binary kernel with time dependant external orientation data.




**Parameters:**


* `kpath` Path to the kernel 



**Returns:**

std::vector of start and stop times 





        

<hr>



### function glob 

_glob, like python's glob.glob, except C++_ 
```C++
std::vector< std::string > SpiceQL::glob (
    std::string const & root,
    std::string const & reg=".*",
    bool recursive=false
) 
```



Given a root and a regular expression, give all the files that match.




**Parameters:**


* `root` The root directory to search 
* `reg` std::regex object to pattern to search, defaults to ".\*", or match averything. 
* `recursive` recursively iterates through directories if true



**Returns:**

list of paths matching regex 





        

<hr>



### function globKernelStartStopTimes 

```C++
std::string SpiceQL::globKernelStartStopTimes (
    std::string mission
) 
```




<hr>



### function globKernels 

_acquire all kernels of a type according to a configuration JSON object_ 
```C++
nlohmann::json SpiceQL::globKernels (
    std::string root,
    nlohmann::json conf,
    std::string kernelType
) 
```



Given the root directotry with kernels, a JSON configuration object and a kernel type string (e.g. ck, fk, spk), return a JSON object containing kernels. The kernel config's regular expressions are replaced by a concrete kernel list located in the passed in root.




**Parameters:**


* `root` Directory with kernels somewhere in the directory or its subdirectories 
* `conf` JSON config file, usually this is a JSON object read from one of the db files that shipped with the library 
* `kernelType` Some CK kernel type, see KERNEL\_TYPES 




        

<hr>



### function globTimeIntervals 

_Get start and stop times for all kernels._ 
```C++
std::string SpiceQL::globTimeIntervals (
    std::string mission
) 
```





**Returns:**

string json map of kernel names to list of time segments 





        

<hr>



### function inferMission 

_Infer the_ [_**SpiceQL**_](namespaceSpiceQL.md) _config (mission) name from identifying inputs._
```C++
std::string SpiceQL::inferMission (
    const std::vector< std::string > & nameCandidates,
    const std::vector< int > & codeCandidates
) 
```



Used to make the `mission` parameter of the API optional. String candidates (e.g. frame/instrument/target names) are resolved via the alias map. Integer candidates (NAIF frame/body codes) are resolved to a name without furnishing kernels (via the precomputed frame cache, then NAIF's built-in mappings), trying both the code and its bus code (code / 1000), then mapped via the alias map.




**Parameters:**


* `nameCandidates` Ordered string candidates to resolve. 
* `codeCandidates` Ordered NAIF code candidates to resolve. 



**Returns:**

The first [**SpiceQL**](namespaceSpiceQL.md) config name found, or an empty string. 





        

<hr>



### function isLskLoaded 

_Returns true if a leapseconds kernel (LSK) is loaded in the CSPICE pool._ 
```C++
bool SpiceQL::isLskLoaded () 
```



Detects an LSK by checking the kernel pool for the DELTET/DELTA\_T\_A leapseconds variable that every LSK defines. This catches LSKs loaded outside of [**SpiceQL**](namespaceSpiceQL.md) (e.g. furnished into the shared pool by a host application such as ISIS). 


        

<hr>



### function json2DArrayTo2DVector 

_Returns std::vector&lt;std::vector&lt;string&gt;&gt; interpretation of a json array._ 
```C++
std::vector< std::vector< std::string > > SpiceQL::json2DArrayTo2DVector (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

string vector containing arr data 





        

<hr>



### function json2DArrayToDoublePair 

_Returns std::vector&lt;std::vector&lt;string&gt;&gt; interpretation of a json array._ 
```C++
std::vector< std::pair< double, double > > SpiceQL::json2DArrayToDoublePair (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Throws exception if the json obj is not an Nx2 array of doubles.




**Parameters:**


* `arr` input json arr



**Returns:**

pair vector containing arr data 





        

<hr>



### function json2DFloatArrayTo2DVector 

_Returns std::vector&lt;std::vector&lt;float&gt;&gt; interpretation of a json array._ 
```C++
std::vector< std::vector< double > > SpiceQL::json2DFloatArrayTo2DVector (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

double vector containing arr data 





        

<hr>



### function json2DIntArrayTo2DVector 

_Returns std::vector&lt;std::vector&lt;int&gt;&gt; interpretation of a json array._ 
```C++
std::vector< std::vector< int > > SpiceQL::json2DIntArrayTo2DVector (
    nlohmann::json arr,
    bool retainEmpty=false
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

int vector containing arr data 





        

<hr>



### function jsonArrayToVector 

_Returns std::vector&lt;string&gt; interpretation of a json array._ 
```C++
std::vector< std::string > SpiceQL::jsonArrayToVector (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

string vector containing arr data 





        

<hr>



### function jsonDoubleArrayToVector 

_Returns std::vector&lt;double&gt; interpretation of a json array._ 
```C++
std::vector< double > SpiceQL::jsonDoubleArrayToVector (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

double containing arr data 





        

<hr>



### function jsonIntArrayToVector 

_Returns std::vector&lt;int&gt; interpretation of a json array._ 
```C++
std::vector< int > SpiceQL::jsonIntArrayToVector (
    nlohmann::json arr
) 
```



Attempts to convert the json array to a C++ array. Also handles strings in cases where one element arrays are stored as scalars. Throws exception if the json obj is not an array.




**Parameters:**


* `arr` input json arr



**Returns:**

int containing arr data 





        

<hr>



### function listMissionKernels 

_Returns all kernels available for a mission._ 
```C++
nlohmann::json SpiceQL::listMissionKernels (
    std::string root,
    nlohmann::json conf
) 
```



Returns a structured json object containing all available kernels for a specified mission along with their dependencies.


TODO: Add a "See Also" on json format after the format matures a bit more.




**Parameters:**


* `root` root path to search 
* `conf` json conf file 



**Returns:**

list of paths matching ext 





        

<hr>



### function load 

```C++
void SpiceQL::load (
    std::string path,
    bool force_refurnsh=true
) 
```




<hr>



### function loadSelectKernels 

_Loads PCK kernels associated to mission name._ 
```C++
nlohmann::json SpiceQL::loadSelectKernels (
    std::string kernelType,
    std::string mission
) 
```





**Parameters:**


* `kernelType` kernelType to search for and load 
* `mission` mission name of the config file 




        

<hr>



### function loadTranslationKernels 

_Loads translation kernels (fk, ik, and iaks) associated to mission name._ 
```C++
nlohmann::json SpiceQL::loadTranslationKernels (
    std::string mission,
    bool loadFk=true,
    bool loadIk=true,
    bool loadIak=true
) 
```





**Parameters:**


* `mission` mission name of the config file 




        

<hr>



### function load\_aliases 

_Free function to trigger loading aliases._ 
```C++
void SpiceQL::load_aliases (
    std::string path=""
) 
```





**Parameters:**


* `path` Optional path to a specific JSON file 




        

<hr>



### function logLevelName 

```C++
inline const char * SpiceQL::logLevelName (
    LogLevel lvl
) 
```




<hr>



### function logMessage 

```C++
template<typename... Args>
inline void SpiceQL::logMessage (
    LogLevel lvl,
    fmt::string_view f,
    Args &&... args
) 
```




<hr>



### function ls 

_ls, like in unix, kinda. Also it's a function._ 
```C++
std::vector< std::string > SpiceQL::ls (
    std::string const & root,
    bool recursive
) 
```



Iterates the input path and returning a list of files. Optionally, recursively.




**Parameters:**


* `root` The root directory to search 
* `recursive` recursively iterates through directories if true



**Returns:**

list of paths 





        

<hr>



### function mergeConfigs 

_Merge two json configs._ 
```C++
void SpiceQL::mergeConfigs (
    nlohmann::json & baseConfig,
    const nlohmann::json & mergingConfig
) 
```



When arrays are merged, the values from the base config will appear first in the merged config.




**Parameters:**


* `baseConfig` The config to merge into 
* `mergingConfig` The config to merge into the base config 




        

<hr>



### function merge\_json 

_Merges the right json to the left json._ 
```C++
nlohmann::json SpiceQL::merge_json (
    nlohmann::json & j1,
    nlohmann::json & j2,
    bool overwrite=false
) 
```





**Parameters:**


* `j1` input json obj, this json is having it's values edited 
* `j2` input json obj to be merged into j1 
* `overwrite` if true, overwrites the keys in j1 in favors of those in j2. If overwrite=true, this is the same as nlohmann::json::merge\_patch 




**Returns:**

j1 with j2 keys + values 





        

<hr>



### function parseLogLevel 

```C++
inline LogLevel SpiceQL::parseLogLevel (
    const std::string & s
) 
```




<hr>



### function replaceAll 

_find and replace one substring with another_ 
```C++
std::string SpiceQL::replaceAll (
    std::string str,
    const std::string & from,
    const std::string & to
) 
```





**Parameters:**


* `str` input string to search 
* `from` substring to find 
* `to` substring to replace "from" instances to 



**Returns:**

std::string 





        

<hr>



### function resolveConfigDependencies 

_resolve the dependencies in a config in place_ 
```C++
void SpiceQL::resolveConfigDependencies (
    nlohmann::json & config,
    const nlohmann::json & dependencies
) 
```



Given a config with "deps" keys in it and a second config to extract dependencies from, recursively resolve all of the deps into their actual values. Only allows up to 10 recurssions.




**Parameters:**


* `config` The config to populate 
* `dependencies` The config to pull dependencies from



**Returns:**

The full instrument config 





        

<hr>



### function searchForKernelsets 

_Searches for kernels given mission(s) and parameters._ 
```C++
std::pair< std::string, nlohmann::json > SpiceQL::searchForKernelsets (
    std::vector< std::string > spiceqlNames,
    std::vector< std::string > types={"ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"},
    double startTime=-std::numeric_limits< double >::max(),
    double stopTime=std::numeric_limits< double >::max(),
    std::vector< std::string > ckQualities={"smithed", "reconstructed"},
    std::vector< std::string > spkQualities={"smithed", "reconstructed"},
    bool useWeb=false,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    bool overwrite=false
) 
```



Searches for kernels given mission(s) and parameters.




**Parameters:**


* `spiceqlNames` mission names 
* `types` kernel types 
* `startTime` Ephemeris time to start searching at 
* `stopTime` Ephemeris time to stop searching at 
* `ckQualities` vector of string describing the quality of cks to try and obtain 
* `spkQualities` vector of string describing the quality of spks to try and obtain 
* `useWeb` whether to use web [**SpiceQL**](namespaceSpiceQL.md) 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` limitCk int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `overwrite` subkernels will take precedent over parent mission kernels if true



**Returns:**

An empty return and list of kernels 





        

<hr>



### function setAliasMap 

_Setter for the aliasMap._ 
```C++
void SpiceQL::setAliasMap (
    const nlohmann::json & newAliasMap
) 
```





**Parameters:**


* `newAliasMap` The new JSON object to set as the aliasMap. 




        

<hr>



### function setCacheDir 

```C++
void SpiceQL::setCacheDir (
    std::string cache_dir,
    bool override=false
) 
```




<hr>



### function setConfigDirectory 

_Override the directory pointing to the db files._ 
```C++
void SpiceQL::setConfigDirectory (
    std::string configDir
) 
```



Sets an explicit config directory that takes precedence over the $SPICEQL\_CONFIG\_DIR env var and the default install/dev locations. This lets callers point [**SpiceQL**](namespaceSpiceQL.md) at its configs independent of the active conda environment. Pass an empty string to clear the override and fall back to the default resolution.




**Parameters:**


* `configDir` directory containing db files 



**See also:** [**getConfigDirectory**](namespaceSpiceQL.md#function-getconfigdirectory) 



        

<hr>



### function spiceAPIQuery 

_Query implementation for_ [_**SpiceQL**_](namespaceSpiceQL.md) _'s REST API._
```C++
nlohmann::json SpiceQL::spiceAPIQuery (
    std::string functionName,
    nlohmann::json args,
    std::string method="GET"
) 
```





**Parameters:**


* `functionName` [**SpiceQL**](namespaceSpiceQL.md)'s REST API endpoint name 
* `args` Endpoint query params as json 
* `method` REST HTTP method type as string 



**Returns:**

Response payload as json 





        

<hr>



### function split 

_turn a string into a vector with a deliminator_ 
```C++
std::vector< std::string > SpiceQL::split (
    const std::string & s,
    char delim
) 
```





**Parameters:**


* `s` input string 
* `delim` char deliminator 



**Returns:**

std::vector&lt;std::string&gt; 





        

<hr>



### function strSclkToEt 

_Converts a given string spacecraft clock time to an ephemeris time._ 
```C++
std::pair< double, nlohmann::json > SpiceQL::strSclkToEt (
    int frameCode,
    std::string sclk,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Given a known frame code strSclkToEt converts a given spacecraft clock time as a string to an ephemeris time. Call this function if your clock time looks something like: 1/281199081:48971




**Parameters:**


* `frameCode` int Frame id to use 
* `sclk` string Spacecraft Clock formatted as a string 
* `mission` string Mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load 
 



**Returns:**

double 





        

<hr>



### function toLower 

_force a string to lower case_ 
```C++
std::string SpiceQL::toLower (
    std::string s
) 
```





**Parameters:**


* `s` input string 



**Returns:**

copy of input string, in lower case 





        

<hr>



### function toUpper 

_force a string to upper case_ 
```C++
std::string SpiceQL::toUpper (
    std::string s
) 
```





**Parameters:**


* `s` input string 



**Returns:**

copy of input string, in upper case 





        

<hr>



### function translateCodeToName 

_Switch between NAIF frame integer code to string frame name._ 
```C++
std::pair< std::string, nlohmann::json > SpiceQL::translateCodeToName (
    int frame,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



See [NAIF's Docs on frame codes](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html) for more information




**Parameters:**


* `frame` int NAIF frame code to translate 
* `searchKernels` bool Whether to search the kernels for the user 
* `mission` Mission name as it relates to the config files 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

string Naif frame name 





        

<hr>



### function translateNameToCode 

_Switch between NAIF frame string name to integer frame code._ 
```C++
std::pair< int, nlohmann::json > SpiceQL::translateNameToCode (
    std::string frame,
    std::string mission="",
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



See [NAIF's Docs on frame codes](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html) for more information




**Parameters:**


* `frame` String frame name to translate to a NAIF code 
* `mission` Mission name as it relates to the config files 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 
* `kernelList` vector&lt;string&gt; vector of additional kernels to load



**Returns:**

integer Naif frame code 





        

<hr>



### function unload 

```C++
void SpiceQL::unload (
    std::string path
) 
```




<hr>



### function url\_encode 

_URL encodes a given string._ 
```C++
std::string SpiceQL::url_encode (
    const std::string & value
) 
```





**Parameters:**


* `value` The string to encode. 



**Returns:**

The encoded string. 





        

<hr>



### function utcToEt 

_convert a UTC string to an ephemeris time_ 
```C++
std::pair< double, nlohmann::json > SpiceQL::utcToEt (
    std::string utc,
    bool useWeb=false,
    bool searchKernels=true,
    bool fullKernelPath=false,
    int limitCk=-1,
    int limitSpk=1,
    std::vector< std::string > kernelList={}
) 
```



Basically a wrapper around NAIF's cspice str2et function except it also temporarily loads the required kernels. See Also: [https://naif.jpl.nasa.gov/pub/naif/toolkit\_docs/C/cspice/str2et\_c.html](https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/str2et_c.html)




**Parameters:**


* `et` UTC string, e.g. "1988 June 13, 12:29:48 TDB" 
* `searchKernels` bool Whether to search the kernels for the user 
* `fullKernelPath` bool if true returns full kernel paths, default returns relative paths 
* `limitCk` int number of cks to limit to, default is -1 to retrieve all 
* `limitSpk` int number of spks to limit to, default is 1 to retrieve only one 



**Returns:**

double precision ephemeris time 





        

<hr>



### function writeCk 

_Write CK segments to a file._ 
```C++
void SpiceQL::writeCk (
    std::string fileName,
    std::vector< std::vector< double > > quats,
    std::vector< double > times,
    int bodyCode,
    std::string referenceFrame,
    std::string segmentId,
    std::vector< std::vector< double > > angularVelocity={},
    std::string comment=""
) 
```



Given orientations, angular velocities, times, target and reference frames, write data as a segment in a CK kernel.




**Parameters:**


* `fileName` path to file to write the segment to 
* `quats` nx4 vector of orientations as quaternions 
* `times` nx1 vector of encoded SCLK ticks 
* `bodyCode` NAIF body code identifying the orientations belong to 
* `referenceFrame` NAIF string for the reference frame the orientations are in 
* `segmentId` Some ID to give the segment 
* `angularVelocity` optional, nx3 array of angular velocities 
* `comment` optional, comment to be associated with the segment 




        

<hr>



### function writeCk 

_Write CK segments to a file._ 
```C++
void SpiceQL::writeCk (
    std::string fileName,
    std::vector< CkSegment > segments
) 
```



Given orientations, angular velocities, times, target and reference frames, write data as a segment in a CK kernel. The segments' times must already be encoded SCLK ticks.




**Parameters:**


* `fileName` path to file to write the segment to 
* `segments` spkSegments to be writte 




        

<hr>



### function writeComment 

```C++
void SpiceQL::writeComment (
    std::string fileName,
    std::string comment
) 
```




<hr>



### function writeSpk 

_Write SPK segments to a file._ 
```C++
void SpiceQL::writeSpk (
    std::string fileName,
    std::vector< SpkSegment > segments
) 
```



Given a vector of SPK segments, write them to the requested SPK file.




**Parameters:**


* `fileName` file specification to have the SPK segments written to 
* `segments` spkSegments to be written 




        

<hr>



### function writeSpk 

_Write SPK to path._ 
```C++
void SpiceQL::writeSpk (
    std::string fileName,
    std::vector< std::vector< double > > statePositions,
    std::vector< double > stateTimes,
    int bodyCode,
    int centerOfMotion,
    std::string referenceFrame,
    std::string segmentId,
    int polyDegree,
    std::vector< std::vector< double > > stateVelocities={},
    std::string segmentComment=""
) 
```





**Parameters:**


* `fileName` full path to file to write the segment to 
* `statePositions` Nx3 array of positions in X,Y,Z order 
* `stateTimes` Nx1 array of times 
* `bodyCode` NAIF integer code for the body the states belong to 
* `centerOfMotion` is the NAIF integer code for the center of motion of the object identified by body. 
* `referenceFrame` The NAIF code the states are relative to 
* `segmentId` ID for the segment 
* `polyDegree` degree of the hermite polynomials used for interpolation 
* `stateVelocities` Nx3 array of state velocities in VX, VY, VZ order, optional 
* `segmentComment` Comment associated with the segment, optional 




        

<hr>



### function writeTextKernel 

_Write json key value pairs into a NAIF text kernel._ 
```C++
void SpiceQL::writeTextKernel (
    std::string fileName,
    std::string type,
    nlohmann::json & keywords,
    std::string comment=""
) 
```





**Parameters:**


* `fileName` pull path to the text kernel 
* `type` kernel type string, valid text kernel types: FK, IK, LSK, MK, PCK, SCLK 
* `comment` the comment to add to the top of the kernel 
* `keywords` json object containing key/value pairs to write to the text kernel 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `SpiceQL/include/SpiceQL/alias_map.h`

