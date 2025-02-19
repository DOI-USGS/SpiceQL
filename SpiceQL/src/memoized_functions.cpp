
#include "memo.h"
#include "memoized_functions.h"

#include "spice_types.h"

using json = nlohmann::json;
using namespace std;

namespace SpiceQL {

  vector<pair<double, double>> Memo::getTimeIntervals(string kpath) {
    Memory c;
    static auto func_memoed = make_memoized(c, "spiceql_getTimeIntervals", SpiceQL::getTimeIntervals);
    return func_memoed(kpath); 
  }


  string Memo::globTimeIntervals(string mission) { 
    Memory c;
    SPDLOG_TRACE("Calling globTimeIntervals via cache");
    static auto func_memoed = make_memoized(c, "spiceql_globTimeIntervals", SpiceQL::globTimeIntervals);
    return func_memoed(mission);
  }


  vector<vector<string>> Memo::getPathsFromRegex (string root, vector<string> regexes) { 
    Memory c;
    SPDLOG_TRACE("Calling globTimeIntervals via cache");
    static auto func_memoed = make_memoized(c, "spiceql_getPathsFromRegex", SpiceQL::getPathsFromRegex);
    return func_memoed(root, regexes); 
  }


  vector<string> Memo::ls(string const & root, bool recursive) {
    Memory c;
    SPDLOG_TRACE("Calling ls via cache");
    static auto func_memoed = make_memoized(c, "spiceql_ls", SpiceQL::ls);
    return func_memoed(root, recursive);
  }

}