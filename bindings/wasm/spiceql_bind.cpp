/**
 * @file
 *
 * Emscripten/Embind bindings exposing SpiceQL's public API (api.h) to JavaScript.
 *
 * Design (see the WASM plan, §6):
 *  - A recursive nlohmann::json <-> emscripten::val converter marshals JSON args
 *    and return values as native JS objects/arrays (not dumped strings).
 *  - std::vector<...> parameters are accepted as JS arrays via register_vector.
 *  - Every api.h function returns std::pair<T, json>; each is wrapped so JS
 *    receives an object { result, kernels }.
 *
 * The heavy defaulted-parameter surface of api.h is kept ergonomic by exposing,
 * for each function, a single wrapper that takes the required positional args
 * plus a JS "options" object (emscripten::val) carrying the optional params.
 * Callers omit whatever they don't set; defaults mirror api.h.
 */

#include <string>
#include <vector>
#include <utility>
#include <limits>

#include <memory>

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/val.h>
#include <nlohmann/json.hpp>

#include <SpiceQL/api.h>
#include <SpiceQL/spice_types.h>
#include <SpiceQL/utils.h>

using json = nlohmann::json;
using emscripten::val;

// Throw a real JS Error (with message) across the wasm boundary. Embind
// otherwise surfaces C++ exceptions to JS as opaque numeric pointers, so we
// catch std::exception in each wrapper and rethrow through this helper.
EM_JS(void, spiceql_throw_js, (const char *msg), {
  throw new Error(UTF8ToString(msg));
});

