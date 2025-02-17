#include <gtest/gtest.h>

#include <fmt/format.h>
#include <ghc/fs_std.hpp>
#include <chrono>

#include <string>

using namespace std::chrono;

#include "TestUtilities.h"

#include "memo.h"
#include "memoized_functions.h"
#include "spiceql.h"
#include "io.h"

#include <spdlog/spdlog.h>

using namespace SpiceQL;
using namespace std;

TEST(UtilTests, testHashCollisions) {
  std::string s1 = "/isisdata/mro/kernels/ck";
  std::string s2 = "/isisdata/mro/kernels/spk"; 

  size_t seed1 = 0;
  size_t seed2 = 0; 
  seed1 = Memo::_hash_combine(seed1, s1);
  seed2 = Memo::_hash_combine(seed2, s2);
  SPDLOG_DEBUG("seed1 {}", seed1);
  SPDLOG_DEBUG("seed2 {}", seed2);
 
  EXPECT_NE(seed1, seed2);

  seed1 = 0;
  seed2 = 0;

  seed1 = Memo::hash_combine(seed1, s1, false);
  seed2 = Memo::hash_combine(seed2, s1, true);
  SPDLOG_DEBUG("seed1 {}", seed1);
  SPDLOG_DEBUG("seed2 {}", seed2);

  EXPECT_NE(seed1, seed2);
}


TEST(UtilTests, testGetKernelTimes) {  
  // fs::path temp_dir = fs::temp_directory_path();
  // fs::path ck_path = temp_dir / "testck.bsp";
  
  fs::path path = static_cast<fs::path>(getenv("SPICEROOT")) / "test_ck.bsp";
  fs::path lskPath = fs::path("data") / "naif0012.tls"; 
  fs::path sclkPath = fs::path("data") / "lro_clkcor_2020184_v00.tsc";

  std::vector<std::vector<double>> orientations = {{0.2886751, 0.2886751, 0.5773503, 0.7071068 }, {0.4082483, 0.4082483, 0.8164966, 0 }};
  std::vector<std::vector<double>> av = {{1,1,1}, {1,2,3}};
  std::vector<double> times = {110000000, 120000001};
  int bodyCode = -85000; 
  std::string referenceFrame = "j2000";
  std::string segmentId = "CKCKCK";

  writeCk(path, orientations, times, bodyCode, referenceFrame, segmentId, sclkPath, lskPath, av);

  Kernel lsk(lskPath);
  Kernel sclk(sclkPath);
  std::vector<std::pair<double, double>> v_nonmemo = getTimeIntervals(path);

  SPDLOG_DEBUG("non-cached times");
  for (auto &e : v_nonmemo) { 
    SPDLOG_DEBUG("{}, {}", e.first, e.second);
  }  

  std::vector<std::pair<double, double>> v_memo_init = getTimeIntervals(path);

  SPDLOG_DEBUG("cached times");
  for (auto &e : v_memo_init) { 
    SPDLOG_DEBUG("{}, {}", e.first, e.second);
  }  

  std::vector<std::pair<double, double>> v_memo = getTimeIntervals(path);

  SPDLOG_DEBUG("times from memo");
  for (auto &e : v_memo) { 
    SPDLOG_DEBUG("{}, {}", e.first, e.second);
  }  

  EXPECT_EQ(v_nonmemo, v_memo);
  EXPECT_EQ(v_memo, v_memo_init);
}


TEST(UtilTests, testExiringCache) {  
  string tempname = "spiceql-cachetest-" + SpiceQL::gen_random(10);

  fs::path t = fs::temp_directory_path() / tempname / "tests"; 
  fs::create_directories(t);

  // make some stuff 
  fs::create_directory(t / "t1");
  fs::create_directory(t / "t2");
  fs::create_directory(t / "t3");

  vector<string> v1 = Memo::ls(t, false);
  SPDLOG_DEBUG("first ls results {}", fmt::join(v1, ", "));
  
  // this should hit the cache
  vector<string> v2 = Memo::ls(t, false);
  SPDLOG_DEBUG("second ls results {}", fmt::join(v2, ", "));

  // they should be the same 
  EXPECT_EQ(v1, v2);

  // clock is pretty low res
  sleep(2);

  fs::create_directory(t / "t4");
  SPDLOG_DEBUG("added {}", (t / "t4").string());

  vector<string> v3 = Memo::ls(t, false);
  SPDLOG_DEBUG("third ls results {}", fmt::join(v3, ", "));
  
  EXPECT_NE(v2, v3);

}


TEST(UtilTests, testCacheDeleteDep) { 
  string tempname = "spiceql-cachetest-" + SpiceQL::gen_random(10); 
  fs::path t = fs::temp_directory_path() / tempname / "tests"; 
  fs::create_directories(t);

  // make some stuff 
  fs::create_directories(t / "t1");
  fs::create_directories(t / "t2");
  fs::create_directories(t / "t3");

  vector<string> v1 = Memo::ls(t, false);
  SPDLOG_DEBUG("first ls results {}", fmt::join(v1, ", "));

  // clock is pretty low res
  sleep(2);

  // delete a folder
  fs::remove_all(t / "t1");

  // this should miss
  vector<string> v2 = Memo::ls(t, false);
  SPDLOG_DEBUG("second ls results {}", fmt::join(v2, ", "));

  EXPECT_NE(v1, v2); 
}
