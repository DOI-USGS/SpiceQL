#include <SpiceQL/alias_map.h>
#include <SpiceQL/config.h>
#include <SpiceQL/utils.h>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <SpiceQL/spiceql_logging.h>

using json = nlohmann::json;
using namespace std;

namespace SpiceQL {

  AliasMap& AliasMap::instance() {
    static AliasMap inst;
    return inst;
  }

  void AliasMap::ensure_init() {
    if (!m_initialized) {
      SPDLOG_DEBUG("Loading default aliases.");
      load_internal(""); 
    }
  }

  void load_aliases(string path) {
    AliasMap::instance().load(path);
  }

  void AliasMap::load_internal(string path) {    
    if (path.empty()) {
      path = getAliasMapJsonFile(); 
    }

    SPDLOG_INFO("Loading aliases from {}", path);
    ifstream file(path);
    if (!file.is_open()) {
      throw runtime_error("Failed to open alias file: " + path);
    }

    nlohmann::json j;
    file >> j;

    m_lookupTable.clear();
    for (auto& [mission, aliases] : j.items()) {
      for (const string& alias : aliases) {
        m_lookupTable[toUpper(alias)] = mission;
      }
    }
    
    m_initialized = true;
  }

  void AliasMap::load(string path) {
    lock_guard<mutex> lock(m_mutex);
    load_internal(path);
  }

  string AliasMap::getSpiceqlName(const string& name) {
    lock_guard<mutex> lock(m_mutex);
    ensure_init();

    string upperAlias = toUpper(name);
    auto it = m_lookupTable.find(upperAlias);
    if (it != m_lookupTable.end()) {
        return it->second;
    }

    // Fallback: match frameList() case-insensitively, returning the canonical
    // config key (e.g. NAIF's "LRO" -> "lro"). Indexed by uppercased key for
    // O(1) lookup.
    unordered_map<string, string> frames;
    for (const auto& frame : frameList()) {
        frames.emplace(toUpper(frame), frame);
    }
    auto fit = frames.find(upperAlias);
    if (fit != frames.end()) return fit->second;

    return "";
  }

  void AliasMap::addAliasKey(const std::string &alias, const std::string &spiceqlName) {
    lock_guard<mutex> lock(m_mutex);
    ensure_init();
    m_lookupTable[toUpper(alias)] = spiceqlName;
  }

  nlohmann::json AliasMap::getAliasMap() {
    lock_guard<mutex> lock(m_mutex);
    ensure_init();

    // Unflatten and reformat as original JSON formatting
    nlohmann::json aliasMapJson;
    for (auto const& [alias, key] : m_lookupTable) {
      aliasMapJson[key].push_back(alias);
    }
    return aliasMapJson;
  }

  void AliasMap::setAliasMap(const nlohmann::json& newAliasMap) {
    lock_guard<mutex> lock(m_mutex);
    
    m_lookupTable.clear();

    if (!newAliasMap.is_object()) {
      throw runtime_error("Provided alias map must be a JSON object.");
    }

    for (auto& [mission, aliases] : newAliasMap.items()) {
      if (aliases.is_array()) {
        for (const auto& alias_val : aliases) {
          if (alias_val.is_string()) {
            m_lookupTable[toUpper(alias_val.get<string>())] = mission;
          }
        }
      }
    }
    
    m_initialized = true; 
    SPDLOG_INFO("Alias map manually updated with {} entries.", m_lookupTable.size());
  }

}
