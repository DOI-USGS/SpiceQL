#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>

#include "TestUtilities.h"
#include "Fixtures.h"

#include <SpiceQL/inventory.h>
#include <SpiceQL/inventoryimpl.h>
#include <SpiceQL/api.h>

#include <algorithm>
#include <map>
#include <fstream>
#include <set>
#include <SpiceQL/spiceql_logging.h>
#include <highfive/highfive.hpp>


static std::set<std::string> kernelFilenames(const nlohmann::json &list) {
  std::set<std::string> names;
  for (const auto &k : list) names.insert(fs::path(k.get<std::string>()).filename().string());
  return names;
}


TEST_F(LroKernelSet, TestInventorySmithed) {
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"smithed", "reconstructed"}, {"smithed", "reconstructed"}, false);
  SPDLOG_DEBUG("Returned Kernels {} ", kernels.dump());

  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("lro_frames_1111111_v01.tf"));
  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("moon_080317.tf"));
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc"); 
  
  EXPECT_EQ(kernels["spk"].size(), 1);
  
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed");  
  EXPECT_EQ(kernels["lroc_spk_quality"], "smithed");  
}

TEST_F(LroMocQualityKernelSet, TestInventoryQualityUnion) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("moc", {"ck"}, 110000000, 120000000, 
                                                           {"smithed", "reconstructed"}, 
                                                           {"smithed", "reconstructed"}, false);
  SPDLOG_DEBUG("Union kernels: {}", kernels.dump());

  vector<string> cks;
  for (auto &e : kernels["ck"]) cks.push_back(fs::path(e.get<string>()).filename());

  // Both tiers, not just the better one.
  ASSERT_EQ(cks.size(), 2);
  EXPECT_NE(find(cks.begin(), cks.end(), "moc42r_1111111_1111111_v01.bc"), cks.end());
  EXPECT_NE(find(cks.begin(), cks.end(), "LROC_NPOLE_2017Merged_Lidar2Image_Left_2012_ck.bc"), cks.end());

  EXPECT_EQ(cks.back(), "LROC_NPOLE_2017Merged_Lidar2Image_Left_2012_ck.bc");

  EXPECT_EQ(kernels["moc_ck_quality"], "reconstructed");
}


TEST_F(LroMocQualityKernelSet, TestInventoryQualityUnionSingleTier) { 
  Inventory::create_database();
  // Nothing smithed covers this window, so the union is one tier and the answer
  // is exactly what it was before the union existed.
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"ck"}, 130000000, 140000000, 
                                                           {"smithed", "reconstructed"}, 
                                                           {"smithed", "reconstructed"}, false);
  ASSERT_EQ(kernels["ck"].size(), 1);
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "lrolc_1111111_1111111_v11.bc");
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed");
}


TEST_F(LroMocQualityKernelSet, TestInventoryQualityOwnerReporting) {
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"ck"}, 110000000, 120000000,
                                                           {"smithed", "reconstructed"},
                                                           {"smithed", "reconstructed"}, false);
  SPDLOG_DEBUG("Owner reporting kernels: {}", kernels.dump());

  // The searched name reports the weakest link in its chain.
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed");

  ASSERT_TRUE(kernels.contains("moc_ck_quality"));
  EXPECT_EQ(kernels["moc_ck_quality"], "smithed");
}


TEST_F(LroKernelSet, TestInventoryRecon) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"reconstructed"}, {"reconstructed"}, false);
  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("lro_frames_1111111_v01.tf"));
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc");
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed"); 
}


TEST_F(LroKernelSet, TestInventoryPredicted) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"predicted"}, {"predicted"});
  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("lro_frames_1111111_v01.tf"));
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_TRUE(!kernels.contains("cks")); // no cks
}


TEST_F(LroKernelSet, TestInventoryEmpty) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("mgs", {"fk", "sclk", "spk", "ck"});
  EXPECT_TRUE(kernels.empty());
}


TEST_F(LroKernelSet, TestInventoryPortability) { 
  fs::path dbfile = Inventory::getDbFilePath();
  HighFive::File file(dbfile, HighFive::File::ReadOnly);
  
  auto dataset = file.getDataSet("/spice/lroc/sclk");
  vector<string> data = dataset.read<vector<string>>();   
  dataset.read(data);

  // assert that the path in the db is relative 
  EXPECT_EQ(data.at(0), "clocks/lro_clkcor_2020184_v00.tsc");

  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"});

  // these paths should be expanded
  EXPECT_TRUE(kernels["sclk"][0].get<string>().size() == data.at(0).size());
}


