%module pyspiceql

%feature("autodoc", "3");

%include "std_vector.i"
%include "std_string.i"
%include "std_array.i"
%include "std_map.i"
%include "carrays.i"
%include "std_pair.i"

// #include <nlohmann/json.hpp>

%{
  #include <array>
  #include <vector> 
  #include <nlohmann/json.hpp>
%}

%template(DoublePair) std::pair<double, double>;

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

%typemap(out) nlohmann::json& {
  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");
  std::string jsonString = ($1)->dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);
  $result = PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL);
}

%typemap(in) nlohmann::json& {
  if (PyDict_Check($input) || PyList_Check($input)) {
    PyObject* module = PyImport_ImportModule("json");
    PyObject* jsonDumps = PyUnicode_FromString("dumps");
    PyObject* pythonJsonString = PyObject_CallMethodObjArgs(module, jsonDumps, $input, NULL);
    $1 = new nlohmann::json(nlohmann::json::parse(PyUnicode_AsUTF8(pythonJsonString)));
  }
  else {
    PyErr_SetString(PyExc_TypeError, "not a json serializable type");
    SWIG_fail;
  }
}

// pair<vector<vector<double>>, json>
%typemap(out) std::pair<std::vector<std::vector<double>>, nlohmann::json> (PyObject* _inner,PyObject* _outer) {
  _outer = PyList_New($1.first.size());
  for (size_t i = 0; i < $1.first.size(); ++i) {
    _inner = PyList_New($1.first[i].size());
    for (size_t j = 0; j < $1.first[i].size(); ++j){
      PyList_SetItem(_inner,j,PyFloat_FromDouble($1.first[i][j]));
    }
    PyList_SetItem(_outer,i,_inner);
  }

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, _outer);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<vector<vector<int>>, json>
%typemap(out) std::pair<std::vector<std::vector<int>>, nlohmann::json> (PyObject* _inner,PyObject* _outer) {
  _outer = PyList_New($1.first.size());
  for (size_t i = 0; i < $1.first.size(); ++i) {
    _inner = PyList_New($1.first[i].size());
    for (size_t j = 0; j < $1.first[i].size(); ++j){
      PyList_SetItem(_inner,j,PyInt_FromLong($1.first[i][j]));
    }
    PyList_SetItem(_outer,i,_inner);
  }

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, _outer);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<vector<double>, json>
%typemap(out) std::pair<std::vector<double>, nlohmann::json> {
  PyObject* vec_list = PyList_New($1.first.size());
  for (size_t i = 0; i < $1.first.size(); ++i) {
      PyList_SetItem(vec_list, i, PyFloat_FromDouble($1.first[i]));
  }

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, vec_list);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<vector<int>, json>
%typemap(out) std::pair<std::vector<int>, nlohmann::json> {
  PyObject* vec_list = PyList_New($1.first.size());
  for (size_t i = 0; i < $1.first.size(); ++i) {
      PyList_SetItem(vec_list, i, PyInt_FromLong($1.first[i]));
  }

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, vec_list);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<double, json>
%typemap(out) std::pair<double, nlohmann::json> {
  PyObject* dblOut = PyFloat_FromDouble($1.first);

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, dblOut);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<int, json>
%typemap(out) std::pair<int, nlohmann::json> {
  PyObject* intOut = PyInt_FromLong($1.first);

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, intOut);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<string, json>
%typemap(out) std::pair<std::string, nlohmann::json> {
  PyObject* strOut = PyString_FromString($1.first.c_str());

  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString = $1.second.dump();
  PyObject* pythonJsonString = PyUnicode_DecodeUTF8(jsonString.c_str(), jsonString.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0, strOut);
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString, NULL));
}

// pair<json, json>
%typemap(out) std::pair<nlohmann::json, nlohmann::json> {
  PyObject* module = PyImport_ImportModule("json");
  PyObject* jsonLoads = PyUnicode_FromString("loads");

  std::string jsonString1 = $1.first.dump();
  std::string jsonString2 = $1.second.dump();
  PyObject* pythonJsonString1 = PyUnicode_DecodeUTF8(jsonString1.c_str(), jsonString1.size(), NULL);
  PyObject* pythonJsonString2 = PyUnicode_DecodeUTF8(jsonString2.c_str(), jsonString2.size(), NULL);

  $result = PyTuple_New(2);
  PyTuple_SetItem($result, 0,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString1, NULL));
  PyTuple_SetItem($result, 1,  PyObject_CallMethodObjArgs(module, jsonLoads, pythonJsonString2, NULL));
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

%typemap(out) std::map<std::string, std::string> {
  PyObject* dict = PyDict_New();
  for (auto it = $1.begin(); it != $1.end(); ++it) {
    PyObject* key = PyUnicode_FromString(it->first.c_str());
    PyObject* value = PyUnicode_FromString(it->second.c_str());
    PyDict_SetItem(dict, key, value);
    Py_DECREF(key);
    Py_DECREF(value);
  }
  $result = dict;
}

%typemap(in) std::map<std::string, std::string> {
  if (!PyDict_Check($input)) {
    PyErr_SetString(PyExc_TypeError, "Parameter must be a dict");
    SWIG_fail;
  }
  PyObject *key, *value;
  Py_ssize_t pos = 0;
  std::map<std::string, std::string> temp;
  while (PyDict_Next($input, &pos, &key, &value)) {
    if (!PyUnicode_Check(key) || !PyUnicode_Check(value)) {
      PyErr_SetString(PyExc_TypeError, "Keys and values must be strings");
      SWIG_fail;
    }
    temp[std::string(PyUnicode_AsUTF8(key))] = std::string(PyUnicode_AsUTF8(value));
  }
  $1 = temp;
}



%include "config.i"
%include "io.i"
%include "query.i"
%include "spice_types.i"
%include "utils.i"
%include "memoized_functions.i"
%include "inventory.i"
%include "api.i"