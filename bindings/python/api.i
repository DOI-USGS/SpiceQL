%module(package="pyspiceql") api

%{
  #include <SpiceQL/api.h>
%}

// wrap struct members by value so python lists can be assigned to vector members
%naturalvar SpiceQL::SpiceQLOptions::kernelList;

// The SpiceQLOptions overloads in api.h are hidden from SWIG since python loses
// keyword args on overloaded functions. Instead each function takes
// options=SpiceQLOptions(...) (or a dict), spread into its option keyword args.
%define SPICEQL_OPTIONS_KWARG(name, keys)
%pythonprepend SpiceQL::name %{
    kwargs = _unpack_options(kwargs, keys)
%}
%enddef

SPICEQL_OPTIONS_KWARG(getTargetStates, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getTargetStatesRanged, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getTargetOrientations, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getTargetOrientationsRanged, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(strSclkToEt, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(doubleSclkToEt, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(doubleEtToSclk, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(utcToEt, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(etToUtc, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(doubleEtsToSclkTicks, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(translateNameToCode, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(translateCodeToName, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getFrameInfo, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getTargetFrameInfo, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(findMissionKeywords, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(findTargetKeywords, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(frameTrace, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(extractExactCkTimes, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(getExactTargetOrientations, _OPTION_KEYS)
SPICEQL_OPTIONS_KWARG(searchForKernelsets, ("useWeb", "fullKernelPath", "limitCk", "limitSpk"))

%include <SpiceQL/api.h>

// SpiceQLOptions({"limitCk": 2, ...}) and options.toJson(), using the
// to_json/from_json defined in api.h; missing keys keep their defaults.
// j defaults to {} because declaring a constructor here hides SWIG's implicit
// default constructor
%extend SpiceQL::SpiceQLOptions {
  SpiceQLOptions(nlohmann::json j=nlohmann::json::object()) {
    return new SpiceQL::SpiceQLOptions(j.get<SpiceQL::SpiceQLOptions>());
  }

  nlohmann::json toJson() {
    return *$self;
  }
}

%pythoncode %{
_OPTION_KEYS = ("useWeb", "searchKernels", "fullKernelPath", "limitCk", "limitSpk", "kernelList")

def _unpack_options(kwargs, keys=_OPTION_KEYS):
    options = kwargs.pop("options", None)
    if options is None:
        return kwargs
    if not isinstance(options, SpiceQLOptions):
        options = SpiceQLOptions(options)
    options = options.toJson()
    for key in keys:
        if key in kwargs:
            raise TypeError(f"{key} given both as a keyword argument and in options")
        kwargs[key] = options[key]
    return kwargs
%}