TEST_F(KernelsWithQualities, TestUnenforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, {"smithed", "reconstructed"}, {"smithed", "reconstructed"}, false);
  // smithed kernels should not exist so it should return reconstructed
  EXPECT_EQ(kernels["odyssey_spk_quality"].get<string>(), "reconstructed");
}


TEST_F(KernelsWithQualities, TestEnforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, {"smithed"}, {"smithed"}, false);
  // Should be empty since we are enforcing smithed
  EXPECT_TRUE(kernels.is_null());
}

TEST_F(LroKernelSet, TestInventorySearch) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"lsk", "sclk", "ck", "spk", "iak", "fk"}, 110000000, 140000001, {"smithed", "reconstructed"}, {"smithed", "reconstructed"}, false);
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["ck"].size(), 2);
  EXPECT_EQ(fs::path(kernels["ck"][0].get<string>()).filename(), "soc31_1111111_1111111_v21.bc" );
  EXPECT_EQ(fs::path(kernels["iak"][0].get<string>()).filename(), "lro_instrumentAddendum_v11.ti");
  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("lro_frames_1111111_v01.tf"));
  EXPECT_EQ(fs::path(kernels["sclk"][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}

TEST_F(LroKernelSet, TestInventorySearchSetsNoOverwrite) {
  // do a time query
  pair<string, nlohmann::json> result = searchForKernelsets({"moon", "base"}, {"pck", "fk"}, 110000000, 140000001, {"reconstructed"}, {"reconstructed"}, false);
  nlohmann::json kernels = result.second;
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["pck"].size(), 1);
  EXPECT_TRUE(kernelFilenames(kernels["pck"]).count("pck00009.tpc"));
  EXPECT_TRUE(kernelFilenames(kernels["fk"]).count("moon_080317.tf"));
}


TEST_F(TempTestingFiles, SpiceQLPerformanceInventory) { 
	std::ofstream outfile;
  outfile.open((fs::path(SpiceQL::getCacheDir()) / "spiceqltimes_themis.txt").string(), std::ios_base::app); // append instead of overwrite
  for(int i = 0; i < 1; i++) {
    const clock_t begin_time = clock();
    SPDLOG_DEBUG("KERNELS: {}", searchForKernelsets({"odyssey", "mars"}, KERNEL_TYPES, 715662878.32324, 715663065.2303).second.dump());
    float seconds = float( clock () - begin_time ) /  CLOCKS_PER_SEC;
    SPDLOG_DEBUG("SECONDS: {}", seconds);
    outfile << seconds << endl;
  }
}


TEST_F(LroKernelSet, TestInventorySearchSetsOverwrite) {
  // do a time query
  pair<string, nlohmann::json> result = searchForKernelsets({"moon", "base"}, {"pck"}, 110000000, 140000001, {"reconstructed"}, {"reconstructed"}, false, -1, 1, true);
  nlohmann::json kernels = result.second;
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["pck"].size(), 1);
  EXPECT_TRUE(kernelFilenames(kernels["pck"]).count("pck00009.tpc"));
}


TEST_F(LroKernelSet, TestInventorySearchSetFromRegex) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset_from_regex({"/lroc/sclk/.*", "/lroc/spk/smithed/LRO_TEST_GRGM660MAT[0-9]{3}.bsp", "/iau_moon/fk/moon.*"});
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["spk"].size(), 3);
  EXPECT_EQ(kernels["spk_quality"].get<string>(), "smithed");
  EXPECT_EQ(fs::path(kernels["fk"][0].get<string>()).filename(), "moon_080317.tf");
}

TEST_F(TempTestingFiles, TestInventorySetCacheDir) {
  SpiceQL::Inventory::setDbFilePath(tempDir.string());
  const char* cache_dir = getenv("SPICEQL_CACHE_DIR");
  EXPECT_EQ(SpiceQL::Inventory::getDbFilePath(), fs::path(cache_dir)/"spiceqldb.hdf");
}

TEST_F(TempTestingFiles, TestInventorySetCacheDirOverride) { 
  fs::path new_path = fs::path(tempDir.string())/"new_path";
  SpiceQL::Inventory::setDbFilePath(new_path.string(), true);
  EXPECT_EQ(SpiceQL::Inventory::getDbFilePath(), new_path/"spiceqldb.hdf");
}

