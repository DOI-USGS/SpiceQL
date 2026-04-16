#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace SpiceQL {

  class AliasMap {
    public:
      /**
       * @brief Accessor for AliasMap instance
       * 
       * @return A reference to the global AliasMap instance
       */
      static AliasMap& instance();

      /**
       * @brief Loads an alias map from a JSON file path
       * 
       * @param path Filesystem path to the JSON file 
       */
      void load(std::string path);

      /**
       * @brief Gets SpiceQL name given alias or frame name
       * 
       * @param name Given string name to look up
       * @return The SpiceQL name or empty string if not found
       */
      std::string getSpiceqlName(const std::string& name);

      /**
       * @brief Adds an alias to the lookup table for the given SpiceQL name
       * 
       * @param alias Alias string for SpiceQL name
       * @param spiceqlName The target SpiceQL name
       */
      void addAliasKey(const std::string &alias, const std::string &spiceqlName);

      /**
       * @brief Retrieves the alias map lookup table as JSON object
       * 
       * @return A JSON object where keys are SpiceQL names and the values are an array of aliases
       */
      nlohmann::json getAliasMap();

      /**
       * @brief Overwrites the current lookup table with a user-provided JSON object
       * 
       * @param newAliasMap The alias map as a JSON object
       */
      void setAliasMap(const nlohmann::json& newAliasMap);

    private:
      AliasMap() = default; // Prevents others from making new instances

      /**
       * @brief Checks initialization status and loads default aliases if necessary
       */
      void ensure_init();

      /**
       * @brief Internal logic for parsing JSON and populating the lookup table without locking
       * 
       * @param path Path to the JSON file to load
       */
      void load_internal(std::string path);

      // These live inside the class so the Singleton "owns" them
      std::unordered_map<std::string, std::string> m_lookupTable;
      std::mutex m_mutex;
      bool m_initialized = false;
  };

  /**
   * @brief Free function to trigger loading aliases
   * 
   * @param path Optional path to a specific JSON file
   */
  void load_aliases(std::string path = "");

}
