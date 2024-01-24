%module pyspiceql

%feature("autodoc", "3");

%include "std_vector.i"
%include "std_string.i"
%include "std_array.i"
%include "std_map.i"
%include "carrays.i"
%include "std_pair.i"

#include <nlohmann/json.hpp>

%{
  #include <array>
  #include <vector> 
%}

%typemap(in) nlohmann::json {
  if (PyDict_Check($input) || PyList_Check($input)) {
    PyObject* module = PyImport_ImportModule("json");
    PyObject* jsonDumps = PyUnicode_FromString("dumps");
    PyObject* pythonJsonString = PyObject_CallMethodObjArgs(module, jsonDumps, $input, NULL);
    $1 = nlohmann::json::parse(PyUnicode_AsUTF8(pythonJsonString));
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a json serializable type");
    SWIG_fail;
  }
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_MAP) nlohmann::json {
  $1 = PyDict_Check($input) ? 1 : 0;
}

%typemap(out) nlohmann::json {
  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");
  std::string jsonString = ($1).dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);
  $result = PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL);
}


namespace std {
  %template(IntVector) vector<int>;
  %template(DoubleVector) vector<double>;
  %template(VectorDoubleVector) vector< vector<double> >;
  %template(VectorIntVector) vector< vector<int> >;
  %template(StringVector) vector<string>;
  %template(VectorStringVector) vector< vector<string> >;
  %template(ConstCharVector) vector<const char*>;
  %template(PairDoubleVector) vector<pair<double, double>>;
  %template(DoubleArray6) array<double, 6>;
}

%exception {
  try {
    $action
  } catch (std::exception const& e) {
    SWIG_exception(SWIG_RuntimeError, (std::string("std::exception: ") + e.what()).c_str());
  } catch (...) {
    SWIG_exception(SWIG_UnknownError, "Unknown error");
  }
}

%include "config.i"
%include "io.i"
%include "query.i"
%include "spice_types.i"
%include "utils.i"
%include "memoized_functions.i"