TEST(TestInventory, GetCacheDirAutoInitialize) {
  // Test getCacheDir auto-initialization

  // Save original state
  const char* original_cache_dir = getenv("SPICEQL_CACHE_DIR");

  // Clear the environment variable and force reset internal cache directory
  unsetenv("SPICEQL_CACHE_DIR");
  SpiceQL::setCacheDir("", true);  // Reset internal state to trigger auto-init path

  // getCacheDir should auto-initialize with a temp directory instead of throwing
  std::string cache_dir;
  EXPECT_NO_THROW({
    cache_dir = SpiceQL::getCacheDir();
  });

  // Verify auto-initialized directory is not empty
  EXPECT_FALSE(cache_dir.empty());

  // Verify it contains "spiceql-cache" in the path
  EXPECT_TRUE(cache_dir.find("spiceql-cache") != std::string::npos);

  // Verify the directory was created
  EXPECT_TRUE(fs::exists(cache_dir));
  EXPECT_TRUE(fs::is_directory(cache_dir));

  // Verify subsequent calls return the same cached value
  std::string cache_dir2 = SpiceQL::getCacheDir();
  EXPECT_EQ(cache_dir, cache_dir2);

  // Restore original environment
  if (original_cache_dir != nullptr) {
    setenv("SPICEQL_CACHE_DIR", original_cache_dir, 1);
  }
}

TEST(TestInventory, SetCacheDirPriority) {
  // Test setCacheDir priority order: override > env var > provided value > auto-generate

  // Save original state
  const char* original_cache_dir = getenv("SPICEQL_CACHE_DIR");

  fs::path env_dir = fs::temp_directory_path() / "spiceql-env-test";
  fs::path provided_dir = fs::temp_directory_path() / "spiceql-provided-test";

  // Test 1: env var takes precedence over provided value when override=false
  setenv("SPICEQL_CACHE_DIR", env_dir.string().c_str(), 1);
  SpiceQL::setCacheDir(provided_dir.string(), false);
  EXPECT_EQ(SpiceQL::getCacheDir(), env_dir.string());

  // Test 2: override=true forces provided value even with env var set
  SpiceQL::setCacheDir(provided_dir.string(), true);
  EXPECT_EQ(SpiceQL::getCacheDir(), provided_dir.string());

  // Restore original environment
  if (original_cache_dir != nullptr) {
    setenv("SPICEQL_CACHE_DIR", original_cache_dir, 1);
  } else {
    unsetenv("SPICEQL_CACHE_DIR");
  }
}

TEST_F(LroKernelSet, InferMissionInitCache) {
  // Test inferMission triggers cache auto-init
  // Save original state
  const char* original_cache_dir = getenv("SPICEQL_CACHE_DIR");

  // Clear environment to force auto-initialization
  unsetenv("SPICEQL_CACHE_DIR");

  // Empty mission triggers: inferMission() → frameList() → getCacheDir() → auto-init
  // Auto-initializes with temp directory
  std::pair<nlohmann::json, nlohmann::json> result;
  EXPECT_NO_THROW({
    result = SpiceQL::getTargetFrameInfo(301, "");
  });

  // Verify cache was auto-initialized
  std::string cache_dir = SpiceQL::getCacheDir();
  EXPECT_FALSE(cache_dir.empty());
  EXPECT_TRUE(fs::exists(cache_dir));

  // Restore original environment
  if (original_cache_dir != nullptr) {
    setenv("SPICEQL_CACHE_DIR", original_cache_dir, 1);
  }
}


TEST_F(LroKernelSet, TestInventoryIntervals) {
  fs::path dbfile = Inventory::getDbFilePath();
  HighFive::File file(dbfile, HighFive::File::ReadOnly);

  std::string group = "/spice/lroc/ck/reconstructed/";
  auto paths = file.getDataSet(group + "path_index").read<std::vector<std::string>>();

  // The existing time index is still written as before
  EXPECT_EQ(file.getDataSet(group + "starttime").read<std::vector<double>>().size(), paths.size());

  // Each fixture CK has one segment and one interpolation interval for -85000
  std::map<std::string, std::pair<double, double>> expected = {
    {"soc31_1111111_1111111_v21.bc", {110000000, 120000000}},
    {"lrolc_1111111_1111111_v11.bc", {130000000, 140000000}}
  };

  auto starts = file.getDataSet(group + "intervals_start").read<std::vector<double>>();
  auto stops = file.getDataSet(group + "intervals_stop").read<std::vector<double>>();
  auto start_kindex = file.getDataSet(group + "intervals_start_kindex").read<std::vector<size_t>>();
  auto stop_kindex = file.getDataSet(group + "intervals_stop_kindex").read<std::vector<size_t>>();
  auto codes = file.getDataSet(group + "interval_body_code").read<std::vector<int>>();

  ASSERT_EQ(starts.size(), 2);
  ASSERT_EQ(stops.size(), 2);
  ASSERT_EQ(codes.size(), 2);
  EXPECT_EQ(start_kindex, stop_kindex);

  for (size_t i = 0; i < starts.size(); i++) {
    // rows are grouped by kernel in path_index order
    EXPECT_EQ(start_kindex[i], i);
    std::string name = fs::path(paths.at(start_kindex[i])).filename().string();
    ASSERT_TRUE(expected.count(name)) << name;
    EXPECT_NEAR(starts[i], expected[name].first, 1e-3) << name;
    EXPECT_NEAR(stops[i], expected[name].second, 1e-3) << name;
    EXPECT_EQ(codes[i], -85000) << name;
  }

  // Segments aren't stored
  EXPECT_FALSE(file.exist(group + "segments_start"));
}


