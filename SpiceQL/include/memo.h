/**
 * Implements an almost transparent disk cache/memoization for C++ functions.
 * 
 * Originally adapted from: https://github.com/temporaer/cached_function 
 * 
 */
#ifndef memo_h
#define memo_h

#include <map>
#include <fstream>
#include <utility>
#include <functional>
#include <any>
#include <string>
#include <chrono>
#include <iomanip>

#include <ghc/fs_std.hpp>
#include <spdlog/spdlog.h>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/archives/json.hpp>

#include "memoized_functions.h"

#define CACHED(cache, func, ...) cache(#func, func, __VA_ARGS__)

namespace SpiceQL {
namespace Memo {

    template <class T>
    inline size_t _hash_combine(std::size_t& seed, const T& v) {
        SPDLOG_TRACE("_hash_combine seed: {}", seed);

        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);

        return seed;
    }

    inline size_t hash_combine(std::size_t& seed) {
        return seed;
    }

    template <typename T, typename... Params>
    inline size_t hash_combine(std::size_t& seed, const T& t, const Params&... params) {
        seed = _hash_combine(seed, t);
        return hash_combine(seed, params...);
    }

    template <typename TP>
    inline std::time_t to_time_t(TP tp) {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                + system_clock::now());
        return system_clock::to_time_t(sctp);
    }
    

    inline std::time_t time_from_str(std::string time) {
        static const char* TIME_FORMAT="%b %d %Y %H:%M:%S";
 
        // get last modified time
        std::tm tm = {};
        std::stringstream ss(time);
        ss >> std::get_time(&tm, TIME_FORMAT);
        std::time_t numerical_time = std::mktime(&tm);
        return numerical_time; 
    }


    inline bool has_cache_expired(time_t latest, std::vector<std::string> files) { 
        if (files.empty()) { 
            // if no files, cache cannot expire 
            return false;
        }

        std::vector<std::time_t> times(files.size());
        for (int i = 0; i<files.size(); i++) {
            if (!fs::exists(files.at(i)))  {
                // if dep doesn't exist anymore, that counts as expiring
                return true; 
            }
            std::time_t t = to_time_t(fs::last_write_time(files.at(i)));
            times.insert(times.begin()+i, t);
        }

        // if any of the files is newer than input time, the cache expired. 
        return std::any_of(times.begin(), times.end(), [&latest](std::time_t t){return latest < t;});
    }


    inline std::string getCacheDir() { 
        static std::string  CACHE_DIRECTORY = "";
	// return "/home/ec2-user/efs_prod/spiceql_cache/";
        if (CACHE_DIRECTORY == "") { 
            const char* cache_dir_char = getenv("SPICEQL_CACHE_DIR");
        
            std::string cache_dir; 
        
            if (cache_dir_char == NULL) {
                std::string  tempname = "spiceql-cache-" + gen_random(10);
                cache_dir = (fs::temp_directory_path() / tempname / "spiceql_cache").to_string(); 
            }
            else {
                cache_dir = cache_dir_char;
            }

            if (!fs::is_directory(cache_dir)) { 
                SPDLOG_DEBUG("{} does not exist, attempting to create the directory", cache_dir);
                fs::create_directories(cache_dir); 
            }
        
            CACHE_DIRECTORY = cache_dir;
            SPDLOG_DEBUG("Setting cache directory to: {}", CACHE_DIRECTORY);  
        } 
        else { 
            SPDLOG_TRACE("Cache Directory Already Set: {}", CACHE_DIRECTORY);  
        }

        return CACHE_DIRECTORY;
    }

    class Cache {
       public:
        // if cache is older than this directory, then the cache is reloaded

        std::vector<std::string> m_dependants;
        
        Cache(std::vector<std::string> deps)
        :  m_dependants(deps)  {
        }


        template<typename Func, typename... Params>
        auto operator()(const Func& f, Params&&... params) -> decltype(f(params...))const{
            return this->operator()("anonymous", f, std::forward<Params>(params)...);
        }


        template<typename Func, typename... Params>
        auto operator()(const std::string& descr, const Func& f, Params&&... params) -> decltype(f(params...))const {
            std::size_t seed = 0; 
            hash_combine(seed, descr, params...);
            return this->operator()(descr, seed, f, std::forward<Params>(params)...);
        }


        template<typename Func, typename... Params>
        auto operator()(const std::string& descr, std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...))const {
            static const char* TIME_FORMAT="%b %d %Y %H:%M:%S";

            return use_disk_cache(descr, seed, f, params...);
        }

        // TODO: this is jank, make it less jank
        template<typename Func, typename... Params>
        auto use_disk_cache(const std::string& descr, std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...))const{
                typedef decltype(f(params...)) retval_t;
                std::string name = descr + "-" + std::to_string(seed);
                
                SPDLOG_TRACE("Cache name: {}", name);

                std::string fn = (fs::path(getCacheDir()) / name).string();
                
                SPDLOG_TRACE("cache full path: {}", fn);
                
                if(fs::exists(fn)) {
                    std::time_t cache_write_time = to_time_t(fs::last_write_time(fn));                   
                    if (!has_cache_expired(cache_write_time, m_dependants)) { 
                        SPDLOG_TRACE("Cached access of {}", name);
                        std::ifstream ifs(fn);
                        cereal::BinaryInputArchive  ia(ifs);
                        retval_t ret;
                        ia >> ret;
                        return ret;
                    }
                    else { 
                        // delete and reload cache
                        SPDLOG_TRACE("Cache {} has expired", fn);
                        fs::remove(fn);
                    }
                }
                // if dependant doesn't exist, treat it as a cache miss. Function might naturally fail.
                
                SPDLOG_TRACE("Non-cached access, creating cache {}", fn);
                retval_t ret = f(std::forward<Params>(params)...);
                std::ofstream ofs(fn);
                cereal::BinaryOutputArchive oa(ofs);
                oa << ret;

                return ret;
            }
    };
    

    class Memory {
        public: 
        mutable std::map<std::size_t, std::any> m_data;

        template<typename Func, typename... Params>
            auto operator()(const Func& f, Params&&... params) -> decltype(f(params...)) const {
                return (*this)("anonymous", f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(std::string descr, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                std::size_t seed = 0; 
                hash_combine(seed, descr, params...);
                return (*this)(seed, f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(const std::string& descr, std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                hash_combine(seed, descr);
                return (*this)(seed, f, std::forward<Params>(params)...);
            }
        template<typename Func, typename... Params>
            auto operator()(std::size_t seed, const Func& f, Params&&... params) -> decltype(f(params...)) const {
                typedef decltype(f(params...)) retval_t;
                auto it = m_data.find(seed);
                if(it != m_data.end()){
                    SPDLOG_TRACE("Cached access from memory");
                    return std::any_cast<retval_t>(it->second);
                }
                retval_t ret = f(std::forward<Params>(params)...);
                SPDLOG_TRACE("Non-cached access");
                m_data[seed] = ret;
                return ret;
            }
    };


    template<typename Cache, typename Function>
    struct memoize{
        const Function m_func; // we require copying the function object here.
        std::string m_id;
        Cache m_fc;
        memoize(Cache& fc, std::string id, const Function& f)
            :m_func(f), m_id(id), m_fc(fc){}
        template<typename... Params>
        auto operator()(Params&&... args) 
                -> decltype(std::bind(m_func, args...)()) {
            SPDLOG_TRACE("Calling a memo func with ID: {}", m_id);
            return m_fc(m_id, m_func, std::forward<Params>(args)...);
        }
    };
      
    template<typename Cache, typename Function>
    memoize<Cache, Function>
    make_memoized(Cache& fc, const std::string& id, Function f){
        SPDLOG_TRACE("Making function with ID {}",id);   
        return memoize<Cache, Function>(fc, id, f);
    }
 
}
}

namespace std {
  // literally have no idea why I need to make this specialization... can't compile otherwise
  template <> struct hash<std::vector<std::string >(const std::string&, bool)> {
    size_t operator()(const auto & x) const {
      return 0;
    }
  };
}

template<>
struct std::hash<std::vector<std::string>>
{
    std::size_t operator()(vector<string> const& vec) const noexcept {
        std::size_t seed = 0;  
        std::hash<std::string> hasher;

        for(auto &e : vec) { 
           seed = SpiceQL::Memo::hash_combine(seed, hasher(e)); 
        }
        return seed;
    }
};

#endif