namespace {

// Run f() and translate any C++ exception into a JS Error. Used by every
// api.h wrapper so JS callers get catchable errors with readable messages
// (e.g. "Kernel search is unavailable in the WASM build ...").
template <typename F>
auto guard(F &&f) -> decltype(f()) {
  try {
    return f();
  } catch (const std::exception &e) {
    spiceql_throw_js(e.what());
  } catch (...) {
    spiceql_throw_js("Unknown SpiceQL error");
  }
  return decltype(f())();  // unreachable; spiceql_throw_js does not return
}

// ---- json <-> emscripten::val -------------------------------------------------

val jsonToVal(const json &j) {
  switch (j.type()) {
    case json::value_t::null:
      return val::null();
    case json::value_t::boolean:
      return val(j.get<bool>());
    case json::value_t::number_integer:
      return val(j.get<int64_t>());
    case json::value_t::number_unsigned:
      return val(j.get<uint64_t>());
    case json::value_t::number_float:
      return val(j.get<double>());
    case json::value_t::string:
      return val(j.get<std::string>());
    case json::value_t::array: {
      val arr = val::array();
      for (size_t i = 0; i < j.size(); ++i) {
        arr.call<void>("push", jsonToVal(j[i]));
      }
      return arr;
    }
    case json::value_t::object: {
      val obj = val::object();
      for (auto it = j.begin(); it != j.end(); ++it) {
        obj.set(it.key(), jsonToVal(it.value()));
      }
      return obj;
    }
    default:
      return val::null();
  }
}

json valToJson(const val &v) {
  const std::string t = v.typeOf().as<std::string>();
  if (v.isNull() || v.isUndefined()) {
    return json(nullptr);
  }
  if (t == "boolean") {
    return json(v.as<bool>());
  }
  if (t == "number") {
    // Preserve integers where possible, otherwise fall back to double.
    double d = v.as<double>();
    if (d == static_cast<double>(static_cast<int64_t>(d))) {
      return json(static_cast<int64_t>(d));
    }
    return json(d);
  }
  if (t == "string") {
    return json(v.as<std::string>());
  }
  if (v.isArray()) {
    json arr = json::array();
    const size_t n = v["length"].as<size_t>();
    for (size_t i = 0; i < n; ++i) {
      arr.push_back(valToJson(v[i]));
    }
    return arr;
  }
  if (t == "object") {
    json obj = json::object();
    const val keys = val::global("Object").call<val>("keys", v);
    const size_t n = keys["length"].as<size_t>();
    for (size_t i = 0; i < n; ++i) {
      const std::string key = keys[i].as<std::string>();
      obj[key] = valToJson(v[key]);
    }
    return obj;
  }
  return json(nullptr);
}

// ---- option-object helpers ----------------------------------------------------

bool hasKey(const val &opts, const char *key) {
  return !opts.isNull() && !opts.isUndefined() && opts.hasOwnProperty(key);
}

std::string optStr(const val &opts, const char *key, const std::string &def) {
  return hasKey(opts, key) ? opts[key].as<std::string>() : def;
}

bool optBool(const val &opts, const char *key, bool def) {
  return hasKey(opts, key) ? opts[key].as<bool>() : def;
}

int optInt(const val &opts, const char *key, int def) {
  return hasKey(opts, key) ? opts[key].as<int>() : def;
}

std::vector<std::string> optStrVec(const val &opts, const char *key,
                                   const std::vector<std::string> &def) {
  if (!hasKey(opts, key)) return def;
  const val a = opts[key];
  std::vector<std::string> out;
  const size_t n = a["length"].as<size_t>();
  for (size_t i = 0; i < n; ++i) out.push_back(a[i].as<std::string>());
  return out;
}

std::vector<double> valToDoubleVec(const val &a) {
  std::vector<double> out;
  const size_t n = a["length"].as<size_t>();
  for (size_t i = 0; i < n; ++i) out.push_back(a[i].as<double>());
  return out;
}

std::vector<std::string> valToStrVec(const val &a) {
  std::vector<std::string> out;
  if (a.isNull() || a.isUndefined()) return out;
  const size_t n = a["length"].as<size_t>();
  for (size_t i = 0; i < n; ++i) out.push_back(a[i].as<std::string>());
  return out;
}

const std::vector<std::string> kDefaultQualities = {"smithed", "reconstructed"};

// Wrap std::pair<T, json> into a JS { result, kernels } object.
template <typename T>
val wrapPair(const std::pair<T, json> &p) {
  val out = val::object();
  out.set("result", val(p.first));
  out.set("kernels", jsonToVal(p.second));
  return out;
}

// Overloads for the pair result types that need json/vector marshalling.
val wrapPair(const std::pair<json, json> &p) {
  val out = val::object();
  out.set("result", jsonToVal(p.first));
  out.set("kernels", jsonToVal(p.second));
  return out;
}

template <typename V>
val wrapVecPair(const std::pair<V, json> &p) {
  val arr = val::array();
  for (const auto &e : p.first) {
    // e may be scalar or a nested vector; jsonToVal handles both via json round-trip.
    arr.call<void>("push", jsonToVal(json(e)));
  }
  val out = val::object();
  out.set("result", arr);
  out.set("kernels", jsonToVal(p.second));
  return out;
}

// ---- api.h wrappers -----------------------------------------------------------
// Each wrapper takes the function's required positional args plus a JS options
// object for the optional trailing params (mission, qualities, useWeb, ...).

val getTargetStates_w(const val &ets, std::string target, std::string observer,
                      std::string frame, std::string abcorr, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getTargetStates(
      valToDoubleVec(ets), target, observer, frame, abcorr,
      optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optStrVec(opts, "spkQualities", kDefaultQualities),
      optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val getTargetStatesRanged_w(double startEt, double stopEt, int numRecords,
                            std::string target, std::string observer,
                            std::string frame, std::string abcorr, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getTargetStatesRanged(
      startEt, stopEt, numRecords, target, observer, frame, abcorr,
      optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optStrVec(opts, "spkQualities", kDefaultQualities),
      optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val getTargetOrientations_w(const val &ets, int toFrame, int refFrame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getTargetOrientations(
      valToDoubleVec(ets), toFrame, refFrame,
      optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val getTargetOrientationsRanged_w(double startEt, double stopEt, int numRecords,
                                  int toFrame, int refFrame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getTargetOrientationsRanged(
      startEt, stopEt, numRecords, toFrame, refFrame,
      optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val strSclkToEt_w(int frameCode, std::string sclk, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::strSclkToEt(
      frameCode, sclk, optStr(opts, "mission", ""),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val doubleSclkToEt_w(int frameCode, double sclk, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::doubleSclkToEt(
      frameCode, sclk, optStr(opts, "mission", ""),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val doubleEtToSclk_w(int frameCode, double et, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::doubleEtToSclk(
      frameCode, et, optStr(opts, "mission", ""),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val utcToEt_w(std::string utc, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::utcToEt(
      utc, optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val etToUtc_w(double et, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::etToUtc(
      et, optStr(opts, "format", ""),
      hasKey(opts, "precision") ? opts["precision"].as<double>() : 0.0,
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val translateNameToCode_w(std::string frame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::translateNameToCode(
      frame, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val translateCodeToName_w(int frame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::translateCodeToName(
      frame, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val getFrameInfo_w(int frame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getFrameInfo(
      frame, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val getTargetFrameInfo_w(int targetId, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getTargetFrameInfo(
      targetId, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val findMissionKeywords_w(std::string key, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::findMissionKeywords(
      key, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val findTargetKeywords_w(std::string key, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::findTargetKeywords(
      key, optStr(opts, "mission", ""), optBool(opts, "useWeb", false),
      optBool(opts, "searchKernels", true), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optStrVec(opts, "kernelList", {}));
  return wrapPair(r);
  });
}

val frameTrace_w(double et, int initialFrame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::frameTrace(
      et, initialFrame, optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optStrVec(opts, "spkQualities", kDefaultQualities),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val extractExactCkTimes_w(double observStart, double observEnd, int targetFrame,
                          const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::extractExactCkTimes(
      observStart, observEnd, targetFrame, optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", 1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val getExactTargetOrientations_w(double startEt, double stopEt, int toFrame,
                                 int refFrame, int exactCkFrame, const val &opts) {
  return guard([&]() -> val {
  auto r = SpiceQL::getExactTargetOrientations(
      startEt, stopEt, toFrame, refFrame, exactCkFrame, optStr(opts, "mission", ""),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optBool(opts, "useWeb", false), optBool(opts, "searchKernels", true),
      optBool(opts, "fullKernelPath", false), optInt(opts, "limitCk", -1),
      optInt(opts, "limitSpk", 1), optStrVec(opts, "kernelList", {}));
  return wrapVecPair(r);
  });
}

val searchForKernelsets_w(const val &spiceqlNames, const val &opts) {
  static const std::vector<std::string> kAllTypes = {
      "ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"};
  return guard([&]() -> val {
  auto r = SpiceQL::searchForKernelsets(
      valToStrVec(spiceqlNames),
      optStrVec(opts, "types", kAllTypes),
      hasKey(opts, "startTime") ? opts["startTime"].as<double>()
                                : -std::numeric_limits<double>::max(),
      hasKey(opts, "stopTime") ? opts["stopTime"].as<double>()
                               : std::numeric_limits<double>::max(),
      optStrVec(opts, "ckQualities", kDefaultQualities),
      optStrVec(opts, "spkQualities", kDefaultQualities),
      optBool(opts, "useWeb", false), optBool(opts, "fullKernelPath", false),
      optInt(opts, "limitCk", -1), optInt(opts, "limitSpk", 1),
      optBool(opts, "overwrite", false));
  return wrapPair(r);
  });
}

// ---- alias-map / utility wrappers ---------------------------------------------

std::string getSpiceqlName_w(std::string name) {
  return guard([&] { return SpiceQL::getSpiceqlName(name); });
}
void addAliasKey_w(std::string k, std::string v) {
  guard([&] { SpiceQL::addAliasKey(k, v); return 0; });
}
val getAliasMap_w() {
  return guard([&]() -> val { return jsonToVal(SpiceQL::getAliasMap()); });
}
void setAliasMap_w(const val &m) {
  guard([&] { SpiceQL::setAliasMap(valToJson(m)); return 0; });
}
std::string urlEncode_w(std::string v) {
  return guard([&] { return SpiceQL::url_encode(v); });
}

val spiceAPIQuery_w(std::string functionName, const val &args, std::string method) {
  return guard([&]() -> val {
    return jsonToVal(SpiceQL::spiceAPIQuery(functionName, valToJson(args), method));
  });
}

// Backs the naifspice namespace (bindings/wasm/naifspice.js). SpiceQL's
// checkNaifErrors() lazily sets the CSPICE error action to RETURN (CSPICE
// defaults to ABORT, which would exit() the whole module), throws a
// runtime_error carrying the SPICE short+long message when an error is
// signalled, and resets the toolkit so the module stays usable. guard() turns
// that throw into a real JS Error. naifspice.js calls this after every raw
// CSPICE call and once at load time to prime the RETURN action.
void naifCheckErrors_w() {
  guard([&] { SpiceQL::checkNaifErrors(); return 0; });
}

// ---- manual kernel-pool management --------------------------------------------
// These let JS callers furnish kernels once and reuse them across many api.h
// calls (pass searchKernels:false and an empty kernelList so the call reads the
// already-furnished pool). Mirrors SpiceQL's C++ load()/unload()/KernelSet.

void load_w(std::string path) {
  guard([&] { SpiceQL::load(path); return 0; });
}
void unload_w(std::string path) {
  guard([&] { SpiceQL::unload(path); return 0; });
}
val getLoadedKernels_w() {
  return guard([&]() -> val {
    val arr = val::array();
    for (const auto &k : SpiceQL::getLoadedKernels()) arr.call<void>("push", val(k));
    return arr;
  });
}
bool isLskLoaded_w() {
  return guard([&] { return SpiceQL::isLskLoaded(); });
}

// Turn the JS argument accepted by the KernelSet constructor / load() into the
// json shape SpiceQL::KernelSet expects. An array of paths is grouped by kernel
// type via formatKernels() (same helper the WASM inventory uses); an object is
// assumed to already be in {type: [paths]} form and passed through.
json kernelsArgToJson(const val &arg) {
  if (arg.isArray()) {
    return SpiceQL::formatKernels(valToStrVec(arg));
  }
  return valToJson(arg);
}

// RAII KernelSet exposed to JS as a class. Constructing it furnishes the
// kernels; calling unload() (or letting JS garbage-collect and .delete() it)
// unfurnishes them. While it is alive, api.h calls with searchKernels:false see
// its kernels in the pool.
class KernelSetJs {
 public:
  explicit KernelSetJs(const val &kernels) {
    guard([&] { m_set.load(kernelsArgToJson(kernels)); return 0; });
  }
  // Furnish additional kernels into this set (merged with what it already holds).
  void load(const val &kernels) {
    guard([&] { m_set.load(kernelsArgToJson(kernels)); return 0; });
  }
  // Unfurnish everything this set furnished.
  void unload() {
    guard([&] { m_set.unload(); return 0; });
  }
  // The kernels this set is tracking, as a { type: [paths] } object.
  val getKernels() const {
    return guard([&]() -> val { return jsonToVal(m_set.m_kernels); });
  }

 private:
  SpiceQL::KernelSet m_set;
};

}  // namespace

EMSCRIPTEN_BINDINGS(spiceql) {
  emscripten::register_vector<double>("VectorDouble");
  emscripten::register_vector<std::string>("VectorString");

  emscripten::function("getSpiceqlName", &getSpiceqlName_w);
  emscripten::function("addAliasKey", &addAliasKey_w);
  emscripten::function("getAliasMap", &getAliasMap_w);
  emscripten::function("setAliasMap", &setAliasMap_w);
  emscripten::function("urlEncode", &urlEncode_w);
  emscripten::function("spiceAPIQuery", &spiceAPIQuery_w);
  emscripten::function("naifCheckErrors", &naifCheckErrors_w);

  emscripten::function("getTargetStates", &getTargetStates_w);
  emscripten::function("getTargetStatesRanged", &getTargetStatesRanged_w);
  emscripten::function("getTargetOrientations", &getTargetOrientations_w);
  emscripten::function("getTargetOrientationsRanged", &getTargetOrientationsRanged_w);
  emscripten::function("strSclkToEt", &strSclkToEt_w);
  emscripten::function("doubleSclkToEt", &doubleSclkToEt_w);
  emscripten::function("doubleEtToSclk", &doubleEtToSclk_w);
  emscripten::function("utcToEt", &utcToEt_w);
  emscripten::function("etToUtc", &etToUtc_w);
  emscripten::function("translateNameToCode", &translateNameToCode_w);
  emscripten::function("translateCodeToName", &translateCodeToName_w);
  emscripten::function("getFrameInfo", &getFrameInfo_w);
  emscripten::function("getTargetFrameInfo", &getTargetFrameInfo_w);
  emscripten::function("findMissionKeywords", &findMissionKeywords_w);
  emscripten::function("findTargetKeywords", &findTargetKeywords_w);
  emscripten::function("frameTrace", &frameTrace_w);
  emscripten::function("extractExactCkTimes", &extractExactCkTimes_w);
  emscripten::function("getExactTargetOrientations", &getExactTargetOrientations_w);
  emscripten::function("searchForKernelsets", &searchForKernelsets_w);

  // Manual kernel-pool management.
  emscripten::function("load", &load_w);
  emscripten::function("unload", &unload_w);
  emscripten::function("getLoadedKernels", &getLoadedKernels_w);
  emscripten::function("isLskLoaded", &isLskLoaded_w);

  emscripten::class_<KernelSetJs>("KernelSet")
      .constructor<const val &>()
      .function("load", &KernelSetJs::load)
      .function("unload", &KernelSetJs::unload)
      .function("getKernels", &KernelSetJs::getKernels);
}