TEST_F(LroKernelSet, TestGetKernelIntervals) {
  std::string soc_key = "/lroc/ck/reconstructed/soc31_1111111_1111111_v21.bc";
  std::string lrolc_key = "/lroc/ck/reconstructed/lrolc_1111111_1111111_v11.bc";

  // By filename, by SpiceQL key, and a kernel that isn't in the DB
  auto [intervals, kernels] = getKernelIntervals({"soc31_1111111_1111111_v21.bc",
                                                     lrolc_key,
                                                     "not_a_kernel.bc"});
  SPDLOG_DEBUG("Intervals: {}", intervals.dump());
  EXPECT_TRUE(kernels.empty());

  ASSERT_TRUE(intervals.contains(soc_key)) << intervals.dump();
  ASSERT_TRUE(intervals.contains(lrolc_key)) << intervals.dump();
  // A filename matches every group the kernel is in, a SpiceQL key only its own
  EXPECT_TRUE(intervals.contains("/minirf/ck/reconstructed/soc31_1111111_1111111_v21.bc")) << intervals.dump();
  EXPECT_FALSE(intervals.contains("/minirf/ck/reconstructed/lrolc_1111111_1111111_v11.bc")) << intervals.dump();
  for (auto &[k, v] : intervals.items()) {
    EXPECT_EQ(k.find("not_a_kernel"), std::string::npos) << k;
  }

  ASSERT_EQ(intervals[soc_key].size(), 1);
  EXPECT_NEAR(intervals[soc_key][0]["start"].get<double>(), 110000000, 1e-3);
  EXPECT_NEAR(intervals[soc_key][0]["stop"].get<double>(), 120000000, 1e-3);
  EXPECT_EQ(intervals[soc_key][0]["body_code"], -85000);

  ASSERT_EQ(intervals[lrolc_key].size(), 1);
  EXPECT_NEAR(intervals[lrolc_key][0]["start"].get<double>(), 130000000, 1e-3);
  EXPECT_NEAR(intervals[lrolc_key][0]["stop"].get<double>(), 140000000, 1e-3);

  // A SpiceQL key only matches its own group
  auto [wrong_group, _] = getKernelIntervals({"/lroc/ck/smithed/soc31_1111111_1111111_v21.bc"});
  EXPECT_TRUE(wrong_group.empty()) << wrong_group.dump();

  // A path to the kernel, as searchForKernelsets returns it
  nlohmann::json found = Inventory::search_for_kernelset("lroc", {"ck"}, 110000000, 120000000, {"reconstructed"}, {"reconstructed"}, true);
  ASSERT_EQ(found["ck"].size(), 1);
  auto [by_path, __] = getKernelIntervals({found["ck"][0].get<std::string>()});
  EXPECT_EQ(by_path[soc_key], intervals[soc_key]);
}


TEST_F(LroKernelSet, TestSearchForKernelsetsGetIntervals) {
  // Off by default
  auto [ret, kernels] = searchForKernelsets({"lroc"}, {"ck"}, 110000000, 140000000, {"reconstructed"}, {"reconstructed"});
  EXPECT_FALSE(kernels.contains("intervals"));

  auto [ret2, with_intervals] = searchForKernelsets({"lroc", "base"}, {"ck", "fk"}, 110000000, 140000000, {"reconstructed"}, {"reconstructed"},
                                                    false, false, -1, 1, false, true);
  SPDLOG_DEBUG("Kernels: {}", with_intervals.dump());
  ASSERT_TRUE(with_intervals.contains("intervals"));
  ASSERT_EQ(with_intervals["ck"].size(), 2);

  // Same output as getKernelIntervals, one entry per CK found. By path the
  // fixture CKs are also in minirf, so look them up by their lroc SpiceQL keys.
  vector<string> cks;
  for (auto &ck : with_intervals["ck"]) {
    cks.push_back("/lroc/ck/reconstructed/" + fs::path(ck.get<string>()).filename().string());
  }
  auto [expected, _] = getKernelIntervals(cks);
  EXPECT_EQ(with_intervals["intervals"], expected);
  EXPECT_EQ(with_intervals["intervals"].size(), 2);

  // Still loadable as a kernel set
  KernelSet kset(with_intervals);
}
