#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>

#include "TestUtilities.h"
#include "Fixtures.h"

#include <SpiceQL/inventory.h>
#include <SpiceQL/inventoryimpl.h>
#include <SpiceQL/api.h>

#include <fstream>
#include <SpiceQL/spiceql_logging.h>
#include <highfive/highfive.hpp>


TEST_F(LroKernelSet, TestInventorySmithed) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"smithed", "reconstructed"}, {"smithed", "reconstructed"}, false);
  SPDLOG_DEBUG("Returned Kernels {} ", kernels.dump());
  
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc"); 
  
  EXPECT_EQ(kernels["spk"].size(), 1);
  
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed");  
  EXPECT_EQ(kernels["lroc_spk_quality"], "smithed");  
}

TEST_F(LroKernelSet, TestInventoryRecon) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"reconstructed"}, {"reconstructed"}, false);
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc");
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed"); 
}


TEST_F(LroKernelSet, TestInventoryPredicted) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"predicted"}, {"predicted"});
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
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
  EXPECT_EQ(fs::path(kernels["fk"][0].get<string>()).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}

TEST_F(LroKernelSet, TestInventorySearchSetsNoOverwrite) {
  // do a time query
  pair<string, nlohmann::json> result = searchForKernelsets({"moon", "base"}, {"pck"}, 110000000, 140000001, {"reconstructed"}, {"reconstructed"}, false);
  nlohmann::json kernels = result.second;
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["pck"].size(), 2);
  // Check that both expected kernels are present (order may vary)
  std::set<std::string> filenames;
  for (const auto& kernel : kernels["pck"]) {
    filenames.insert(fs::path(kernel.get<string>()).filename().string());
  }
  EXPECT_TRUE(filenames.count("moon_080317.tf"));
  EXPECT_TRUE(filenames.count("pck00009.tpc"));
}


TEST_F(TempTestingFiles, SpiceQLPerformanceInventory) { 
	std::ofstream outfile;
  outfile.open("/home/ec2-user/spiceqltimes_themis.txt", std::ios_base::app); // append instead of overwrite
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
  EXPECT_EQ(kernels["pck"].size(), 2);
  // Check that pck00009.tpc is present (order may vary)
  std::set<std::string> filenames;
  for (const auto& kernel : kernels["pck"]) {
    filenames.insert(fs::path(kernel.get<string>()).filename().string());
  }
  EXPECT_TRUE(filenames.count("pck00009.tpc"));
}


TEST_F(LroKernelSet, TestInventorySearchSetFromRegex) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset_from_regex({"/lroc/sclk/.*", "/lroc/spk/smithed/LRO_TEST_GRGM660MAT[0-9]{3}.bsp", "/iau_moon/pck/moon.*"});
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["spk"].size(), 3);
  EXPECT_EQ(kernels["spk_quality"].get<string>(), "smithed"); 
  EXPECT_EQ(fs::path(kernels["pck"][0].get<string>()).filename(), "moon_080317.tf");
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

  // Clear the environment variable and reset internal state
  unsetenv("SPICEQL_CACHE_DIR");

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
